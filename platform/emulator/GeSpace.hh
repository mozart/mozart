/*
 *  Main authors:
 *     Raphael Collet <raph@info.ucl.ac.be>
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
 *     Alberto Delgado <adelgado@cic.puj.edu.co>
 *
 *  Contributing authors:
 *     Alejandro Arbelaez <aarbelaez@puj.edu.co>
 *
 *  Copyright:
 *     Gustavo Gutierrez, 2006
 *     Alberto Delgado, 2006
 *
 *  Last modified:
 *     $Date$
 *     $Revision$
 *
 *  This file is part of GeOz, a module for integrating gecode 
 *  constraint system to Mozart: 
 *     http://home.gna.org/geoz
 *
 *  See the file "LICENSE" for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */

/**
 * @file   GeSpace.hh
 * 
 * @brief This file contains all classes and operations related to GeSpace's.
 *        A GeSpace is an object in the oz heap pointing to a GenericSpace that is a
 *        a gecode space containing contraint variables and propagators.  
 */


#ifndef __GECODE_SPACE_HH__
#define __GECODE_SPACE_HH__

#include <vector>

#include "misc.hh"
#include "mozart.h"
#include "gecode/kernel.hh"



//-----------------------------------------------------------------------------

/** 
 * \brief GeSpace module initialization function
 * 
 */
//void module_init_space(void);



#include "gecode/int.hh"
#include "gecode/set.hh"


class GenericSpace;


/** 
 * \brief Container for Gecode variables and Mozart heap references. 
 * Each variable has a corresponding heap reference.
 *
 */

class VarRefArray {

private:
  /** 
   * \brief Container for variables.
   * VarBase is used because we need a generic way to store variables of different types in the 
   * space.
   * TODO: test if std::vector is efficient and if not then change it for a proper implementation
   */ 
  std::vector<Gecode::VarBase*> vars; 
  /**
   * \brief Container for references.
   * This vector contains references to variable nodes in the mozart heap. 
   * There is a one-to-one correspondence between a variable in vars and a reference in 
   * references. Position in the vector  is used to achieve this.
   */
  std::vector<OZ_Term> refs;
 
  /**
     Invariant:
     vars.size() = refs.size() 
  */
public:
  /// \name Constructors
  //@{
  /** 
   * \brief Creates a new VarRefArray with a fixed size.
   * The size will be updated automatically as needed.
   * 
   * @param s The space where the variables will be created
   */
  VarRefArray() :
    vars(0), refs(0) {}

  /** 
   * \brief Copy constructor
   * This constructor is used for cloning of boards and must *know* how to clone variables
   * from each constraint system.
   * 
   */
  VarRefArray(Gecode::Space* s, VarRefArray& v, bool share = false);
  //@}

  /** 
   * \brief Creates a new variable.  
   * 
   * @param x A pointer to the corresponding variable implementation
   * @param r A reference to the mozart heap reference corresponding to the variable
   * @return The index of the new allocated variable
   */
  int newVar(Gecode::VarBase *x, OZ_Term r) {
    refs.push_back(r); vars.push_back(x);
  }

  int getSize(void) {
    return vars.size();
  }

  OZ_Term *getRef(int n) { 
    Assert(n >= 0 && n < vars.size());
    return &(refs[n]); 
  }

  Gecode::VarBase& getVar(int n) {
    Assert(n >= 0 && n < vars.size());
    return *vars[n];
  }

  void setRef(int n, OZ_Term t) { 
    Assert(n >= 0 && n < vars.size());
    refs[n] = t; 
  }
};

// Garbage collection related stuff 
// Pontier to the begining of the list of allocated Generic Spaces
static GenericSpace* GeSpaceCollectList = NULL;
// Pointer to the last element in the list of allocated Generic Spaces
static GenericSpace* GeSpaceCollectLast = NULL;
// Total memory allocated in generic spaces
static size_t GeSpaceAllocatedMem = 0;
/* This method adds the generic space gs to the list of allocated generic spaces.
   It must be called for all created generic space in order to put it in the
   control of the garbage collector.
*/
void registerGeSpace(GenericSpace* gs);
/* Collects the memory allocated by unused generic spaces. */
void gCollectGeSpaces(void);

/** 
 * \brief Container for Gecode variables and propagators. A GenericSpace 
 * is a Gecode::Space that contains Gecode variables, which are visible in 
 * Oz as variables.  That space will also contain all the propagators created 
 * at the Oz level.
 */
class GenericSpace : public Gecode::Space {
  friend void gCollectGeSpaces(void);
  friend void registerGeSpace(GenericSpace*);
  friend class Board;
private:
  Board *board;  // pointer to the mozart space 

  /// Variable containers
  VarRefArray vars;     /// Array container for variables

  
  /** Space stability is used to reflect the fact that there could be 
      pending work to do. It is for example, a new propagator was posted
      in the space but no call to status() has been performed so far. Other
      things that can make a space to become unstable are unification or 
      bindings events from mozart.
  */
  TaggedRef trigger; /// Reflect space stability

  /* Invariant: trigger is an unbound read-only while the GenericSpace
     remains stable.  It is bound to unit as soon as the space becomes
     unstable and needs to be run.  An Oz thread suspending on trigger
     will therefore be woken up when the space becomes unstable.
  */

  /*
   * Garbage collection:
   * Generic spaces are stored outside the mozart heap. In order to collect 
   * memory, every time a GenericSpace is created it is added to 
   * GeSpaceCollectList. This list is travesed by garbage collector to free
   * memory allocated by unused gespaces. A double linked list is used for that
   * purpose. gc_marked reflects whether the object is in use or not.
   */
  GenericSpace* gc_pred;    // predecessor in list
  GenericSpace* gc_succ;    // successor in list
  //Garbage Collection
  bool gc_marked;
  
  // Last amount of memory computed for this space and its structures.
  size_t allocatedMemory;
  
  /**
     \brief Computes the memory allocated by this space.
  */
  size_t usedMem(void) {
    return allocated() + cached() + sizeof(*this);
  }
  

  void makeStable(void);     // assigns trigger into a read-only

  /**
   * \brief This variable is used to count how many variables are 
   * determined. This criteria is used to know when a space becomes 
   * solved (i.e. determined = vars.size. A space is solved 
   * when all the variables in the space become determined. 
   */
  unsigned int determined;

  /**
   * \brief This variable is to count how many foreign propagators 
   * have been posted in the space. Foreign propagators include domain
   * reflection propagators.
   */
  unsigned int foreignProps;


  /**
   * \brief This variable is to count how many unification propagators 
   * have been posted in the space. 
   */
  unsigned int unifyProps;

  static unsigned long int unused_uli;

  /// \name Reference access
  //@{  
  /** 
   * \brief Set the reference to the corresponding variable in Mozart
   * 
   * @param n Variable index 
   * @param OZ_Term Heap reference
   */
  void setVarRef(int n,OZ_Term);

public:
  
  static int gscounter;

/// \name Constructors
  //@{
  /// Default constructor
  GenericSpace(Board* b);
  /// Copy constructor
  explicit GenericSpace(GenericSpace& s, bool share = false);
  //@}

/// \name Destructor
  //@{
  /**
   * \brief This destructor is called by the garbage collector to free
   * the memory allocated by an unused object. The object is also deleted 
   * when a space becomes solved or failed.
   */
  ~GenericSpace(void);
  //@}

  /** 
   * \brief copy operation used for space clonning.
   * 
   * @param share Whether to share data structures inside the space
   * 
   * @return A clone of the space
   */
  virtual Space* copy(bool share) {
    printf("GeSpace.hh copy\n");fflush(stdout);
    return new GenericSpace(*this,share);
  }

  void makeUnstable(void);   // binds current trigger to unit

  // return current trigger
  TaggedRef getTrigger(void) { return trigger; }

  bool isSolved(void) { 
    return determined == (vars.getSize()); }
  void incDetermined(void) { determined++; }

  /**
     \brief Returns whether the generic space is stable or not.
     
  */
  bool isStable();

  bool isEntailed(void) { 
    Assert(isStable());
    int nondet = vars.getSize() - determined;
    /*    printf("Nondet: %d\n",nondet);fflush(stdout);
    printf("foreignProps: %d\n",foreignProps);fflush(stdout);
    printf("Propagators: %d\n",propagators());fflush(stdout);
    printf("Propagators: %d\n",unifyProps);fflush(stdout);*/
    return (nondet + foreignProps + unifyProps/2) == propagators();
  }

  void incForeignProps(void) { foreignProps++; }
  void decForeignProps(int d = 1) { foreignProps -= d; }

  void incUnifyProps(void) { unifyProps+=2; }
  void decUnifyProps(int d = 1) { unifyProps -= d; }
 

  /// \name Variable creation
  //@{
  /** 
   * \brief Creates a new variable inside the space
   * 
   * @param v A pointer to the var implementation
   * @param r A reference to the mozart heap node corresponding to the variable
   * 
   * @return The index of the new variable
   */
  int newVar(Gecode::VarBase *v, OZ_Term r);

  int getVarsSize(void) {return vars.getSize();}

  //@}


  /// \name Variable access
  //@{
  /** 
   * \brief Access the IntVar at a given index
   * 
   * @param n Index in the IntVarArray
   * @return A reference to the variable
   */

  Gecode::VarBase* getVar(int n);

  /**
   * \brief Access IntVar at a given index for 
   * information (read only) purposes
   *
   * @param n Index in the IntVarArray
   * @return A reference to the variable
   *
   */

  Gecode::VarBase* getVarInfo(int n);

  //@}

  /** 
   * \brief Get the reference to the heap of the corresponding variable
   * 
   * @param n Variable index 
   * @return reference to the mozart heap 
   */
  OZ_Term getVarRef(int n) { return *vars.getRef(n); }
  //@}

  /// Space stability information
  //@{
  /**
     \brief Mark the space as unstable
  */
  void setBoard(Board* b);
  //@}

  /** 
      \brief Wraps the status function of the Space class to add stability attribute support.
  */
  Gecode::SpaceStatus mstatus(unsigned long int& pn=unused_uli);
  

  /// Garbage collection and space cloning for references
  //@{
  /** 
   * \brief Update references to the mozart heap
   * 
   */
  void gCollect();
  void sClone();
  //@}

};

#endif /* __GESPACE_HH__ */
