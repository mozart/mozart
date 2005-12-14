/*
 *  Authors:
 *    Zacharias El Banna
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
#ifndef __DSS_TEMPLATES
#define __DSS_TEMPLATES

// Place nice templates in here for code reusability
//

#include <stdlib.h> // For the macro 
#include "dss_enums.hh" // For the macro 

template<class T>
void t_swap(T& t1, T& t2){
  T tmp = t1; t1 = t2; t2 = tmp;
}

//
// The Oncecontainer should be extended with non-defined functions such as
// - m_makeGCpreps()
// - Destructor
// - m_stringrep() 

  // ****************** SMALL FUNS ********************
  //
  template <class T1>
  T1 t_max(const T1& t1, const T1& t2){ return ((t1 > t2) ? t1 : t2); }

  template <class T1>
  T1 t_min(const T1& t1, const T1& t2){ return ((t1 < t2) ? t1 : t2); }
 
  // **************** One element Containers **********
  //
  template <class T1>
  class OneContainer{
  public:
    T1* const a_contain1;
    OneContainer* a_next;

  public:
    OneContainer(T1* const t, OneContainer* const n):
      a_contain1(t),
      a_next(n){
    };
    inline void m_makeGCpreps(){ a_contain1->m_makeGCpreps(); }

    MACRO_NO_DEFAULT_CONSTRUCTORS(OneContainer);
  };

  template <typename T1>
  class OneTypeContainer{
  public:
    T1 a_contain1;
    OneTypeContainer* a_next;

  public:
    OneTypeContainer(const T1& t, OneTypeContainer* const n):
      a_contain1(t),
      a_next(n){
    };
    inline void m_makeGCpreps(){};

    MACRO_NO_DEFAULT_CONSTRUCTORS(OneTypeContainer);
  };


  // **************** Two element Containers **********
  //
  template <class T1, typename T2>
  class TwoContainer{
  public:
    T1*           a_contain1;
    T2            a_contain2;
    TwoContainer* a_next;

  public:


    TwoContainer(T1* const t1, const T2& t2, TwoContainer* const n):
      a_contain1(t1),
      a_contain2(t2),
      a_next(n){
    };
    
    inline bool operator==(const TwoContainer*& tc){ 
      return (tc->a_contain1 == a_contain1 && tc->a_contain2 == a_contain2); }

    inline void m_makeGCpreps(){ a_contain1->m_makeGCpreps(); }
    ~TwoContainer(){};

    MACRO_NO_DEFAULT_CONSTRUCTORS(TwoContainer);
  };

template <class T1, class T2>
class TwoClassContainer{
  public:
    T1* a_contain1;
    T2* a_contain2;
    TwoClassContainer* a_next;
    TwoClassContainer(T1* const t1, T2* const t2, TwoClassContainer* const n):
      a_contain1(t1),
      a_contain2(t2),
      a_next(n){
    };
    inline void m_makeGCpreps(){ a_contain1->m_makeGCpreps(); a_contain2->m_makeGCpreps(); }
    ~TwoClassContainer(){};
  };

  template <typename T1, typename T2>
  class TwoTypeContainer{
   public:
    T1 a_contain1;
    T2 a_contain2;
    TwoTypeContainer* a_next;
    TwoTypeContainer(const T1& t1, const T2& t2, TwoTypeContainer* const n):
      a_contain1(t1),
      a_contain2(t2),
      a_next(n){
    };
    inline void m_makeGCpreps(){}
    ~TwoTypeContainer(){}

    MACRO_NO_DEFAULT_CONSTRUCTORS(TwoTypeContainer);
  };

  // ************* List Operators *****************
  //

  template<class T1>
  void t_forAllMeth(T1* head, void (T1::*fptr)()){
    while(head){
      (head->*fptr)();
      head  = head->a_next;
    }
  }

  template<class T1, typename T2>
    T2 t_foldMeth(T1* head, void (T1::*fptr)(T2), T2 arg){
    while(head){
      arg = (head->*fptr)(arg);
      head  = head->a_next;
    }
    return arg; 
  }
    

  // ************* Garbage collect a list ************
  //

  template<class T1>
  void t_gcList(T1* const head){
    T1* tmp = head;
    while(tmp){
      tmp->m_makeGCpreps();
      tmp = tmp->a_next;
    }
  }


  // ************* Delete a list ********************
  //
  // Takes a pointer to a list structure and deletes all elements and
  // also sets the pointer to NULL
  //

  template<class T1>
  void t_deleteList(T1*& head){
    while(head){
      T1* tmp = head;
      head  = head->a_next;
      delete tmp;
    }
  }


  template<class T1>
  void t_deleteListCont(T1*& head){
    while(head){
      T1* tmp = head;
      head  = head->a_next;
      delete tmp->a_contain1;
      delete tmp;
    }
  }

  
  template<class T1>
  void t_deleteListAction(T1*& head, void (T1::*fptr)()){
    while(head){
      T1* tmp = head;
      head  = head->a_next;
      (tmp->*fptr)();
      delete tmp;
    }
  }
  
 
  // ************* Delete element from list ********************
  //
  // - return whether the object was found in the list or not
  // - safe to call with any pointer since it's const guarded
  //

  template<class T1>
  bool t_deleteElement(T1** const headptr, T1* elem){
    T1** tmp = headptr;
    while((*tmp) != NULL){
      if ((*tmp) != elem){
	tmp = &((*tmp)->a_next);
      }else {
	T1* tmpdel = (*tmp);
	(*tmp) = (*tmp)->a_next;
	delete tmpdel;
	return true;
      }
    }
    return false;
  }
  
  // *********** Delete Element with container equal to ****** 
  //
  // - return whether the object was found in the list or not
  // - safe to call with any pointer since it's const guarded
  // 

  template<class T1, class T2>
  bool t_deleteCompare(T1** const headptr, T2* const compare){
    T1** tmp = headptr;
    while((*tmp) != NULL){
      if ((*tmp)->a_contain1 != compare){
	tmp = &((*tmp)->a_next);
      } else {
	T1* tmpdel = (*tmp);
	(*tmp) = (*tmp)->a_next;
	delete tmpdel;
	return true;
      }
    }
    return false;
  }

  //
  // Same as above except that it removes all occurances with same
  // element
  //

template<class T1, class T2>
void t_deleteAllCompare(T1** const headptr, T2* const compare){
  T1** tmp = headptr;
  while((*tmp) != NULL){
    if (compare != (*tmp)->a_contain){
	tmp = &((*tmp)->a_next);
    } else {
      T1* tmpdel = (*tmp);
      (*tmp) = (*tmp)->a_next;
      delete tmpdel;
      }
  }
}







  // ******************* SIMPLE FIFO QUEUE *****************
  //
  // - Uses elements which has a "a_next" pointer and might do
  // m_makeGCpreps() (if method not invoked this isn't tested either)
  //
  // !! Remember, the last elements "next"-ptr is never touched !!
  // !! We say last element is when back and front is equal     !!
  //
  template <class T1>
  class FifoQueue{
  private:
    T1* a_front;
    T1* a_back;

    FifoQueue& operator=(const FifoQueue& f);
  public:
    FifoQueue( FifoQueue& f):
      a_front(f.a_front),a_back(f.a_back){
      f.a_back = NULL; 
      f.a_front = NULL;
    };
    FifoQueue():a_front(NULL),a_back(NULL){};
    

    void append(T1* const t){
      if (a_front != NULL){ // empty
	a_back->a_next = t;
	a_back = t;
      }else {
	a_front = a_back = t;
      }
      t->a_next = NULL;
    }
    
    T1* drop(){
      T1* tmp = a_front;
      if(a_front == a_back) a_front = a_back = NULL;   // tmp is last element
      else                  a_front = a_front->a_next; // at least one element left (besides tmp)
      return tmp; // Yes this might return NULL if the queue is empty
    }
    
    T1* peek(){ return a_front;}
    
    bool isEmpty(){ return (a_front == NULL); }

    unsigned int m_size() {
      int i = 0;
      for (T1* tmp = a_front; tmp != NULL; tmp = tmp->a_next) i++;
      return i;
    }

    void m_makeGCpreps(){ t_gcList(a_front); }

    ~FifoQueue(){ t_deleteList(a_front); };
  };

  // ********************** SIMPLE LIST STRUCTURE ********************
  //
  //
  template <class T1>
  class SimpleList{
  protected:
    T1* a_head;

  public:
    
    SimpleList():a_head(NULL){};

    ~SimpleList(){
      while(a_head){
	T1* tmp = a_head;
	a_head  = a_head->a_next;
	delete tmp;
      }
    }
    
    bool isEmpty(){ return a_head == NULL; }
    
    void insertElement(T1* a){
      a->a_next = a_head;
      a_head = a;
    }
    
    T1* dropListHead(){
      T1* tmp = a_head;
      a_head = a_head->a_next;
      return tmp;
    }

    T1** getHead(){ return (&a_head); }

    T1* dropAllList(){
      T1* tmp = a_head;
      a_head = NULL;
      return tmp;
    }

    MACRO_NO_DEFAULT_CONSTRUCTORS(SimpleList);
  };



#endif
