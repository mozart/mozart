#ifndef __INTERROUTER_HH
#define __INTERROUTER_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "mslBase.hh"

namespace _msl_internal{ //Start namespace

class ComObj;
class TimerElement;
class TimerParam;  // defined later in this file

// This represents a route state for each src-dst pair, to be stored at
// the InterRouter. It is a soft state guarded by a_routeTimer.
  class Route  {
  public:
    MsgnLayerEnv *a_mslEnv;

    Route* nxtRoute;

    Site* srcSiteId;  // the id of the site source
    Site* dstSiteId;  // the id of the site destination
    int    routeId;


  // the two sites to be connected in order to form the bootstrap
    Site* fromSite;
    Site* toSite;

    int    routeState; //!! to be completed

    InterRouter *a_master;  // the guy who manages the route
    TimerElement *a_routeTimer;
    TimerParam *a_timerParam;

    Route(InterRouter* master, MsgnLayerEnv*);

    ~Route();

  // Set the timer for this route. Cancel the old timer (if there is one).
  // Called each time a message uses the route.
    void setRouteTimer();

    // clear the timer for this route
    void clearRouteTimer();

  private:
    Route(const Route&):
      a_mslEnv(NULL), nxtRoute(NULL), srcSiteId(NULL),
      dstSiteId(NULL), routeId(0), fromSite(NULL), toSite(NULL), routeState(0),
      a_master(NULL), a_routeTimer(NULL),  a_timerParam(NULL){}
    Route& operator=(const Route&){ return *this; }
  };


//
// Class used globally as a routing table.
//
class InterRouter  {

  MsgnLayerEnv *a_mslEnv;
  Route *a_routeLst;  // list of routes

private:
  InterRouter(const InterRouter&):a_mslEnv(NULL), a_routeLst(NULL){};
  InterRouter& operator=(const InterRouter&){ return *this; }

public:

  InterRouter(MsgnLayerEnv*);

  ~InterRouter();

  // register this route
  bool registerRoute(Site* srcSiteId, Site* dstSiteId, int routeId,
                     Site* fromSite, Site* toSite);

  //  unregister this route
  void unregisterRoute(Site* srcSiteId, Site* dstSiteId, int routeId);

  void unregisterRoute(Route *route) {
    unregisterRoute(route->srcSiteId, route->dstSiteId, route->routeId);
  }

  // get the site for the next router
  Site *getRouteSite(Site* srcSiteId, Site* dstSiteId, int routeId);

};


// This is the parameter to be passed when setting a route timer.
class TimerParam {
public:
  Route *a_route;
  InterRouter *a_irouter;

  TimerParam(Route *route, InterRouter *irouter):a_route(route), a_irouter(irouter){ }

  ~TimerParam() {;}

private:
  TimerParam(const TimerParam&):a_route(NULL), a_irouter(NULL){}
  TimerParam& operator=(const TimerParam&){ return *this; }

};

} //End namespace
#endif
