/*
     Maciej Szeszko,
     id: ms335814,
     University of Warsaw
*/

#ifndef __SERVER_SERVICE_H_
#define __SERVER_SERVICE_H_

#include <pthread.h>

#include "server_structures.h"

/* Initialization */
extern void initializeServerSyncTools(sharedSynchronizationTools* tools);

extern void initializeServerIPCQueues(sharedIPCQueueIds* queueIds);

extern void initializeThreadAttribute(pthread_attr_t* threadAttribute,
  int detachState);

/* Free resources. */
extern void freeServerIPCQueuesResources(sharedIPCQueueIds* queueIds);

extern void destroyServerSyncTools(sharedSynchronizationTools* tools);

/* general reports stuff */
extern void receiveReportRequestMessage(int IPCQueueId, int* pid, int* list);

extern int findIndexInThreadsArray(sharedSynchronizationTools* tools,
  sharedThreadVariables* threads);

extern void releaseIndexInThreadsArray(sharedSynchronizationTools* tools,
  sharedThreadVariables* threads, int threadIndex);

#endif
