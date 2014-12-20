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
#include "server_structures.h"

sharedIPCQueueIds queueIds;
sharedDataStructures data;
sharedSynchronizationVariables synchVariables;
sharedSynchronizationTools synchTools;

int main(int argc, char** argv) {

  if (argc != 4) {
    printf(REPORT_USAGE_ERROR_CODE, argv[0]);
    exit(EXIT_FAILURE);
  }

  return EXIT_SUCCESS;
}
