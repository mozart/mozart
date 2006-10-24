declare 
[Maple] = {Module.link ['x-oz://contrib/Maple.ozf']}
{Wait Maple}
{Show Maple}



declare B1 = {Maple.call simplify '+'('*'('X'#const 'A') '-'(0 1))}
{Browse result1#B1}
declare B2 = {Maple.call simplify '/'(3.0 4.0)}
{Browse result2#B2}

/*
declare B3 = {Maple.call simplify '/'(1.0 0)}
{Browse B3}
*/

