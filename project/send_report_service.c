/*
     Maciej Szeszko,
     id: ms335814,
     University of Warsaw
*/

#include "send_report_service.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "constants.h"
#include "err.h"
#include "error_codes.h"
#include "message_structures.h"

void prepareAndSendStatisticsHeaderReport(const sharedDataStructures* data,
  long operationId, int IPCQueueId) {
  
  reportHeaderMessage headerMessage;
  const int reportHeaderMessageSize =
    sizeof(reportHeaderMessage) - sizeof(long);

  /* Initialization. */
  headerMessage.operationId = operationId;
  headerMessage.processedCommittees = data->processedCommittees;
  headerMessage.committees = data->committees;
  headerMessage.validVotes = data->validVotes;
  headerMessage.invalidVotes = data->invalidVotes;
  headerMessage.eligibledVoters = data->eligibledVoters;

  /* Send report header message. */
  if (msgsnd(IPCQueueId, (void*) &headerMessage,
    reportHeaderMessageSize, 0) != 0)
    syserr(IPC_QUEUE_SEND_OPERATION_ERROR_CODE);
}

void sendSingleReportMessage(const singleListReport* report,
  long operationId, int IPCQueueId) {
  singleListReportMessage listReportMessage;
  const int singleListReportMessageSize =
    sizeof(singleListReportMessage) - sizeof(long);

  listReportMessage.operationId = operationId;
  listReportMessage.listReport = *report;
  
  if (msgsnd(IPCQueueId, (void*) &listReportMessage,
    singleListReportMessageSize, 0) != 0)
    syserr(IPC_QUEUE_SEND_OPERATION_ERROR_CODE);
}

void prepareAndSendSingleListReport(const sharedDataStructures* data,
  int list, int candidate, long operationId, int IPCQueueId) {
  singleListReport listReport;

  /* Initialization */
  listReport.list = list;
  listReport.candidateVotes = data->electionResults[list][candidate];

  listReport.votes = data->summaryListVotes[list];
  listReport.reportProgress = REPORT_IN_PROGRESS;
  
  /* Set candidate position. */
  if (candidate == 1)
    listReport.position = FIRST;
  else if (candidate == data->candidatesPerList)
    listReport.position = LAST;
  else
    listReport.position = MIDDLE;

  sendSingleReportMessage(&listReport, operationId, IPCQueueId);
}

void prepareAndSendReportFinishMessage(long operationId, int IPCQueueId) {
  singleListReport listReport;
  listReport.reportProgress = REPORT_COMPLETED;
  
  sendSingleReportMessage(&listReport, operationId, IPCQueueId);
}

void prepareAndSendCompleteReport(const sharedDataStructures* data,
  int pid, int list, int IPCQueueId) {
  int i, j;
  long operationId = (long) pid;
  prepareAndSendStatisticsHeaderReport(data, operationId, IPCQueueId);
  if (list == ALL_LISTS_ID) {
    for (i = 1; i<= data->lists; ++i)
      for (j = 1; j<= data->candidatesPerList; ++j)
        prepareAndSendSingleListReport(data, i, j, operationId, IPCQueueId);
  } else {
    for (j = 1; j<= data->candidatesPerList; ++j)
      prepareAndSendSingleListReport(data, list, j, operationId, IPCQueueId);
  }
  prepareAndSendReportFinishMessage(operationId, IPCQueueId);
}
