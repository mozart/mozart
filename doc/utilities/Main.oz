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
   Syslet(spec args exit)
   Property(get)
   System(printError)
   Error(printExc)
   OzDocToHTML(translate)
define
   Syslet.spec = single('in'(type: string)
			'type'(type: string)
			'out'(type: string))
   try
      case Syslet.args.'in' of "" then
	 {Raise usage('no input file name specified')}
      elsecase Syslet.args.'out' of "" then
	 {Raise usage('no output directory name specified')}
      elsecase Syslet.args.2 of _|_ then
	 {Raise usage('unrecognized command line arguments')}
      elsecase Syslet.args.'type' of "html-color" then
	 {OzDocToHTML.translate true Syslet.args.'in' Syslet.args.'out'}
      elseof "html-mono" then
	 {OzDocToHTML.translate false Syslet.args.'in' Syslet.args.'out'}
      else
	 {Raise usage('illegal output type specified')}
      end
      {Syslet.exit 0}
   catch E then
      case E of usage(M) then
	 {System.printError
	  'Command line option error: '#M#'\n'#
	  'Usage: '#{Property.get 'root.url'}#' [options]\n'#
	  '--in=<File>      Specify the input SGML file.\n'#
	  '--type=<Type>    Specify what format to generate\n'#
	  '                 (supported: html-mono html-color).\n'#
	  '--out=<Dir>      Specify the output directory or file.\n'}
	 {Syslet.exit 2}
      else
	 {Error.printExc E}
	 {Syslet.exit 1}
      end
   end
end
