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
