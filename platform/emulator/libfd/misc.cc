#include "misc.hh"
#include "rel.hh"
#include "auxcomp.hh"

//-----------------------------------------------------------------------------

// #define NDEBUG 1

class ExtendedExpect : public OZ_Expect
{
 public:
  OZ_expect_t expectIntVarAny(OZ_Term t)
   {return expectIntVar(t);}
  OZ_expect_t expectIntVarSingl(OZ_Term t)
   {return expectIntVar(t, fd_prop_singl);}
  OZ_expect_t expectIntVarMinMax(OZ_Term t)
   {return expectIntVar(t, fd_prop_bounds);}
  OZ_expect_t expectVectorInt(OZ_Term t)
   {return expectVector(t, &expectInt);}
  OZ_expect_t expectVectorIntVarAny(OZ_Term t)
   {return expectVector(t, (OZ_ExpectMeth) &expectIntVarAny);}
  OZ_expect_t expectVectorIntVarSingl(OZ_Term t)
   {return expectVector(t, (OZ_ExpectMeth) &expectIntVarSingl);}
  OZ_expect_t expectVectorIntVarMinMax(OZ_Term t)
   {return expectVector(t, (OZ_ExpectMeth) &expectIntVarMinMax);}
};

class SumACProp : public OZ_Propagator
{
 private:
  static OZ_CFunHeader spawner;
  int c,size;
  OZ_Term *_a,*_x,_d;
 public:
  SumACProp(OZ_Term a, OZ_Term x, OZ_Term d)
  :size(OZ_vectorSize(x)),_d(d)
   {
    _a = OZ_hallocOzTerms(size);
    _x = OZ_hallocOzTerms(size);
    OZ_getOzTermVector(a, _a);
    OZ_getOzTermVector(x, _x);
    c=0;
   }
  virtual OZ_Return propagate(void);
  virtual size_t sizeOf(void) {return sizeof(SumACProp);}
  virtual void updateHeapRefs(OZ_Boolean)
   {
    OZ_updateHeapTerm(_d);
    OZ_Term *new_a=OZ_hallocOzTerms(size),
            *new_x=OZ_hallocOzTerms(size);
    for(int i=size; i--;)
     {
      new_a[i]=_a[i];
      OZ_updateHeapTerm(new_a[i]);
      new_x[i]=_x[i];
      OZ_updateHeapTerm(new_x[i]);
     }
    _a=new_a;
    _x=new_x;
   }
  virtual OZ_Term getParameters(void) const
   {
    OZ_Term _a_list=OZ_nil(),_x_list=OZ_nil();
    for(int i=size;i--;)
     {
      _a_list=OZ_cons(_a[i],_a_list);
      _x_list=OZ_cons(_x[i],_x_list);
     }
    return OZ_cons(_a_list, OZ_cons(_x_list, OZ_cons(_d, OZ_nil())));
   }
  virtual OZ_CFunHeader * getHeader(void) const {return &spawner;}
  friend int simplify(int*,int*,OZ_Term*,OZ_Term,int*);
};

int simplify(int *size,int *a,OZ_Term *x,OZ_Term d,int *c)
{ // gibt Position von d im Vektor zur"uck
 if (size==0) return -1;
 DECL_DYN_ARRAY(OZ_Term,xd,*size+1);
 int j;
 for(j=*size;j--;) xd[j]=x[j];
 xd[*size]=d;
 int *is=OZ_findEqualVars(*size+1,xd);
 int dpos=is[*size];
 for (j=*size;j--;)
  {
   if(is[j]==-1)       // singleton in x
    {
     *c+=int(OZ_intToC(x[j])*NUMBERCAST(a[j]));
     x[j]=0;
    }
   else if(j!=is[j])    // multiple appearing var in x
    {
     a[is[j]]+=a[j];
     x[j]=0;
    }
  }

 int from=0,to=0;
 for (;from<*size;from++)
  {
   if(x[from]!=0 && a[from]!=0)
    {
     if(from!=to)
      {
       a[to]=a[from];
       x[to]=x[from];
      }
     to++;
    }
   else if(dpos!=*size && from<dpos) dpos--;
  }
 *size=to;
 return dpos;
}


OZ_Return SumACProp::propagate(void)
{
 int summax,summin,axmax,axmin,dmax,dmin,dpos,i,j,k,dummy;
 double bound1,bound2;
 OZ_FiniteDomain d_aux_neg,d_aux_pos;
 OZ_Boolean klausel,unified=OZ_FALSE,changed,vars_left;

 DECL_DYN_ARRAY(int,a,size);
 for(j=size; j--;) a[j]=OZ_intToC(_a[j]);

 dpos=simplify(&size,a,_x,_d,&c);
 if(dpos>-1 && dpos<size) unified=OZ_TRUE;

 OZ_FDIntVar d(_d);
 DECL_DYN_ARRAY(OZ_FDIntVar,x,size);
 DECL_DYN_ARRAY(OZ_FiniteDomain,x_aux_neg,size);
 DECL_DYN_ARRAY(OZ_FiniteDomain,x_aux_pos,size);

 for(j=size; j--;)
  {
   x[j].read(_x[j]);
   x_aux_neg[j]=x_aux_pos[j]=*x[j];
  }

 if(unified)
  {
   a[dpos]--;

   /****************************************/

   printf("%s %s\n",x[dpos]->toString(),d->toString());
   *(x[dpos])&=*d;

   /****************************************/

   d_aux_neg.initSingleton(0);
   d_aux_pos.initSingleton(0);
  }
 else d_aux_neg=d_aux_pos=*d;


 klausel=OZ_FALSE;
 do
  {
   changed=OZ_FALSE;
   summin=summax=(klausel ? -c : c);

   for(j=size; j--;)
    {
     axmax=int(NUMBERCAST(a[j])*
           (klausel ? x_aux_neg[j].getMaxElem() : x_aux_pos[j].getMaxElem()));
     axmin=int(NUMBERCAST(a[j])*
           (klausel ? x_aux_neg[j].getMinElem() : x_aux_pos[j].getMinElem()));
     if(a[j]<0)
      {
       summin+=axmax;
       summax+=axmin;
      }
     if(a[j]>0)
      {
       summin+=axmin;
       summax+=axmax;
      }
    }
   dummy=(klausel ? d_aux_neg.getSize() : d_aux_pos.getSize());
   (klausel ? d_aux_neg : d_aux_pos)>=summin;
   (klausel ? d_aux_neg : d_aux_pos)<=summax;

 #ifndef NDEBUG
   printf("\n");
   printf("%s\n", klausel ? "negativ:" : "positiv:");
   printf("summin=%d summax=%d\n",summin,summax);
   printf("%d<=d<=%d\n",summin,summax);
   printf("d_%s%s\n",
          (klausel ? "neg=" : "pos="),
          (klausel ? d_aux_neg : d_aux_pos).toString());
 #endif

   changed|=!(dummy==(klausel ? d_aux_neg.getSize() : d_aux_pos.getSize()));
   dmax=(klausel ? d_aux_neg.getMaxElem() : d_aux_pos.getMaxElem());
   dmin=(klausel ? d_aux_neg.getMinElem() : d_aux_pos.getMinElem());

   for(j=size;j--;)
    {
     summin=summax=(klausel ? -c : c);
     for(k=size;k--;)
      if(j!=k)
       {
        axmax=int(NUMBERCAST(a[k])*
           (klausel ? x_aux_neg[k].getMaxElem() : x_aux_pos[k].getMaxElem()));
        axmin=int(NUMBERCAST(a[k])*
           (klausel ? x_aux_neg[k].getMinElem() : x_aux_pos[k].getMinElem()));
        if(a[k]<0)
         {
          summin+=axmax;
          summax+=axmin;
         }
        if(a[k]>0)
         {
          summin+=axmin;
          summax+=axmax;
         }
       }
     bound1=(dmin-summax)/double(a[j]);
     bound2=(dmax-summin)/double(a[j]);
     dummy=(klausel ? x_aux_neg[j].getSize() : x_aux_pos[j].getSize());
     if(a[j]<0)
      {
       (klausel ? x_aux_neg[j] : x_aux_pos[j])>=int(ceil (bound2));
       (klausel ? x_aux_neg[j] : x_aux_pos[j])<=int(floor(bound1));
      }
     if(a[j]>0)
      {
       (klausel ? x_aux_neg[j] : x_aux_pos[j])>=int(ceil (bound1));
       (klausel ? x_aux_neg[j] : x_aux_pos[j])<=int(floor(bound2));
      }
     changed|=!
      (dummy==(klausel ? x_aux_neg[j].getSize() : x_aux_pos[j].getSize()));

 #ifndef NDEBUG
     printf("summin=%d summax=%d\n",summin,summax);
     if(a[j]<0)
       printf("%d<=x%d<=%d\n",int(ceil (bound2)),j,int(floor(bound1)));
     if(a[j]>0)
      printf("%d<=x%d<=%d\n",int(ceil (bound1)),j,int(floor(bound2)));
     printf("a%d=%d x%d%s%s\n",j,a[j],j,(klausel ? "_neg=" : "_pos="),
            (klausel ? x_aux_neg[j] : x_aux_pos[j]).toString());
 #endif

    }
   if(!changed && !klausel)
    {
     klausel=changed=OZ_TRUE;
     if(unified) a[dpos]+=2;        // a[dpos]++, Vorz. umk., a[dpos]--
     for(j=size; j--;) a[j]=-a[j];
    }
  }
 while(changed);

 for(j=size; j--;) FailOnEmpty(*x[j]&=x_aux_neg[j]|x_aux_pos[j]);
 FailOnEmpty(*d&=(unified ? *x[dpos] : d_aux_neg|d_aux_pos));
 vars_left=d.leave();
 for(j=size; j--;) vars_left|=x[j].leave();
 return (vars_left ? SLEEP : OZ_ENTAILED); // Iteratorclass

 failure:
 for(j=size;j--;) x[j].fail();
 d.fail();
 return FAILED;
}

OZ_C_proc_begin(fdtest_sumac, 3)
{
 OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_INT","OZ_EM_VECT OZ_EM_FD","OZ_EM_FD);

 #ifndef NDEBUG
  printf("fdtest_sumac=0x%x\n",fdtest_sumac);
 #endif

 ExtendedExpect pe;
 OZ_EXPECT(pe, 0, expectVectorInt);
 OZ_EXPECT(pe, 1, expectVectorIntVarMinMax);
 OZ_EXPECT(pe, 2, expectIntVarMinMax);

 if(OZ_vectorSize(OZ_args[0])==0 ||
    OZ_vectorSize(OZ_args[1])==0 ||
    OZ_vectorSize(OZ_args[0])!=OZ_vectorSize(OZ_args[1]))
  return pe.fail();

 return pe.impose(new SumACProp(OZ_args[0],OZ_args[1],OZ_args[2]));
}
OZ_C_proc_end

OZ_CFunHeader SumACProp::spawner = fdtest_sumac;

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdtest_spawnLess, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectIntVarAny);
  OZ_EXPECT(pe, 1, expectIntVarAny);

  return pe.impose(new SpawnLess(OZ_args[0], OZ_args[1]));
}
OZ_C_proc_end

OZ_CFunHeader SpawnLess::spawner = fdtest_spawnLess;

OZ_Return SpawnLess::propagate(void)
{
  printf("spawn less count down: %d\n",c);

  c -= 1;

  if (!c) {
    printf("Spawning less!!!\n"); fflush(stdout);
    addImpose(fd_prop_bounds, a);
    addImpose(fd_prop_bounds, b);
    impose(new Less(a, b));
    return OZ_ENTAILED;
  }

  return SLEEP;
}


OZ_Return Less::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in ");

  if (mayBeEqualVars() && OZ_isEqualVars(_x, _y))
    return PROCEED;

  OZ_FDIntVar x(_x), y(_y);
  PropagatorController_V_V P(x, y);

  FailOnEmpty(*x <= (y->getMaxElem())-1);
  FailOnEmpty(*y >= (x->getMinElem())+1);

  if (x->getMaxElem() < y->getMinElem()) return P.vanish();
  if (x->getMinElem() > y->getMaxElem()) goto failure;

  OZ_DEBUGPRINTTHIS("out ");

  return P.leave();

failure:
  OZ_DEBUGPRINT(("fail"));

  return P.fail();
}

OZ_C_proc_begin(fdtest_counter, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_INT "," OZ_EM_STREAM);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectInt);
  OZ_EXPECT(pe, 1, expectStream);

  return pe.impose(new Counter(OZ_args[0], OZ_args[1]));
}
OZ_C_proc_end

OZ_CFunHeader Counter::spawner = fdtest_counter;

OZ_Return Counter::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in ");

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
    } else if (OZ_isTuple(e) && ! OZ_isLiteral(e)) {
      char * l = OZ_atomToC(OZ_label(e));

      if (! strcmp("get", l)) {
        if (OZ_unify(OZ_int(c), OZ_getArg(e, 0)) == FAILED)
          goto failure;
      } else if (! strcmp ("put", l)) {
        OZ_Term e_0 = OZ_getArg(e, 0);
        if (OZ_isSmallInt(e_0)) {
          c = OZ_intToC(OZ_getArg(e, 0));
        } else if (OZ_isVariable(e_0)) {
          imposeOn(e_0);
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

  return pe.impose(new FirstFail(OZ_args[0], OZ_args[1]));
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

OZ_CFunHeader FirstFail::spawner = fdtest_firstFail;

OZ_Return FirstFail::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in ");

  OZ_Stream st(stream);

  while (!st.isEostr()) {
    OZ_Term e = st.get();
    if (OZ_isTuple(e) && ! OZ_isLiteral(e)) {
      char * l = OZ_atomToC(OZ_label(e));
      if (! strcmp("dist", l)) {
        int last = 0;
        int current = 0;
        int smallest;
        OZ_FDIntVar var;
        do {
          var.ask(reg_fds[current]);
        } while ((*var == fd_singl) && (++current < size));

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
              if (*var == fd_singl)
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
  static OZ_CFunHeader spawner;
  OZ_Term _x, _y, _z;
public:
  DPlusPropagator(OZ_Term a, OZ_Term b, OZ_Term c)
   : _x(a), _y(b), _z(c) {}
  virtual OZ_Return propagate(void);

  virtual size_t sizeOf(void) {
    return sizeof(DPlusPropagator);
  }
  virtual void updateHeapRefs(OZ_Boolean) {
    OZ_updateHeapTerm(_x); OZ_updateHeapTerm(_y); OZ_updateHeapTerm(_z);
  }
  virtual OZ_Term getParameters(void) const {
    return OZ_cons(_x, OZ_cons(_y, OZ_cons(_z, OZ_nil())));
  }
  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }
};


OZ_C_proc_begin(fdtest_plus, 3)
{
  OZ_EXPECTED_TYPE(OZ_EM_FD "," OZ_EM_FD "," OZ_EM_FD);

  OZ_Expect pe;

  OZ_EXPECT(pe, 0, expectIntVar);
  OZ_EXPECT(pe, 1, expectIntVar);
  OZ_EXPECT(pe, 2, expectIntVar);

  return pe.impose(new DPlusPropagator(OZ_args[0],
                                      OZ_args[1],
                                      OZ_args[2]));
}
OZ_C_proc_end

OZ_CFunHeader DPlusPropagator::spawner = fdtest_plus;

OZ_Return DPlusPropagator::propagate(void)
{
  OZ_FDIntVar x(_x), y(_y), z(_z);
  PropagatorController_V_V_V P(x, y, z);

  OZ_FiniteDomain x_aux(fd_empty),
                  y_aux(fd_empty),
                  z_aux(fd_empty);

  int i, j;
  for (i = x->getMinElem(); i != -1; i = x->getNextLargerElem(i)) {
    for (j = y->getMinElem(); j != -1; j = y->getNextLargerElem(j)) {
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

#ifdef ALLDIFF
#include "_alldiff.cc"
#endif

//-----------------------------------------------------------------------------

OZ_C_proc_begin(fdtest_gensum, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_VECT OZ_EM_FD);

  PropagatorExpect pe;

  OZ_EXPECT(pe, 0, expectVectorIntVarMinMax);
  OZ_EXPECT(pe, 1, expectIntVarMinMax);

  return pe.impose(new TestGenSum(OZ_args[0], OZ_args[1]));
}
OZ_C_proc_end

OZ_CFunHeader TestGenSum::header = fdtest_gensum;

OZ_Boolean add3(OZ_FiniteDomain &x,
                OZ_FiniteDomain &y,
                OZ_FiniteDomain &z,
                OZ_Boolean &doagain)
{
  int txl = z.getMinElem() - y.getMaxElem();
  int txu = z.getMaxElem() - y.getMinElem();
  int tyl = z.getMinElem() - x.getMaxElem();
  int tyu = z.getMaxElem() - x.getMinElem();
  int tzl = x.getMinElem() + y.getMinElem();
  int tzu = x.getMaxElem() + y.getMaxElem();

  OZ_Boolean repeat;

  do {
    repeat = OZ_FALSE;

    if (x.getMinElem() < txl) {
      FailOnEmpty(x >= txl);
      tyu = z.getMaxElem() - x.getMinElem();
      tzl = x.getMinElem() + y.getMinElem();
      repeat = OZ_TRUE;
    }

    if (x.getMaxElem() > txu) {
      FailOnEmpty(x <= txu);
      tyl = z.getMinElem() - x.getMaxElem();
      tzu = x.getMaxElem() + y.getMaxElem();
      repeat = OZ_TRUE;
    }

    if (y.getMinElem() < tyl) {
      FailOnEmpty(y >= tyl);
      txu = z.getMaxElem() - y.getMinElem();
      tzl = x.getMinElem() + y.getMinElem();
      repeat = OZ_TRUE;
    }

    if (y.getMaxElem() > tyu) {
      FailOnEmpty(y <= tyu);
      txl = z.getMinElem() - y.getMaxElem();
      tzu = x.getMaxElem() + y.getMaxElem();
      repeat = OZ_TRUE;
    }

    if (z.getMinElem() < tzl) {
      FailOnEmpty(z >= tzl);
      txl = z.getMinElem() - y.getMaxElem();
      tyl = z.getMinElem() - x.getMaxElem();
      repeat = OZ_TRUE;
    }

    if (z.getMaxElem() > tzu) {
      FailOnEmpty(z <= tzu);
      txu = z.getMaxElem() - y.getMinElem();
      tyu = z.getMaxElem() - x.getMinElem();
      repeat = OZ_TRUE;
    }

    doagain |= repeat;
  } while (repeat);

  return OZ_TRUE;

failure:
  return OZ_FALSE;
} // OZ_Boolean add3()

OZ_Return TestGenSum::propagate(void)
{
  OZ_DEBUGPRINTTHIS("in ");

  int &vd_size = reg_l_sz;
  DECL_DYN_ARRAY(OZ_FDIntVar, vd, vd_size);
  OZ_FDIntVar d(_v);
  OZ_FiniteDomain * aux = _aux;
  PropagatorController_VV_V P(vd_size, vd, d);
  int i;

  for (i = vd_size; i--; )
    vd[i].read(reg_l[i]);

  OZ_Boolean doagain;

  if (vd_size == 0) {
    OZ_DEBUGPRINTTHIS("vd_size == 0");
    FailOnEmpty(*d &= aux[0]);
    return P.vanish();
  } else if (vd_size == 1) {
    OZ_DEBUGPRINTTHIS("vd_size == 1");
    do {
      doagain = OZ_FALSE;

      if (!add3(aux[0], *vd[0], *d, doagain))
        goto failure;

    } while (doagain);

    return P.leave();
  }

  do {
    doagain = OZ_FALSE;


    for (i = 0; i < vd_size - 1; i += 1)
      if (!add3(aux[i], *vd[i], aux[i+1], doagain))
        goto failure;

    if (!add3(*vd[vd_size-1], aux[vd_size-1], *d, doagain))
      goto failure;

  } while (doagain);

  {
    OZ_Return r = P.leave();

    if (r == OZ_SLEEP) {

      int j = 0;
      for (i = 0; i < vd_size; i += 1) {
        if (*vd[i] == 0)
          continue;

        if (i != j) {
          reg_l[j] = reg_l[i];
          _aux[j] = _aux[i];
        }
        j += 1;
      }
      vd_size = j;
    }

    OZ_DEBUGPRINTTHIS("out ");
    return r;
  }


failure:
  OZ_DEBUGPRINTTHIS("failed");
  return P.fail();
}

//-----------------------------------------------------------------------------
// eof
