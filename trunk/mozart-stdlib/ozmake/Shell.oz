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
import
   Property(get)     at 'x-oz://system/Property.ozf'
   OS(system getEnv) at 'x-oz://system/OS.ozf'
   Open(pipe)        at 'x-oz://system/Open.ozf'
define
   %% on Windows we use the shell specified by environment
   %% variable COMSPEC, on other platforms we specify
   %% nothing which means we get `sh'.

   IsWindows=({Property.get 'platform.os'}=='win32')
   
   SHELL = if IsWindows then
	      {OS.getEnv 'COMSPEC'}#' /c '
	   else nil end

   %% the arguments on the command given to the shell for
   %% execution need to be quoted to avoid problems with
   %% embeded spaces in filenames and special shell
   %% characters
   
   QUOTE = if {Property.get 'platform.os'}=='win32'
	   then '"' else '\'' end

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

   fun {ToProgramVS CMD} {QuoteUsing CMD QUOTE} end
   fun {ToCommandVS CMD|ARGS}
      QARGS={QuoteUsing ARGS QUOTE}
   in
      SHELL#' '#CMD#if QARGS==nil then nil else ' '#QARGS end
   end

   proc {ExecuteCommand CMD} VS={ToCommandVS CMD} in
      if {OS.system VS}\=0 then
	 raise shell(VS) end
      end
   end
   
   proc {ExecuteProgram CMD}
      if IsWindows then
	 PGM={New Open.pipe init(cmd:CMD.1 args:CMD.2)}
      in
	 for break:Break do
	    S={PGM read(list:$)}
	 in
	    if S==nil then {Break} else
	       {System.printError S}
	    end
	 end
	 try {PGM close} catch _ then skip end
      else VS={ToProgramVS CMD} in
	 if {OS.system VS}\=0 then
	    raise shell(VS) end
	 end
      end
   end
end
