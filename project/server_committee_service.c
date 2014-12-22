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
  committeeWorkerResources* resources, int** partialResults, int list) {
  int i, j;
  
  const int rows = sharedData->lists;
  const int columns = sharedData->candidatesPerList;

  /* Initialize partial results array. */
  for(i = 1; i<= rows; ++i)
    for (j = 1; j<= columns; ++j)
      partialResults[i * rows + columns] = 0;
  
  /* Other.. */
  resources->eligibledVoters = 0;
  resources->totalVotes = 0;
  resources->validVotes = 0;
  resources->list = list; 
}

void initializeDynamicCommitteeStructure(int** partialResults, int rows,
  int columns) {
  int i;
  partialResults = (int**) malloc ((rows + 1) * sizeof(int*));
  for (i = 0; i<= rows; ++i)
    partialResults[i] = (int*) malloc ((columns + 1) * sizeof(int));
}

void freeDynamicCommitteeStructure(int** partialResults, int rows) {
  int i;
  if (partialResults == NULL)
    return;
  for (i = 0; i<= rows; ++i)
    free(partialResults[i]);
  free(partialResults);
  partialResults = NULL;
}

void freeCommitteeAllocatedStructures(
  sharedSynchronizationVariables* threads,
  sharedDataStructures* sharedData) {
  int i;
  const int rows = sharedData->lists;

  for (i = 0; i< MAX_DEDICATED_SERVER_THREADS; ++i)
    freeDynamicCommitteeStructure(threads->partialResults[i], rows);
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
  committeeWorkerResources* resources, int** partialResults) {
  int i, j;
  
  /* 1) election results: */
  for (i = 1; i<= sharedData->lists; ++i)
    for (j = 1; j<= sharedData->candidatesPerList; ++j)
      sharedData->electionResults[i][j] += partialResults[i][j];

  /* 2) summary list votes: */
  sharedData->summaryListVotes[resources->list] = resources->totalVotes; 
 
  /* 3) processed committees: */
  ++(sharedData->processedCommittees);

  /* 4) eligibled voters: */
  sharedData->eligibledVoters += resources->eligibledVoters;

  /* 5) valid votes: */
  sharedData->validVotes += resources->validVotes;

  /* 6) invalid votes: */
  sharedData->invalidVotes += resources->totalVotes - resources->validVotes;
}

void sendAckMessage(int IPCQueueId, long committee,
  int processedMessages, int validVotes) {
  serverAckMessage message;
  const int serverAckMessageSize = sizeof(serverAckMessage) - sizeof(long);
  
  message.operationId = committee;
  message.processedMessages = processedMessages;
  message.validVotes = validVotes;

  if (msgsnd(IPCQueueId, (void*) &message, serverAckMessageSize, 0) != 0)
    syserr(IPC_QUEUE_SEND_OPERATION_ERROR_CODE);
}
