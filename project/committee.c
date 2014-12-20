/*
     Maciej Szeszko,
     id: ms335814,
     University of Warsaw
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "error_codes.h"
#include "message_structures.h"

int main(int argc, char** argv) {
  /* Validate `committee` argument list. */
  if (argc != 1) {
    printf(COMMITTEE_USAGE_ERROR_CODE, argv[0]);
    exit(EXIT_FAILURE);
  }
  
  unsigned int eligibleVoters;
  unsigned int votes;
  
  long committee;
  
  unsigned int list;
  unsigned int candidate;
  unsigned int candidateVotes;

  basicCommitteeInfo localInfo;

  int committeeDataIPCQueueId;

  committee = (argc == 0) ? 0 : atoi(argv[1]);

  /* Set up connections if possible. */
  tryInitialConnection(committee);
  tryCommitteeDataQueueConnection(&committeeDataIPCQueueId, committee);

  /* Read the data as we got `green light` on processing.. */
  scanf("%u %u", &eligibleVoters, &votes);
  
  /* Compose and send initial data message to committee-dedicated
     server thread. */
  prepareAndSendBasicCommitteeInfo(committeeDataIPCQueueId, &localInfo,
    elligibleVoters, votes, committee);

  /* Send ordinary chunks of data. */  
  while (scanf("%u %u %u", &list, &candidate, &candidateVotes) != EOF) {
    prepareAndSendCommitteeMessage(committeeDataIPCQueueId, committee, list,
      candidate, candidateVotes);
  }

  /* Send `finish` message to the thread and wait for ACK. */
  prepareAndSendFinishMessage(committeeDataIPCQueueId, committee);
  
  waitForServerResponseAndPrintResults(committeeDataIPCQueueId, committee,
    &localInfo);
}
