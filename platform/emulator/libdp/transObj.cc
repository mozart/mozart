#include "transObj.hh"
#include "timers.hh"
#include "connection.hh"

TransController::TransController():
  FreeListManager(TransController_CUTOFF) {
  PD((TCPCACHE,"TransController instantiated"));
  wc = 0;
  used = 0;
  // DPMarshalers is initialized empty;
  allocm = 0;
  Assert(getMNum() == 0);
  usedm=0;
  usedum=0;
  running=running_last=waiting=waiting_last=NULL;
  timer=NULL;
}

Bool transController_closeOne(void *arg) {
  return ((TransController *) arg)->closeOne();
}

// If resources are available a fresh transObj to be used at accept
// is returned, else NULL
// The comObj must report in as soon as he gets the transObj.
TransObj *TransController::getTransObj() {
  PD((TCPCACHE,"TransObj for accept requested"));
  if (used<getMaxNumOfResources()) {
    used++;
    if(used>getWeakMaxNumOfResources() && timer==NULL) {
      timers->setTimer(timer,ozconf.perdioTempRetryFloor,
                       transController_closeOne,this);
    }
    return newTransObj();
  }
  else {
    PD((TCPCACHE,"No transObj for accept provided"));
    return NULL;
  }
}

// Must be called by the comObj after accept
void TransController::addRunning(ComObj *comObj) {
  addLast(running,running_last,comObj);
}

// Called when comObj:s pass on a connection
void TransController::switchRunning(ComObj *inList,ComObj *newc) {
  PD((TCPCACHE,"switch running from %x to %x",inList,newc));
  ComObj *prev=NULL;
  ComObj *tmp=running;
  while(tmp!=NULL && tmp!=inList) {
    prev=tmp;
    tmp=tmp->next_cache;
  }

  Assert(tmp!=NULL && tmp==inList);
  if(prev!=NULL)
    prev->next_cache=newc;
  else
    running=newc;
  if(running_last==inList)
    running_last=newc;
  newc->next_cache=tmp->next_cache;
}

// When (possibly now) resources are available comObj->transObjReady
// is called with a fresh transObj.
void TransController::getTransObj(ComObj *comObj) {
  PD((TCPCACHE,"TransObj for connect requested %d %d",used,
      getWeakMaxNumOfResources()));
  TransObj *transObj;
  if(used<getWeakMaxNumOfResources()) {
    used++;
    transObj=newTransObj();
//      transObj->setOwner(comObj);
//      addLast(running,running_last,comObj);
    transObjReady(comObj,transObj);
  }
  else
    enqueue(comObj);
}

// After comObj->preemptTransObj this method shall be called by comObj
// It should also be called when returning a TransObj after everything is done
void TransController::transObjFreed(ComObj *comObj,TransObj *transObj) {
  PD((TCPCACHE,"TransObj returned, used:%d",used));
  remove(running,running_last,comObj);
  if(used<=getWeakMaxNumOfResources()) {
    ComObj *next=getFirst(waiting,waiting_last);
    if (next!=NULL) {
      PD((TCPCACHE,"Reusing a resource"));
      transObj->init(); // Refresh!
//        transObj->setOwner(next);
//        addLast(running,running_last,next);
      transObjReady(next,transObj);
      return;
    }
    PD((TCPCACHE,"Have a resource, but no one who wants it"));
  }
  PD((TCPCACHE,"Deleting a transObj, now used: %d",used-1));
  deleteTransObj(transObj);
  used--;
}

// When the comObj is done and no longer wants to wait for a resource:
void TransController::comObjDone(ComObj *comObj) {
  remove(waiting,waiting_last,comObj);
}

void TransController::allocateMarshalersForResources(int numOfResources)
{
  // there is one marshaler pair per resource currently:
  allocm = numOfResources;
  //
  dpAllocateMarshalers(allocm);
  Assert(getMNum() == allocm);
  Assert(usedm < allocm);
}


void TransController::changeNumOfResources() {
  PD((TCPCACHE,"****************************** CHANGE *************"));

  //
  int weakmax=getWeakMaxNumOfResources();
  allocateMarshalersForResources(getMaxNumOfResources());

  if(used>weakmax && timer==NULL) { // If timer is set things will work
    timers->setTimer(timer,ozconf.perdioTempRetryFloor,
                     transController_closeOne,this);
  }
  ComObj *next=(ComObj *) 0x1; // Assignement to enter while
  TransObj *transObj;
  while(used<weakmax && next!=NULL) {
    next=getFirst(waiting,waiting_last);
    if (next!=NULL) {
      used++;
      transObj=newTransObj();
//        transObj->setOwner(next);
//        addLast(running,running_last,next);
      transObjReady(next,transObj);
    }
    else { // All satisfied, can clear timer!
      timers->clearTimer(timer);
      PD((TCPCACHE,"Have a new resource, but no one who wants it"));
    }
  }
}

DPMarshaler *TransController::getMarshaler()
{
  if (usedm < allocm) {
    usedm++;
    return (dpGetMarshaler());
  } else {
    if (!allocm) {
      allocateMarshalersForResources(getMaxNumOfResources());
      return (getMarshaler());
    } else {
      // error?
      return ((DPMarshaler *) 0x0);
    }
  }
}

Builder *TransController::getUnmarshaler()
{
  if (usedum < allocm) {
    usedum++;
    return (dpGetUnmarshaler());
  } else {
    if (!allocm) {
      allocateMarshalersForResources(getMaxNumOfResources());
      return (getUnmarshaler());
    } else {
      // error?
      return ((Builder *) 0x0);
    }
  }
}

void TransController::returnMarshaler(DPMarshaler *dpm)
{
  usedm--;
  dpReturnMarshaler(dpm);
}

void TransController::returnUnmarshaler(Builder *dpm)
{
  usedum--;
  dpReturnUnmarshaler(dpm);
}

// Private methods: ///////////////////////////////////////////////////////

ComObj *TransController::getFirst(ComObj *&list, ComObj *&list_last) {
  ComObj *first=list;
  if(first!=NULL) {
    list=first->next_cache;
    if(list==NULL)
      list_last=NULL;
  }
  PD((TCPCACHE,"getFirst found %x",first));
  return first;
}

void TransController::addLast(ComObj *&list,ComObj *&list_last,ComObj *c) {
  PD((TCPCACHE,"''''''''''addLast %x %x %x",list,list_last,c));
  if(list_last!=NULL) {
    list_last->next_cache=c;
    list_last=c;
  }
  else {
    Assert(list==NULL);
    list=list_last=c;
  }
  c->next_cache=NULL;
}

void TransController::enqueue(ComObj *c) {
  PD((TCPCACHE,"enqueue timer is %x",timer));
  addLast(waiting,waiting_last,c);
  if(timer==NULL) {
    //    closeOne(); // UNHEALTHY
    // Set timer to wake us up to close someones connection soon.
    // ozconf.perdioTempRetryFloor used as timer interval for now
    timers->setTimer(timer,ozconf.perdioTempRetryFloor,
                     transController_closeOne,this);
  }
}

void TransController::remove(ComObj *&list,ComObj *&list_last,ComObj *c) {
  PD((TCPCACHE,".............remove %x %x %x",list,list_last,c));
  ComObj *cur=list;
  ComObj *prev=NULL;
  while(cur!=NULL && c!=NULL) {
    if(c==cur) {
      if(prev!=NULL)
        prev->next_cache=c->next_cache;
      else {
        list=c->next_cache;
      }
      if(list_last==c)
        list_last=prev;
      return;
    }
    else {
      prev=cur;
      cur=cur->next_cache;
    }
  }
}

Bool TransController::closeOne() {
  ComObj *cur=running;

  ComObj *ebeq=NULL;   // Empty buffer+Empty queue (first choice)
  ComObj *ebq=NULL;    // Empty buffer+Queue       (second choice)
  ComObj *b=NULL;      // Buffer                   (third choice)

  if(used>getWeakMaxNumOfResources() ||
     (used==getWeakMaxNumOfResources() && waiting!=NULL)) {
    PD((TCPCACHE,"Trying to close one"));
    while(cur!=NULL) {
      if(cur->canBeClosed()) {
        if(cur->transObj->hasEmptyBuffers()) {
          if(!cur->hasQueued()) {
            ebeq=cur;
            break;
          }
          else if(ebq==NULL)
            ebq=cur; // Go on looking
        }
        else if(b==NULL)
          b=cur;
      }
      cur=cur->next_cache;
    }

    if(ebeq!=NULL)
      ebeq->preemptTransObj();
    else if(ebq!=NULL)
      ebq->preemptTransObj();
    else if(b!=NULL)
      b->preemptTransObj();
    else
      PD((TCPCACHE,"found no one to close (running %x)",running));

    return TRUE;
  }
  else {
    timer=NULL;
    return FALSE;
  }
}
