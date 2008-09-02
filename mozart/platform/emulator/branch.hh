/*
 *  Main authors:
 *     Raphael Collet <raph@info.ucl.ac.be>
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
 *
 *  Contributing authors:
 *     Andres Felipe Barco <anfelbar@univalle.edu.co>
 *     Victor Alfonso Rivera <varivera@puj.edu.co>
 *
 *  Copyright:
 *    Raphael Collet, 2008
 *    Gustavo Gutierrez, 2008
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

#ifndef __GECODE_BRANCH_HH__
#define __GECODE_BRANCH_HH__

#include "var_base.hh"
#include "builtins.hh"
#include "distributor.hh"


template <class View, class Val, class ViewSel, class ValSel>
class GeVarDistributor;

template <class View, class Val, class ViewSel, class ValSel>
class GFDAssignment;


/*
  \brief This class encapsulates the operations that concern to
  specific constraint system variables. For the distributor we need to
  know when consider an OZ_Term to be a bound representation of the
  specific constraint variable, for instance an integer is considered
  a bound FD variable. Also we need a way to create a view from an
  OZ_Term to be able to use gecode provided view selection strategies.
*/
template<class View, class Val>
class VarBasics {
public:
  static View getView(OZ_Term t);
  static bool assigned(OZ_Term t);
  static OZ_Term getValue(Val v);
  static Val getValue(OZ_Term v);
};



template <class View, class Val, class ViewSel, class ValSel>
class GeVarDistributor : public Distributor {
protected:
  /**
     \brief Sinchronization variable. This variable is bound when
     the distributor work is finished. 
  */
  TaggedRef sync;

  /// Vector of variables to distribute
  TaggedRef *vars;

  /// Number of variables to distribute
  int size;

#ifdef DEBUG_CHECK
  Board *home;
  GenericSpace *gs_home;
#endif
  //ViewArray<IntView> xv;

public:

  GeVarDistributor(Board *bb, TaggedRef *vs, int n);

  virtual void finish(void);

  virtual void make_branch(Board *bb, int pos, Val val);

  /*
    \brief As the distributor injects tell operations in the board
    that not take place inmediately, termination is notified by
    binding \a sync to an atom.
  */
  TaggedRef getSync(void);
  
  
  void dispose(void);

  /**
     \brief Commits branching description \a bd in board bb. This
     operation is performed from the search engine. The prepareTell
     operation push the tell on the top of the thread that performs
     propagation. This allows all tell operations to be performed
     *before* the propagation of the gecode space. Also, as a side
     effect, tell operations are lazy, this is, are posted in the
     gecode space when space status is needed.
  */
  virtual int commitBranch(Board *bb, TaggedRef bd);
  
  virtual int notifyStable(Board *bb);
  
  bool status(void);

  /*
    Important: A tell operation involves the post of a propagator in
    the gecode space. The space needs to be *unstable* after a tell
    call to _notify_ mozart that propagation can be performed to
    compute a fix point.
  */
  virtual OZ_Return tell(RefsArray *args);

  virtual Distributor * gCollect(void);
  
  virtual Distributor * sClone(void);

};



template <class View, class Val, class ViewSel, class ValSel>
class GeVarAssignment : public Distributor {
protected:
  /**
     \brief Sinchronization variable. This variable is bound when
     the distributor work is finished. 
  */
  TaggedRef sync;


  /// Vector of variables to distribute
  TaggedRef *vars;

  /// Number of variables to distribute
  int size;

#ifdef DEBUG_CHECK
  Board *home;
  GenericSpace *gs_home;
#endif

public:

  GeVarAssignment(Board *bb, TaggedRef *vs, int n);

  virtual void finish(void);
  
  virtual void make_branch(Board *bb, int pos, Val val);

  /*
    \brief As the distributor injects tell operations in the board
    that not take place inmediately, termination is notified by
    binding \a sync to an atom.
  */
  TaggedRef getSync(void);
  
  
  void dispose(void);

  /**
     \brief Commits branching description \a bd in board bb. This
     operation is performed from the search engine. The prepareTell
     operation push the tell on the top of the thread that performs
     propagation. This allows all tell operations to be performed
     *before* the propagation of the gecode space. Also, as a side
     effect, tell operations are lazy, this is, are posted in the
     gecode space on space status demand.
  */
  virtual int commitBranch(Board *bb, TaggedRef bd);
  
  virtual int notifyStable(Board *bb);
  
  bool status(void);
   
  /*
    Important: A tell operation involves the post of a propagator in
    the gecode space. The space needs to be *unstable* after a tell
    call to _notify_ mozart that propagation can be performed to
    compute a fix point.
  */
  virtual OZ_Return tell(RefsArray *args);

  virtual Distributor * gCollect(void);
  
  virtual Distributor * sClone(void);
};

#include "branch.icc"

#endif
