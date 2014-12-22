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

pthread_t reportDispatcher;
pthread_t committeeDispatcher;


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
  const int list = *((int *) data);

  /* Acquire mutex for `read` operation. */
  processResultsServiceInitialProtocol(&syncTools, &syncVariables);
  
  /* Process the data. */
  prepareAndSendCompleteReport(&sharedData, list, queueIds.reportDataIPCQueueId);
   
  /* Perform ending protocol. */
  processResultsServiceEndingProtocol(&syncTools, &syncVariables);

  /* After passing data for specific list or complete set of all lists
	   put back group access token of value `list` to appropriate IPC queue. */
  putBackGroupAccessToken(queueIds.reportGroupAccessTokenIPCQueueId, list);
  
  fprintf(stderr, "--------------------------------\n");
  fprintf(stderr, "Raport na listę: %d chce zbudzić czekającego..\n", list);
  wakeUpAwaitingThread();
  fprintf(stderr, "Raport na listę: %d zbudził czekającego..\n", list);

  return 0;
}

void* reportDispatcherThread(void* data) {
  int list;
  pthread_t reportWorkerThreads[ALL_LISTS_ID + 1];

  /* Initialize all report group access tokens in appropriate  IPC queue. */
  initializeReportGroupAccessTokenIPCQueue(
    queueIds.reportGroupAccessTokenIPCQueueId, sharedData.committees);

  while (1) {
    receiveReportRequestMessage(queueIds.reportDataIPCQueueId, &list);
    
    fprintf(stderr, "Raport, czekam na wejście, lista: %d\n", list);
    waitBeforeSpawningWorkerThread();
    fprintf(stderr, "Raport, wszedłem, lista: %d\n", list);

    /* Spawn new report-dedicated thread to handle report sending. */
    if (pthread_create(&reportWorkerThreads[list],
      &syncTools.workerThreadAttribute, reportWorkerThread,
      (void*) &list) != 0)
      syserr(REPORT_WORKER_THREAD_INITIALIZATION_ERROR_CODE);
  }
 
  return 0;
}

void* committeeWorkerThread(void* data) {
  committeeMessage message;
  committeeWorkerResources resources;
  
  /* Initialize resources. */
  int transferFinished = 0;
  long committee = *((long*) data);
  fprintf(stderr, "Komisja numer: %ld\n", committee);
  fprintf(stderr, "Tyle jest teraz pisarzy: %d\n", syncVariables.workingThreads);
  initializeCommitteeWorkerResources(&sharedData, &resources, committee);

  fprintf(stderr, "Komisja: %ld zaczyna przesyłać wyniki\n", committee);
  /* Process committee data. */
  while (!transferFinished) {
    receiveCommitteeMessage(queueIds.committeeDataIPCQueueId, &message,
      committee);
    
    switch (message.type) {
      case HEADER:
        resources.eligibledVoters = message.localInfo.eligibledVoters;
        resources.totalVotes = message.localInfo.totalVotes;
        ++resources.processedMessages;
        break;
      case DATA:
        resources.partialResults[message.list][message.candidate] =
          message.candidateVotes;
        resources.validVotes += message.candidateVotes;
        ++resources.processedMessages;
        break;
      case FINISH:
        transferFinished = 1;
        break;
    }
  }
  fprintf(stderr, "Komisja: %ld skończyła nadawać!\n", committee);
  
  /* Send ack message to committee. */
  sendAckMessage(queueIds.finishIPCQueueId, committee,
    resources.processedMessages, resources.validVotes);
  
  fprintf(stderr, "Komisja będzie chciała zapisać dane!\n");
  /* Wait for exclusive access to update shared data. */
  updateResultsServiceInitialProtocol(&syncTools, &syncVariables);
  
  fprintf(stderr, "Locked!\n");
  /* Update shared data. */
  updateSharedData(&sharedData, &syncVariables, &resources);
  fprintf(stderr, "Update zakończony pomyślnie.\n");

  /* Release mutex and wake up awaiting thread. */
  updateResultsServiceEndingProtocol(&syncTools, &syncVariables);

  fprintf(stderr, "--------------------------------\n");
  fprintf(stderr, "Komisja: %ld chce wpuścić czekającego..\n", committee);
  wakeUpAwaitingThread();
  fprintf(stderr, "Komisja: %ld wpuściła czekającego\n", committee);
  return 0;
}

void* committeeDispatcherThread(void* data) {
  long committee;
  long committee_available;
  
  initialConnectionMessage initMessage;
  int initialConnectionMessageSize;

  pthread_t committeeWorkerThreads[MAX_COMMITTEES + 1];

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

    fprintf(stderr, "-----------------------------------\n");
    fprintf(stderr, "Ogłaszam się komisja: %ld\n", committee);
    fprintf(stderr, "-----------------------------------\n");

    /* Send response back to the specific committee. */
    initMessage.operationId = committee;
    initMessage.ack = (committee_available == 0) ?
      CONNECTION_SUCCEEDED : CONNECTION_REFUSED;
    if (msgsnd(queueIds.initConnectionIPCQueueId, (void*) &initMessage,
      initialConnectionMessageSize, 0) != 0)
      syserr(IPC_QUEUE_SEND_OPERATION_ERROR_CODE);

    fprintf(stderr, "-----------------------------------\n");
    fprintf(stderr, "Committee_available: %ld\n", committee_available);
    fprintf(stderr, "-----------------------------------\n");
    
    /* In case of connection refused statement, stop further processing. */
    if (committee_available == 1)
      continue;
    
    fprintf(stderr, "Czekam na wejście, komisja: %ld\n", committee);
    waitBeforeSpawningWorkerThread();
    fprintf(stderr, "Wchodzę, komisja: %ld\n", committee);

    /* Spawn new committee-dedicated thread processing committee data. */

    fprintf(stderr, "Przed stworzeniem, komisja nr: %ld\n", committee);
    if (pthread_create(&committeeWorkerThreads[committee],
      &syncTools.workerThreadAttribute, committeeWorkerThread,
      (void*) &committee) != 0)
      syserr(COMMITTEE_WORKER_THREAD_INITIALIZATION_ERROR_CODE);
   fprintf(stderr, "Po stworzeniu, komisja nr: %ld\n", committee);
  }

  return 0;
}

/* Signal handling. */

void SIGUSR1handler(int signal) {
  fprintf(stderr, "\nThread %lx received SIGUSR1.\n", pthread_self());
}

void freeResources(int signum) {
  fprintf(stderr, "\nFree resources..\n");

  /* Kill dispatcher threads..*/
  pthread_kill(reportDispatcher, SIGUSR1);
  pthread_kill(committeeDispatcher, SIGUSR1);

  /* Free resources. */
  freeServerIPCQueuesResources(&queueIds);
  destroyServerSyncTools(&syncTools);
  
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
  if (pthread_create(&reportDispatcher, &syncTools.dispatcherThreadAttribute,
    reportDispatcherThread, (void*) &syncTools.workerThreadAttribute) != 0)
    syserr(REPORT_DISPATCHER_THREAD_INITIALIZATION_ERROR_CODE);
  if (pthread_create(&committeeDispatcher,
    &syncTools.dispatcherThreadAttribute, committeeDispatcherThread,
    (void*) &syncTools.workerThreadAttribute) != 0)
    syserr(COMMITTEE_DISPATCHER_THREAD_INITIALIZATION_ERROR_CODE);
  
  /* Wait for dispatcher threads. */
  if (pthread_join(reportDispatcher, 0) != 0)
    syserr(REPORT_DISPATCHER_THREAD_JOIN_ERROR_CODE);
  if (pthread_join(committeeDispatcher, 0) != 0)
    syserr(COMMITTEE_DISPATCHER_THREAD_JOIN_ERROR_CODE);

  return EXIT_SUCCESS;
}
