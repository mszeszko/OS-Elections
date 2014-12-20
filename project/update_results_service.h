/*
     Maciej Szeszko,
     id: ms335814,
     University of Warsaw
*/

#ifndef __UPDATE_RESULTS_SERVICE_H_
#define __UPDATE_RESULTS_SERVICE_H_

#include "server_structures.h"

extern void updateResultsServiceInitialProtocol(
  sharedSynchronizationTools* tools, sharedSynchronizedVariables* variables);

extern void updateResultsServiceEndingProtocol(
  sharedSynchronizationTools* tools, sharedSynchronizedVariables* variables);

#endif
