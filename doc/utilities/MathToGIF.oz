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
   'class': MathToGIFClass
define
   LATEX2GIF = 'latex2gif'

   class MathToGIFClass
      attr DirName: unit N: unit
      meth init(Dir)
         DirName <- Dir
         N <- 0
      end
      meth convertLaTeX(VS Display ?OutFileName) FileName File in
         FileName = {OS.tmpnam}
         File = {New Open.file init(name: FileName
                                    flags: [write create truncate])}
         {File write(vs: ('\\documentclass{report}\n'#
                          '\\usepackage{wasysym}\n'#
                          '\\pagestyle{empty}\n'#
                          '\\begin{document}\n'#
                          case Display of display then '\\[\n'#VS#'\n\\]\n'
                          [] inline then '$'#VS#'$\n'
                          end#
                          '\\end{document}\n'))}
         {File close()}
         try
            N <- @N + 1
            OutFileName = 'math'#@N#'.gif'
            case
               {OS.system LATEX2GIF#' '#FileName#' '#@DirName#'/'#OutFileName}
            of 0 then skip
            elseof I then
               {Exception.raiseError ozDoc(mathToGif VS I)}
            end
         finally
            {OS.unlink FileName}
         end
      end
   end
end
