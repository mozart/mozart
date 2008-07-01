
template <class SERVICE, 
  class FDVAR, class FDM, class P_PFDVAR, class PFDVAR, class ENGINE>
class FilterTasksOverlap : public OZ_PersistentFilter {
private:
  P_PFDVAR _cl1_t1, _cl1_t2, _cl1_o;
  P_PFDVAR _cl2_t1, _cl2_t2, _cl2_o;
  P_PFDVAR _cl3_t1, _cl3_t2, _cl3_o; 
  //
  // persistent part of propagation engine
  //
  ENGINE _engine_cl1, _engine_cl2, _engine_cl3;
  //
  int _first;
  //
public:
  FilterTasksOverlap(int xd, int yd) {
    _first = 0;
    // clause 1
    {
      _engine_cl1.init();
      make_PEL_GreaterOffset<ENGINE,P_PFDVAR,PFDVAR>(_engine_cl1, _cl1_t1, xd, _cl1_t2);
      make_PEL_GreaterOffset<ENGINE,P_PFDVAR,PFDVAR>(_engine_cl1, _cl1_t2, yd, _cl1_t1);
      //      
      (*_cl1_t1).initFull();
      (*_cl1_t2).initFull();
      (*_cl1_o).initSingleton(1);
    }
    // clause 2
    {
      _engine_cl2.init();
      make_PEL_LessEqOffset<ENGINE,P_PFDVAR,PFDVAR>(_engine_cl2, _cl2_t1, xd, _cl2_t2);
      (*_cl2_t1).initFull();
      (*_cl2_t2).initFull();
      (*_cl2_o).initSingleton(0);
    }
    // clause 3
    {
      _engine_cl3.init();
      make_PEL_LessEqOffset<ENGINE,P_PFDVAR,PFDVAR>(_engine_cl3, _cl3_t2, yd, _cl3_t1);
      //
      (*_cl3_t1).initFull();
      (*_cl3_t2).initFull();
      (*_cl3_o).initSingleton(0);
    }
  }
  virtual void gCollect(void) {
    _engine_cl1.gCollect();
    _engine_cl2.gCollect();
    _engine_cl3.gCollect();
    _cl1_t1.gCollect();
    _cl1_t2.gCollect();
    _cl1_o.gCollect();
    _cl2_t1.gCollect();
    _cl2_t2.gCollect();
    _cl2_o.gCollect();
    _cl3_t1.gCollect();
    _cl3_t2.gCollect();
    _cl3_o.gCollect();
  }
  virtual void sClone(void) {
    _engine_cl1.sClone();
    _engine_cl2.sClone();
    _engine_cl3.sClone();
    _cl1_t1.sClone();
    _cl1_t2.sClone();
    _cl1_o.sClone();
    _cl2_t1.sClone();
    _cl2_t2.sClone();
    _cl2_o.sClone();
    _cl3_t1.sClone();
    _cl3_t2.sClone();
    _cl3_o.sClone();
  }
  SERVICE &filter(SERVICE & s, FDVAR &x, int xd, FDVAR &y, int yd, FDVAR &o);
};

