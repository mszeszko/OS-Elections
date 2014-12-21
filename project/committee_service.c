/*
     Maciej Szeszko,
     id: ms335814,
     University of Warsaw
*/

#include "committee_service.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "constants.h"
#include "err.h"
#include "error_codes.h"
#include "message_structures.h"

void prepareConnectionMessage(initialConnectionMessage* message,
  long committee) {
  message->operationId = committee;  
}

void tryInitialConnection(long committee) {

  initialConnectionMessage connectionMessage;
  int initConnectionIPCQueueId;
  const int initialConnectionMessageSize =
    sizeof(initialConnectionMessage) - sizeof(long);

  /* Try fetching connection IPC queue id. */
  if ((initConnectionIPCQueueId =
    msgget(INIT_CONNECTION_IPC_QUEUE_KEY, 0)) == -1)
    syserr(IPC_QUEUE_NOT_INITIALIZED_ERROR_CODE);
  
  /* Try connecting to the server. */
  if (msgsnd(initConnectionIPCQueueId, (void*) &connectionMessage,
    initialConnectionMessageSize, 0) != 0)
    syserr(IPC_QUEUE_SEND_OPERATION_ERROR_CODE);
    
  /* Wait for servers' response, because maybe there is engaged the maximal
     number of committee-dedicated server threads. */
  if (msgrcv(initConnectionIPCQueueId, &connectionMessage,
    initialConnectionMessageSize, committee, 0) !=
    initialConnectionMessageSize)
    syserr(IPC_QUEUE_RECEIVE_OPERATION_ERROR_CODE);

  /* Examine server response. */
  if (connectionMessage.ack == CONNECTION_REFUSED_ERROR_CODE)
    syserr(ACCESS_DENIED_ERROR_CODE);
}

void tryCommitteeDataQueueConnection(int* committeeDataIPCQueueId,
  long committee) {

  /* Try fetching connection IPC queue id. */
  if ((*committeeDataIPCQueueId =
    msgget(COMMITTEE_DATA_IPC_QUEUE_KEY, 0)) == -1)
    syserr(IPC_QUEUE_NOT_INITIALIZED_ERROR_CODE);
}

void sendCommitteeMessage(int IPCQueueId, committeeMessage* message) {
  const int committeeMessageSize = sizeof(committeeMessage) - sizeof(long);
  if (msgsnd(IPCQueueId, (void*) message, committeeMessageSize, 0) != 0)
    syserr(IPC_QUEUE_SEND_OPERATION_ERROR_CODE);
}

void prepareAndSendBasicCommitteeInfo(int IPCQueueId,
  long committee, basicCommitteeInfo* localInfo, unsigned int eligibledVoters,
  unsigned int totoalVotes) {
 
  committeeMessage message;

  /* Initialization. */
  localInfo->eligibledVoters = eligibledVoters;
  localInfo->totalVotes = totalVotes;
  message.operationId = committee;
  message.localInfo = *localInfo;

  sendCommitteeMessage(committeeDataIPCQueueId, &message);
}

void prepareAndSendCommitteeMessage(int committeeDataIPCQueueId,
  long committee, unsigned int list, unsigned int candidate,
  unsigned int candidateVotes) {

  committeeMessage message;

  /* Initialization. */
  message.operationId = committee;
  message.list = list;
  message.candidate = candidate;
  message.candidateVotes = candidateVotes;
  
  sendCommitteeMessage(committeeDataIPCQueueId, &message);
}

void prepareAndSendFinishMessage(int committeeDataIPCQueueId,
  long committee) {
  
  committeeMessage message;

  /* Initialization */
  message.operationId = committee;
  message.finished = FINISHED_DATA_PROCESSING;

  sendCommitteeMessage(committeeDataIPCQueueId, &message);
}

void printResults(basicCommitteeInfo* localInfo,
  unsigned int processedMessages, unsigned int validVotes) {

  float turnoutPercentage =
    ((float)localInfo->eligibledVoters * 100) / localInfo->totalVotes;

  printf(PROCESSED_MESSAGES_TEMPLATE, processedMessages);
  printf(ELIGIBLED_VOTERS_TEMPLATE, localInfo->eligibledVoters);
  printf(VALID_VOTES_TEMPLATE, validVotes);
  printf(INVALID_VOTES_TEMPLATE, localInfo->totalVotes - validVotes);
  printf(TURNOUT_TEMPLATE, turnoutPercentage);
}

void waitForServerResponseAndPrintResults(long committee,
  basicCommitteeInfo* localInfo) {
  
  int finishIPCQueueId;
  serverAckMessage serverResponse;
  const int serverResponseSize = sizeof(serverAckMessage) - sizeof(long);


  /* Try fetching finish IPC queue id. */
  if ((finishIPCQueueId = msgget(FINISH_IPC_QUEUE_KEY, 0)) == -1)
    syserr(IPC_QUEUE_NOT_INITIALIZED_ERROR_CODE);

  /* Read response package from server. */
  if (msgrcv(finishIPCQueueId, &serverResponse, serverResponseSize,
    committee, 0) != serverResponseSize)
    syserr(IPC_QUEUE_RECEIVE_OPERATION_ERROR_CODE);

  printResults(localInfo, serverResponse.processedMessages,
    serverResponse.validVotes);	
}
