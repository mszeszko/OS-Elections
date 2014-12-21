/*
     Maciej Szeszko,
     id: ms335814,
     University of Warsaw
*/

#ifndef __SERVER_CONFIG_SERVICE_H_
#define __SERVER_CONFIG_SERVICE_H_

#include <pthread.h>

extern void initializeThreadAttribute(pthread_attr_t* threadAttribute,
  int detachState);

extern void initializeReportGroupAccessTokenIPCQueue(int IPCQueueId,
  unsigned int committees);

/*extern void createReportDispatcherThread(pthread_t* thread,
  pthread_attr_t threadAttribute);*/

extern void receiveReportRequestMessage(int IPCQueueId, unsigned int* list);

/*extern void createReportWorkerThread(pthread_t* thread, unsigned int list);*/

extern void putBackGroupAccessToken(int IPCQueueId, unsigned int list);

#endif
