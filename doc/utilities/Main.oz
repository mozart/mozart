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
%%%    http://www.mozart-oz.org
%%%
%%% See the file "LICENSE" or
%%%    http://www.mozart-oz.org/LICENSE.html
%%% for information on usage and redistribution
%%% of this file, and for a DISCLAIMER OF ALL
%%% WARRANTIES.
%%%

functor
import
   Application(getCmdArgs exit)
   Property(get put)
   System(printError showInfo)
   Module(link)
   Narrator('class')
   ErrorListener('class')
   SGML(namePI parse)
   XML(namePI parse) at 'XMLOzdoc.ozf'
   OzDocToHTML(translate)
   OS(getEnv putEnv)
   URL
   Chunk(getChunk processSpecs)
   %% for html-global-index
   Indexer(makeSplitIndex)
   Gdbm at 'x-oz://contrib/gdbm'
   File(write)
   HTML(seq: SEQ pcdata: PCDATA toVirtualString)
prepare
   Spec = record('in'(single char: &i type: string optional: false)
		 'parser'(single type: string default: unit)
		 'xml'(alias:'parser'#xml)
		 'type'(single char: &t type: string optional: false
			validate: alt(when(chunk false) when(true true)))
		 'html'(alias: 'type'#"html-stylesheets")
		 'out'(single char: &o type: string optional: false
		       validate: alt(when(chunk false) when(true true)))
		 'autoindex'(rightmost type: bool default: false)
		 'chunk'(multiple type: string)
		 'chunk-sep'(single type: string default: "\t")
		 %% HTML options
		 'link'(multiple type: string default: nil)
		 'stylesheet'(single type: string default: unit)
		 'latextogif'(rightmost type: bool default: true)
		 'latexdb'(single type: string default: unit)
		 'split'(rightmost type: bool default: true)
		 'abstract'(rightmost type: bool default: false)
		 'keeppictures'(rightmost type: bool default: false)
		 'xrefdb'(single type: string default: unit)
		 'xrefdir'(single type: string default: unit)
		 'xreftree'(single type: string default: '../')
		 'indexdb'(single type: string default: unit)
		 'make-hhc'(single type: string default: unit)
		 %% Path names
		 'ozdoc-home'(single type: string default: unit)
		 'author-path'(single type: string default: unit)
		 'bib-path'(single type: string default: unit)
		 'bst-path'(single type: string default: unit)
		 'elisp-path'(single type: string default: unit)
		 'sbin-path'(single type: string default: unit)
		 'catalog'(single type: string default: unit)
		 'include'(multiple type: list(string) default: nil)
		)
define
   class MyListenerClass from ErrorListener.'class'
      attr Sync: unit
      meth init(O X)
	 Sync <- X
	 ErrorListener.'class', init(O ServeOne true)
      end
      meth ServeOne(M)
	 case M of done() then @Sync = unit
	 else skip
	 end
      end
   end

   try
      Args = {Application.getCmdArgs Spec}
      Reporter Sync
      MyNarrator = {New Narrator.'class' init(?Reporter)}
      MyListener = {New MyListenerClass init(MyNarrator Sync)}
   in
      {Reporter setLogPhases(true)}
      %% Process path name options and store results in ozdoc.* properties
      local
	 %% Determine the directory in which document source files are located:
	 SRC_DIR =
	 local
	    X    = Args.'in'
	    Url  = {URL.make X}
	    Path = {CondSelect Url path unit}
	 in
	    case Path
	    of unit then '.'
	    [] nil  then '.'
	    [] [_]  then
	       {URL.toString
		{AdjoinAt Url path
		 if {CondSelect Url absolute false}
		 then [nil] else ["."] end}}
	    elsecase {Reverse Path} of _|L then
	       {URL.toString {AdjoinAt Url path {Reverse L}}}
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
	       SRC_DIR#':'#{Property.get 'oz.home'}#'/share/elisp'
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
	       {Property.get 'oz.home'}#'/share/doc'
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
	 INCLUDE =
	 case Args.'include' of unit then
	    case {OS.getEnv 'OZDOC_INCLUDE'} of false then
	       nil
	    elseof X then {String.tokens X &:} end
	 elseof X then X end
	 {Property.put 'ozdoc.include' INCLUDE}
	 TEXINPUTS =
	 '.:'#
	 case {OS.getEnv 'OZDOC_TEXINPUTS'} of false then
	    SRC_DIR#':'
	 elseof X then X end
	 #':'#
	 case {OS.getEnv 'TEXINPUTS'} of false then nil
	 elseof X then X end
      in
	 {OS.putEnv 'PATH' SBIN_PATH#':'#{OS.getEnv 'PATH'}}
	 {OS.putEnv 'OZDOC_ELISP_PATH' ELISP_PATH}
	 {OS.putEnv 'EMACS_UNIBYTE' 'yes'}
	 {OS.putEnv 'TEXINPUTS' TEXINPUTS}
      end
      %% The actual translation
      case Args.1 of _|_ then
	 {Raise usage('extra command line arguments')}
      elsecase {CondSelect Args 'type' unit}
      of "html-global-index" then DocType Xs in
	 DocType = ('<!DOCTYPE html PUBLIC '#
		    '"-//W3C//DTD HTML 4.0 Transitional//EN">\n')
	 if
	    try DB in
	       DB = {Gdbm.new read(Args.'in')}
	       Xs = {Gdbm.entries DB}
	       {IsList Xs _}
	       {Gdbm.close DB}
	       true
	    catch _ then
	       false
	    end
	 then Table Pages Address HTML2 in
	    Table#Pages = {Indexer.makeSplitIndex
			   {FoldR Xs
			    fun {$ Prefix#(DocumentTitle#Entries) Rest}
			       {FoldR Entries
				fun {$ Ands#(RURL#SectionTitle)#_ Rest}
				   Ands#a(href: '../'#Prefix#'/'#RURL
					  SEQ([DocumentTitle PCDATA(', ')
					       SectionTitle]))|Rest
				end Rest}
			    end nil} Args.'out'}
	    Address = address(span('class':[version]
				   PCDATA('Version '#
					  {Property.get 'oz.version'}#
					  ' ('#{Property.get 'oz.date'}#
					  ')')))
	    HTML2 = html(head(title(PCDATA('Global Index'))
			      link(rel: stylesheet
				   type: 'text/css'
				   href: {Property.get 'ozdoc.stylesheet'}))
			 body(h1(PCDATA('Global Index'))
			      Table hr() Address))
	    {File.write DocType#{HTML.toVirtualString HTML2}#'\n'
	     Args.'out'#'/index.html'}
	    {ForAll Pages
	     proc {$ GroupName#Name#HTML1} HTML2 in
		HTML2 = html(head(title(PCDATA('Global Index - ') GroupName)
				  link(rel: stylesheet
				       type: 'text/css'
				       href:
					  {Property.get 'ozdoc.stylesheet'}))
			     'body'(h1(PCDATA('Global Index - ') GroupName)
				    Table HTML1 hr() Address))
		{File.write DocType#{HTML.toVirtualString HTML2}#'\n' Name}
	     end}
	 else HTML0 in
	    HTML0 = html(head(title(PCDATA('Empty Global Index')
				    link(rel: stylesheet
					 type: 'text/css'
					 href:
					    {Property.get 'ozdoc.stylesheet'}))
			      body(h1(PCDATA('Empty Global Index'))
				   p(PCDATA('Sorry.')))))
	    {File.write DocType#{HTML.toVirtualString HTML0}#'\n'
	     Args.'out'#'/index.html'}
	 end
      else SGMLParser SGMLNode in
	 {Reporter startBatch()}
	 {Reporter startPhase('parsing input file')}
	 SGML.namePI = {NewName}
	 SGMLParser = case Args.'parser' of unit then SGML.parse
		      [] xml then XML.namePI=SGML.namePI XML.parse
		      elseof ParserURL then Parser in
			 Parser = try
				     case {Module.link [ParserURL]} of [M] then
					{Wait M}
					M
				     end
				  catch _ then
				     {Exception.raiseError
				      ap(usage 'parser functor not found')}
				     unit
				  end
			 if {HasFeature Parser namePI}
			    andthen {HasFeature Parser parse}
			    andthen {IsProcedure Parser.parse}
			    andthen {Procedure.arity Parser.parse} == 3
			 then
			    Parser.namePI = SGML.namePI
			    Parser.parse
			 else
			    {Exception.raiseError
			     ap(usage 'parser functor has bad signature')} unit
			 end
		      end
	 SGMLNode = {SGMLParser Args.'in' Reporter}
	 if {Reporter hasSeenError($)} then skip
	 elseif {HasFeature Args 'chunk'} then
	    {Chunk.processSpecs Reporter SGMLNode Args.'chunk'
	     if Args.'chunk-sep'==nil then &\t else Args.'chunk-sep'.1 end}
	 elsecase Args.'type' of "html-color" then
	    {OzDocToHTML.translate Reporter color SGMLNode Args}
	 elseof "html-mono" then
	    {OzDocToHTML.translate Reporter mono SGMLNode Args}
	 elseof "html-stylesheets" then
	    {OzDocToHTML.translate Reporter stylesheets SGMLNode Args}
	 elseof "chunk" then
	    {System.showInfo {Chunk.getChunk Reporter SGMLNode Args.'out'}}
	 else
	    {Raise usage('illegal output type specified')}
	 end
	 {Reporter tell(done())}
	 {Wait Sync}
      end
      if {MyListener hasErrors($)} then
	 {Application.exit 1}
      else
	 {Application.exit 0}
      end
   catch error(ap(usage M) ...) then
      {System.printError
       'Command line option error: '#M#'\n'#
       'Usage: '#{Property.get 'application.url'}#' [options]\n'#
       '--in=<File>         The input file; typically SGML.\n'#
       '--parser=<URL>      Specify a functor used to parse the input.\n'#
       '--type=<Type>       What output to generate\n'#
       '                    (supported: '#
       'html-color html-mono html-stylesheets\n'#
       '                    html-global-index chunk).\n'#
       '--out=<Dir>         The output directory.\n'#
       '--(no)autoindex     Automatically generate index entries.\n'#
       '--include=A1,...,An Assume `<!ENTITY & Ai "INCLUDE">\'.\n'#
       '\n'#
       'HTML options\n'#
       '--link=<Text>,<RelURL>\n'#
       '                    Include a link in the margin of each page.\n'#
       '--stylesheet=<RURL> What style sheet to use for generated pages.\n'#
       '--(no)latextogif    Generate GIF files from LaTeX code.\n'#
       '--latexdb=<File>    Reuse GIFs generated from LaTeX code.\n'#
       '--(no)split         Split the document into several nodes.\n'#
       '--(no)abstract      Generate an abstract.html auxiliary file.\n'#
       '--keeppictures      Do no recreate GIF from PS if already there.\n'#
       '\n'#
       'Inter-Document Cross-Referencing\n'#
       '--xrefdb=<File>     Where to look up resp. store references.\n'#
       '--xrefdir=<RelURL>  Where this document goes relative to the\n'#
       '                    whole documentation installation directory.\n'#
       '--xreftree=<RelURL> How to get to whole doc installation from\n'#
       '                    the directory where this document goes.\n'#
       '--indexdb=<File>    Where to look up resp. store index entries.\n'#
       '--make-hhc=<File>   Where to write a HTML Help contents file.\n'#
       '\n'#
       'Parametrization\n'#
       '--ozdoc-home=<DIR>  ozdoc installation directory.\n'#
       '--author-path=<Search Path>\n'#
       '--bib-path=<Search Path>\n'#
       '--bst-path=<Search Path>\n'#
       '--sbin-path=<Search Path>\n'#
       '                    Where to look for author databases,\n'#
       '                    bib files, bst files, and ozdoc scripts.\n'#
       '--catalog=<File>    Specify the catalog file to use for parsing.\n'}
      {Application.exit 2}
   [] error then
      {Application.exit 1}
   end
end
