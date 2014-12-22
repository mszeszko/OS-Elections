/*
     Maciej Szeszko,
     id: ms335814,
     University of Warsaw
*/

#include "process_results_service.h"

#include <stdio.h>
#include <stdlib.h>

void processResultsServiceInitialProtocol(
  sharedSynchronizationTools* tools,
  sharedSynchronizationVariables* variables) {
  pthread_mutex_lock(&tools->mutex);
  while(variables->committeesWantToUpdateResults)
    pthread_cond_wait(&tools->reportProcessResultsCondition, &tools->mutex);
  ++(variables->reportsProcessingResults);
  pthread_mutex_unlock(&tools->mutex);
}

void processResultsServiceEndingProtocol(
  sharedSynchronizationTools* tools,
  sharedSynchronizationVariables* variables) {
  pthread_mutex_lock(&tools->mutex);
  if (--(variables->reportsProcessingResults) == 0)
    pthread_cond_signal(&tools->committeeUpdateResultsCondition);
  pthread_mutex_unlock(&tools->mutex);
}
