%%%
%%% Author:
%%%   Leif Kornstaedt <kornstae@ps.uni-sb.de>
%%%
%%% Contributors:
%%%   Tobias Mueller <tmueller@ps.uni-sb.de>
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
   SGML(parse namePI getSubtree isOfClass)
   OS(system localTime)
   Open(file)
   AuthorDB('class')
   BibliographyDB('class')
   Fontifier('class' noProgLang)
   Thumbnails('class')
   MathToGIF('class')
   HTML(empty: EMPTY
        seq: SEQ
        common: COMMON
        block: BLOCK
        pcdata: PCDATA
        verbatim: VERBATIM
        toVirtualString)
   Property(get)
export
   Translate
define
   DOCTYPE_PUBLIC = '"-//W3C//DTD HTML 4.0 Transitional//EN"'

   %%
   %% Note: order is important in the following list!
   %%
   EntryClasses = [private#'private' protected#'protected' public#'public'
                   datatype#'data type' enumtype#'enum type'
                   static#'static' virtual#'virtual' purevirtual#'pure virtual'
                   constructor#'constructor' destructor#'destructor'
                   operator#'operator' member#'member' function#'function'
                   macro#'macro'
                   variable#'variable' command#'command']

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
      fun {MakeLIs LIs}
         case LIs of li(T)|(Y=ul(...))|Xr then {MakeLIs li(SEQ([T Y]))|Xr}
         elseof X|Xr then X|{MakeLIs Xr}
         [] nil then nil
         end
      end

      fun {FormatTOCLevel TOC Level LIss}
         case TOC of Entry|TOCr then N#Label#Node#Text = Entry in
            if N < Level then LIs1|LIs2|LIsr = LIss in
               {FormatTOCLevel TOC Level - 1
                (ul('class': [toc] SEQ({MakeLIs {Reverse LIs1}}))|LIs2)|LIsr}
            elseif N > Level then
               {FormatTOCLevel TOC Level + 1 nil|LIss}
            else LIs|LIsr = LIss in
               {FormatTOCLevel TOCr Level
                (li(a(href: Node#"#"#Label Text))|LIs)|LIsr}
            end
         [] nil then
            {FoldL LIss
             fun {$ In LIs} LIs1 in
                LIs1 = case In of unit then LIs else In|LIs end
                ul('class': [toc] SEQ({MakeLIs {Reverse LIs1}}))
             end unit}
         end
      end
   in
      fun {FormatTOC TOC}
         case TOC of Entry|_ then N#_#_#_ = Entry in
            {FormatTOCLevel TOC N [nil]}
         [] nil then SEQ(nil)
         end
      end
   end

   local
      WeekDays = weekDays('Sunday' 'Monday' 'Tuesday' 'Wednesday'
                          'Thursday' 'Friday' 'Saturday')

      Months = months('January' 'February' 'March' 'April' 'May' 'June'
                      'July' 'August' 'September' 'October' 'November'
                      'December')

      Ordinals = ordinals(1: 'st' 2: 'nd' 3: 'rd'
                          21: 'st' 22: 'nd' 23: 'rd' 31: 'st')

      fun {TwoDig N}
         if N < 10 then '0'#N
         else N
         end
      end
   in
      fun {FormatDate Date}
         WeekDays.(Date.wDay + 1)#', '#
         Months.(Date.mon + 1)#' '#
         Date.mDay#{CondSelect Ordinals Date.mDay 'th'}#' '#
         (Date.year + 1900)#', '#
         {TwoDig Date.hour}#':'#
         {TwoDig Date.min}#':'#
         {TwoDig Date.sec}
      end
   end

   fun {Spaces N}
      if N =< 0 then ""
      else & |{Spaces N - 1}
      end
   end

   proc {WriteFile VS File}
      F = {New Open.file init(name: File flags: [write create truncate])}
   in
      {F write(vs: VS)}
      {F close()}
   end

   class OzDocToHTML
      attr
         % fontification:
         FontifyMode: unit
         MyFontifier: unit
         % constructing the output:
         OutputDirectory: unit CurrentNode: unit NodeCounter: unit
         ToWrite: unit
         % managing common attributes:
         Common: unit BodyCommon: unit
         ProgLang: unit
         % front matter:
         TopTitle: unit
         MyAuthorDB: unit
         Authors: unit
         Meta: unit
         Comic: unit
         Abstract: unit
         StyleSheet: unit
         % main matter:
         TOC: unit TOCMode: unit
         Part: unit Chapter: unit Section: unit SubSection: unit
         Appendix: unit
         Labels: unit ToGenerate: unit
         % for Math and Math.Choice:
         MathDisplay: unit
         MyMathToGIF: unit
         % for Picture:
         PictureDisplay: unit
         MyThumbnails: unit
         % for Figure:
         Floats: unit
         FigureCounters: unit
         % for Grammar:
         GrammarHead: unit
         GrammarAltType: unit
         GrammarNote: unit
         % for List:
         InDescription: unit
         OLTypes: (X='1'|'a'|'i'|'A'|'I'|X in X)
         % back matter:
         MyBibliographyDB: unit
         BibNode: unit
      meth init(Mode SGML Args)
         FontifyMode <- Mode
         StyleSheet <- {Property.get 'ozdoc.stylesheet'}
         MyFontifier <- {New Fontifier.'class' init()}
         OutputDirectory <- Args.'out'
         {OS.system "mkdir -p "#@OutputDirectory _}   %--** OS.mkDir
         MyThumbnails <- {New Thumbnails.'class' init(@OutputDirectory)}
         MyMathToGIF <- if Args.'latexmath' then
                           {New MathToGIF.'class' init(@OutputDirectory)}
                        else unit
                        end
         CurrentNode <- "index.html"
         NodeCounter <- 0
         ToWrite <- nil
         ProgLang <- Fontifier.noProgLang
         Labels <- {NewDictionary}
         ToGenerate <- nil
         OzDocToHTML, Process(SGML unit)
         OzDocToHTML, GenerateLabels()
         {ForAll @ToWrite
          proc {$ Node#File}
             {WriteFile
              '<!DOCTYPE html PUBLIC '#DOCTYPE_PUBLIC#'>\n'#
              {HTML.toVirtualString Node} File}
          end}
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
               PCDATA(S)|OzDocToHTML, BatchSub(M I + 1 $)
            [] nil then
               OzDocToHTML, BatchSub(M I + 1 $)
            elseof N then
               OzDocToHTML, Process(N $)|OzDocToHTML, BatchSub(M I + 1 $)
            end
         else nil
         end
      end
      meth BatchCode(M I $)
         SEQ(OzDocToHTML, BatchCodeSub(M I $))
      end
      meth BatchCodeSub(M I $)
         %--** interpret <Span class=ignore>...</Span> differently
         if {HasFeature M I} then
            case M.I of S=_|_ then VS in
               {@MyFontifier enqueueVirtualString(@ProgLang S '<BR>' ?VS)}
               code(COMMON: @Common 'class': [M.display]
                    VERBATIM(VS))|   %--** VERBATIM?
               OzDocToHTML, BatchCodeSub(M I + 1 $)
            [] nil then
               OzDocToHTML, BatchCodeSub(M I + 1 $)
            elseof N then
               OzDocToHTML, Process(N $)|OzDocToHTML, BatchCodeSub(M I + 1 $)
            end
         else nil
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
            [] ellipsis then
               VERBATIM('...')
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
            [] 'PI:LATEX' then
               VERBATIM('LaTeX')
            [] 'PI:EG' then
               VERBATIM('e.&nbsp;g.')
            else
               {Exception.raiseError
                ozDoc(sgmlToHTML unsupportedProcessingInstruction M.1)} unit
            end
         else
            %-----------------------------------------------------------
            % Document Structure
            %-----------------------------------------------------------
            case Tag of book then HTML in
               Floats <- nil
               FigureCounters <- {NewDictionary}
               MyBibliographyDB <- {New BibliographyDB.'class'
                                    init(@OutputDirectory)}
               BibNode <- _
               HTML = [OzDocToHTML, Process(M.1=front(...) $)
                       if {HasFeature M 3} then
                          OzDocToHTML, Process(M.3=back(...) $)
                       else EMPTY
                       end
                       OzDocToHTML, Process(M.2='body'(...) $)
                       case {@MyBibliographyDB process($)} of unit then EMPTY
                       elseof VS then Title Label X HTML1 HTML in
                          Title = PCDATA('Bibliography')
                          OzDocToHTML, PrepareBibNode(?X ?HTML1)
                          ToGenerate <- Label|@ToGenerate
                          TOC <- {Append @TOC [2#Label#@CurrentNode#Title]}
                          @BibNode = @CurrentNode
                          HTML = SEQ([HTML1
                                      h1(a(name: Label Title))
                                      VERBATIM(VS)])   %--** VERBATIM?
                          OzDocToHTML, FinishNode(Title X HTML $)
                       end]
               OzDocToHTML, MakeNode(@TopTitle SEQ(HTML))
               {@MyFontifier process(case @FontifyMode
                                     of color then 'html-color'
                                     [] mono then 'html-mono'
                                     [] stylesheets then 'html-stylesheets'
                                     end)}
               unit
            %-----------------------------------------------------------
            % Front and Back Matter
            %-----------------------------------------------------------
            [] front then HTML in
               Authors <- nil
               MyAuthorDB <- {New AuthorDB.'class' init()}
               Meta <- {NewDictionary}
               OzDocToHTML, Batch(M 1 ?HTML)
               'div'(COMMON: @Common
                     h1(align: center 'class': [title] @TopTitle)
                     HTML
                     case @Authors of nil then EMPTY
                     else
                        h2(align: center 'class': [authors]
                           OzDocToHTML, FormatAuthors($))
                     end
                     case @Comic of unit then EMPTY
                     elseof M then p(OzDocToHTML, Process(M $))
                     end
                     case @Abstract of unit then EMPTY
                     elseof A then A
                     end)
            [] title then
               TopTitle <- span(COMMON: @Common OzDocToHTML, Batch(M 1 $))
               EMPTY
            [] author then
               Authors <- {Append @Authors
                           [author(name: OzDocToHTML, Process(M $))]}
               EMPTY
            [] 'author.extern' then
               Authors <- {Append @Authors
                           [{@MyAuthorDB get(M.to M.key $)}]}
               EMPTY
            [] meta then
               {Dictionary.put @Meta M.name
                {Append {Dictionary.condGet @Meta M.name nil}
                 [{String.toAtom M.value}]}}
               EMPTY
            [] abstract then
               Abstract <- blockquote(COMMON: @Common
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
            [] 'body' then
               BodyCommon <- @Common
               TOC <- nil
               TOCMode <- false
               Part <- 0
               Chapter <- 0
               Appendix <- false
               OzDocToHTML, Batch(M 1 $)
            [] part then Title X HTML1 HTML2 HTML3 HTML4 NodeTitle in
               Title = case {Label M.1} of title then
                          OzDocToHTML, Batch(M.1 1 $)
                       else unit
                       end
               OzDocToHTML, PrepareNode(M ?X ?HTML1)
               OzDocToHTML, MakeTitle(""
                                      fun {$}
                                         Part <- @Part + 1
                                         'Part&nbsp;'#{RomanU @Part}
                                      end
                                      ': ' Title
                                      fun {$ N}
                                         h1(align: center 'class': [part] N)
                                      end
                                      M 1 ?HTML2 ?NodeTitle)
               OzDocToHTML, Batch(M 2 ?HTML3)
               OzDocToHTML, FinishNode(NodeTitle X 'div'(COMMON: @Common
                                                         HTML2 HTML3) ?HTML4)
               SEQ([HTML1 HTML4])
            [] chapter then Title X HTML1 HTML2 HTML3 HTML4 NodeTitle in
               Title = case {Label M.1} of title then
                          OzDocToHTML, Batch(M.1 1 $)
                       else unit
                       end
               OzDocToHTML, PrepareNode(M ?X ?HTML1)
               OzDocToHTML, MakeTitle('Chapter&nbsp;'
                                      fun {$}
                                         Chapter <- @Chapter + 1
                                         Section <- 0
                                         FigureCounters <- {NewDictionary}
                                         @Chapter
                                      end
                                      ' ' Title
                                      fun {$ N} h1(N) end
                                      M 2 ?HTML2 ?NodeTitle)
               OzDocToHTML, Batch(M 2 ?HTML3)
               OzDocToHTML, FinishNode(NodeTitle X 'div'(COMMON: @Common
                                                         HTML2 HTML3) ?HTML4)
               SEQ([HTML1 HTML4])
            [] appendix then Title X HTML1 HTML2 HTML3 HTML4 NodeTitle in
               Title = case {Label M.1} of title then
                          OzDocToHTML, Batch(M.1 1 $)
                       else unit
                       end
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
                                      ' ' Title
                                      fun {$ N} h1(N) end
                                      M 2 ?HTML2 ?NodeTitle)
               OzDocToHTML, Batch(M 2 ?HTML3)
               OzDocToHTML, FinishNode(NodeTitle X 'div'(COMMON: @Common
                                                         HTML2 HTML3) ?HTML4)
               SEQ([HTML1 HTML4])
            [] section then Title X HTML1 HTML2 HTML3 HTML4 NodeTitle in
               Title = case {Label M.1} of title then
                          OzDocToHTML, Batch(M.1 1 $)
                       else unit
                       end
               OzDocToHTML, PrepareNode(M ?X ?HTML1)
               OzDocToHTML, MakeTitle('Section&nbsp;'
                                      fun {$}
                                         Section <- @Section + 1
                                         SubSection <- 0
                                         if @Appendix then {Alpha @Chapter}
                                         else @Chapter
                                         end#'.'#@Section
                                      end
                                      ' ' Title
                                      fun {$ N} h2(N) end
                                      M 3 ?HTML2 ?NodeTitle)
               OzDocToHTML, Batch(M 2 ?HTML3)
               OzDocToHTML, FinishNode(NodeTitle X 'div'(COMMON: @Common
                                                         HTML2 HTML3) ?HTML4)
               SEQ([HTML1 HTML4])
            [] subsection then Title X HTML1 HTML2 HTML3 HTML4 NodeTitle in
               Title = case {Label M.1} of title then
                          OzDocToHTML, Batch(M.1 1 $)
                       else unit
                       end
               OzDocToHTML, PrepareNode(M ?X ?HTML1)
               OzDocToHTML, MakeTitle('Section&nbsp;'
                                      fun {$}
                                         SubSection <- @SubSection + 1
                                         if @Appendix then {Alpha @Chapter}
                                         else @Chapter
                                         end#'.'#@Section#'.'#@SubSection
                                      end
                                      ' ' Title
                                      fun {$ N} h3(N) end
                                      M 4 ?HTML2 ?NodeTitle)
               OzDocToHTML, Batch(M 2 ?HTML3)
               OzDocToHTML, FinishNode(NodeTitle X 'div'(COMMON: @Common
                                                         HTML2 HTML3) ?HTML4)
               SEQ([HTML1 HTML4])
            [] subsubsection then Title in
               Title = case {Label M.1} of title then
                          OzDocToHTML, Batch(M.1 1 $)
                       else unit
                       end
               'div'(COMMON: @Common
                     h4(Title)
                     OzDocToHTML, Batch(M 2 $))
            %-----------------------------------------------------------
            % Paragraphs
            %-----------------------------------------------------------
            [] p then
               p(COMMON: @Common
                 if {SGML.isOfClass M warning} then strong(PCDATA('Warning:'))
                 else EMPTY
                 end
                 OzDocToHTML, Batch(M 1 $))
            [] para then
               %--** copy common attributes from title element to the p element
               SEQ([p('class': [margin]
                      OzDocToHTML, Batch(M.1=title(...) 1 $))
                    p(COMMON: @Common
                      OzDocToHTML, Batch(M 2 $))])
            [] 'div' then
               'div'(COMMON: @Common
                     OzDocToHTML, Batch(M 1 $))
            %-----------------------------------------------------------
            % Lists
            %-----------------------------------------------------------
            [] list then
               %--** display attribute
               if {HasFeature M continues} then
                  {Exception.raiseError
                   ozDoc(sgmlToHTML notImplemented M continues)}   %--**
               end
               if {Label M.1} == entry then X HTML in
                  X = @InDescription
                  InDescription <- true
                  OzDocToHTML, Batch(M 1 ?HTML)
                  InDescription <- X
                  BLOCK(dl(COMMON: @Common HTML))
               elseif {HasFeature M enum} then X Y HTML in
                  X = @InDescription
                  InDescription <- false
                  Y = @OLTypes
                  OLTypes <- Y.2
                  OzDocToHTML, Batch(M 1 ?HTML)
                  OLTypes <- Y
                  InDescription <- X
                  BLOCK(ol(COMMON: @Common
                           type: @OLTypes.1
                           start: {CondSelect M n unit}
                           HTML))
               elseif {HasFeature M n} then
                  {Exception.raiseError
                   ozDoc(sgmlToHTML illegalAttributes M)} unit
               else X HTML in
                  X = @InDescription
                  InDescription <- false
                  OzDocToHTML, Batch(M 1 ?HTML)
                  InDescription <- X
                  BLOCK(ul(COMMON: @Common HTML))
               end
            [] entry then HTML1 ClassName HTML2 in
               OzDocToHTML, Batch(M.1 1 ?HTML1)
               ClassName = {FoldLTail
                            {FoldR EntryClasses
                             fun {$ C#T In}
                                if {SGML.isOfClass M C} then T|In else In end
                             end nil}
                            fun {$ In T|Tr}
                               In#T#case Tr of nil then "" else ' ' end
                            end ""}
               HTML2 = case ClassName of "" then EMPTY
                       else
                          SEQ([VERBATIM('&nbsp;')
                               PCDATA('[') i(PCDATA(ClassName)) PCDATA(']')])
                       end
               dt(COMMON: @Common HTML1 HTML2)
            [] synopsis then
               dd(COMMON: @Common
                  'class': [synopsis]
                  blockquote(OzDocToHTML, Batch(M 1 $)))
            [] item then
               if @InDescription then
                  dd(COMMON: @Common OzDocToHTML, Batch(M 1 $))
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
                  case @MyMathToGIF of unit then
                     HTML = code(PCDATA(M.1))
                  else
                     {@MyMathToGIF convertLaTeX(M.1 Display ?FileName)}
                     HTML = img(src: FileName alt: M.1)
                  end
               [] 'HTML' then
                  HTML = VERBATIM(M.1)
               else
                  {Exception.raiseError
                   ozDoc(sgmlToHTML unsupportedMathNotation M)}   %--**
               end
               case Display of display then
                  BLOCK(blockquote(COMMON: @Common p(HTML)))
               [] inline then
                  span(COMMON: @Common HTML)
               end
            [] 'math.extern' then
               {Exception.raiseError
                ozDoc(sgmlToHTML unsupported M)} unit   %--**
            [] 'math.choice' then HTML in
               MathDisplay <- M.display
               OzDocToHTML, Process(M.1 ?HTML)   %--** make better choice
               MathDisplay <- unit
               HTML
            %-----------------------------------------------------------
            % Picture Element
            %-----------------------------------------------------------
            [] picture then
               {Exception.raiseError
                ozdoc(sgmlToHTML unsupported M)} unit   %--**
            [] 'picture.extern' then
               case {CondSelect M type unit} of 'gif' then Display HTML in
                  Display = case @PictureDisplay of unit then M.display
                            elseof X then X
                            end
                  HTML = if {SGML.isOfClass M thumbnail} then ThumbnailName in
                            {@MyThumbnails get(M.to ?ThumbnailName)}
                            a(href: M.to
                              img(COMMON: @Common src: ThumbnailName alt: ''))
                         else
                            img(COMMON: @Common src: M.to alt: '')
                         end
                  case Display of display then Align in
                     Align = if {SGML.isOfClass M left} then left
                             elseif {SGML.isOfClass M right} then right
                             else center
                             end
                     BLOCK('div'(align: Align HTML))
                  [] inline then HTML
                  end
               [] unit then
                  %--** the notation should be derived from the file name
                  {Exception.raiseError
                   ozDoc(sgmlToHTML unspecifiedPictureNotation M)} unit
               else
                  {Exception.raiseError
                   ozDoc(sgmlToHTML unsupportedPictureNotation M)} unit   %--**
               end
            [] 'picture.choice' then HTML in
               %--** propagate common attributes
               PictureDisplay <- M.display
               OzDocToHTML, Process(M.1 ?HTML)   %--** make better choice
               PictureDisplay <- unit
               HTML
            %-----------------------------------------------------------
            % Code
            %-----------------------------------------------------------
            [] code then HTML in
               OzDocToHTML, BatchCode(M 1 ?HTML)
               case M.display of display then BLOCK(blockquote(p(HTML)))
               [] inline then HTML
               end
            [] 'code.extern' then HTML in
               %--** class=linenumbers
               HTML = VERBATIM({@MyFontifier   %--** VERBATIM?
                                enqueueFile(@ProgLang M.to '\n' $)})
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
                       OzDocToHTML, Batch(M 1 $))
               [] meta then
                  i(COMMON: @Common OzDocToHTML, Batch(M 1 $))
               [] env then
                  code(COMMON: @Common OzDocToHTML, Batch(M 1 $))
               [] grammar then
                  span(COMMON: @Common
                       PCDATA('<')
                       i(OzDocToHTML, Batch(M 1 $))
                       PCDATA('>'))
               end
            %-----------------------------------------------------------
            % Cross References
            %-----------------------------------------------------------
            [] ref then
               {Exception.raiseError
                ozDoc(sgmlToHTML unsupported M)} unit   %--**
            [] 'ref.extern' then
               %--** key attribute?
               a(COMMON: @Common href: M.to OzDocToHTML, Batch(M 1 $))
            [] ptr then Node HTML in
               OzDocToHTML, ID(M.to ?Node ?HTML)
               a(COMMON: @Common href: Node#"#"#M.to HTML)
            [] 'ptr.extern' then
               {Exception.raiseError
                ozDoc(sgmlToHTML unsupported M)} unit   %--**
            %-----------------------------------------------------------
            % Phrasal Elements
            %-----------------------------------------------------------
            [] file then
               code(COMMON: @Common OzDocToHTML, Batch(M 1 $))
            [] kbd then
               kbd(COMMON: @Common OzDocToHTML, Batch(M 1 $))
            [] key then
               {Exception.raiseError
                ozDoc(sgmlToHTML unsupported M)} unit   %--**
            [] samp then
               code(COMMON: @Common OzDocToHTML, Batch(M 1 $))
            [] name then
               span(COMMON: @Common OzDocToHTML, Batch(M 1 $))
            [] q then
               %--** use different quotes, depending on class and/or lang?
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
            [] span then
               span(COMMON: @Common OzDocToHTML, Batch(M 1 $))
            [] def then
               em(COMMON: @Common OzDocToHTML, Batch(M 1 $))
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
               {Exception.raiseError ozDoc(sgmlToHTML internalError M)} unit
            %-----------------------------------------------------------
            % Note
            %-----------------------------------------------------------
            [] note then
               {Exception.raiseError
                ozDoc(sgmlToHTML unsupported M)} unit   %--**
            %-----------------------------------------------------------
            % Index
            %-----------------------------------------------------------
            [] index then
               {Exception.raiseError
                ozDoc(sgmlToHTML unsupported M)} unit   %--**
            [] and then
               {Exception.raiseError
                ozDoc(sgmlToHTML unsupported M)} unit   %--**
            [] see then
               {Exception.raiseError
                ozDoc(sgmlToHTML unsupported M)} unit   %--**
            %-----------------------------------------------------------
            % BNF Markup
            %-----------------------------------------------------------
            [] 'grammar.rule' then X HTML in
               %--** respect the display attribute?
               X = @GrammarAltType#@GrammarHead
               GrammarAltType <- def
               GrammarHead <- OzDocToHTML, Process(M.1 $)
               OzDocToHTML, Batch(M 2 ?HTML)
               GrammarAltType <- X.1
               GrammarHead <- X.2
               BLOCK(table(COMMON: @Common
                           border: 0 cellpadding: 0 cellspacing: 0
                           HTML))
            [] 'grammar.head' then
               span(COMMON: @Common OzDocToHTML, Batch(M 1 $))
            [] 'grammar.alt' then HTML1 HTML2 in
               HTML1 = case {CondSelect M type @GrammarAltType} of def then
                          SEQ([td(@GrammarHead)
                               td(align: center VERBATIM('&nbsp;::=&nbsp;'))])
                       [] add then
                          SEQ([td(@GrammarHead)
                               td(align: center VERBATIM('&nbsp;+=&nbsp;'))])
                       [] 'or' then
                          SEQ([td()
                               td(align: center VERBATIM('&nbsp;|&nbsp;'))])
                       [] space then
                          SEQ([td() td()])
                       end
               GrammarAltType <- 'or'
               OzDocToHTML, Batch(M 1 ?HTML2)
               tr(COMMON: @Common valign: top
                  HTML1
                  td(HTML2)
                  case @GrammarNote of unit then EMPTY
                  elseof N then GrammarNote <- unit N
                  end)
            [] 'grammar.note' then
               GrammarNote <- td(COMMON: @Common align: left
                                 i(VERBATIM('&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;% ')
                                   OzDocToHTML, Batch(M 1 $)))
               EMPTY
            [] 'grammar' then
               'div'(COMMON: @Common OzDocToHTML, Batch(M 1 $))
            %-----------------------------------------------------------
            % Tables
            %-----------------------------------------------------------
            [] table then Title Mr in
               %--** display attribute
               Title = {SGML.getSubtree M title ?Mr}
               BLOCK(table(COMMON: @Common
                           align: center
                           border: if {SGML.isOfClass M dyptic} then 0
                                   else 1
                                   end
                           case Title of unit then EMPTY
                           else
                              p(align: center b(OzDocToHTML, Batch(Title 1 $)))
                           end
                           OzDocToHTML, Batch(Mr 1 $)))
            [] tr then
               tr(COMMON: @Common valign: top OzDocToHTML, Batch(M 1 $))
            [] th then
               th(COMMON: @Common
                  colspan: {CondSelect M colspan unit}
                  OzDocToHTML, Batch(M 1 $))
            [] td then
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
            %--** ozdoc.sgml Specials
            %-----------------------------------------------------------
            [] tag then
               code(COMMON: @Common OzDocToHTML, Batch(M 1 $))
            [] attrib then
               code(COMMON: @Common OzDocToHTML, Batch(M 1 $))
            else
               {Exception.raiseError
                ozDoc(sgmlToHTML unsupported M)} unit   %--**
            end
         end = Res
         OzDocToHTML, PopCommon(OldCommon)
         Res
      end
      meth MakeTitle(PtrText FormatNumber Sep Title LayoutTitle M Level
                     HTML ?NodeTitle) HTML1 Label Res in
         OzDocToHTML, FlushFloats(?HTML1)
         if {HasFeature M id} then
            Label = M.id
         else
            ToGenerate <- Label|@ToGenerate
         end
         Res = a(name: Label NodeTitle)
         if {SGML.isOfClass M unnumbered} then
            if Title == unit then
               {Exception.raiseError ozDoc(sgmlToHTML emptySectionTitle M)}
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
               OzDocToHTML, ID(M.id @CurrentNode VERBATIM(PtrText#NumberVS))
            end
         end
         TOC <- {Append @TOC [Level#Label#@CurrentNode#NodeTitle]}
         HTML = SEQ([HTML1 {LayoutTitle Res}])
      end
      meth ID(L Node HTML)
         if {Dictionary.member @Labels L} then
            Node#HTML = {Dictionary.get @Labels L}
         else
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
         if {Member {CondSelect M id unit}
             {Dictionary.condGet @Meta 'html.split' nil}}
         then
            X = @CurrentNode#@TOC#@TOCMode
            NodeCounter <- @NodeCounter + 1
            CurrentNode <- 'node'#@NodeCounter#'.html'
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
      meth PrepareBibNode(?X ?HTML)
         if {Dictionary.member @Meta 'html.split.bib'} then
            X = @CurrentNode#@TOC#@TOCMode
            CurrentNode <- 'bib.html'
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
         [] C#T#M then Res in
            OzDocToHTML, MakeNode(Title HTML)
            CurrentNode <- C
            Res = if M then {FormatTOC @TOC}
                  else SEQ([hr() {FormatTOC @TOC}])
                  end
            TOC <- {Append T @TOC}
            TOCMode <- true
            Res
         end
      end
      meth MakeNode(Title BodyContents) Node in
         %--** convert Title to simple text (it might contain tags!)
         Node = html(head(title(Title)
                          link(rel: stylesheet
                               type: 'text/css'
                               href: @StyleSheet))
                     'body'(COMMON: @BodyCommon
                            %--** navigation panel
                            BodyContents
                            OzDocToHTML, FlushFloats($)
                            hr()
                            address(case @Authors of nil then EMPTY
                                    else As in
                                       OzDocToHTML, FormatAuthors(?As)
                                       SEQ([As br()])
                                    end
                                    PCDATA('Generated on '#
                                           {FormatDate {OS.localTime}}))))
         ToWrite <- Node#(@OutputDirectory#'/'#@CurrentNode)|@ToWrite
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
      meth OutputFigure(M $) Class N Number Title Mr1 Caption Mr2 in
         Class = {CondSelect M 'class' ['figure']}.1
         N = {Dictionary.condGet @FigureCounters Class 0} + 1
         {Dictionary.put @FigureCounters Class N}
         Number = ({InitialCapital {Atom.toString Class}}#'&nbsp;'#
                   if @Appendix then {Alpha @Chapter}
                   else @Chapter
                   end#'.'#N)
         Title = {SGML.getSubtree M title ?Mr1}
         Caption = {SGML.getSubtree Mr1 caption ?Mr2}
         'div'(COMMON: @Common
               hr()
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
                  p(strong(VERBATIM(Number#'.')))
               else
                  p(strong(VERBATIM(Number#':')) PCDATA(' ')
                    OzDocToHTML, Batch(Caption.1 1 $))
               end
               hr())
      end
      meth FormatAuthors($)
         case @Authors of nil then ""
         [] A|Ar then
            fun {FormatAuthor A} N H in
               N = {FoldLTail [{CondSelect A firstname unit}
                               {CondSelect A middlename unit}
                               {CondSelect A lastname unit}]
                    fun {$ In X|Xr}
                       case X of unit then In
                       else In#X#case Xr of nil then "" else ' ' end
                       end
                    end ""}
               H = if {HasFeature A www} then A.www
                   elseif {HasFeature A email} then 'email:'#A.email
                   else unit
                   end
               case H of unit then PCDATA(N)
               else a(href: H PCDATA(N))
               end
            end
         in
            {FoldLTail Ar
             fun {$ In A|Ar}
                SEQ([In
                     PCDATA(case Ar of nil then ' and' else ',' end)
                     {FormatAuthor A}])
             end {FormatAuthor A}}
         end
      end
   end

   proc {Translate Mode Args}
      {New OzDocToHTML init(Mode {SGML.parse Args.'in'} Args) _}
   end
end
