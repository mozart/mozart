#include <stdio.h>
#include "mozart_cpi.hh"

#define FailOnEmpty(X) if((X) == 0) goto failure;

OZ_BI_proto(test_sync);

class SyncProp : public OZ_Propagator {
private:
  static OZ_PropagatorProfile sync_profile;
protected:
  OZ_Term _fs;
  OZ_Term _fd;
public:
  SyncProp(OZ_Term, OZ_Term);

  virtual void updateHeapRefs(OZ_Boolean) {
    OZ_updateHeapTerm(_fd);
    OZ_updateHeapTerm(_fs);
  }

  virtual size_t sizeOf(void) {
    return sizeof(SyncProp);
  }

  virtual OZ_Term getParameters(void) const {
    return OZ_cons(_fs,
                   (OZ_cons(_fd,
                            OZ_nil())));
  }

  virtual OZ_PropagatorProfile *getProfile(void) const {
    return &sync_profile;
  }

  virtual OZ_Return propagate();
};

OZ_Return SyncProp::propagate() {
  // semantics is as follows:
  // the finite domain *fd and the finite set constraint *fs are synchronized.

  // that means, every element not in (*fd) has to be in (*fs).OUT,
  // and every element in (*fs).out has to be removed from (*fd).

  printf("SyncProp::propagate() invoked\n");

  bool entailed;
  OZ_FDIntVar fd(_fd);
  OZ_FSetVar  fs(_fs);

  OZ_FSetConstraint fd_out_constraint((OZ_FSetValue(*fd)));
  OZ_FiniteDomain   fs_not_in((*fs).getNotInSet());

  // constrain to exactly one element.
  fs->putCard(1, 1);

  // what is excluded from fsc must be thrown out of fd.
  FailOnEmpty(*fd -= fs_not_in);

  // what's not in fd cannot be in fsc, either.
  if ((*fs <= fd_out_constraint) == OZ_FALSE) goto failure;

  entailed = ((*fd).getSize() == 1);

  fd.leave();
  fs.leave();

  printf("leaving SyncProp::propagate ");
  if (entailed)
    printf("(entailed)\n");
  else
    printf("(sleeping)\n");

  // check if fd is determined now:
  return entailed? OZ_ENTAILED : OZ_SLEEP;

failure:
  printf("SyncProp::propagate has failed.\n");
  fd.fail();
  fs.fail();
  return OZ_FAILED;
}

SyncProp::SyncProp(OZ_Term fsvar, OZ_Term fdvar)
  : _fs(fsvar), _fd(fdvar)  {
}

OZ_C_proc_begin(test_sync, 2)
{
  OZ_EXPECTED_TYPE(OZ_EM_FSET","OZ_EM_FD);
  OZ_Expect pe;
  OZ_EXPECT(pe, 0, expectFSetVar);
  OZ_EXPECT(pe, 1, expectIntVar);

  return pe.impose(new SyncProp(OZ_args[0], OZ_args[1]));
}
OZ_C_proc_end

OZ_PropagatorProfile SyncProp::sync_profile;

extern "C"
{
  OZ_C_proc_interface *oz_init_module(void)
  {
    static OZ_C_proc_interface i_table[] = {
      {"sync", 2, 0, test_sync},
      {0, 0, 0, 0}
    };

    printf("Sync propagator loaded.\n");
    return i_table;
  }
}
