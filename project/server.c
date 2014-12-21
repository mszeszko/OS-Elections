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

void initializeCommitteeWorkerResources(committeeWorkerResources* resources,
  unsigned int list) {
  int i, j;

  /* Initialize partial results array. */
  for(i = 0; i<= sharedData.lists; ++i)
    for (j = 0; j<= sharedData.candidates_per_list; ++j)
      resources->partialResults[i][j] = 0;
  
  /* Other.. */
  resources->eligibledVoters = 0;
  resources->totalVoters = 0;
  resources->validVotes = 0;
  resources->list = list; 
}

void receiveCommitteeMessage(int IPCQueueId, committeeMessage* message,
  unsigned int list) {
  const int committeeMessageSize = sizeof(committeeMessage) - sizeof(long);
  if (msgrcv(IPCQueueId, &message, committeeMessageSize, list, 0)
    != committeeMessageSize)
    syserr(IPC_QUEUE_RECEIVE_OPERATION_ERROR_CODE);
}

void* committeeWorkerThread(void* data) {
  committeeMessage message;
  committeeWorkerResources resources;
  
  int i, j;

  /* Initialize resources. */
  unsigned int transferFinished = 0;
  unsigned int committee = *((unsigned int*) data);
  initializeCommitteeWorkerResources(&resources, list);

  /* Process committee data. */
  while (!transferFinished) {
    receiveCommitteeMessage(queueIds.committeeDataIPCQueueId, &message,
      committee);
    
    switch (message.type) {
      case HEADER:
        resources.eligibledVoters = message.eligibledVoters;
        resources.totalVotes = message.totalVotes;
        break;
      case DATA:
        resources.partialResults[message.list][message.candidate] =
          message.candidateVotes;
        resources.validVotes += message.candidateVotes;
        break;
      case FINISH:
        transferFinished = 1;
        break;
    }
  }

  /* Wait for exclusive access to update shared data. */
  updateResultsServiceInitialProtocol(&syncTools, &syncVariables);

  /* Update shared data. */
  
  /* 1) election results: */
  for (i = 1; i<= sharedData.lists; ++i)
    for (j = 1; j<= sharedData.candidatesPerList; ++j)
      sharedData.electionResults[i][j] += resources.partialResults[i][j];

  /* 2) summary list votes: */
  sharedData.summaryListVotes = resources.totalVotes; 
 
  /* 3) processed committees: */
  ++sharedData.processedCommittees;

  /* 4) eligibled voters: */
  sharedData.eligibledVoters += resources.eligibledVoters;

  /* 5) valid votes: */
  sharedData.validVotes += resources.validVotes;

  /* 6) invalid votes: */
  sharedData.invalidVotes += resources.totalVotes - resources.validVotes;

  /* 7) working committee threads: */
  --syncVariables.workingCommitteeThreads;

  /* Release mutex. */
  updateResultsServiceEndingProtocol(&syncTools, &syncVariables);
  
  // Remember to wake up some thread!
  exit(EXIT_SUCCESS);
}

void* committeeDispatcherThread(void* data) {
  unsigned int committee;
  unsigned int committee_available;
  
  initialConnectionMessage initMessage;
  const int initialConnectionMessageSize;

  pthread_t committeeWorkerThreads[MAX_COMMITTEES + 1];
  pthread_attr_t* workerThreadAttribute;

  initialConnectionMessageSize =
    sizeof(initialConnectionMessage) - sizeof(long);
  pthread_attr_t* workerThreadAttribute = (pthread_attr_t*) data;

  /* Handle committees connection requests. */
  while (1) {
    receiveConnectionRequest(queueIds.initConnectionIPCQueueId, &committee);

    /* Make sure that list is a one of all possible committee values. */
    if (committee >= 1 && committee <= sharedData.committees) {
    	/* Check whether committee connection has been arranged yet or not.
    	   Connection available: 0, connection arranged yet: 1. */
    	pthread_mutex_lock(&syncTools->mutex);
    	committee_available = sharedData->committeeConnectionPossible[list];
    	if (committee_available == 0)
    	  sharedData->committeeConnectionPossible[committee] = 1;
    	pthread_mutex_unlock(&syncTools->mutex);
    } else {
      committee_available = 1;
    }

    /* Send response back to the specific committee. */
    initMessage.operationId = (long) committee;
    initMessage.ack = (committee_available == 0) ?
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
      committeeWorkerThread, (void*) &committee) != 0)
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
