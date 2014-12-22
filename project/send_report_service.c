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

void prepareAndSendStatisticsHeaderReport(sharedDataStructures* data,
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

void sendSingleReportMessage(singleListReport* report, long operationId,
  int IPCQueueId) {
  singleListReportMessage listReportMessage;
  const int singleListReportMessageSize =
    sizeof(singleListReportMessage) - sizeof(long);

  listReportMessage.operationId = operationId;
  listReportMessage.listReport = *report;
  
  if (msgsnd(IPCQueueId, (void*) &listReportMessage,
    singleListReportMessageSize, 0) != 0)
    syserr(IPC_QUEUE_SEND_OPERATION_ERROR_CODE);
}

void prepareAndSendSingleListReport(sharedDataStructures* data,
  unsigned int list, long operationId, int IPCQueueId) {
  int i;  
  singleListReport listReport;

  /* Initialization */
  listReport.list = list;
  listReport.candidates = data->candidatesPerList;

  for (i = 1; i<= data->candidatesPerList; ++i)
    listReport.candidateVotes[i] = data->electionResults[list][i];

  listReport.votes = data->summaryListVotes[list];
  listReport.reportProgress = REPORT_IN_PROGRESS;
  
  sendSingleReportMessage(&listReport, operationId, IPCQueueId);
}

void prepareAndSendReportFinishMessage(long operationId, int IPCQueueId) {
  singleListReport listReport;
  listReport.reportProgress = REPORT_COMPLETED;
  
  sendSingleReportMessage(&listReport, operationId, IPCQueueId);
}

void prepareAndSendCompleteReport(sharedDataStructures* data,
  unsigned int list, int IPCQueueId) {
  int i;
  long operationId = (long) list;
  prepareAndSendStatisticsHeaderReport(data, operationId, IPCQueueId);
  if (operationId == ALL_LISTS_ID) {
    for (i = 1; i<= data->lists; ++i)
      prepareAndSendSingleListReport(data, i, operationId, IPCQueueId);
  } else {
    prepareAndSendSingleListReport(data, list, operationId, IPCQueueId);
  }
  prepareAndSendReportFinishMessage(operationId, IPCQueueId);
}
