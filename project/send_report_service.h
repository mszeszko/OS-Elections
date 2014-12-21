/*
     Maciej Szeszko,
     id: ms335814,
     University of Warsaw
*/

#ifndef __SEND_REPORT_SERVICE_H_
#define __SEND_REPORT_SERVICE_H_

#include "server_structures.h"

extern void prepareAndSendStatisticsReportHeader(sharedDataStructures* data,
  long operationId, int IPCQueueId);

extern void prepareAndSendSingleListReport(sharedDataStructures* data,
  unsigned int list, long operationId, int IPCQueueId);

extern void prepareAndSendCompleteReport(sharedDataStructures* data,
  unsigned int list, int IPCQueueId);

#endif