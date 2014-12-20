/*
     Maciej Szeszko,
     id: ms335814,
     University of Warsaw
*/

#ifndef __SERVER_STRUCTURES_H_
#define __SERVER_STRUCTURES_H_

#include <pthread.h>

#include "constants.h"


/* Shared IPC queue ids. */
typedef struct {
  int initConnectionIPCQueueId;
  int committeeDataIPCQueueId;
  int finishIPCQueueId;
  int reportGroupAccessTokenIPCQueueId;
  int reportDataIPCQueueId;
} sharedIPCQueueIds;

/* Shared data structures. */
typedef struct {
  unsigned int electionsResults[MAX_LISTS][MAX_CANDIDATES_PER_LIST];
  unsigned int summaryListVotes[MAX_LISTS];
  char committeeConnectionPossible[MAX_COMMITTEES];
  unsigned int eligibledVoters;
  unsigned int validVotes;
  unsigned int invalidVotes;
} sharedDataStructures;

/* Shared synchronization variables. */
typedef struct {
  unsigned int committeesWaitingForUpdatingResults;
  unsigned int committeeUpdatingResults;
  unsigned int reportsWaitingForProcessingResults;
  unsigned int reportsProcessingResults;
} sharedSynchronizationVariables;
 
/* 
   Shared synchronization tools:
   -> `mutex`: ensures exclusive access to data structures
       and synchronization variables.
   -> `committeeUpdateResults`: signaled when committee can safely
       write results into shared data structures.
   -> `reportProcessResults`: signaled when report can safely process
       results and send them safely to the client.
*/
typedef struct {
  pthread_mutex_t mutex; 
  pthread_cond_t committeeUpdateResults;
  pthread_cond_t reportProcessResults;
} sharedSynchronizationTools;

#endif

