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
export
   'class': LaTeXToGIFClass
define
   LATEX2GIF = 'latex2gif'

   class LaTeXToGIFClass
      attr Dict: unit Keys: unit DirName: unit N: unit
      meth init(Dir)
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
         case @Dict of unit then
            Dict <- {NewDictionary}
            Keys <- nil
         else skip
         end
         A = {VirtualString.toAtom VS}
         case {Dictionary.condGet @Dict A unit} of unit then
            N <- @N + 1
            OutFileName = 'latex'#@N#'.gif'
            {Dictionary.put @Dict A OutFileName}
            Keys <- A|@Keys
         else
            {Dictionary.get @Dict A OutFileName}
         end
      end
      meth process(Packages Reporter)
         case @Dict of unit then skip
         else FileName File in
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
            {ForAll {Reverse @Keys}
             proc {$ X}
                {File write(vs: X#'\n\\clearpage\n')}
             end}
            {File write(vs: '\\end{document}\n')}
            {File close()}
            try
               case
                  {OS.system LATEX2GIF#' '#FileName#' '#@N#' '#@DirName}
               of 0 then skip
               elseof I then
                  {Exception.raiseError ozDoc(latexToGif I)}
               end
            finally
               {OS.unlink FileName}
            end
         end
      end
   end
end
