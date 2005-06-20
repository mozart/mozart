/*
 *  Authors:
 *    Zacharias El Banna, 2002
 *    Erik Klintskog, 2002
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
#ifndef __GLUE_TABLES_HH
#define __GLUE_TABLES_HH

#ifdef INTERFACE  
#pragma interface
#endif


#include "value.hh"
#include "glue_mediators.hh"
class AddressHashTableO1Reset;
class Watcher;


/*
  The engine table handles the address objects and look-ups for these.

  The engine table itself  involved in garbage collection with three
  sweeps. Primary -> Weak -> CleanUp. CleanUp needs to be run before DSS-cleanup
  and has to set action on each proxy.

  The engine table is currently realized as a hash table on tagged refs and
  a "waste" list for keeping address objects only reached from the DSS (notably
  variables) this list is however a root to the local gc and ao:s here are
  treated as any other address object in that sense.

 */

class EngineTable{
private:
  AddressHashTableO1Reset *addressMain;
  AddressHashTableO1Reset *addressBackup;
 
  Mediator *aoList;


  void remove(Mediator *ao);
  void backup(Mediator *ao);
  bool isEmpty();
  int  getSize();

public:
  EngineTable(int size);
  ~EngineTable();

  void insert(Mediator *ao, TaggedRef tr);

  Mediator *lookupMediator(TaggedRef tr);
  Mediator *findMediator(void *en);

  /******************* DSS ***********************/
  void gcPrimary();
  void gcWeak();
  void gcCleanUp(); // removes old entries

  void print();
  /****************** LATER **********************/
  //ProxyInterface *      getPN(TaggedRef entity);
};


extern EngineTable *engineTable;


/*************************** GC **************************/
void gcEngineTablePrimary();
void gcEngineTableWeak();
void gcEngineTableCleanUp();

/************************ LOOK-UPS ***********************/

// Get an EngineName from a tagged ref
//EngineName tr2En(TaggedRef tr); 

// Get an Mediator from a taggedref, 
// this demands a full lookup in the hash table. 

Mediator *taggedref2Me(TaggedRef tr);

Mediator *index2Me(const int&);
// Get a ProxyInterface * from a engine name
AbstractEntity *index2AE(const int&);
CoordinatorAssistantInterface *index2CAI(const int&);

// Get a ProxyInterface * from a TaggedRef, returns
// the ProxyInterface * 0 if no entry was found. 



#endif
