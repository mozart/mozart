

#if defined(INTERFACE)
#pragma implementation "msl_interRouter.hh"
#endif

#include <stdlib.h>

#include "msl_interRouter.hh"
#include "msl_dsite.hh"
#include "mslBase.hh"
#include "msl_timers.hh"
#define ROUTE_TIMEOUT    3 * 20000 // 3*m_getEnvironment()->a_dssconf.dpProbeTimeout

namespace _msl_internal{

// function triggered when the route timer expired 
static unsigned int if_interRouter_routeTimerExpired(void *timerParam) {
  TimerParam *tp =  static_cast<TimerParam *>(timerParam);
  
  //printf ("routeTimer expired routeId:%d\n", (tp->a_route)->routeId);
  (tp->a_irouter)->unregisterRoute(tp->a_route);

  return false; /* do not call it again */
}

  Route::Route(InterRouter* master,  MsgnLayerEnv* env) : 
    a_mslEnv(env),  nxtRoute(NULL), srcSiteId(NULL), dstSiteId(NULL),
    routeId(0), fromSite(NULL), toSite(NULL), routeState(0), a_master(master),
    a_routeTimer(NULL), a_timerParam(NULL){
    a_timerParam = new TimerParam(this, a_master);
}

  Route::~Route() {
  delete a_timerParam;
  a_mslEnv->a_timers->clearTimer(a_routeTimer);
}

  void Route::setRouteTimer() {
    a_mslEnv->a_timers->setTimer(a_routeTimer, ROUTE_TIMEOUT, 
				 if_interRouter_routeTimerExpired, static_cast<void *>(a_timerParam));
    //printf("set routeTimer for routeId:%d\n", routeId);
  }

  void Route::clearRouteTimer() {
  a_mslEnv->a_timers->clearTimer(a_routeTimer);
}

  InterRouter::InterRouter(MsgnLayerEnv* env) : 
  a_mslEnv(env), a_routeLst(NULL) {;}

  InterRouter::~InterRouter() {
    while (a_routeLst != NULL) {
      Route* tmpRoute = a_routeLst;
      a_routeLst = a_routeLst->nxtRoute;
      delete tmpRoute;
    }
  }
  
bool 
InterRouter::registerRoute(Site* srcSiteId, Site* dstSiteId, int routeId,
			   Site* fromSite, Site* toSite) {
  Route* newRoute = new Route(this, a_mslEnv);
  newRoute->nxtRoute = a_routeLst;
  a_routeLst = newRoute;

  newRoute->srcSiteId = srcSiteId;
  newRoute->dstSiteId = dstSiteId;
  newRoute->fromSite = fromSite;
  newRoute->toSite = toSite;
  newRoute->routeId = routeId;

  //printf("register routeid:%d src:%p dst:%p between sites from:%p to:%p\n", routeId, srcSiteId, dstSiteId, fromSite, toSite);

  newRoute->setRouteTimer();

  return true; //!! for the moment, return always true 
}

void 
InterRouter::unregisterRoute(Site* srcSiteId, Site* dstSiteId, int routeId) {
  Route* tmpRoute = a_routeLst;

  // check the head
  if (tmpRoute != NULL) {
     if ((routeId == tmpRoute->routeId) && (srcSiteId == tmpRoute->srcSiteId) &&
	(dstSiteId == tmpRoute->dstSiteId)) {
       a_routeLst = tmpRoute->nxtRoute;
       //printf("unregister routeId:%d\n", tmpRoute->routeId);
       delete tmpRoute;
       return;
     }
  }
  else { return; }

  // check in the tail
  while (tmpRoute->nxtRoute != NULL) {
    Route* nxttmpRoute = tmpRoute->nxtRoute;

    if ((routeId == nxttmpRoute->routeId) && 
	(srcSiteId == nxttmpRoute->srcSiteId) &&
	(dstSiteId == nxttmpRoute->dstSiteId)) {
      tmpRoute->nxtRoute = nxttmpRoute->nxtRoute;
      //printf("unregister routeId:%d\n", nxttmpRoute->routeId);
      delete nxttmpRoute;
      return;
    }

    tmpRoute = tmpRoute->nxtRoute;
  }
}

Site *
InterRouter::getRouteSite(Site* srcSiteId, Site* dstSiteId, int routeId){
  Route* tmpRoute = a_routeLst;

  //printf("get routeid:%d src:%p dst:%p\n", routeId, srcSiteId, dstSiteId);
  
  while (tmpRoute != NULL) {
    if (routeId == tmpRoute->routeId) {
      if ((srcSiteId == tmpRoute->srcSiteId) &&
	  (dstSiteId == tmpRoute->dstSiteId)) {
	tmpRoute->setRouteTimer();
	return tmpRoute->toSite;
      }
      else if ((dstSiteId == tmpRoute->srcSiteId) &&
	       (srcSiteId == tmpRoute->dstSiteId)) {
	//tmpRoute->setRouteTimer(); //! does the passive end maintain the route? 
	return tmpRoute->fromSite;
      }
    }
    tmpRoute = tmpRoute->nxtRoute;
  }
  
  return NULL;
}
  
} //End namespace





