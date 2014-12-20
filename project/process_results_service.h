/*
     Maciej Szeszko,
     id: ms335814,
     University of Warsaw
*/

#ifndef __PROCESS_RESULTS_SERVICE_H_
#define __PROCESS_RESULTS_SERVICE_H_

#include "server_structures.h"

extern void processResultsServiceInitialProtocol(
  sharedSynchronizationTools* tools, sharedSynchronizedVariables* variables);

extern void processResultsServiceEndingProtocol(
  sharedSynchronizationTools* tools, sharedSynchronizedVariables* variables);

#endif
