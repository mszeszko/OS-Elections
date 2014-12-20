/*
     Maciej Szeszko,
     id: ms335814,
     University of Warsaw
*/

#ifndef __COMMITTEE_SERVICE_H_
#define __COMMITTEE_SERVICE_H_

#include "message_structures.h"

extern void prepareConnectionMessage(initialConnectionMessage* message,
  long committee);

extern void tryInitialConnection(long committee);

extern void tryCommitteeDataQueueConnection(int* committeeDataIPCQueueId,
  long committee);

extern void sendCommitteeMessage(int IPCQueueId,
  committeeMessage* message);

extern void prepareAndSendBasicCommitteeInfo(int committeeDataIPCQueueId,
  long committee, basicCommitteeInfo* localInfo,
  unsigned int eligibleVoters, unsigned int vote);

extern void prepareAndSendCommitteeMessage(int committeeDataIPCQueueId,
  long committee, unsigned int list, unsigned int candidate,
  unsigned int candidateVotes);

extern void prepareAndSendFinishMessage(int committeeDataIPCQueueId,
  long committee);

extern void printResults(basicCommitteeInfo* localInfo,
  unsigned int processedMessage, unsigned int validVotes);

extern void waitForServerResponseAndPrintResults(long committee,
  basicCommitteeInfo* localInfo);

#endif
