/*
  Hydra Project, DFKI Saarbruecken,
  Stuhlsatzenhausweg 3, D-W-6600 Saarbruecken 11, Phone (+49) 681 302-5312
  Author: scheidhr
  Last modified: $Date$ from $Author$
  Version: $Revision$
  State: $State$

  ------------------------------------------------------------------------
*/

#if defined(INTERFACE) && !defined(PEANUTS)
#pragma implementation "taskstk.hh"
#endif

#include "am.hh"

int TaskStack::frameSize(ContFlag cFlag)
{
  switch (cFlag){
  case C_CONT: 
    return 3;      
  case C_XCONT:
    return 4;
  case C_DEBUG_CONT:
  case C_SET_SELF:
  case C_LTQ:
  case C_CATCH:
  case C_ACTOR:  
    return 2;
  case C_CALL_CONT:  
  case C_CFUNC_CONT:
    return 3;
    
  default:
    Assert(0);
    return -1;
  }
}

int TaskStack::tasks()
{
  TaskStackEntry *oldTos=tos;
  int len=0;

  while (!isEmpty()) {
    len++;

    TaskStackEntry entry=*(--tos);
    ContFlag cFlag = getContFlag(ToInt32(entry));
    tos = tos - frameSize(cFlag) + 1;
  }
  tos=oldTos;
  return len;
}

void TaskStack::checkMax()
{
  int maxSize = getMaxSize();
  if (ozconf.stackMaxSize != -1 &&
      maxSize >= ozconf.stackMaxSize) {
    int newMaxSize = (maxSize*3)/2;

loop:
    prefixError();
    printf("\n\n*** Task stack maxsize exceeded. Increase from %d to %d? (y/n/b) ",
	   ozconf.stackMaxSize,newMaxSize);
    fflush(stdout);
    char buf[1000];
    osfgets(buf,1000,stdin);
    switch (buf[0]) {
    case 'n':
      am.exitOz(1);
    case 'y':
      break;
    case 'b':
      printTaskStack();
      goto loop;
    default:
      goto loop;
    }
    ozconf.stackMaxSize = newMaxSize;
  }
}


TaggedRef TaskStack::findCatch(TaggedRef &out) {
  out = nil();

  Assert(this);

  while (!isEmpty()) {
    TaggedPC topElem = ToInt32(pop());
    ContFlag flag = getContFlag(topElem);
    switch (flag){
    case C_CONT:
      {
        ProgramCounter PC = getPC(C_CONT,topElem);
        RefsArray Y = (RefsArray) pop();
        RefsArray G = (RefsArray) pop();
        out = cons(CodeArea::dbgGetDef(PC),out);
      }
      break;

    case C_XCONT:
      {
        ProgramCounter PC = getPC(C_XCONT,topElem);
        RefsArray Y = (RefsArray) pop();
        RefsArray G = (RefsArray) pop();
        RefsArray X = (RefsArray) pop();
        out = cons(CodeArea::dbgGetDef(PC),out);
        break;
      }

    case C_ACTOR:
      pop();
      out = cons(OZ_atom("actor"),out);
      break;

    case C_CFUNC_CONT:
      {
        OZ_CFun biFun    = (OZ_CFun) pop();
        RefsArray X      = (RefsArray) pop();
        out = cons(OZ_mkTupleC("builtin",2,
                               OZ_atom(builtinTab.getName((void *) biFun)),
                               OZ_toList(getRefsArraySize(X),X)),
		   out);
        break;
      }

    case C_DEBUG_CONT:
      {
        OzDebug *deb = (OzDebug*) pop();
        out = cons(OZ_atom("debug"),out);
        break;
      }

    case C_CALL_CONT:
      {
        TaggedRef pred = (TaggedRef) ToInt32(pop());
        RefsArray X = (RefsArray) pop();
        out = cons(OZ_mkTupleC("apply",2,
			       pred,OZ_toList(getRefsArraySize(X),X)),
		   out);
        break;
      }

    case C_CATCH:
      {
        TaggedRef pred = (TaggedRef) ToInt32(pop());
	return pred;
      }

    case C_SET_SELF:
      { 
        Object *oldSelf = am.getSelf();
        Object *newSelf = (Object *) pop();
	oldSelf->setDeepness(0); /* free the object */
	am.setSelf(newSelf);
        out = cons(OZ_atom("setSelf"),out);
        break;
      }

    case C_LTQ:
      {
        ThreadQueueImpl * ltq = (ThreadQueueImpl *) pop();
        out = cons(OZ_atom("ltq"),out);
        break;
      }
    default:
      Assert(0);
    } // switch
  } // while

  return 0;
}


