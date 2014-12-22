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
  unsigned int list);

extern void receiveConnectionRequest(int IPCQueueId, unsigned int* committee);

extern void receiveCommitteeMessage(int IPCQueueId, committeeMessage* message,
  unsigned int committee);

extern void sendAckMessage(int IPCQueueId, unsigned int commitee,
  unsigned int processedMessages, unsigned int validVotes);

extern void updateSharedData(sharedDataStructures* sharedData,
  sharedSynchronizationVariables* syncVariables,
  committeeWorkerResources* resources);

#endif
