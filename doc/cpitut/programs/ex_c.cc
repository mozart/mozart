#ifndef NDEBUG
#include <stdio.h>
#endif

#include "mozart_cpi.hh"

void fd_start(void) {;}

class TwiceProp : public OZ_Propagator {
private:
  static OZ_PropagatorProfile profile;
  OZ_Term _x, _z;
public:
  TwiceProp(OZ_Term a, OZ_Term b)
   : _x(a), _z(b) {}
  virtual OZ_Return propagate(void);

  virtual size_t sizeOf(void) {
    return sizeof(TwiceProp);
  }
  virtual void updateHeapRefs(OZ_Boolean) {
    OZ_updateHeapTerm(_x);
    OZ_updateHeapTerm(_z);
  }
  virtual OZ_Term getParameters(void) const {
    return OZ_cons(_x,
                   OZ_cons(_z,
                           OZ_nil()));
  }
  virtual OZ_PropagatorProfile *getProfile(void) const {
    return &profile;
  }
};

OZ_PropagatorProfile TwiceProp::profile;

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
  virtual void updateHeapRefs(OZ_Boolean) {
    OZ_updateHeapTerm(_x);
    OZ_updateHeapTerm(_y);
    OZ_updateHeapTerm(_z);
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

OZ_PropagatorProfile AddProp::profile;


#define ReplaceOnUnify(EQ01, EQ02, EQ12) \
  if (mayBeEqualVars()) {                \
    if (OZ_isEqualVars(_x, _y)) {        \
      return (EQ01);                     \
    }                                    \
    if (OZ_isEqualVars(_x, _z)) {        \
      return (EQ02);                     \
    }                                    \
    if (OZ_isEqualVars(_y, _z)) {        \
      return (EQ12);                     \
    }                                    \
  }

#define FailOnEmpty(X) if((X) == 0) goto failure;

OZ_Return AddProp::propagate(void)
{

  ReplaceOnUnify(replaceBy(new TwiceProp(_x, _z)),
                 replaceByInt(_y, 0),
                 replaceByInt(_x, 0));

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


OZ_Return TwiceProp::propagate(void)
{
  OZ_FDIntVar x(_x), z(_z);

  OZ_FiniteDomain x_aux(fd_empty), z_aux(fd_empty);

  for (int i = x->getMinElem(); i != -1;
       i = x->getNextLargerElem(i)) {
    int i2 = 2 * i;
    if (z->isIn(i2)) {
      x_aux += i; z_aux += i2;
    }
  }

  FailOnEmpty(*x &= x_aux);
  FailOnEmpty(*z &= z_aux);

  return (x.leave() | z.leave())
    ? OZ_SLEEP : OZ_ENTAILED;

failure:
  x.fail(); z.fail();
  return OZ_FAILED;
}


OZ_C_proc_begin(fd_add, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD","OZ_EM_FD","OZ_EM_FD);

  OZ_Expect pe;

  OZ_EXPECT(pe, 0, expectIntVar);
  OZ_EXPECT(pe, 1, expectIntVar);
  OZ_EXPECT(pe, 2, expectIntVar);

  return pe.impose(new AddProp(OZ_args[0],
                               OZ_args[1],
                               OZ_args[2]));
}
OZ_C_proc_end

OZ_C_proc_begin(fd_twice, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD","OZ_EM_FD);

  OZ_Expect pe;

  OZ_EXPECT(pe, 0, expectIntVar);
  OZ_EXPECT(pe, 1, expectIntVar);

  return pe.impose(new TwiceProp(OZ_args[0],
                                 OZ_args[1]));
}
OZ_C_proc_end


OZ_C_proc_begin(fd_init, 0)
{
#ifndef NDEBUG
  printf("fd_start=0x%p\n", (void *) fd_start);
  fflush(stdout);
#endif
  return PROCEED;
}
OZ_C_proc_end


OZ_C_proc_begin(fd_add_nestable, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD","OZ_EM_FD","OZ_EM_FD);

  OZ_Expect pe;
  int susp_count = 0;

  OZ_EXPECT_SUSPEND(pe,0,expectIntVar,susp_count);
  OZ_EXPECT_SUSPEND(pe,1,expectIntVar,susp_count);
  OZ_EXPECT_SUSPEND(pe,2,expectIntVar,susp_count);

  if (susp_count > 1)
    return pe.suspend(OZ_makeSelfSuspendedThread());

  return pe.impose(new AddProp(OZ_args[0],
                               OZ_args[1],
                               OZ_args[2]));
}
OZ_C_proc_end


class ExtendedExpect : public OZ_Expect {
public:
  OZ_expect_t expectIntVarSingl(OZ_Term t) {
    return expectIntVar(t, fd_prop_singl);
  }

private:
  OZ_expect_t _expectIntVarAny(OZ_Term t) {
    return expectIntVar(t, fd_prop_any);
  }
public:
  OZ_expect_t expectVectorIntVarAny(OZ_Term t) {
    return expectVector(t,
              (OZ_ExpectMeth) &_expectIntVarAny);
  }
  OZ_expect_t expectVectorIntVarSingl(OZ_Term t) {
    return expectVector(t,
              (OZ_ExpectMeth) &expectIntVarSingl);
  }

};

class ElementProp : public OZ_Propagator {
private:
  static OZ_PropagatorProfile profile;
  OZ_Term _n, _v, * _d;
  int _d_size;
public:
  ElementProp(OZ_Term n, OZ_Term d, OZ_Term v)
    : _n(n), _v(v), _d_size (OZ_vectorSize(d))
  {
    _d =  OZ_hallocOzTerms(_d_size);
    OZ_getOzTermVector(d, _d);
  }

  virtual OZ_Return propagate(void);

  virtual size_t sizeOf(void) {
    return sizeof(ElementProp);
  }
  virtual OZ_PropagatorProfile *getProfile(void) const {
    return &profile;
  }
  virtual OZ_Term getParameters(void) const;
  virtual void updateHeapRefs(OZ_Boolean);
};

OZ_PropagatorProfile ElementProp::profile;

OZ_Term ElementProp::getParameters(void) const {
  OZ_Term list = OZ_nil();

  for (int i = _d_size; i--; )
    list = OZ_cons(_d[i], list);

  return OZ_cons(_n,
                 OZ_cons(list,
                         OZ_cons(_v, OZ_nil())));
}

void ElementProp::updateHeapRefs(OZ_Boolean) {
  OZ_updateHeapTerm(_n); OZ_updateHeapTerm(_v);

  OZ_Term * new_d = OZ_hallocOzTerms(_d_size);
  for (int i = _d_size; i--; ) {
    new_d[i] = _d[i];
    OZ_updateHeapTerm(new_d[i]);
  }
  _d = new_d;
}

OZ_C_proc_begin(fd_element, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD
                   ","OZ_EM_VECT OZ_EM_FD
                   ","OZ_EM_FD);

  ExtendedExpect pe;

  OZ_EXPECT(pe, 0, expectIntVar);
  OZ_EXPECT(pe, 1, expectVectorIntVarAny);
  OZ_EXPECT(pe, 2, expectIntVar);

  if (OZ_vectorSize(OZ_args[1]) == 0)
    return pe.fail();

  return pe.impose(new ElementProp(OZ_args[0],
                                   OZ_args[1],
                                   OZ_args[2]));
}
OZ_C_proc_end


class Iterator_OZ_FDIntVar {
private:
  int _l_size;
  OZ_FDIntVar * _l;
public:
  Iterator_OZ_FDIntVar(int s, OZ_FDIntVar * l)
    : _l_size(s), _l(l) { }

  OZ_Boolean leave(void) {
    OZ_Boolean vars_left = OZ_FALSE;
    for (int i = _l_size; i--; )
      vars_left |= _l[i].leave();
    return vars_left;
  }
  void fail(void) {
    for (int i = _l_size; i--; _l[i].fail());
  }
};

OZ_Return ElementProp::propagate(void)
{
  OZ_FDIntVar n(_n), v(_v), d[_d_size];
  Iterator_OZ_FDIntVar D(_d_size, d);

  for (int i = _d_size; i--; )
    d[i].read(_d[i]);

  { /* propagation rule for n */
    OZ_FiniteDomain aux_n(fd_empty);

    for (int i = _d_size; i --; )
      if ((*(d[i]) & *v) != fd_empty)
        aux_n += (i + 1);

    FailOnEmpty(*n &= aux_n);
  }
  { /* propagation rule for v */
    OZ_FiniteDomain aux_v(fd_empty);

    for (int i = n->getMinElem();
         i != -1;
         i = n->getNextLargerElem(i))
      aux_v = aux_v | *(d[i - 1]);

    FailOnEmpty(*v &= aux_v);
  }
  { /* propagation rule for d[n] */
    if (n->getSize() == 1) {
      int o = n->getSingleElem();
      D.leave(); n.leave(); v.leave();
      return replaceBy(_v, _d[o - 1]);
    }
  }
  return (D.leave() | n.leave() | v.leave())
    ? OZ_SLEEP : OZ_ENTAILED;

failure:
  D.fail(); n.fail(); v.fail();
  return OZ_FAILED;
}

OZ_BI_proto(fd_add);
OZ_BI_proto(fd_twice);
OZ_BI_proto(fd_init);
OZ_BI_proto(fd_nestable);
OZ_BI_proto(fd_element);

extern "C"
{

  OZ_C_proc_interface *oz_init_module(void)
  {
    static OZ_C_proc_interface i_table[] = {
      {"init", 0, 0, fd_init},
      {"add", 3, 0, fd_add},
      {"twice", 2, 0, fd_twice},
      {"add_nestable", 3, 0, fd_add_nestable},
      {"element", 3, 0, fd_element},
      {0,0,0,0}
    };

    printf("propagators loaded\n");
    return i_table;
  }
}
