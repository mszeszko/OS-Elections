/*
     Maciej Szeszko,
     id: ms335814,
     University of Warsaw
*/

#include "server_committee_service.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "constants.h"
#include "err.h"
#include "error_codes.h"
#include "message_structures.h"
#include "server_structures.h"

void initializeCommitteeWorkerResources(sharedDataStructures* sharedData,
  committeeWorkerResources* resources, unsigned int list) {
  int i, j;

  /* Initialize partial results array. */
  for(i = 0; i<= sharedData->lists; ++i)
    for (j = 0; j<= sharedData->candidatesPerList; ++j)
      resources->partialResults[i][j] = 0;
  
  /* Other.. */
  resources->eligibledVoters = 0;
  resources->totalVotes = 0;
  resources->validVotes = 0;
  resources->list = list; 
}

void receiveConnectionRequest(int IPCQueueId, long* committee) {
  initialConnectionMessage initMessage;
  const int initialConnectionMessageSize =
    sizeof(initialConnectionMessage) - sizeof(long);

  if (msgrcv(IPCQueueId, &initMessage, initialConnectionMessageSize,
    INIT_CONNECTION_MESSAGE_TYPE, 0) != initialConnectionMessageSize)
    syserr(IPC_QUEUE_RECEIVE_OPERATION_ERROR_CODE);

  *committee = initMessage.committee;
}

void receiveCommitteeMessage(int IPCQueueId, committeeMessage* message,
  long committee) {
  const int committeeMessageSize = sizeof(committeeMessage) - sizeof(long);
  if (msgrcv(IPCQueueId, message, committeeMessageSize, committee, 0)
    != committeeMessageSize)
    syserr(IPC_QUEUE_RECEIVE_OPERATION_ERROR_CODE);
}

void updateSharedData(sharedDataStructures* sharedData,
  sharedSynchronizationVariables* syncVariables,
  committeeWorkerResources* resources) {
  int i, j;
  
  /* 1) election results: */
  for (i = 1; i<= sharedData->lists; ++i)
    for (j = 1; j<= sharedData->candidatesPerList; ++j)
      sharedData->electionResults[i][j] += resources->partialResults[i][j];

  /* 2) summary list votes: */
  sharedData->summaryListVotes[resources->list] = resources->totalVotes; 
 
  /* 3) processed committees: */
  ++sharedData->processedCommittees;

  /* 4) eligibled voters: */
  sharedData->eligibledVoters += resources->eligibledVoters;

  /* 5) valid votes: */
  sharedData->validVotes += resources->validVotes;

  /* 6) invalid votes: */
  sharedData->invalidVotes += resources->totalVotes - resources->validVotes;

  /* 7) working committee threads: */
  --(syncVariables->workingCommitteeThreads);
}

void sendAckMessage(int IPCQueueId, long committee,
  unsigned int processedMessages, unsigned int validVotes) {
  serverAckMessage message;
  const int serverAckMessageSize = sizeof(serverAckMessage) - sizeof(long);
  
  fprintf(stderr, "Wysyłam ACKA\n");
  message.operationId = committee;
  message.processedMessages = processedMessages;
  message.validVotes = validVotes;

  if (msgsnd(IPCQueueId, (void*) &message, serverAckMessageSize, 0) != 0)
    syserr(IPC_QUEUE_SEND_OPERATION_ERROR_CODE);

  fprintf(stderr, "ACK został wysłany do komisji: %d!\n", message.operationId);
}
