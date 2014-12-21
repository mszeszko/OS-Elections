/*
     Maciej Szeszko,
     id: ms335814,
     University of Warsaw
*/

#include "server_config_service.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "constants.h"
#include "err.h"
#include "error_codes.h"
#include "message_structures.h"

void initializeThreadAttribute(pthread_attr_t* threadAttribute,
  int detachState) {
  if (pthread_attr_init(threadAttribute) != 0)
    syserr(THREAD_ATTRIBUTE_INITIALIZATION_ERROR_CODE);
  if (pthread_attr_setdetachstate(threadAttribute, detachState) != 0)
    syserr(THREAD_ATTRIBUTE_DETACHSTATE_INITIALIZATION_ERROR_CODE);
}

void putSingleGroupAccessToken(int IPCQueueId, unsigned int token) {
  groupAccessTokenMessage tokenMessage;
  const int groupAccessTokenMessageSize =
    sizeof(groupAccessTokenMessage) - sizeof(long);

  tokenMessage.operationId = token;
  if (msgsnd(IPCQueueId, (void*) &tokenMessage,
    groupAccessTokenMessageSize, 0) != 0)
    syserr(IPC_QUEUE_SEND_OPERATION_ERROR_CODE);
}

void initializeReportGroupAccessTokenIPCQueue(int IPCQueueId,
  unsigned int committees) {
  int i;
  for (i = 1; i<= committees; ++i)
    putSingleGroupAccessToken(IPCQueueId, i);
}

void receiveReportRequestMessage(int IPCQueueId, unsigned int* list) {
  getReportMessage request;
  const int getReportMessageSize = sizeof(getReportMessage) - sizeof(long); 

  if (msgrcv(IPCQueueId, &request, getReportMessageSize,
    REPORT_REQUEST_MESSAGE_TYPE, 0) != getReportMessageSize)
    syserr(IPC_QUEUE_RECEIVE_OPERATION_ERROR_CODE);

  *list = request.reportList;
}

void putBackGroupAccessToken(int IPCQueueId, unsigned int list) {
  groupAccessTokenMessage tokenMessage;
  putSingleGroupAccessToken(IPCQueueId, list);
}
