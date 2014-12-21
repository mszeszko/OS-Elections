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
  unsigned int electionResults[MAX_LISTS][MAX_CANDIDATES_PER_LIST];
  unsigned int summaryListVotes[MAX_LISTS];
  unsigned int processedCommittees;
  unsigned int committeeConnectionPossible[MAX_COMMITTEES];
  unsigned int eligibledVoters;
  unsigned int validVotes;
  unsigned int invalidVotes;
  unsigned int lists;
  unsigned int candidatesPerList;
  unsigned int committees;
} sharedDataStructures;

/* Shared synchronization variables. */
typedef struct {
  unsigned int workingCommitteeThreads;
  unsigned int committeesWantToUpdateResults;
  unsigned int committeesUpdatingResults;
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
  pthread_attr_t dispatcherThreadAttribute;
  pthread_attr_t workerThreadAttribute;
  pthread_mutex_t mutex; 
  pthread_cond_t committeeUpdateResultsCondition;
  pthread_cond_t reportProcessResultsCondition;
  pthread_cond_t committeeWorkingThreadsCondition;
} sharedSynchronizationTools;

typedef struct {
  sharedIPCQueues queueIds;
  sharedDataStructures sharedData;
  sharedSynchronizationVariables syncVariables;
  sharedSynchronizationTools syncTools;
} applicationPackage;

typedef struct {
  unsigned int partialResults[MAX_LISTS][MAX_CANDIDATES_PER_LIST];
  unsigned int eligibledVoters;
  unsigned int totalVotes;
  unsigned int validVotes;
  unsigned int list;
} committeeWorkerResources;

#endif

