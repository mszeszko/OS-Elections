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
#define SERVER_USAGE_ERROR_CODE "Usage: %s <lists>, <candidates per list>, <committees>"

/* 
   IPC queue error codes.
*/
#define IPC_QUEUE_INITIALIZATION_ERROR_CODE "Error: IPC queue initialization failed!\n"
#define IPC_QUEUE_NOT_INITIALIZED_ERROR_CODE "Error: IPC queue not initialized in server.\n"
#define IPC_QUEUE_SEND_OPERATION_ERROR_CODE "Error: IPC send.\n"
#define IPC_QUEUE_RECEIVE_OPERATION_ERROR_CODE "Error: IPC receive.\n"
#define IPC_QUEUE_REMOVE_OPERATION_ERROR_CODE "Error: IPC queue removal failed!\n"

/*
   Sync tools error codes.
*/
#define MUTEX_INITIALIZATION_ERROR_CODE "Error: Mutex initialization failed!\n"
#define MUTEX_DESTROY_ERROR_CODE "Error: Mutex destruction failed!\n"

#define THREAD_ATTRIBUTE_INITIALIZATION_ERROR_CODE "Error: threads' attribute initialization failed!\n"
#define THREAD_ATTRIBUTE_DETACHSTATE_INITIALIZATION_ERROR_CODE "Error: threads' attribute detachstate initialization failed!\n"

#define COMMITTEES_CONDITION_INITIALIZATION_ERROR_CODE "Error: Committees condition initialization failed!\n"
#define COMMITTEE_WORKER_THREAD_INITIALIZATION_ERROR_CODE "Error: Committee worker thread initialization failed!\n"
#define COMMITTEE_DISPATCHER_THREAD_INITIALIZATION_ERROR_CODE "Error: Comittee dispatcher thread initialization failed!\n"
#define COMMITTEES_CONDITION_DESTROY_ERROR_CODE "Error: Committees condition destruction failed!\n"
#define COMMITTEE_DISPATCHER_THREAD_JOIN_ERROR_CODE "Error: Joining committee dispatcher thread failed!\n"

#define REPORTS_CONDITION_INITIALIZATION_ERROR_CODE "Error: Reports condition initialization failed!\n"
#define REPORT_WORKER_THREAD_INITIALIZATION_ERROR_CODE "Error: Report worker thread initialization failed!\n"
#define REPORT_DISPATCHER_THREAD_INITIALIZATION_ERROR_CODE "Error: Report dispatcher thread initalization failed!\n"
#define REPORTS_CONDITION_DESTROY_ERROR_CODE "Error: Reports condition destruction failed!\n"
#define REPORT_DISPATCHER_THREAD_JOIN_ERROR_CODE "Error: Joining report dispatcher thread failed!\n"

/*
   Permission error codes.
*/
#define ACCESS_DENIED_ERROR_CODE "Error: Access denied!\n"
#define CONNECTION_REFUSED_ERROR_CODE '!'

/*
   Signal error codes.
*/
#define SIGNAL_HANDLING_INITIALIZATION_ERROR_CODE "Error: Signal handler initialization failed!\n"

#endif
