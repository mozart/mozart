/*
 *  Main authors:
 *     Diana Lorena Velasco <dlvelasco@puj.edu.co>
 *     Juan Gabriel Torres  <juantorres@puj.edu.co>
 *
 *  Contributing authors:
 *     Andres Felipe Barco <anfelbar@univalle.edu.co>
 *
 *  Copyright:
 *     Diana Lorena Velasco, 2007
 *     Juan Gabriel Torres, 2007
 *
 *  Last modified:
 *     $Date$
 *     $Revision$
 *
 *  This file is part of Mozart, an implementation 
 *  of Oz 3:
 *     http://www.mozart-oz.org
 *
 *  See the file 'LICENSE' for information on usage and
 *  redistribution of this file, and for a
 *     DISCLAIMER OF ALL WARRANTIES.
 *
 */


/**
    This file is generated automatic by other aplication, 
                    please do not modified this file.
*/

#ifndef __GEOZ_BOOLVAR_PROP_BUILTINS_CC__
#define __GEOZ_BOOLVAR_PROP_BUILTINS_CC__

#include "SetVarMacros.hh"

/**
    Projector constraints
*/
/*
OZ_BI_define(gfs_projector_4,4,0){
  DeclareGSpace(home);
  DeclareGeSetVar(0, __xa, home);
  DeclareGeSetVar(1, __ya, home);
  if(OZ_isGeSetVar(OZ_in(0)) && OZ_isGeSetVar(OZ_in(1)) && OZ_isProjectorSet(OZ_in(2)) && OZ_isBool(OZ_in(3))){
    DECLARE_PROJECTOR_SET(2, __ps);
    DeclareBool(3, __negated);
    try{
      Gecode::projector(home, __xa, __ya, __ps, __negated);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeSetVar(OZ_in(0)) && OZ_isGeSetVar(OZ_in(1)) && OZ_isGeBoolVar(OZ_in(2)) && OZ_isProjectorSet(OZ_in(3))){
    DeclareGeBoolVar(2, __bv, home);
    DECLARE_PROJECTOR_SET(3, __ps);
    try{
      Gecode::projector(home, __xa, __ya, __bv, __ps);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeSetVar(OZ_in(0)) && OZ_isGeSetVar(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2)) && OZ_isProjector(OZ_in(3))){
    DeclareGeIntVar(2, __i, home);
    DECLARE_PROJECTOR(3, __p);
    try{
      Gecode::projector(home, __xa, __ya, __i, __p);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else{
    OZ_typeError(0, "Malformed Propagator");
  }
  CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfs_projector_3,3,0){
  DeclareGSpace(home);
  DECLARE_SETVARARGS(0, __xa, home);
  DECLARE_PROJECTOR_SET(1, __ps);
  DeclareBool(2, __negated);
  try{
    Gecode::projector(home, __xa, __ps, __negated);
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfs_projector_5,5,0){
  DeclareGSpace(home);
  DeclareGeSetVar(0, __xa, home);
  DeclareGeSetVar(1, __ya, home);
  DeclareGeSetVar(2, __za, home);
  if(OZ_isGeSetVar(OZ_in(0)) && OZ_isGeSetVar(OZ_in(1)) && OZ_isGeSetVar(OZ_in(2)) && OZ_isProjectorSet(OZ_in(3)) && OZ_isBool(OZ_in(4))){
    DECLARE_PROJECTOR_SET(3, __ps);
    DeclareBool(4, __negated);
    try{
      Gecode::projector(home, __xa, __ya, __za, __ps, __negated);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeSetVar(OZ_in(0)) && OZ_isGeSetVar(OZ_in(1)) && OZ_isGeSetVar(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3)) && OZ_isProjectorSet(OZ_in(4))){
    DeclareGeBoolVar(3, __bv, home);
    DECLARE_PROJECTOR_SET(4, __ps);
    try{
      Gecode::projector(home, __xa, __ya, __za, __bv, __ps);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeSetVar(OZ_in(0)) && OZ_isGeSetVar(OZ_in(1)) && OZ_isGeSetVar(OZ_in(2)) && OZ_isGeIntVar(OZ_in(3)) && OZ_isProjector(OZ_in(4))){
    DeclareGeIntVar(3, __i, home);
    DECLARE_PROJECTOR(4, __p);
    try{
      Gecode::projector(home, __xa, __ya, __za, __i, __p);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else{
    OZ_typeError(0, "Malformed Propagator");
  }
  CHECK_POST(home);
}OZ_BI_end
*/
/**
    domains constraints
*/
OZ_BI_define(gfs_dom_3,3,0){
  DeclareGSpace(home);
  DeclareGeSetVar(0, __s, home);
  DeclareSetRelType(1, __r);
  if(OZ_isGeSetVar(OZ_in(0)) && OZ_isSetRelType(OZ_in(1)) && OZ_isInt(OZ_in(2))){
    DeclareInt2(2, __i);
    try{
      Gecode::dom(home, __s, __r, __i);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeSetVar(OZ_in(0)) && OZ_isSetRelType(OZ_in(1)) && OZ_isIntSet(OZ_in(2))){
    DECLARE_INT_SET(2, __is);
    try{
      Gecode::dom(home, __s, __r, __is);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else{
    OZ_typeError(0, "Malformed Propagator");
  }
  CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfs_dom_4,4,0){
  DeclareGSpace(home);
  DeclareGeSetVar(0, __s, home);
  DeclareSetRelType(1, __r);
  if(OZ_isGeSetVar(OZ_in(0)) && OZ_isSetRelType(OZ_in(1)) && OZ_isInt(OZ_in(2)) && OZ_isInt(OZ_in(3))){
    DeclareInt2(2, __i);
    DeclareInt2(3, __j);
    try{
      Gecode::dom(home, __s, __r, __i, __j);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeSetVar(OZ_in(0)) && OZ_isSetRelType(OZ_in(1)) && OZ_isInt(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3))){
    DeclareInt2(2, __i);
    DeclareGeBoolVar(3, __b, home);
    try{
      Gecode::dom(home, __s, __r, __i, __b);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeSetVar(OZ_in(0)) && OZ_isSetRelType(OZ_in(1)) && OZ_isIntSet(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3))){
    DECLARE_INT_SET(2, __is);
    DeclareGeBoolVar(3, __b, home);
    try{
      Gecode::dom(home, __s, __r, __is, __b);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else{
    OZ_typeError(0, "Malformed Propagator");
  }
  CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfs_dom_5,5,0){
  DeclareGSpace(home);
  DeclareGeSetVar(0, __s, home);
  DeclareSetRelType(1, __r);
  DeclareInt2(2, __i);
  DeclareInt2(3, __j);
  DeclareGeBoolVar(4, __b, home);
  try{
    Gecode::dom(home, __s, __r, __i, __j, __b);
  }
  catch(Exception e){
    RAISE_GE_EXCEPTION(e);
  }
  CHECK_POST(home);
}OZ_BI_end

/**
    Relation constraints
*/
 /*
OZ_BI_define(gfs_rel_3,3,0){
  DeclareGSpace(home);
  if(OZ_isGeSetVar(OZ_in(0)) && OZ_isSetRelType(OZ_in(1)) && OZ_isGeSetVar(OZ_in(2))){
    DeclareGeSetVar(0, __x, home);
    DeclareSetRelType(1, __r);
    DeclareGeSetVar(2, __y, home);
    try{
      Gecode::rel(home, __x, __r, __y);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeSetVar(OZ_in(0)) && OZ_isSetRelType(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2))){
    DeclareGeSetVar(0, __s, home);
    DeclareSetRelType(1, __r);
    DeclareGeIntVar(2, __x, home);
    try{
      Gecode::rel(home, __s, __r, __x);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeIntVar(OZ_in(0)) && OZ_isSetRelType(OZ_in(1)) && OZ_isGeSetVar(OZ_in(2))){
    DeclareGeIntVar(0, __x, home);
    DeclareSetRelType(1, __r);
    DeclareGeSetVar(2, __s, home);
    try{
      Gecode::rel(home, __x, __r, __s);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeSetVar(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2))){
    DeclareGeSetVar(0, __s, home);
    DeclareIntRelType(1, __r);
    DeclareGeIntVar(2, __x, home);
    try{
      Gecode::rel(home, __s, __r, __x);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeIntVar(OZ_in(0)) && OZ_isIntRelType(OZ_in(1)) && OZ_isGeSetVar(OZ_in(2))){
    DeclareGeIntVar(0, __x, home);
    DeclareIntRelType(1, __r);
    DeclareGeSetVar(2, __s, home);
    try{
      Gecode::rel(home, __x, __r, __s);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isSetOpType(OZ_in(0)) && OZ_isSetVarArgs(OZ_in(1)) && OZ_isGeSetVar(OZ_in(2))){
    DeclareSetOpType(0, __op);
    DECLARE_SETVARARGS(1, __x, home);
    DeclareGeSetVar(2, __y, home);
    try{
      Gecode::rel(home, __op, __x, __y);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isSetOpType(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isGeSetVar(OZ_in(2))){
    DeclareSetOpType(0, __op);
    DECLARE_INTVARARGS(1, __x, home);
    DeclareGeSetVar(2, __y, home);
    try{
      Gecode::rel(home, __op, __x, __y);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else{
    OZ_typeError(0, "Malformed Propagator");
  }
  CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfs_rel_4,4,0){
  DeclareGSpace(home);
  if(OZ_isGeSetVar(OZ_in(0)) && OZ_isSetRelType(OZ_in(1)) && OZ_isGeSetVar(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3))){
    DeclareGeSetVar(0, __x, home);
    DeclareSetRelType(1, __r);
    DeclareGeSetVar(2, __y, home);
    DeclareGeBoolVar(3, __b, home);
    try{
      Gecode::rel(home, __x, __r, __y, __b);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeSetVar(OZ_in(0)) && OZ_isSetRelType(OZ_in(1)) && OZ_isGeIntVar(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3))){
    DeclareGeSetVar(0, __s, home);
    DeclareSetRelType(1, __r);
    DeclareGeIntVar(2, __x, home);
    DeclareGeBoolVar(3, __b, home);
    try{
      Gecode::rel(home, __s, __r, __x, __b);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeIntVar(OZ_in(0)) && OZ_isSetRelType(OZ_in(1)) && OZ_isGeSetVar(OZ_in(2)) && OZ_isGeBoolVar(OZ_in(3))){
    DeclareGeIntVar(0, __x, home);
    DeclareSetRelType(1, __r);
    DeclareGeSetVar(2, __s, home);
    DeclareGeBoolVar(3, __b, home);
    try{
      Gecode::rel(home, __x, __r, __s, __b);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isSetOpType(OZ_in(0)) && OZ_isSetVarArgs(OZ_in(1)) && OZ_isIntSet(OZ_in(2)) && OZ_isGeSetVar(OZ_in(3))){
    DeclareSetOpType(0, __op);
    DECLARE_SETVARARGS(1, __x, home);
    DECLARE_INT_SET(2, __z);
    DeclareGeSetVar(3, __y, home);
    try{
      Gecode::rel(home, __op, __x, __z, __y);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isSetOpType(OZ_in(0)) && OZ_isIntVarArgs(OZ_in(1)) && OZ_isIntSet(OZ_in(2)) && OZ_isGeSetVar(OZ_in(3))){
    DeclareSetOpType(0, __op);
    DECLARE_INTVARARGS(1, __x, home);
    DECLARE_INT_SET(2, __z);
    DeclareGeSetVar(3, __y, home);
    try{
      Gecode::rel(home, __op, __x, __z, __y);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else{
    OZ_typeError(0, "Malformed Propagator");
  }
  CHECK_POST(home);
}OZ_BI_end

OZ_BI_define(gfs_rel_5,5,0){
  DeclareGSpace(home);
  DeclareSetOpType(1, __op);
  DeclareSetRelType(3, __r);
  if(OZ_isGeSetVar(OZ_in(0)) && OZ_isSetOpType(OZ_in(1)) && OZ_isGeSetVar(OZ_in(2)) && OZ_isSetRelType(OZ_in(3)) && OZ_isGeSetVar(OZ_in(4))){
    DeclareGeSetVar(0, __x, home);
    DeclareGeSetVar(2, __y, home);
    DeclareGeSetVar(4, __z, home);
    try{
      Gecode::rel(home, __x, __op, __y, __r, __z);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntSet(OZ_in(0)) && OZ_isSetOpType(OZ_in(1)) && OZ_isGeSetVar(OZ_in(2)) && OZ_isSetRelType(OZ_in(3)) && OZ_isGeSetVar(OZ_in(4))){
    DECLARE_INT_SET(0, __x);
    DeclareGeSetVar(2, __y, home);
    DeclareGeSetVar(4, __z, home);
    try{
      Gecode::rel(home, __x, __op, __y, __r, __z);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeSetVar(OZ_in(0)) && OZ_isSetOpType(OZ_in(1)) && OZ_isIntSet(OZ_in(2)) && OZ_isSetRelType(OZ_in(3)) && OZ_isGeSetVar(OZ_in(4))){
    DeclareGeSetVar(0, __x, home);
    DECLARE_INT_SET(2, __y);
    DeclareGeSetVar(4, __z, home);
    try{
      Gecode::rel(home, __x, __op, __y, __r, __z);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeSetVar(OZ_in(0)) && OZ_isSetOpType(OZ_in(1)) && OZ_isGeSetVar(OZ_in(2)) && OZ_isSetRelType(OZ_in(3)) && OZ_isIntSet(OZ_in(4))){
    DeclareGeSetVar(0, __x, home);
    DeclareGeSetVar(2, __y, home);
    DECLARE_INT_SET(4, __z);
    try{
      Gecode::rel(home, __x, __op, __y, __r, __z);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isIntSet(OZ_in(0)) && OZ_isSetOpType(OZ_in(1)) && OZ_isGeSetVar(OZ_in(2)) && OZ_isSetRelType(OZ_in(3)) && OZ_isIntSet(OZ_in(4))){
    DECLARE_INT_SET(0, __x);
    DeclareGeSetVar(2, __y, home);
    DECLARE_INT_SET(4, __z);
    try{
      Gecode::rel(home, __x, __op, __y, __r, __z);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else if(OZ_isGeSetVar(OZ_in(0)) && OZ_isSetOpType(OZ_in(1)) && OZ_isIntSet(OZ_in(2)) && OZ_isSetRelType(OZ_in(3)) && OZ_isIntSet(OZ_in(4))){
    DeclareGeSetVar(0, __x, home);
    DECLARE_INT_SET(2, __y);
    DECLARE_INT_SET(4, __z);
    try{
      Gecode::rel(home, __x, __op, __y, __r, __z);
    }
    catch(Exception e){
      RAISE_GE_EXCEPTION(e);
    }
  }
  else{
    OZ_typeError(0, "Malformed Propagator");
  }
  CHECK_POST(home);
}OZ_BI_end
 */
/**
    Convexity constraints
*/

// OZ_BI_define(gfs_convex_1,1,0){
//   DeclareGSpace(home);
//   DeclareGeSetVar(0, __x, home);
//   try{
//     Gecode::convex(home, __x);
//   }
//   catch(Exception e){
//     RAISE_GE_EXCEPTION(e);
//   }
//   CHECK_POST(home);
// }OZ_BI_end

// OZ_BI_define(gfs_convexHull_2,2,0){
//   DeclareGSpace(home);
//   DeclareGeSetVar(0, __x, home);
//   DeclareGeSetVar(1, __y, home);
//   try{
//     Gecode::convexHull(home, __x, __y);
//   }
//   catch(Exception e){
//     RAISE_GE_EXCEPTION(e);
//   }
//   CHECK_POST(home);
// }OZ_BI_end

/**
    Sequence constraints  
*/
// OZ_BI_define(gfs_sequence_1,1,0){
//   DeclareGSpace(home);
//   DECLARE_SETVARARGS(0, __xa, home);
//   try{
//     Gecode::sequence(home, __xa);
//   }
//   catch(Exception e){
//     RAISE_GE_EXCEPTION(e);
//   }
//   CHECK_POST(home);
// }OZ_BI_end

// OZ_BI_define(gfs_sequentialUnion_2,2,0){
//   DeclareGSpace(home);
//   DECLARE_SETVARARGS(0, __xa, home);
//   DeclareGeSetVar(1, __y, home);
//   try{
//     Gecode::sequentialUnion(home, __xa, __y);
//   }
//   catch(Exception e){
//     RAISE_GE_EXCEPTION(e);
//   }
//   CHECK_POST(home);
// }OZ_BI_end

/**
    Connection constraints to finite domain variables
*/
// OZ_BI_define(gfs_min_2,2,0){
//   DeclareGSpace(home);
//   DeclareGeSetVar(0, __s, home);
//   DeclareGeIntVar(1, __x, home);
//   try{
//     Gecode::min(home, __s, __x);
//   }
//   catch(Exception e){
//     RAISE_GE_EXCEPTION(e);
//   }
//   CHECK_POST(home);
// }OZ_BI_end

// OZ_BI_define(gfs_match_2,2,0){
//   DeclareGSpace(home);
//   DeclareGeSetVar(0, __s, home);
//   DECLARE_INTVARARGS(1, __x, home);
//   try{
//     Gecode::match(home, __s, __x);
//   }
//   catch(Exception e){
//     RAISE_GE_EXCEPTION(e);
//   }
//   CHECK_POST(home);
// }OZ_BI_end

// OZ_BI_define(gfs_channel_2,2,0){
//   DeclareGSpace(home);
//   if(OZ_isIntVarArgs(OZ_in(0)) && OZ_isSetVarArgs(OZ_in(1))){
//     DECLARE_INTVARARGS(0, __x, home);
//     DECLARE_SETVARARGS(1, __y, home);
//     try{
//       Gecode::channel(home, __x, __y);
//     }
//     catch(Exception e){
//       RAISE_GE_EXCEPTION(e);
//     }
//   }
//   else if(OZ_isBoolVarArgs(OZ_in(0)) && OZ_isGeSetVar(OZ_in(1))){
//     DECLARE_BOOLVARARGS(0, __x, home);
//     DeclareGeSetVar(1, __y, home);
//     try{
//       Gecode::channel(home, __x, __y);
//     }
//     catch(Exception e){
//       RAISE_GE_EXCEPTION(e);
//     }
//   }
//   else{
//     OZ_typeError(0, "Malformed Propagator");
//   }
//   CHECK_POST(home);
// }OZ_BI_end

// OZ_BI_define(gfs_cardinality_2,2,0){
//   DeclareGSpace(home);
//   DeclareGeSetVar(0, __s, home);
//   DeclareGeIntVar(1, __x, home);
//   try{
//     Gecode::cardinality(home, __s, __x);
//   }
//   catch(Exception e){
//     RAISE_GE_EXCEPTION(e);
//   }
//   CHECK_POST(home);
// }OZ_BI_end

// OZ_BI_define(gfs_max_2,2,0){
//   DeclareGSpace(home);
//   DeclareGeSetVar(0, __s, home);
//   DeclareGeIntVar(1, __x, home);
//   try{
//     Gecode::max(home, __s, __x);
//   }
//   catch(Exception e){
//     RAISE_GE_EXCEPTION(e);
//   }
//   CHECK_POST(home);
// }OZ_BI_end

// OZ_BI_define(gfs_weights_4,4,0){
//   DeclareGSpace(home);
//   DECLARE_INTARGS(0, __elements);
//   DECLARE_INTARGS(1, __weights);
//   DeclareGeSetVar(2, __x, home);
//   DeclareGeIntVar(3, __y, home);
//   try{
//     Gecode::weights(home, __elements, __weights, __x, __y);
//   }
//   catch(Exception e){
//     RAISE_GE_EXCEPTION(e);
//   }
//   CHECK_POST(home);
// }OZ_BI_end

// /**
//     Selection constraints
// */
// OZ_BI_define(gfs_selectUnion_3,3,0){
//   DeclareGSpace(home);
//   DeclareGeSetVar(1, __y, home);
//   DeclareGeSetVar(2, __z, home);
//   if(OZ_isSetVarArgs(OZ_in(0)) && OZ_isGeSetVar(OZ_in(1)) && OZ_isGeSetVar(OZ_in(2))){
//     DECLARE_SETVARARGS(0, __x, home);
//     try{
//       Gecode::selectUnion(home, __x, __y, __z);
//     }
//     catch(Exception e){
//       RAISE_GE_EXCEPTION(e);
//     }
//   }
//   else if(OZ_isIntSetArgs(OZ_in(0)) && OZ_isGeSetVar(OZ_in(1)) && OZ_isGeSetVar(OZ_in(2))){
//     DECLARE_INT_SET_ARGS(0, __s);
//     try{
//       Gecode::selectUnion(home, __s, __y, __z);
//     }
//     catch(Exception e){
//       RAISE_GE_EXCEPTION(e);
//     }
//   }
//   else{
//     OZ_typeError(0, "Malformed Propagator");
//   }
//   CHECK_POST(home);
// }OZ_BI_end

// OZ_BI_define(gfs_selectInterIn_4,4,0){
//   DeclareGSpace(home);
//   DECLARE_SETVARARGS(0, __x, home);
//   DeclareGeSetVar(1, __y, home);
//   DeclareGeSetVar(2, __z, home);
//   DECLARE_INT_SET(3, __universe);
//   try{
//     Gecode::selectInterIn(home, __x, __y, __z, __universe);
//   }
//   catch(Exception e){
//     RAISE_GE_EXCEPTION(e);
//   }
//   CHECK_POST(home);
// }OZ_BI_end

// OZ_BI_define(gfs_selectSet_3,3,0){
//   DeclareGSpace(home);
//   DeclareGeIntVar(1, __y, home);
//   DeclareGeSetVar(2, __z, home);
//   if(OZ_isSetVarArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isGeSetVar(OZ_in(2))){
//     DECLARE_SETVARARGS(0, __x, home);
//     try{
//       Gecode::selectSet(home, __x, __y, __z);
//     }
//     catch(Exception e){
//       RAISE_GE_EXCEPTION(e);
//     }
//   }
//   else if(OZ_isIntSetArgs(OZ_in(0)) && OZ_isGeIntVar(OZ_in(1)) && OZ_isGeSetVar(OZ_in(2))){
//     DECLARE_INT_SET_ARGS(0, __s);
//     try{
//       Gecode::selectSet(home, __s, __y, __z);
//     }
//     catch(Exception e){
//       RAISE_GE_EXCEPTION(e);
//     }
//   }
//   else{
//     OZ_typeError(0, "Malformed Propagator");
//   }
//   CHECK_POST(home);
// }OZ_BI_end

// OZ_BI_define(gfs_selectInter_3,3,0){
//   DeclareGSpace(home);
//   DECLARE_SETVARARGS(0, __x, home);
//   DeclareGeSetVar(1, __y, home);
//   DeclareGeSetVar(2, __z, home);
//   try{
//     Gecode::selectInter(home, __x, __y, __z);
//   }
//   catch(Exception e){
//     RAISE_GE_EXCEPTION(e);
//   }
//   CHECK_POST(home);
// }OZ_BI_end

// OZ_BI_define(gfs_selectDisjoint_2,2,0){
//   DeclareGSpace(home);
//   DECLARE_SETVARARGS(0, __x, home);
//   DeclareGeSetVar(1, __y, home);
//   try{
//     Gecode::selectDisjoint(home, __x, __y);
//   }
//   catch(Exception e){
//     RAISE_GE_EXCEPTION(e);
//   }
//   CHECK_POST(home);
// }OZ_BI_end

#endif
