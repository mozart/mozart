%%%
%%% Author:
%%%   Denys Duchier <duchier@ps.uni-sb.de>
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Denys Duchier. 1998
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

%%% ==================================================================
%%% HISTORY
%%%	The idea of using emacs for fontification is due to Leif
%%% Kornstaed who wrote the first version of this library. This new
%%% implementation was written by Denys Duchier, and generalizes the
%%% previous one.  In particular it supports structured requests.
%%%
%%% INTERFACE
%%%
%%%
%%% ==================================================================

functor
import
   Open OS Property
export
   'class'	: Fontifier
   vs		: FontifyVirtualString
   file		: FontifyFile
   complex	: FontifyComplex
   SetEmacs
   SetEmacsLoadPath
   GetEmacsLoadPath
   PushEmacsLoadPath
   SetEmacsRequires
   GetEmacsRequires
   PushEmacsRequires
define
   EMACS = {NewCell 'emacs'}
   EPATH = {NewCell [{Property.get 'oz.home'}#'/share/elisp']}
   ELOAD = {NewCell ['oz.elc' 'ozdoc-fontify.elc']}

   fun {MakeCommand}
      {Access EMACS}#' --batch '#
      {FoldR {Access EPATH}
       fun {$ DIR VS} '-L '#DIR#' '#VS end nil}#
      {FoldR {Access ELOAD}
       fun {$ FILE VS} '-l '#FILE#' '#VS end nil}#
      '-f ozdoc-fontify'
   end

   proc {SetEmacs X} {Assign EMACS X} end
   proc {SetEmacsLoadPath L} {Assign EPATH L} end
   proc {GetEmacsLoadPath L} {Access EPATH L} end
   proc {SetEmacsRequires L} {Assign ELOAD L} end
   proc {GetEmacsRequires L} {Access ELOAD L} end
   proc {PushEmacsLoadPath D} {Assign EPATH D|{Access EPATH}} end
   proc {PushEmacsRequires F} {Assign ELOAD F|{Access ELOAD}} end

   %% !!! TEMPORARY WORK AROUND FOR BUG WITH BYNEED VARIABLES !!!
   proc {Force L N}
      if N==0 then skip else
	 {Wait L}
	 case L of _|T then
	    {Force T N-1}
	 else skip end
      end
   end
   
   %% The builtin mark is Control-D
   %% but something else could be used instead
   
   proc {UpToMark L Prefix Suffix}
      {Force L 1}
      case L of nil then Prefix=Suffix=nil
      [] H|T then
	 if H==4 then Prefix=nil Suffix=T
	 else MorePrefix in
	    Prefix=H|MorePrefix
	    {UpToMark T MorePrefix Suffix}
	 end
      end
   end

   %% ReadFile opens a file for read and returns a lazy
   %% stream of its contents
   
   fun {ReadFile FileName}
      {LazyRead {New Open.file init(name:FileName flags:[read])}}
   end

   fun lazy {LazyRead File}
      N Head Tail
   in
      {File read(len:N list:Head tail:Tail)}
      if N==0 then Tail=nil else
	 Tail={LazyRead File}
      end
      Head
   end

   %% ReadEvents returns a lazy list of alternating mark(_) and
   %% data(_) records by splitting up its lazy input string at
   %% mark characters (i.e. Control-Ds).

   fun lazy {ReadEvents L}
      {Force L 3}
      case L of nil then nil
      [] 4|M|T then
	 case M
	 of &B then begin|{ReadEvents T}
	 [] &E then 'end'|{ReadEvents T}
	 [] &O then
	    case T of H|LL then 
	       case H
	       of &S then simple |{ReadEvents LL}
	       [] &C then complex|{ReadEvents LL}
	       end
	    end
	 [] &C then close|{ReadEvents T}
	 [] &T then L in
	    text|data({ReadData T L})|{ReadEvents L}
	 end
      end
   end

   fun {ReadData L LL}
      {Force L 1}
      case L of H|T then
	 if H==4 then LL=L nil
	 else H|{ReadData T LL} end
      end
   end

   %% a Fontifier object accumulates requests for fontifying
   %% virtual strings and/or files.  The synck method causes the
   %% requests accumulated so far to be processed in one batch.

   class Fontifier
      prop locking
      attr Head Tail
      meth init L in Head<-L Tail<-L end
      meth Enqueue(Request)
	 lock NewTail in
	    @Tail = Request|NewTail
	    Tail <- NewTail
	 end
      end
      meth enqueueFile(Mode FileName Res)
	 Fontifier,Enqueue(file(Mode FileName Res))
      end
      meth enqueueVirtualString(Mode VS Res)
	 Fontifier,Enqueue(data(Mode simple(VS Res)))
      end
      meth enqueueComplex(Mode Complex)
	 Fontifier,Enqueue(data(Mode Complex))
      end
      meth synck Requests in
	 lock L in
	    Requests=@Head @Tail=nil
	    Head <- L
	    Tail <- L
	 end
	 {ProcessRequests Requests}
      end
      meth processFile(Mode File Res)
	 lock
	    {self enqueueFile(Mode File Res)}
	    {self synck}
	 end
      end
      meth processVirtualString(Mode Vs Res)
	 lock
	    {self enqueueVirtualString(Mode Vs Res)}
	    {self synck}
	 end
      end
      meth processComplex(Mode Complex)
	 lock
	    {self enqueueComplex(Mode Complex)}
	    {self synck}
	 end
      end
   end

   proc {CopyFile FileNameIn FileOut}
      {CopyText {ReadFile FileNameIn} FileOut}
   end

   CopySize = 200

   proc {CopyText Text FileOut}
      if Text==nil then skip else
	 Prefix Suffix
      in
	 {List.takeDrop Text CopySize Prefix Suffix}
	 {FileOut write(vs:Prefix)}
	 {CopyText Suffix FileOut}
      end
   end

   proc {ProcessRequests Requests}
      FileNameIn  = {OS.tmpnam}
      FileNameOut = {OS.tmpnam}
      FileIn      = {New Open.file init(name :FileNameIn
					flags:[write create truncate])}
      Specs
   in
      try
	 Specs =
	 {Map Requests
	  fun {$ R} {WriteRequest R FileIn} end}
	 {FileIn close}
	 case {OS.system
	       {MakeCommand}#' '#FileNameIn#' '#FileNameOut}
	 of 0 then skip
	 elseof I then
	    {Exception.raiseError fontify(I)}
	 end
	 {FoldL Specs
	  fun {$ Events Spec}
	     {GetAnswer Events $ Spec}
	  end
	  {ReadEvents {ReadFile FileNameOut}} _}
      finally
	 try {OS.unlink FileNameIn } catch _ then skip end
	 try {OS.unlink FileNameOut} catch _ then skip end
      end
   end

   MarkBegin	= [4 &B]
   MarkEnd	= [4 &E]
   MarkText	= [4 &T]
   MarkOpen	= [4 &O]
   MarkClose	= [4 &C]

   %% WriteRequest returns a result spec
   %% <Result Spec> ::= simple(R)
   %%		    |   complex([<Result Spec> ...])
   %% where R is the corresponding result var in the request

   fun {WriteRequest Request File}
      case {Label Request}
      of file then {WriteFileRequest Request File}
      [] data then {WriteDataRequest Request File}
      else raise bad end end
   end

   fun {WriteFileRequest Request File}
      file(Mode FileName Result) = Request
   in
      {File write(vs:MarkBegin)}
      {File write(vs:Mode)}
      {File write(vs:MarkText)}
      {CopyFile FileName File}
      {File write(vs:MarkEnd)}
      simple(Result)
   end

   proc {WriteDataRequest Request File ?Result}
      data(Mode Spec) = Request
   in
      {File write(vs:MarkBegin)}
      {File write(vs:Mode)}
      {WriteSpec Spec File Result}
      {File write(vs:MarkEnd)}
   end

   proc {WriteSpec Spec File Result}
      case Spec
      of simple(VS Res) then
	 {File write(vs:MarkText#VS)}
	 Result=simple(Res)
      [] complex(Specs) then
	 {File write(vs:MarkOpen)}
	 Result=complex({Map Specs fun {$ S} {WriteSpec S File} end})
	 {File write(vs:MarkClose)}
      end
   end

   fun lazy {GetAnswer Events RestEvents}
      {Force Events 1}
      case Events of begin|Events then
	 {GetSpec Events 'end'|RestEvents}
      end
   end

   fun {GetSpec Events RestEvents}
      {Force Events 1}
      case Events of Kind|Events then
	 case Kind
	 of simple  then {GetSimple  Events RestEvents}
	 [] complex then {GetComplex Events RestEvents}
	 end
      end
   end

   fun {GetSimple Events RestEvents}
      simple({GetAlist Events RestEvents})
   end

   fun {GetAlist Events RestEvents}
      {Force Events 4}
      case Events
      of close|L then RestEvents=L nil
      %%[]
      elseof text|data(Face)|text|data(Text)|L then
	 ({String.toAtom Face}#Text)
	 | {GetAlist L RestEvents}
      end
   end

   fun {GetComplex Events RestEvents}
      complex({GetSpecs Events RestEvents})
   end

   fun {GetSpecs Events RestEvents}
      {Force Events 1}
      case Events of close|Events then Events=RestEvents nil
      else TmpEvents in
	 {GetSpec Events TmpEvents}
	 | {GetSpecs TmpEvents RestEvents}
      end
   end

   fun {FontifyVirtualString Mode Vs}
      F = {New Fontifier init}
   in
      {F processVirtualString(Mode Vs $)}
   end

   fun {FontifyFile Mode FileName}
      F = {New Fontifier init}
   in
      {F processFile(Mode FileName $)}
   end

   proc {FontifyComplex Mode Complex}
      F = {New Fontifier init}
   in
      {F processComplex(Mode Complex)}
   end
end
