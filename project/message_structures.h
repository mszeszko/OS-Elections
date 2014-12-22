/*
      Maciej Szeszko
      id: ms335814,
      University of Warsaw
*/

#ifndef __MESSAGE_STRUCTURES_H_
#define __MESSAGE_STRUCTURES_H_

#include "constants.h"

/* ***** */
/* ENUMS */
/* ***** */
enum CommitteeOperationType {
  HEADER = 0,
  DATA,
  FINISH
};

enum CandidatePosition {
  FIRST = 0,
  MIDDLE,
  LAST
};

enum ReportProgress {
  REPORT_IN_PROGRESS = 0,
  REPORT_COMPLETED
};

enum Connection {
  CONNECTION_SUCCEEDED = 0,
  CONNECTION_REFUSED
};

enum IndexAccessibility {
  AVAILABLE = 0, /* should not be changed at any reason. */
  NOT_AVAILABLE,
  NOT_FOUND = -1
};

typedef struct {
  int threadIndex;
  long committee;
} committeeThreadData;

typedef struct {
  int threadIndex;
  int pid;
  int list;
} reportThreadData;

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
  int committee;
  enum Connection ack;
} initialConnectionMessage;


/*
   Basic local committee information.
*/
typedef struct {
  int eligibledVoters;
  int totalVotes;
} basicCommitteeInfo;


/*
   Data message structure. Send by committee to dedicated server thread.
   Note:
   -> `localInfo`: is set only when sending first chunk of data through IPC.
   -> `finished`: set as the only message after whole data has been processed.
*/
typedef struct {
  long operationId;
  int list;
  int candidate;
  int candidateVotes;
  basicCommitteeInfo localInfo;
  enum CommitteeOperationType type;
} committeeMessage;


/*
   Acknowledgment that last chunk of data from particular committee process
   has been just processed.
*/
typedef struct {
  long operationId;
  int processedMessages;
  int validVotes;
} serverAckMessage;


/*
   Report header structure.
*/
typedef struct {
  long operationId;
  int processedCommittees;
  int committees;
  int validVotes;
  int invalidVotes;
  int eligibledVoters;
} reportHeaderMessage;


/*
   Single list report structure.
*/
typedef struct {
  int list;
  int candidateVotes;
  int votes;
  enum ReportProgress reportProgress;
  enum CandidatePosition position;
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
} getAccessTokenMessage;

typedef struct {
  long operationId;
  int pid;
  int reportList;
} getReportMessage;

typedef struct {
  long operationId;
  singleListReport listReport;
} singleListReportMessage;

typedef struct {
  long operationId;
} groupAccessTokenMessage;

#endif
