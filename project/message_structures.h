/*
      Maciej Szeszko
      id: ms335814,
      University of Warsaw
*/

#ifndef __MESSAGE_STRUCTURES_H_
#define __MESSAGE_STRUCTURES_H_

#include "constants.h"

/* ************** */
/* IPC STRUCTURES */
/* ************** */

/* 
   Message send by committee to initiate connection with server.
   As a response server returns appropriately set `ack` field,
   that represents whether connection has been successfuly initiated or not. 
*/
typedef struct {
  long operationId;
  char ack;
} initialConnectionMessage;


/*
   Basic local committee information.
*/
typedef struct {
  unsigned int eligibledVoters;
  unsigned int totalVotes;
} basicCommitteeInfo;


/*
   Data message structure. Send by committee to dedicated server thread.
   Note:
   -> `localInfo`: is set only when sending first chunk of data through IPC.
   -> `finished`: set as the only message after whole data has been processed.
*/
typedef struct {
  long operationId;
  unsigned int list;
  unsigned int candidate;
  unsigned int candidateVotes;
  basicCommitteeInfo localInfo;
	char finished; 
} committeeMessage;


/*
   Acknowledgment that last chunk of data from particular committee process
   has been just processed.
*/
typedef struct {
  long operationId;
  unsigned int processedMessages;
  unsigned int validVotes;
} serverAckMessage;


/*
   Report header structure.
*/
typedef struct {
  long operationId;
  unsigned int processedCommittees;
  unsigned int committees;
  unsigned int validVotes;
  unsigned int invalidVotes;
  unsigned int eligibledVoters;
} reportHeaderMessage;


/*
   Single list report structure.
*/
typedef struct {
  unsigned int list;
  unsigned int candidates;
  unsigned int* candidateVotes;
  unsigned int votes;
  char finish;
} singleListReport;


/*
   Report message structures.
   -> `getAccessTokenMessage`,
   -> `getReportMessage`: sent by `report` process to get back report
      for specific votelist(or eventually all of them),
   -> `singleListReportMessage`: represents report for specified list,
   -> `allListReportMessage`: report for all lists
*/
typedef struct {
  long operationId;
  long reportList;
} getAccessTokenMessage;

typedef struct {
  long operationId;
  long reportList;
} getReportMessage;

typedef struct {
  long operationId;
  singleListReport listReport;
} singleListReportMessage;

#endif
