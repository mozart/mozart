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
%%% {Fontifier.emacs.set PGM}
%%% {Fontifier.emacs.get PGM}
%%%
%%%	set or get the name of the emacs program.  You only need to
%%% set this if emacs has an unusual name on your system, or you want
%%% to pick up a different version (say, not in your path).
%%%
%%% {Fontifier.loadpath.get  LOADPATH}
%%% {Fontifier.loadpath.set  LOADPATH}
%%% {Fontifier.loadpath.push DIR}
%%%
%%%	LOADPATH is a list of directories to add to emacs load-path.
%%% The default adds $OZHOME/share/elisp to search for oz specific
%%% elisp libraries.
%%%
%%% {Fontifier.requires.get  FILES}
%%% {Fontifier.requires.set  FILES}
%%% {Fontifier.requires.push FILE}
%%%
%%%	the FILES will be loaded on startup.  One of them should
%%% provide an implementation for (ozdoc-fontify).  The default is to
%%% load both oz.{el,elc} and Fontifier.{el,elc}.
%%%
%%% {Fontifier.processVirtualString MODE VS RESULT}
%%%
%%%	MODE is the name of an emacs mode (with or without the -mode
%%% suffix) that is appropriate for editing the code represented by
%%% virtual string VS.  Emacs is invoked, the code is installed in a
%%% buffer, MODE is turned on, and fontification is requested.  The
%%% resulting face assignments are then examined and returned in
%%% RESULT as a list of pairs FACE#STRING, where FACE is a symbol
%%% denoting the face that was assigned to the piece of text
%%% represented by STRING.  The pairs are presented in sequential
%%% order of the source code (of course).
%%%
%%% {Fontifier.processFile MODE FILENAME RESULT}
%%%
%%%	This is similar to the above, except that the source code is
%%% to be obtained from a file.
%%%
%%% {Fontifier.processRequest MODE REQUEST}
%%%
%%%	Sometimes the source code has already been parsed and you want
%%% to add highlighting indications to this parsed representation. You
%%% could try to highlight separately each piece of code text in your
%%% parse tree, but the result would be poor since, in general,
%%% highlighting is context dependent.  Instead, you can map you parse
%%% tree to a "Structured Request", that accurately encodes the tree
%%% structure of you parse tree, but performs highligting using the
%%% full textual data that it contains.  A <REQUEST> has the following
%%% structure:
%%%		<REQUEST> ::= simple(<STRING> <RESULT>)
%%%			  |   complex([<REQUEST> ...])
%%%
%%% <RESULT> is simply a variable that gets bound to a list of
%%% FACE#STRING pairs as described earlier.
%%%
%%% An application of this technique devised by Leif Kornstaed is to
%%% provide context to obtain proper highlighting of code.  For
%%% example, if FooClass is the name of an Oz class, in order to have
%%% it properly highlighted, it must be preceded by keyword `class'.
%%% In this case you can use the following request:
%%%
%%%	complex([simple("class " _) simple("FooClass" RESULT)])
%%%
%%% where RESULT will be bound to the correct highlighting annotations
%%%
%%% Fontifier.'class'
%%%
%%%	When many code chunks need to be processed for highlighting,
%%% such as is frequently the case during document processing, then it
%%% it more efficient to only invoke emacs once and process all of
%%% them in a batch manner.  By creating an OBJECT of class
%%% Fontifier.'class', you can queue processing requests and then
%%% cause all of them to be processed in one go.
%%%
%%% OBJECT = {New Fontifier.'class' init}
%%%
%%% {OBJECT enqueueVirtualString(MODE VS RESULT)}
%%% {OBJECT enqueueFile(MODE FILENAME RESULT)}
%%% {OBJECT enqueueRequest(MODE REQUEST)}
%%%
%%% The arguments are the same as for the process* procedures
%%% described earlier.
%%%
%%% {OBJECT synck}
%%%
%%%	causes the currently buffered requests to be processed.
%%%
%%% {OBJECT processVirtualString(MODE VS RESULT)}
%%% {OBJECT processFile(MODE FILENAME RESULT)}
%%% {OBJECT processRequest(MODE REQUEST)}
%%%
%%%	These are equivalent to an enqueue* followed by a synck.  They
%%% are used to implement the process* procedures described earlier.
%%% ==================================================================

functor
import
   Open OS Property
   IO at 'x-oz://contrib/os/io'
   PROC at 'x-oz://contrib/os/process'
export
   'class'	: FontifierClass
   emacs	: ApiEmacs
   loadpath	: ApiEpath
   requires	: ApiEload
   ProcessVirtualString
   ProcessFile
   ProcessRequest
define
   EMACS = {NewCell 'emacs'}
   EPATH = {NewCell [{Property.get 'oz.home'}#'/share/elisp']}
   ELOAD = {NewCell ['oz' 'Fontifier']}

   ApiEmacs = o(get :proc {$ X} {Access EMACS X} end
		set :proc {$ X} {Assign EMACS X} end)
   ApiEpath = o(get :proc {$ X} {Access EPATH X} end
		set :proc {$ X} {Assign EPATH X} end
		push:proc {$ X} L in {Exchange EPATH L X|L} end)
   ApiEload = o(get :proc {$ X} {Access ELOAD X} end
		set :proc {$ X} {Assign ELOAD X} end
		push:proc {$ X} L in {Exchange ELOAD L X|L} end)

   proc {MakeProcess FileIn FileOut PID}
      IN   = {IO.devNull}
      ARGS = '--batch'|EVAL
      EVAL = case {Access EPATH} of nil then LOAD
	     elseof DIRS then
		'--eval'|
		'(setq load-path (append \'('#
		{FoldR DIRS
		 fun {$ DIR L}
		    %% skip empty directories
		    if DIR==nil then L
		    elseif L==nil then '"'#DIR#'"'
		    else '"'#DIR#'" '#L end
		 end nil}#
		') load-path))'|LOAD
	     end
      LOAD = {FoldR {Access ELOAD}
	      fun {$ FILE L}
		 %% skip empty files
		 if FILE==nil then L else '-l'|FILE|L end
	      end
	      ['-f' 'ozdoc-fontify' FileIn FileOut]}
   in
      {IO.run process({Access EMACS} ARGS stdin:IN) PID}
      {IO.close IN}
   end

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
      if N==0 then {File close} Tail=nil else
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

   class FontifierClass
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
	 FontifierClass,Enqueue(file(Mode FileName Res))
      end
      meth enqueueVirtualString(Mode VS Res)
	 FontifierClass,Enqueue(data(Mode simple(VS Res)))
      end
      meth enqueueRequest(Mode Complex)
	 FontifierClass,Enqueue(data(Mode Complex))
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
      meth processRequest(Mode Complex)
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
      Specs Proc
   in
      try
	 Specs =
	 {Map Requests
	  fun {$ R} {WriteRequest R FileIn} end}
	 {FileIn close}
	 Proc = {MakeProcess FileNameIn FileNameOut}
	 case {PROC.status Proc}
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

   fun {ProcessVirtualString Mode Vs}
      F = {New FontifierClass init}
   in
      {F processVirtualString(Mode Vs $)}
   end

   fun {ProcessFile Mode FileName}
      F = {New FontifierClass init}
   in
      {F processFile(Mode FileName $)}
   end

   proc {ProcessRequest Mode Request}
      F = {New FontifierClass init}
   in
      {F processRequest(Mode Request)}
   end
end
