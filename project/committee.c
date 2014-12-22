/*
     Maciej Szeszko,
     id: ms335814,
     University of Warsaw
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "committee_service.h"
#include "error_codes.h"
#include "message_structures.h"

int main(int argc, char** argv) {
  
  int eligibleVoters;
  int votes;
  
  long committee;
  
  int list;
  int candidate;
  int candidateVotes;

  basicCommitteeInfo localInfo;

  int committeeDataIPCQueueId;

  /* Validate `committee` argument list. */
  if (argc != 2) {
    printf(COMMITTEE_USAGE_ERROR_CODE, argv[0]);
    exit(EXIT_FAILURE);
  }
  
  committee = atoi(argv[1]);

  /* Set up connections if possible. */
  tryInitialConnection(committee);
  tryCommitteeDataQueueConnection(&committeeDataIPCQueueId, committee);

  /* Read the data as we got `green light` on processing.. */
  scanf("%d %d", &eligibleVoters, &votes);
  
  /* Compose and send initial data message to committee-dedicated
     server thread. */
  prepareAndSendBasicCommitteeInfo(committeeDataIPCQueueId, committee,
    &localInfo, eligibleVoters, votes);

  /* Send ordinary chunks of data. */  
  while (scanf("%d %d %d", &list, &candidate, &candidateVotes) != EOF) {
    prepareAndSendCommitteeMessage(committeeDataIPCQueueId, committee, list,
      candidate, candidateVotes);
  }

  /* Send `finish` message to the thread and wait for ACK. */
  prepareAndSendFinishMessage(committeeDataIPCQueueId, committee);
  
  waitForServerResponseAndPrintResults(committee, &localInfo);

  return EXIT_SUCCESS;
}
