#ifndef NDEBUG
#include <stdio.h>
#endif

#include "mozart_cpi.hh"

OZ_BI_proto(fd_init);
OZ_BI_proto(fd_add);

void fd_start(void) {;}

class AddProp : public OZ_Propagator {
private:
  static OZ_PropagatorProfile profile;
  OZ_Term _x, _y, _z;
public:
  AddProp(OZ_Term a, OZ_Term b, OZ_Term c) 
   : _x(a), _y(b), _z(c) {}
  virtual OZ_Return propagate(void);

  virtual size_t sizeOf(void) { 
    return sizeof(AddProp); 
  }
  virtual void gCollect(void) { 
    OZ_gCollectTerm(_x); 
    OZ_gCollectTerm(_y); 
    OZ_gCollectTerm(_z);
  } 
  virtual void sClone(void) { 
    OZ_sCloneTerm(_x); 
    OZ_sCloneTerm(_y); 
    OZ_sCloneTerm(_z);
  } 
  virtual OZ_Term getParameters(void) const {
    return OZ_cons(_x, 
		   OZ_cons(_y, 
			   OZ_cons(_z, 
				   OZ_nil())));
  }
  virtual OZ_PropagatorProfile *getProfile(void) const { 
    return &profile; 
  }
};

#define FailOnEmpty(X) if((X) == 0) goto failure;

OZ_PropagatorProfile AddProp::profile;

OZ_Return AddProp::propagate(void)
{

  OZ_FDIntVar x(_x), y(_y), z(_z);
  
  OZ_FiniteDomain x_aux(fd_empty), 
                  y_aux(fd_empty), 
                  z_aux(fd_empty);

  for (int i = x->getMinElem(); i != -1; 
       i = x->getNextLargerElem(i))
    for (int j = y->getMinElem(); j != -1; 
	 j = y->getNextLargerElem(j))
      if (z->isIn(i + j)) {
	x_aux += i;
	y_aux += j; 
	z_aux += (i + j);
      }
  
  FailOnEmpty(*x &= x_aux);
  FailOnEmpty(*y &= y_aux);
  FailOnEmpty(*z &= z_aux);
  
  return (x.leave() | y.leave() | z.leave()) 
    ? OZ_SLEEP : OZ_ENTAILED;

failure: 
  x.fail();
  y.fail();
  z.fail();
  return OZ_FAILED;
}


OZ_BI_define(fd_add, 3, 0)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD","OZ_EM_FD","OZ_EM_FD);

  OZ_Expect pe;

  OZ_EXPECT(pe, 0, expectIntVar);
  OZ_EXPECT(pe, 1, expectIntVar);
  OZ_EXPECT(pe, 2, expectIntVar);

  return pe.impose(new AddProp(OZ_in(0), 
			       OZ_in(1), 
			       OZ_in(2)));
}
OZ_BI_end

OZ_BI_define(fd_init, 0, 0)
{
#ifndef NDEBUG
  printf("fd_start=0x%p\n", (void *) fd_start); 
  fflush(stdout);
#endif
  return PROCEED;
} 
OZ_BI_end

OZ_C_proc_interface *oz_init_module(void)
{
  static OZ_C_proc_interface i_table[] = {
    {"init", 0, 0, fd_init},
    {"add", 3, 0, fd_add},
    {0,0,0,0}
  };

  printf("addition propagator loaded\n");
  return i_table;
}
