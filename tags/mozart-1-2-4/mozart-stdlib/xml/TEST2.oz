declare [NS FASTSAX GEN]={Link ['NameSpaces.ozf' 'FastSAX.ozf' 'XSLTGenerator.ozf']}
PM = {NS.newPrefixMap}
local
   DOC = {PM.intern unit doc}.key
   HEAD = {PM.intern unit head}.key
   SECTION = {PM.intern unit section}.key
in
   STRIP =
   o(DOC: true
     HEAD: true
     SECTION: true)
end
fun {NoParent X}
   Y = {Record.subtract X parent}
in
   if {HasFeature Y children}
   then {AdjoinAt Y children {Map Y.children NoParent}}
   else Y end
end
DOC = {FASTSAX.new o(url:'test.xml' prefixmap:PM stripspaces:STRIP)}.root

{Inspect {NoParent DOC}}

declare G1={GEN.newGenDescendant DOC}
declare N={G1}
{Inspect {NoParent N}}

declare G2={GEN.newGenAncestorOrSelf N}
{Inspect {NoParent {G2}}}

declare G3={GEN.newGenFollowingSibling N}
{Inspect {NoParent {G3}}}

declare G4={GEN.newGenFollowing N}
{Inspect {NoParent {G4}}}

declare G5={GEN.newGenPreceding N}
{Inspect {NoParent {G5}}}