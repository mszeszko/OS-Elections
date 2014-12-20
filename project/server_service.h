/*
     Maciej Szeszko,
     id: ms335814,
     University of Warsaw
*/

#ifndef __SERVER_SERVICE_H_
#define __SERVER_SERVICE_H_

#include "server_structures.h"

/* Initialization */
export void initializeServerSyncTools(sharedSynchronizationTools* tools);

export void initializeServerIPCQueues(sharedIPCQueueIds* queueIds);

/* Free resources. */
export void freeServerIPCQueuesResources(sharedIPCQueueIds* queueIds);

export void destroyServerSyncTools(sharedSynchronizationTools* tools);


#endif
