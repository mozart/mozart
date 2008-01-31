/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 * 
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 * 
 *  Copyright:
 *    Erik Klintskog, 2002
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


#ifndef __GLUE_SITE_HH
#define __GLUE_SITE_HH

#include "dss_object.hh"
#include "base.hh"
#include "value.hh"

class Glue_SiteRep:public CsSiteInterface{
private:
  int a_ipAddress; // In network byte order
  int a_portNum; 
  int a_idNum; // ip+port+id identifies the site.
  DSite *a_dssSite;
  Glue_SiteRep *a_next; 
  OZ_Term a_ozSite; 
public:
  Glue_SiteRep(int ip, int port, int id, DSite*, OZ_Term);
  ~Glue_SiteRep() {}
  void dispose(){
    a_ozSite = (OZ_Term) 0;
  }
public:
  int getIpNum() {return a_ipAddress;}
  int getPortNum() {return a_portNum;}
  int getIdNum() {return a_idNum;}
  
  DSite* m_getDssSite() {return a_dssSite;}
  void m_setDssSite(DSite* sa) {a_dssSite = sa;}
  void m_setConnection(DssChannel* vc);

  Glue_SiteRep *m_getNext() {return a_next;}
  Glue_SiteRep **m_getNextPP() {return &a_next;}
  
  void m_gc();
  OZ_Term m_getOzSite(){return a_ozSite;}
  OZ_Term m_getInfo();
public:
  virtual void    marshalCsSite( DssWriteBuffer* const buf);
  virtual void    updateCsSite( DssReadBuffer* const buf); 
  virtual void    disposeCsSite(); 
  virtual void    monitor(); 
  virtual void    reportRtViolation(int measuredRT, int installedLow,
				    int installedHigh); 
  virtual DssChannel *establishConnection();
  virtual void closeConnection(DssChannel* con);
};


extern Glue_SiteRep* site_address_representations; 

extern Glue_SiteRep* thisGSite; // the gsa proper to the current process 

extern int RTT_UPPERBOUND;     // the maximum rtt used to detect tempFail

void gCollectGASreps(); 


void putInt(DssWriteBuffer *buf, int i);
int  getInt(DssReadBuffer *buf);
void putStr(DssWriteBuffer *buf, char *str, int len);
void getStr(DssReadBuffer *buf, char *str, int len);
void cleanStr(DssReadBuffer *buf, int len);



#endif
