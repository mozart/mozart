declare

[GFD] = {Module.link ['x-oz://contrib/geoz/GeIntVar.ozf']}
S = {GFD.int  0#3}
%{GFD.rel S GFD.rt.'\\=:' 0 GFD.cl.bnd}
{Wait S}
{Show finished}
