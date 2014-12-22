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
  int eligibleVoters, int vote);

extern void prepareAndSendCommitteeMessage(int committeeDataIPCQueueId,
  long committee, int list, int candidate,
  int candidateVotes);

extern void prepareAndSendFinishMessage(int committeeDataIPCQueueId,
  long committee);

extern void printResults(basicCommitteeInfo* localInfo,
  int processedMessage, int validVotes);

extern void waitForServerResponseAndPrintResults(long committee,
  basicCommitteeInfo* localInfo);

#endif
