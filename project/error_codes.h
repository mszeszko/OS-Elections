/*
     Maciej Szeszko,
     id: ms335814,
     University of Warsaw
*/

#ifndef __ERROR_CODE_H_
#define __ERROR_CODE_H_

/*
   Usage error codes.
*/
#define COMMITTEE_USAGE_ERROR_CODE "Usage: %s <valid committee number>\n"
#define REPORT_USAGE_ERROR_CODE "Usage: %s [optional: <list>]\n"
/* 
   IPC queue error codes.
*/
#define IPC_QUEUE_NOT_INITIALIZED_ERROR_CODE "Error: IPC queue not initialized in server.\n"
#define IPC_QUEUE_SEND_OPERATION_ERROR_CODE "Error: IPC send.\n"
#define IPC_QUEUE_RECEIVE_OPERATION_ERROR_CODE "Error: IPC receive.\n"

/*
   Permission error codes.
*/
#define ACCESS_DENIED_ERROR_CODE "Access denied!\n"
#define CONNECTION_REFUSED_ERROR_CODE '!'

#endif
