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

#include "constants.h"
#include "err.h"
#include "error_codes.h"
#include "message_structures.h"
#include "process_results_service.h"
#include "send_report_service.h"
#include "server_committee_service.h"
#include "server_config_service.h"
#include "server_service.h"
#include "server_structures.h"
#include "update_results_service.h"


sharedIPCQueueIds queueIds;
sharedDataStructures sharedData;
sharedSynchronizationVariables syncVariables;
sharedSynchronizationTools syncTools;


void* reportWorkerThread(void* data) {
  const unsigned int list = *((unsigned int *) data);
  /* Acquire mutex for `read` operation. */
  /*processResultsServiceInitialProtocol(&syncTools, &syncVariables);*/
  
  /* Process the data. */
  prepareAndSendCompleteReport(&sharedData, list, queueIds.reportDataIPCQueueId);
  /* Perform ending protocol. */
  /*processResultsServiceEndingProtocol(&syncTools, &syncVariables);*/

  /* After passing data for specific list or complete set of all lists
	   put back group access token of value `list` to appropriate IPC queue. */
  putBackGroupAccessToken(queueIds.reportGroupAccessTokenIPCQueueId, list);
 
  return 0;
}

void* reportDispatcherThread(void* data) {
  unsigned int list;
  pthread_t reportWorkerThreads[ALL_LISTS_ID + 1];

  /* Initialize all report group access tokens in appropriate  IPC queue. */
  initializeReportGroupAccessTokenIPCQueue(
    queueIds.reportGroupAccessTokenIPCQueueId, sharedData.committees);

  while (1) {
    receiveReportRequestMessage(queueIds.reportDataIPCQueueId, &list);
    
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
  unsigned int transferFinished = 0;
  long committee = *((long*) data);
  initializeCommitteeWorkerResources(&sharedData, &resources, committee);

  /* Process committee data. */
  while (transferFinished == 0) {
    receiveCommitteeMessage(queueIds.committeeDataIPCQueueId, &message,
      committee);
    
    switch (message.type) {
      case HEADER:
        resources.eligibledVoters = message.localInfo.eligibledVoters;
        resources.totalVotes = message.localInfo.totalVotes;
        fprintf(stderr, "HEADER--------\n");
        break;
      case DATA:
        resources.partialResults[message.list][message.candidate] =
          message.candidateVotes;
        resources.validVotes += message.candidateVotes;
        fprintf(stderr, "DATA----------\n");
        break;
      case FINISH:
        transferFinished = 1;
        fprintf(stderr, "FINISH--------\n");
        break;
    }

    ++resources.processedMessages;
  }
  fprintf(stderr, "Wyszedłem\n");
  /* Send ack message to committee. */
  sendAckMessage(queueIds.finishIPCQueueId, committee,
    resources.processedMessages, resources.validVotes);
 
  /* Wait for exclusive access to update shared data. */
  /*updateResultsServiceInitialProtocol(&syncTools, &syncVariables);*/

  /* Update shared data. */
  updateSharedData(&sharedData, &syncVariables, &resources);

  /* Release mutex and wake up awaiting thread. */
  /*updateResultsServiceEndingProtocol(&syncTools, &syncVariables);*/
  
  /* Wake up awaiting worker thread! */
  pthread_mutex_lock(&syncTools.mutex);
  pthread_cond_signal(&syncTools.committeeWorkingThreadsCondition);
  pthread_mutex_unlock(&syncTools.mutex);

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

    /* Wait to process committee data  until there will be less than maximal
       number of committee-dedicated working threads. */
    pthread_mutex_lock(&syncTools.mutex);
    while (syncVariables.workingCommitteeThreads ==
      MAX_DEDICATED_COMMITTEE_SERVER_THREADS) {
      pthread_cond_wait(&syncTools.committeeWorkingThreadsCondition,
        &syncTools.mutex);
    }
    pthread_mutex_unlock(&syncTools.mutex);

    /* Spawn new committee-dedicated thread processing committee data. */
    if (pthread_create(&committeeWorkerThreads[committee],
      &syncTools.workerThreadAttribute, committeeWorkerThread,
      (void*) &committee) != 0)
      syserr(COMMITTEE_WORKER_THREAD_INITIALIZATION_ERROR_CODE);
    
    fprintf(stderr, "Nowy dedykowany wątek został zespawnowany dla komisji: %ld\n", committee);
  }

  return 0;
}

void freeResources(int signum) {
  freeServerIPCQueuesResources(&queueIds);
  destroyServerSyncTools(&syncTools);

  exit(signum);
}

int main(int argc, char** argv) {
  pthread_t reportDispatcher;
  pthread_t committeeDispatcher;
  
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
