declare [SAX]={Link ['SAX.ozf']}

declare
T1={Property.get 'time.total'}
P=try {New SAX.parser initFromURL('/home/denys/Mozart/ozlib/OzMake2/ozmake.xml')} catch E then {Inspect E} unit end
T2={Property.get 'time.total'}
{Show T2-T1}


declare
T1={Property.get 'time.total'}
P={New SAX.parser initFromURL('/home/denys/src/ozstuff/xml/apptut.xml')}
T2={Property.get 'time.total'}
{Show T2-T1}


{Inspect P.root}


declare [SAX1]={Link ['SlowSAX.ozf']}

declare
T1={Property.get 'time.total'}
P=try {New SAX1.parser initFromURL('/home/denys/Mozart/ozlib/OzMake2/ozmake.xml')} catch E then {Inspect E} unit end
T2={Property.get 'time.total'}
{Show T2-T1}

declare [GSAX]={Link ['GenSAX.ozf']}

declare
T1={Property.get 'time.total'}
P=try {New GSAX.parser initFromURL('/home/denys/Mozart/ozlib/OzMake2/ozmake.xml')} catch E then {Inspect E} unit end
T2={Property.get 'time.total'}
{Show T2-T1}

declare [SAX2]={Link ['SlowSAX2.ozf']}

declare
T1={Property.get 'time.total'}
P=try {New SAX2.parser initFromURL('/home/denys/Mozart/ozlib/OzMake2/ozmake.xml')} catch E then {Inspect E} unit end
T2={Property.get 'time.total'}
{Show T2-T1}

declare [SAX3]={Link ['SAX2.ozf']}

declare
T1={Property.get 'time.total'}
P=try {New SAX3.parser initFromURL('/home/denys/Mozart/ozlib/OzMake2/ozmake.xml')} catch E then {Inspect E} unit end
T2={Property.get 'time.total'}
{Show T2-T1}

declare [NS FAST]={Link ['NameSpaces.ozf' 'FastParser.ozf']}

declare PM = {NS.newPrefixMap}
declare TAG1={PM.intern unit 'package'}
declare TAG2={PM.intern unit 'author'}

declare
T1={Property.get 'time.total'}
P={FAST.newFromURL '/home/denys/Mozart/ozlib/OzMake2/ozmake.xml' unit}
T2={Property.get 'time.total'}
{Show T2-T1}

declare
T1={Property.get 'time.total'}
P={FAST.newFromURLOld '/home/denys/Mozart/ozlib/OzMake2/ozmake.xml' unit}
T2={Property.get 'time.total'}
{Show T2-T1}

declare
KEY1=TAG1.key
KEY2=TAG2.key
STRIP=o(KEY1:true KEY2:true)
T1={Property.get 'time.total'}
P={FAST.newFromURL '/home/denys/Mozart/ozlib/OzMake2/ozmake.xml' unit(prefixMap:PM stripSpaces:STRIP)}
T2={Property.get 'time.total'}
{Show T2-T1}

{Inspect P}

declare
T1={Property.get 'time.total'}
P={FAST.newFromURL '/home/denys/src/ozstuff/xml/apptut.xml' unit}
T2={Property.get 'time.total'}
{Show T2-T1}

declare
T1={Property.get 'time.total'}
P={FAST.newFromURLOld '/home/denys/src/ozstuff/xml/apptut.xml' unit}
T2={Property.get 'time.total'}
{Show T2-T1}


declare [TOK]={Link ['FastTokenizer.ozf']}
declare T={TOK.newFromURL  '/home/denys/Mozart/ozlib/OzMake2/ozmake.xml'}
{Inspect {T.get}}

declare [FASTER]={Link ['FasterParser.ozf']}

declare
T1={Property.get 'time.total'}
P={FASTER.newFromURL '/home/denys/Mozart/ozlib/OzMake2/ozmake.xml' unit}
T2={Property.get 'time.total'}
{Show T2-T1}

declare
T1={Property.get 'time.total'}
P={FASTER.newFromURL '/home/denys/src/ozstuff/xml/apptut.xml' unit}
T2={Property.get 'time.total'}
{Show T2-T1}

%% it appears that the Faster parser is actually slower than the Fast parser

%%====================================================================

declare [PARSER]={Link ['Parser.ozf']}
proc {TRY Fast Feat Arg}
   T1={Property.get 'time.total'}
   {if Fast then PARSER.fast.Feat else PARSER.Feat end Arg unit _}
   T2={Property.get 'time.total'}
in
   {Show T2-T1}
end

{TRY false newFromURL '/home/denys/src/ozstuff/xml/apptut.xml'} %% 180, 200 ms
{TRY true  newFromURL '/home/denys/src/ozstuff/xml/apptut.xml'} %% 100, 120 ms

{Inspect {PARSER.fast.newFromURL '/home/denys/Mozart/ozlib/OzMake2/ozmake.xml' unit}}

%%====================================================================

declare [PARSER]={Link ['Parser.ozf']}
proc {TRY Init}
   T1={Property.get 'time.total'}
   {PARSER.new Init _}
   T2={Property.get 'time.total'}
in
   {Show T2-T1}
end

{TRY o(url:'/home/denys/src/ozstuff/xml/apptut.xml')} %% 180, 200 ms
{TRY o(url:'/home/denys/src/ozstuff/xml/apptut.xml' namespaces:false)} %% 160 ms
{TRY o(url:'/home/denys/src/ozstuff/xml/apptut.xml' fast:true)} %% 120, 140 ms
{TRY o(url:'/home/denys/src/ozstuff/xml/apptut.xml' fast:true namespaces:false)} %% 100, 120 ms

declare [SAX]={Link ['SAX.ozf']}
proc {TRY2 Init}
   T1={Property.get 'time.total'}
   {New SAX.parser Init _}
   T2={Property.get 'time.total'}
in
   {Show T2-T1}
end

{TRY2 init(url:'/home/denys/src/ozstuff/xml/apptut.xml')} %% 190, 210 ms
{TRY2 init(url:'/home/denys/src/ozstuff/xml/apptut.xml' fast:true)} %% 150 ms
{TRY2 init(url:'/home/denys/src/ozstuff/xml/apptut.xml' namespaces:false)} %% 170 ms
{TRY2 init(url:'/home/denys/src/ozstuff/xml/apptut.xml' namespaces:false fast:true)} %% 120 ms

%%====================================================================

declare [PARSER]={Link ['NewParser.ozf']}
proc {TRY Init}
   T1={Property.get 'time.total'}
   {PARSER.new Init _}
   T2={Property.get 'time.total'}
in
   {Show T2-T1}
end

{TRY o(url:'/home/denys/src/ozstuff/xml/apptut.xml')} %% 190, 210 ms
{TRY o(url:'/home/denys/src/ozstuff/xml/apptut.xml' namespaces:false)} %% 170 ms
{TRY o(url:'/home/denys/src/ozstuff/xml/apptut.xml' fast:true)} %% 130, 140, 150 ms
{TRY o(url:'/home/denys/src/ozstuff/xml/apptut.xml' fast:true namespaces:false)} %% 100, 120 ms

{TRY o(url:'/home/denys/src/ozstuff/xml/apptut.xml' makeNode:min)} %% 200 ms
{TRY o(url:'/home/denys/src/ozstuff/xml/apptut.xml' namespaces:false makeNode:min)} %% 160 ms
{TRY o(url:'/home/denys/src/ozstuff/xml/apptut.xml' fast:true makeNode:min)} %% 140 ms
{TRY o(url:'/home/denys/src/ozstuff/xml/apptut.xml' fast:true namespaces:false makeNode:max)} %% 100, 120 ms

%%====================================================================

declare [NS FASTSAX]={Link ['NameSpaces.ozf' 'FastSAX.ozf']}
proc {TRY Init}
   T1={Property.get 'time.total'}
   {FASTSAX.new Init _}
   T2={Property.get 'time.total'}
in
   {Show T2-T1}
end

{TRY o(url:'/home/denys/src/ozstuff/xml/apptut.xml')} %% 150 ms
{TRY o(url:'/home/denys/src/ozstuff/xml/apptut.xml' namespaces:false)} %% 130 ms
{TRY o(url:'/home/denys/src/ozstuff/xml/apptut.xml' fast:true)} %% 110 ms
{TRY o(url:'/home/denys/src/ozstuff/xml/apptut.xml' fast:true namespaces:false)} %% 80 ms

{Inspect {FASTSAX.new o(url:'/home/denys/src/ozstuff/xml/apptut.xml')}.root}
{Inspect {FASTSAX.new o(url:'/home/denys/src/ozstuff/xml/apptut.xml' fast:true namespaces:false)}.root}

{Inspect foo}
{Inspect {FASTSAX.new o(url:'/home/denys/Mozart/ozlib/OzMake2/ozmake.xml')}.root}

declare
PM = {NS.newPrefixMap}
local
   PACKAGE = {PM.intern unit package}.key
   HEAD    = {PM.intern unit head   }.key
   AUTHOR  = {PM.intern unit author }.key
   SECTION = {PM.intern unit section}.key
   DLIST   = {PM.intern unit dlist  }.key
   ITEM    = {PM.intern unit item   }.key
   ALIGN   = {PM.intern unit align  }.key
   ROW     = {PM.intern unit row    }.key
in
   STRIP =
   o(PACKAGE : true
     HEAD    : true
     AUTHOR  : true
     SECTION : true
     DLIST   : true
     ITEM    : true
     ALIGN   : true
     ROW     : true)
end

declare
fun {NoParent X}
   Y = {Record.subtract X parent}
in
   case {Label Y}
   of root then root({Map Y.1 NoParent})
   [] element then
      {AdjoinAt Y children {Map Y.children NoParent}}
   else Y end
end

{Inspect {NoParent {FASTSAX.new o(url:'/home/denys/Mozart/ozlib/OzMake2/ozmake.xml' prefixmap:PM stripspaces:STRIP)}.root}}

{Inspect {ByteString.make "a b c"}}