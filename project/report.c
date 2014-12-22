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
#include <unistd.h>

#include "constants.h"
#include "error_codes.h"
#include "message_structures.h"
#include "report_service.h"

int main(int argc, char** argv) {
  int list;
  int reportDataIPCQueueId;
  pid_t pid;

  if (argc > 2) {
    printf(REPORT_USAGE_ERROR_CODE, argv[0]);
    exit(EXIT_FAILURE);
  }
 
  pid = getpid();
  /* By default, non-parametrized `report` process expects results
     from all lists and it's represented by `ALL_LISTS_ID`. */
  list = (argc == 2) ? atoi(argv[1]) : ALL_LISTS_ID;

  tryReportConnection(&reportDataIPCQueueId);

  sendGetReportMessageRequest(reportDataIPCQueueId, pid, list);

  receiveAndPrintData(reportDataIPCQueueId, pid);

  return EXIT_SUCCESS;
}
