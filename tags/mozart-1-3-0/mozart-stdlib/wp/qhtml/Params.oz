functor
require Open
prepare
   NoInit={NewName}

   InitDb={NewDictionary}
   {ForAll ["see individual properties"#NoInit
	    "the value of the 'color' property"#NoInit
	    "0% 0%"#NoInit
	    "depends on user agent"#NoInit
	    "empty string"#""
	    "XX"#NoInit
	    "UA dependent"#NoInit
	    "depends on user agent and writing direction"#NoInit
	   ]
    proc{$ K#E}
       InitDb.{String.toAtom K}:=E
    end}

   AllWidgets=[button checkbox file frame frameset hr html hyperlink img label
	       listbox lr lrframe menubutton menucheck menuhr menuitem menuradio password
	       placeholder radio submenu table tc td tdframe text textarea toplevel tr]
   ApplyDb={NewDictionary}
   {ForAll [""#AllWidgets
	    "block-level and replaced elements"#[label html placeholder]
	    "'table' and 'inline-table' elements"#[table]
	    "positioned elements"#[label html placeholder]
	    "'table-caption' elements"#nil
	    "block-level elements"#[label html placeholder]
	    ":before and :after pseudo-elements"#nil
	    "all elements, but see prose"#AllWidgets
	    "'table-cell' elements"#[tc]
	    "all but positioned elements and generated content"#AllWidgets
	    "all elements but non-replaced inline elements, table columns, and       column groups"#AllWidgets
	    "elements with 'display: list-item'"#[listbox]
	    "elements with 'display: marker'"#nil
	    "page context"#nil
	    "all elements except non-replaced inline elements and table elements"#{List.subtract AllWidgets table}
	    "all elements, but not to generated content"#AllWidgets
	    "the page context"#nil
	    "elements that have table header information"#[table]
	    "all elements but non-replaced inline elements, table rows, and row       groups"#AllWidgets
	   ]
    proc{$ K#E}
       ApplyDb.{String.toAtom K}:=E
    end}
   
   %% load CSS2.html file
   Data
   local
      File={New Open.file init(url:"CSS2.html")}
   in
      Data={File read(list:$ size:all)}
      {File close}
   end

   %% parse data

   Ignore=nil % [azimuth background border font margin padding]

   fun{SkipTag Data Str}
      fun{SkipClosure Data}
	 case Data
	 of &>|Xs then Xs
	 [] _|Xs then {SkipClosure Xs}
	 else nil
	 end
      end
      Len={Length Str}
      fun{Loop Data}
	 case Data
	 of &<|Xs then
	    if {List.take Xs Len}==Str then
	       {SkipClosure Xs}
	    else
	       {Loop Xs}
	    end
	 [] _|Xs then
	    {Loop Xs}
	 else nil end
      end
   in
      {Loop Data}
   end

   fun{GetUntil Data Str}
      Len={Length Str}
      fun{Loop Data}
	 if {List.take Data Len}==Str then nil
	 elsecase Data
	 of X|Xs then X|{Loop Xs}
	 else nil end
      end
   in
      {Loop Data}
   end

   fun{GetNoTagUntil Data Str}
      Len={Length Str}
      fun{Skip Data}
	 case Data
	 of &>|Xs then {Loop Xs}
	 [] _|Xs then {Skip Xs}
	 else nil
	 end
      end
      fun{Loop Data}
	 if {List.take Data Len}==Str then nil
	 elsecase Data
	 of &<|Xs then {Skip Xs}
	 [] X|Xs then X|{Loop Xs}
	 else nil
	 end
      end
   in
      {Loop Data}
   end

   fun{GetSpecTagUntil Data Str}
      Len={Length Str}
      fun{Skip Data}
	 case Data
	 of &>|Xs then {Loop Xs}
	 [] _|Xs then {Skip Xs}
	 else nil
	 end
      end
      fun{Loop Data}
	 if {List.take Data Len}==Str then nil
	 elsecase Data
	 of &<|&A|Xs then 'A'|{Skip Xs}
	 [] &<|&a|Xs then 'A'|{Skip Xs}
	 [] &<|Xs then {Skip Xs}
	 [] X|Xs then X|{Loop Xs}
	 else nil
	 end
      end
   in
      {Loop Data}
   end

   fun{ParseValue Str}
      fun{MakeList Str}
	 case Str
	 of 'A'|&&|&l|&t|&;|Xs then
	    fun{Loop Str R}
	       case Str
	       of &&|&g|&t|&;|Xs then R=Xs nil
	       [] X|Xs then X|{Loop Xs R}
	       else R=nil nil
	       end
	    end
	    R
	    V={String.toAtom {Loop Xs R}}
	 in
	    type(V)|{MakeList R}
	 [] &&|&n|&b|&s|&p|&;|Xs then
	    {MakeList Xs}
	 [] &||&||Xs then
	    '||'|{MakeList Xs}
	 [] &||Xs then
	    '|'|{MakeList Xs}
	 [] &{|Xs then
	    From R1
	    {List.takeDropWhile Xs fun{$ C} C\=&, end From R1}
	    To R2
	    {List.takeDropWhile {List.drop R1 1} fun{$ C} C\=&} end To R2}
	 in
	    repeat({String.toInt From} {String.toInt To})|{MakeList {List.drop R2 1}}
	 [] &[|Xs then
	    '['|{MakeList Xs}
	 [] &]|Xs then
	    ']'|{MakeList Xs}
	 [] &\r|Xs then
	    {MakeList Xs}
	 [] &\n|Xs then
	    {MakeList Xs}
	 [] & |Xs then
	    {MakeList Xs}
	 [] &?|Xs then
	    '?'|{MakeList Xs}
	 [] 'A'|Xs then
	    T R
	 in
	    {List.takeDropWhile Xs fun{$ C} {Not {List.member C [& &] &|]}} end T R}
	    if T=="inherit" then inherit else
	       ref({String.toAtom
		    case T of &'|_ then
		       {List.drop {List.take T {Length T}-1} 1}
		    else T end})
	    end|{MakeList R}	    
	 [] _|_ then
	    T R
	 in
	    {List.takeDropWhile Str fun{$ C} {Not {List.member C [& &] &|]}} end T R}
	    {StringToAtom T}|{MakeList R}
	 else nil end
      end
%       fun{BuildStruct L Mode R}
% 	 fun{Next This Remain}
% 	    case Remain
% 	    of repeat(F T)|Xs then
% 	       repeat(This F T)|{BuildStruct Xs Mode R}
% 	    [] '*'|Xs then
% 	       repeat(This 0 inf)|{BuildStruct Xs Mode R}
% 	    else
% 	       This|{BuildStruct Remain Mode R}
% 	    end
% 	 end
%       in
% %	 {Show {List.take L 1}#Mode}
% 	 case L
% 	 of '|'|Xs then
% 	    Mode=switch
% 	    {BuildStruct Xs Mode R}
% 	 [] '||'|Xs then
% 	    Mode=aggregate
% 	    {BuildStruct Xs Mode R}
% 	 [] '['|Xs then
% 	    T
% 	    Remain
% 	    TL={BuildStruct Xs T Remain}
% 	    if {IsFree T} then T=abs end
% 	    This={List.toTuple T TL}
% 	 in
% 	    {Next This Remain}
% 	 [] ']'|Xs then
% 	    R=Xs nil
% 	 [] X|'?'|Xs then
% 	    repeat(X 0 1)|{BuildStruct Xs Mode R}
% 	 [] X|Xs then
% 	    {Next X Xs}
% 	 else
% 	    R=nil nil
% 	 end
%       end
%       T TL
%       TL={BuildStruct {MakeList Str} T _}
%    in
%       if {IsFree T} then T=abs end
%       {List.toTuple T TL}
      fun{BuildStruct L R}
	 fun{Next L R}
	    case L
	    of This|repeat(F T)|Xs then
	       R=Xs repeat(This F T)
	    [] This|'*'|Xs then
	       R=Xs repeat(This 0 inf)
	    [] This|'+'|Xs then
	       R=Xs repeat(This 1 inf)
	    [] This|'?'|Xs then
	       R=Xs repeat(This 0 1)
	    [] This|Xs then
	       R=Xs This
	    end
	 end
	 fun{GetElems L R}
	    case L
	    of '['|Xs then
	       R1 R2
	    in
	       {Next {BuildStruct Xs R1}|R1 R2}|{GetElems R2 R}
	    [] ']'|Xs then
	       R=Xs nil
	    [] _|_ then
	       R1
	    in
	       {Next L R1}|{GetElems R1 R}
	    else R=nil nil
	    end
	 end
	 Li={GetElems L R}
	 fun{GetAbs L R}
	    case L
	    of '|'|_ then R=L nil
	    [] '||'|_ then R=L nil
	    [] X|Xs then X|{GetAbs Xs R}
	    else R=nil nil end
	 end
	 Mode
	 fun{Loop L}
	    if L\=nil then
	       R1
	       B={GetAbs L R1}
	       if {IsFree Mode} then
		  case R1
		  of '|'|_ then Mode=switch
		  [] '||'|_ then Mode=aggregate
		  else Mode=succ
		  end
	       end
	    in
	       if {Length B}==1 then {List.last B}
	       else {List.toTuple succ B}
	       end|{Loop if R1==nil then nil else {List.drop R1 1} end}
	    else nil end
	 end
	 T={Loop Li}
	 Result=if {Length T}==1 then {List.last T}
		else
		   {List.toTuple Mode T}
		end
      in
	 Result
      end
   in
      {BuildStruct {MakeList Str} _}
   end
   
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
   

   fun{Purge Str}
      fun{Loop Str}
	 case Str
	 of &&|&n|&b|&s|&p|&;|Xs then {Loop Xs}
	 [] &\r|Xs then {Loop Xs}
	 [] &\n|Xs then {Loop Xs}
	 [] X|Xs then X|{Loop Xs}
	 else nil
	 end
      end
   in
      {Reverse
       {List.dropWhile
	{Reverse {List.dropWhile {Loop Str} fun{$ C} C==&  end}}
	fun{$ C} C==&  end}}
   end
   
   fun{ParseInit V3}
      P={Purge V3}
      A={String.toAtom P}
   in
      if {Dictionary.member InitDb A} then
	 InitDb.A
      else
	 {Str2distance P}
      end
   end

   fun{ParseApply V4}
      {Dictionary.condGet ApplyDb {String.toAtom {Purge V4}} nil}
   end
   
   Quit
   CSS={NewDictionary}
   
   proc{Loop Data}
      % row
      Row={SkipTag Data "TR"}
   in
      if Row\=nil then
	 % first cell
	 C1={SkipTag Row "TD"}
         % content of span in cell
	 V1={GetNoTagUntil C1 "<TD>"}
         % second cell
	 C2={SkipTag C1 "TD"}
         % content of cell
	 V2={GetSpecTagUntil C2 "<TD>"} % possible values
         % third cell
	 C3={SkipTag C2 "TD"}
         % content of cell
	 V3={GetUntil C3 "<TD>"} % default value
         % fourth cell
	 C4={SkipTag C3 "TD"}
	 V4={GetUntil C4 "<TD>"} % applies to...
      in
	 % process all informations
	 local
	    fun{GetName Str R}
	       L1={List.dropWhile Str fun{$ C} C\=&' end}
	    in
	       if L1==nil then R=nil nil
	       else
		  L2={List.drop L1 1}
		  L3 L4
		  {List.takeDropWhile L2 fun{$ C} C\=&' end L3 L4}
	       in
		  R={List.drop L4 1}
		  {String.toAtom L3}
	       end
	    end
	    Init
	    Apply
	    Values
	    proc{Loop Str}
	       R
	       Name={GetName Str R}
	    in
	       if Name\=nil then
		  if {Not {List.member Name Ignore}} then
		     % process this Name
		     if {IsFree Values} then
			Values={ParseValue V2}
			Init={ParseInit V3}
			Apply={ParseApply V4}
%			{Show Name#Apply}
		     end
		     % store data
		     CSS.Name:=r(values:Values
				 init:Init
				 apply:Apply)
		  end
		  {Loop R}
	       end
	    end
	 in
	    {Loop V1}
	 end
	 if {IsFree Quit} then {Loop C4} end
      end
   end
   
   TBody={SkipTag Data "TBODY"}
   {Loop TBody}

   %% references must still be changed by their actual value
   local
      fun{RemoveRef T}
	 {Record.map T
	  fun{$ E}
	     case E
	     of ref(X) then
		{Dictionary.get CSS X}.values
	     else
		if {Record.is E} then
		   {RemoveRef E}
		else
		   E
		end
	     end
	  end}
      end
   in
      {ForAll {Dictionary.keys CSS}
       proc{$ K}
	  CSS.K:={RemoveRef CSS.K}
       end}
   end
   

   %% at this point, the CSS dictionary contain all CSS parameter informations, index by parameter name
   %% now we must build a list index by widgets

   fun{Capitalize S}
      case S
      of &-|X|Xs then
	 if (X>=&a) andthen (X=<&z) then
	    X+&A-&a|{Capitalize Xs}
	 else
	    X|{Capitalize Xs}
	 end
      [] X|Xs then X|{Capitalize Xs}
      else nil
      end
   end

   DB={NewDictionary}
   {ForAll {Dictionary.entries CSS}
    proc{$ N#D}
       Data1=r(attribute:N
	       get:internal(true)
	       init:style(true)
	       set:style(true)
	       type:css(D.values)
	      )
       Data=Data1 % if D.init\=NoInit then {Record.adjoinAt Data1 default D.init} else Data1 end
       Name={String.toAtom {Capitalize {Atom.toString N}}}
    in
       {ForAll D.apply
	proc{$ W}
	   {Dictionary.put DB W
	    Name#Data|{Dictionary.condGet DB W nil}}
	end}
    end}

   % events definition is build by hand for now... info comes from html40\index\attributes.html
   % list of possible events : onblur onchange onclick ondblclick onfocus onkeydown onkeypress onkeyup onload onmousedown onmousemove onmouseout onmouseover onmouseup onreset onselect  onunload
   
   DEVENTS=[anchor#[onblur onclick ondblclick onfocus onkeydown onkeypress onkeyup onmousedown onmousemove onmouseout onmouseover onmouseup]
	    button#[onblur onchange onclick ondblclick onfocus onkeydown onkeypress onkeyup onmousedown onmousemove onmouseout onmouseover onmouseup]
	    checkbox#[onblur onchange onclick ondblclick onfocus onkeydown onkeypress onkeyup onmousedown onmousemove onmouseout onmouseover onmouseup]
	    file#[onblur onchange onclick ondblclick onfocus onkeydown onkeypress onkeyup onmousedown onmousemove onmouseout onmouseover onmouseup onselect]
	    frame#nil
	    frameset#[onload onunload]
	    hr#[onclick ondblclick onkeydown onkeypress onkeyup onmousedown onmousemove onmouseout onmouseover onmouseup]
	    html#[onclick ondblclick onkeydown onkeypress onkeyup onmousedown onmousemove onmouseout onmouseover onmouseup]
	    hyperlink#[onblur onclick ondblclick onfocus onkeydown onkeypress onkeyup onmousedown onmousemove onmouseout onmouseover onmouseup]
	    img#[onclick ondblclick onkeydown onkeypress onkeyup onmousedown onmousemove onmouseout onmouseover onmouseup]
	    label#[onclick ondblclick onkeydown onkeypress onkeyup onmousedown onmousemove onmouseout onmouseover onmouseup]
	    listbox#[onblur onchange onclick ondblclick onfocus onkeydown onkeypress onkeyup onmousedown onmousemove onmouseout onmouseover onmouseup]
	    lr#[onchange onclick ondblclick onkeydown onkeypress onkeyup onmousedown onmousemove onmouseout onmouseover onmouseup]
	    lrframe#[onload onunload]
	    menubutton#nil
	    menucheck#nil
	    menuhr#nil
	    menuitem#nil
	    menuradio#nil
	    password#[onblur onchange onclick ondblclick onfocus onkeydown onkeypress onkeyup onmousedown onmousemove onmouseout onmouseover onmouseup onselect]
	    placeholder#[onclick ondblclick onkeydown onkeypress onkeyup onmousedown onmousemove onmouseout onmouseover onmouseup]
	    radio#[onblur onchange onclick ondblclick onfocus onkeydown onkeypress onkeyup onmousedown onmousemove onmouseout onmouseover onmouseup]
	    submenu#nil
	    table#[onclick ondblclick onkeydown onkeypress onkeyup onmousedown onmousemove onmouseout onmouseover onmouseup]
	    tc#[onclick ondblclick onkeydown onkeypress onkeyup onmousedown onmousemove onmouseout onmouseover onmouseup]
	    td#[onclick ondblclick onkeydown onkeypress onkeyup onmousedown onmousemove onmouseout onmouseover onmouseup]
	    tdframe#[onload onunload]
	    text#[onchange onclick ondblclick onfocus onkeydown onkeypress onkeyup onmousedown onmousemove onmouseout onmouseover onmouseup onselect]
	    textarea#[onblur onchange onclick ondblclick onfocus onkeydown onkeypress onkeyup onmousedown onmousemove onmouseout onmouseover onmouseup onselect]
	    toplevel#[onclick ondblclick onkeydown onkeypress onkeyup onload onmousedown onmousemove onmouseout onmouseover onmouseup onunload]
	    tr#[onclick ondblclick onkeydown onkeypress onkeyup onmousedown onmousemove onmouseout onmouseover onmouseup]]
 
   DBL={Dictionary.entries DB}
export
   data   : DBL
   events : DEVENTS
end 
 
