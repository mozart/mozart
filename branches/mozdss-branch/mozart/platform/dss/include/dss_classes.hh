/*
 *  Authors:
 *   Zacharias El Banna
 *   Erik Klintskog 
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
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

#ifndef __DSS_CLASSES_HH
#define __DSS_CLASSES_HH

#ifdef INTERFACE  
#pragma interface
#endif


#include "dss_enums.hh"
#include <stddef.h> // size_t
//#include "../src/bucketHashTable.hh"




class PstOutContainerInterface;
class PstInContainerInterface;

class MediatorInterface;
class DssWriteBuffer; 
class DssReadBuffer; 
class CoordinatorAssistantInterface; 

//class BucketHashNode;

namespace _dss_internal{
  class DSS_Environment;
}

#ifndef WIN32
#define DSSDLLSPEC
#else
#ifdef DSS_EXPORTING
#define DSSDLLSPEC __declspec(dllexport)
#else
#define DSSDLLSPEC __declspec(dllimport)
#endif
#endif

//****************************** Not in the documantation yet *************************

class GlobalNameInterface;

typedef struct unmarshal_return{
  bool exist;
  bool skel;
} DSS_unmarshal_status;

// Used to identify an operation
class DSSDLLSPEC DssOperationId
{
public: 
  DssOperationId(){;}
};

// Used to identify a thread, local or remote. 
// The Map has to inherit from this class to implement 
// its thread identifiers. 
class DSSDLLSPEC ThreadMediator{
public:
  ThreadMediator();
  virtual WakeRetVal resumeDoLocal(DssOperationId*)=0;
  virtual WakeRetVal resumeRemoteDone(PstInContainerInterface* pstin)=0;

};


class DSSDLLSPEC DssThreadId{
private:
  ThreadMediator* a_thread; 

public: 
  DssThreadId():a_thread(NULL){}

  void setThreadMediator(ThreadMediator* t){
    a_thread = t; 
  }
  ThreadMediator* getThreadMediator(){
    return a_thread; 
  }
  virtual void dispose() =0; 

  MACRO_NO_DEFAULT_CONSTRUCTORS(DssThreadId);
};

// ***************************** SECTION 5.1 - THE PROXY *******************************
//Exported by the DSS


class DSSDLLSPEC AbstractEntity{
protected:
  MediatorInterface *a_mediator;
  
  AbstractEntity():a_mediator(NULL){};

public:
  void assignMediator(MediatorInterface *mediator){ a_mediator = mediator; };
  
  MediatorInterface *accessMediator() const { return a_mediator; }; 
  
  virtual CoordinatorAssistantInterface  *getCoordinatorAssistant() const = 0; 
  
  virtual ~AbstractEntity(){};
  
  virtual void remoteInitatedOperationCompleted(DssOperationId*,
						PstOutContainerInterface*) = 0; 
  virtual void localInitatedOperationCompleted() = 0; 
  
  MACRO_NO_DEFAULT_CONSTRUCTORS(AbstractEntity);
};

class DSSDLLSPEC CoordinatorAssistantInterface{
public:
  // **************************** Coordination Manipulation ********************
  virtual bool  manipulateCNET(void* argument) = 0; 

  // ***************************** Reference Consistency ************************
  virtual bool  manipulateRC(const RCalg& alg,
			     const RCop&  op,
			     opaque&      argument)=0;
  
  virtual DSS_GC     getDssDGCStatus()=0;

  virtual bool       clearWeakRoot()=0;

  // ******************************* Fault interface ****************************  

  virtual FaultState  getRegisteredFS() const = 0; 
  virtual void        setRegisteredFS(const FaultState&) = 0;
  virtual FaultState  getFaultState() const= 0; 
  virtual bool        marshal(DssWriteBuffer          *buf,
			      const ProxyMarshalFlag&  flag )=0;
};

class DSSDLLSPEC RelaxedMutableAbstractEntity:public  AbstractEntity{
public:
  virtual OpRetVal   abstractOperation_Read(DssThreadId *id,
					    PstOutContainerInterface**& pstout)=0;
  
  virtual OpRetVal   abstractOperation_Write(PstOutContainerInterface**& pstout)=0;
  
};

class DSSDLLSPEC MutableAbstractEntity:public  AbstractEntity{
public:
  virtual OpRetVal   abstractOperation_Read(DssThreadId *id,
					    PstOutContainerInterface**& pstout)=0;
  
  virtual OpRetVal   abstractOperation_Write(DssThreadId *id,
					     PstOutContainerInterface**& pstout)=0;
  
};

class DSSDLLSPEC MonotonicAbstractEntity:public  AbstractEntity{
public:
  virtual OpRetVal   abstractOperation_Bind(DssThreadId *id,
					    PstOutContainerInterface**& pstout)=0;
  
  virtual OpRetVal   abstractOperation_Append(DssThreadId *id,
					      PstOutContainerInterface**& pstout)=0;
};


class DSSDLLSPEC ImmutableAbstractEntity:public AbstractEntity{
public:
  virtual OpRetVal   abstractOperation_Read(DssThreadId *,
					    PstOutContainerInterface**&)=0;
  
};


// ****************************  SECTION 5.4 - THE MEDIATOR  *****************************
// - Imported by the DSS from the MAP.
//
// This class defines the general interafce methods for all shared entities. 
// Further specialization is done in the <AbstractEntity>MediatorInterafaces classes. 
// 

class DSSDLLSPEC MediatorInterface{
public:
  MediatorInterface();
  virtual PstOutContainerInterface *retrieveEntityRepresentation() = 0;
  virtual void installEntityRepresentation(PstInContainerInterface*) = 0; 

  virtual void localize() {;}
  virtual void reportFaultState(const FaultState& fs) = 0;
};

class DSSDLLSPEC  MutableMediatorInterface: public MediatorInterface{
public:
  MutableMediatorInterface();
  virtual AOcallback callback_Write(DssThreadId* id_of_calling_thread,
				    DssOperationId* operation_id,
				    PstInContainerInterface* operation,
				    PstOutContainerInterface*& possible_answer)=0;


  virtual AOcallback callback_Read(DssThreadId* id_of_calling_thread,
				   DssOperationId* operation_id,
				   PstInContainerInterface* operation,
				   PstOutContainerInterface*& possible_answer)=0;
  
};

class DSSDLLSPEC  RelaxedMutableMediatorInterface: public MediatorInterface{
public:
  RelaxedMutableMediatorInterface();
  virtual AOcallback callback_Write(DssThreadId* id_of_calling_thread, 
				    DssOperationId* operation_id,
				    PstInContainerInterface* operation)=0;
  
  virtual AOcallback callback_Read(DssThreadId* id_of_calling_thread,
				   DssOperationId* operation_id,
				   PstInContainerInterface* operation,
				   PstOutContainerInterface*& possible_answer)=0;
};



class DSSDLLSPEC  MonotonicMediatorInterface: public MediatorInterface{
public:
  MonotonicMediatorInterface();
  virtual AOcallback callback_Bind(DssOperationId* operation_id,
				   PstInContainerInterface* operation) = 0;
  
  virtual AOcallback callback_Append(DssOperationId* operation_id,
				     PstInContainerInterface* operation) = 0;

};

class DSSDLLSPEC  ImmutableMediatorInterface: public MediatorInterface{
public:
  ImmutableMediatorInterface();
  virtual AOcallback callback_Read(DssThreadId* id_of_calling_thread,
				   DssOperationId* operation_id,
				   PstInContainerInterface* operation,
				   PstOutContainerInterface*& possible_answer)=0;

};





// ********************** Marshaling and unmarshaling routines **********************
//Imported by the DSS from the MAP

class DSSDLLSPEC PstInContainerInterface{
public:
  PstInContainerInterface();
  virtual bool unmarshal(DssReadBuffer*) = 0; 
  virtual void dispose() = 0;
  virtual PstOutContainerInterface* loopBack2Out() = 0;
};

class DSSDLLSPEC PstOutContainerInterface{
public:
  PstOutContainerInterface();
  virtual bool marshal(DssWriteBuffer*) = 0;
  virtual void resetMarshaling() = 0;
  virtual void dispose() = 0;
  virtual PstInContainerInterface* loopBack2In() = 0;
  virtual PstOutContainerInterface* duplicate() = 0; 
};

// ********************************* I/O routines **********************************
// Imported


// ******************************* Connection Routines ********************************
// Exported 




// NOT IN DOCUMENTATION YET!
//
// The mediation entry, i.e. the entry to the MAP.
//

class DSSDLLSPEC Mediation_Object{
public:
  Mediation_Object();

  virtual PstInContainerInterface* createPstInContainer()=0;
  virtual void GL_error(const char* const format, ...)=0;
  virtual void GL_warning(const char* const format, ...)=0;
  
};


class DSSDLLSPEC GlobalNameInterface{
protected:
  void* a_ref;
public:
  GlobalNameInterface(void*& ref):
    a_ref(ref){;}
  void setRef(void* ref) { a_ref = ref;}
  void* getRef(){ return a_ref; }  
  virtual void marshal(DssWriteBuffer* bb)=0;
  
  MACRO_NO_DEFAULT_CONSTRUCTORS(GlobalNameInterface);
};


class DSSDLLSPEC KbrCallbackInterface{
public:
  virtual void m_kbrMessage(int key, PstInContainerInterface*)                    = 0;
  virtual PstOutContainerInterface* m_kbrDivideResp(int start, int stop, int n)   = 0; 
  virtual void m_kbrNewResp(int start, int stop, int n, PstInContainerInterface*) = 0; 
  virtual void m_kbrFunctional()                                                  = 0; 
  virtual void m_bcMessage(PstInContainerInterface*)                              = 0; 
};

class DSSDLLSPEC KbrInstance{
public: 
  virtual void m_setCallback(KbrCallbackInterface*)         = 0;
  virtual KbrCallbackInterface* m_getCallback()             = 0;
  virtual KbrResult m_route(int, PstOutContainerInterface*) = 0; 
  virtual KbrResult m_broadcast(PstOutContainerInterface*)  = 0;
  virtual int  m_getId()                                    = 0;
  virtual void m_join()                                     = 0; 
  virtual void m_leave()                                    = 0; 
  virtual void m_marshal(DssWriteBuffer*)                   = 0; 
};

#endif
 
