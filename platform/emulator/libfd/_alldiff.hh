// alldiff.hh
// interval consistent sum and sumC

// loeckelt@ps.uni-sb.de

#include <stdio.h>

#include "_generic.hh"
#include "_noleda.hh"

//-----------------------------------------------------------------------------

class alldiffProp : public Propagator_VD {
private:
  static OZ_CFunHeader spawner;
  // graph buildGraph(OZ_FDIntVar *l);
  list<edge> removeEdgesFromG(graph &g,
                              list<edge> matching,
                              OZ_FDIntVar *reg);
  void computeMaximumMatching(list<edge> &matching,
                              graph &g);
  OZ_NonMonotonic _nm;
  void mark_bfs(graph &g, node n);

public:
  virtual OZ_Boolean isMonotonic(void) const { return OZ_FALSE; }
  virtual OZ_NonMonotonic::order_t getOrder(void) const {
    return _nm.getOrder();
  }

  alldiffProp(OZ_Term t) : Propagator_VD(t) {};

  virtual OZ_Return propagate(void);

  virtual OZ_CFunHeader * getHeader(void) const { return &spawner; }


  virtual OZ_Term getParameters(void) const {
    return Propagator_VD::getParameters();
  }
  virtual void updateHeapRefs(OZ_Boolean duplicate=0) {
    DEBUG(("updateHeapRefs() called!!! duplicate=%d\r\n", (int)duplicate));
    Propagator_VD::updateHeapRefs(duplicate);
    DEBUG(("Propagator_VD::updateHeapRefs() okay\r\n"));
  }
  virtual size_t sizeOf(void) {
    DEBUG(("sizeOf() called\r\n"));
    return sizeof(*this);
  }
};
