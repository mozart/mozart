declare
X1 = {RI.var.bounds 0.0 RI.sup}
X2 = {RI.var.bounds 0.0 RI.sup}
Ret Sol
in
{LP.solve
 [X1 X2]
 objfn(row: [8.0 5.0] opt: max)
 constrs(
    constr(row: [1.0 1.0] type: '=<' rhs:6.0) 
    constr(row: [9.0 5.0] type: '=<' rhs:45.0))
 Sol
 Ret}

