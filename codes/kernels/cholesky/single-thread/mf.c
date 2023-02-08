

/*************************************************************************/
/*                                                                       */
/*  Copyright (c) 1994 Stanford University                               */
/*                                                                       */
/*  All rights reserved.                                                 */
/*                                                                       */
/*  Permission is given to use, copy, and modify this software for any   */
/*  non-commercial purpose as long as this copyright notice is not       */
/*  removed.  All other uses, including redistribution in whole or in    */
/*  part, are forbidden without prior written permission.                */
/*                                                                       */
/*  This software is provided with absolutely no warranty and no         */
/*  support.                                                             */
/*                                                                       */
/*************************************************************************/



#include <stdlib.h>
#include <semaphore.h>
#include <assert.h>
#if __STDC_VERSION__ >= 201112L
#include <stdatomic.h>
#endif
#include <stdint.h>
#define PAGE_SIZE 4096


#include "matrix.h"
#define HashNum 1024
#define Bucket(desti, destj, src) ((desti+destj+src)%HashNum)

long uMiss = 0;
extern struct GlobalMemory *Global;
struct Update **updateHash;

struct taskQ {
	struct Task *taskQ;
	struct Task *taskQlast;
	struct Task *probeQ;
	struct Task *probeQlast;
	} *tasks;

extern BMatrix LB;

void InitTaskQueues(long P)
{
  long i;

  tasks = (struct taskQ *) MyMalloc(P*sizeof(struct taskQ), DISTRIBUTED);
  for (i=0; i<P; i++) {
    tasks[i].taskQ = (struct Task *) NULL;
    tasks[i].taskQlast = (struct Task *) NULL;
    tasks[i].probeQ = (struct Task *) NULL;
    tasks[i].probeQlast = (struct Task *) NULL;
    }
}


/* Find block number of block at position (i,j) */

long FindBlock(long i, long j)
{
  long lo, hi, probe;

  lo = LB.col[j]; hi = LB.col[j+1];
  for (;;) {
    if (lo == hi)
      break;
    probe = (lo+hi)/2;
    if (LB.row[probe] == i)
      break;
    else if (LB.row[probe] > i)
      hi = probe;
    else
      lo = probe+1;
  }

  if (LB.row[probe] == i)
    return(probe);
  else
    return(-1);
}


/* p is processor no if block_num = -1, ignored otherwise */

void Send(long src_block, long dest_block, long desti, long destj, struct Update *update, long p, long MyNum, struct LocalCopies *lc)
{
  long procnum, is_probe;
  struct Task *t;

  procnum = p;

  is_probe = (dest_block == -2);

  if (lc->freeTask) {
    t = lc->freeTask;
    lc->freeTask = t->next;
  }
  else {
    t = (struct Task *) MyMalloc(sizeof(struct Task), MyNum);
  }

  t->block_num = dest_block;
  t->desti = desti; t->destj = destj; t->src = src_block; t->update = update;
  t->next = NULL;

  if (is_probe) {
    if (tasks[procnum].probeQlast)
      tasks[procnum].probeQlast->next = t;
    else
      tasks[procnum].probeQ = t;
    tasks[procnum].probeQlast = t;
  }
  else {
    if (tasks[procnum].taskQlast)
      tasks[procnum].taskQlast->next = t;
    else
      tasks[procnum].taskQ = t;
    tasks[procnum].taskQlast = t;
  }

}


long TaskWaiting(long MyNum)
{
  return(tasks[MyNum].taskQ != NULL);
}


void GetBlock(long *desti, long *destj, long *src, struct Update **update, long MyNum, struct LocalCopies *lc)
{
  struct Task *t;

  for (;;) {
      t = NULL;
	while(!tasks[MyNum].probeQ && !tasks[MyNum].taskQ)
      if (tasks[MyNum].probeQ) {
        t = (struct Task *) tasks[MyNum].probeQ;
        tasks[MyNum].probeQ = t->next;
	if (!t->next)
	  tasks[MyNum].probeQlast = NULL;
      }
      else if (tasks[MyNum].taskQ) {
        t = (struct Task *) tasks[MyNum].taskQ;
        tasks[MyNum].taskQ = t->next;
	if (!t->next)
	  tasks[MyNum].taskQlast = NULL;
      }

    if (t) {
        break;
    }
  }

  *desti = t->desti; *destj = t->destj; *src = t->src; *update = t->update;

  /* free task */
  t->next = lc->freeTask;
  lc->freeTask = t;

  /* straight through is 32 cycles */
}


