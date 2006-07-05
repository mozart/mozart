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

#ifndef __DSS_OBJECT_HH
#define __DSS_OBJECT_HH

#ifdef INTERFACE  
#pragma interface
#endif

#include "dss_enums.hh"
#include "dss_classes.hh"
#include "dss_comService.hh"

// Make things visible outside the dll for windows
// DSS_EXPORTING should be defined by DSS-dll compilation
#ifndef WIN32
#define DSSDLLSPEC
#else
#ifdef DSS_EXPORTING
#define DSSDLLSPEC __declspec(dllexport)
#else
#define DSSDLLSPEC __declspec(dllimport)
#endif
#endif

// ***************************** DEFINES, FLAGS & DEBUG **************************
//
// There are several compiler flags used in the DSS which triggers different behavior
// by configuring the desired build with different flags
//
// The available flags are:
// - DEBUG_CHECK, trigger asserts.
// - DSS_LOG, enables the logging utility
// - INTERFACE, enables pragma directives when debug compiling ("no inline"-ing)
// - EXCEPTIONS, enables exceptions.


// *************** Imported by the DSS from the MAP ***********************
//
// 
//

namespace _dss_internal{
  class DSS_Environment;
}

class DSSDLLSPEC DSS_Object{
private:
  _dss_internal::DSS_Environment* _a_env;

public:
  DSS_Object(IoFactoryInterface* const io, ComServiceInterface*  const sa, Mediation_Object* const mo, const bool& sec_channel = false);
  virtual ~DSS_Object();

  // create/unmarshal a coordination proxy
  CoordinatorAssistant* createProxy(const ProtocolName&,
				    const AccessArchitecture&,
				    const RCalg&);

  CoordinatorAssistant* unmarshalProxy(DssReadBuffer* const,
				       const ProxyUnmarshalFlag&,
				       AbstractEntityName&);

  DssThreadId* m_createDssThreadId();
  
  
  // returns true if first can be said to be (globally) logically
  // ordered before second.
  bool m_orderEntities(AbstractEntity* const ae_first,
		       AbstractEntity* const ae_second);

  // Periodically invoke to clean up internal DSS constructs
  void gcDssResources();
  

  ParamRetVal operateIntParam(const DSS_AREA&    area,
			      const DSS_AREA_ID& id,
			      const int&         param,
			      int&         arg);
  
  ParamRetVal operateStrParam(const DSS_AREA&    area,
			      const DSS_AREA_ID& id,
			      const int&         param,
			      const char* const  str);


  // KBR-Interface
  


  GlobalNameInterface* getName(void *);
  GlobalNameInterface* findName(GlobalNameInterface*);
  void addName(GlobalNameInterface *);
  GlobalNameInterface* unmarshalName(DssReadBuffer*);  

  // ************** KBR management ********************'
  KbrInstance* m_createKbr(int K, int Bits, int Fail, KbrCallbackInterface*);
  bool  m_unmarshalKbr(DssReadBuffer* buf, KbrInstance*&); 

  
  // ************** Backbone management *****************
  void m_createBackboneTicket(DssWriteBuffer* buf);
  void m_joinBackbone(DssReadBuffer *buf);
  
  MACRO_NO_DEFAULT_CONSTRUCTORS(DSS_Object);
};

#endif
