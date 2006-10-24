#include "mozart.h"

class Celloid : public OZ_Extension {
public:
  OZ_Term content;
  Celloid(OZ_Term t):content(t){}
  static int id;
  virtual int getIdV() { return id; }
  virtual OZ_Term typeV() { return OZ_atom("celloid"); }
  virtual OZ_Extension* gcV();
  virtual void gcRecurseV();
  virtual OZ_Term printV(int depth = 10);
  virtual OZ_Extension * gCollectV();
  virtual void gCollectRecurseV();
  virtual OZ_Extension * sCloneV();
  virtual void sCloneRecurseV();
};

OZ_BI_define(celloid_new,1,1)
{
  OZ_declareTerm(0,t);
  OZ_RETURN(OZ_extension(new Celloid(t)));
}
OZ_BI_end

int Celloid::id;

inline OZ_Boolean OZ_isCelloid(OZ_Term t)
{
  t = OZ_deref(t);
  return OZ_isExtension(t) &&
    OZ_getExtension(t)->getIdV()==Celloid::id;
}

OZ_BI_define(celloid_is,1,1)
{
  OZ_declareDetTerm(0,t);
  OZ_RETURN_BOOL(OZ_isCelloid(t));
}
OZ_BI_end

inline Celloid* OZ_CelloidToC(OZ_Term t)
{
  return (Celloid*) OZ_getExtension(OZ_deref(t));
}

#define OZ_declareCelloid(ARG,VAR) \
OZ_declareType(ARG,VAR,Celloid*,"celloid",OZ_isCelloid,OZ_CelloidToC)

OZ_BI_define(celloid_access,1,1)
{
  OZ_declareCelloid(0,c);
  OZ_RETURN(c->content);
}
OZ_BI_end

OZ_BI_define(celloid_assign,2,0)
{
  OZ_declareCelloid(0,c);
  OZ_declareTerm(1,t);
  if (c->isLocal()) { c->content = t; return PROCEED; }
  else return OZ_raiseErrorC("celloid",3,OZ_atom("nonLocal"),
			     OZ_in(0),OZ_in(1));
}
OZ_BI_end

OZ_Term Celloid::printV(int depth = 10)
{
  return OZ_atom("<celloid>");
}

OZ_Extension * Celloid::gCollectV() { return new Celloid(content); }
void Celloid::gCollectRecurseV() { OZ_gCollect(&content); }
OZ_Extension * Celloid::sCloneV() { return new Celloid(content); }
void Celloid::sCloneRecurseV() { OZ_sClone(&content); }

OZ_C_proc_interface * oz_init_module(void)
{
  static OZ_C_proc_interface table[] = {
    {"new",1,1,celloid_new},
    {"is",1,1,celloid_is},
    {"access",1,1,celloid_access},
    {"assign",2,0,celloid_assign},
    {0,0,0,0}
  };
  Celloid::id = OZ_getUniqueId();
  return table;
}
