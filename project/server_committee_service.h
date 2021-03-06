/*
     Maciej Szeszko,
     id: ms335814,
     University of Warsaw
*/

#ifndef __SERVER_COMMITTEE_SERVICE_H_
#define __SERVER_COMMITTEE_SERVICE_H_

#include "message_structures.h"
#include "server_structures.h"

extern void initializeCommitteeWorkerResources(
  sharedDataStructures* sharedData, committeeWorkerResources* resources,
  int** partialResults, int list);

extern int** initializeDynamicCommitteeStructure(int rows, int columns);

extern void freeDynamicCommitteeStructure(int** partialResults, int rows);

extern void freeCommitteeAllocatedStructures(
  sharedSynchronizationVariables* threads, sharedDataStructures* sharedData);

extern void receiveConnectionRequest(int IPCQueueId, long* committee);

extern void receiveCommitteeMessage(int IPCQueueId, committeeMessage* message,
  long committee);

extern void sendAckMessage(int IPCQueueId, long committee,
  int processedMessages, int validVotes);

extern void updateSharedData(sharedDataStructures* sharedData,
  sharedSynchronizationVariables* syncVariables,
  committeeWorkerResources* resources, int** partialResults);

#endif
