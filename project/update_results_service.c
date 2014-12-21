/*
     Maciej Szeszko,
     id: ms335814,
     University of Warsaw
*/

#include "update_results_service.h"

void updateResultsServiceInitialProtocol(
  sharedSynchronizationTools* tools, sharedSynchronizedVariables* variables) {
  pthread_mutex_lock(&tools->mutex);
  ++(variables->committeeWantToUpdateResults);
  while (!((variables->reportsProcessing == 0) &&
    (variables->committeeUpdatingResults)))
    pthread_cond_wait(&tools->committeeUpdateResultsCondition, &tools->mutex);
  ++(variables->committeeUpdatingResults);
  pthread_mutex_unlock(&tools->mutex);
}

void updateResultsServiceEndingProtocol(
  sharedSynchronizationTools* tools, sharedSynchronizedVariables* variables) {
  pthread_mutex_lock(&tools->mutex);
  --(variables->committeeUpdatingResults);
  if (--(variables->committeeWantToUpdateResults) == 0)
    pthread_cond_broadcast(&tools->reportProcessResultsCondition);
  else
    pthread_cond_signal(&tools->committeeUpdateResultsCondition);
  pthread_mutex_unlock(&tools->mutex);
}
