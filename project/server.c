/*
     Maciej Szeszko,
     id: ms335814,
     University of Warsaw
*/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#include "constants.h"
#include "err.h"
#include "error_codes.h"
#include "message_structures.h"
#include "process_results_service.h"
#include "send_report_service.h"
#include "server_committee_service.h"
#include "server_service.h"
#include "server_structures.h"
#include "update_results_service.h"


sharedIPCQueueIds queueIds;
sharedDataStructures sharedData;
sharedSynchronizationVariables syncVariables;
sharedSynchronizationTools syncTools;
sharedThreadVariables threads;


void waitBeforeSpawningWorkerThread() {
  /* Wait to proces the data until there will be less than maximal
     number of dedicated working threads. */
  pthread_mutex_lock(&syncTools.mutex);
  while (syncVariables.workingThreads ==
    MAX_DEDICATED_SERVER_THREADS) {
    pthread_cond_wait(&syncTools.workingThreadsCondition,
      &syncTools.mutex);
  }
  ++(syncVariables.workingThreads);
  pthread_mutex_unlock(&syncTools.mutex);
}

void wakeUpAwaitingThread() {
  /* Wake up any awaiting worker thread! */
  pthread_mutex_lock(&syncTools.mutex);
  /* Decreasing #(working threads) when having mutex instead of during
     data update prevents from starvation of awaiting processes. */
  --(syncVariables.workingThreads);
  pthread_cond_signal(&syncTools.workingThreadsCondition);
  pthread_mutex_unlock(&syncTools.mutex);
}

void* reportWorkerThread(void* data) {
  /*const int list = *((int *) data);*/
  reportThreadData threadData = *((reportThreadData*) data);

  /* Acquire mutex for `read` operation. */
  processResultsServiceInitialProtocol(&syncTools, &syncVariables);
  
  /* Process the data. */
  prepareAndSendCompleteReport(&sharedData, threadData.pid, threadData.list, queueIds.reportDataIPCQueueId);
   
  /* Perform ending protocol. */
  processResultsServiceEndingProtocol(&syncTools, &syncVariables);

  /* Release index in thread array. */
  releaseIndexInThreadsArray(&syncTools, &threads, threadData.threadIndex);

  fprintf(stderr, "--------------------------------\n");
  fprintf(stderr, "Raport na listę: %d chce zbudzić czekającego..\n", threadData.list);
  wakeUpAwaitingThread();
  fprintf(stderr, "Raport na listę: %d zbudził czekającego..\n", threadData.list);
 
  
  return 0;
}

void* reportDispatcherThread(void* data) {
  reportThreadData threadData;

  /*pthread_t reportWorkerThreads[ALL_LISTS_ID + 1];*/
  
  /* Initialize all report group access tokens in appropriate  IPC queue. */
  /*initializeReportGroupAccessTokenIPCQueue(
    queueIds.reportGroupAccessTokenIPCQueueId, sharedData.committees);*/
  
  while (1) {
    receiveReportRequestMessage(queueIds.reportDataIPCQueueId,
      &threadData.pid, &threadData.list);
    
    fprintf(stderr, "Raport, czekam na wejście, lista: %d\n", threadData.list);
    waitBeforeSpawningWorkerThread();
    fprintf(stderr, "Raport, wszedłem, lista: %d\n", threadData.list);

    threadData.threadIndex = findIndexInThreadsArray(&syncTools, &threads);
    /* Spawn new report-dedicated thread to handle report sending. */
    if (pthread_create(&threads.workerThreads[threadData.threadIndex],
      &syncTools.workerThreadAttribute, reportWorkerThread,
      (void*) &threadData) != 0)
      syserr(REPORT_WORKER_THREAD_INITIALIZATION_ERROR_CODE);
  }
  
  return 0;
}

void* committeeWorkerThread(void* data) {
  committeeMessage message;
  committeeWorkerResources resources;

  int** partialResults;
  int i;

  /* Initialize resources. */
  int transferFinished = 0;
  committeeThreadData threadData = *((committeeThreadData*) data);
  
  int rows = sharedData.lists;
  int columns = sharedData.candidatesPerList;

  partialResults = (int**) malloc ((rows + 1) * sizeof(int*));
  for (i = 0; i<= rows; ++i)
    partialResults[i] = (int*) malloc ((columns + 1) * sizeof(int));
  
  syncVariables.partialResults[threadData.threadIndex] = partialResults;
  
  /*initializeDynamicCommitteeStructure(partialResults, rows, columns);*/

  initializeCommitteeWorkerResources(&sharedData, &resources, partialResults,
    threadData.committee);
  /* Process committee data. */
  while (!transferFinished) {
    receiveCommitteeMessage(queueIds.committeeDataIPCQueueId, &message,
      threadData.committee);
    
    switch (message.type) {
      case HEADER:
        resources.eligibledVoters = message.localInfo.eligibledVoters;
        resources.totalVotes = message.localInfo.totalVotes;
        ++(resources.processedMessages);
        break;
      case DATA:
        partialResults[message.list][message.candidate] =
          message.candidateVotes;
        resources.validVotes += message.candidateVotes;
        ++(resources.processedMessages);
        break;
      case FINISH:
        transferFinished = 1;
        break;
    }
  }
  
  /* Send ack message to committee. */
  sendAckMessage(queueIds.finishIPCQueueId, threadData.committee,
    resources.processedMessages, resources.validVotes);
  
  /* Wait for exclusive access to update shared data. */
  updateResultsServiceInitialProtocol(&syncTools, &syncVariables);
  
  /* Update shared data and free dynamically allocated resources. */
  updateSharedData(&sharedData, &syncVariables, &resources, partialResults);
  /*freeDynamicCommitteeStructure(partialResults, rows);*/
  
  /* Release mutex and wake up awaiting thread. */
  updateResultsServiceEndingProtocol(&syncTools, &syncVariables);

  /* Release index in thread array. */
  releaseIndexInThreadsArray(&syncTools, &threads, threadData.threadIndex);

  wakeUpAwaitingThread();
  
  return 0;
}

void* committeeDispatcherThread(void* data) {
  long committee;
  long committee_available;
  
  committeeThreadData threadData; 
 
  initialConnectionMessage initMessage;
  int initialConnectionMessageSize;

  initialConnectionMessageSize =
    sizeof(initialConnectionMessage) - sizeof(long);

  /* Handle committees connection requests. */
  while (1) {
    receiveConnectionRequest(queueIds.initConnectionIPCQueueId, &committee);

    /* Make sure that list is a one of all possible committee values. */
    if (committee >= 1 && committee <= sharedData.committees) {
    	/* Check whether committee connection has been arranged yet or not.
    	   Connection available: 0, connection arranged yet: 1. */
    	pthread_mutex_lock(&syncTools.mutex);
    	committee_available = sharedData.committeeConnectionPossible[committee];
    	if (committee_available == 0)
    	  sharedData.committeeConnectionPossible[committee] = 1;
    	pthread_mutex_unlock(&syncTools.mutex);
    } else {
      committee_available = 1;
    }

    /* Send response back to the specific committee. */
    initMessage.operationId = committee;
    initMessage.ack = (committee_available == 0) ?
      CONNECTION_SUCCEEDED : CONNECTION_REFUSED;
    if (msgsnd(queueIds.initConnectionIPCQueueId, (void*) &initMessage,
      initialConnectionMessageSize, 0) != 0)
      syserr(IPC_QUEUE_SEND_OPERATION_ERROR_CODE);
    
    /* In case of connection refused statement, stop further processing. */
    if (committee_available == 1)
      continue;
    
    waitBeforeSpawningWorkerThread();

    /* Initialize commitee thread data. */   
    threadData.threadIndex = findIndexInThreadsArray(&syncTools, &threads);
    threadData.committee = committee;
     
    /* Spawn new committee-dedicated thread processing committee data. */

    if (pthread_create(&threads.workerThreads[threadData.threadIndex],
      &syncTools.workerThreadAttribute, committeeWorkerThread,
      (void*) &threadData) != 0)
      syserr(COMMITTEE_WORKER_THREAD_INITIALIZATION_ERROR_CODE);
  }

  return 0;
}

/* Signal handling. */
void freeResources(int signum) {
  int i;
  
  /* Cancel dispatcher threads..*/
  pthread_cancel(threads.reportDispatcher);
  pthread_cancel(threads.committeeDispatcher);

  /* Cancel all worker threads. */
  for (i = 0; i< MAX_DEDICATED_SERVER_THREADS; ++i)
    if (threads.workerThreads[i] == 0)
      pthread_cancel(threads.workerThreads[i]);
  
  /* Free resources. */
  /*freeCommitteeAllocatedStructures(&syncVariables, &sharedData); */
  freeServerIPCQueuesResources(&queueIds);
  destroyServerSyncTools(&syncTools);
  
  fprintf(stderr, "\nFree resources..\n");
  exit(signum);
}

int main(int argc, char** argv) {
  
  struct sigaction setup_action;
  sigset_t block_mask;
  
  int dispatcherThreadDetachState = PTHREAD_CREATE_JOINABLE;
  int workerThreadDetachState = PTHREAD_CREATE_DETACHED;

  if (argc != 4) {
    printf(SERVER_USAGE_ERROR_CODE, argv[0]);
    exit(EXIT_FAILURE);
  }

  /* Parse constraints from programs' argument list. */
  sharedData.lists = atoi(argv[1]);
  sharedData.candidatesPerList = atoi(argv[2]);
  sharedData.committees = atoi(argv[3]);
  
  /* Set signal handling. */
  sigfillset(&block_mask);
  sigdelset(&block_mask, SIGINT);
  sigdelset(&block_mask, SIGUSR1);
  
  setup_action.sa_handler = freeResources;
  setup_action.sa_mask = block_mask;
  
  if (sigaction(SIGINT, &setup_action, 0) == -1)
    syserr(SIGNAL_HANDLING_INITIALIZATION_ERROR_CODE);

  /* Initialize server IPC queues and synchronization tools. */
  initializeServerSyncTools(&syncTools);
  initializeServerIPCQueues(&queueIds);

  /* Initialize distinct thread attributes. */
  initializeThreadAttribute(&syncTools.dispatcherThreadAttribute,
    dispatcherThreadDetachState);
  initializeThreadAttribute(&syncTools.workerThreadAttribute,
    workerThreadDetachState);

  /* Create dispatcher threads. */
  if (pthread_create(&threads.reportDispatcher,
    &syncTools.dispatcherThreadAttribute, reportDispatcherThread,
    (void*) &syncTools.workerThreadAttribute) != 0)
    syserr(REPORT_DISPATCHER_THREAD_INITIALIZATION_ERROR_CODE);
  if (pthread_create(&threads.committeeDispatcher,
    &syncTools.dispatcherThreadAttribute, committeeDispatcherThread,
    (void*) &syncTools.workerThreadAttribute) != 0)
    syserr(COMMITTEE_DISPATCHER_THREAD_INITIALIZATION_ERROR_CODE);
  
  /* Wait for dispatcher threads. */
  if (pthread_join(threads.reportDispatcher, 0) != 0)
    syserr(REPORT_DISPATCHER_THREAD_JOIN_ERROR_CODE);
  if (pthread_join(threads.committeeDispatcher, 0) != 0)
    syserr(COMMITTEE_DISPATCHER_THREAD_JOIN_ERROR_CODE);

  return EXIT_SUCCESS;
}
