functor

import
   QUI
   QHTMLDevel(join:Join
	      get:Get
	      undefined:Undefined)
   HTMLmarshaller(hTML2VS:Html2vs
		  vS2HTML:Vs2html1)
   
export
   CheckType
   ExistEvent
   Event2OZ
   RegisterType
   GetTypeInfo
   ExistType
   EnableChecking
   Translate2HTML
   Translate2OZ
   Marshall
   Unmarshall
   Record2str
   Css2Str

define

   CheckDict={NewDictionary}
   Checking={NewCell true}
   EventDict={NewDictionary}

   %% type stuff

   fun{LOf R} %% list of elements
      fun{$ E}
	 {List.is E} andthen {List.all E R}
      end
   end

   fun{IsPct D}
      case D
      of I#'%' then {Int.is I}
      else false end
   end

   fun{IsLength D}
      case D
      of I#E then {Float.is I} andthen {List.member E [px em ex 'in' cm mm pi pt]}
      else false end
   end

   fun{IsColor D}
      fun{IsCol I}
	 {IsPct I} orelse {Int.is I} andthen I>=0 andthen I=<255
      end
   in
      case D
      of rgb(A B C) then {IsCol A} andthen {IsCol B} andthen {IsCol C}
      [] X then {Atom.is X}
      else false end
   end

   fun{IsURL D}
      case D of url(X) then {VirtualString.toString X}
      else false end
   end

   %% transformation stuff

   fun{Vs2html V} {Quote {Vs2html1 V}} end
   
   fun{Str2distance V}
      L R
   in
      {List.takeDropWhile {Reverse V} fun{$ C} C<&0 orelse C>&9 end R L}
      if R==nil then
	 X
      in
	 try X={String.toFloat V} catch _ then skip end
	 try X={String.toInt V} catch _ then skip end
	 try X=V catch _ then skip end
	 X
      elseif L==nil then {String.toAtom V}
      elseif R=="%" then
	 {String.toInt {Reverse L}}#'%'
      else
	 X LL={Reverse L}
      in
	 try X={String.toFloat LL} catch _ then skip end
	 try X={Int.toFloat {String.toInt LL}} catch _ then skip end
	 try X=V catch _ then skip end
	 X#{String.toAtom {Reverse R}}
      end
   end

   fun{Str2record V}
      fun{Skip Str Remain Close Stop}
	 case Str of C|Cs then
	    case C
	    of &\\ then
	       case Cs of D|Ds then C|D|{Skip Ds Remain Close Stop}
	       else Remain=nil C end
	    []!Close then
	       C|{Get Cs Remain Stop}
	    else
	       C|{Skip Cs Remain Close Stop}
	    end
	 else
	    Remain=nil nil
	 end
      end
      fun{Get Str Remain Stop}
	 case Str of C|Cs then
	    if {List.member C Stop} then % stops here
	       Remain=Str
	       nil
	    else
	       case C
	       of &\\ then
		  case Cs of D|Ds then C|D|{Get Ds Remain Stop} %% ignore next character
		  else Remain=nil C end
	       [] &" then
		  C|{Skip Cs Remain &" Stop}
	       [] &' then
		  C|{Skip Cs Remain &' Stop}
	       else C|{Get Cs Remain Stop}
	       end
	    end
	 else
	    Remain=nil nil
	 end
      end
      fun{Parse Str}
	 R
	 P1={Get Str R ",)"}
      in
	 if P1==nil then nil else
	    P2=if P1.1==&" orelse P1.1==&' then % "
		  {List.take {List.drop P1 1} {Length P1}-2}
	       else P1 end
	 in
	    P2|{Parse {List.drop R 1}}
	 end
      end
      Remain
      Label={Get V Remain "("}
   in
      {List.toTuple {VirtualString.toAtom Label} {Parse {List.drop Remain 1}}}
   end

   fun{Split V Sep}
      fun{Skip Str Remain Close Stop}
	 case Str of C|Cs then
	    case C
	    of &\\ then
	       case Cs of D|Ds then C|D|{Skip Ds Remain Close Stop}
	       else Remain=nil C end
	    []!Close then
	       C|{Get Cs Remain Stop}
	    else
	       C|{Skip Cs Remain Close Stop}
	    end
	 else
	    Remain=nil nil
	 end
      end
      fun{Get Str Remain Stop}
	 case Str of C|Cs then
	    if {List.member C Stop} then % stops here
	       Remain=Str
	       nil
	    else
	       case C
	       of &\\ then
		  case Cs of D|Ds then C|D|{Get Ds Remain Stop} %% ignore next character
		  else Remain=nil C end
	       [] &" then
		  C|{Skip Cs Remain &" Stop}
	       [] &' then
		  C|{Skip Cs Remain &' Stop}
	       else C|{Get Cs Remain Stop}
	       end
	    end
	 else
	    Remain=nil nil
	 end
      end
      fun{Parse Str}
	 R
	 P1={Get Str R ","}
      in
	 if P1==nil then nil else
	    P2=if P1.1==&" orelse P1.1==&' then %"
		  {List.take {List.drop P1 1} {Length P1}-2}
	       else P1 end
	 in
	    P2|{Parse {List.drop R 1}}
	 end
      end
   in
      {Parse V}
   end
   
   fun{Record2str V}
      if {Atom.is V} then '"'#V#'"' else
	 '"'#{Label V}#"("#{Join {Record.toList V} ","}#")"#'"'
      end
   end

   % public functions

   fun{Noop V} V end

   fun{Quote V} '"'#V#'"' end
   
   proc{RegisterType Type Proc Str Marshall Unmarshall}
      CheckDict.Type := r(p:Proc s:Str m:Marshall u:Unmarshall)
   end

   fun{ExistType Type}
      {Procedure.is Type} orelse
      {Dictionary.member CheckDict Type}
   end

   fun{GetTypeInfo Type}
      {Dictionary.get CheckDict Type}
   end

   fun{CheckCss D V}
      if {Atom.is D} then
	 D==V
      elsecase {Label D}
      of switch then
	 {Record.some D fun{$ E} {CheckCss E V} end}
      [] repeat then
	 if {List.is V} then
	    L={Length V}
	 in
	    ((L>=D.2) andthen ((D.3==inf) orelse (L=<D.3))) andthen
	    {List.all V fun{$ E} {CheckCss D.1 E} end}
	 else
	    {CheckCss D.1 V}
	 end
      [] succ then
	 {Record.is V} andthen
	 local
	    fun{Loop A B}
	       case A
	       of X|Xs then
		  case X of repeat(_ 0 _) then
		     %% next element is optional
		     case B of Y|Ys then
			if {CheckCss X Y} then
			   %% element was present
			   {Loop Xs Ys}
			else
			   %% element was missig
			   {Loop Xs B}
			end
		     else
			%% no more elements => assume missing
			{Loop Xs B}
		     end
		  else
		     case B of Y|Ys then
			{CheckCss X Y} andthen {Loop Xs Ys}
		     else
			false
		     end
		  end
	       else
		  case B of _|_ then
		     false % too many elements
		  else
		     true
		  end
	       end
	    end
	 in
	    {Loop {Record.toList D}
	     {Record.toList if {Atom.is V} then r(V) else V end}}
	 end
      [] aggregate then
	 {Record.is V} andthen
	 local
	    fun{Loop Pb Dt}
	       case Dt of X|Xs then
		  NPb
		  fun{Match L}
		     case L of Y|Ys then
			if {CheckCss Y X} then
			   NPb={List.subtract Pb Y}
			   true
			else
			   {Match Ys}
			end
		     else false end
		  end
	       in
		  if {Match Pb} then
		     {Loop NPb Xs}
		  else
		     false
		  end
	       else
		  true
	       end
	    end
	 in
	    {Loop {Record.toList D}
	     {Record.toList if {Atom.is V} then r(V) else V end}}
	 end
      [] type then
	 case D.1
	 of 'absolute-size' then
	    {List.member V ['xx-small' 'x-small' small medium large 'x-large' 'xx-large']}
	 [] angle then
	    case V of N#U then
	       ({Int.is N} orelse {Float.is N}) andthen
	       {List.member U [deg grad rad]}
	    else false end
	 [] 'border-style' then
	    {List.member V [none hidden dotted dashed solid double groove ridge inset outset]}
	 [] 'border-width' then
	    {List.member V [thin medium thick]} orelse {CheckCss type(length) V}
	 [] color then
	    case V
	    of rgb(A B C) then
	       {List.all [A B C]
		fun{$ E}
		   ({Int.is E} andthen (E>=0) andthen (E=<255)) orelse
		   case E of N#'%' then {Int.is N} andthen (N>=0) andthen (N=<100)
		   else false end
		end}
	    else {Atom.is V} end % should check if atom is a correct color name
	 [] counter then
	    {Record.is V} andthen ({Label V}==counter)
	 [] 'family-name' then
	    {String.is V}
	 [] frequency then
	    case V of N#U then
	       {Float.is N} andthen {List.member U ['Hz' kHz]}
	    else false end
	 [] 'generic-family' then
	    {List.member V [serif 'sans-serif' cursive fantasy monospace]}
	 [] 'generic-voice' then
	    {List.member V [male female child]}
	 [] identifier then
	    {Atom.is V}
	 [] integer then
	    {Int.is V}
	 [] length then
	    case V
	    of N#U then
	       ({Int.is N} orelse {Float.is N})
	       andthen {List.member U [em ex px 'in' cm mm pt pc]}
	    else
	       {Int.is V} orelse {Float.is V}
	    end
	 [] 'margin-width' then
	    (V==auto) orelse {CheckCss type(length) V} orelse {CheckCss type(percentage) V}
	 [] number then
	    {Int.is V} orelse {Float.is V}
	 [] 'padding-width' then
	    case V
	    of N#U then
	       ({Int.is N} orelse {Float.is N})
	       andthen {List.member U [em ex px 'in' cm mm pt pc '%']}
	    else
	       {Int.is V} orelse {Float.is V}
	    end
	 [] percentage then
	    case V
	    of N#'%' then {Int.is N} orelse {Float.is N}
	    else false
	    end
	 [] 'relative-size' then
	    {List.member V [smaller larger]}
	 [] shape then
	    case V of rect(A B C D) then
	       {List.all [A B C D]
		fun{$ E} (E==auto) orelse {CheckCss type(length) E} end}
	    else false end
	 [] 'specific-voice' then
	    {String.is V}
	 [] string then
	    {String.is V}
	 [] time then
	    case V of N#U then
	       {Float.is N} andthen {List.member U [ms s]}
	    else false end
	 [] uri then
	    case V of url(S) then {String.is S}
	    else false end
	 end
      else true end
   end

   fun{JoinS L S}
      case L
      of Y|nil then
	 Y
      [] Y|Ys then
	 fun{Add Ll Lr}
	    case Ll of X|Xs then
	       X|{Add Xs Lr}
	    elsecase Lr of X|Xs then
	       X|{Add Ll Xs}
	    else
	       {JoinS Ys S}
	    end
	 end
      in
	 {Add Y S}
      else
	 nil
      end
   end

   fun{Parenth E}
      fun{Loop1 R}
	 case R
	 of &(|Xs then {Loop2 0 Xs}
	 [] & |_ then false
	 [] _|Xs then {Loop1 Xs}
	 else false end
      end
      fun{Loop2 Depth R}
	 case R
	 of &)|nil then true
	 [] &(|Xs then {Loop2 Depth+1 Xs}
	 [] &)|Xs then if Depth==0 then false else {Loop2 Depth-1 Xs} end
	 [] _|Xs then {Loop2 Depth Xs}
	 else false end
      end
      R={Css2Str E}
   in
      if {List.member &  R} then
	 if {Loop1 R} then
	    R
	 else
	    {VirtualString.toString "("#R#")"}
	 end
      else
	 R
      end
   end

   fun{Css2Str D}
      if {Atom.is D} then
	 {Atom.toString D}
      elsecase {Label D}
      of switch then
	 {JoinS {List.map {Record.toList D} fun{$ E} {Parenth E} end} " | "}
      [] repeat then
	 if (D.2==0) andthen (D.3==1) then
	    {VirtualString.toString {Parenth D.1}#"?"}
	 elseif (D.2==0) andthen (D.3==inf) then
	    {VirtualString.toString {Parenth D.1}#"*"}
	 elseif (D.2==1) andthen (D.3==2) then
	    {VirtualString.toString {Parenth D.1}#" "#{Parenth D.1}#"?"}
	 elseif (D.2==1) andthen (D.3==inf) then
	    {VirtualString.toString {Parenth D.1}#" "#{Parenth D.1}#"*"}
	 else
	    {VirtualString.toString {Parenth D.1}#"["#D.2#","#D.3#"]"}
	 end
      [] succ then
	 {JoinS {List.map {Record.toList D} fun{$ E} {Parenth E} end} " "}
      [] aggregate then
	 {JoinS {List.map {Record.toList D} fun{$ E} {Parenth E} end} " || "}
      [] type then
	 case D.1
	 of 'absolute-size' then
	    "'xx-small' | 'x-small' | small | medium | large | 'x-large' | 'xx-large'"
	 [] angle then
	    "IntOrFloat#(deg | grad | rad)"
	 [] 'border-style' then
	    "none | hidden | dotted | dashed | solid | double | groove | ridge | inset | outset"
	 [] 'border-width' then
	    "thin | medium | thick | IntOrFloat | IntOrFloat#(em | ex | px | 'in' | cm | mm | pt | pc) "
	 [] color then
	    "rgb( (Int#'%' | Int) (Int#'%' | Int) (Int#'%' | Int) ) | Atom"
	 [] counter then
	    "counter( Atom Atom? Atom?)"
	 [] 'family-name' then
	    "VS"
	 [] frequency then
	    "Float#( 'Hz' | kHz )"
	 [] 'generic-family' then
	    "serif | 'sans-serif' | cursive | fantasy | monospace"
	 [] 'generic-voice' then
	    "male | female | child"
	 [] identifier then
	    "Atom"
	 [] integer then
	    "Int"
	 [] length then
	    "IntOrFloat | IntOrFloat#(em | ex | px | 'in' | cm | mm | pt | pc)"
	 [] 'margin-width' then
	    "auto | IntOrFloat | IntOrFloat#(em | ex | px | 'in' | cm | mm | pt | pc | '%')"
	 [] number then
	    "IntOrFloat"
	 [] 'padding-width' then
	    "IntOrFloat | IntOrFloat#(em | ex | px | 'in' | cm | mm | pt | pc | '%')"
	 [] percentage then
	    "IntOrFloat#'%'"
	 [] 'relative-size' then
	    "smaller | larger"
	 [] shape then
	    "rect( (auto | IntOrFloat | IntOrFloat#(em | ex | px | 'in' | cm | mm | pt | pc) )[4,4] )"
	 [] 'specific-voice' then
	    "VS"
	 [] string then
	    "VS"
	 [] time then
	    "Float#(ms | s)"
	 [] uri then
	    "url(VS)"
	 end
      else true end
   end

   fun{Escape Str}
      case Str
      of &'|Xs then
	 &\\|&'|{Escape Xs}
      [] &"|Xs then
	 &\\|&"|{Escape Xs}
      [] &\\|Xs then
	 &\\|&\\|{Escape Xs}
      [] X|Xs then
	 X|{Escape Xs}
      else nil end
   end   

   fun{Oz2Css _ V}
      if {Atom.is V} then
	 {Atom.toString V}
      elseif {Int.is V} then
	 {Int.toString V}
      elseif {Float.is V} then
	 {Float.toString V}
      elseif {Record.is V} then
	 case {Label V}
	 of '#' then {VirtualString.toString {Record.map V fun{$ E} {Oz2Css _ E} end}}
	 [] '|' then {VirtualString.toString '"'#{Escape V}#'"'}
	 else
	    {VirtualString.toString {Label V}#"("#{JoinS
						   {List.map {Record.toList V} Oz2Css}
						   " "}#")"}
	 end
      end
   end

   fun{Css2Oz _ V}
      V
   end
   
   proc{CheckType Value Type M}
      if {Access Checking} then
	 case Type of css(X) then
	    if {Not {CheckCss X Value}} then
	       {QUI.raiseError Value "Invalid type, expecting : "#{Css2Str X} M}
	    end
	 elseif {List.is Type} then
	    if {Not {List.member Value Type}} then
	       {QUI.raiseError Value "Invalid type, expecting : one of "#{Join Type ", "} M}
	    end
	 elseif {Procedure.is Type} then
	    if {Not {Type Value}} then
	       {QUI.raiseError Value "Invalid type" M}
	    end
	 elseif {Not {(CheckDict.Type).p Value}} then
	    {QUI.raiseError Value "Invalid type, expecting : "#(CheckDict.Type).s M}
	 end
      end
   end

   proc{EnableChecking T}
      if {Bool.is T} then
	 {Assign Checking T}
      end
   end

   fun{Translate P V Obj}
      if V==Undefined then V else if {Procedure.arity P}==2 then {P V} else {P V Obj} end end
   end
   
   fun{Translate2HTML V Type}
      {Translate {Dictionary.condGet CheckDict Type r(m:Noop)}.m V unit}
   end

   fun{Translate2OZ V Type}
      {Translate {Dictionary.condGet CheckDict Type r(u:Noop)}.u {VirtualString.toString V} unit}
   end

   fun{Marshall Obj Info V}
      {Translate
       if {HasFeature Info marshall} then
	  Info.marshall
       else
	  Type={CondSelect Info type no}
       in
	  if {List.is Type} then Noop
	  elsecase Type of css(X) then
	     fun{$ V} {Oz2Css X V} end
	  else
	     {Dictionary.condGet CheckDict Type r(m:Noop)}.m
	  end
       end V Obj}
   end

   fun{Unmarshall Obj Info V}
      {Translate
       if {HasFeature Info unmarshall} then
	  Info.unmarshall
       else
	  Type={CondSelect Info type no}
       in
	  if {List.is Type} then Noop
	  elsecase Type of css(X) then
	     fun{$ V} {Css2Oz X V} end
	  else
	     {Dictionary.condGet CheckDict Type r(u:Noop)}.u
	  end
       end V Obj}
   end
   
   fun{ExistEvent E} {Dictionary.member EventDict E} end

   fun{Event2OZ V Type}
      case V of &0|Xs then
	 if {ExistEvent Type} then {EventDict.Type Xs} else Undefined end
      else Undefined end
   end

   %% type definitions

   {ForAll [no(c:fun{$ _} true end
	       s:"Anything"
	       m:Noop
	       u:Noop)
	    vs(c:VirtualString.is
	       s:"A Virtual String"
	       m:Quote
	       u:Noop)
	    html(c:VirtualString.is
		 s:"A Virtual String"
		 m:Vs2html
		 u:Html2vs)
	    int(c:Int.is
		s:"An integer"
		m:Noop
		u:String.toInt)
	    float(c:Float.is
		  s:"A float"
		  m:Noop
		  u:String.toFloat)
	    atom(c:Atom.is
		 s:"An atom"
		 m:Quote
		 u:String.toAtom)
	    record(c:Record.is
		   s:"A record"
		   m:Quote
		   u:Str2distance)
	    list(c:List.is
		 s:"A list"
		 m:Noop
		 u:Noop)
	    event(c:QUI.isEvent
		  s:"An event's action (0 parameter procedure or Objet#Method or Port#Message)"
		  m:Noop
		  u:Noop)
	    bool(c:Bool.is
		 s:"true or false"
		 m:fun{$ V} if V then "true" else "false" end end
		 u:fun{$ V} V=="true" end)
	    lvs(c:{LOf VirtualString.is}
		s:"A list of virtual strings"
		m:fun{$ V} {Quote {Join V ","}} end
		u:fun{$ V} {Split V ","} end)
	    lint(c:{LOf Int.is}
		 s:"A list of integers"
		 m:fun{$ V} {Quote {Join V ","}} end
		 u:fun{$ V} {List.map {Split V ","} String.toInt} end)
	    lfloat(c:{LOf Float.is}
		   s:"A list of floats"
		   m:fun{$ V} {Quote {Join V ","}} end
		   u:fun{$ V} {List.map {Split V ","} String.toFloat} end)
	    latom(c:{LOf Atom.is}
		  s:"A list of atoms"
		  m:fun{$ V} {Quote {Join V ","}} end
		  u:fun{$ V} {List.map {Split V ","} String.toAtom} end)
	    lbool(c:{LOf Bool.is}
		  s:"A list of booleans"
		  m:fun{$ V} {Quote {Join V ","}} end
		  u:fun{$ V} {List.map {Split V ","} fun{$ V} V=="true" end} end)
	    framewidth(c:{LOf fun{$ E}
				 E=='*' orelse {IsPct E} orelse {Int.is E}
			      end}
		       s:"A list of integers/pair integer#'%'/atom '*'"
		       m:fun{$ V} {Quote {Join V ","}} end
		       u:Noop)
	    qhtmldesc(c:Record.is
		      s:"A valid QHTML description record"
		      m:Noop
		      u:Noop)
	    length(c:IsLength
		   s:"A pair float#atom where atom is one of : px, em, ex, in, cm, mm, pi or pt"
		   m:Quote
		   u:Str2distance)
	    pct(c:IsPct
		s:"A pair integer#'%'"
		m:Quote
		u:Str2distance)
	    intlength(c:fun{$ V} {Int.is V} orelse {IsLength V} end
		      s:"An integer or a pair float#atom where atom is one of : px, em, ex, in, cm, mm, pi or pt"
		      m:Quote
		      u:Str2distance)
	    lengthpct(c:fun{$ V} {IsPct V} orelse {IsLength V} end
		      s:"A pair integer#'%' or a pair float#atom where atom is one of : px, em, ex, in, cm, mm, pi or pt"
		      m:Quote
		      u:Str2distance)
	    intpct(c:fun{$ V} {IsPct V} orelse {Int.is V} end
		   s:"An integer or a pair integer#'%'"
		   m:Quote
		   u:Str2distance)
	    intlengthpct(c:fun{$ V} {IsPct V} orelse {Int.is V} orelse {IsLength V} end
			 s:"An integer, a pair integer#'%' or a pair float#atom where atom is one of : px, em, ex, in, cm, mm, pi or pt"
			 m:Quote
			 u:Str2distance)
	    widthlength(c:fun{$ V} V==auto orelse V==glue orelse {IsPct V} orelse {Int.is V} orelse {IsLength V} end
			s:"The atoms auto or glue, an integer, a pair integer#'%' or a pair float#atom where atom is one of : px, em, ex, in, cm, mm, pi or pt"
			m:fun{$ V O} {Quote if V==glue then if {O Get(glue $)}.exph then "100%" else "auto" end else V end} end
			u:fun{$ V O} if {O Get(width $)}==glue then glue else {Str2distance V} end end)
			     
	    heightlength(c:fun{$ V} V==auto orelse V==glue orelse {IsPct V} orelse {Int.is V} orelse {IsLength V} end
			 s:"The atoms auto or glue, an integer, a pair integer#'%' or a pair float#atom where atom is one of : px, em, ex, in, cm, mm, pi or pt"
			 m:fun{$ V O} {Quote if V==glue then if {O Get(glue $)}.expv then "100%" else "auto" end else V end} end
			 u:fun{$ V O} if {O Get(height $)}==glue then glue else {Str2distance V} end end)
	    autointlengthpct(c:fun{$ V} V==auto orelse {IsPct V} orelse {Int.is V} orelse {IsLength V} end
			     s:"The atom auto, an integer, a pair integer#'%' or a pair float#atom where atom is one of : px, em, ex, in, cm, mm, pi or pt"
			     m:Quote
			     u:Str2distance)
	    autolengthpct(c:fun{$ V} V==auto orelse {IsPct V} orelse {IsLength V} end
			  s:"The atom auto, a pair integer#'%' or a pair float#atom where atom is one of : px, em, ex, in, cm, mm, pi or pt"
			  m:Quote
			  u:Str2distance)
	    normalintlengthpct(c:fun{$ V} V==normal orelse {IsPct V} orelse {Int.is V} orelse {IsLength V} end
			       s:"The atom normal, an integer, a pair integer#'%' or a pair float#atom where atom is one of : px, em, ex, in, cm, mm, pi or pt"
			       m:Quote
			       u:Str2distance)
	    normallengthpct(c:fun{$ V} V==normal orelse {IsPct V}  orelse {IsLength V} end
			    s:"The atom normal, a pair integer#'%' or a pair float#atom where atom is one of : px, em, ex, in, cm, mm, pi or pt"
			    m:Quote
			    u:Str2distance)
	    normallength(c:fun{$ V} V==normal  orelse {IsLength V} end
			 s:"The atom normal or a pair float#atom where atom is one of : px, em, ex, in, cm, mm, pi or pt"
			 m:Quote
			 u:Str2distance)
	    normalintpct(c:fun{$ V} V==normal orelse {IsPct V} orelse {Int.is V} end
			 s:"The atom normal, an integer, a pair integer#'%'"
			 m:Quote
			 u:Str2distance)
	    autononelengthpct(c:fun{$ V} V==none orelse V==auto orelse {IsPct V}  orelse {IsLength V} end
			      s:"The atom none, the atom auto, a pair integer#'%' or a pair float#atom where atom is one of : px, em, ex, in, cm, mm, pi or pt"
			      m:Quote
			      u:Str2distance)
	    backgroundpos(c:fun{$ V} V==center orelse V==top orelse V==bottom orelse {IsPct V} orelse {IsLength V} end
			  s:"One of the atoms center top or bottom, a pair integer#'%' or a pair float#atom where atom is one of : px, em, ex, in, cm, mm, pi or pt"
			  m:Quote
			  u:Str2distance)
	    backgroundpos2(c:fun{$ V}
				case V of r(A B) then
				   ({List.member A [top center bottom]} orelse {IsPct A} orelse {IsLength A})
				   andthen
				   ({List.member B [left center right]} orelse {IsPct B} orelse {IsLength B})
				else false end
			     end
			   s:"A record of the form r(A B) where A is the atom top, center or bottom, or a pair integer#'%' or a pair float#atom where atom is one of : px, em, ex, in, cm, mm, pi or pt; and B is the atom left, center or right, or a pair integer#'%' or a pair float#atom where atom is one of : px, em, ex, in, cm, mm, pi or pt"
			   m:fun{$ V} {Quote V.1#" "#V.2} end
			   u:fun{$ V} L R in {List.takeDropWhile V fun{$ C} C\=&  end L R}
				r({Str2distance L}
				  if R=="" then 0#'%' else {Str2distance {List.drop R 1}} end)
			     end)
	    fontsize(c:fun{$ V} {List.member V ['xx-small' 'x-small' small medium large 'x-large' 'xx-large' larger smaller]}
			  orelse {IsPct V}  orelse {IsLength V} end
		     s:"One of theses atoms : 'xx-small' 'x-small' small medium large 'x-large' 'xx-large' larger smaller, a pair integer#'%' or a pair float#atom where atom is one of : px, em, ex, in, cm, mm, pi or pt"
		     m:Quote
		     u:Str2distance)
	    borderwidth(c:fun{$ V} {List.member V [medium thin thick]} orelse {IsLength V} end
			s:"One of theses atoms : medium thin thick, or a pair float#atom where atom is one of : px, em, ex, in, cm, mm, pi or pt"
			m:Quote
			u:Str2distance)
	    color(c:IsColor
		  s:"An atom which represents a color or a record of the form rgb(R G B) with R, G and B integers between 0 and 255 or pairs integer#'%'"
		  m:Record2str
		  u:Str2record)
	    transparentcolor(c:fun{$ V} V==transparent orelse {IsColor V} end
			     s:"The atom transparent or an atom which represents a color or a record of the form rgb(R G B) with R, G and B integers between 0 and 255 or pairs integer#'%'"
			     m:Record2str
			     u:Str2record)
	    url(c:IsURL
		s:"A record of the form url(VS) with VS a virtual string"
		m:Record2str
		u:Str2record)
	    noneurl(c:fun{$ V} V==none orelse {IsURL V} end
		    s:"The atom none or a record of the form url(VS) with VS a virtual string"
		    m:{Quote fun{$ V} if V==none then V else {Record2str V} end end}
		    u:fun{$ V} if V=="none" then none else {Str2record V} end end)
	    autoint(c:fun{$ V} V==auto orelse {Int.is V} end
		    s:"The atom auto or an integer"
		    m:Quote
		    u:Str2distance)
	    clip(c:fun{$ V} V==auto orelse
		      case V of rect(X1 Y1 X2 Y2) then {List.all [X1 Y1 X2 Y2]
							fun{$ E} E==auto orelse {IsLength E} orelse {Int.is E} end}
		      else false end end
		 s:"A record of the form rect(X1 Y1 X2 Y2) where X1,Y1,X2 and Y2 are valid lengths or integers"
		 m:Record2str
		 u:fun{$ V} {Record.map {Str2record V} Str2distance} end)
	    inheritpct(c:fun{$ V} V==inherit orelse {IsPct V}  end
		       s:"The atom inherit or  a pair integer#'%'"
		       m:Quote
		       u:Str2distance)
	   ]
    proc{$ L}
       {RegisterType {Label L} L.c L.s L.m L.u}
    end}

   {ForAll
    [altKey#bool
     button#int
     cancelBubble#bool
     clientX#int
     clientY#int
     ctrlKey#bool
     keyCode#int
     offsetX#int
     offsetY#int
     returnValue#bool
     screenX#int
     screenY#int
     shiftKey#bool
     type#vs
     x#int
     y#int]
    proc{$ P#M}
       EventDict.P := fun{$ V} {Translate2OZ V M} end
    end}

   
%    fun{Str2intdash V}
%       L R
%    in
%       {List.takeDropWhile V fun{$ C} C>=&0 andthen C=<&9 end L R}
%       if R==nil then
% 	 {String.toInt L}
%       else
% 	 {String.toInt L}#{String.toAtom R}
%       end
%    end

%    fun{Str2atomintdash V}
%       X Y
%       try X={Str2intdash V} catch _ then Y={VirtualString.toAtom V} end
%    in if {IsDet Y} then Y else X end
%    end

%    fun{Str2floatdash V}
%       L R
%    in
%       {List.takeDropWhile {Reverse V} fun{$ C} C<&0 orelse C>&9 end R L}
%       if R==nil then
% 	 X
%       in
% 	 try X={String.toFloat V} catch _ then skip end
% 	 try X={String.toInt V} catch _ then skip end
% 	 try X={String.toAtom V} catch _ then skip end
% 	 try X=V catch _ then skip end
% 	 X
%       else
% 	 {String.toFloat {Reverse L}}#{String.toAtom {Reverse R}}
%       end
%    end
%
%    {ForAll [no#fun{$ _} true end#""#no#no
% 	    vs#VirtualString.is#"Virtual String"#no#no
% 	    int#Int.is#"Integer"#int2vs#vs2int
% 	    float#Float.is#"Float"#float2vs#vs2float
% 	    atom#Atom.is#"Atom"#atom2vs#vs2atom
% 	    event#QUI.isEvent#"Event (0 parameter procedure or Objet#Method or Port#Message)"
% 	    bool#Bool.is#"Boolean"#bool2vs#vs2bool
% 	    lvs#{LOf VirtualString.is}#"List of Virtual Strings"
% 	    lint#{LOf Int.is}#"List of Integers"
% 	    latom#{LOf Atom.is}#"List of Atoms"
% 	    levent#{LOf QUI.isEvent}#"List of Events"
% 	    lbool#{LOf Bool.is}#"List of Booleans"
% 	    qhtmldesc#Record.is#"A valid QHTML description"
% 	    length#IsLength#"A valid length (pair float#code where code is one of these atoms : px, em, ex, in, cm, mm, pi or pt)"#floatdash2vs#vs2floatdash
% 	    pct#IsPct#"A pair of the form integer#'%'"#intdash2vs#vs2intdash
% 	    intlength#fun{$ P} {IsLength P} orelse {Int.is P} end#"An integer or a valid length (pair integer#code where code is one of these atoms : px, em, ex, in, cm, mm, pi or pt)"
% 	    lengthpct#fun{$ P} {IsLength P} orelse {IsPct P} end#"A pair float#code where code is one of these atoms : px, em, ex, in, cm, mm, pi or pt or a pair integer#'%')"#floatdashpct2vs#vs2floatdashpct
% 	    intpct#fun{$ P} {Int.is P} orelse {IsPct P} end#"An integer or a pair of the form integer#'%'"
% 	    intlengthpct#fun{$ P} {Int.is P} orelse {IsLength P} orelse {IsPct P} end#"An integer or a valid length (pair integer#code where code is one of these atoms : px, em, ex, in, cm, mm, pi, pt or '%')"
% 	    autoglueintlengthpct#fun{$ P} P==auto orelse P==glue orelse {Int.is P} orelse {IsLength P} orelse {IsPct P} end#"The atom auto or the atom glue or a valid length"
% 	    autointlengthpct#fun{$ P} P==auto orelse {Int.is P} orelse {IsPct P} orelse {IsLength P} end#"The atom auto or a valid length"
% 	    autolengthpct#fun{$ P} P==auto orelse {IsPct P} orelse {IsLength P} end#"The atom auto or a valid length"
% 	    normalintlengthpct#fun{$ P} P==normal orelse {Int.is P} orelse {IsPct P} orelse {IsLength P} end#"The atom normal or a valid length"
% 	    color#IsColor#"An atom which represents a record, or a record rgb(RR GG BB) where RR,GG and BB are either integer between 0 and 255 or percentage value (pair integer#'%')"
% 	    url#IsURL#"A record of the form url(URL) where URL is a virtual string"
% 	    autoint#fun{$ P} P==auto orelse {Int.is P} end#"The atom auto or an integer"
% 	    clip#fun{$ P}
% 		    case P of rect(X1 Y1 X2 Y2) then {List.all [X1 Y1 X2 Y2]
% 						      fun{$ E} E==auto orelse {IsLength E} orelse {Int.is E} end}
% 		    else false end end#"A record of the form rect(X1 Y1 X2 Y2) where X1,Y1,X2 and Y2 are valid lengths"
% 	    inheritintpct#fun{$ P} P==inherit orelse {Int.is P} orelse {IsPct P} end#"The atom inherit or an integer or a pair integer#'%'"
% 	    noneurl#fun{$ P} P==none orelse {IsURL P} end#"The atom none or a record of the form url(URL) where URL is a virtual string"

	    
	    
% % 	    length#IsLength#"A valid length (integer or pair integer#code where code is one of px, em, ex, in, cm, mm, pi or pt)"
% % 	    wh#fun{$ P} P==auto orelse P==glue orelse {IsIntOrPct P} orelse {IsLength P} end#"The atom auto or the atom glue or a valid length"
% % 	    autolength#fun{$ P} P==auto orelse {IsIntOrPct P} orelse {IsLength P} end#"The atom auto or a valid length"
% % 	    normallength#fun{$ P} P==normal orelse {IsIntOrPct P} orelse {IsLength P} end#"The atom normal or a valid length"
% % 	    color#fun{$ P} if {Atom.is P} then true
% % 			   elsecase P of rgb(R G B) then {IsIntOrPct R} andthen {IsIntOrPct G} andthen {IsIntOrPct B}
% % 			   else false end
% % 		  end#"An atom which represents a record, or a record rgb(RR GG BB) where RR,GG and BB are either integer between 0 and 255 or percentage value (pair integer#'%')"
% % 	    urlornone#fun{$ P} case P
% % 			       of none then true
% % 			       [] url(_) then true
% % 			       else false end end#"The atom none or a record of the form url(URL) where URL is a virtual string"
% % 	    url#fun{$ P} case P
% % 			 of url(_) then true
% % 			 else false end end#"A record of the form url(URL) where URL is a virtual string"
% % 	    autoorint#fun{$ P} P==auto orelse {Int.is P} end#"The atom auto or an integer"
% % 	    clip#fun{$ P}
% % 		    case P of rect(X1 Y1 X2 Y2) then {List.all [X1 Y1 X2 Y2]
% % 						      fun{$ E} E==auto orelse {IsLength E} orelse {Int.is E} end}
% % 		    else false end end#"A record of the form rect(X1 Y1 X2 Y2) where X1,Y1,X2 and Y2 are valid lengths"
% % 	    intorpct#IsIntOrPct#"An integer or a pair integer#'%'"
% % 	    lengthorpct#fun{$ P} {IsIntOrPct P} orelse {IsLength P} end#"A valid length or a pair integer#'%'"
% % 	    textkashidaspace#fun{$ P} P==inherit orelse {IsIntOrPct P} end#"The atom inherit or an integer or a pair integer#'%'"
% 	   ]
    
%     proc{$ I}
%        case I
%        of T#P#S then
% 	  {RegisterType T P S no no}
%        [] T#P#S#M#U then
% 	  {RegisterType T P S M U}
%        end
%     end}

%    %% transformation definitions

%    {ForAll [no#fun{$ V _} V end
% 	    html2vs#fun{$ V _} {Html2vs V} end
% 	    vs2html#fun{$ V _} {Vs2html V} end
% 	    html2atom#fun{$ V _} {String.toAtom {Html2vs V}} end
% 	    atom2html#fun{$ V _} {Vs2html V} end
% 	    vs2atom#fun{$ V _} {VirtualString.toAtom V} end
% 	    atom2vs#fun{$ V _} V end
% 	    width2vs#fun{$ V O}
% 			if V==glue then
% 			   if {O Get(glue $)}.exph then "100%" else "auto" end
% 			else V end
% 		     end
% 	    vs2width#fun{$ V O}
% 			if {O Get(width $)}==glue then glue else
% 			   {Str2intdash V}
% 			end
% 		     end
% 	    height2vs#fun{$ V O}
% 			 if V==glue then
% 			    if {O Get(glue $)}.expv then "100%" else "auto" end
% 			 else V end
% 		      end
% 	    vs2height#fun{$ V O}
% 			 if {O Get(height $)}==glue then glue else
% 			    {Str2intdash V}
% 			 end
% 		      end
% 	    vs2intdash#fun{$ V _} {Str2intdash V} end
% 	    intdash2vs#fun{$ V _} V end
% 	    vs2record#fun{$ V _} {Str2record V} end
% 	    record2vs#fun{$ V _} {Record2str V} end
% 	    html2record#fun{$ V _} {Record.map {Str2record V} Html2vs} end
% 	    record2html#fun{$ V _} {Record2str {Record.map V Vs2html}} end
% 	    vs2recordintdash#fun{$ V _} {Record.map {Str2record V} Str2intdash} end
% 	    recordintdash2vs#fun{$ V _} {Record2str V} end
% 	    vs2atomintdash#fun{$ V _} {Str2atomintdash V} end
% 	    atomintdash2vs#fun{$ V _} V end
% 	    bool2vs#fun{$ V _}
% 		       if V then "true" else "false" end
% 		    end
% 	    vs2bool#fun{$ V _}
% 		       V=="true"
% 		    end
% 	    int2vs#fun{$ V _} V end
% 	    vs2int#fun{$ V _} {String.toInt V} end
% 	    float2vs#fun{$ V _} V end
% 	    vs2float#fun{$ V _} {String.toFloat V} end
% 	    floatdash2vs#fun{$ V _} V end
% 	    vs2floatdash#fun{$ V _} {Str2floatdash V} end
% 	    floatdashpct2vs#fun{$ V _} V end
% 	    vs2floatdashpct#fun{$ V _} R={Str2floatdash V} in case R of V#'%' then {Float.toInt V}#'%' else R end
% 	    atomrecord2vs#fun{$ V _} if {Atom.is V} then V else {Record2str V} end end
% 	    vs2atomrecord#fun{$ V _} {Record2str V} end
% 	    atomrecordint2vs#fun{$ V _} if {Atom.is V} then V else {Record2str V} end end
% 	    vs2atomrecordint#fun{$ V _} {Record.map {Record2str V} String.toInt} end
% 	    atomrecordintdash2vs#fun{$ V _} if {Atom.is V} then V else {Record2str V} end end
% 	    vs2atomrecordintdash#fun{$ V _} {Record.map {Record2str V} Str2intdash} end
% 	   ]
%     proc{$ K#V}
%        {RegisterMarshaller K V}
%     end}

end
