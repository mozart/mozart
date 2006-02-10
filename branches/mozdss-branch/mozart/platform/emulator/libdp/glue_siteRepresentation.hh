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
  int a_ipAddress; 
  int a_portNum; 
  DSite *a_dssSite;
  Glue_SiteRep *a_next; 
  OZ_Term a_ozSite; 
  OZ_Term a_conThreadVar; 
  char *a_RId;  // readable id of the site
public:
  Glue_SiteRep(int ip, int port, DSite*, OZ_Term);
  ~Glue_SiteRep() { 
    if (a_RId != NULL)
      free(a_RId); 
  }
  void dispose(){
    a_ozSite = (OZ_Term) 0;
  }
public:
  int getIpNum() {return a_ipAddress;}
  int getPortNum() {return a_portNum;}
  
  void m_marshal(DssWriteBuffer*);
  char* m_stringrep();
  
  void m_updateAddress(DssReadBuffer* const buf);
  
  DSite* m_getDssSite();
  void m_setDssSite(DSite*);
  void m_setConnection(VirtualChannelInterface* vc);
  void m_monitorConnection();     // install a rtt monitor

  Glue_SiteRep *m_getNext();
  Glue_SiteRep **m_getNextPP() {return &a_next;}
  
  void m_storeConnectionThread( OZ_Term v);
  void m_gc();
  OZ_Term m_getOzSite(); 
  OZ_Term m_getInfo();
  const char *m_getRId() { return a_RId; }
  void m_setRId(char* RId);
  void m_showRId() { printf("site RId:%s\n", a_RId); }
public:
  virtual void    marshalCsSite( DssWriteBuffer* const buf);
  virtual void    updateCsSite( DssReadBuffer* const buf); 
  virtual void    disposeCsSite(); 
  virtual void    monitor(); 
  virtual void    reportRtViolation(int measuredRT, int installedLow,
				    int installedHigh); 
  virtual VirtualChannelInterface *establishConnection();
  virtual void closeConnection( VirtualChannelInterface* con);
};


extern Glue_SiteRep* site_address_representations; 

extern Glue_SiteRep* thisGSite; // the gsa proper to the current process 

void gCollectGASreps(); 


void putInt(DssWriteBuffer *buf, int i);
int  getInt(DssReadBuffer *buf);
void putStr(DssWriteBuffer *buf, char *str, int len);
void getStr(DssReadBuffer *buf, char *str, int len);
void cleanStr(DssReadBuffer *buf, int len);



#endif
