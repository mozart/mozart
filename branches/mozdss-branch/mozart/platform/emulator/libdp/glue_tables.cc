/*
 *  Authors:
 *    Zacharias El Banna, 2002
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Zacharias El Banna, 2002
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
#pragma implementation "glue_tables.hh"
#endif

#include "am.hh"
#include "hashtbl.hh"
#include "glue_tables.hh"
#include "pstContainer.hh"
#include "engine_interface.hh"

void doPortSend(PortWithStream *port,TaggedRef val,Board*);
OZ_Return accessCell(OZ_Term cell,OZ_Term &out);

EngineTable *engineTable;




/**********************************************************************/
/*   Localizing                 should be more localize */
/**********************************************************************/

void Object::localize(){
  setTertType(Te_Local);
  setBoard(oz_currentBoard());
}


/********************************* ENGINE TABLE ***********************************/


EngineTable::EngineTable(int size){
  addressMain   = new AddressHashTableO1Reset(size);
  addressBackup = new AddressHashTableO1Reset(size); 
  aoList = NULL;
}

int
EngineTable::getSize(){
  int i = 0;
  Mediator *ao_tmp=aoList;
  while(ao_tmp != NULL){
    ao_tmp = ao_tmp->next;
    i++;
  }
  return i;
}

/*
  Insert address object into big chained list and also into hashtable for the glue
 */
void
EngineTable::insert(Mediator *ao, TaggedRef tr){
  // insert (and possibly overwrite obsolete) hash table
  addressMain->htAddOverWrite((void *)tr, (void *)ao);
  //insert into chained list
  ao->prev = NULL;
  ao->next = aoList;
  if (aoList != NULL)
    aoList->prev = ao;
  aoList = ao;
}

void
EngineTable::remove(Mediator *ao){
  if (ao->prev != NULL)
    ao->prev->next = ao->next;
  else // first in list
    aoList = ao->next;
  if(ao->next != NULL)
    ao->next->prev = ao->prev;
  delete (ao);
}


void
EngineTable::backup(Mediator *ao){
  //Reset for next garbage collection
  ao->resetGC();

  if (ao->connect == MEDIATOR_CONNECT_HASH) // If still known by engine
    addressBackup->htAdd((void*)ao->getEntity(),(void*)ao);
}

bool
EngineTable::isEmpty(){
  return (aoList == NULL); 
}

EngineTable::~EngineTable(){
  delete addressMain;
  delete addressBackup;
  Mediator *ao_tmp;
  while (aoList != NULL) {
    ao_tmp = aoList;
    aoList = aoList->next;
    delete ao_tmp;
  }
}

Mediator*
EngineTable::lookupMediator(TaggedRef tr){
  void* med = addressMain->htFind((void *)tr) ;
  if ( med ==  htEmpty) return NULL;
  else reinterpret_cast<Mediator*>(med); 
}

Mediator *
EngineTable::findMediator(void *en){
#ifdef INTERFACE
  Mediator *ao_tmp = static_cast<Mediator *>(en); // Just for debugging simplicity
  Assert((int)en != 0xbedda);
  Assert(ao_tmp->check());
  return ao_tmp;
#else
  return static_cast<Mediator *>(en);
#endif
}


void
EngineTable::print(){
  printf("************************* ENGINE TABLE *************************\n");
  printf("%d\n",getSize());
  Mediator *ao_tmp = aoList;
  while(ao_tmp != NULL){
    ao_tmp->print();
    ao_tmp = ao_tmp->next;
  }
  printf("********************** ENGINE TABLE - DONE *********************\n");
}

// ZACHARIAS
// Should create a weak list on the fly to avoid scanning the entire
// dist-table twice
//

void
EngineTable::gcPrimary(){
  printf("--- raph: EngineTable::gcPrimary\n");
  Mediator *ao_tmp = aoList;
  while(ao_tmp != NULL){
    // If we don't localize things this could be opted away to just be
    // performed when engine_gc is DEAD
    ao_tmp->dss_gc = ao_tmp->getCoordinatorAssistant()->getDssDGCStatus();
    if(ao_tmp->dss_gc == DSS_GC_PRIMARY)
      ao_tmp->dssGC();
    ao_tmp = ao_tmp->next;
  }
  //  printf("**** DONE ****\n");
}


void
EngineTable::gcWeak(){
  printf("--- raph: EngineTable::gcWeak\n");
  Mediator *ao_tmp = aoList;
  while(ao_tmp != NULL){
    if(ao_tmp->dss_gc == DSS_GC_WEAK){
      // might not be collected yet
      ao_tmp->dssGC();
    }
    ao_tmp = ao_tmp->next;
  }
}


void
EngineTable::gcCleanUp(){
  //This method is responsible for quite a few things
  // 1) calculate what action the DSS should take
  // 2) deleting the ao if it is obsolete
  // 3) reset the status of the ao for next gc
  // 4) back up the ao for the cleaning phase
  printf("--- raph: EngineTable::gcCleanUp\n");
  Mediator *ao_tmp = aoList;
  bool remove;

  while(ao_tmp != NULL){
    remove = true;
    switch(ao_tmp->dss_gc){ // have been retreived in the primary step
    case DSS_GC_LOCALIZE:
      // If not wanted by engine we haven't collected it so it is safe to remove
      // else we try to localize it
      if (ao_tmp->hasBeenGC())
	{
	  printf("localizing, not working\n"); 
	  Assert(0); 
	}
      break;
    case DSS_GC_NONE:
      //If neither dss nor engine wants it then clean out else save
      remove = !(ao_tmp->hasBeenGC());
      break;
    case DSS_GC_PRIMARY:
      remove = false; //We have collected it so its a keeper;
      break;
    case DSS_GC_WEAK:
      remove = false;
      if (!(ao_tmp->hasBeenGC())){ // Try remove weak
	// Ok so we could actually remove it if it succeds, introduce that later
	ao_tmp->getCoordinatorAssistant()->clearWeakRoot();
      }
      break;
    default:
      OZ_error("Unknown dss action %d",ao_tmp->dss_gc);
      break;
    }
    Mediator* tmp_ao = ao_tmp->next;
    if (remove){
      EngineTable::remove(ao_tmp);
    }
    else //Back up
      backup(ao_tmp);
    //printf(" ACTION:%d\n",action);
    ao_tmp = tmp_ao;
  }
  
  AddressHashTableO1Reset *tmp=addressBackup;
  addressMain->mkEmpty();
  addressBackup = addressMain;
  addressMain = tmp;
}



void gcEngineTablePrimary(){
  //printf("GC PRIMARY\n");
  engineTable->gcPrimary();
};

void gcEngineTableWeak(){
  //  printf("GC WEAK CLEAN\n");
  engineTable->gcWeak();
};

void gcEngineTableCleanUp(){
  //  printf("GC CLEAN UP\n");
  engineTable->gcCleanUp();
};


// Todo: the shortcuts must return error values
// if no addressobject was found. Erik 

Mediator *taggedref2Me(TaggedRef tr){
  return engineTable->lookupMediator(tr);
}

Mediator *index2Me(const int& indx){
  return engineTable->findMediator(reinterpret_cast<Mediator *>(indx));
}
// Was index2Pi now index2AE
// Get a AbstractEntity *from a engine name
AbstractEntity *index2AE(const int& indx){
  return engineTable->findMediator(reinterpret_cast<Mediator *>(indx))->getAbstractEntity();
};

CoordinatorAssistantInterface *index2CAI(const int& indx){
  return engineTable->findMediator(reinterpret_cast<Mediator *>(indx))->getCoordinatorAssistant();
};



