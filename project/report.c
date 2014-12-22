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

#include "constants.h"
#include "error_codes.h"
#include "message_structures.h"
#include "report_service.h"

int main(int argc, char** argv) {
  int list;

  int reportDataIPCQueueId;

  if (argc > 2) {
    printf(REPORT_USAGE_ERROR_CODE, argv[0]);
    exit(EXIT_FAILURE);
  }
 
  /* By default, non-parametrized `report` process expects results
     from all lists and it's represented by `ALL_LISTS_ID`. */
  list = (argc == 2) ? atoi(argv[1]) : ALL_LISTS_ID;

  /* We do allow to read reports in parellel for at most MAX_LISTS + 1 processes
     that represents groups of distinct lists demands. */
  getReportGroupAccessToken(list);

  /* We do have access to the token that represents reports for our `list`
     so request the data from the server. */
  tryReportConnection(&reportDataIPCQueueId);

  sendGetReportMessageRequest(reportDataIPCQueueId, list);

  receiveAndPrintData(reportDataIPCQueueId, list);

  return EXIT_SUCCESS;
}
