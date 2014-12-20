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

#include "constants"
#include "err.h"
#include "error_codes.h"

void initializeServerSyncTools(sharedSynchronizationTools* tools) {
  int error;
  if ((error = pthread_mutex_init(&tools->mutex, 0)) != 0)
    syserr(error, MUTEX_INITIALIZATION_ERROR_CODE);
  if ((error = pthread_cond_init(&tools->committeeUpdateResults, 0)) != 0)
    syserr(error, COMMITTEES_CONDITION_INITIALIZATION_ERROR_CODE);
  if ((error = pthread_cond_init(&tools->reportProcessResults, 0)) != 0)
    syserr(error, REPORTS_CONDITION_INITIALIZATION_ERROR_CODE);
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
  int error;
  if ((error = pthread_mutex_destroy(&tools->mutex)) != 0)
    syserr(error, MUTEX_DESTROY_ERROR_CODE);
  if ((error = pthread_cond_destroy(&tools->committeeUpdateResults)) != 0)
    syserr(error, COMMITTEES_CONDITION_DESTROY_ERROR_CODE);
  if ((error = pthread_cond_destroy(&tools->reportProcessResults)) != 0)
    syserr(error, REPORTS_CONDITION_DESTROY_ERROR_CODE);
}
