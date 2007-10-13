declare

LTs = [d(0 0 1) d(0 1 2) d(1 1 0) d(2 0 0) d(~1 0 0)]
%LTs = [d(0 0 1)]
L1 = dfa(0 LTs [0 ~1])
Vars = [_ _ _ _]
Vars:::0#10
{GFD.int_ext Vars L1}

{Show Vars}