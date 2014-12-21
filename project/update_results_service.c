/*
     Maciej Szeszko,
     id: ms335814,
     University of Warsaw
*/

#include "update_results_service.h"

#include "server_structures.h"

void updateResultsServiceInitialProtocol(
  sharedSynchronizationTools* tools,
  sharedSynchronizationVariables* variables) {
  pthread_mutex_lock(&tools->mutex);
  ++(variables->committeesWantToUpdateResults);
  while (!((variables->reportsProcessingResults == 0) &&
    (variables->committeesUpdatingResults)))
    pthread_cond_wait(&tools->committeeUpdateResultsCondition, &tools->mutex);
  ++(variables->committeesUpdatingResults);
  pthread_mutex_unlock(&tools->mutex);
}

void updateResultsServiceEndingProtocol(
  sharedSynchronizationTools* tools,
  sharedSynchronizationVariables* variables) {
  pthread_mutex_lock(&tools->mutex);
  --(variables->committeesUpdatingResults);
  if (--(variables->committeesWantToUpdateResults) == 0)
    pthread_cond_broadcast(&tools->reportProcessResultsCondition);
  else
    pthread_cond_signal(&tools->committeeUpdateResultsCondition);
  pthread_mutex_unlock(&tools->mutex);
}
