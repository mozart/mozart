%%%
%%% Authors:
%%%   Author's name (Author's email address)
%%%
%%% Contributors:
%%%   optional, Contributor's name (Contributor's email address)
%%%
%%% Copyright:
%%%   Organization or Person (Year(s))
%%%
%%% Last change:
%%%   $Date$ by $Author$
%%%   $Revision$
%%%
%%% This file is part of Mozart, an implementation
%%% of Oz 3
%%%    $MOZARTURL$
%%%
%%% See the file "LICENSE" or
%%%    $LICENSEURL$
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%
%%% $Id$
%%% Benjamin Lorenz <lorenz@ps.uni-sb.de>

fun
\ifdef NEWCOMPILER
   instantiate
\endif
   {$ IMPORT}
   \insert 'SP.env'
       = IMPORT.'SP'
   \insert 'OP.env'
       = IMPORT.'OP'

   TimeoutToUpdateBar = 430
   TimeoutToConfigBar = 70

   S2A = String.toAtom
   fun {VS2A X}
      {S2A {VirtualString.toString X}}
   end
   fun {V2VS X}
      P = {System.get errors}
   in
      {System.valueToVirtualString X P.depth P.width}
   end

   Platform = local
		 X#Y = {System.get platform}
	      in
		 {VS2A X#'-'#Y}
	      end
   WindowsPlatform = 'win32-i486'

   OzRawPath       = {OS.getEnv 'OZPATH'}
   FieldSeparator  = case Platform == WindowsPlatform then &; else &: end
   OzPath

   local
      fun {PathList RawPath} % RawPath must be of type string
	 H T P in
	 {List.takeDropWhile RawPath fun {$ C} C \= FieldSeparator end H T}
	 P = {VS2A H#'/'}
	 case T == nil then P|nil
	 else P|{PathList T.2}
	 end
      end
   in
      OzPath = {PathList OzRawPath}
   end

   proc {Trace M}
      case {Emacs checkVerbose($)} then
	 {System.showInfo 'Emacs: ' # M}
      else skip end
   end

   proc {MagicEmacsBar File Line Column State}
      C = case Column == unit then 0 else Column end
   in
      {Delay 5}
      {Print {VS2A 'oz-bar ' # File # ' ' # Line # ' ' # C # ' ' # State}}
      {Delay 5}
   end

   fun {UnknownFile F}
      F == nofile orelse F == ''
   end

   local
      LS = 'file lookup: '
      fun {DoLookupFile SearchList F OrigF}
	 case SearchList of nil then
	    %% must have been the name of an unsaved file or buffer in Emacs:
	    OrigF
	 elseof Path|SearchListRest then Try = Path # F in
	    try
	       case {OS.stat Try}.type == reg then
		  {Trace LS # F # ' is ' # Try}
		  {VS2A Try}
	       else
		  {Trace LS # F # ' is not ' #
		   Try # ': ' # {V2VS {OS.stat Try}}}
		  {DoLookupFile SearchListRest F OrigF}
	       end
	    catch system(...) then
	       {Trace LS # F # ' is not ' # Try # ': file not found'}
	       {DoLookupFile SearchListRest F OrigF}
	    end
	 end
      end
   in
      fun {LookupFile F}
	 S   = {Atom.toString F}
	 Abs = case S                   % absolute path?
	       of     &/|_   then true
	       elseof _|&:|_ then Platform == WindowsPlatform
	       else false end
      in
	 case Abs then
	    %% the file doesn't need to exist, since it may be the name of
	    %% an unsaved buffer or file in Emacs:
	    F
	 else                           % ...no!
	    %% strip "./" or "././"
	    Suffix = case S of &.|&/|T then
			case T of &.|&/|R then R
			else T end
		     else S end
	 in
	    {DoLookupFile OzPath Suffix F}
	 end
      end
   end

   Emacs =
   {New class $

	   prop
	      final

	   attr
	      BarSync  : _
	      Verbose  : false

	   meth init
	      skip
	   end

	   %% should we print debugging code?
	   meth setVerbose(B) Verbose <- B end
	   meth checkVerbose($) @Verbose end

	   meth bar(file:F line:L column:C state:S)=M
	      BarSync <- _ = unit
	      {Trace {V2VS M}}
	      case {UnknownFile F} orelse L == unit then
		 {self removeBar}
	      else
		 {MagicEmacsBar {LookupFile F} L C S}
	      end
	   end

	   meth delayedBar(file:F line:L column:C state:S<=unchanged)
	      New in BarSync <- New = unit
	      thread
		 {WaitOr New {Alarm TimeoutToUpdateBar}}
		 case {IsDet New} then skip else
		    {self bar(file:F line:L column:C state:S)}
		 end
	      end
	   end

	   meth configureBar(State)=M
	      New in BarSync <- New = unit
	      {Trace {V2VS M}}
	      thread
		 {WaitOr New {Alarm TimeoutToConfigBar}}
		 case {IsDet New} then skip else
		    {MagicEmacsBar unchanged 0 0 State}
		 end
	      end
	   end

	   meth removeBar
	      BarSync <- _ = unit
	      {Trace 'removing bar'}
	      {MagicEmacsBar nofile 0 0 hide}
	   end

	end init}

in

   \insert 'Emacs.env'

end
