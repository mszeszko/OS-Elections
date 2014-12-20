#include "report_service.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "constants.h"
#include "err.h"
#include "error_codes.h"
#include "message_structures.h"

void getReportGroupAccessToken(unsigned int list) {
  int reportGroupAccessTokenIPCQueueId;

 	getAccessTokenMessage accessTokenMessage;
  const int getAccessTokenMessageSize =
    sizeof(getAccessTokenMessage) - sizeof(long);

  if ((reportGroupAccessTokenIPCQueueId =
    msgget(REPORT_GROUP_ACCESS_TOKEN_IPC_QUEUE_KEY, 0)) == -1)
    syserr(IPC_QUEUE_NOT_INITIALIZED_ERROR_CODE);
 
  /* Wait for reading access token.
     If `list` = 0 then we wait for complexed report for all lists. */
  if (msgrcv(reportGroupAccessTokenIPCQueueId, &accessTokenMessage,
    getAccessTokenMessageSize, (long)list, 0) != getAccessTokenMessageSize)
    syserr(IPC_QUEUE_RECEIVE_OPERATION_ERROR_CODE);
}

void tryReportConnection(int* reportDataIPCQueueId) {
  if ((*reportDataIPCQueueId = msgget(REPORT_DATA_IPC_QUEUE_KEY, 0)) == -1)
    syserr(IPC_QUEUE_NOT_INITIALIZED_ERROR_CODE);
}

void printReportHeader(int reportDataIPCQueueId, unsigned int list) {
  
  reportHeaderMessage header;
  float turnoutPercentage;
  const int reportHeaderMessageSize =
    sizeof(reportHeaderMessage) - sizeof(long);

  if (msgrcv(reportDataIPCQueueId, &header, reportHeaderMessageSize,
    (long)list, 0) != reportHeaderMessageSize)
    syserr(IPC_QUEUE_RECEIVE_OPERATION_ERROR_CODE);

  turnoutPercentage = (((float)header.validVotes + (float)header.invalidVotes) 
    / header.eligibledVoters) * 100;

  printf(PROCESSED_COMMITTEES_TEMPLATE, header.processedCommittees,
    header.committees);
  printf(ELIGIBLED_VOTERS_TEMPLATE, header.eligibledVoters);
  printf(VALID_VOTES_TEMPLATE, header.validVotes);
  printf(INVALID_VOTES_TEMPLATE, header.invalidVotes);
  printf(TURNOUT_TEMPLATE, turnoutPercentage);
  printf(COMMITTEE_RESULTS_TEMPLATE);
}

void printSingleListReport(singleListReport* listReport) {
  int i;

  /* List. */
  printf("%u ", listReport->list);
  
  /* Votes. */
  printf("%u ", listReport->votes);

  /* Votes for particular candidate. */
  for (i = 1; i<= listReport->candidates; ++i)
    printf("%u ", listReport->candidateVotes[i]);
}

void receiveAndPrintData(int reportDataIPCQueueId, unsigned int list) {
  
  singleListReportMessage reportMessage;
  const int singleListReportMessageSize =
    sizeof(singleListReportMessage) - sizeof(long);
  
  printReportHeader(reportDataIPCQueueId, list);

  /* As long as server did not confirm the end of the current report.. */
  while (1) {
    if (msgrcv(reportDataIPCQueueId, &reportMessage,
      singleListReportMessageSize, (long)list, 0) !=
      singleListReportMessageSize)
      syserr(IPC_QUEUE_RECEIVE_OPERATION_ERROR_CODE);

    if (reportMessage.listReport.finish == REPORT_FINISHED)
      break;
    
    printSingleListReport(&reportMessage.listReport);
  }

  /* Access token is automatically restored by report-dedicated server thread
     after sending all data requested by current `report` process. */
}
