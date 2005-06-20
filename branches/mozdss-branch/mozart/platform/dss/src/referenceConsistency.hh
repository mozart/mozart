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


#ifndef __REFERENCE_CONCISTENCY_HH
#define __REFERENCE_CONCISTENCY_HH

#ifdef INTERFACE  
#pragma interface
#endif

#include "dssBase.hh"
#include "dss_msgLayerInterface.hh"
#include "dgc.hh"

namespace _dss_internal{ // Start namespace
  
  class RemoteReference;
  class HomeReference;
  class Coordinator; 
  class Proxy; 
  
  
  

  class Reference{
  public:
    GCalgorithm* a_algs;

  public:
    //Initialize
    Reference(): a_algs(NULL) {;}
    virtual ~Reference(){ m_removeAlgs(); a_algs = NULL;};
    
    inline GCalgorithm *m_findAlg(const RCalg& alg) const {
      GCalgorithm *tmp = a_algs;
      for(; tmp->a_type != alg; tmp = tmp->a_next);
      return tmp;
    }
  
    
    // ******************* MISC *************************
    unsigned int m_getAlgorithms(); // returns a bitvector of all algorithms
    void m_removeAlgs();
    inline int  m_getNoOfAlgs() const {
      int i = 0; for(GCalgorithm *tmp = a_algs;tmp != NULL ; tmp = tmp->a_next) i++; return i;
    }
    

    void m_makePersistent() { m_removeAlgs(); }
    bool m_isPersistent()   { return (a_algs == NULL); }
    bool m_removeAlgorithmType(const RCalg& atype);
    static void sf_cleanType(const RCalg& type, DssReadBuffer* bs);
    
    RCalg m_msgToGcAlg(MsgContainer *msg, DSite* from);
    
    void m_dropReference();

    void m_getReferenceInfo(DssWriteBuffer *bs, DSite* dest);
    
    virtual void m_mergeReferenceInfo(DssReadBuffer* bs) = 0; 
    virtual void m_makeGCpreps();
    MACRO_NO_DEFAULT_CONSTRUCTORS(Reference);
  };
 
    

  class HomeReference : public Reference{
    friend class Coordinator;
  public:
#ifdef DEBUG_CHECK
    static int a_allocated;
#endif
    Coordinator *a_coordinator; 
  public:
    bool m_isRoot(); // H: one or more non-roots: true R: one or more roots: false
    void m_mergeReferenceInfo(DssReadBuffer* bs);

    HomeReference(Coordinator *c, const unsigned int& gc_annot);
    virtual ~HomeReference(){ DebugCode(a_allocated--);};
    
    // ************ METHODS *************
    void m_mergeAlgorithms(DssReadBuffer *) { ;}

    bool m_manipulateRC(const RCalg& alg, const RCop& op, opaque& data);

    virtual char *m_stringrep();

    DSS_Environment* m_getEnvironment() const ; 

    MACRO_NO_DEFAULT_CONSTRUCTORS(HomeReference);
  };
  
  
  class RemoteReference : public Reference{
  public:
#ifdef DEBUG_CHECK
    static int a_allocated;
#endif
    Proxy *a_proxy; 
    
    RemoteReference(Proxy *a, DssReadBuffer *bs);
    RemoteReference(Proxy *a);
    virtual ~RemoteReference(){ DebugCode(a_allocated--);};

    void m_mergeAlgorithms(DssReadBuffer *bs);
    
    bool m_isRoot();
    void m_mergeReferenceInfo(DssReadBuffer* bs);
    void m_buildAlgorithms(DssReadBuffer *bs);
    
    bool m_manipulateRC(const RCalg& alg, const RCop& op, opaque& data);

    virtual char *m_stringrep();



    DSS_Environment* m_getEnvironment() const ; 
  
    MACRO_NO_DEFAULT_CONSTRUCTORS(RemoteReference);
  };


}//End namespace
#endif // __REFERENCE_CONCISTENCY_HH
