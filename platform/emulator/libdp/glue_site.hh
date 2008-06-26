/*
 *  Authors:
 *    Erik Klintskog (erik@sics.se)
 *
 *  Contributors:
 *    Raphael Collet (raph@info.ucl.ac.be)
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

/*
  The classes GlueSite and OzSite provide objects to interface DSite
  objects from the DSS.  OzSite objects are Oz chunks, they simply
  reflect DSS sites in the language.  GlueSite implements the
  interface CsSiteInterface for the DSS, and provides specific data to
  connect to the site.  GlueSite also implements a tempFail failure
  detector, and maintains a fault stream for each site.

                             OzSite    FaultStream
                                 ^      ^
                                 |      |
                                 v      |
                                 GlueSite (CsSiteInterface)
                                    ^
                                    |
                                    v
                                  DSite

  - TempFail detection: GlueSite implements an adaptive timeout for
  the round-trip times measured in the DSS.  It maintains a round-trip
  time average and median, and modifies the DSS timeout accordingly.
  This technique relieves the user from providing an ad hoc timeout
  value.

  - Memory management: when marked, an OzSite forces its corresponding
  GlueSite to mark its DSite in the DSS.  The DSS also marks the
  DSites it needs, then deletes the unused DSites and dispose their
  corresponding GlueSite.  By design, the disposed GlueSites are no
  longer referenced from anywhere.  A GlueSite is free to drop its
  corresponding OzSite if the latter is not marked by the garbage
  collector; a new OzSite can be created when needed (without any
  visible inconsistency).

 */

class GlueSite : public CsSiteInterface {
private:
  DSite*  dsite;           // the site's corresponding DSite
  OZ_Term ozsite;          // the site's OzSite
  OZ_Term faultstream;     // the site's fault stream

  GlueSite* next;          // GlueSite form a linked list
  bool      gcmarked:1;    // flag set by OzSite
  bool      disposed:1;    // flag set when GlueSite must be deleted

  OZ_Term info;            // site info (a ByteString)

  int rtt_avg;         // average rtt
  int rtt_mdev;        // median deviation of rtt
  int rtt_timeout;     // adaptive timeout for detecting tempFail

public:
  GlueSite(DSite*);
  ~GlueSite();

  // get DSite/OzSite
  DSite* getDSite() const { return dsite; }
  void setDSite(DSite* s) { dsite = s; }
  OZ_Term getOzSite();

  OZ_Term getFaultState();
  OZ_Term getFaultStream();

  GlueSite* getNext() const { return next; }
  GlueSite** getNextPtr() { return &next; }
  bool isDisposed() const { return disposed; }

  // glue-specific information
  OZ_Term getInfo() { return info; }
  void    setInfo(OZ_Term);

  // channels
  void m_setConnection(DssChannel* vc);

  // gc
  void m_gcRoots();
  void m_gcMark() { gcmarked = true; }
  bool m_isMarked() { return gcmarked; }
  void m_gcFinal();

  // CsSiteInterface
  virtual int  getCsSiteSize();
  virtual void marshalCsSite( DssWriteBuffer* const buf);
  virtual void updateCsSite( DssReadBuffer* const buf);
  virtual void disposeCsSite();

  virtual void working();
  virtual void reportRTT(int);
  virtual void reportTimeout(int);

  virtual void reportFaultState(FaultState);

  virtual DssChannel *establishConnection();
  virtual void receivedMsg(MsgContainer*);
};



extern GlueSite* thisGSite;

GlueSite* getGlueSites();     // to iterate on them

void gcGlueSiteRoots();
void gcGlueSiteFinal();



void putInt(DssWriteBuffer *buf, int i);
int  getInt(DssReadBuffer *buf);
void putStr(DssWriteBuffer *buf, char *str, int len);
void getStr(DssReadBuffer *buf, char *str, int len);
void cleanStr(DssReadBuffer *buf, int len);



#define OZ_E_SITE  4211

class OzSite: public OZ_Extension {
private:
  GlueSite *gsite;

public:
  OzSite(GlueSite* gs) : gsite(gs) {}

  GlueSite* getGlueSite() const { return gsite; }

  virtual int           getIdV(void);

  virtual OZ_Extension* gCollectV(void);
  virtual void          gCollectRecurseV(void);
  virtual OZ_Extension* sCloneV(void);
  virtual void          sCloneRecurseV(void);

  virtual OZ_Term       printV(int = 10) ;
  virtual OZ_Term       printLongV(int depth = 10, int offset = 0);
  virtual OZ_Term       typeV(void);
  virtual OZ_Boolean    isChunkV(void) { return OZ_TRUE; }
  virtual OZ_Term       getFeatureV(OZ_Term);
  virtual OZ_Return     getFeatureV(OZ_Term, OZ_Term&);
  virtual OZ_Return     putFeatureV(OZ_Term, OZ_Term);
  //
  virtual OZ_Boolean    toBePickledV() { return (OZ_TRUE); }
  virtual void          pickleV(MarshalerBuffer *mb, GenTraverser *gt);
  virtual OZ_Boolean    toBeMarshaledV() { return (OZ_TRUE); }
  virtual void          marshalSuspV(OZ_Term te,
                                     ByteBuffer *bs, GenTraverser *gt);
  virtual int           minNeededSpace();
};

void OzSite_init();

inline
Bool oz_isOzSite(TaggedRef ref) {
  return OZ_isExtension(ref) && OZ_getExtension(ref)->getIdV() == OZ_E_SITE;
}

inline
OzSite* tagged2OzSite(TaggedRef ref) {
  Assert(oz_isOzSite(ref));
  return static_cast<OzSite*>(OZ_getExtension(ref));
}

inline
GlueSite* ozSite2GlueSite(TaggedRef ref) {
  return tagged2OzSite(ref)->getGlueSite();
}

inline
DSite* ozSite2DSite(TaggedRef ref) {
  return ozSite2GlueSite(ref)->getDSite();
}

#endif
