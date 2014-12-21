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

  /* Initialize report group access token IPC queue. */
  initializeReportGroupAccessTokenIPCQueue(
    queueIds.reportGroupAccessTokenIPCQueueId, sharedData.committees);

  while (1) {
    receiveReportRequestMessage(queueIds.reportDataIPCQueueId, &list);

    /* Spawn new report-dedicated thread to handle report sending. */
    /* createReportWorkerThread(&reportWorkerThreads[list], list); */
  }

  exit(EXIT_SUCCESS);
}

void* committeeWorkerThread(void* data) {
  exit(EXIT_SUCCESS);
}

int main(int argc, char** argv) {
  
  pthread_t reportDispatcher;
  pthread_attr_t threadAttribute;
  int detachState;

  if (argc != 4) {
    printf(SERVER_USAGE_ERROR_CODE, argv[0]);
    exit(EXIT_FAILURE);
  }
  
  /* Initialize server IPC queues and synchronization tools. */
  initializeServerSyncTools(&syncTools);
  initializeServerIPCQueues(&queueIds);

  /* Initialize common threads attribute. */
  initializeThreadAttribute(&threadAttribute, detachState);

  /* Initialize report dispatcher thread. */
  /* createReportDispatcherThread(&reportDispatcher, &threadAttribute); */

  /* Handle committees connection requests. */
  while (1) {
    
  }
  
  freeServerIPCQueuesResources(&queueIds);
  destroyServerSyncTools(&syncTools);

  return EXIT_SUCCESS;
}
