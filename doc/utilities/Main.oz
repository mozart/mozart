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
   Application(getCmdArgs exit)
   Property(get put)
   System(printError)
   Error(printExc)
   OzDocToHTML(translate)
   OS(getEnv putEnv)
   URL
define
   Args = {Application.getCmdArgs
           record('in'(single char: &i type: string optional: false)
                  'type'(single char: &t type: string optional: false)
                  'html'(alias: 'type'#"html-stylesheets")
                  'out'(single char: &o type: string optional: false)
                  'autoindex'(single type: bool default: false)
                  % HTML options
                  'stylesheet'(single type: string default: unit)
                  'latexmath'(rightmost type: bool default: true)
                  'split'(rightmost type: bool default: true)
                  'abstract'(rightmost type: bool default: false)
                  % Path names
                  'ozdoc-home'(single type: string default: unit)
                  'author-path'(single type: string default: unit)
                  'bib-path'(single type: string default: unit)
                  'bst-path'(single type: string default: unit)
                  'elisp-path'(single type: string default: unit)
                  'sbin-path'(single type: string default: unit)
                  'catalog'(single type: string default: unit)
                 )}
   % Process path name options and store results in ozdoc.* properties
   local
      % Determine the directory in which document source files are located:
      SRC_DIR =
      local
         X    = Args.'in'
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
      case Args.'ozdoc-home' of unit then
         case {OS.getEnv 'OZDOC_HOME'} of false then
            {Property.get 'oz.home'}#'/share/doc'
         elseof X then X end
      elseof X then X end
      {Property.put 'ozdoc.home' OZDOC_HOME}
      AUTHOR_PATH =
      case Args.'author-path' of unit then
         case {OS.getEnv 'OZDOC_AUTHOR_PATH'} of false then
            SRC_DIR#':'#OZDOC_HOME
         elseof X then X end
      elseof X then X end
      {Property.put 'ozdoc.author.path' AUTHOR_PATH}
      BIB_PATH =
      case Args.'bib-path' of unit then
         case {OS.getEnv 'OZDOC_BIB_PATH'} of false then
            SRC_DIR#':'#OZDOC_HOME
         elseof X then X end
      elseof X then X end
      {Property.put 'ozdoc.bib.path' BIB_PATH}
      BST_PATH =
      case Args.'bst-path' of unit then
         case {OS.getEnv 'OZDOC_BST_PATH'} of false then
            BIB_PATH
         elseof X then X end
      elseof X then X end
      {Property.put 'ozdoc.bst.path' BST_PATH}
      ELISP_PATH =
      case Args.'elisp-path' of unit then
         case {OS.getEnv 'OZDOC_ELISP_PATH'} of false then
            {Property.get 'oz.home'}#'/share/elisp'
         elseof X then X end
      elseof X then X end
      {Property.put 'ozdoc.elisp.path' ELISP_PATH}
      SBIN_PATH =
      case Args.'sbin-path' of unit then
         case {OS.getEnv 'OZDOC_SBIN_PATH'} of false then
            OZDOC_HOME
         elseof X then X end
      elseof X then X end
      {Property.put 'ozdoc.sbin.path' SBIN_PATH}
      CSS =
      case Args.'stylesheet' of unit then
         case {OS.getEnv 'OZDOC_STYLESHEET'} of false then
            'http://www.ps.uni-sb.de/css/ozdoc.css'
         elseof X then X end
      elseof X then X end
      {Property.put 'ozdoc.stylesheet' CSS}
      CATALOG =
      case Args.'catalog' of unit then
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
      case Args.1 of _|_ then
         {Raise usage('extra command line arguments')}
      elsecase Args.'type' of "html-color" then
         {OzDocToHTML.translate color Args}
      elseof "html-mono" then
         {OzDocToHTML.translate mono Args}
      elseof "html-stylesheets" then
         {OzDocToHTML.translate stylesheets Args}
      else
         {Raise usage('illegal output type specified')}
      end
      {Application.exit 0}
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
          '--(no)abstract      Generate an abstract.html auxiliary file.\n'#
          '\n'#
          'Parametrization\n'#
          '--ozdoc-home=<DIR>  ozdoc installation directory.\n'#
          '--author-path=<Search Path>\n'#
          '--bib-path=<Search Path>\n'#
          '--bst-path=<Search Path>\n'#
          '--sbin-path=<Search Path>\n'#
          '                    Where to look for author databases,\n'#
          '                    bib files, bst files, and ozdoc scripts.\n'}
         {Application.exit 2}
      [] error then
         {Application.exit 1}
      else
         {Error.printExc E}
         {Application.exit 1}
      end
   end
end
