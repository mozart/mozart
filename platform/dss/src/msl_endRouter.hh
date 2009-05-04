/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
 * 
 *  Copyright:
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

#ifndef __ENDROUTE_HH
#define __ENDROUTE_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "msl_transObj.hh"
#include "msl_buffer.hh"
#include "msl_dct.hh"

namespace _msl_internal{ //Start namespace 

  class RouteTransController;
  class EndRouterDeliver;

  class EndRouter: public BufferedTransObj {
    friend class RouteTransController;

  private:
    ComObj *a_succ;  // the successor ComObj
    int a_routeId;   // the route Id

    EndRouterDeliver* deliverEvent; 

    EndRouter(const EndRouter&) : 
      BufferedTransObj(0,NULL),
      a_succ(NULL), a_routeId(0), deliverEvent(NULL) {}
    EndRouter& operator=(const EndRouter&){ return *this; }

  public:
    EndRouter(MsgnLayerEnv*); 
    virtual ~EndRouter() {}

    virtual void m_closeConnection();
    virtual void deliver();
    virtual void readyToReceive();

    // Set the next comObj to communicate through
    void setSuccessor(ComObj *succ) { a_succ = succ; }
    void setRouteId(int routeId) { a_routeId = routeId; }

    // Init the route set up procedure, by sending C_SET_ROUTE
    void initRouteSetUp(DSite *succ[], int nrSites);

    void writeHandler();

    // Read handler for transparent DAC messages.
    void readHandler(DssSimpleDacDct *dac);
    
    // Called by the succ ComObj when receiving C_TARGET_TOUCHED.
    // It specifies that the route routeId was set up.
    // !!!to be continued ...
    void routeSetUp(int routeId);

    virtual TransMedium getTransportMedium() {return TM_ROUTE;}
  };

} //End namespace
#endif
