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
%%%    http://mozart.ps.uni-sb.de
%%%
%%% See the file "LICENSE" or
%%%    http://mozart.ps.uni-sb.de/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor
import
   OS(tmpnam system unlink)
   Open(file)
   Gdbm at 'x-oz://contrib/gdbm'
   File(read: ReadFile write: WriteFile)
export
   'class': LaTeXToGIFClass
define
   LATEX2GIF = 'latex2gif'

   class LaTeXToGIFClass
      attr DB: unit Dict: unit Keys: unit DirName: unit N: unit
      meth init(Dir DBName)
	 DB <- case DBName of unit then unit
	       else
		  try
		     {Gdbm.new write(DBName)}
		  catch _ then
		     {Gdbm.new create(DBName)}
		  end
	       end
	 Dict <- {NewDictionary}
	 Keys <- nil
	 DirName <- Dir
	 N <- 0
      end
      meth convertPicture(VS ?OutFileName)
	 LaTeXToGIFClass, Enter(VS ?OutFileName)
      end
      meth convertMath(VS Display ?OutFileName)
	 LaTeXToGIFClass, Enter(case Display of inline then '$'#VS#'$\n'
				[] display then '\\[\n'#VS#'\n\\]\n'
				end ?OutFileName)
      end
      meth Enter(VS OutFileName) A in
	 A = {VirtualString.toAtom VS}
	 case {Dictionary.condGet @Dict A unit} of unit then
	    N <- @N + 1
	    OutFileName = 'latex'#@N#'.gif'
	    {Dictionary.put @Dict A OutFileName}
	    case @DB == unit orelse {Gdbm.condGet @DB A true} of true then
	       Keys <- A#@N|@Keys
	    elseof X then
	       {WriteFile X @DirName#'/'#OutFileName}
	    end
	 elseof FileName then
	    OutFileName = FileName
	 end
      end
      meth process(Packages Reporter)
	 case @Keys of nil then skip
	 else FileName File Outs in
	    {Reporter startSubPhase('converting LaTeX sections to GIF')}
	    FileName = {OS.tmpnam}
	    File = {New Open.file init(name: FileName
				       flags: [write create truncate])}
	    {File write(vs: '\\documentclass{report}\n')}
	    {ForAll Packages
	     proc {$ P}
		{File write(vs: '\\usepackage{'#P#'}\n')}
	     end}
	    {File write(vs: ('\\pagestyle{empty}\n'#
			     '\\begin{document}\n'))}
	    Outs = {FoldR @Keys
		    fun {$ X#N In}
		       {File write(vs: X#'\n\\clearpage\n')}
		       In#' '#N
		    end ""}
	    {File write(vs: '\\end{document}\n')}
	    {File close()}
	    try
	       case {OS.system LATEX2GIF#' '#FileName#' '#@DirName#Outs}
	       of 0 then skip
	       elseof I then
		  {Exception.raiseError ozDoc(latexToGif I)}
	       end
	       if @DB \= unit then
		  {ForAll @Keys
		   proc {$ X#_}
		      {Gdbm.put @DB X
		       {ReadFile @DirName#'/'#{Dictionary.get @Dict X}}}
		   end}
	       end
	    finally
	       {OS.unlink FileName}
	       if @DB \= unit then
		  {Gdbm.close @DB}
	       end
	    end
	 end
      end
   end
end
