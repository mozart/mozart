functor
import
   Open
export
   Program Catalog object:SgmlParserObject
define
   ExceptionLabel = sgmlParser

   ParamProgram = {NewCell 'nsgmls'}
   Program = program(get:proc {$ X} {Access ParamProgram X} end
		     put:proc {$ X} {Assign ParamProgram X} end)
   ParamCatalog = {NewCell 'catalog'}
   Catalog = catalog(get:proc {$ X} {Access ParamCatalog X} end
		     put:proc {$ X} {Assign ParamCatalog X} end)

   class TextPipe from Open.pipe Open.text end

   fun {MakeAtom L}
      {String.toAtom {Unescape L}}
   end

   fun {MakeBytes L}
      {ByteString.make {Unescape L}}
   end

   fun {MakeInt L}
      {String.toInt {Unescape L}}
   end

   fun {MakeToken L}
      S = {Unescape L}
   in
      try {String.toInt S} catch _ then {String.toAtom S} end
   end

   %% for uniformity the value of an attribute is always a list

   proc {ParseKindValue L K V}
      Kind Value
   in
      {String.token L &  Kind Value}
      K = {MakeAtom Kind}
      case K
      of 'IMPLIED'  then V=nil
      [] 'CDATA'    then V=[{MakeBytes Value}]
      [] 'NOTATION' then V=[{MakeAtom Value}]
      [] 'ENTITY'   then
	 V={Map {String.tokens Value & } MakeAtom}
      [] 'TOKEN'    then
	 V={Map {String.tokens Value & } MakeToken}
      [] 'ID'       then V=[{MakeToken Value}]
      end
   end

   fun {GetEvent Output}
      case {Output getS($)} of false then false
      [] C|L then
	 case C
	 of &( then '('( {MakeAtom  L})
	 [] &) then ')'( {MakeAtom  L})
	 [] &- then '-'( {MakeBytes L})
	 [] && then '\&'({MakeAtom  L})
	 [] &? then '?'( {MakeAtom  L})
	 [] &A then Name Kind Value Tmp in
	    {String.token L   &  Name Tmp}
	    {ParseKindValue Tmp Kind Value}
	    'A'({MakeAtom Name} Kind Value)
	 [] &D then Ename Name Kind Value Tmp1 Tmp2 in
	    {String.token L    &  Ename Tmp1}
	    {String.token Tmp1 &  Name  Tmp2}
	    {ParseKindValue Tmp2 Kind Value}
	    'D'({MakeAtom Ename} {MakeAtom Name} Kind Value)
	 [] &a then Type Name Kind Value Tmp1 Tmp2 in
	    {String.token L    &  Type Tmp1}
	    {String.token Tmp1 &  Name Tmp2}
	    {ParseKindValue Tmp2 Kind Value}
	    'a'({MakeAtom Type} {MakeAtom Name} Kind Value)
	 [] &N then 'N'({MakeAtom L})
	 [] &E then Ename Type Nname Tmp in
	    {String.token L   &  Ename Tmp}
	    {String.token Tmp &  Type Nname}
	    'N'({MakeAtom Ename}
		{MakeAtom Type}
		{MakeAtom Nname})
	 [] &I then Ename Type Text Tmp in
	    {String.token L   &  Ename Tmp}
	    {String.token Tmp &  Type Text}
	    'I'({MakeAtom  Ename}
		{MakeAtom  Type}
		{MakeBytes Text})
	 [] &S then 'S'({MakeAtom L})
	 [] &T then 'T'({MakeAtom L})
	 [] &s then 's'({MakeAtom L})
	 [] &p then 'p'({MakeAtom L})
	 [] &f then 'f'({MakeAtom L})
	 [] &{ then '{'({MakeAtom L})
	 [] &} then '}'({MakeAtom L})
	 [] &L then Line File in
	    {String.token L &  Line File}
	    if File==nil then 'L'({MakeInt Line})
	    else 'L'({MakeInt Line} {MakeAtom File}) end
	 [] &# then '#'({MakeBytes L})
	 [] &C then 'C'
	 [] &i then 'i'
	 [] &e then 'e'
	 else
	    {Exception.raiseError
	     ExceptionLabel(unknownEventChar C|L)}
	    unit
	 end
      end
   end

   fun {IsOctalDigit C}
      C >= &0 andthen C =< &7
   end

   fun {Unescape L}
      case L of nil then nil
      [] H|T then
	 if H==&\\ then
	    case T of H|T then
	       case H
	       of     &\\ then &\\|{Unescape T}
	       elseof &n  then &\n|{Unescape T}
	       elsecase {IsOctalDigit H} then
		  H2|H3|L = T
	       in
		  (H3-&0+(H2-&0)*8+(H-&0)*16)
		  | {Unescape L}
	       else
		  {Exception.raiseError
		   ExceptionLabel(unsupportedEscape L)}
		  unit
	       end
	    end
	 else H|{Unescape T} end
      end
   end

   fun {MakeEventGenerator Cmd Args}
      Output = {New TextPipe init(cmd:Cmd args:Args)}
      fun {Loop}
	 {GetEvent Output}
      end
   in Loop end

   fun {Parse NextEvent}
      DocElem
      Attr  = {NewCell nil}
      Link = {NewCell nil}
      Stack = {NewCell nil}
      Children = {NewCell [DocElem]}
      EMap = {Dictionary.new} EAttMap = {Dictionary.new}
      SysId = {NewCell unit}
      PubId = {NewCell unit}
      GenId = {NewCell unit}
      proc {Push ElemTag}
	 ElemAttr = {Exchange Attr $ nil}
	 ElemLink = {Exchange Link $ nil}
	 ElemChildren
	 Elem = element(
		   tag            : ElemTag
		   attributes     : ElemAttr
		   linkAttributes : ElemLink
		   children       : ElemChildren)
	 OldChildren
	 {Exchange Children Elem|OldChildren ElemChildren}
	 OldStack
	 {Exchange Stack OldStack OldChildren|OldStack}
      in skip end
      proc {Pop Tag}
	 OldStack OldChildren
      in
	 {Assign Attr nil}
	 {Assign Link nil}
	 {Exchange Stack OldChildren|OldStack OldStack}
	 {Exchange Children nil OldChildren}
      end
      fun {Idents}
	 id(sys:{Exchange SysId $ unit}
	    pub:{Exchange PubId $ unit}
	    gen:{Exchange GenId $ unit})
      end
      proc {Loop}
	 case {NextEvent}
	 of '('(Tag)   then {Push Tag}
	 [] ')'(Tag)   then {Pop  Tag}
	 [] '-'(Bytes) then L in
	    {Exchange Children data(Bytes)|L L}
	 [] '\&'(Name) then L in
	    {Exchange Children edata(Name)|L L}
	 [] '?'(Name)  then L in
	    {Exchange Children pi(Name)|L L}
	 [] 'A'(Prop Kind Value) then L in
	    {Exchange Attr L
	     attribute(name:Prop kind:Kind value:Value)|L}
	 [] 'D'(Name Prop Kind Value) then L in
	    {Dictionary.get EAttMap Name
	     attribute(name:Prop kind:Kind value:Value)|L}
	    {Dictionary.put EAttMap Name L}
	 [] 'a'(Type Prop Kind Value) then L in
	    {Exchange Link L
	     linkAttribute(
		type:Type
		name:Prop kind:Kind value:Value)|L}
	 [] 'N'(Name) then
	    {Dictionary.put EMap Name
	     notation(name:Name id:{Idents})}
	 [] 'E'(Ename Type Nname) then
	    {Dictionary.put EMap Ename
	     edata(name:Ename type:Type notation:Nname id:{Idents})}
	 [] 'I'(Ename Type Text) then
	    {Dictionary.put EMap Ename
	     idata(name:Ename type:Type text:Text)}
	 [] 'S'(Ename) then
	    {Dictionary.put EMap Ename
	     subdoc(Ename id:{Idents})}
	 [] 'T'(Ename) then
	    {Dictionary.put EMap Ename
	     esgml(name:Ename id:{Idents})}
	 [] 's'(ID) then {Assign SysId ID}
	 [] 'p'(ID) then {Assign PubId ID}
	 [] 'f'(ID) then {Assign GenId ID}
	 [] '{'(Ename) then skip
	 [] '}'(Ename) then skip
	 [] 'L'(Line) then skip
	 [] 'L'(Line File) then skip
	 [] '#'(Text) then skip
	 [] 'C' then raise done end
	 [] 'i' then skip
	 [] 'e' then skip
	 end
	 {Loop}
      end
   in
      try
	 {Loop}
	 {Exception.raiseError
	  ExceptionLabel(cEventNotFound)}
	 unit
      catch done then
	 {ForAll {Dictionary.items EAttMap} proc {$ L} L=nil end}
	 document(docElem:DocElem emap:{Dictionary.toRecord emap EMap})
      end
   end 

   class SgmlParser
      meth init skip end
      meth process(Files Document program:P<=unit catalog:C<=unit)
	 Pgm = case P of unit then {Access ParamProgram} else P end
	 Cat = case C of unit then {Access ParamCatalog} else C end
      in
	 {Parse
	  {MakeEventGenerator Pgm '-c'#Cat|Files}
	  Document}
      end
   end

   SgmlParserObject = {New SgmlParser init}
end
