%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Contributor:
%%%   Denys Duchier <duchier@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 1998
%%%   Denys Duchier, 1998
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
   Property(get put)
   System(printError)
   Error(printExc)
   OzDocToHTML(translate)
   OS(getEnv putEnv)
   URL
define
   Syslet.spec = single('in'(         type:string optional:false default:unit)
			'type'(       type:string optional:false default:unit)
			'out'(        type:string optional:false default:unit)
			'autoindex'(  type:bool                  default:false)
			% HTML options
			'stylesheet'( type:string optional:false default:unit)
			'latexmath'(  type:bool                  default:true)
			'split'(      type:bool                  default:true)
			% Path names
			'ozdoc-home'( type:string optional:false default:unit)
			'author-path'(type:string optional:false default:unit)
			'bib-path'(   type:string optional:false default:unit)
			'bst-path'(   type:string optional:false default:unit)
			'elisp-path'( type:string optional:false default:unit)
			'sbin-path'(  type:string optional:false default:unit)
			'catalog'(    type:string optional:false default:unit)
		       )
   % Process path name options and store results in ozdoc.* properties
   local
      % Determine the directory in which document source files are located:
      SRC_DIR =
      case Syslet.args.'in' of unit then '.'
      elseof X then
	 Url  = {URL.make X}
	 Path = {CondSelect Url path unit}
      in
	 if Path==unit orelse Path.1==nil then
	    '.'
	 else
	    Lab = {Label Path}
	    L1  = Path.1
	    N   = {Length L1}
	    L2  = {List.take L1 N-1}
	 in
	    case L2 of nil then
	       {URL.toString {AdjoinAt Url path Lab(["."#false])}}
	    elsecase {Reverse L2} of (C#_)|L then
	       {URL.toString {AdjoinAt Url path
			      Lab({Reverse (C#false)|L})}}
	    end
	 end
      end
      {Property.put 'ozdoc.src.dir' SRC_DIR}
      OZDOC_HOME =
      case Syslet.args.'ozdoc-home' of unit then
	 case {OS.getEnv 'OZDOC_HOME'} of false then
	    {Property.get 'oz.home'}#'/share/doc'
	 elseof X then X end
      elseof X then X end
      {Property.put 'ozdoc.home' OZDOC_HOME}
      AUTHOR_PATH =
      case Syslet.args.'author-path' of unit then
	 case {OS.getEnv 'OZDOC_AUTHOR_PATH'} of false then
	    SRC_DIR#':'#OZDOC_HOME
	 elseof X then X end
      elseof X then X end
      {Property.put 'ozdoc.author.path' AUTHOR_PATH}
      BIB_PATH =
      case Syslet.args.'bib-path' of unit then
	 case {OS.getEnv 'OZDOC_BIB_PATH'} of false then
	    SRC_DIR#':'#OZDOC_HOME
	 elseof X then X end
      elseof X then X end
      {Property.put 'ozdoc.bib.path' BIB_PATH}
      BST_PATH =
      case Syslet.args.'bst-path' of unit then
	 case {OS.getEnv 'OZDOC_BST_PATH'} of false then
	    BIB_PATH
	 elseof X then X end
      elseof X then X end
      {Property.put 'ozdoc.bst.path' BST_PATH}
      ELISP_PATH =
      case Syslet.args.'elisp-path' of unit then
	 case {OS.getEnv 'OZDOC_ELISP_PATH'} of false then
	    {Property.get 'oz.home'}#'/share/elisp'
	 elseof X then X end
      elseof X then X end
      {Property.put 'ozdoc.elisp.path' ELISP_PATH}
      SBIN_PATH =
      case Syslet.args.'sbin-path' of unit then
	 case {OS.getEnv 'OZDOC_SBIN_PATH'} of false then
	    OZDOC_HOME
	 elseof X then X end
      elseof X then X end
      {Property.put 'ozdoc.sbin.path' SBIN_PATH}
      CSS =
      case Syslet.args.'stylesheet' of unit then
	 case {OS.getEnv 'OZDOC_STYLESHEET'} of false then
	    'http://www.ps.uni-sb.de/css/ozdoc.css'
	 elseof X then X end
      elseof X then X end
      {Property.put 'ozdoc.stylesheet' CSS}
      CATALOG =
      case Syslet.args.'catalog' of unit then
	 case {OS.getEnv 'OZDOC_CATALOG'} of false then
	    OZDOC_HOME#'/catalog'
	 elseof X then X end
      elseof X then X end
      {Property.put 'ozdoc.catalog' CATALOG}
   in
      {OS.putEnv 'PATH' SBIN_PATH#':'#{OS.getEnv 'PATH'}}
      {OS.putEnv 'OZDOC_ELISP_PATH' ELISP_PATH}
   end
   % The actual translation
   try
      case Syslet.args.'in' of unit then
	 {Raise usage('no input file name specified')}
      elsecase Syslet.args.'out' of unit then
	 {Raise usage('no output directory name specified')}
      elsecase Syslet.args.2 of _|_ then
	 {Raise usage('unrecognized command line arguments')}
      elsecase Syslet.args.'type' of "html-color" then
	 {OzDocToHTML.translate color Syslet.args}
      elseof "html-mono" then
	 {OzDocToHTML.translate mono Syslet.args}
      elseof "html-stylesheets" then
	 {OzDocToHTML.translate stylesheets Syslet.args}
      else
	 {Raise usage('illegal output type specified')}
      end
      {Syslet.exit 0}
   catch E then
      case E of usage(M) then
	 {System.printError
	  'Command line option error: '#M#'\n'#
	  'Usage: '#{Property.get 'root.url'}#' [options]\n'#
	  '--in=<File>         The input SGML file.\n'#
	  '--type=<Type>       What format to generate\n'#
	  '                    (supported: '#
	  'html-color html-mono html-stylesheets).\n'#
	  '--out=<Dir>         The output directory.\n'#
	  '--(no)autoindex     Automatically generate index entries.\n'#
	  '\n'#
	  'HTML options\n'#
	  '--stylesheet=<URL>  What style sheet to use for generated pages.\n'#
	  '--(no)latexmath     Generate GIF files from LaTeX math.\n'#
	  '--(no)split         Split the document into several nodes.\n'#
	  '\n'#
	  'Parametrization\n'#
	  '--ozdoc-home=<DIR>  ozdoc installation directory.\n'#
	  '--author-path=<Search Path>\n'#
	  '--bib-path=<Search Path>\n'#
	  '--bst-path=<Search Path>\n'#
	  '--sbin-path=<Search Path>\n'#
	  '                    Where to look for author databases,\n'#
	  '                    bib files, bst files, and ozdoc scripts.\n'}
	 {Syslet.exit 2}
      else
	 {Error.printExc E}
	 {Syslet.exit 1}
      end
   end
end
