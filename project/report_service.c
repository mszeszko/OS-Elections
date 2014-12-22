#include "report_service.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "constants.h"
#include "err.h"
#include "error_codes.h"
#include "message_structures.h"

void tryReportConnection(int* reportDataIPCQueueId) {
  if ((*reportDataIPCQueueId = msgget(REPORT_DATA_IPC_QUEUE_KEY, 0)) == -1)
    syserr(IPC_QUEUE_NOT_INITIALIZED_ERROR_CODE);
}

void sendGetReportMessageRequest(int IPCQueueId, int pid, int list) {
  getReportMessage reportMessage;
  const int getReportMessageSize = sizeof(getReportMessage) - sizeof(long);
 
  reportMessage.operationId = REPORT_REQUEST_MESSAGE_TYPE;
  reportMessage.pid = pid;
  reportMessage.reportList = (long) list;
  
  if (msgsnd(IPCQueueId, &reportMessage, getReportMessageSize, 0) != 0)
    syserr(IPC_QUEUE_SEND_OPERATION_ERROR_CODE);
}

void printReportHeader(int reportDataIPCQueueId, int pid) {
  reportHeaderMessage header;
  float turnoutPercentage;
  const int reportHeaderMessageSize =
    sizeof(reportHeaderMessage) - sizeof(long);

  if (msgrcv(reportDataIPCQueueId, &header, reportHeaderMessageSize,
    (long) pid, 0) != reportHeaderMessageSize)
    syserr(IPC_QUEUE_RECEIVE_OPERATION_ERROR_CODE);

  turnoutPercentage = 0;
  if (header.validVotes != 0) {
    turnoutPercentage =
      (((float)header.validVotes + (float)header.invalidVotes) /
      header.eligibledVoters) * 100;
  }

  fprintf(stderr, PROCESSED_COMMITTEES_TEMPLATE, header.processedCommittees,
    header.committees);
  fprintf(stderr, ELIGIBLED_VOTERS_TEMPLATE, header.eligibledVoters);
  fprintf(stderr, VALID_VOTES_TEMPLATE, header.validVotes);
  fprintf(stderr, INVALID_VOTES_TEMPLATE, header.invalidVotes);
  fprintf(stderr, TURNOUT_TEMPLATE, turnoutPercentage);
  fprintf(stderr, COMMITTEE_RESULTS_TEMPLATE);
}

void printSingleListReport(singleListReport* listReport) {
  if (listReport->position == FIRST) {
    /* List. */
    fprintf(stderr, "%d ", listReport->list);
    /* Votes. */
    fprintf(stderr, "%d ", listReport->votes);
  }
  /* Votes for particular candidate. */
  fprintf(stderr, "%d", listReport->candidateVotes);
  /* Format report line. */
  fprintf(stderr, (listReport->position == LAST) ? "\n" : " ");
}

void receiveAndPrintData(int reportDataIPCQueueId, int pid) {
  singleListReportMessage reportMessage;
  const int singleListReportMessageSize =
    sizeof(singleListReportMessage) - sizeof(long);
  
  printReportHeader(reportDataIPCQueueId, pid);

  /* As long as server did not confirm the end of the current report.. */
  while (1) {
    if (msgrcv(reportDataIPCQueueId, &reportMessage,
      singleListReportMessageSize, (long) pid, 0) !=
      singleListReportMessageSize)
      syserr(IPC_QUEUE_RECEIVE_OPERATION_ERROR_CODE);
    if (reportMessage.listReport.reportProgress == REPORT_COMPLETED)
      break;
    
    printSingleListReport(&reportMessage.listReport);
  }
}

