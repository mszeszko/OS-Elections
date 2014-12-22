/*
		 Maciej Szeszko,
		 id: ms335814,
		 University of Warsaw
*/

#ifndef __REPORT_SERVICE_H_
#define __REPORT_SERVICE_H_

extern void getReportGroupAccessToken(unsigned int list);

extern void tryReportConnection(int* reportDataIPCQueueId);

extern void receiveAndPrintData(int reportDataIPCQueueId, unsigned int list);

extern void sendGetReportMessageRequest(int IPCQueueId, unsigned int list);

#endif
