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


class GenOFSVariable: public GenCVariable {

    friend class GenCVariable;
    friend inline void addSuspOFSVar(TaggedRef, Thread *);

private:
    TaggedRef label;
    DynamicTable* dynamictable;

public:
    GenOFSVariable(DynamicTable &dt)
    : GenCVariable(OFSVariable) {
        label=oz_newVariable();
        dynamictable= &dt;
    }

    GenOFSVariable()
    : GenCVariable(OFSVariable) {
        label=oz_newVariable();
        dynamictable=DynamicTable::newDynamicTable();
    }

    // With new table of given size (must be pwr. of 2) in given space:
    GenOFSVariable(Board* hm, dt_index size)
    : GenCVariable(OFSVariable,hm) {
        label=makeTaggedRef(newTaggedUVar(hm));
        dynamictable=DynamicTable::newDynamicTable(size);
    }

    // With new table of given size (must be pwr. of 2):
    GenOFSVariable(dt_index size)
    : GenCVariable(OFSVariable) {
        label=oz_newVariable();
        dynamictable=DynamicTable::newDynamicTable(size);
    }

    GenOFSVariable(TaggedRef lbl)
    : GenCVariable(OFSVariable) {
        Assert(isLiteral(lbl));
        label=lbl;
        dynamictable=DynamicTable::newDynamicTable();
    }

    GenOFSVariable(TaggedRef lbl, dt_index size)
    : GenCVariable(OFSVariable) {
        Assert(isLiteral(lbl));
        label=lbl;
        dynamictable=DynamicTable::newDynamicTable(size);
    }

    // Methods relevant for term copying (gc and solve)
    void gc(void);
    size_t getSize(void) { return sizeof(GenOFSVariable); }

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

    Bool unifyOFS(TaggedRef *, TaggedRef, TaggedRef *, TaggedRef, ByteCode *);

    // Return the feature value if feature exists, return NULL if it doesn't exist
    TaggedRef getFeatureValue(TaggedRef feature) {
	Assert(isFeature(feature));
        return dynamictable->lookup(feature);
    }

    // Add the feature and its value
    // If the feature already exists, do not insert anything
    // Return TRUE if feature successfully inserted, FALSE if it already exists
    // ATTENTION: only use this for terms that do not have to be trailed
    Bool addFeatureValue(TaggedRef feature, TaggedRef term) {
	Assert(isFeature(feature));
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
            // propagate(makeTaggedCVar(this), suspList, makeTaggedCVar(this), pc_propagator);
            am.addFeatOFSSuspensionList(makeTaggedCVar(this),suspList,feature,FALSE);
	    return TRUE;
        } else {
	    return FALSE;
	}
    }

    // Destructively update feature's value, if feature exists
    // Return TRUE if feature exists, FALSE if it does not
    Bool setFeatureValue(TaggedRef feature, TaggedRef term) {
	Assert(isFeature(feature));
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
      propagate(makeTaggedCVar(this), suspList, pc_propagator);
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
};


Bool isGenOFSVar(TaggedRef term);
Bool isGenOFSVar(TaggedRef term, TypeOfTerm tag);
GenOFSVariable *tagged2GenOFSVar(TaggedRef term);

inline
void addSuspOFSVar(TaggedRef v, Thread * el)
{
  GenOFSVariable * ofs = tagged2GenOFSVar(v);
  AddSuspToList(ofs->suspList, el, ofs->home);
}


#endif
