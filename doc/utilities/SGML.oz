%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 1998
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation of Oz 3:
%%%   $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%   $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor prop once
import
   Property(get)
   Open(pipe text file)
   OS(tmpnam unlink)
   ErrorRegistry(put)
   Error(dispatch format formatGeneric)
export
   Parse
   namePI: PI
   GetSubtree
   IsOfClass
define
   PI = {NewName}

   class TextPipe from Open.pipe Open.text
      prop final
   end

   class TextFile from Open.file Open.text
      prop final
   end

   fun {IsNoSpace C}
      {Not {Char.isSpace C}}
   end

   fun {MakeName S}
      {String.toAtom {Map S Char.toLower}}
   end

   local
      fun {MakeTokens S}
	 case S of & |Cr then Token Rest in
	    {List.takeDropWhile Cr IsNoSpace ?Token ?Rest}
	    {MakeName Token}|{MakeTokens Rest}
	 [] nil then nil
	 end
      end
   in
      fun {MakeAttr Name Type Rest}
	 case {String.toAtom Type} of 'IMPLIED' then unit
	 [] 'CDATA' then & |Value = Rest in
	    {MakeName Name}#Value
	 [] 'NOTATION' then & |Value = Rest in
	    %--** use s or p identifier
	    {MakeName Name}#{String.toAtom Value}
	 [] 'TOKEN' then
	    case {MakeName Name} of 'class' then 'class'#{MakeTokens Rest}
	    elseof T then T#{MakeTokens Rest}.1
	    end
	 elsecase Rest of & |Value then
	    {Exception.raiseError sgml(unsupportedAttributeType Type Value)}
	    unit
	 elseof nil then
	    {Exception.raiseError sgml(unsupportedAttributeType Type)}
	    unit
	 end
      end
   end

   local
      fun {MakeDataSub S}
	 case S of C|Cr then
	    case C of &\\ then
	       case Cr of &\\|Crr then &\\|{MakeDataSub Crr}
	       [] &n|Crr then &\n|{MakeDataSub Crr}
	       [] &||Crr then
		  %--** internal SDATA entities are bracketed by these
		  {Raise unsupportedEscape} unit
	       [] &#|Crr then N Crrr in
		  %--** internal character set
		  {List.takeDropWhile Crr Char.isDigit ?N &;|Crrr}
		  {String.toInt N}|{MakeDataSub Crrr}
	       [] &%|Crr then N Crrr in
		  %--** document character set
		  {List.takeDropWhile Crr Char.isDigit ?N &;|Crrr}
		  {String.toInt N}|{MakeDataSub Crrr}
	       else Octals Crrr in
		  % A record start character will be represented by \012.
		  % Most applications will need to ignore \012 and
		  % translate \n into newline.
		  {List.takeDrop Cr 3 ?Octals ?Crrr}
		  {String.toInt &0|Octals}|{MakeDataSub Crrr}
	       end
	    else
	       C|{MakeDataSub Cr}
	    end
	 [] nil then ""
	 end
      end
   in
      fun {MakeData S}
	 try
	    {MakeDataSub S}
	 catch unsupportedEscape then
	    {Exception.raiseError sgml(unsupportedEscape S)} unit
	 end
      end
   end

   local
      fun {Read File}
	 case {File getS($)} of false then nil
	 elseof S then S|{Read File}
	 end
      end
   in
      proc {ReadFile FileName ?VSs}
	 File = {New TextFile init(name: FileName)}
      in
	 VSs = {Read File}
	 {File close()}
      end
   end

   class SGMLParser
      attr File ErrFile Pipe Attributes Children Saved
      meth init(FileVS)
	 File <- FileVS
	 ErrFile <- {OS.tmpnam}
	 %--** add includes (nsgmls -i...)?
	 Pipe <- {New TextPipe init(cmd: 'nsgmls'
				    args: ['-c'#{Property.get 'oz.home'}#
					   '/share/doc/catalog' &-|&f|@ErrFile
					   FileVS])}
	 Attributes <- nil
	 Children <- nil
      end
      meth parse()
	 case {@Pipe getS($)} of false then VSs in
	    VSs = {ReadFile @ErrFile}
	    {OS.unlink @ErrFile}
	    {Exception.raiseError sgml(nonconforming @File VSs)}
	 [] C|Cr then
	    case C of &s then
		  %--** ignore for now
	       SGMLParser, parse()
	    [] &p then
		  %--** ignore for now
	       SGMLParser, parse()
	    [] &N then
		  %--** ignore for now
	       SGMLParser, parse()
	    [] &A then Name Rest1 Type Rest2 in
	       {List.takeDropWhile Cr IsNoSpace ?Name & |?Rest1}
	       {List.takeDropWhile Rest1 IsNoSpace ?Type ?Rest2}
	       case {MakeAttr Name Type Rest2} of unit then skip
	       elseof Attr then Attributes <- Attr|@Attributes
	       end
	       SGMLParser, parse()
	    [] &( then
	       Saved <- {MakeName Cr}#@Attributes#@Children|@Saved
	       Attributes <- nil
	       Children <- nil
	       SGMLParser, parse()
	    [] &- then
	       Children <- {MakeData Cr}|@Children
	       SGMLParser, parse()
	    [] &? then
	       Children <- PI({String.toAtom Cr})|@Children
	       SGMLParser, parse()
	    [] &) then
	       Ss = @Saved
	       L#As#Cs|Sr = Ss
	       {MakeName Cr} = L
	       NewChild = {List.toRecord L
			   {List.foldLInd {Reverse @Children}
			    fun {$ I In X}
			       I#X|In
			    end As}}
	    in
	       @Attributes = nil
	       Children <- NewChild|Cs
	       Saved <- Sr
	       SGMLParser, parse()
	    [] &C then
	       skip
	    else
	       {Exception.raiseError sgml(unsupportedCommand C Cr)}
	    end
	 end
      end
      meth get(?C)
	 [C] = @Children
      end
   end

   fun {Parse File} O in
      O = {New SGMLParser init(File)}
      {O parse()}
      {O get($)}
   end

   fun {GetSubtree M L ?Mr}
      if {IsTuple M} then
	 case {Record.toList M} of (X=L(...))|Xr then
	    Mr = {List.toTuple {Label M} Xr}
	    X
	 else
	    Mr = M
	    unit
	 end
      else
	 {GetSubtree
	  {List.toRecord {Label M}
	   {Filter {Record.toListInd M} fun {$ X#_} {IsInt X} end}} L ?Mr}
      end
   end

   fun {IsOfClass M C}
      {Member C {CondSelect M 'class' nil}}
   end

   MAXERRORS = 17

   {ErrorRegistry.put sgml
    fun {$ Exc}
       E = {Error.dispatch Exc}
       T = 'SGML parser error'
    in
       case E of sgml(unsupportedCommand C S) then
	  {Error.format T
	   'unsupported command'
	   [hint(l: 'Command' m: [C])
	    hint(l: 'Arguments' m: S)]
	   Exc}
       elseof sgml(unsupportedEscape S) then
	  {Error.format T
	   'unsupported argument escape sequence'
	   [hint(l: 'Arguments' m: S)]
	   Exc}
       elseof sgml(unsupportedAttributeType Type) then
	  {Error.format T
	   'unsupported attribute type'
	   [hint(l: 'Type' m: {Atom.toString Type})]
	   Exc}
       elseof sgml(unsupportedAttributeType Type Value) then
	  {Error.format T
	   'unsupported attribute type'
	   [hint(l: 'Type' m: {Atom.toString Type})
	    hint(l: 'Value' m: Value)]
	   Exc}
       elseof sgml(nonconforming FileNameVS VSs) then
	  {Error.format T
	   'nonconforming sgml file'
	   if {Length VSs} > MAXERRORS then
	      {Append
	       {Map {List.take VSs MAXERRORS} fun {$ VS} line(VS) end}
	       [line('...')]}
	   else
	      {Map VSs fun {$ VS} line(VS) end}
	   end
	   Exc}
       else
	  {Error.formatGeneric T Exc}
       end
    end}
end
