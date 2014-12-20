/*
     Maciej Szeszko,
     id: ms335814,
     University of Warsaw
*/

#ifndef __CONSTANTS_H_
#define __CONSTANTS_H_

/*
   IPC message queues keys.
*/
#define INIT_CONNECTION_IPC_QUEUE_KEY 1234L
#define COMMITTEE_DATA_IPC_QUEUE_KEY 1235L
#define FINISH_IPC_QUEUE_KEY 1236L
#define RAPORT_IPC_QUEUE_KEY 1237L

/*
   System constraints.
*/
#define MAX_DEDICATED_COMMITTEE_SERVER_THREADS 100
#define MAX_LISTS 100
#define MAX_CANDIDATES_PER_LIST 100

/*
   IPC message constant message types.
*/
#define INIT_CONNECTION_MESSAGE_TYPE 0L
#define SEND_RAPORT_REQUEST_MESSAGE_TYPE 0L
#define READ_RAPORT_MESSAGE_TYPE 1L + MAX_LISTS

#endif
