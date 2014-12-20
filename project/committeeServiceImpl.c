/*
     Maciej Szeszko,
     id: ms335814,
     University of Warsaw
*/

#include "committeeService.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>

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
  if (initialConnectionMessage.ack == CONNECTION_REFUSED_ERROR_CODE) {
    printf(ACCESS_DENIED_ERROR_CODE);
    exit(EXIT_FAILURE);
  }
}

void sendCommitteeMessage(int IPCQueueId, committeeMessage* message) {
  const int committeeMessageSize = sizeof(committeeMessage) - sizeof(long);
  if (msgsnd(IPCQueueId, (void*) message, committeeMessageSize, 0) != 0) {
    printf(IPC_QUEUE_SEND_OPERATION_ERROR_CODE);
    exit(EXIT_FAILURE);
  }
}

void prepareAndSendBasicCommitteeInfo(int committeeDataIPCQueueId,
  long committee, basicCommitteeInfo* localInfo,
  unsigned int eligibleVoters, unsigned int vote) {
 
  committeeMessage message;
  const int committeeMessageSize = sizeof(committeeMessage) - sizeof(long);

  /* Initialization. */
  message.operationId = committee;
  message.locationInfo = *locationInfo;

  sendCommitteeMessage(committeeDataIPCQueueId, &message);
}

void prepareAndSendCommitteeMessage(int committeeDataIPCQueueId,
  long committee, unsigned int list, unsigned int candidate,
  unsigned int candidateVotes) {

  committeeMessage message;
  const int committeeMessageSize = sizeof(committeeMessage) - sizeof(long);

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
  const int committeeMessageSize = sizeof(committeeMessage) - sizeof(long);

  /* Initialization */
  message.operationId = committee;
  message.finished = FINISHED_DATA_PROCESSING;

  sendCommitteeMessage(committeeDataIPCQueueId, &message);
}

void printResults(committeeBasicInfo* localInfo,
  unsigned int processedMessage, unsigned int validVotes) {

  float turnout =
    ((float)localInfo->eligibledVoters * 100) / localInfo->totalVotes;

  printf("Prztworzonych wpisów: %u\n", processedMessages);
  printf("Uprawnionych do głosowania: %u\n", localInfo->eligibledVoters);
  printf("Głosów ważnych: %u\n", validVotes);
  printf("Głosów nieważnych: %u\n", localInfo->totalVotes - validVotes);
  printf("Frekwencja w lokalu: %.2f\n", turnout);
}

void waitForServerResponseAndPrintResults(long committee,
  committeeBasicInfo* localInfo) {
  
  int finishIPCQueueId;
  serverAckMessage serverResponse;
  const int serverResponseSize = sizeof(serverAckMessage) - sizeof(long);


  /* Try fetching finish IPC queue id. */
  if ((finishIPCQueueId = msgget(INIT_CONNECTION_IPC_QUEUE_KEY, 0)) == -1)
    syserr(IPC_QUEUE_NOT_INITIALIZED_ERROR_CODE);

  /* Read response package from server. */
  if (msgrcv(finishIPCQueueId, &serverResponse, serverResponseSize,
    committee, 0) != serverResponseSize)
    syserr(IPC_QUEUE_RECEIVE_OPERATION_ERROR_CODE);

  printResults(localInfo, serverResponse.processedMessages,
    serverResponse.validVotes);	
}
