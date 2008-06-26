/*
 *  Authors:
 *    Erik Klintskog (erikd@sics.se)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
 *    Erik Klintskog, 1998
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
#ifndef __DGC_TL_HH
#define __DGC_TL_HH

#ifdef INTERFACE
#pragma interface
#endif

#include "dgc.hh"

namespace _dss_internal{ // Start namespace
    class ::TimerElementInterface;
  //


  const int LEAST_PERIOD = 1000;
  // ******************** TL HOME ***********************

  class TL_Home:public HomeGCalgorithm{
  private:
    int           a_periodTime;
    DSS_LongTime  a_expireDate;
    inline void extend_period(const int& lease_ms);

    TL_Home(const TL_Home&):
      HomeGCalgorithm(NULL,NULL,RC_ALG_PERSIST),
      a_periodTime(0),a_expireDate(DSS_LongTime()){}
    TL_Home& operator=(const TL_Home&){ return *this; }

  public:
    inline int getPeriod(){ return a_periodTime; }

    TL_Home(HomeReference* const p, GCalgorithm* const g,
            const int& period);
    virtual ~TL_Home();

    bool m_isRoot();
    void m_getReferenceInfo(DssWriteBuffer *bs, DSite* dest);
    int  m_getReferenceSize() const { return sz_MNumberMax; }
    void m_getCtlMsg(DSite* msite, MsgContainer* msg);

    // val must be longer than until the announced lease has
    // expired (otherwise we fool the remotes)
    bool setLeasePeriod(const int& val);
  };


  // ******************* TL REMOTE *********************

  class TL_Remote:public RemoteGCalgorithm{
  private:
    int           a_periodTime;
    DSS_LongTime  a_expireDate;
    TimerElementInterface *a_timer;

    inline void setTimer(const int& period);
    inline void extend_period(const int& lease_ms);

    TL_Remote(const TL_Remote&):
      RemoteGCalgorithm(NULL,NULL,RC_ALG_PERSIST),
      a_periodTime(0), a_expireDate(DSS_LongTime()), a_timer(NULL){}
    TL_Remote& operator=(const TL_Remote&){ return *this; }


  public:
    inline int getPeriod(){ return a_periodTime; }
    unsigned int updateTimerExpired();

    TL_Remote(RemoteReference* const p, DssReadBuffer *bs,
              GCalgorithm* const g, const int& period);
    virtual ~TL_Remote();

    bool m_isRoot();
    void m_mergeReferenceInfo(DssReadBuffer *bs);
    void m_dropReference();
    void m_getReferenceInfo(DssWriteBuffer *bs, DSite* dest);
    int  m_getReferenceSize() const { return sz_MNumberMax; }
    void m_getCtlMsg(DSite* msite, MsgContainer* msg);

    // val must be shorter than the expire date (otherwise we
    // fool purself by not requesting an update), we know that a timer
    // will go off or has already done so so don't install a new one
    bool setUpdatePeriod(const int& val);
  };


}

#endif
