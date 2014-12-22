/*
     Maciej Szeszko,
     id: ms335814,
     University of Warsaw
*/

#include "server_service.h"

#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "constants.h"
#include "err.h"
#include "error_codes.h"
#include "message_structures.h"

/* Initialization. */
void initializeServerSyncTools(sharedSynchronizationTools* tools) {
  if (pthread_mutex_init(&tools->mutex, 0) != 0)
    syserr(MUTEX_INITIALIZATION_ERROR_CODE);
  if (pthread_cond_init(&tools->committeeUpdateResultsCondition, 0) != 0)
    syserr(COMMITTEES_CONDITION_INITIALIZATION_ERROR_CODE);
  if (pthread_cond_init(&tools->reportProcessResultsCondition, 0) != 0)
    syserr(REPORTS_CONDITION_INITIALIZATION_ERROR_CODE);
}

void initializeServerIPCQueues(sharedIPCQueueIds* queueIds) {
  if ((queueIds->initConnectionIPCQueueId =
    msgget(INIT_CONNECTION_IPC_QUEUE_KEY, 0666 | IPC_CREAT | IPC_EXCL)) == -1)
    syserr(IPC_QUEUE_INITIALIZATION_ERROR_CODE);
  if ((queueIds->committeeDataIPCQueueId =
    msgget(COMMITTEE_DATA_IPC_QUEUE_KEY, 0666 | IPC_CREAT | IPC_EXCL)) == -1)
    syserr(IPC_QUEUE_INITIALIZATION_ERROR_CODE);
  if ((queueIds->finishIPCQueueId =
    msgget(FINISH_IPC_QUEUE_KEY, 0666 | IPC_CREAT | IPC_EXCL)) == -1)
    syserr(IPC_QUEUE_INITIALIZATION_ERROR_CODE);
  if ((queueIds->reportGroupAccessTokenIPCQueueId =
    msgget(REPORT_GROUP_ACCESS_TOKEN_IPC_QUEUE_KEY, 0666 | IPC_CREAT |
    IPC_EXCL)) == -1)
    syserr(IPC_QUEUE_INITIALIZATION_ERROR_CODE);
  if ((queueIds->reportDataIPCQueueId =
    msgget(REPORT_DATA_IPC_QUEUE_KEY, 0666 | IPC_CREAT | IPC_EXCL)) == -1)
    syserr(IPC_QUEUE_INITIALIZATION_ERROR_CODE);
}

void initializeThreadAttribute(pthread_attr_t* threadAttribute,
  int detachState) {
  if (pthread_attr_init(threadAttribute) != 0)
    syserr(THREAD_ATTRIBUTE_INITIALIZATION_ERROR_CODE);
  if (pthread_attr_setdetachstate(threadAttribute, detachState) != 0)
    syserr(THREAD_ATTRIBUTE_DETACHSTATE_INITIALIZATION_ERROR_CODE);
}


/* Free resources & destroy. */
void freeServerIPCQueuesResources(sharedIPCQueueIds* queueIds) {
  if (msgctl(queueIds->initConnectionIPCQueueId, IPC_RMID, 0) == -1)
    syserr(IPC_QUEUE_REMOVE_OPERATION_ERROR_CODE);
  if (msgctl(queueIds->committeeDataIPCQueueId, IPC_RMID, 0) == -1)
    syserr(IPC_QUEUE_REMOVE_OPERATION_ERROR_CODE);
  if (msgctl(queueIds->finishIPCQueueId, IPC_RMID, 0) == -1)
    syserr(IPC_QUEUE_REMOVE_OPERATION_ERROR_CODE);
  if (msgctl(queueIds->reportGroupAccessTokenIPCQueueId, IPC_RMID, 0) == -1)
    syserr(IPC_QUEUE_REMOVE_OPERATION_ERROR_CODE);
  if (msgctl(queueIds->reportDataIPCQueueId, IPC_RMID, 0) == -1)
    syserr(IPC_QUEUE_REMOVE_OPERATION_ERROR_CODE);
}

void destroyServerSyncTools(sharedSynchronizationTools* tools) {
  if (pthread_mutex_destroy(&tools->mutex) != 0)
    syserr(MUTEX_DESTROY_ERROR_CODE);
  if (pthread_cond_destroy(&tools->committeeUpdateResultsCondition) != 0)
    syserr(COMMITTEES_CONDITION_DESTROY_ERROR_CODE);
  if (pthread_cond_destroy(&tools->reportProcessResultsCondition) != 0)
    syserr(REPORTS_CONDITION_DESTROY_ERROR_CODE);
}


/* General. */
void putSingleGroupAccessToken(int IPCQueueId, long token) {
  groupAccessTokenMessage tokenMessage;
  const int groupAccessTokenMessageSize =
    sizeof(groupAccessTokenMessage) - sizeof(long);

  tokenMessage.operationId = token;
  if (msgsnd(IPCQueueId, (void*) &tokenMessage,
    groupAccessTokenMessageSize, 0) != 0)
    syserr(IPC_QUEUE_SEND_OPERATION_ERROR_CODE);
}

void receiveReportRequestMessage(int IPCQueueId, int* list) {
  getReportMessage request;
  const int getReportMessageSize = sizeof(getReportMessage) - sizeof(long); 
  
  if (msgrcv(IPCQueueId, &request, getReportMessageSize,
    REPORT_REQUEST_MESSAGE_TYPE, 0) != getReportMessageSize)
    syserr(IPC_QUEUE_RECEIVE_OPERATION_ERROR_CODE);
  *list = request.reportList;
}

void putBackGroupAccessToken(int IPCQueueId, long list) {
  putSingleGroupAccessToken(IPCQueueId, list);
}

void initializeReportGroupAccessTokenIPCQueue(int IPCQueueId,
  int committees) {
  int i;
  for (i = 1; i<= committees; ++i)
    putSingleGroupAccessToken(IPCQueueId, i);
  putSingleGroupAccessToken(IPCQueueId, ALL_LISTS_ID);
}

