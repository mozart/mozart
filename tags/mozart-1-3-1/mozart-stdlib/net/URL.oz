functor
export
   'class' : URLClass
   Is Make
prepare
   VS2S = VirtualString.toString
   VS2A = VirtualString.toAtom
   ToTuple = List.toTuple
   ToRecord = List.toRecord
   Tokens = String.tokens
   Token = String.token
   CharIsAlNum = Char.isAlNum
   ToLower = Char.toLower

   IS_URL = {NewName}
   fun {Is X} {HasFeature X IS_URL} end
   
   %% a url may contain occurrences of %XY escape sequences
   %% Here is how to decode them

   local
      D = x(&0:0 &1:1 &2:2 &3:3 &4:4 &5:5 &6:6 &7:7 &8:8 &9:9
	    &a:10 &b:11 &c:12 &d:13 &e:14 &f:15
	    &A:10 &B:11 &C:12 &D:13 &E:14 &F:15)
   in
      fun {Decode L}
	 case L of nil then nil
	 [] H|T then
	    if H==&% then
	       case T of X1|X2|T then
		  (D.X1*16)+D.X2 | {Decode T}
	       else H | {Decode T} end
	    else H | {Decode T} end
	 end
      end
   end

   proc {SplitAt S Pred Prefix Sep Suffix}
      case S
      of nil then Prefix=Suffix=nil Sep=unit
      [] H|T then
	 if {Pred H}
	 then Prefix=nil Sep=H Suffix=T
	 else More in
	    Prefix=H|More
	    {SplitAt T Pred More Sep Suffix}
	 end
      end
   end

   fun {SepStart C}
      C==&:  orelse
      C==&/  orelse
      C==&\\ orelse
      C==&?  orelse
      C==&#  orelse
      C==&{
   end

   fun {SepAuthority C}
      C==&/  orelse
      C==&\\ orelse
      C==&?  orelse
      C==&#  orelse
      C==&{
   end

   SepPath = SepAuthority

   fun {SepQuery C}
      C==&# orelse C==&{
   end

   fun {SepFragment C}
      C==&{
   end

   fun {Parse S}
      {ParseFromString {VS2S S}}
   end

   URL_EMPTY = url(scheme    : unit
		   authority : unit
		   device    : unit
		   absolute  : false
		   path      : nil
		   query     : unit
		   fragment  : unit
		   info      : unit)
   
   fun {ParseFromString S}
      {ParseStart S URL_EMPTY}
   end

   fun {ParseStart S U}
      Prefix Sep Suffix
   in
      {SplitAt S SepStart Prefix Sep Suffix}
      case Sep
      of unit then
	 {AdjoinAt U path [{Decode Prefix}]}
      [] &: then
	 case Prefix of [C] then
	    U2 = {AdjoinAt U device C}
	 in
	    case Suffix of H|T then
	       if H==&/ orelse H==&\\ then
		  {ParsePath T nil {AdjoinAt U2 absolute true}}
	       else
		  {ParseDevice Suffix U2}
	       end
	    [] nil then U2 end
	 else
	    U2 = {AdjoinAt U scheme Prefix}
	 in
	    case Suffix
	    of &/|&/|T then {ParseAuthority T U2}
	    [] &/   |T then {ParsePath T nil {AdjoinAt U2 absolute true}}
	    else {ParseDevice Suffix U2} end
	 end
      [] &/ then
	 case Prefix of nil then
	    case Suffix
	    of &/|T then {ParseAuthority T U}
	    else {ParsePath Suffix nil {AdjoinAt U absolute true}} end
	 else {ParsePath Suffix [{Decode Prefix}] U} end
      [] &\\ then
	 case Prefix of nil then
	    {ParsePath Suffix nil {AdjoinAt U absolute true}}
	 else {ParsePath Suffix [{Decode Prefix}] U} end
      [] &? then
	 {ParseQuery Suffix
	  if Prefix==nil then U else
	     {AdjoinAt U path [{Decode Prefix}]}
	  end}
      [] &# then
	 {ParseFragment Suffix
	  if Prefix==nil then U else
	     {AdjoinAt U path [{Decode Prefix}]}
	  end}
      [] &{ then
	 {ParseInfo Suffix
	  if Prefix==nil then U else
	     {AdjoinAt U path [{Decode Prefix}]}
	  end}
      end
   end

   fun {ParseAuthority S U}
      Prefix Sep Suffix
      {SplitAt S SepAuthority Prefix Sep Suffix}
      U2 = {AdjoinAt U authority Prefix}
   in
      case Sep
      of unit then U2
      [] &/   then {ParsePath Suffix nil {AdjoinAt U2 absolute true}}
      [] &\\  then {ParsePath Suffix nil {AdjoinAt U2 absolute true}}
      [] &?   then {ParseQuery Suffix U2}
      [] &#   then {ParseFragment Suffix U2}
      [] &{   then {ParseInfo Suffix U2}
      end
   end

   fun {ParseDevice S U}
      case S of C|&:|T then
	 {ParsePath T nil {AdjoinAt U device C}}
      else {ParsePath S nil U} end
   end

   fun {ParsePath S L U}
      Prefix Sep Suffix
      {SplitAt S SepPath Prefix Sep Suffix}
      COM={Decode Prefix}
      L2 = case COM
	   of "." then
	      if L==nil then [COM] else L end
	   [] ".." then
	      case L
	      of nil then [COM]
	      [] ["."] then [COM]
	      [] [_] then COM|L
	      else L.2 end
	   else COM|L end
   in
      case Sep
      of unit then {AdjoinAt U path {Reverse L2}}
      [] &/   then {ParsePath Suffix L2 U}
      [] &\\  then {ParsePath Suffix L2 U}
      [] &?   then {ParseQuery    Suffix {AdjoinAt U path {Reverse L2}}}
      [] &#   then {ParseFragment Suffix {AdjoinAt U path {Reverse L2}}}
      [] &{   then {ParseInfo     Suffix {AdjoinAt U path {Reverse L2}}}
      end
   end

   fun {ParseQuery S U}
      Prefix Sep Suffix
      {SplitAt S SepQuery Prefix Sep Suffix}
      U2 = {AdjoinAt U query Prefix}
   in
      case Sep
      of unit then U2
      [] &# then {ParseFragment Suffix U2}
      [] &{ then {ParseInfo Suffix U2}
      end
   end

   fun {ParseFragment S U}
      Prefix Sep Suffix
      {SplitAt S SepFragment Prefix Sep Suffix}
      U2 = {AdjoinAt U fragment Prefix}
   in
      if Sep==unit then U2 else
	 {ParseInfo Suffix U2}
      end
   end

   fun {ParseInfo S U}
      case {Reverse S} of  &}|L then
	 {AdjoinAt U info
	  {ToRecord info
	   {Map {Tokens {Reverse L} &,} ParseInfoElem}}}
      end
   end

   fun {ParseInfoElem S}
      Prop Value
   in
      {Token S &= Prop Value}
      {VS2A Prop}#{VS2A Value}
   end

   %% URLToVS converts a url to a virtual string.  The 2nd argument is
   %% a record whose features parametrize the conversion process.
   %% `full:true'
   %%	indicates that full information is desired, including #FRAGMENT
   %%	and {INFO} sections.  Normally, the #FRAGMENT indicator is
   %%	intended only for client-side usage.  Similarly, but even more
   %%	so, the {INFO} section is a Mozart extension, where, for
   %%	example, it is indicated whether the url denotes a native
   %%	functor or not.  Thus, neither should be included when
   %%	constructing a url for retrieval.
   %% `cache:true'
   %%	indicates that a syntax appropriate for cache lookup is desired.
   %%	the `:' after the scheme and the '//' before the authority are
   %%	in both cases replaced by a single `/'.
   %% `raw:true'
   %%	indicates that no encoding should take place, i.e. special url
   %%	characters are not escaped.
   %% `normalized:true'
   %%   indicates that scheme, device and authority should be lowercased

   fun {URLToVS U How}
      Full      = {CondSelect How full  false}
      Cache     = {CondSelect How cache false}
      Raw       = {CondSelect How raw   false}
      Normalized= {CondSelect How normalized false}
      CompAdd   =
      if Cache then
	 if Raw then CompAdd_c_raw else CompAdd_c_enc end
      else
	 if Raw then CompAdd_raw else CompAdd_enc end
      end
      Scheme0   = {CondSelect U scheme unit}
      Scheme    =
      if Normalized andthen Scheme0\=unit then
	 {Map Scheme0 ToLower}
      else Scheme0 end
      Device0   = {CondSelect U device unit}
      Device    =
      if Normalized andthen Device0\=unit then
	 {ToLower Device0}
      else Device0 end
      Authority0= {CondSelect U authority unit}
      Authority =
      if Normalized andthen Authority0\=unit then
	 {Map Authority0 ToLower}
      else Authority0 end
      Absolute	= {CondSelect U absolute false}
      Path      = {CondSelect U path      nil}
      Query     = {CondSelect U query unit}
      Fragment  = if Full then {CondSelect U fragment unit} else unit end
      Info      = if Full then {CondSelect U info unit} else unit end
      %%
      V1        = if Full andthen Info\=unit then
		     {InfoPropsAdd {Record.toListInd Info} ['}'] true}
		  else nil end
      V2        = if Full andthen Fragment\=unit then
		     "#"|Fragment|V1
		  else V1 end
      V3        = if Query==unit then V2 else '?'|Query|V2 end
      V4        = {Append
		   %% yes we want the nil here otherwise
		   %%Slashit adds an erroneous slash
		   {FoldR Path CompAdd nil} V3}
      V5        = if Absolute then {Slashit V4} else V4 end
      V6        = if Device==unit then V5
		  else
		     [Device]
		     |if Cache then {Slashit V5} else ':'|V5 end
		  end
      V7        = if Authority==unit then V6
		  elseif Cache then
		     if Authority==nil then V6
		     else Authority|{Slashit V6} end
		  else
		     '/'|'/'|Authority|{Slashit V6}
		  end
      V8        = if Scheme==unit then V7
		  elseif Cache then Scheme|{Slashit V7}
		  else Scheme|':'|V7 end
   in
      {ToTuple '#' V8}
   end

   fun {Slashit L}
      case L of nil then nil
      [] '/'|_ then L
      else '/'|L end
   end
   
   fun {InfoPropsAdd L Rest First}
      case L of nil then '{'|Rest
      [] (K#V)|T then
	 {InfoPropsAdd T
	  K|'='|V|if First then Rest else ','|Rest end
	  false}
      end
   end

   fun {CompAdd_raw H L}
      H|{Slashit L}
   end
   
   fun {CompAdd_enc H L}
      {Encode H}|{Slashit L}
   end

   fun {CompAdd_c_raw H L}
      if H==nil then L else H|{Slashit L} end
   end

   fun {CompAdd_c_enc H L}
      if H==nil then L else {Encode H}|{Slashit L} end
   end

   fun {URLIsAbsolute U}
      {CondSelect U scheme unit}\=unit orelse
      {CondSelect U device unit}\=unit orelse
      {CondSelect U absolute false} orelse
      case {CondSelect U path nil}
      of (&~|_)|_ then true
      [] "."   |_ then true
      [] ".."  |_ then true
      else false end
   end
   %% resolving a relative url with respect to a base url

   fun {URLResolve Base Rel}
      if {CondSelect Rel scheme unit}\=unit then Rel
      else
	 Scheme = {CondSelect Base scheme unit}
	 Rel2 = if Scheme==unit then Rel
		else {AdjoinAt Rel scheme Scheme} end
      in
	 if {CondSelect Rel2 authority unit}\=unit then Rel2
	 else
	    Authority = {CondSelect Base authority unit}
	    Rel3 = if Authority==unit then Rel2
		   else {AdjoinAt Rel2 authority Authority} end
	 in
	    if {CondSelect Rel3 device unit}\=unit then Rel3
	    else
	       Device = {CondSelect Base device unit}
	       Rel4 = if Device==unit then Rel3
		      else {AdjoinAt Rel3 device Device} end
	       BPath = {CondSelect Base path unit}
	       RAbs  = {CondSelect Rel4 absolute false}
	    in
	       if RAbs orelse BPath==unit then Rel4
	       else
		  Rel5 = if {CondSelect Base absolute false}
			 then {AdjoinAt Rel4 absolute true}
			 else Rel4 end
		  RPath = {CondSelect Rel path nil}
	       in
		  {AdjoinAt Rel5 path
		   {ResolvePaths BPath
		    if RPath==unit orelse RPath==nil
		    then [nil] else RPath end}}
	       end
	    end
	 end
      end
   end

   fun {ResolvePaths Base Rel}
      {ResolvePathsX {SkipEmpties {Reverse Base}} Rel}
   end

   fun {SkipEmpties L}
      case L of nil|L then {SkipEmpties L} else L end
   end

   fun {ResolvePathsX Base Rel}
      case Rel
      of nil then {Reverse Base}
      [] H|T then
	 case H
	 of "." then
	    {ResolvePathsX
	     if Base==nil then [H] else Base end
	     T}
	 [] ".." then
	    {ResolvePathsX
	     case Base
	     of nil then [H]
	     [] ["."] then [H]
	     [] [_] then H|Base
	     else Base.2 end
	     T}
	 else
	    {ResolvePathsX H|Base T}
	 end
      end
   end

   %% for producing really normalized url vstrings, we need to encode
   %% its components.  Encode performs the `escape encoding' of a
   %% string (e.g. a single path component).

   local
      D = x(0:&0 1:&1 2:&2 3:&3 4:&4 5:&5 6:&6 7:&7 8:&8 9:&9
	    10:&a 11:&b 12:&c 13:&d 14:&e 15:&f)
   in
      fun {Encode S}
	 case S of nil then nil
	 [] H|T then
	    %% check that it is an `ascii' alphanum
	    if H<128 andthen {CharIsAlNum H} orelse
	       H==&; orelse
	       H==&- orelse
	       H==&_ orelse
	       H==&. orelse
	       H==&! orelse
	       H==&~ orelse
	       H==&* orelse
	       H==&' orelse
	       H==&( orelse
	       H==&) orelse
	       H==&: orelse
	       H==&@ orelse
	       H==&& orelse
	       H==&= orelse
	       H==&+ orelse
	       H==&$ orelse
	       H==&,
	    then H|{Encode T} else
	       X1 = H div 16
	       X2 = H mod 16
	    in &%|D.X1|D.X2|{Encode T} end
	 end
      end
   end

import
   Path at 'x-oz://system/os/Path.ozf'
define
   fun {Make X}
      if {Is X} then X else
	 {New URLClass init(X)}
      end
   end

   class URLClass
      feat !IS_URL:unit
      attr info
      meth INIT(R)
	 info <- R
      end
      meth GetInfo($)
	 @info
      end
      meth init(S)
	 URLClass,INIT({Parse S})
      end
      meth toVS($
		full       : FULL  <=false
		cache      : CACHE <=false
		raw        : RAW   <=false
		normalized : NORM  <=false)
	 {URLToVS @info
	  unit(full:FULL cache:CACHE raw:RAW normalized:NORM)}
      end
      meth toString($
		full       : FULL  <=false
		cache      : CACHE <=false
		raw        : RAW   <=false
		normalized : NORM  <=false)
	 {VS2S URLClass,toVS($ full:FULL cache:CACHE raw:RAW normalized:NORM)}
      end
      meth toAtom($
		full       : FULL  <=false
		cache      : CACHE <=false
		raw        : RAW   <=false
		normalized : NORM  <=false)
	 {VS2A URLClass,toVS($ full:FULL cache:CACHE raw:RAW normalized:NORM)}
      end
      meth isAbsolute($)
	 {URLIsAbsolute @info}
      end
      meth isRelative($)
	 {Not {URLIsAbsolute @info}}
      end
      meth resolve(U $)
	 {New URLClass INIT({URLResolve @info {{Make U} GetInfo($)}})}
      end
      meth isRoot($)
	 @info.path == nil
      end
      meth dirname($)
	 case {Reverse @info.path}
	 of nil then self
	 [] _|T then
	    {New URLClass INIT({AdjoinAt @info path {Reverse T}})}
	 end
      end
      meth basename($)
	 {New URLClass
	  INIT(
	     case {Reverse @info.path}
	     of nil then URL_EMPTY
	     [] H|_ then {AdjoinAt URL_EMPTY path [H]}
	     end)}
      end
      meth getScheme($)    @info.scheme end
      meth getAuthority($) @info.authority end
      meth getDevice($)    @info.device end
      meth getAbsolute($)  @info.absolute end
      meth getPath($)      @info.path end
      meth getQuery($)     @info.query end
      meth getFragment($)  @info.fragment end
      meth getInfo($)      @info.info end

      meth normalized($)
	 Scheme = @info.scheme
	 Device = @info.device
	 Authority = @info.authority

	 SCHEME =
	 if Scheme==unit then unit else
	    {Map Scheme ToLower}
	 end
	 DEVICE =
	 if Device==unit then unit else
	    {ToLower Device}
	 end
	 AUTHORITY =
	 if Authority==unit then unit else
	    {Map Authority ToLower}
	 end
      in
	 if Scheme==SCHEME andthen
	    Device==DEVICE andthen
	    Authority==AUTHORITY
	 then self
	 else {New URLClass INIT({Adjoin @info
				  url(scheme:SCHEME
				      device:DEVICE
				      authority:AUTHORITY)})}
	 end
      end

      meth toPath($ windows:WIN<=false)
	 S1 = {FoldL @info.path
	       fun {$ Accu Comp}
		  if Accu==nil then Comp else
		     Accu#'/'#Comp
		  end
	       end nil}
	 S2 = if URLClass,getAbsolute($)
	      then '/'#S1 else S1 end
	 WIN2
	 S3 = case URLClass,getDevice($)
	      of unit then WIN2=WIN S2
	      [] C then WIN2=true [C]#S2 end
      in
	 {New Path.'class' init(S3 windows:WIN2)}
      end
   end
end
