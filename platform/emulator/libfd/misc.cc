#include "misc.hh"
#include "rel.hh"
#include "auxcomp.hh"


//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdtest_spawnLess, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectIntVarAny);
  OZ_EXPECT(pe, 1, expectIntVarAny);

  return pe.spawn(new SpawnLess(OZ_args[0], OZ_args[1]));
}
OZ_C_proc_end

OZ_CFun SpawnLess::spawner = fdtest_spawnLess;

OZ_Return SpawnLess::run(void)
{
  cout << "spawn less count down: " << c << endl << flush;

  c -= 1;

  if (!c) {
    cout << "Spawning less!!!" << endl << flush;
    addSpawn(fd_bounds, a);
    addSpawn(fd_bounds, b);
    spawn(new Less(a, b));
    return ENTAILED;
  }

  return SLEEP;
}


OZ_Return Less::run(void)
{
  OZ_DEBUGPRINT("in " << *this);

  if (mayBeEqualVars() && OZ_isEqualVars(_x, _y))
    return PROCEED;

  OZ_FDIntVar x(_x), y(_y);
  PropagatorController_V_V P(x, y);

  FailOnEmpty(*x <= (y->getMaxElem())-1);
  FailOnEmpty(*y >= (x->getMinElem())+1);

  if (x->getMaxElem() < y->getMinElem()) return P.vanish();
  if (x->getMinElem() > y->getMaxElem()) goto failure;

  OZ_DEBUGPRINT("out " << *this);

  return P.leave();

failure:
  OZ_DEBUGPRINT("fail");

  return P.fail();
}

OZ_C_proc_begin(fdtest_counter, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_INT "," OZ_EM_STREAM);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectInt);
  OZ_EXPECT(pe, 1, expectStream);

  return pe.spawn(new Counter(OZ_args[0], OZ_args[1]));
}
OZ_C_proc_end

OZ_CFun Counter::spawner = fdtest_counter;

OZ_Return Counter::run(void)
{
  OZ_DEBUGPRINT("in " << *this);

  OZ_Stream stream(s);

  while (!stream.isEostr()) {
    OZ_Term e = stream.get();
    if (OZ_isAtom(e)) {
      char * a = OZ_atomToC(e);

      if (! strcmp("inc", a)) {
        c += 1;
      } else if (! strcmp ("dec", a)) {
        c -= 1;
      } else {
        goto failure;
      }
    } else if (OZ_isTuple(e)) {
      char * l = OZ_atomToC(OZ_label(e));

      if (! strcmp("get", l)) {
        if (OZ_unify(OZ_int(c), OZ_getArg(e, 0)) == FAILED)
          goto failure;
      } else if (! strcmp ("put", l)) {
        OZ_Term e_0 = OZ_getArg(e, 0);
        if (OZ_isSmallInt(e_0)) {
          c = OZ_intToC(OZ_getArg(e, 0));
        } else if (OZ_isVariable(e_0)) {
          postOn(e_0);
          return SLEEP;
        } else {
          goto failure;
        }
      } else {
        goto failure;
      }

    } else {
      goto failure;
    }
  }
  if (!stream.isValid())
    goto failure;

  s = stream.getTail();
  return stream.leave() ? SLEEP : PROCEED;

failure:
  stream.fail();
  return FAILED;
}



FirstFail::FirstFail(OZ_Term l, OZ_Term st)
{
  stream = st;
  reg_fds = vectorToOzTerms(l, size);
}

OZ_C_proc_begin(fdtest_firstFail, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FD "," OZ_EM_STREAM);

  PropagatorExpect pe;

  pe.collectVarsOff();
  OZ_EXPECT(pe, 0, expectVectorIntVarAny);
  pe.collectVarsOn();

  OZ_EXPECT(pe, 1, expectStream);

  return pe.spawn(new FirstFail(OZ_args[0], OZ_args[1]));
}
OZ_C_proc_end

void FirstFail::updateHeapRefs(OZ_Boolean)
{
  OZ_updateHeapTerm(stream);

  OZ_Term * new_reg_fds = OZ_hallocOzTerms(size);

  for (int i = size; i--; ) {
    new_reg_fds[i] = reg_fds[i];
    OZ_updateHeapTerm(new_reg_fds[i]);
  }
  reg_fds = new_reg_fds;
}

OZ_CFun FirstFail::spawner = fdtest_firstFail;

OZ_Return FirstFail::run(void)
{
  OZ_DEBUGPRINT("in " << *this);

  OZ_Stream st(stream);

  while (!st.isEostr()) {
    OZ_Term e = st.get();
    if (OZ_isTuple(e)) {
      char * l = OZ_atomToC(OZ_label(e));
      if (! strcmp("dist", l)) {
        int last = 0;
        int current = 0;
        int smallest;
        OZ_FDIntVar var;
        do {
          var.ask(reg_fds[current]);
        } while ((*var == fd_singleton) && (++current < size));

        // No elements left
        if (current==size) {
          size = 0;
          if (OZ_unify(OZ_getArg(e, 0), OZ_int(-1)) == FAILED)
            goto failure;
        }
        else {
            if (current!=0)
              reg_fds[0] = reg_fds[current];
            int minsize = var->getSize();
            int new_cur = 1;
            int minElem = 0;

            for (int i=current+1; i<size; i++) {
              var.ask(reg_fds[i]);
              if (*var == fd_singleton)
                continue;
              if (i != new_cur)
                reg_fds[new_cur] = reg_fds[i];

              int cursize = var->getSize();

              if (cursize < minsize) {
                minsize    = cursize;
                minElem = new_cur;
              }
              new_cur++;

            }
            size = new_cur;

            if (OZ_unify(reg_fds[minElem], OZ_getArg(e, 0)) == FAILED)
              goto failure;
        }
      } else {
        goto failure;
      }

    } else {
      goto failure;
    }
  }
  if (!st.isValid())
    goto failure;

  stream = st.getTail();
  return st.leave() ? SLEEP : PROCEED;

failure:
  st.fail();
  return FAILED;
}

//-----------------------------------------------------------------------------


class DPlusPropagator : public OZ_Propagator {
private:
  static OZ_CFun spawner;
  OZ_Term _x, _y, _z;
public:
  DPlusPropagator(OZ_Term a, OZ_Term b, OZ_Term c)
   : _x(a), _y(b), _z(c) {}
  virtual OZ_Return run(void);

  virtual size_t sizeOf(void) {
    return sizeof(DPlusPropagator);
  }
  virtual void updateHeapRefs(OZ_Boolean) {
    OZ_updateHeapTerm(_x); OZ_updateHeapTerm(_y); OZ_updateHeapTerm(_z);
  }
  virtual OZ_Term getArguments(void) const {
    return OZ_cons(_x, OZ_cons(_y, OZ_cons(_z, OZ_nil())));
  }
  virtual OZ_CFun getSpawner(void) const { return spawner; }
};


OZ_C_proc_begin(fdtest_plus, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD "," OZ_EM_FD);

  OZ_Expect pe;

  OZ_EXPECT(pe, 0, expectIntVarAny);
  OZ_EXPECT(pe, 1, expectIntVarAny);
  OZ_EXPECT(pe, 2, expectIntVarAny);

  return pe.spawn(new DPlusPropagator(OZ_args[0],
                                      OZ_args[1],
                                      OZ_args[2]));
}
OZ_C_proc_end

OZ_CFun DPlusPropagator::spawner = fdtest_plus;

OZ_Return DPlusPropagator::run(void)
{
  OZ_FDIntVar x(_x), y(_y), z(_z);
  PropagatorController_V_V_V P(x, y, z);

  OZ_FiniteDomain x_aux(fd_empty),
                  y_aux(fd_empty),
                  z_aux(fd_empty);

  int i, j;
  for (i = x->getMinElem(); i != -1; i = x->getNextLargerEl(i)) {
    for (j = y->getMinElem(); j != -1; j = y->getNextLargerEl(j)) {
      if (z->isIn(i + j)) {
        x_aux += i;
        y_aux += j;
        z_aux += (i + j);
      }
    }
  }

  FailOnEmpty(*x &= x_aux);
  FailOnEmpty(*y &= y_aux);
  FailOnEmpty(*z &= z_aux);

  return (x.leave() | y.leave() | z.leave())
    ? SLEEP : PROCEED;

failure:
  x.fail();
  y.fail();
  z.fail();
  return FAILED;
}
