/*
 *  Main authors:
 *     Raphael Collet <raph@info.ucl.ac.be>
 *     Gustavo Gutierrez <ggutierrez@cic.puj.edu.co>
 *     Alberto Delgado <adelgado@cic.puj.edu.co> 
 *     Alejandro Arbelaez (aarbelaez@cic.puj.edu.co)
 *
 *  Contributing authors:
 *
 *  Copyright:
 *    Alberto Delgado, 2006-2007
 *    Alejandro Arbelaez, 2006-2007
 *    Gustavo Gutierrez, 2006-2007
 *    Raphael Collet, 2006-2007
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

/**
 * @file   GeSpace.hh
 * 
 * @brief This file contains all classes and operations related to
 *        GeSpace's.  A GeSpace is an object in the oz heap pointing
 *        to a GenericSpace that is a a gecode space containing
 *        contraint variables and propagators.
 */


#ifndef __GECODE_SPACE_HH__
#define __GECODE_SPACE_HH__

#include <vector>

#include "misc.hh"
#include "mozart.h"
#include "gecode/kernel.hh"
#include "gecode/serialization.hh"


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

  /// Number of variables currently stored
  int size;

  /** 
   * \brief Container for variables.  VarImpBase is used because we
   * need a generic way to store variables of different types in the
   * space.  TODO: test if std::vector is efficient and if not then
   * change it for a proper implementation
   */ 
  Gecode::Support::DynamicArray<Gecode::VarImpBase*> vars; 
  /**
   * \brief Container for references.  This vector contains references
   * to variable nodes in the mozart heap.  There is a one-to-one
   * correspondence between a variable in vars and a reference in
   * references. Position in the vector is used to achieve this.
   */
  Gecode::Support::DynamicArray<OZ_Term> refs;
 
  /**
   * \brief Root variables.  Distinction between variables and root
   * variables is needed when merging spaces.  This array stores the
   * index of variables at vars that are part of root.  TODO: replace
   * int by VarImpBase*
   */
  // TODO: remove root vars array if not needed
  //Gecode::Support::DynamicArray<int> root;
  
  /**
     Invariant:
     vars.size() = refs.size() 
     root.size() <= refs.size()
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
    vars(0), refs(0), size(0) {}

  /** 
   * \brief Copy constructor This constructor is used for cloning of
   * boards and must *know* how to clone variables from each
   * constraint system.
   * 
   */
  VarRefArray(Gecode::Space* s, VarRefArray& v, bool share = false);
  //@}

  /** 
   * \brief Creates a new variable.  
   * 
   * @param x A pointer to the corresponding variable implementation
   * @param r A reference to the mozart heap reference corresponding
   * to the variable @return The index of the new allocated variable
   */
  int newVar(Gecode::VarImpBase *x, OZ_Term r) {
    int i = size;
    refs[size]=r; vars[size]=x; size++;
    return i;
  }

  int getSize(void) { return size; }

  OZ_Term *getRef(int n) { 
    Assert(n >= 0 && n < size);
    return &(refs[n]); 
  }

  OZ_Term getRef2(int n) const { 
    Assert(n >= 0 && n < size);
    return refs[n]; 
  }

  Gecode::VarImpBase& getVar(int n) {
    Assert(n >= 0 && n < size);
    return *vars[n];
  }

  void setRef(int n, OZ_Term t) { 
    Assert(n >= 0 && n < size);
    refs[n] = t; 
  }
  
  /**
     \brief Sets the root variables of this space to variables
     constained in vector \a v.
   */
  //void setRoot(OZ_Term v);
};


/* This method adds the generic space gs to the list of allocated
   generic spaces.  It must be called for all created generic space in
   order to put it in the control of the garbage collector.
*/
void registerGeSpace(GenericSpace* gs);

/* Collects the memory allocated by unused generic spaces. */
void gCollectGeSpaces(void);

/** 
 * \brief Container for Gecode variables and propagators. A
 * GenericSpace is a Gecode::Space that contains Gecode variables,
 * which are visible in Oz as variables.  That space will also contain
 * all the propagators created at the Oz level.
 */
class GenericSpace : public Gecode::Space {
  friend void gCollectGeSpaces(void);
  friend void registerGeSpace(GenericSpace*);
  friend class Board;
private:
  Board *board;  // pointer to the mozart space 

  /// Variable containers
  VarRefArray vars;     /// Array container for variables

  
  /** 
      Space stability is used to reflect the fact that there is
      pending work to do. It is for example, a new propagator was
      posted in this space but no call to mstatus() has been performed
      so far. Other things that can make a space to become unstable
      are unification or bindings events from mozart.
  */
  bool trigger;
  
  /* 
     Invariant: trigger is true while the GenericSpace remains stable.
     It is assigned to false as soon as the space becomes unstable and
     lateThread is added to the status suspension list. If at top
     level space (?).  The Oz thread suspending on status will
     therefore be woken up when the space becomes unstable.
  */

  /*
   * Garbage collection: Generic spaces are stored outside the mozart
   * heap. In order to collect memory, every time a GenericSpace is
   * created it is added to GeSpaceCollectList. This list is travesed
   * by garbage collector to free memory allocated by unused
   * gespaces. A double linked list is used for that
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
    return allocated() + sizeof(*this);
  }
  
  /**
     \brief Sets stability of this space. This is done by binding
	trigger to a read only variable.
  */
  void makeStable(void);

  /**
   * \brief This variable is used to count how many variables are
   * determined. This criteria is used to know when a space becomes
   * solved (i.e. determined = vars.size. A space is solved when all
   * the variables in the space become determined.
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

  /// \name Reference access
  //@{  
  /** 
   * \brief Set the reference to the corresponding variable in Mozart
   * 
   * @param n Variable index 
   * @param OZ_Term Heap reference
   */
  void setVarRef(int n,OZ_Term);

  /**
     \brief Fill vm with variables in \a vars
   */
  void varReflect(Gecode::Reflection::VarMap &vm, bool registerOnly = false);

  
  void reflect(std::vector<Gecode::Reflection::ActorSpec>& as, 
	       std::vector<Gecode::Reflection::VarSpec>& vs);

  void unreflect(std::vector<Gecode::Reflection::ActorSpec>& as, 
		 std::vector<Gecode::Reflection::VarSpec>& vs);
  
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
   * the memory allocated by an unused object. The object is also
   * deleted when a space becomes solved or failed.
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
    //printf("GeSpace.hh copy\n");fflush(stdout);
    return new GenericSpace(*this,share);
  }

  void makeUnstable(void);   // binds current trigger to unit

  // return current trigger value
  bool getTrigger(void) { return trigger; }
  
  /**
  	\brief Tests whether the space is a solution space.
  */
  bool isSolved(void) { return determined == (vars.getSize()); }
  
  /**
  	\brief Increment the counter of determined variables in the space
  */
  void incDetermined(void) { determined++; }

  /**
     \brief Returns whether the generic space is stable or not.
     
  */
  bool isStable();
  
  /**
     \brief Tests if the space is entailed. A space is entailed if the
     number of non determined variables plus the number of
     foreignProps (reflections) plus the number of unifications is the
     number of propagators. Since unification happens between two
     variables, the number of propagators enforcing unifications have
     to be divided by two.
  */
  bool isEntailed(void) { 
    Assert(isStable());
    int nondet = vars.getSize() - determined;
    /*printf("Nondet: %d\n",nondet);fflush(stdout);
    printf("foreignProps: %d\n",foreignProps);fflush(stdout);
    printf("Propagators: %d\n",propagators());fflush(stdout);
    printf("Propagators: %d\n",unifyProps);fflush(stdout);*/
    return (nondet + foreignProps + unifyProps/2) == propagators();
  }

  /**
     \brief Foreign propagators help to reflect changes in variables
     to mozart.  Those include domain reflection and variable binding.
  */
  void incForeignProps(void) { foreignProps++; }
  void decForeignProps(int d = 1) { foreignProps -= d; }

  /**
     \brief Every time a unification between two constraint variables
     takes place, a propagator is used to enforce *equality* (in the
     sense of the underlying constraint system).
  */
  void incUnifyProps(void) { unifyProps+=2; }
  void decUnifyProps(int d = 1) { unifyProps -= d; }
 

  /// \name Variable creation
  //@{
  /** 
   * \brief Creates a new variable inside the space
   * 
   * @param v A pointer to the var implementation.  @param r A
   * reference to the mozart heap node corresponding to the variable
   * 
   * @return The index of the new variable
   */
  int newVar(Gecode::VarImpBase *v, OZ_Term r);

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

  Gecode::VarImpBase* getVar(int n);

  /**
   * \brief Access IntVar at a given index for 
   * information (read only) purposes
   *
   * @param n Index in the IntVarArray
   * @return A reference to the variable
   *
   */

  Gecode::VarImpBase* getVarInfo(int n);

  //@}

  /** 
   * \brief Get the reference to the heap of the corresponding variable
   * 
   * @param n Variable index 
   * @return reference to the mozart heap 
   */
  OZ_Term getVarRef(int n) { return *vars.getRef(n); }

  OZ_Term getVarRef2(int n) const {
    return vars.getRef2(n);
  }
  //@}

 
  /** 
      \brief Wraps the status function of the Space class to perform memory 
	  counting and stability operations
  */
  Gecode::SpaceStatus mstatus(void);
  
  /**
	\brief Merges variables and propagators of space \a src in *this.
  */
  void merge(GenericSpace *src, Board *tgt);

  /// Garbage collection and space cloning for references
  //@{
  /** 
   * \brief Update references to the mozart heap
   * 
   */
  void gCollect();
  void sClone();
  //@}

  /**
     \brief Garbage collection for generic spaces. To properly handle
     garbage collection of generic spaces a double linked list is
     used. Every new generic space is stored in the list, when the
     garbage collector runs, all the unused spaces are removed from
     memory. Generic spaces are stored outside the mozart heap.
  */
  //@{
  /// Pontier to the begining of the list of allocated GenericSpace's.
  static GenericSpace* GeSpaceCollectList;

  /// Pointer to the last element in the list of allocated Generic Spaces
  static GenericSpace* GeSpaceCollectLast;

  /// Total memory allocated in generic spaces
  static size_t GeSpaceAllocatedMem;

  //@}


};

#endif /* __GESPACE_HH__ */
