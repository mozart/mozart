/*
 *  Authors:
 *    Peter van Roy (pvr@info.ucl.ac.be)
 *
 *  Contributors:
 *    optional, Contributor's name (Contributor's email address)
 *
 *  Copyright:
 *    Organization or Person (Year(s))
 *
 *  Last change:
 *    $Date$ by $Author$
 *    $Revision$
 *
 *  This file is part of Mozart, an implementation
 *  of Oz 3:
 *     http://mozart.ps.uni-sb.de
 *
 *  See the file "LICENSE" or
 *     http://mozart.ps.uni-sb.de/LICENSE.html
 *  for information on usage and redistribution
 *  of this file, and for a DISCLAIMER OF ALL
 *  WARRANTIES.
 *
 */

#ifndef __GENOFSVAR__H__
#define __GENOFSVAR__H__

#if defined(INTERFACE)
#pragma interface
#endif

#include "am.hh"
#include "genvar.hh"
#include "tagged.hh"
#include "value.hh"
#include "mem.hh"
#include "dictionary.hh"

//-------------------------------------------------------------------------
//                           class GenOFSVariable
//-------------------------------------------------------------------------


// from ofbi.cc
extern
void addFeatOFSSuspensionList(TaggedRef var, SuspList * suspList,
                              TaggedRef flist, Bool determ);


class GenOFSVariable: public GenCVariable {

    friend class GenCVariable;
    friend inline void addSuspOFSVar(TaggedRef, Suspension);

private:
    TaggedRef label;
    DynamicTable* dynamictable;

public:
    GenOFSVariable(DynamicTable &dt,Board *bb)
    : GenCVariable(OFSVariable,bb) {
        label=oz_newVariable();
        dynamictable= &dt;
    }

    GenOFSVariable(Board *bb)
    : GenCVariable(OFSVariable,bb) {
        label=oz_newVariable();
        dynamictable=DynamicTable::newDynamicTable();
    }

    // With new table of given size (must be pwr. of 2):
    GenOFSVariable(dt_index size,Board *bb)
    : GenCVariable(OFSVariable,bb) {
        label=oz_newVariable();
        dynamictable=DynamicTable::newDynamicTable(size);
    }

    GenOFSVariable(TaggedRef lbl,Board *bb)
    : GenCVariable(OFSVariable,bb) {
        Assert(oz_isLiteral(lbl));
        label=lbl;
        dynamictable=DynamicTable::newDynamicTable();
    }

    GenOFSVariable(TaggedRef lbl, dt_index size,Board *bb)
    : GenCVariable(OFSVariable,bb) {
        Assert(oz_isLiteral(lbl));
        label=lbl;
        dynamictable=DynamicTable::newDynamicTable(size);
    }

    // Methods relevant for term copying (gc and solve)
    void gcRecurse(void);

    TaggedRef getLabel(void) {
        return label;
    }

    long getWidth(void) {
        return (long)(dynamictable->numelem);
    }

    DynamicTable* getTable(void) {
        return dynamictable;
    }

    // Return a sorted difference list of all the features currently in the OFS
    // The head is the return value and the tail is returned through an argument.
    TaggedRef getOpenArityList(TaggedRef*,Board*);

    // Return a sorted list of all features currently in the OFS
    TaggedRef getArityList();

    // Return the feature value if feature exists, return NULL if it doesn't exist
    TaggedRef getFeatureValue(TaggedRef feature) {
        Assert(oz_isFeature(feature));
        return dynamictable->lookup(feature);
    }

  void installPropagators(GenOFSVariable * glob_var) {
    installPropagatorsG(glob_var);
  }

    // Add the feature and its value
    // If the feature already exists, do not insert anything
    // Return TRUE if feature successfully inserted, FALSE if it already exists
    // ATTENTION: only use this for terms that do not have to be trailed
    Bool addFeatureValue(TaggedRef feature, TaggedRef term) {
        Assert(oz_isFeature(feature));
        Bool valid;
        if (dynamictable->fullTest()) resizeDynamicTable(dynamictable);
        TaggedRef prev=dynamictable->insert(feature,term,&valid);
        if (!valid) {
            resizeDynamicTable(dynamictable);
            prev=dynamictable->insert(feature,term,&valid);
        }
        Assert(valid);
        // (future optimization: a second suspList only waiting on features)
        if (prev==makeTaggedNULL()) {
            // propagate(suspList, pc_propagator);
            addFeatOFSSuspensionList(makeTaggedCVar(this),suspList,feature,FALSE);
            return TRUE;
        } else {
            return FALSE;
        }
    }

    // Destructively update feature's value, if feature exists
    // Return TRUE if feature exists, FALSE if it does not
    Bool setFeatureValue(TaggedRef feature, TaggedRef term) {
        Assert(oz_isFeature(feature));
        return dynamictable->update(feature,term);
    }

    // Remove the feature from the OFS
    // Reclaims memory if table becomes too sparse
    void removeFeature(TaggedRef feature) {
        dynamictable=dynamictable->remove(feature);
    }

    // Used to propagate suspensions (addFeatureValue, getLabel don't do it):
    void propagateOFS(void) {
      /* third argument must be ignored --> use AtomNil */
      propagate(suspList, pc_propagator);
    }

    int getSuspListLength(void) {
        // see suspension.{hh,cc}:
        return suspList->length();
    }

    int getNumOfFeatures(void) {
        return (int) dynamictable->numelem;
    }

    // Is X=val still valid, i.e., is val a feature and is width(ofs)==0 (see GenFDVariable::valid)
  Bool valid(TaggedRef val);
  OZ_Return unify(TaggedRef *vPtr, TaggedRef term, ByteCode *scp);

  int hasFeature(TaggedRef fea,TaggedRef *out) {
    TaggedRef t = getFeatureValue(fea);
    if (t == makeTaggedNULL()) return SUSPEND;
    if (out) *out = t;
    return PROCEED;
  }
    // Hooks for indexing: the following two methods should return
    // OK iff "this" disentails its label being "l" or its
    // arity being "tupleArity"/"recordArity"
    // note: "tupleArity" may be zero
    // for now they always return NO, leading to suspension
    Bool disentailed(Literal *l, int tupleArity);
    Bool disentailed(Literal *l, Arity *recordArity);

    // These procedures exist as well in the class GenFDVariable,
    // but they are not needed in GenOFSVariable:

    // void propagate (needs no redefinition from GenCVariable version)
    // void relinkSuspList (needs no redefinition from GenCVariable version)
    // void becomesSmallIntAndPropagate (meaningless for ofs)
    // void setDom (meaningless for ofs)
    // FiniteDomain &getDom (meaningless for ofs)


  void dispose(void) { freeListDispose(this, sizeof(GenOFSVariable)); }
  void printStream(ostream &out,int depth = 10) {
    oz_printStream(getLabel(),out,0,0);
    out << '(';
    if (depth > 0) {
      getTable()->newprint(out,depth-1);
    } else {
      out << ",,, ";
      return;
    }
    out << "...)";
  }
  void printLongStream(ostream &out,int depth = 10,
                        int offset = 0) {
    printStream(out,depth); out << endl;
  }
};


Bool isGenOFSVar(TaggedRef term);
Bool isGenOFSVar(TaggedRef term, TypeOfTerm tag);
GenOFSVariable *tagged2GenOFSVar(TaggedRef term);

inline
void addSuspOFSVar(TaggedRef v, Suspension susp)
{
  GenOFSVariable * ofs = tagged2GenOFSVar(v);
  AddSuspToList(ofs->suspList, susp, ofs->home);
}


#endif
