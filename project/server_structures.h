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
  int electionResults[MAX_LISTS][MAX_CANDIDATES_PER_LIST];
  int summaryListVotes[MAX_LISTS];
  int processedCommittees;
  int committeeConnectionPossible[MAX_COMMITTEES];
  int eligibledVoters;
  int validVotes;
  int invalidVotes;
  int lists;
  int candidatesPerList;
  int committees;
} sharedDataStructures;

/* Shared synchronization variables. */
typedef struct {
  int workingThreads;
  int committeesWantToUpdateResults;
  int committeesUpdatingResults;
  int reportsProcessingResults;
  int** partialResults[MAX_DEDICATED_SERVER_THREADS];
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
  pthread_attr_t dispatcherThreadAttribute;
  pthread_attr_t workerThreadAttribute;
  pthread_mutex_t mutex; 
  pthread_cond_t committeeUpdateResultsCondition;
  pthread_cond_t reportProcessResultsCondition;
  pthread_cond_t workingThreadsCondition;
} sharedSynchronizationTools;

typedef struct {
  pthread_t reportDispatcher;
  pthread_t committeeDispatcher;
  pthread_t workerThreads[MAX_DEDICATED_SERVER_THREADS];
} sharedThreadVariables;

typedef struct {
  sharedIPCQueueIds queueIds;
  sharedDataStructures sharedData;
  sharedSynchronizationVariables syncVariables;
  sharedSynchronizationTools syncTools;
} applicationPackage;

typedef struct {
  int eligibledVoters;
  int processedMessages;
  int totalVotes;
  int validVotes;
  int list;
} committeeWorkerResources;

#endif

