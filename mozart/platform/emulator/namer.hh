/*
 *  Authors:
 *    Tobias Mueller (tmueller@ps.uni-sb.de)
 * 
 *  Contributors:
 * 
 *  Copyright:
 *    Organization or Person (Year(s))
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

#ifndef __NAMER_HH__
#define __NAMER_HH__

//////////////////////////////////////////////////////////////////////////////

#define NEW_NAMER
// #define NEW_NAMER_DEBUG
#ifdef NEW_NAMER
#define NAME_PROPAGATORS
#endif

#ifdef NEW_NAMER_DEBUG
#define NEW_NAMER_DEBUG_PRINT(A) printf A; fflush(stdout)
#else
#define NEW_NAMER_DEBUG_PRINT(A)
#endif


#ifdef NEW_NAMER

// to enable a class to be garbage collected by `GCMeManager' is has
// to be derived from `GCMe'
class GCMe {
public:
  virtual void gCollect(void) = 0;
  virtual void sClone(void) = 0;
};

class GCMeManager {
private:
  static GCMeManager * _head;
  GCMeManager * _next;
  GCMe * _object;
  GCMeManager(GCMe * o, GCMeManager * n) : _object(o), _next(n) {}
public:
  static void registerObject(GCMe * o) {        
    for (GCMeManager * tmp = _head; tmp; tmp = tmp->_next)
      if (tmp->_object == o) // already registered
	return; 
    
    _head = new GCMeManager(o, _head);
  }
  static void unregisterObject(GCMe * o) {
    GCMeManager * tmp = _head;
    GCMeManager ** next_before = &_head;

    while (tmp) {
      if (tmp->_object == o) // found, remove it  
	break;
      next_before = &(tmp->_next);
      tmp = tmp->_next;
    }
    
    if (! tmp) // not registered
      return;

    *next_before = tmp->_next; // skip `tmp'
    delete tmp;
  }
  static void gCollect(void) {
    for (GCMeManager * tmp = _head; tmp; tmp = tmp->_next)
      tmp->_object->gCollect();
  }
  static void sClone(void) {
    for (GCMeManager * tmp = _head; tmp; tmp = tmp->_next)
      tmp->_object->sClone();
  }
};


//=============================================================================

Bool isCacMarkedNamer(OZ_Term);
void GCollectIndexNamer(OZ_Term &);
void GCollectDataNamer(const char * &);
OZ_Term derefIndexNamer(OZ_Term);
const char * toStringNamer(const char *);

Bool isCacMarkedNamer(Propagator *);
void GCollectIndexNamer(Propagator * &p);
void GCollectDataNamer(OZ_Term &);
Propagator * derefIndexNamer(Propagator *);
const char * toStringNamer(OZ_Term);
OZ_Term getCacForward(OZ_Term t);
Propagator * getCacForward(Propagator * p);

template <class T_INDEX, class T_NAME>
class Namer : public GCMe {
private:
  T_INDEX _index;
  T_NAME _name;
  Namer<T_INDEX, T_NAME> * _next;

  static Namer<T_INDEX, T_NAME> * _head;

  Namer(T_INDEX index, T_NAME name, Namer<T_INDEX, T_NAME> * next)
    : _index(index), _name(name), _next(next) {}
public:
  Namer(void) { 
    GCMeManager::registerObject(this);
  }
  static Namer<T_INDEX, T_NAME> * getHead(void) { 
    return _head; }
  static T_NAME getName(T_INDEX index) {
    //printf("namer.hh getName _head=%p\n",_head);fflush(stdout);
    for (Namer<T_INDEX, T_NAME> * tmp = _head; tmp; tmp = tmp->_next) {
      tmp->_index = derefIndexNamer(tmp->_index);
      //printf("getName %d -- %d -- %s \n",tmp->_index,index,tmp->_name);fflush(stdout);
      if (tmp->_index == index) {
	return tmp->_name;
      }
    }
    return (T_NAME) NULL;
  }
  static void addName(T_INDEX index, T_NAME name) {
    Assert(index != 0);

    for (Namer<T_INDEX, T_NAME> * tmp = _head; tmp; tmp = tmp->_next)
      if (tmp->_index == index)  // it is already contained
	return;

    _head = new Namer<T_INDEX, T_NAME>(index, name, _head);
    //printf("addName namer.hh %d %s\n",index,name);fflush(stdout);
    
    NEW_NAMER_DEBUG_PRINT(("adding %s at index %x\n", toStringNamer(name), (int) index));
  }
  static void cloneEntry(T_INDEX index_org, T_INDEX index_clone) {
    T_NAME name = getName(index_org); 
    
    if (!name) 
      return;
    
    addName(index_clone, name);
  }
  void gCollect(void) {
    //printf("namer.cc gCollect\n");fflush(stdout);
    Namer<T_INDEX, T_NAME> * tmp = _head;
    _head = NULL;

    while (tmp) {
      NEW_NAMER_DEBUG_PRINT(("gc namer: what 0x%x ", (OZ_Term) tmp->_index));

      if (isCacMarkedNamer(tmp->_index)) {
	NEW_NAMER_DEBUG_PRINT(("keeping %s\n", toStringNamer(tmp->_name)));

	GCollectIndexNamer(tmp->_index);
	GCollectDataNamer(tmp->_name);
	Namer<T_INDEX, T_NAME> * tmp_add = tmp;
	tmp = tmp->_next;
	tmp_add->_next = _head;
	_head = tmp_add;
      } else {
	NEW_NAMER_DEBUG_PRINT(("deleting %s\n", toStringNamer(tmp->_name)));

	Namer<T_INDEX, T_NAME> * tmp_delete = tmp;
	tmp = tmp->_next;
	delete tmp_delete;
      }
      
    }
  }
  void sClone(void) {
    printf("namer.hh sClone\n");fflush(stdout);
    Namer<T_INDEX, T_NAME> * tmp = _head;

    while (tmp) {
      NEW_NAMER_DEBUG_PRINT(("sClone namer what 0x%x ", (OZ_Term) tmp->_index));

      if (isCacMarkedNamer(tmp->_index)) {
	NEW_NAMER_DEBUG_PRINT(("copied %s\n", toStringNamer(tmp->_name)));
	addName(getCacForward(tmp->_index), tmp->_name);
      } else {
	NEW_NAMER_DEBUG_PRINT(("untouched %s\n", toStringNamer(tmp->_name)));
      }
      tmp = tmp->_next;      
    }
  }
};
  
#endif /* NEW_NAMER */

/* ------------------------------------------------------------------------
 * maintain the mapping of variables to print names
 * ------------------------------------------------------------------------ */

const char *oz_varGetName(TaggedRef v);
void oz_varAddName(TaggedRef v, const char *nm);
void oz_varCleanup();


#ifdef NEW_NAMER
OZ_Term oz_propGetName(Propagator *);
void oz_propAddName(Propagator *, OZ_Term);
#endif

//////////////////////////////////////////////////////////////////////////////
#endif /* __NAMER_HH__ */
