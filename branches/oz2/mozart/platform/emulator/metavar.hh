#ifndef __metavar__hh__
#define __metavar__hh__


#if defined(INTERFACE)
#pragma interface
#endif

#include "genvar.hh"
#include "oz.h"

#ifdef METAVAR

#if defined(OUTLINE)
#define inline
#endif

typedef enum {
  meta_unconstr     = 0,
  meta_det          = 1,
  meta_left_constr  = 2,
  meta_right_constr = 4,
  meta_fail         = 8
} mur_t;

typedef enum {
  OZ_Type_Cell,
  OZ_Type_Chunk,
  OZ_Type_Cons,
  OZ_Type_HeapChunk,
  OZ_Type_CVar,
  OZ_Type_Float,
  OZ_Type_Int,
  OZ_Type_Literal,
  OZ_Type_Procedure,
  OZ_Type_Record,
  OZ_Type_Tuple,
  OZ_Type_Var,
  OZ_Type_Unknown
} OZ_TermType;


typedef void * OZ_MetaType;

typedef mur_t _FUNDECL((* OZ_UnifyMetaDet), (OZ_Term, OZ_Term, OZ_Term, 
					     OZ_TermType, OZ_Term *));
typedef mur_t _FUNDECL((* OZ_UnifyMetaMeta), (OZ_Term, OZ_Term, OZ_Term, 
					      OZ_Term, OZ_MetaType, OZ_Term *));

typedef char * _FUNDECL((* OZ_PrintMeta), (OZ_Term, int));
typedef int    _FUNDECL((* OZ_IsSingleValue), (OZ_Term));

struct MetaTag {
friend class GenMetaVariable;

private:
  OZ_UnifyMetaDet     unify_meta_det;
  OZ_UnifyMetaMeta    unify_meta_meta;
  OZ_PrintMeta        print_meta_data;
  OZ_IsSingleValue    sgl_val_meta_data;
  char *              name;
public:
  MetaTag(OZ_UnifyMetaDet unify_md,
	  OZ_UnifyMetaMeta unify_mm,
	  OZ_PrintMeta print,
	  OZ_IsSingleValue sgl_val,
	  char * n)
  {
    unify_meta_det = unify_md;
    unify_meta_meta = unify_mm;
    print_meta_data = print;
    sgl_val_meta_data = sgl_val;
    name = n;
  }
};

class GenMetaVariable : public GenCVariable {
friend class GenCVariable;

private:
  TaggedRef data;
  MetaTag * tag;
  Bool unifyMeta(TaggedRef *, TaggedRef, TaggedRef *, TaggedRef, Bool);

public:
  GenMetaVariable(MetaTag * t, TaggedRef tr);

  MetaTag * getTag(void) { return tag; }
  void putTag(MetaTag * t) { tag = t; }

  TaggedRef getData(void) { return data; }
  void setData(TaggedRef d) { data = d; }

  size_t getSize(void) { return sizeof(GenMetaVariable); }
  void gc(void);

  char * getName(void) { return tag->name; }
  
  char * toString(int depth) { return tag->print_meta_data(data, depth); }

  Bool valid(TaggedRef v);

  void constrainVar(TaggedRef v, TaggedRef d) {
    setData(d);
    propagate(v, suspList, pc_propagator);
  }

  Bool isStrongerThan(TaggedRef var, TaggedRef vdata);

  Bool isSingleValue(void) {return tag->sgl_val_meta_data(data);}

  mur_t check(TaggedRef var, OZ_MetaType vtype, TaggedRef vdata) {
    return tag->unify_meta_meta(makeTaggedCVar(this), getData(), 
				var, vdata, vtype, NULL);
  }
};


inline
Bool isHeapChunk(TaggedRef term);

inline
Bool isGenMetaVar(TaggedRef term);

inline
Bool isGenMetaVar(TaggedRef term, TypeOfTerm tag);

#if !defined(OUTLINE)
#include "metavar.icc"
#else
#undef inline
#endif

#endif /* METAVAR */

#endif
