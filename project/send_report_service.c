/*
     Maciej Szeszko,
     id: ms335814,
     University of Warsaw
*/

#include "send_report_service.h"

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

void prepareAndSendSingleListReport(sharedDataStructures* data,
  unsigned int list, long operationId, int IPCQueueId) {
  
  singleListReport listReport;
  singleListReportMessage listReportMessage;
  const int singleListReportMessageSize =
    sizeof(singleListReportMessageSize) - sizeof(long);

  /* Initialization */
  listReport.list = list;
  listReport.candidates = data->candidates_per_list;
  listReport.candidateVotes = data->electionResults[list];
  listReport.votes = data->summaryListVotes[list];
  listReport.finish = NOT_FINISHED_YET;
  
  listReportMessage.operationId = operationId;
  listReportMessage.listReport = listReport;

  if (msgsnd(IPCQueueId, (void*) &listReportMessage,
    singleListReportMessageSize, 0) != 0)
    syserr(IPC_QUEUE_SEND_OPERATION_ERROR_CODE);
}

void prepareAndSendCompleteReport(sharedDataStructures* data,
  unsigned int list, int IPCQueueId) {
  int i;
  long operationId = (long) list;
  prepareAndSendStatisticsHeaderReport(data, operationId, IPCQueueId);
  if (list == 0) {
    for (i = 1; i<= data->lists; ++i)
      prepareAndSendSingleListReport(data, i, operationId, IPCQueueId);
  } else {
    prepareAndSendSingleListReport(data, list, operationId, IPCQueueId);
  }
}
