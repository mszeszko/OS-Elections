/*
     Maciej Szeszko,
     id: ms335814,
     University of Warsaw
*/

#ifndef __CONSTANTS_H_
#define __CONSTANTS_H_

/*
   IPC message queue keys.
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
   IPC constant message types.
*/
#define INIT_CONNECTION_MESSAGE_TYPE 0L
#define SEND_RAPORT_REQUEST_MESSAGE_TYPE 0L
#define READ_RAPORT_MESSAGE_TYPE 1L + MAX_LISTS

/*
   Process specific symbols.
*/
#define FINISHED_DATA_PROCESSING '#'
   
/*
   Process specific templates.
*/
#define PROCESSED_MESSAGES_TEMPLATE "Przetworzonych wpisów: %u\n"
#define ELIGIBLED_VOTERS_TEMPLATE "Uprawnionych do głosowania: %u\n"
#define VALID_VOTES_TEMPLATE "Głosów ważnych: %u\n"
#define INVALID_VOTES_TEMPLATE "Głosów nieważnych: %u\n"
#define TURNOUT_TEMPLATE "Frekwencja w lokalu: %.2f\n"

#endif
