%%% ==================================================================
%%% {Shell.executeCommand CMD}
%%% {Shell.executeProgram CMD}
%%%	CMD is a list whose 1st element is the program to execute, and
%%%	the remaining elements are the arguments to this program.  In
%%%	order to be run by the shell, CMD must be transformed into a
%%%	virtual string with all elements appropriately quoted.  The
%%%	quote character is " for Windows and ' otherwise.  With
%%%	executeCommand, the command is explicitly passed to a shell.
%%%
%%% {Shell.quoteUsing CMD QUOTE}
%%%	CMD is as above and QUOTE is a virtual string to use as the
%%%	quoting character.
%%%
%%% {Shell.toUserVS CMD}
%%%	calls Shell.quoteUsing with an empty QUOTE.
%%% ==================================================================
functor
export
   ExecuteProgram
   ExecuteCommand
   QuoteUsing
   ToUserVS
import% System
   Property(get)     at 'x-oz://system/Property.ozf'
   OS(system getEnv) at 'x-oz://system/OS.ozf'
   Open(pipe)        at 'x-oz://system/Open.ozf'
   Windows at 'Windows.ozf'
define

   %% the arguments on the command given to the shell for
   %% execution need to be quoted to avoid problems with
   %% embeded spaces in filenames and special shell
   %% characters
   
   QUOTE = if Windows.isWin then '"' else '\'' end

   fun {QuoteUsing CMD Quote}
      %% CMD is a list: we are going to quote each element of
      %% this list using the Quote string specified and then
      %% we are going to concatenate all them separated by
      %% single spaces.
      {FoldL CMD
       %% in principle, we should be careful about embedded
       %% occurrences of characters used for quoting - we
       %% ignore this issue for the nonce
       fun {$ VS I} VS#' '#Quote#I#Quote end nil}
   end

   %% an advantage of using an arbitrary VS as a Quote, in
   %% the above is that we can also use an empty VS.  This
   %% turns out to be useful when we want to display commands
   %% to the user.  They are harder to read when properly
   %% quoted
   
   fun {ToUserVS CMD} {QuoteUsing CMD ''} end

   %% for execution, use the platform specific quote

   %% On Win95/98 (aka old Windows), we need to invoke
   %% COMMAND.COM, i.e. the shell specified by environment
   %% variable COMSPEC.  On other systems OS.system already
   %% does the right thing.

   ToProgramVS
   if Windows.isOldWin then
      fun {!ToProgramVS CMD}
	 'COMMAND.COM /C'#{QuoteUsing CMD QUOTE}
      end
   else
      fun {!ToProgramVS CMD}
	 {QuoteUsing CMD QUOTE}
      end
   end

   ToCommandVS
   if Windows.isWin then
      if Windows.isOldWin then
	 fun {!ToCommandVS CMD}
	    First|Rest = CMD
	 in
	    'COMMAND.COM /C '#First#{QuoteUsing Rest QUOTE}
	 end
      else
	 fun {!ToCommandVS CMD}
	    First|Rest = CMD
	 in
	    First#{QuoteUsing Rest QUOTE}
	 end
      end
   else
      !ToCommandVS=ToProgramVS
   end
	
   proc {Execute VS}
      %{System.showInfo 'EXECUTING: '#VS}
      if {OS.system VS}\=0 then raise shell(VS) end end
   end

   proc {ExecuteProgram CMD} {Execute {ToProgramVS CMD}} end
   proc {ExecuteCommand CMD} {Execute {ToCommandVS CMD}} end
end
