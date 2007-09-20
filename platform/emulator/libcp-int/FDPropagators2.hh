// Domain constraints
void 	Gecode::dom (Space *home, IntVar x, int l, int m, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::dom (Space *home, IntVarArgs &x, int l, int m, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::dom (Space *home, IntVar x, const IntSet &s, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::dom (Space *home, IntVarArgs &x, const IntSet &s, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::dom (Space *home, IntVar x, int l, int m, BoolVar b, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::dom (Space *home, IntVar x, const IntSet &s, BoolVar b, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);

//Simple relation constraints over integer variables
void 	Gecode::rel (Space *home, IntVar x0, IntRelType r, IntVar x1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::rel (Space *home, IntVar x, IntRelType r, int c, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::rel (Space *home, IntVar x0, IntRelType r, IntVar x1, BoolVar b, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::rel (Space *home, IntVar x, IntRelType r, int c, BoolVar b, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::rel (Space *home, const IntVarArgs &x, IntRelType r, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::rel (Space *home, const IntVarArgs &x, IntRelType r, const IntVarArgs &y, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);

//Simple relation constraints over Boolean variables
void 	Gecode::rel (Space *home, BoolVar x0, IntRelType r, BoolVar x1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::rel (Space *home, BoolVar x, IntRelType r, int n, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::rel (Space *home, const BoolVarArgs &x, IntRelType r, const BoolVarArgs &y, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::rel (Space *home, const BoolVarArgs &x, IntRelType r, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::rel (Space *home, BoolVar x0, BoolOpType o, BoolVar x1, BoolVar x2, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::rel (Space *home, BoolVar x0, BoolOpType o, BoolVar x1, int n, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::rel (Space *home, const BoolVarArgs &x, BoolOpType o, BoolVar y, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::rel (Space *home, const BoolVarArgs &x, BoolOpType o, int n, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);

//Element constraints
void 	Gecode::element (Space *home, const IntArgs &n, IntVar x0, IntVar x1, int offset=0, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::element (Space *home, const IntArgs &n, IntVar x0, BoolVar x1, int offset=0, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::element (Space *home, const IntArgs &n, IntVar x0, int x1, int offset=0, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::element (Space *home, const IntVarArgs &x, IntVar y0, IntVar y1, int offset=0, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::element (Space *home, const IntVarArgs &x, IntVar y0, int y1, int offset=0, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::element (Space *home, const BoolVarArgs &x, IntVar y0, BoolVar y1, int offset=0, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::element (Space *home, const BoolVarArgs &x, IntVar y0, int y1, int offset=0, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);

//Distinct constraints
void 	Gecode::distinct (Space *home, const IntVarArgs &x, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::distinct (Space *home, const IntArgs &n, const IntVarArgs &x, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);

//Channel constraints
void 	Gecode::channel (Space *home, const IntVarArgs &x, const IntVarArgs &y, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::channel (Space *home, BoolVar x0, IntVar x1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::channel (Space *home, IntVar x0, BoolVar x1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::channel (Space *home, const BoolVarArgs &x, IntVar y, int o=0, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);

//Graph constraints
void 	Gecode::circuit (Space *home, const IntVarArgs &x, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);

//Scheduling constraints
void 	Gecode::cumulatives (Space *home, const IntVarArgs &machine, const IntVarArgs &start, const IntVarArgs &duration, const IntVarArgs &end, const IntVarArgs &height, const IntArgs &limit, bool at_most, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::cumulatives (Space *home, const IntArgs &machine, const IntVarArgs &start, const IntVarArgs &duration, const IntVarArgs &end, const IntVarArgs &height, const IntArgs &limit, bool at_most, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::cumulatives (Space *home, const IntVarArgs &machine, const IntVarArgs &start, const IntArgs &duration, const IntVarArgs &end, const IntVarArgs &height, const IntArgs &limit, bool at_most, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::cumulatives (Space *home, const IntArgs &machine, const IntVarArgs &start, const IntArgs &duration, const IntVarArgs &end, const IntVarArgs &height, const IntArgs &limit, bool at_most, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::cumulatives (Space *home, const IntVarArgs &machine, const IntVarArgs &start, const IntVarArgs &duration, const IntVarArgs &end, const IntArgs &height, const IntArgs &limit, bool at_most, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::cumulatives (Space *home, const IntArgs &machine, const IntVarArgs &start, const IntVarArgs &duration, const IntVarArgs &end, const IntArgs &height, const IntArgs &limit, bool at_most, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::cumulatives (Space *home, const IntVarArgs &machine, const IntVarArgs &start, const IntArgs &duration, const IntVarArgs &end, const IntArgs &height, const IntArgs &limit, bool at_most, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::cumulatives (Space *home, const IntArgs &machine, const IntVarArgs &start, const IntArgs &duration, const IntVarArgs &end, const IntArgs &height, const IntArgs &limit, bool at_most, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);

//Sorted constraints
void 	Gecode::sorted (Space *home, const IntVarArgs &x, const IntVarArgs &y, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::sorted (Space *, const IntVarArgs &x, const IntVarArgs &y, const IntVarArgs &z, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);

//Cardinality constraints
void 	Gecode::count (Space *home, const IntVarArgs &x, int n, IntRelType r, int m, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::count (Space *home, const IntVarArgs &x, IntVar y, IntRelType r, int m, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::count (Space *home, const IntVarArgs &x, const IntArgs &y, IntRelType r, int m, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::count (Space *home, const IntVarArgs &x, int n, IntRelType r, IntVar z, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::count (Space *home, const IntVarArgs &x, IntVar y, IntRelType r, IntVar z, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::count (Space *home, const IntVarArgs &x, const IntArgs &y, IntRelType r, IntVar z, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::count (Space *home, const IntVarArgs &x, const IntVarArgs &c, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::count (Space *home, const IntVarArgs &x, const IntSetArgs &c, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::count (Space *home, const IntVarArgs &x, const IntVarArgs &c, const IntArgs &v, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::count (Space *home, const IntVarArgs &x, const IntSetArgs &c, const IntArgs &v, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::count (Space *home, const IntVarArgs &x, const IntSet &c, const IntArgs &v, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);

//Extensional constraints
void 	Gecode::extensional (Space *home, const IntVarArgs &x, DFA d, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::extensional (Space *home, const BoolVarArgs &x, DFA d, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::extensional (Space *home, const IntVarArgs &x, const Table &t, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::extensional (Space *home, const IntArgs &c, const IntVarArgs &x, const Table &t, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);

//Arithmetic constraints
void 	Gecode::min (Space *home, IntVar x0, IntVar x1, IntVar x2, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::min (Space *home, const IntVarArgs &x, IntVar y, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::max (Space *home, IntVar x0, IntVar x1, IntVar x2, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::max (Space *home, const IntVarArgs &x, IntVar y, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::abs (Space *home, IntVar x0, IntVar x1, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::mult (Space *home, IntVar x0, IntVar x1, IntVar x2, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);

//Linear constraints over integer variables
void 	Gecode::linear (Space *home, const IntVarArgs &x, IntRelType r, int c, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::linear (Space *home, const IntVarArgs &x, IntRelType r, IntVar y, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::linear (Space *home, const IntVarArgs &x, IntRelType r, int c, BoolVar b, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::linear (Space *home, const IntVarArgs &x, IntRelType r, IntVar y, BoolVar b, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::linear (Space *home, const IntArgs &a, const IntVarArgs &x, IntRelType r, int c, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::linear (Space *home, const IntArgs &a, const IntVarArgs &x, IntRelType r, IntVar y, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::linear (Space *home, const IntArgs &a, const IntVarArgs &x, IntRelType r, int c, BoolVar b, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::linear (Space *home, const IntArgs &a, const IntVarArgs &x, IntRelType r, IntVar y, BoolVar b, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);

//Linear constraints over Boolean variables
void 	Gecode::linear (Space *home, const BoolVarArgs &x, IntRelType r, int c, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::linear (Space *home, const BoolVarArgs &x, IntRelType r, IntVar y, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::linear (Space *home, const IntArgs &a, const BoolVarArgs &x, IntRelType r, int c, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);
void 	Gecode::linear (Space *home, const IntArgs &a, const BoolVarArgs &x, IntRelType r, IntVar y, IntConLevel icl=ICL_DEF, PropKind pk=PK_DEF);


