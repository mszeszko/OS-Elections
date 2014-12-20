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

void sendBasicCommitteeInfo(int committeeDataIPCQueueId, long committee,
  basicCommitteeInfo* localInfo, unsigned int list, unsigned int candidate,
  unsigned int candidateVotes) {

  committeeMessage message;
  const int committeeMessageSize = sizeof(committeeMessage) - sizeof(long);

  /* Initialize `committeeMessage`. */
  message.operationId = committee;
  message.list = list;
  message.candidate = candidate;
  message.candidateVotes = candidateVotes;
  
  if (msgsnd(committeeDataIPCQueueId, (void*) &message,
    committeeMessageSize, 0) != 0)
    syserr(IPC_QUEUE_SEND_OPERATION_ERROR_CODE);
}

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

  committee = (argc == 0) ? 0 : atoi(argv[1]);

  tryInitialConnection(committee);

  /* Read the data As we got `green light` on processing.. */
  scanf("%u %u", &eligibleVoters, &votes);
  
  /* Compose and send initial data message to committee-dedicated
     server thread. */
  sendBasicCommitteeInfo(&localInfo, elligibleVoters, votes, committee);

  /* Send ordinary chunks of data. */  
  while (scanf("%u %u %u", &list, &candidate, &candidateVotes) != EOF) {
    sendCommitteeMessage(committee, list, candidate, candidateVotes);
  }

  /* Send `finish` message to the thread and wait for ACK. */
}
