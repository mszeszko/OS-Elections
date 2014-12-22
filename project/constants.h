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
#define REPORT_GROUP_ACCESS_TOKEN_IPC_QUEUE_KEY 1237L
#define REPORT_DATA_IPC_QUEUE_KEY 1238L

/*
   System constraints.
*/
#define MAX_DEDICATED_SERVER_THREADS 2
#define MAX_LISTS 100
#define MAX_CANDIDATES_PER_LIST 100
#define MAX_COMMITTEES 10000
#define ALL_LISTS_ID MAX_LISTS + 1

/*
   IPC constant message types.
*/
#define INIT_CONNECTION_MESSAGE_TYPE MAX_LISTS + 1
#define REPORT_REQUEST_MESSAGE_TYPE MAX_LISTS + 2
   
/*
   Process specific templates.
*/
#define PROCESSED_MESSAGES_TEMPLATE "Przetworzonych wpisów: %d\n"
#define ELIGIBLED_VOTERS_TEMPLATE "Uprawnionych do głosowania: %d\n"
#define VALID_VOTES_TEMPLATE "Głosów ważnych: %d\n"
#define INVALID_VOTES_TEMPLATE "Głosów nieważnych: %d\n"
#define TURNOUT_TEMPLATE "Frekwencja w lokalu: %.2f\n"

#define PROCESSED_COMMITTEES_TEMPLATE "Przetworzonych komisji: %d / %d\n"
#define COMMITTEE_RESULTS_TEMPLATE "Wyniki poszczególnych list:\n"

#endif
