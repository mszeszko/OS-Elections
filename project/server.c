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
#include "server_service.h"
#include "server_structures.h"

sharedIPCQueueIds queueIds;
sharedDataStructures data;
sharedSynchronizationVariables syncVariables;
sharedSynchronizationTools syncTools;

int main(int argc, char** argv) {

  if (argc != 4) {
    printf(SERVER_USAGE_ERROR_CODE, argv[0]);
    exit(EXIT_FAILURE);
  }

  initializeServerSyncTools(&syncTools);
  initializeServerIPCQueues(&queueIds);
  
  freeServerIPCQueuesResources(&queueIds);
  destroyServerSyncTools(&syncTools);

  return EXIT_SUCCESS;
}
