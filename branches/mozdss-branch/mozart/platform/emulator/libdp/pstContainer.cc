/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Erik Klintskog, 2002
 * 
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 * 
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://www.mozart-oz.org
 * 
 *  See the file "LICENSE" or
 *     http://www.mozart-oz.org/LICENSE.html
 *  for information on usage and redistribution 
 *  of this file, and for a DISCLAIMER OF ALL 
 *  WARRANTIES.
 *
 */

#if defined(INTERFACE)
#pragma implementation "pstContainer.hh"
#endif

#include "pstContainer.hh"
#include "glue_buffer.hh"

static BYTE shared_byte_area[16000];
DPMarshalers* DPM_Repository;

static PstContainer *g_PstContainers = NULL; 


PstContainer::PstContainer(OZ_Term trm):a_term(trm), a_prev(NULL), a_next(g_PstContainers){
  if(g_PstContainers != NULL) // if someone is behind repoint him to me.
    g_PstContainers->a_prev = this;
  g_PstContainers = this;
}

PstContainer::~PstContainer(){
  if(a_prev != NULL) // if someone before, repoint his next to my next
    a_prev->a_next = a_next;
  else // I'm first, repoint global list to next
    g_PstContainers = a_next;
  if(a_next != NULL) // If someone after, repoint his before to mine
    a_next->a_prev = a_prev;
}



//***************************** INCONTAINER ****************************

#ifdef INTERFACE
int PstInContainer::a_allocated=0;
#endif

PstInContainer::PstInContainer():PstContainer((OZ_Term) 0), a_builder_cont(NULL) {
#ifdef INTERFACE
  a_allocated++;
#endif
}
PstInContainer::PstInContainer(PstOutContainer *pst):
  PstContainer(pst->a_term), a_builder_cont(NULL) {
#ifdef INTERFACE
  a_allocated++;
#endif
}


void PstInContainer::gc(){
  oz_gCollectTerm(a_term, a_term);
  if(a_builder_cont != NULL) a_builder_cont->gCollect();
}

void PstInContainer::gcStart(){
  // The builder doesn't have to be g-collected in three 
  // phases
}

void PstInContainer::gcFinish(){
  // The builder doesn't have to be g-collected in three 
  // phases  
}


bool PstInContainer::unmarshal(DssReadBuffer *buf){
  int len = buf->availableData();
  buf->readFromBuffer(shared_byte_area, len); 
  GlueReadBuffer bb(shared_byte_area, len);
  Builder *dpum;

  if (a_builder_cont == NULL)
    {
      dpum = DPM_Repository->dpGetUnmarshaler();
      // The new unmarshaler needs to be initiated.. 
      dpUnmarshalerStartBatch(dpum);
    }
  else
    {
      // Since the old one allready was 
      // initiated we dont have to do it again. 
      dpum = a_builder_cont;

    }
  
  OZ_Term utRet = dpUnmarshalTerm(&bb, dpum);
  int readLen = bb.bufferUsed();
  buf->commitRead(readLen);

  switch (utRet) {
  case (OZ_Term) 0:
    dpUnmarshalerFinishBatch(dpum);
    return FALSE;
  case (OZ_Term) -1:
    a_builder_cont = dpum;
    return FALSE;
  default:
    dpUnmarshalerFinishBatch(dpum);
    a_term =  utRet;
    DPM_Repository->dpReturnUnmarshaler(dpum);
    return TRUE;
  }
  
  
  
}


PstOutContainerInterface* 
PstInContainer::loopBack2Out(){
  return static_cast<PstOutContainerInterface*>(new PstOutContainer(this));
}



// ***************************** PST Out *********************

#ifdef INTERFACE
int PstOutContainer::a_allocated=0;
#endif



PstOutContainer::PstOutContainer(OZ_Term t):
  PstContainer(t), a_marshal_cont(NULL), a_fullTopTerm(FALSE), a_pushContents(FALSE){
#ifdef INTERFACE
  a_allocated++;
#endif
}


PstOutContainer::PstOutContainer(PstInContainer *pst):
  PstContainer(pst->a_term), a_marshal_cont(NULL), a_fullTopTerm(FALSE), a_pushContents(FALSE){
#ifdef INTERFACE
  a_allocated++;
#endif
}

void PstOutContainer::gc(){
  oz_gCollectTerm(a_term, a_term);
  if (a_marshal_cont != NULL) a_marshal_cont->gCollect();
}

void PstOutContainer::gcStart(){
  if (a_marshal_cont != NULL) a_marshal_cont->gcStart();
}

void PstOutContainer::gcFinish(){
  if (a_marshal_cont != NULL) a_marshal_cont->gcFinish();
}
bool PstOutContainer::marshal(DssWriteBuffer* buf){
  int len = buf->availableSpace();
  GlueWriteBuffer bb(shared_byte_area, len);
  
  if (a_marshal_cont == NULL)
    {
      DPMarshaler *dpm = DPM_Repository->dpGetMarshaler();
      
      if(a_fullTopTerm){
	dpm->genFullToplevel();
      }
      
      if(a_pushContents){
	dpm->pushContents();
      }
      a_marshal_cont = dpMarshalTerm(a_term, &bb, dpm);	

      int written_len = bb.bufferUsed();
      buf->writeToBuffer(shared_byte_area,written_len);
      if(a_marshal_cont == NULL)
	{
	  DPM_Repository->dpReturnMarshaler(dpm);
	  return TRUE;
	}
      return FALSE; 
    }
  else
    {
      DPMarshaler *dpm = (DPMarshaler *) a_marshal_cont;
      a_marshal_cont = dpMarshalContTerm(&bb, dpm);
      
      int written_len = bb.bufferUsed();
      buf->writeToBuffer(shared_byte_area,written_len);
      
      if (a_marshal_cont == NULL) 
	{
	  DPM_Repository->dpReturnMarshaler(dpm);
	  return TRUE; 
	}
      return FALSE; 
    }
}


void PstOutContainer::resetMarshaling(){
  if (a_marshal_cont != NULL)
    {
      DPM_Repository->dpReturnMarshaler(a_marshal_cont);
      a_marshal_cont = NULL; 
    }
}

PstInContainerInterface* PstOutContainer::loopBack2In(){
  return static_cast<PstInContainerInterface*>(new PstInContainer(this));
}

PstOutContainerInterface*
PstOutContainer::duplicate(){
  Assert(0); 
  return NULL; 
}





void gcPstContainersStart(){
  for(PstContainer *ptr = g_PstContainers; ptr!=NULL; ptr = ptr->getNext())
    ptr->gcStart();   
}

void gcPstContainersRoot(){
  for(PstContainer *ptr = g_PstContainers; ptr!=NULL; ptr = ptr->getNext())
    ptr->gc();
}

 void gcPstContainersFinish(){
  for(PstContainer *ptr = g_PstContainers; ptr!=NULL; ptr = ptr->getNext())
    ptr->gcFinish();
#ifdef INTERFACE
  //printf("PstInContainers  allocated:%d\n",PstInContainer::a_allocated);
  //printf("PstOutContainers allocated:%d\n",PstOutContainer::a_allocated);
#endif
}
