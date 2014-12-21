/*
     Maciej Szeszko,
     id: ms335814,
     University of Warsaw
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "constants.h"
#include "error_codes.h"
#include "message_structures.h"
#include "process_results_service.h"
#include "send_report_service.h"
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
  processResultsServiceInitialProtocol(&syncTools, &syncVariables);
  
  /* Process the data. */
  prepareAndSendCompleteReport(&sharedData, list, queueIds.reportDataIPCQueueId);

  /* Perform ending protocol. */
  processResultsServiceEndingProtocol(&syncTools, &syncVariables);

  /* After passing data for specific list or complete set of all lists
	   put back group access token of value `list` to appropriate IPC queue. */
  putBackGroupAccessToken(list, queueIds.reportGroupAccessTokenIPCQueueId); 
  exit(EXIT_SUCCESS);
}

void* reportDispatcherThread(void* data) {
  unsigned int list;

  pthread_t reportWorkerThreads[MAX_LISTS + 1];
  pthread_attr_t* workerThreadAttribute = (pthread_attr_t*) data;

  /* Initialize all report group access tokens in appropriate  IPC queue. */
  initializeReportGroupAccessTokenIPCQueue(
    queueIds.reportGroupAccessTokenIPCQueueId, sharedData.committees);

  while (1) {
    receiveReportRequestMessage(queueIds.reportDataIPCQueueId, &list);

    /* Spawn new report-dedicated thread to handle report sending. */
    if (pthread_create(&reportWorkerThreads[list], workerThreadAttribute,
      reportWorkerThread, (void*) &list) != 0)
      syserr(REPORT_WORKER_THREAD_INITIALIZATION_ERROR_CODE);
  }

  exit(EXIT_SUCCESS);
}

void* committeeWorkerThread(void* data) {
  exit(EXIT_SUCCESS);
}

void* committeeDispatcherThread(void* data) {
  unsigned int list;
  unsigned int list_available;
  
  initialConnectionMessage initMessage;
  const int initialConnectionMessageSize;

  pthread_t committeeWorkerThreads[MAX_COMMITTEES + 1];
  pthread_attr_t* workerThreadAttribute;

  initialConnectionMessageSize =
    sizeof(initialConnectionMessage) - sizeof(long);
  pthread_attr_t* worerThreadAttribute = (pthread_attr_t*) data;

  /* Handle committees connection requests. */
  while (1) {
    receiveConnectionRequest(queueIds.initConnectionIPCQueueId, &list);

    /* Make sure that list is a one of all possible committee values. */
    if (list >= 1 && list <= sharedData.committees) {
    	/* Check whether committee connection has been arranged yet or not.
    	   Connection available: 0, connection arranged yet: 1. */
    	pthread_mutex_lock(&syncTools->mutex);
    	list_available = sharedData->committeeConnectionPossible[list];
    	if (list_available == 0)
    	  sharedData->committeeConnectionPossible[list] = 1;
    	pthread_mutex_unlock(&syncTools->mutex);
    } else {
      list_available = 1;
    }

    /* Send response back to the specific committee. */
    initMessage.operationId = (long) list;
    initMessage.ack = (list_available == 0) ?
      CONNECTION_SUCCEEDED : CONNECTION_REFUSED_ERROR_CODE;
    if (msgsnd(queueIds.initConnectionIPCQueueId, (void*) &initMessage,
      initialConnectionMessageSize, 0) != 0)
      syserr(IPC_QUEUE_SEND_OPERATION_ERROR_CODE);

    /* In case of connection refused statement, stop further processing. */
    if (list_available == 1)
      continue;

    /* Wait to process committee data  until there will be less than maximal
       number of committee-dedicated working threads. */
    pthread_mutex_lock(&syncTools->mutex);
    while (sharedVariables->workingCommitteeThreads ==
      MAX_DEDICATED_COMMITEE_SERVER_THREADS) {
      pthread_cond_wait(&syncTools->committeeWorkingThreadsCondition,
        &syncTools->mutex);
    }
    pthread_mutex_unlock(&syncTools->mutex);

    /* Spawn new committee-dedicated thread processing committee data. */
    if (pthread_create(&reportWorkerThreads[list], workerThreadAttribute,
      committeeWorkerThread, (void*) &list) != 0)
      syserr(COMMITTEE_WORKER_THREAD_INITIALIZATION_ERROR_CODE);
  }

  exit(EXIT_SUCCESS);
}

int main(int argc, char** argv) {
  pthread_t reportDispatcher;
  pthread_t committeeDispatcher;

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

  /* Initialize server IPC queues and synchronization tools. */
  initializeServerSyncTools(&syncTools);
  initializeServerIPCQueues(&queueIds);

  /* Initialize distinct thread attributes. */
  initializeThreadAttribute(&syncTools.dispatcherThreadAttribute,
    dipspatcherThreadDetachState);
  initializeThreadAttribute(&syncTools.workerThreadAttribute,
    workerThreadDetachState);

  /* Create dispatcher threads. */
  if (pthread_create(&reportDispatcher, &threadAttribute,
    reportDispatcherThread, (void*) &syncTools.workerThreadAttribute) != 0)
    syserr(REPORT_DISPATCHER_THREAD_INITIALIZATION_ERROR_CODE);
  if (pthread_create(&committeeDispatcher, &threadAttribute,
    committeeDispatcherThread, (void*) &syncTools.workerThreadAttribute) != 0)
    syserr(COMMITTEE_DISPATCHER_THREAD_INITIALIZATION_ERROR_CODE);
  
  /* Wait for dispatcher threads. */
  if (pthread_join(reportDispatcherThread, 0) != 0)
    syserr(REPORT_DISPATCHER_THREAD_JOIN_ERROR_CODE);
  if (pthread_join(committeeDispatcherThread, 0) != 0)
    syserr(COMMITTEE_DISPATCHER_THREAD_JOIN_ERROR_CODE);
  
  freeServerIPCQueuesResources(&queueIds);
  destroyServerSyncTools(&syncTools);

  return EXIT_SUCCESS;
}
