/*
     Maciej Szeszko,
     id: ms335814,
     University of Warsaw
*/

#ifndef __SERVER_SERVICE_H_
#define __SERVER_SERVICE_H_

#include "server_structures.h"

/* Initialization */
extern void initializeServerSyncTools(sharedSynchronizationTools* tools);

extern void initializeServerIPCQueues(sharedIPCQueueIds* queueIds);

/* Free resources. */
extern void freeServerIPCQueuesResources(sharedIPCQueueIds* queueIds);

extern void destroyServerSyncTools(sharedSynchronizationTools* tools);


#endif
