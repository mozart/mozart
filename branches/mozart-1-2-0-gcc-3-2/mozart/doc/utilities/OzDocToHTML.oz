%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Contributors:
%%%   Tobias Mueller <tmueller@ps.uni-sb.de>
%%%   Christian Schulte <schulte@ps.uni-sb.de>
%%%
%%% Copyright:
%%%   Leif Kornstaedt, 1998
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
   %% System Modules
   Property(get)
   OS(system)
   Narrator('class')
   ErrorListener('class')
   %% Application Modules
   SGML(parse namePI getSubtree isOfClass)
   AuthorDB('class')
   BibliographyDB('class')
   Indexer('class')
   Fontifier('class' noProgLang)
   CrossReferencer('class')
   Thumbnails('class')
   LaTeXToGIF('class')
   PostScriptToGIF('class')
   File(read: ReadFile write: WriteFile)
   HTML(empty: EMPTY
	seq: SEQ
	common: COMMON
	block: BLOCK
	pcdata: PCDATA
	verbatim: VERBATIM
	toVirtualString clean)
export
   Translate
define
   DOCTYPE_PUBLIC = '"-//W3C//DTD HTML 4.0 Transitional//EN"'

   OzDocError = 'ozdoc to html error'
   OzDocWarning = 'ozdoc to html warning'

   fun {CollapseSpaces S DropSpace}
      case S of C|Cr then
	 if {Char.isSpace C} then
	    if DropSpace then {CollapseSpaces Cr true}
	    else & |{CollapseSpaces Cr true}
	    end
	 else C|{CollapseSpaces Cr false}
	 end
      [] nil then ""
      end
   end

   fun {InitialCapital S}
      case S of C|Cr then {Char.toUpper C}|Cr
      [] nil then ""
      end
   end

   local
      RomanN = romanN(&I &V &X &L &C &D &M)
      RomanC = romanC(1 5 10 50 100 500 1000)

      fun {Roman X I}
	 if X > 0 then N in
	    N = X div RomanC.I
	    case N of 0 then ""
	    [] 1 then [RomanN.I]
	    [] 2 then [RomanN.I RomanN.I]
	    [] 3 then [RomanN.I RomanN.I RomanN.I]
	    [] 4 then [RomanN.I RomanN.(I + 1)]
	    [] 5 then [RomanN.(I + 1)]
	    [] 6 then [RomanN.(I + 1) RomanN.I]
	    [] 7 then [RomanN.(I + 1) RomanN.I RomanN.I]
	    [] 8 then [RomanN.(I + 1) RomanN.I RomanN.I RomanN.I]
	    [] 9 then [RomanN.I RomanN.(I + 2)]
	    end#{Roman (X mod RomanC.I) I - 2}
	 else ""
	 end
      end
   in
      fun {RomanU X}
	 case X >= 0 andthen X < 4000 of true then {Roman X 7} end
      end
   end

   fun {Alpha X}
      case X >= 1 andthen X =< 26 of true then [&A + X - 1] end
   end

   local
      fun {Class MakeHHC}
	 case MakeHHC of unit then [toc]
	 else nil
	 end
      end

      fun {Object Link Text MakeHHC}
	 case MakeHHC of unit then a(href: Link Text)
	 else
	    object(type: 'text/sitemap'
		   param(name: 'Name'
			 value: {HTML.toVirtualString {HTML.clean Text}})
		   param(name: 'Local' value: MakeHHC#'/'#Link))
	 end
      end

      fun {MakeLIs LIs}
	 case LIs of li(T)|(Y=ul(...))|Xr then {MakeLIs li(SEQ([T Y]))|Xr}
	 elseof X|Xr then X|{MakeLIs Xr}
	 [] nil then nil
	 end
      end

      fun {FormatTOCLevel TOC Level LIs1 LIss MakeHHC}
	 case TOC of Entry|TOCr then N#Label#Node#Text = Entry in
	    if N < Level then
	       case LIss of LIs2|LIsr then
		  {FormatTOCLevel TOC Level - 1
		   ul('class': {Class MakeHHC}
		      SEQ({MakeLIs {Reverse LIs1}}))|LIs2 LIsr MakeHHC}
	       [] nil then
		  {FormatTOCLevel TOC Level - 1
		   [ul('class': {Class MakeHHC}
		       SEQ({MakeLIs {Reverse LIs1}}))] nil MakeHHC}
	       end
	    elseif N > Level then
	       {FormatTOCLevel TOC Level + 1 nil LIs1|LIss MakeHHC}
	    else
	       {FormatTOCLevel TOCr Level
		li({Object Node#"#"#Label Text MakeHHC})|LIs1 LIss MakeHHC}
	    end
	 [] nil then
	    {FoldL LIs1|LIss
	     fun {$ In LIs} LIs1 in
		LIs1 = case In of unit then LIs else In|LIs end
		ul('class': {Class MakeHHC}
		   SEQ({MakeLIs {Reverse LIs1}}))
	     end unit}
	 end
      end
   in
      fun {FormatTOC TOC Depth MakeHHC}
	 case TOC of Entry|_ then N#_#_#_ = Entry NewTOC in
	    NewTOC = case Depth of ~1 then TOC
		     else {Filter TOC fun {$ M#_#_#_} M < N + Depth end}
		     end
	    {FormatTOCLevel NewTOC N nil nil MakeHHC}
	 [] nil then EMPTY
	 end
      end
   end

   fun {Spaces N}
      if N =< 0 then ""
      else & |{Spaces N - 1}
      end
   end

   fun {TransformAnds M P}
      {Record.mapInd M
       fun {$ F X}
	  if {IsInt F} andthen {Label X} == and then
	     {Record.mapInd X
	      fun {$ F Y} if {IsInt F} then {P Y} else Y end end}
	  else X
	  end
       end}
   end

   local
      fun {GetNext Ts N}
	 case Ts of T|Tr then
	    case T of nav(_) then
	       if N == 0 then sect(Node Label)|_ = Tr in Node#"#"#Label
	       elseif N < 0 then unit
	       else {GetNext Tr N}
	       end
	    [] sect(_ _) then {GetNext Tr N}
	    [] up then
	       {GetNext Tr N - 1}
	    [] down(_) then {GetNext Tr N + 1}
	    end
	 [] nil then unit
	 end
      end
   in
      proc {DoThreading Ts Prev Ups}
	 case Ts of T|Rest then
	    case T of nav(HTML) then Navs in
	       Navs = [case Prev of unit then EMPTY
		       else td(a(href: Prev PCDATA('<< Prev')))
		       end
		       case Ups of nil then EMPTY
		       elseof Node#_|_ then
			  td(a(href: Node PCDATA('- Up -')))
		       end
		       case {GetNext Rest 0} of unit then EMPTY
		       elseof To then td(a(href: To PCDATA('Next >>')))
		       end]
	       HTML = if {All Navs fun {$ Nav} Nav == EMPTY end} then EMPTY
		      else
			 table('class': [nav] align: center border: 0
			       cellpadding: 6 cellspacing: 6
			       tr(bgcolor: '#DDDDDD' SEQ(Navs)))
		      end
	       {DoThreading Rest unit Ups}
	    [] sect(Node Label) then NewPrev in
	       NewPrev = case Prev of unit then Node#"#"#Label
			 else Prev
			 end
	       {DoThreading Rest NewPrev Ups}
	    [] down(Node) then
	       {DoThreading Rest unit Node#Prev|Ups}
	    [] up then
	       case Rest of down(_)|Rest2 then
		  {DoThreading Rest2 Prev Ups}
	       else _#OldPrev|Upr = Ups in
		  {DoThreading Rest OldPrev Upr}
	       end
	    end
	 elseof nil then skip
	 end
      end
   end

   local
      fun {TheBetter C1 C2}
	 case C1.type#C2.type
	 of _#'HTML' then C2
	 [] 'HTML'#_ then C1
	 [] _#'LATEX' then C2
	 [] 'LATEX'#_ then C1
	 else C2 end
      end
      fun {Loop M I C}
	 if {HasFeature M I} then
	    {Loop M I+1 {TheBetter M.I C}}
	 else C end
      end
   in
      fun {PickMathChoice M}
	 {Loop M 2 M.1}
      end
   end

   class OzDocToHTML from Narrator.'class'
      attr
	 Reporter: unit
	 % fontification:
	 FontifyMode: unit
	 MyFontifier: unit
	 % constructing the output:
	 OutputDirectory: unit
	 CurrentNode: unit NavigationPanel: unit NodeCounter: unit
	 ToWrite: unit
	 Split: unit
	 MakeAbstract: unit
	 Threading: unit
	 SomeSplit: unit
	 % managing common attributes:
	 Common: unit BodyCommon: unit
	 ProgLang: unit
	 % front matter:
	 TopLinks: unit
	 TopTitle: unit
	 MyAuthorDB: unit
	 Authors: unit
	 Meta: unit
	 Comic: unit
	 Abstract: unit
	 StyleSheet: unit
	 % main matter:
	 TOC: unit TOCMode: unit WholeTOC: unit
	 Part: unit Chapter: unit Section: unit SubSection: unit
	 Appendix: unit
	 DefaultNodes: unit Labels: unit ToGenerate: unit
	 % for Math and Math.Choice:
	 MathDisplay: unit
	 MyLaTeXToGIF: unit
	 % for Picture:
	 MyPostScriptToGIF: unit
	 PictureDisplay: unit
	 MyThumbnails: unit
	 % for Figure:
	 Floats: unit
	 FigureCounters: unit
	 % for Note:
	 FootNotes: unit
	 % for Grammar:
	 GrammarIsCompact: unit
	 GrammarHead: unit
	 GrammarAltType: unit
	 GrammarNote: unit
	 % for Chunk and Chunk.Ref:
	 ChunkDefinitions: unit ChunkLinks: unit
	 % for Ref.Extern and Ptr.Extern:
	 MyCrossReferencer: unit
	 XRefDir: unit
	 IndexDBName: unit
	 MakeHHC: unit
	 % for Table:
	 TableCols: unit
	 TableRow: unit
	 CurTableCols: unit
	 % for List:
	 ListType: unit
	 OLTypes: (X = '1'|'a'|'i'|'A'|'I'|X in X)
	 % back matter:
	 Exercises: unit Answers: unit
	 MyBibliographyDB: unit
	 BibNode: unit
	 MyIndexer: unit
	 IdxNode: unit
	 IndexSortAs: unit
	 WhereNow: unit
	 AutoIndex: unit
      meth init()
	 Reporter <- Narrator.'class', init($)
	 {@Reporter setLogPhases(true)}
	 Meta <- {NewDictionary}
      end
      meth translate(Mode Args) SGMLNode in
	 {@Reporter startBatch()}
	 {@Reporter startPhase('parsing SGML input')}
	 try
	    SGMLNode = {SGML.parse Args.'in' @Reporter}
	    if {@Reporter hasSeenError($)} then skip
	    else N in
	       FontifyMode <- Mode
	       TopLinks <- {Map Args.'link'
			    fun {$ S} Text Rest in
			       {List.takeDropWhile S
				fun {$ C} C \= &, end ?Text ?Rest}
			       case Rest of &,|URL then URL#Text
			       else
				  {Exception.raiseError
				   ap(usage
				      'comma expected in argument to `link\'')}
				  unit
			       end
			    end}
	       StyleSheet <- {Property.get 'ozdoc.stylesheet'}
	       MyFontifier <- {New Fontifier.'class' init(@Meta)}
	       OutputDirectory <- Args.'out'
	       {OS.system "mkdir -p "#@OutputDirectory _}   %--** OS.mkDir
	       MyThumbnails <- {New Thumbnails.'class' init(@OutputDirectory)}
	       MyLaTeXToGIF <- if Args.'latextogif' then
				  {New LaTeXToGIF.'class'
				   init(@OutputDirectory Args.'latexdb')}
			       else unit
			       end
	       MyPostScriptToGIF <- {New PostScriptToGIF.'class'
				     init(@OutputDirectory
					  Args.'keeppictures')}
	       MyCrossReferencer <- {New CrossReferencer.'class'
				     init(Args.'xrefdir' Args.'xreftree'
					  Args.'xrefdb' @Reporter)}
	       XRefDir <- Args.'xrefdir'
	       IndexDBName <- Args.'indexdb'
	       MakeHHC <- Args.'make-hhc'
	       CurrentNode <- 'index.html'
	       NavigationPanel <- N
	       NodeCounter <- 0
	       ToWrite <- nil
	       Split <- Args.'split'
	       MakeAbstract <- Args.'abstract'
	       SomeSplit <- false
	       Threading <- [nav(N)]
	       ProgLang <- Fontifier.noProgLang
	       DefaultNodes <- {NewDictionary}
	       Labels <- {NewDictionary}
	       ToGenerate <- nil
	       AutoIndex <- Args.'autoindex'
	       {@Reporter startPhase('translating to HTML')}
	       OzDocToHTML, Process(SGMLNode unit)
	    end
	    if {@Reporter hasSeenError($)} then skip
	    else
	       {@Reporter startSubPhase('adding navigation panels')}
	       {DoThreading {Reverse @Threading} unit nil}
	       {@Reporter startSubPhase('generating cross-reference labels')}
	       OzDocToHTML, GenerateLabels()
	       {ForAll {Dictionary.entries @Labels}
		proc {$ L#(N#T)}
		   if {IsFree N} then
		      N = {Dictionary.condGet @DefaultNodes L
			   'file:///dev/null'}
		      T = PCDATA('*')
		   end
		end}
	       {@Reporter startSubPhase('writing output files')}
	       {ForAll @ToWrite
		proc {$ DocType#Node#File}
		   {WriteFile DocType#{HTML.toVirtualString Node}#'\n' File}
		end}
	       case @MakeHHC of unit then skip
	       elseof S then
		  {@Reporter startSubPhase('writing HHC file')}
		  {WriteFile {HTML.toVirtualString
			      {FormatTOC @TOC ~1 @XRefDir}}#'\n' S}
	       end
	    end
	 catch tooManyErrors then
	    {@Reporter
	     tell(info('%** Too many errors, aborting compilation\n'))}
	 finally
	    case @MyCrossReferencer of unit then skip
	    elseof O then {O close()}
	    end
	 end
	 if {@Reporter hasSeenError($)} then
	    {@Reporter endBatch(rejected)}
	 else
	    {@Reporter endBatch(accepted)}
	 end
	 {@Reporter tell(done())}
      end
      meth PushCommon(M OldCommon) ID Class in
	 OldCommon = @ProgLang#@Common
	 case {CondSelect M proglang unit} of unit then skip
	 elseof X then
	    ProgLang <- X
	 end
	 ID = {CondSelect M id unit}
	 Class = {CondSelect M 'class' unit}
	 Common <- if ID == unit andthen Class == unit then COMMON
		   elseif ID == unit then COMMON('class': Class)
		   elseif Class == unit then COMMON(id: ID)
		   else COMMON(id: ID 'class': Class)
		   end
      end
      meth PopCommon(OldCommon)
	 ProgLang <- OldCommon.1
	 Common <- OldCommon.2
      end
      meth Batch(M I $)
	 SEQ(OzDocToHTML, BatchSub(M I $))
      end
      meth BatchSub(M I $)
	 if {HasFeature M I} then
	    case M.I of S=_|_ then
	       PCDATA({CollapseSpaces S false})|
	       OzDocToHTML, BatchSub(M I + 1 $)
	    [] nil then
	       OzDocToHTML, BatchSub(M I + 1 $)
	    elseof N then
	       OzDocToHTML, Process(N $)|OzDocToHTML, BatchSub(M I + 1 $)
	    end
	 else nil
	 end
      end
      meth BatchCode(M I ?HTML) Request in
	 OzDocToHTML, BatchCodeSub(M I ?Request ?HTML)
	 {@MyFontifier enqueueRequest(@ProgLang complex(Request))}
      end
      meth BatchCodeSub(M I ?Request ?HTML)
	 if {HasFeature M I} then
	    case M.I of S=_|_ then HTML1 Rr HTML2 in
	       Request = simple(S HTML1)|Rr
	       HTML = SEQ([code(HTML1) HTML2])
	       OzDocToHTML, BatchCodeSub(M I + 1 ?Rr ?HTML2)
	    [] nil then
	       OzDocToHTML, BatchCodeSub(M I + 1 ?Request ?HTML)
	    elseof N then
	       case {Label N} of span then X R1 HTML1 Rr HTML2 in
		  OzDocToHTML, PushCommon(N ?X)
		  OzDocToHTML, BatchCodeSub(N 1 ?R1 ?HTML1)
		  HTML = SEQ([span(COMMON: @Common HTML1) HTML2])
		  OzDocToHTML, PopCommon(X)
		  Request = complex(R1)|Rr
		  OzDocToHTML, BatchCodeSub(M I + 1 ?Rr ?HTML2)
	       else Rr HTML1 HTML2 in
		  Request = simple(' ' _)|Rr   %--** insert a variable?
		  OzDocToHTML, Process(N ?HTML1)
		  HTML = SEQ([HTML1 HTML2])
		  OzDocToHTML, BatchCodeSub(M I + 1 ?Rr ?HTML2)
	       end
	    end
	 else
	    Request = nil
	    HTML = EMPTY
	 end
      end
      meth Process(M $)
	 Tag = case {Label M} of sect0 then chapter
	       [] sect1 then section
	       [] sect2 then subsection
	       [] sect3 then subsubsection
	       [] 'p.silent' then p
	       [] 'p.level' then p
	       elseof X then X
	       end
	 OldCommon
	 Res
      in
	 case {CondSelect M id unit} of unit then skip
	 elseof I then
	    {Dictionary.put @DefaultNodes I @CurrentNode}
	 end
	 OzDocToHTML, PushCommon(M ?OldCommon)
	 %--------------------------------------------------------------
	 % Processing Instructions
	 %--------------------------------------------------------------
	 if Tag == SGML.namePI then
	    case M.1 of emdash then
	       VERBATIM(' - ')
	    [] endash then
	       VERBATIM('-')
	    [] nbsp then
	       VERBATIM('&nbsp;')
	    [] 'PI:NBSP' then
	       VERBATIM('&nbsp;')
	    [] ellipsis then
	       VERBATIM('...')
	    [] cdots then
	       VERBATIM('&middot;&middot;&middot;')
	    [] slash then
	       VERBATIM('/')
	    [] ie then
	       VERBATIM('i.&nbsp;e.')
	    [] wrt then
	       VERBATIM('w.&nbsp;r.&nbsp;t.')
	    [] eg then
	       VERBATIM('e.&nbsp;g.')
	    [] resp then
	       VERBATIM('resp.')
	    [] etc then
	       VERBATIM('etc.')
	    [] 'LaTeX' then
	       VERBATIM('LaTeX')
	    [] 'PI:LATEX' then
	       VERBATIM('LaTeX')
	    [] 'PI:EG' then
	       VERBATIM('e.&nbsp;g.')
	    else
	       {@Reporter error(kind: OzDocError
				msg: 'unsupported processing instruction'
				items: [hint(l: 'Found' m: M.1)])}
	       unit
	    end
	 else
	    %-----------------------------------------------------------
	    % Document Structure
	    %-----------------------------------------------------------
	    case Tag of book then HTM0 TopTOC IndexHTML in
	       Floats <- nil
	       FootNotes <- nil
	       FigureCounters <- {NewDictionary}
	       Exercises <- nil
	       Answers <- {NewDictionary}
	       MyBibliographyDB <- {New BibliographyDB.'class'
				    init(@OutputDirectory)}
	       BibNode <- _
	       MyIndexer <- {New Indexer.'class' init()}
	       IdxNode <- _
	       WhereNow <- nil
	       TOC <- nil
	       TOCMode <- false
	       ChunkDefinitions <- {NewDictionary}
	       ChunkLinks <- nil
	       HTM0 = [OzDocToHTML, Process(M.1=front(...) $)
		       if {HasFeature M 3} then
			  OzDocToHTML, Process(M.3=back(...) $)
		       else EMPTY
		       end
		       if @Split
			  andthen {Dictionary.member @Meta 'html.split.toc'}
		       then Title WhereBefore Label X HTML1 HTML in
			  Title = PCDATA('Table of Contents')
			  WhereBefore = @WhereNow
			  WhereNow <- Title|WhereBefore
			  OzDocToHTML, PrepareTOCNode(?X ?HTML1)
			  ToGenerate <- Label|@ToGenerate
			  TOC <- {Append @TOC [2#Label#@CurrentNode#Title]}
			  Threading <- sect(@CurrentNode Label)|@Threading
			  WholeTOC <- _
			  HTML = SEQ([HTML1
				      h1(a(name: Label Title)) @WholeTOC])
			  WhereNow <- WhereBefore
			  OzDocToHTML, FinishNode(Title X HTML $)
		       else EMPTY
		       end
		       TopTOC
		       OzDocToHTML, Process(M.2='body'(...) $)
		       case @Exercises of nil then EMPTY
		       else Title WhereBefore Label X HTML1 HTML in
			  {@Reporter startSubPhase('generating answers')}
			  Title = PCDATA('Answers to the Exercises')
			  WhereBefore = @WhereNow
			  WhereNow <- Title|WhereBefore
			  OzDocToHTML, PrepareAnswersNode(?X ?HTML1)
			  ToGenerate <- Label|@ToGenerate
			  TOC <- {Append @TOC [2#Label#@CurrentNode#Title]}
			  Threading <- sect(@CurrentNode Label)|@Threading
			  HTML = SEQ([HTML1
				      h1(a(name: Label Title))
				      OzDocToHTML, OutputAnswers($)])
			  WhereNow <- WhereBefore
			  OzDocToHTML, FinishNode(Title X HTML $)
		       end
		       case {@MyBibliographyDB process(@Reporter $)}
		       of unit then EMPTY
		       elseof VS then Title WhereBefore Label X HTML1 HTML in
			  {@Reporter startSubPhase('generating bibliography')}
			  Title = PCDATA('Bibliography')
			  WhereBefore = @WhereNow
			  WhereNow <- Title|WhereBefore
			  OzDocToHTML, PrepareBibNode(?X ?HTML1)
			  ToGenerate <- Label|@ToGenerate
			  TOC <- {Append @TOC [2#Label#@CurrentNode#Title]}
			  @BibNode = @CurrentNode
			  Threading <- sect(@CurrentNode Label)|@Threading
			  HTML = SEQ([HTML1
				      h1(a(name: Label Title))
				      VERBATIM(VS)])   %--** VERBATIM?
			  WhereNow <- WhereBefore
			  OzDocToHTML, FinishNode(Title X HTML $)
		       end
		       IndexHTML]
	       OzDocToHTML, MakeNode(@TopTitle SEQ(HTM0))
	       {@Reporter startSubPhase('fontifying code')}
	       {@MyFontifier process(case @FontifyMode
				     of color then 'html-color'
				     [] mono then 'html-mono'
				     [] stylesheets then 'html-stylesheets'
				     end)}
	       IndexHTML = if {@MyIndexer empty($)} then EMPTY
			   else Title WhereBefore Label X HTML1 HTML in
			      {@Reporter startSubPhase('generating index')}
			      Title = PCDATA('Index')
			      WhereBefore = @WhereNow
			      WhereNow <- Title|WhereBefore
			      OzDocToHTML, PrepareIdxNode(?X ?HTML1)
			      ToGenerate <- Label|@ToGenerate
			      TOC <- {Append @TOC [2#Label#@CurrentNode#Title]}
			      @IdxNode = @CurrentNode
			      Threading <- sect(@CurrentNode Label)|@Threading
			      HTML = SEQ([HTML1
					  h1(a(name: Label Title))
					  {@MyIndexer
					   process(@IndexDBName @XRefDir
						   @TopTitle $)}])
			      WhereNow <- WhereBefore
			      OzDocToHTML, FinishNode(Title X HTML $)
			   end
	       TopTOC = if @SomeSplit then EMPTY
			else SEQ([hr() {FormatTOC @TOC ~1 unit} hr()])
			end
	       if {IsFree @WholeTOC} then
		  @WholeTOC = SEQ([hr() {FormatTOC @TOC ~1 unit} hr()])
	       end
	       case @MyLaTeXToGIF of unit then skip
	       elseof O then
		  {O process({Dictionary.condGet @Meta 'latex.package' nil}
			     {Dictionary.condGet @Meta 'latex.input' nil}
			     @Reporter)}
	       end
	       {@MyThumbnails process(@Reporter)}
	       {ForAll @ChunkLinks
		proc {$ Name#LinkedTitle}
		   LinkedTitle =
		   case {Dictionary.condGet @ChunkDefinitions Name unit}
		   of unit then VS in
		      VS = {Atom.toString Name}
		      {@Reporter warn(kind: OzDocWarning
				      msg: 'Reference to undefined chunk'
				      items: [hint(l: 'Chunk title' m: VS)])}
		      PCDATA(Name)
		   elseof Tos then a(href: {List.last Tos}.1 PCDATA(Name))
		   end
		end}
	       {ForAll {Dictionary.items @ChunkDefinitions}
		proc {$ Tos}
		   {FoldLTail Tos
		    fun {$ Next To#ChunkNav|Rest}
		       ChunkNav = SEQ([case Rest of Prev#_|_ then
					  SEQ([PCDATA(' ')
					       a(href: Prev PCDATA('<<'))])
				       [] nil then EMPTY
				       end
				       case Next of unit then EMPTY
				       else
					  SEQ([PCDATA(' ')
					       a(href: Next PCDATA('>>'))])
				       end])
		       To
		    end unit _}
		end}
	       unit
	    %-----------------------------------------------------------
	    % Front and Back Matter
	    %-----------------------------------------------------------
	    [] front then HTML in
	       Authors <- nil
	       MyAuthorDB <- {New AuthorDB.'class' init(@Reporter)}
	       {Dictionary.removeAll @Meta}
	       OzDocToHTML, Batch(M 1 ?HTML)
	       if @MakeAbstract then
		  Node = 'div'(hr()
			       case @TopTitle of unit then EMPTY
			       elseof T then h2(T)
			       end
			       case @Authors of nil then EMPTY
			       else
				  span('class': [subtitle]
				       OzDocToHTML, FormatAuthors(@Authors $))
			       end
			       case @Abstract of unit then EMPTY
			       elseof A then
				  A
			       end)
	       in
		  ToWrite <- ''#Node#'abstract.html'|@ToWrite
	       end
	       'div'(COMMON: @Common
		     case @TopLinks of URL#Text|Rest then
			p('class': [margin]
			  SEQ(a(href: URL PCDATA(Text))|
			      {FoldR Rest
			       fun {$ URL#Text In}
				  br()|a(href: URL PCDATA(Text))|In
			       end nil}))
		     [] nil then EMPTY
		     end
		     case @TopTitle of unit then EMPTY
		     elseof T then h1(align: center 'class': [title] T)
		     end
		     HTML
		     case @Authors of nil then EMPTY
		     else
			h2(align: center 'class': [authors]
			   OzDocToHTML, FormatAuthors(@Authors $))
		     end
		     case @Comic of unit then EMPTY
		     elseof M then p(OzDocToHTML, Process(M $))
		     end
		     case @Abstract of unit then EMPTY
		     elseof A then blockquote(A)
		     end)
	    [] title then
	       TopTitle <- span(COMMON: @Common OzDocToHTML, Batch(M 1 $))
	       {@MyCrossReferencer
		putTop(SEQ([PCDATA('``') @TopTitle PCDATA('\'\'')]))}
	       EMPTY
	    [] author then
	       Authors <- {Append @Authors [OzDocToHTML, MakeAuthor(M $)]}
	       EMPTY
	    [] 'author.extern' then
	       Authors <- {Append @Authors [OzDocToHTML, MakeAuthor(M $)]}
	       EMPTY
	    [] meta then
	       if {HasFeature M value} then
		  if {HasFeature M arg1} orelse {HasFeature M arg2} then
		     {@Reporter error(kind: OzDocError
				      msg: 'illegal meta information'
				      items: [hint(l: 'Node' m: oz(M))])}
		  else
		     {Dictionary.put @Meta M.name
		      {Append {Dictionary.condGet @Meta M.name nil}
		       [{String.toAtom M.value}]}}
		  end
	       else
		  if {HasFeature M arg1} andthen {HasFeature M arg2} then
		     {Dictionary.put @Meta M.name
		      {Append {Dictionary.condGet @Meta M.name nil}
		       [{String.toAtom M.arg1}#{String.toAtom M.arg2}]}}
		  else
		     {@Reporter error(kind: OzDocError
				      msg: 'illegal meta information'
				      items: [hint(l: 'Node' m: oz(M))])}
		  end
	       end
	       EMPTY
	    [] abstract then
	       Abstract <- span(COMMON: @Common
				OzDocToHTML, Batch(M 1 $))
	       EMPTY
	    [] back then
	       OzDocToHTML, Batch(M 1 $)
	    [] 'bib.extern' then BibKey in
	       {@MyBibliographyDB get(M.to M.key ?BibKey)}
	       case {CondSelect M id unit} of unit then skip
	       elseof L then
		  OzDocToHTML, ID(L @BibNode VERBATIM(BibKey))
	       end
	       EMPTY
	    %-----------------------------------------------------------
	    % Body and Sectioning Elements
	    %-----------------------------------------------------------
	    [] body then
	       BodyCommon <- @Common
	       Part <- 0
	       Chapter <- 0
	       Appendix <- false
	       OzDocToHTML, Batch(M 1 $)
	    [] part then X HTML1 HTML2 HTML3 HTML4 Title in
	       OzDocToHTML, PrepareNode(M ?X ?HTML1)
	       OzDocToHTML, MakeTitle(""
				      fun {$}
					 Part <- @Part + 1
					 'Part&nbsp;'#{RomanU @Part}
				      end
				      ': '
				      fun {$ N}
					 h1(align: center 'class': [part] N)
				      end
				      M 1 ?HTML2 ?Title)
	       OzDocToHTML, Batch(M 2 ?HTML3)
	       OzDocToHTML, FinishNode(Title X 'div'(COMMON: @Common
						     HTML2 HTML3) ?HTML4)
	       SEQ([HTML1 HTML4])
	    [] chapter then X HTML1 HTML2 HTML3 HTML4 Title in
	       OzDocToHTML, PrepareNode(M ?X ?HTML1)
	       OzDocToHTML, MakeTitle('Chapter&nbsp;'
				      fun {$}
					 Chapter <- @Chapter + 1
					 Section <- 0
					 FigureCounters <- {NewDictionary}
					 @Chapter
				      end
				      ' '
				      fun {$ N} h1(N) end
				      M 2 ?HTML2 ?Title)
	       OzDocToHTML, Batch(M 2 ?HTML3)
	       OzDocToHTML, FinishNode(Title X 'div'(COMMON: @Common
						     HTML2 HTML3) ?HTML4)
	       SEQ([HTML1 HTML4])
	    [] appendix then X HTML1 HTML2 HTML3 HTML4 Title in
	       OzDocToHTML, PrepareNode(M ?X ?HTML1)
	       if @Appendix then skip
	       else
		  Appendix <- true
		  Chapter <- 0
	       end
	       OzDocToHTML, MakeTitle('Appendix&nbsp;'
				      fun {$}
					 Chapter <- @Chapter + 1
					 Section <- 0
					 FigureCounters <- {NewDictionary}
					 {Alpha @Chapter}
				      end
				      ' '
				      fun {$ N} h1(N) end
				      M 2 ?HTML2 ?Title)
	       OzDocToHTML, Batch(M 2 ?HTML3)
	       OzDocToHTML, FinishNode(Title X 'div'(COMMON: @Common
						     HTML2 HTML3) ?HTML4)
	       SEQ([HTML1 HTML4])
	    [] section then X HTML1 HTML2 HTML3 HTML4 Title in
	       OzDocToHTML, PrepareNode(M ?X ?HTML1)
	       OzDocToHTML, MakeTitle('Section&nbsp;'
				      fun {$}
					 Section <- @Section + 1
					 SubSection <- 0
					 if @Appendix then {Alpha @Chapter}
					 else @Chapter
					 end#'.'#@Section
				      end
				      ' '
				      fun {$ N} h2(N) end
				      M 3 ?HTML2 ?Title)
	       OzDocToHTML, Batch(M 2 ?HTML3)
	       OzDocToHTML, FinishNode(Title X 'div'(COMMON: @Common
						     HTML2 HTML3) ?HTML4)
	       SEQ([HTML1 HTML4])
	    [] subsection then X HTML1 HTML2 HTML3 HTML4 Title in
	       OzDocToHTML, PrepareNode(M ?X ?HTML1)
	       OzDocToHTML, MakeTitle('Section&nbsp;'
				      fun {$}
					 SubSection <- @SubSection + 1
					 if @Appendix then {Alpha @Chapter}
					 else @Chapter
					 end#'.'#@Section#'.'#@SubSection
				      end
				      ' '
				      fun {$ N} h3(N) end
				      M 4 ?HTML2 ?Title)
	       OzDocToHTML, Batch(M 2 ?HTML3)
	       OzDocToHTML, FinishNode(Title X 'div'(COMMON: @Common
						     HTML2 HTML3) ?HTML4)
	       SEQ([HTML1 HTML4])
	    [] subsubsection then X HTML1 HTML2 HTML3 HTML4 Title in
	       OzDocToHTML, PrepareNode(M ?X ?HTML1)
	       OzDocToHTML, MakeTitle('Section '
				      fun {$} "" end
				      ""
				      fun {$ N} h4(N) end
				      M 5 ?HTML2 ?Title)
	       OzDocToHTML, Batch(M 2 ?HTML3)
	       OzDocToHTML, FinishNode(Title X 'div'(COMMON: @Common
						     HTML2 HTML3) ?HTML4)
	       SEQ([HTML1 HTML4])
	    %-----------------------------------------------------------
	    % Paragraphs
	    %-----------------------------------------------------------
	    [] p then
	       if {SGML.isOfClass M danger} then
		  'div'(COMMON: @Common
			p('class': [margin]
			  img(src: 'danger.gif' align: top alt: 'Danger'))
			p(OzDocToHTML, Batch(M 1 $)))
	       else
		  p(COMMON: @Common
		    if {SGML.isOfClass M warning} then
		       strong(PCDATA('Warning:'))
		    else EMPTY
		    end
		    OzDocToHTML, Batch(M 1 $))
	       end
	    [] para then Title in
	       Title = M.1=title(...)
	       'div'(COMMON: @Common
		     p(COMMON: COMMON(id: {CondSelect Title id unit}
				      'class': {CondSelect Title 'class' nil})
		       'class': [margin]
		       if {SGML.isOfClass M danger} then
			  SEQ([img(src: 'danger.gif' align: top alt: 'Danger')
			       VERBATIM(' ')])
		       else EMPTY
		       end
		       OzDocToHTML, Batch(Title 1 $))
		     p(OzDocToHTML, Batch(M 2 $)))
	    [] 'div' then
	       'div'(COMMON: @Common
		     if {SGML.isOfClass M danger} then
			p('class': [margin]
			  img(src: 'danger.gif' align: top alt: 'Danger'))
		     else EMPTY
		     end
		     OzDocToHTML, Batch(M 1 $))
	    [] exercise then N Number See HTML HTML2 in
	       N = {Dictionary.condGet @FigureCounters exercise 0} + 1
	       {Dictionary.put @FigureCounters exercise N}
	       Number = if @Appendix then {Alpha @Chapter}
			else @Chapter
			end#'.'#N
	       HTML = VERBATIM('Exercise&nbsp;'#Number)
	       HTML2 = case {CondSelect M id unit} of unit then
			  {@Reporter error(kind: OzDocError
					   msg: 'exercise must have an ID')}
			  See = EMPTY
			  HTML
		       elseof Id then
			  Exercises <- Id#Number#See|@Exercises
			  OzDocToHTML, ID(Id @CurrentNode HTML)
			  a(name: Id HTML)
		       end
	       'div'(COMMON: @Common 'class': [exercise]
		     p(b(HTML) See)
		     blockquote(OzDocToHTML, Batch(M 1 $)))
	    [] answer then
	       {Dictionary.put @Answers M.to M}
	       EMPTY
	    %-----------------------------------------------------------
	    % Lists
	    %-----------------------------------------------------------
	    [] list then
	       %--** display attribute
	       if {HasFeature M continues} then
		  {@Reporter warn(kind: OzDocWarning
				  msg: 'attribute `continues\' not implemented'
				  items: [hint(l: 'Node' m: oz(M))])}
	       end
	       if {Label M.1} == entry then X HTML in
		  X = @ListType
		  ListType <- description
		  OzDocToHTML, Batch(M 1 ?HTML)
		  ListType <- X
		  BLOCK(dl(COMMON: @Common HTML))
	       elseif {HasFeature M enum} then X Y HTML in
		  X = @ListType
		  ListType <- enumeration
		  Y = @OLTypes
		  OLTypes <- Y.2
		  OzDocToHTML, Batch(M 1 ?HTML)
		  OLTypes <- Y
		  ListType <- X
		  BLOCK(ol(COMMON: @Common
			   type: @OLTypes.1
			   start: {CondSelect M n unit}
			   HTML))
	       elseif {HasFeature M n} then
		  {@Reporter error(kind: OzDocError
				   msg: ('illegal attribute `n\' in '#
					 'non-enumerated list'))}
		  unit
	       elseif {SGML.isOfClass M linkmenu} then X HTML in
		  X = @ListType
		  ListType <- menu_first
		  OzDocToHTML, Batch(M 1 ?HTML)
		  ListType <- X
		  BLOCK(p(COMMON: @Common PCDATA('[ ') HTML PCDATA(' ]')))
	       else X HTML in
		  X = @ListType
		  ListType <- bulleted
		  OzDocToHTML, Batch(M 1 ?HTML)
		  ListType <- X
		  BLOCK(ul(COMMON: @Common HTML))
	       end
	    [] entry then HTML1 ClassName HTML2 in
	       OzDocToHTML, Batch(M.1 1 ?HTML1)
	       ClassName = {FoldLTail
			    {FoldR
			     {Dictionary.condGet @Meta 'entry.category' nil}
			     fun {$ Cat In} C T in
				C#T = case Cat of _#_ then Cat
				      else Cat#Cat
				      end
				if {SGML.isOfClass M C} then T|In else In end
			     end nil}
			    fun {$ In T|Tr}
			       In#T#case Tr of nil then "" else ' ' end
			    end ""}
	       HTML2 = case ClassName of "" then EMPTY
		       else
			  span('class': [entrycategory]
			       span('class': [entrycategorybracket]
				    VERBATIM('&nbsp;')
				    PCDATA('['))
			       i(PCDATA(ClassName))
			       span('class': [entrycategorybracket]
				    PCDATA(']')))
		       end
	       dt(COMMON: @Common HTML1 HTML2)
	    [] synopsis then
	       dd(COMMON: @Common
		  blockquote('class': [synopsis] OzDocToHTML, Batch(M 1 $)))
	    [] item then
	       case @ListType of description then
		  dd(COMMON: @Common OzDocToHTML, Batch(M 1 $))
	       [] menu_first then
		  ListType <- menu_rest
		  if {HasFeature M 2} then
		     {@Reporter error(kind: OzDocError
				      msg: ('link menu items must only '#
					    'consist of one paragraph each'))}
		  end
		  span(COMMON: @Common
		       OzDocToHTML, Batch(M.1 1 $))
	       [] menu_rest then
		  if {HasFeature M 2} then
		     {@Reporter error(kind: OzDocError
				      msg: ('link menu items must only '#
					    'consist of one paragraph each'))}
		  end
		  span(COMMON: @Common
		       PCDATA(' | ') OzDocToHTML, Batch(M.1 1 $))
	       else
		  li(COMMON: @Common OzDocToHTML, Batch(M 1 $))
	       end
	    %-----------------------------------------------------------
	    % Math
	    %-----------------------------------------------------------
	    [] math then Display HTML in
	       Display = case @MathDisplay of unit then M.display
			 elseof X then X
			 end
	       case M.type of 'LATEX' then FileName in
		  case @MyLaTeXToGIF of unit then
		     HTML = code(PCDATA(M.1))

		     %% if the math element contains just a number
		     %% then we really don't need a gif
		  elseif {All M.1 fun {$ C}
				     {Char.isDigit C} orelse
				     {Char.isSpace C} orelse
				     C==&. orelse C==&- orelse C==&+
				  end} then
		     HTML = PCDATA(M.1)
		  else
		     {@MyLaTeXToGIF convertMath(M.1 Display ?FileName)}
		     HTML = img(src: FileName
				alt: {CondSelect M info M.1})
		  end
	       [] 'HTML' then
		  HTML = VERBATIM(M.1)
	       else
		  {@Reporter error(kind: OzDocError
				   msg: 'unsupported math notation'   %--**
				   items: [hint(l: 'Notation' m: M.type)])}
		  HTML = unit
	       end
	       case Display of display then
		  BLOCK(blockquote(COMMON: @Common p(HTML)))
	       [] inline then
		  span(COMMON: @Common HTML)
	       end
	    [] 'math.extern' then
	       {@Reporter error(kind: OzDocError
				msg: 'unsupported element'   %--**
				items: [hint(l: 'Node' m: oz(M))])}
	       unit
	    [] 'math.choice' then HTML in
	       MathDisplay <- M.display
	       OzDocToHTML, Process({PickMathChoice M} ?HTML)   %--** make better choice
	       MathDisplay <- unit
	       HTML
	    %-----------------------------------------------------------
	    % Picture Element
	    %-----------------------------------------------------------
	    [] picture then
	       case {CondSelect M type unit}
	       of 'LATEX' then FileName in
		  case @MyLaTeXToGIF of unit then FileName='/dev/null'
		  elseof O then {O convertPicture(M.1 ?FileName)}
		  end
		  OzDocToHTML, PictureExtern("" M FileName $)
	       elseof N then
		  {@Reporter error(kind: OzDocError
				   msg: 'unsupported picture notation'   %--**
				   items: [hint(l: 'Notation' m: N)])}
		  unit
	       end
	    [] 'picture.extern' then
	       case {CondSelect M type unit}
	       of 'gif' then
		  OzDocToHTML, PictureExtern("" M M.to $)
	       [] 'ps' then To DIR = {Property.get 'ozdoc.src.dir'} in
		  {@MyPostScriptToGIF
		   %% we should really use URL.resolve
		   convertPostScript(DIR#'/'#M.to {CondSelect M info ''} ?To)}
		  OzDocToHTML, PictureExtern(@OutputDirectory#'/' M To $)
	       [] 'latex' then DIR FileName in
		  DIR = {Property.get 'ozdoc.src.dir'}
		  case @MyLaTeXToGIF of unit then FileName='/dev/null'
		  elseof O then
		     {O convertPicture({ReadFile DIR#'/'#M.to} ?FileName)}
		  end
		  OzDocToHTML, PictureExtern("" M FileName $)
	       [] unit then
		  %--** the notation should be derived from the file name
		  {@Reporter error(kind: OzDocError
				   msg: 'unspecified picture notation'
				   items: [hint(l: 'Node' m: oz(M))])}
		  unit
	       elseof N then
		  {@Reporter error(kind: OzDocError
				   msg: 'unsupported picture notation'   %--**
				   items: [hint(l: 'Notation' m: N)])}
		  unit
	       end
	    [] 'picture.choice' then HTML in
	       PictureDisplay <- M.display
	       OzDocToHTML, Process(M.1 ?HTML)   %--** make better choice
	       PictureDisplay <- unit
	       BLOCK('div'(COMMON: @Common HTML))
	    %-----------------------------------------------------------
	    % Code
	    %-----------------------------------------------------------
	    [] code then HTML in
	       OzDocToHTML, BatchCode(M 1 ?HTML)
	       case M.display of display then
		  BLOCK(blockquote(COMMON: @Common 'class': [code] HTML))
	       [] inline then span(COMMON: @Common HTML)
	       end
	    [] 'code.extern' then HTML in
	       {@MyFontifier enqueueFile(@ProgLang M.to HTML)}
	       case M.display of display then
		  BLOCK(blockquote(COMMON: @Common pre(HTML)))
	       [] inline then code(COMMON: @Common HTML)
	       end
	    [] var then
	       case M.type of prog then Annotation in
		  Annotation = case {CondSelect M mode unit} of unit then ""
			       [] 'in' then '+'
			       [] out then '?'
			       [] cin then '*'
			       [] cnin then '$'
			       end
		  code(COMMON: @Common
		       PCDATA(Annotation)
		       i(OzDocToHTML, Batch(M 1 $)))
	       [] meta then
		  i(COMMON: @Common OzDocToHTML, Batch(M 1 $))
	       [] env then HTML in
		  HTML = code(COMMON: @Common OzDocToHTML, Batch(M 1 $))
		  if @AutoIndex andthen {Not {SGML.isOfClass M noindex}} then
		     Kind HTML1 HTML2
		  in
		     Kind = 'environment variable'
		     OzDocToHTML, Index(M [PCDATA(Kind) HTML] ?HTML1)
		     OzDocToHTML, Index(M [SEQ([HTML PCDATA(' '#Kind)])]
					?HTML2)
		     SEQ([HTML1 HTML2 HTML])
		  else HTML
		  end
	       [] grammar then
		  span(COMMON: @Common
		       PCDATA('<')
		       i(OzDocToHTML, Batch(M 1 $))
		       PCDATA('>'))
	       end
	    %-----------------------------------------------------------
	    % Literate Programming
	    %-----------------------------------------------------------
	    [] chunk then Title Label Name ChunkNav Left Right Body Anon in
	       Anon = {SGML.isOfClass M anonymous}
	       if Anon then skip else
		  OzDocToHTML, Batch(M.1=title(...) 1 ?Title)
		  ToGenerate <- Label|@ToGenerate
		  Name = {VirtualString.toAtom
			  {HTML.toVirtualString {HTML.clean Title}}}
		  {Dictionary.put @ChunkDefinitions Name
		   (@CurrentNode#"#"#Label)#ChunkNav|
		   {Dictionary.condGet @ChunkDefinitions Name nil}}
	       end
	       Left = span('class': [chunkborder] PCDATA('<'))
	       Right = span('class': [chunkborder] PCDATA('>='))
	       Body = OzDocToHTML, BatchCode(M.2 1 $)
	       if Anon then
		  BLOCK(dl(COMMON: @Common
			dd('class': [code] Body)))
	       else
		  BLOCK(dl(COMMON: @Common
			   dt(span('class': [chunktitle]
				   SEQ([Left a(name: Label Title) Right]))
			      ChunkNav)
			   dd('class': [code] Body)))
	       end
	    [] 'chunk.ref' then Title LinkedTitle Left Right in
	       OzDocToHTML, Batch(M 1 ?Title)
	       ChunkLinks <- ({VirtualString.toAtom
			       {HTML.toVirtualString {HTML.clean Title}}}#
			      LinkedTitle)|@ChunkLinks
	       Left = span('class': [chunkborder] PCDATA('<'))
	       Right = span('class': [chunkborder] PCDATA('>'))
	       span(COMMON: @Common
		    'class': [chunktitle]
		    SEQ([Left LinkedTitle Right]))
	    %-----------------------------------------------------------
	    % Cross References
	    %-----------------------------------------------------------
	    [] ref then Node in
	       OzDocToHTML, ID(M.to ?Node _)
	       a(COMMON: @Common href: Node#"#"#M.to
		 OzDocToHTML, Batch(M 1 $))
	    [] 'ref.extern' then To in
	       case M.to of &o|&z|&d|&o|&c|&:|R then
		  {@MyCrossReferencer get(R {CondSelect M key unit} ?To _)}
	       else
		  %--** key attribute?
		  To = M.to
	       end
	       a(COMMON: @Common href: To OzDocToHTML, Batch(M 1 $))
	    [] ptr then Node HTML in
	       OzDocToHTML, ID(M.to ?Node ?HTML)
	       a(COMMON: @Common href: Node#"#"#M.to HTML)
	    [] 'ptr.extern' then
	       case M.to of &o|&z|&d|&o|&c|&:|R then To HTML in
		  {@MyCrossReferencer get(R {CondSelect M key unit} ?To ?HTML)}
		  a(COMMON: @Common href: To HTML)
	       else
		  %--** use an icon as content
		  {@Reporter
		   error(kind: OzDocError
			 msg: 'unsupported external reference'   %--**
			 items: [hint(l: 'Node' m: oz(M))])}
		  unit
	       end
	    %-----------------------------------------------------------
	    % Phrasal Elements
	    %-----------------------------------------------------------
	    [] file then HTML in
	       HTML = code(OzDocToHTML, Batch(M 1 $))
	       OzDocToHTML, MakeDisplay(M HTML $)
	    [] kbd then HTML in
	       HTML = kbd(OzDocToHTML, Batch(M 1 $))
	       OzDocToHTML, MakeDisplay(M HTML $)
	    [] key then HTML HTML2 in
	       HTML = span('class': [key] OzDocToHTML, Batch(M 1 $))
	       HTML2 = if {HasFeature M id} then
			  OzDocToHTML, ID(M.id @CurrentNode HTML)
			  a(name: M.id HTML)
		       else HTML
		       end
	       OzDocToHTML, MakeDisplay(M HTML2 $)
	    [] samp then HTML in
	       HTML = code(OzDocToHTML, Batch(M 1 $))
	       OzDocToHTML, MakeDisplay(M HTML $)
	    [] name then
	       case {CondSelect M type unit} of unit then
		  span(COMMON: @Common 'class': [name]
		       OzDocToHTML, Batch(M 1 $))
	       [] buffer then
		  code(COMMON: @Common OzDocToHTML, Batch(M 1 $))
	       elseof X then
		  span(COMMON: @Common 'class': [X]
		       OzDocToHTML, Batch(M 1 $))
	       end
	    [] q then
	       %--** use different quotes, depending on lang?
	       if {SGML.isOfClass M terminal} then
		  span(COMMON: @Common
		       PCDATA('"') OzDocToHTML, Batch(M 1 $) PCDATA('"'))
	       elseif {SGML.isOfClass M quasi} then
		  span(COMMON: @Common
		       PCDATA('`') OzDocToHTML, Batch(M 1 $) PCDATA('\''))
	       else
		  span(COMMON: @Common
		       PCDATA('``') OzDocToHTML, Batch(M 1 $) PCDATA('\'\''))
	       end
	    [] span then HTML1 HTML in
	       OzDocToHTML, Batch(M 1 ?HTML1)
	       HTML = span(HTML1)
	       case {CondSelect M display inline}
	       of inline then
		  if {SGML.isOfClass M index} then HTML2 in
		     OzDocToHTML, Index(M [HTML1] ?HTML2)
		     SEQ([HTML2 {AdjoinAt HTML COMMON @Common}])
		  else
		     {AdjoinAt HTML COMMON @Common}
		  end
	       [] display then
		  if {SGML.isOfClass M index} then HTML2 in
		     OzDocToHTML, Index(M [HTML1] ?HTML2)
		     BLOCK(blockquote(COMMON: @Common HTML2 HTML))
		  else
		     BLOCK(blockquote(COMMON: @Common HTML))
		  end
	       end
	    [] def then HTML1 HTML in
	       OzDocToHTML, Batch(M 1 ?HTML1)
	       HTML = em(COMMON: @Common HTML1)
	       if @AutoIndex andthen {Not {SGML.isOfClass M noindex}} then
		  HTML2
	       in
		  OzDocToHTML, Index(M [HTML1] ?HTML2)
		  SEQ([HTML2 HTML])
	       else HTML
	       end
	    %-----------------------------------------------------------
	    % Figure
	    %-----------------------------------------------------------
	    [] figure then
	       if {HasFeature M float} then
		  Floats <- {Append @Floats [M]}
		  EMPTY
	       else
		  BLOCK(OzDocToHTML, OutputFigure(M $))
	       end
	    [] caption then
	       {@Reporter error(kind: OzDocError
				msg: 'internal error - caption unexpected')}
	       unit
	    %-----------------------------------------------------------
	    % Note
	    %-----------------------------------------------------------
	    [] note then
	       case {CondSelect M foot unit} of unit then
		  %--** shouldn't stay here!  Where should the note float to?
		  BLOCK('div'(COMMON: @Common 'class': [note]
			      OzDocToHTML, Batch(M 1 $)))
	       else HTML in
		  FootNotes <- {Append @FootNotes [M#HTML]}
		  HTML
	       end
	    %-----------------------------------------------------------
	    % Index
	    %-----------------------------------------------------------
	    [] index then NewM Ands in
	       %--** scope?
	       IndexSortAs <- {CondSelect M 'sort.as' unit}
	       NewM = if {SGML.isOfClass M menu} then
			 {TransformAnds M
			  fun {$ And} span('class': [menu] And) end}
		      elseif {SGML.isOfClass M module} then
			 {TransformAnds M
			  fun {$ And} code(display: inline And) end}
		      else M
		      end
	       OzDocToHTML, BatchSub(NewM 1 ?Ands)
	       OzDocToHTML, Index(NewM Ands $)
	    [] and then SortAs Item in
	       SortAs = case {CondSelect M 'sort.as' unit}
			of unit then @IndexSortAs
			elseof X then X
			end
	       IndexSortAs <- unit
	       OzDocToHTML, Batch(M 1 ?Item)
	       case SortAs of unit then Item
	       else SortAs#Item
	       end
	    [] see then
	       {@Reporter error(kind: OzDocError
				msg: 'unsupported element'   %--**
				items: [hint(l: 'Node' m: oz(M))])}
	       unit
	    %-----------------------------------------------------------
	    % BNF Markup
	    %-----------------------------------------------------------
	    [] 'grammar.rule' then X HTML in
	       %--** respect the display attribute?
	       X = @GrammarIsCompact#@GrammarHead#@GrammarAltType
	       GrammarIsCompact <- {SGML.isOfClass M compact}
	       GrammarHead <- OzDocToHTML, Process(M.1 $)
	       GrammarAltType <- def
	       OzDocToHTML, Batch(M 2 ?HTML)
	       GrammarIsCompact <- X.1
	       GrammarHead <- X.2
	       GrammarAltType <- X.3
	       BLOCK(blockquote(table(COMMON: @Common
				      border: 0 cellpadding: 0 cellspacing: 0
				      HTML)))
	    [] 'grammar.head' then
	       span(COMMON: @Common OzDocToHTML, Batch(M 1 $))
	    [] 'grammar.alt' then HTML0 HTML1 HTML2 in
	       HTML0 = @GrammarHead
	       HTML1 = case {CondSelect M type @GrammarAltType} of def then
			  VERBATIM('&nbsp;::=&nbsp;')
		       [] add then
			  VERBATIM('&nbsp;+=&nbsp;')
		       [] 'or' then
			  VERBATIM('&nbsp;|&nbsp;')
		       [] space then
			  EMPTY
		       end
	       GrammarHead <- EMPTY
	       GrammarAltType <- 'or'
	       OzDocToHTML, Batch(M 1 ?HTML2)
	       if @GrammarIsCompact then
		  GrammarIsCompact <- false
		  SEQ([tr(valign: top td(colspan: 3 HTML0 HTML1))
		       tr(COMMON: @Common valign: top
			  td(VERBATIM('&nbsp;&nbsp;&nbsp;&nbsp;'))
			  td()
			  td(HTML2)
			  case @GrammarNote of unit then EMPTY
			  elseof N then GrammarNote <- unit N
			  end)])
	       else
		  tr(COMMON: @Common valign: top
		     td(HTML0)
		     td(align: center HTML1)
		     td(HTML2)
		     case @GrammarNote of unit then EMPTY
		     elseof N then GrammarNote <- unit N
		     end)
	       end
	    [] 'grammar.note' then
	       GrammarNote <- td(COMMON: @Common align: left
				 i(VERBATIM('&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;% ')
				   OzDocToHTML, Batch(M 1 $)))
	       EMPTY
	    [] 'grammar' then
	       BLOCK('div'(COMMON: @Common OzDocToHTML, Batch(M 1 $)))
	    %-----------------------------------------------------------
	    % Tables
	    %-----------------------------------------------------------
	    [] table then Old Title Mr HTML in
	       %--** display attribute
	       Old = @TableCols#@TableRow#@CurTableCols
	       TableCols <- unit
	       TableRow <- 1
	       CurTableCols <- unit
	       Title = {SGML.getSubtree M title ?Mr}
	       OzDocToHTML, Batch(Mr 1 ?HTML)
	       TableCols <- Old.1
	       TableRow <- Old.2
	       CurTableCols <- Old.3
	       BLOCK(local
			T=table(COMMON: @Common
				align: center
				case Title of unit then EMPTY
				else
				   tr(td(p(align: center
					   b(OzDocToHTML, Batch(Title 1 $)))))
				end
				HTML)
		     in
			if {SGML.isOfClass M dyptic} then T else
			   {AdjoinAt T bgcolor '#f0f0e0'}
			end
		     end)
	    [] tr then HTML in
	       CurTableCols <- 0
	       OzDocToHTML, Batch(M 1 ?HTML)
	       case @TableCols of unit then
		  TableCols <- @CurTableCols
	       elseof N then
		  if N \= @CurTableCols then
		     {@Reporter warn(kind: OzDocWarning
				     msg: ('inconsistent number of columns '#
					   'in table row')
				     items: [hint(l: 'Row number' m: @TableRow)
					     hint(l: 'Found' m: @CurTableCols)
					     hint(l: 'First row' m: N)])}
		  end
	       end
	       TableRow <- @TableRow + 1
	       tr(COMMON: @Common valign: top HTML)
	    [] th then
	       CurTableCols <- (@CurTableCols +
				case {CondSelect M colspan unit} of unit then 1
				elseof N then N
				end)
	       th(COMMON: @Common
		  colspan: {CondSelect M colspan unit}
		  OzDocToHTML, Batch(M 1 $))
	    [] td then
	       CurTableCols <- (@CurTableCols +
				case {CondSelect M colspan unit} of unit then 1
				elseof N then N
				end)
	       td(COMMON: @Common
		  colspan: {CondSelect M colspan unit}
		  OzDocToHTML, Batch(M 1 $))
	    %-----------------------------------------------------------
	    %--** gump.sgml Specials
	    %-----------------------------------------------------------
	    [] comic then
	       Comic <- M.1
	       EMPTY
	    [] em then
	       em(COMMON: @Common OzDocToHTML, Batch(M 1 $))
	    %-----------------------------------------------------------
	    %--** explorer.sgml Specials
	    %-----------------------------------------------------------
	    [] 'note.gui' then
	       BLOCK(table('class':     [margin]
			   COMMON:      @Common
			   border:      0
			   cellspacing: 0
			   cellpadding: 2
			   tr(td(OzDocToHTML,Batch(M 1 $))
			      if {HasFeature M icon} then
				 td(img(src: M.icon align: top
					alt: {CondSelect M info ''}))
			      else EMPTY
			      end)))
	    [] 'menu' then Menu Mouse in
	       Menu  = table(bgcolor:     '#cccccc'
			     width:       130
			     border:      1
			     cellpadding: 3
			     cellspacing: 0
			     local L in
				L = td(align: left
				       OzDocToHTML,Batch(M 1 $))
				if {HasFeature M key} then
				   tr({AdjoinAt L width '85%'}
				      td(align: right PCDATA(M.key)))
				else
				   tr(L)
				end
			     end)
	       Mouse = if {HasFeature M mouse} then
			  img(src:   'note-gui-'#M.mouse#'.gif'
			      align: middle
			      alt:   case M.mouse
				     of l1 then 'Left mouse click'
				     [] l2 then 'Left mouse double-click'
				     [] r1 then 'Right mouse click'
				     [] r2 then 'Right mouse double-click'
				     [] m1 then 'Middle mouse click'
				     [] m2 then 'Middle mouse double-click'
				     end)
		       else EMPTY
		       end
	       BLOCK(table(COMMON: @Common tr(td(Menu) td(Mouse))))
	    %-----------------------------------------------------------
	    %--** ozdoc.sgml Specials
	    %-----------------------------------------------------------
	    [] tag then
	       code(COMMON: @Common OzDocToHTML, Batch(M 1 $))
	    [] attrib then
	       code(COMMON: @Common OzDocToHTML, Batch(M 1 $))
	    %-----------------------------------------------------------
	    %--** notation.sgml Specials
	    %-----------------------------------------------------------
	    [] rewrite then Vars N in
	       Vars = {Map {Filter {Record.toListInd M}
			    fun {$ _#N} {Label N} == var end}
		       fun {$ _#N} N end}
	       N = {Length Vars}
	       BLOCK(table(COMMON: @Common border: 1
			   case Vars of nil then EMPTY
			   else
			      tr(valign: top
				 td({FoldRTail Vars
				     fun {$ V|Vr In}
					SEQ([OzDocToHTML, Process(V $)
					     case Vr of nil then EMPTY
					     else PCDATA(', ')
					     end In])
				     end EMPTY}
				    PCDATA(' ::=')))
			   end
			   local
			      Row = tr(OzDocToHTML, Process(M.(N + 1) $)
				       td(OzDocToHTML,
					  Process(math(info: '==>'
						       display: inline
						       type: 'LATEX'
						       "\\Longrightarrow") $))
				       OzDocToHTML, Process(M.(N + 2) $))
			   in
			      tr(td(table(width: '100%' Row)))
			   end
			   OzDocToHTML, Batch(M N + 3 $)))
	    [] 'rewrite.from' then
	       td(COMMON: @Common OzDocToHTML, Batch(M 1 $))
	    [] 'rewrite.to' then
	       td(COMMON: @Common OzDocToHTML, Batch(M 1 $))
	    [] 'rewrite.condition' then
	       tr(COMMON: @Common valign: top
		  td(OzDocToHTML, Batch(M 1 $)))
	    else
	       {@Reporter error(kind: OzDocError
				msg: 'unknown element'   %--**
				items: [hint(l: 'Node' m: oz(M))])}
	       unit
	    end
	 end = Res
	 OzDocToHTML, PopCommon(OldCommon)
	 Res
      end
      meth MakeTitle(PtrText FormatNumber Sep LayoutTitle M Level HTML
		     ?NodeTitle)
	 HTML1 Ns Title Authors Abstract TheLabel Res
      in
	 OzDocToHTML, FlushFloats(?HTML1)
	 Ns = {Record.toList M.1=front(...)}
	 Title = case {Filter Ns fun {$ N} {Label N} == title end} of [T] then
		    OzDocToHTML, Batch(T 1 $)
		 [] nil then unit
		 end
	 Authors = {FoldR Ns
		    fun {$ N In}
		       case N of author(...) then
			  OzDocToHTML, MakeAuthor(N $)|In
		       [] 'author.extern'(...) then
			  OzDocToHTML, MakeAuthor(N $)|In
		       else In
		       end
		    end nil}
	 Abstract = case {Filter Ns fun {$ N} {Label N} == abstract end}
		    of [A] then
		       blockquote(span(COMMON: @Common
				       OzDocToHTML, Batch(A 1 $)))
		    [] nil then EMPTY
		    end
	 if {HasFeature M id} then
	    TheLabel = M.id
	 else
	    ToGenerate <- TheLabel|@ToGenerate
	 end
	 Threading <- sect(@CurrentNode TheLabel)|@Threading
	 Res = a(name: TheLabel NodeTitle)
	 if {SGML.isOfClass M unnumbered} then
	    if Title == unit then
	       {@Reporter error(kind: OzDocError
				msg: 'empty unnumbered section title'
				items: [hint(l: 'Node' m: oz(M))])}
	    end
	    NodeTitle = Title
	    if {HasFeature M id} then
	       OzDocToHTML, ID(M.id @CurrentNode
			       SEQ([VERBATIM(PtrText)
				    PCDATA('``') NodeTitle PCDATA('\'\'')]))
	    end
	 else NumberVS in
	    NumberVS = {FormatNumber}
	    NodeTitle = case Title of unit then PCDATA(NumberVS)
			else SEQ([VERBATIM(NumberVS#Sep) Title])
			end
	    if {HasFeature M id} then
	       case {Label M} of subsubsection then
		  OzDocToHTML, ID(M.id @CurrentNode
				  SEQ([VERBATIM(PtrText)
				       PCDATA('``') NodeTitle PCDATA('\'\'')]))
	       else
		  OzDocToHTML, ID(M.id @CurrentNode VERBATIM(PtrText#NumberVS))
	       end
	    end
	 end
	 WhereNow <- NodeTitle|@WhereNow
	 TOC <- {Append @TOC [Level#TheLabel#@CurrentNode#NodeTitle]}
	 HTML = SEQ([HTML1 {LayoutTitle Res}
		     case Authors of nil then EMPTY
		     else
			h3('class': [authors]
			   OzDocToHTML, FormatAuthors(Authors $))
		     end
		     Abstract])
      end
      meth ID(L Node HTML)
	 if {Dictionary.member @Labels L} then
	    Node#HTML = {Dictionary.get @Labels L}
	 else
	    {@MyCrossReferencer put(L Node#"#"#L HTML)}
	    {Dictionary.put @Labels L Node#HTML}
	 end
      end
      meth GenerateLabels()
	 {FoldR @ToGenerate
	  fun {$ L In}
	     OzDocToHTML, GenerateLabel(L In $)
	  end 1 _}
      end
      meth GenerateLabel(L In Next) A in
	 A = {VirtualString.toAtom 'label'#In}
	 if {Dictionary.member @Labels A} then
	    OzDocToHTML, GenerateLabel(L In + 1 Next)
	 else
	    L = A
	    Next = In + 1
	 end
      end
      meth PrepareNode(M ?X ?HTML)
	 case OzDocToHTML, GetSplitNode(M $) of unit then
	    X = unit
	    HTML = if @TOCMode then hr()
		   else EMPTY
		   end
	 elseof Node then N in
	    SomeSplit <- true
	    X = @NavigationPanel#@CurrentNode#@TOC#@TOCMode
	    Threading <- nav(N)|down(@CurrentNode)|@Threading
	    CurrentNode <- Node
	    NavigationPanel <- N
	    TOC <- nil
	    HTML = EMPTY
	 end
	 TOCMode <- false
      end
      meth GetSplitNode(M $)
	 if @Split then
	    case {CondSelect M id unit} of unit then unit
	    elseof ID then Splits in
	       Splits = {Dictionary.condGet @Meta 'html.split' nil}
	       OzDocToHTML, GetSplitNodeSub(Splits ID $)
	    end
	 else unit
	 end
      end
      meth GetSplitNodeSub(Ms ID $)
	 case Ms of M|Mr then
	    case M of !ID then
	       NodeCounter <- @NodeCounter + 1
	       'node'#@NodeCounter#'.html'
	    elseof !ID#Node then Node
	    else
	       OzDocToHTML, GetSplitNodeSub(Mr ID $)
	    end
	 [] nil then unit
	 end
      end
      meth PrepareTOCNode(?X ?HTML) N in
	 SomeSplit <- true
	 X = @NavigationPanel#@CurrentNode#@TOC#@TOCMode
	 Threading <- nav(N)|down(@CurrentNode)|@Threading
	 CurrentNode <- 'toc.html'
	 NavigationPanel <- N
	 TOC <- nil
	 HTML = EMPTY
	 TOCMode <- false
      end
      meth PrepareAnswersNode(?X ?HTML)
	 OzDocToHTML, PrepareBackMatter('html.split.answers' 'answers.html'
					X HTML)
      end
      meth PrepareBibNode(?X ?HTML)
	 OzDocToHTML, PrepareBackMatter('html.split.bib' 'bib.html'
					X HTML)
      end
      meth PrepareIdxNode(?X ?HTML)
	 OzDocToHTML, PrepareBackMatter('html.split.index' 'idx.html'
					X HTML)
      end
      meth PrepareBackMatter(SplitMeta NodeName ?X ?HTML)
	 if @Split andthen {Dictionary.member @Meta SplitMeta} then N in
	    SomeSplit <- true
	    X = @NavigationPanel#@CurrentNode#@TOC#@TOCMode
	    Threading <- nav(N)|down(@CurrentNode)|@Threading
	    CurrentNode <- NodeName
	    NavigationPanel <- N
	    TOC <- nil
	    HTML = EMPTY
	 else
	    X = unit
	    HTML = if @TOCMode then hr()
		   else EMPTY
		   end
	 end
	 TOCMode <- false
      end
      meth FinishNode(Title X HTML $)
	 case X of unit then HTML
	 [] N#C#T#M then Depth Res in
	    OzDocToHTML, MakeNode(Title HTML)
	    Threading <- up|@Threading
	    NavigationPanel <- N
	    CurrentNode <- C
	    Depth = if C == 'index.html' andthen {IsFree @WholeTOC} then 1
		    else ~1
		    end
	    Res = if M then {FormatTOC @TOC Depth unit}
		  else SEQ([hr() {FormatTOC @TOC Depth unit}])
		  end
	    TOC <- {Append T @TOC}
	    TOCMode <- true
	    Res
	 end
      end
      meth MakeNode(Title BodyContents) Node in
	 Node = html(head(title(case Title of unit then
				   PCDATA('Anonymous Document')
				else
				   thread {HTML.clean Title} end
				end)
			  link(rel: stylesheet
			       type: 'text/css'
			       href: @StyleSheet))
		     'body'(COMMON: @BodyCommon
			    @NavigationPanel
			    BodyContents
			    OzDocToHTML, FlushFloats($)
			    @NavigationPanel
			    OzDocToHTML, FlushFootNotes(1 $)
			    hr()
			    address(case @Authors of nil then EMPTY
				    else As in
				       OzDocToHTML, FormatAuthors(@Authors ?As)
				       SEQ([As br()])
				    end
				    span('class':[version]
					 PCDATA('Version '#
						{Property.get 'oz.version'}#
						' ('#
						{Property.get 'oz.date'}#
						')')))))
	 ToWrite <- (('<!DOCTYPE html PUBLIC '#DOCTYPE_PUBLIC#'>\n')#Node#
		     (@OutputDirectory#'/'#@CurrentNode))|@ToWrite
      end
      meth MakeDisplay(M HTML $)
	 case M.display of inline then {AdjoinAt HTML COMMON @Common}
	 [] display then BLOCK(blockquote(COMMON: @Common HTML))
	 end
      end
      meth FlushFloats($) HTML in
	 case @Floats of F|Fr then OldCommon in
	    OzDocToHTML, PushCommon(F ?OldCommon)
	    OzDocToHTML, OutputFigure(F ?HTML)
	    OzDocToHTML, PopCommon(OldCommon)
	    Floats <- Fr
	    SEQ([HTML OzDocToHTML, FlushFloats($)])
	 [] nil then EMPTY
	 end
      end
      meth OutputFigure(M $)
	 ClassR Class N Number Title Mr1 Caption Mr2 HTML
      in
	 ClassR = {Filter {CondSelect M 'class' nil} fun {$ X} X \= maxi end}
	 Class = case ClassR of nil then 'figure'
		 elseof C|_ then C
		 end
	 N = {Dictionary.condGet @FigureCounters Class 0} + 1
	 {Dictionary.put @FigureCounters Class N}
	 Number = ({InitialCapital {Atom.toString Class}}#'&nbsp;'#
		   if @Appendix then {Alpha @Chapter}
		   else @Chapter
		   end#'.'#N)
	 Title = {SGML.getSubtree M title ?Mr1}
	 Caption = {SGML.getSubtree Mr1 caption ?Mr2}
	 HTML = SEQ([hr()
		     case {CondSelect M id unit} of unit then EMPTY
		     elseof L then
			OzDocToHTML, ID(L @CurrentNode VERBATIM(Number))
			p(a(name: L))
		     end
		     case Title of unit then EMPTY
		     else
			p(align: center b(OzDocToHTML, Batch(Title 1 $)))
		     end
		     OzDocToHTML, Batch(Mr2 1 $)
		     case Caption of unit then
			p('class':[caption] strong(VERBATIM(Number#'.')))
		     else
			p('class':[caption] strong(VERBATIM(Number#':'))
			  PCDATA(' ')
			  OzDocToHTML, Batch(Caption.1 1 $))
		     end
		     hr()])
	 if {SGML.isOfClass M maxi} then
	    'div'(COMMON: @Common
		  'class': [maxi]
		  table(width: '100%' tr(td(HTML))))
	 else
	    'div'(COMMON: @Common HTML)
	 end
      end
      meth FlushFootNotes(Count $) HTML in
	 case @FootNotes of M#T|Fr then OldCommon in
	    OzDocToHTML, PushCommon(M ?OldCommon)
	    OzDocToHTML, OutputFootNote(Count M ?T ?HTML)
	    OzDocToHTML, PopCommon(OldCommon)
	    FootNotes <- Fr
	    SEQ([if Count == 1 then hr(align: left width: '30%')
		 else EMPTY
		 end
		 HTML OzDocToHTML, FlushFootNotes(Count + 1 $)])
	 [] nil then EMPTY
	 end
      end
      meth OutputFootNote(N M ?T ?HTML) Label in
	 ToGenerate <- Label|@ToGenerate
	 T = a(href: @CurrentNode#"#"#Label sup(PCDATA(N)))   %--** use [1]
	 HTML = 'div'(COMMON: @Common 'class': [footnote]
		      SEQ([a(name: Label PCDATA(N#'. '))
			   OzDocToHTML, Batch(M.1 1 $)]))
      end
      meth PictureExtern(Dir M To $) Display HTML in
	 Display = case @PictureDisplay of unit then M.display
		   elseof X then X
		   end
	 HTML = if {SGML.isOfClass M thumbnail} then ThumbnailName in
		   {@MyThumbnails get(Dir#To ?ThumbnailName)}
		   a(href: To
		     img(COMMON: @Common src: ThumbnailName alt: ''))
		else
		   img(COMMON: @Common src: To alt: '')
		end
	 case Display of display then Align in
	    Align = if {SGML.isOfClass M left} then left
		    elseif {SGML.isOfClass M right} then right
		    else center
		    end
	    BLOCK('div'(align: Align HTML))
	 [] inline then HTML
	 end
      end
      meth OutputAnswers($)
	 SEQ({FoldL @Exercises
	      proc {$ In ID#Number#See ?HTML}
		 case {Dictionary.condGet @Answers ID unit} of unit then VS in
		    VS = {Atom.toString ID}
		    {@Reporter warn(kind: OzDocWarning
				    msg: 'exercise has no answer'
				    items: [hint(l: 'ID' m: VS)])}
		    See = EMPTY
		    HTML = In
		 elseof M then OldCommon Label in
		    OzDocToHTML, PushCommon(M ?OldCommon)
		    HTML = 'div'(COMMON: @Common 'class': [answer]
				 p(a(name: Label
				     b(VERBATIM('Answer&nbsp;'#Number))))
				 blockquote(OzDocToHTML, Batch(M 1 $)))|In
		    ToGenerate <- Label|@ToGenerate
		    See = SEQ([PCDATA(' (')
			       a(href: @CurrentNode#"#"#Label
				 VERBATIM('See solution'))
			       PCDATA(')')])
		    OzDocToHTML, PopCommon(OldCommon)
		 end
	      end nil})
      end
      meth Index(M Ands $) IsTails L L2 in
	 IsTails = ({SGML.isOfClass M tails}
		    orelse {SGML.isOfClass M menu}
		    orelse {SGML.isOfClass M module})
	 if IsTails orelse {HasFeature M id} then SeeHTML in
	    SeeHTML = SEQ({List.foldRTail Ands
			   fun {$ A|Ar In}
			      case A of _#X then X else A end|
			      case Ar of _|_ then PCDATA(', ')
			      else EMPTY
			      end|In
			   end nil})
	    case {CondSelect M id unit} of unit then
	       ToGenerate <- L|@ToGenerate
	    elseof X then
	       L = X
	       OzDocToHTML, ID(X @IdxNode SeeHTML)
	    end
	    if IsTails then
	       OzDocToHTML, IndexTails(Ands.2 [Ands.1] L SeeHTML L2)
	    end
	 else
	    L = unit
	 end
	 ToGenerate <- L2|@ToGenerate
	 case {CondSelect M see unit} of unit then
	    {@MyIndexer enter(L Ands a(href: @CurrentNode#"#"#L2 @WhereNow.1)
			      (@CurrentNode#"#"#L2)#@WhereNow.1
			      {CondSelect M 'class' nil})}
	 elseof X then Node HTML in
	    OzDocToHTML, ID(X ?Node ?HTML)
	    {@MyIndexer enter(L Ands SEQ([PCDATA('see ')
					  a(href: Node#"#"#X HTML)
					  PCDATA(' (')
					  a(href: @CurrentNode#"#"#L2
					    @WhereNow.1)
					  PCDATA(')')])
			      (@CurrentNode#"#"#L2)#@WhereNow.1
			      [see])}
	 end
	 a(name: L2)
      end
      meth IndexTails(Ands Prefix L HTML L2)
	 case Ands of A|Ar then
	    {@MyIndexer enter(unit {Append Ands Prefix}
			      SEQ([PCDATA('see ')
				   a(href: @IdxNode#"#"#L HTML)
				   PCDATA(' (')
				   a(href: @CurrentNode#"#"#L2
				     @WhereNow.1)
				   PCDATA(')')])
			      (@CurrentNode#"#"#L2)#@WhereNow.1
			      [generated])}
	    OzDocToHTML, IndexTails(Ar {Append Prefix [A]} L HTML L2)
	 [] nil then skip
	 end
      end
      meth MakeAuthor(M $)
	 case M of author(...) then
	    author(name: OzDocToHTML, Batch(M 1 $))
	 [] 'author.extern'(...) then
	    case {CondSelect M key unit} of unit then
	       {@Reporter error(kind: OzDocError
				msg: 'missing attribute `key\''
				items: [hint(l: 'Node' m: oz(M))])}
	       author()
	    elseof Key then
	       {@MyAuthorDB get(M.to Key $)}
	    end
	 end
      end
      meth FormatAuthors(Authors $)
	 case Authors of nil then ""
	 [] A|Ar then
	    fun {FormatAuthor A} N1 N2 H in
	       N1 = {FoldLTail [{CondSelect A firstname unit}
				{CondSelect A lastname unit}]
		     fun {$ In X|Xr}
			case X of unit then In
			else In#X#case Xr of nil then "" else '&nbsp;' end
			end
		     end ""}
	       N2 = case N1 of "" then {CondSelect A name ""}
		    else VERBATIM(N1)
		    end
	       H = if {HasFeature A www} then A.www
		   elseif {HasFeature A email} then 'email:'#A.email
		   else unit
		   end
	       case H of unit then N2
	       else a(href: H N2)
	       end
	    end
	 in
	    {FoldLTail Ar
	     fun {$ In A|Ar}
		SEQ([In
		     VERBATIM(case Ar of nil then ' and&nbsp;' else ', ' end)
		     {FormatAuthor A}])
	     end {FormatAuthor A}}
	 end
      end
   end

   class MyListener from ErrorListener.'class'
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

   proc {Translate Mode Args} O L Sync in
      O = {New OzDocToHTML init()}
      L = {New MyListener init(O Sync)}
      {O translate(Mode Args)}
      {Wait Sync}
      if {L hasErrors($)} then
	 raise error end
      end
   end
end
