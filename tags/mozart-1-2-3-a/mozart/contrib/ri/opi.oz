%{Property.put 'internal.debug' true}
declare [RI LP] = {Module.link ['x-oz://contrib/RI' 'x-oz://contrib/LP.ozf']}
%[RI LP] = {Module.link ['RI.ozf' 'LP.ozf']}

/*

{Wait RI}
{Wait LP}
{Show RI}
{Show LP}


\switch +debuginfocontrol
{Property.put 'internal.debug' true}
declare [R] = {Module.link [{OS.getEnv 'HOME'}#'/Programming/Oz/VisualizeConstraints/ReflectConstraints.ozf']}

declare X Y C in {ForAll [X Y] proc {$ V} V = {RI.var.bounds 0.0 10.0} end}
{RI.lessEqMeta X Y C}

{Show {R.propName C}}....h
{Show {R.propCoordinates C}}
{Show C}

declare 
X={RI.var.decl}
Y={RI.var.decl}
Z={RI.var.decl}
in
{RI.greater X 0.0}
{RI.times Y 2.0 X}
{Show X#Y}

{Browse RI}
*/
