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
   SGML(parse namePI getSubtree isOfClass)
   OS(system localTime)
   Open(file)
   AuthorDB('class')
   BibliographyDB('class')
   Fontifier('class' noProgLang)
export
   Translate
define
   fun {InitialCapital S}
      case S of C|Cr then {Char.toUpper C}|Cr
      [] nil then ""
      end
   end

   fun {MakeCDATA S}
      '"'#S#'"'   %--** quotation marks etc.
   end

   fun {MakePCDATA S}
      case S of C|Cr then
         case C of &< then &&|&l|&t|&;|{MakePCDATA Cr}
         [] &> then &&|&g|&t|&;|{MakePCDATA Cr}
         [] && then &&|&a|&m|&p|&;|{MakePCDATA Cr}
         [] &" then &&|&q|&u|&o|&t|&;|{MakePCDATA Cr}
         else C|{MakePCDATA Cr}
         end
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
      fun {FormatTOCLevel TOC Level TopLevel}
         case TOC of Entry|TOCr then N#Label#Node#Text = Entry in
            if N < Level then
               '</UL>\n'#{FormatTOCLevel TOC Level - 1 TopLevel}
            elseif N > Level then
               '<UL>\n'#{FormatTOCLevel TOC Level + 1 TopLevel}
            else
               '<LI><A href='#thread {MakeCDATA Node#"#"#Label} end#'>'#
               Text#'</A>\n'#{FormatTOCLevel TOCr Level TopLevel}
            end
         [] nil then
            if Level > TopLevel then
               '</UL>\n'#{FormatTOCLevel TOC Level - 1 TopLevel}
            else ""
            end
         end
      end
   in
      fun {FormatTOC TOC}
         case TOC of Entry|_ then N#_#_#_ = Entry in
            '<UL>\n'#
            {FormatTOCLevel TOC N N}#
            '</UL>\n'
         [] nil then ""
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
         IsColor: unit
         MyFontifier: unit
         % constructing the output:
         OutputDirectory: unit CurrentNode: unit NodeCounter: unit
         ToWrite: unit
         Out: unit
         % managing common attributes:
         Targets: unit
         ProgLang: unit
         % front matter:
         Title: unit
         MyAuthorDB: unit
         Authors: unit
         Meta: unit
         Abstract: unit
         % main matter:
         TOC: unit TOCMode: unit
         Part: unit Chapter: unit Section: unit SubSection: unit
         Appendix: unit
         Labels: unit ToGenerate: unit
         % for Math and Math.Choice:
         MathDisplay: unit
         % for Figure:
         Floats: unit
         FigureCounters: unit
         % for List:
         InDescription: unit
         % for GrammarAlt:
         GrammarAltIndent: unit
         % back matter:
         MyBibliographyDB: unit
      meth init(B SGML Dir)
         IsColor <- B
         MyFontifier <- {New Fontifier.'class' init()}
         OutputDirectory <- Dir
         CurrentNode <- "index.html"
         NodeCounter <- 0
         ToWrite <- nil
         Out <- ""
         ProgLang <- [Fontifier.noProgLang]
         Labels <- {NewDictionary}
         ToGenerate <- nil
         OzDocToHTML, Process(SGML)
         OzDocToHTML, EndNode()
         OzDocToHTML, GenerateLabels()
         {OS.system "mkdir -p "#Dir _}   %--** {OS.mkDir Dir}
         {ForAll @ToWrite
          proc {$ VS#File}
             {WriteFile VS File}
          end}
      end
      meth PushCommon(M)
         case {CondSelect M proglang unit} of unit then skip
         elseof X then
            ProgLang <- X|@ProgLang
         end
      end
      meth PopCommon(M)
         if {HasFeature M proglang} then
            ProgLang <- @ProgLang.2
         end
      end
      meth Batch(M I)
         if {HasFeature M I} then
            case M.I of S=_|_ then
               Out <- @Out#{MakePCDATA S}
            [] nil then skip
            elseof N then
               OzDocToHTML, Process(N)
            end
            OzDocToHTML, Batch(M I + 1)
         else skip
         end
      end
      meth BatchCode(M I)
         if {HasFeature M I} then
            case M.I of S=_|_ then
               %--** support <Span class=ignore>...</Span>
               Out <- @Out#('<CODE>'#
                            {@MyFontifier
                             enqueueVirtualString(@ProgLang.1 S '<BR>' $)}#
                            '</CODE>')
            [] nil then skip
            elseof N then
               OzDocToHTML, Process(N)
            end
            OzDocToHTML, BatchCode(M I + 1)
         else skip
         end
      end
      meth BatchMathTex(M I)
         if {HasFeature M I} then
            case M.I of S=_|_ then
               Out <- @Out#('<CODE>'#
                            {MakePCDATA S}#   %--** try to translate
                            '</CODE>')
            [] nil then skip
            elseof N then
               OzDocToHTML, Process(N)
            end
            OzDocToHTML, BatchCode(M I + 1)
         else skip
         end
      end
      meth Excursion(M ?VS) Saved in
         Saved = @Out
         Out <- ""
         OzDocToHTML, Batch(M 1)
         VS = @Out
         Out <- Saved
      end
      meth Process(M)
         Tag = case {Label M} of sect0 then chapter
               [] sect1 then section
               [] sect2 then subsection
               [] sect3 then subsubsection
               [] 'p.silent' then p
               [] 'p.level' then p
               elseof X then X
               end
      in
         OzDocToHTML, PushCommon(M)
         %--------------------------------------------------------------
         % Processing Instructions
         %--------------------------------------------------------------
         if Tag == SGML.namePI then
            case M.1 of emdash then
               Out <- @Out#' - '
            [] endash then
               Out <- @Out#'-'
            [] nbsp then
               Out <- @Out#'&nbsp;'
            [] ellipsis then
               Out <- @Out#'...'
            [] slash then
               Out <- @Out#'/'
            [] ie then
               Out <- @Out#'i.&nbsp;e.'
            [] wrt then
               Out <- @Out#'w.&nbsp;r.&nbsp;t.'
            [] eg then
               Out <- @Out#'e.&nbsp;g.'
            else
               {Exception.raiseError
                ozDoc(sgmlToHTML unsupportedProcessingInstruction M.1)}
            end
         else
            %-----------------------------------------------------------
            % Document Structure
            %-----------------------------------------------------------
            case Tag of book then
               Floats <- nil
               FigureCounters <- {NewDictionary}
               OzDocToHTML, Process(M.1=front(...))
               if {HasFeature M 3} then
                  OzDocToHTML, Process(M.3=back(...))
               end
               OzDocToHTML, Process(M.2='body'(...))
               {@MyFontifier process(if @IsColor then 'html-color'
                                     else 'html-mono'
                                     end)}
            %-----------------------------------------------------------
            % Front and Back Matter
            %-----------------------------------------------------------
            [] front then
               Authors <- nil
               MyAuthorDB <- {New AuthorDB.'class' init()}
               Meta <- {NewDictionary}
               OzDocToHTML, Batch(M 1)
               OzDocToHTML, StartNode(@Title)
               Out <- @Out#'<H1 align=center>'#@Title#'</H1>\n'
               case @Authors of nil then skip
               else
                  Out <- @Out#('\n<H2 align=center>\n'#
                               OzDocToHTML, FormatAuthors($)#
                               '</H2>')
               end
               case @Abstract of unit then skip
               elseof A then
                  Out <- @Out#('<BLOCKQUOTE>\n'#
                               A#
                               '</BLOCKQUOTE>\n')
               end
            [] title then
               Title <- OzDocToHTML, Excursion(M $)
            [] author then
               Authors <- {Append @Authors
                           [author(name: OzDocToHTML, Excursion(M $))]}
            [] 'author.extern' then
               Authors <- {Append @Authors
                           [{@MyAuthorDB get(M.to M.key $)}]}
            [] meta then
               {Dictionary.put @Meta M.name
                {Append {Dictionary.condGet @Meta M.name nil}
                 [{String.toAtom M.value}]}}
            [] abstract then
               Abstract <- OzDocToHTML, Excursion(M $)
            [] back then
               MyBibliographyDB <- {New BibliographyDB.'class' init()}
               OzDocToHTML, Batch(M 1)
            [] 'bib.extern' then BibEntry in
               {@MyBibliographyDB get(M.to M.key ?BibEntry)}
               %--** parametrize: this might be called other than bib.html
               OzDocToHTML, ID(M.id 'bib.html' '['#BibEntry.key#']')
            %-----------------------------------------------------------
            % Body and Sectioning Elements
            %-----------------------------------------------------------
            [] 'body' then
               TOC <- nil
               TOCMode <- false
               Part <- 0
               Chapter <- 0
               Appendix <- false
               OzDocToHTML, Batch(M 1)
            [] part then Title X in
               Title = case {Label M.1} of title then
                          OzDocToHTML, Excursion(M.1 $)
                       else unit
                       end
               OzDocToHTML, CheckNode(M Title ?X)
               OzDocToHTML, MakeTitle(""
                                      fun {$}
                                         Part <- @Part + 1
                                         'Part&nbsp;'#{RomanU @Part}
                                      end
                                      ': ' Title
                                      fun {$ VS}
                                         '<H1 align=center>'#VS#'</H1>\n'
                                      end
                                      M 1)
               OzDocToHTML, Batch(M 2)
               OzDocToHTML, PopNode(X)
            [] chapter then Title X in
               Title = case {Label M.1} of title then
                          OzDocToHTML, Excursion(M.1 $)
                       else unit
                       end
               OzDocToHTML, CheckNode(M Title ?X)
               OzDocToHTML, MakeTitle('Chapter&nbsp;'
                                      fun {$}
                                         Chapter <- @Chapter + 1
                                         Section <- 0
                                         FigureCounters <- {NewDictionary}
                                         @Chapter
                                      end
                                      ' ' Title
                                      fun {$ VS} '<H2>'#VS#'</H2>\n' end
                                      M 2)
               OzDocToHTML, Batch(M 2)
               OzDocToHTML, PopNode(X)
            [] appendix then Title X in
               Title = case {Label M.1} of title then
                          OzDocToHTML, Excursion(M.1 $)
                       else unit
                       end
               OzDocToHTML, CheckNode(M Title ?X)
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
                                      fun {$ VS} '<H2>'#VS#'</H2>\n' end
                                      M 2)
               OzDocToHTML, Batch(M 2)
               OzDocToHTML, PopNode(X)
            [] section then Title X in
               Title = case {Label M.1} of title then
                          OzDocToHTML, Excursion(M.1 $)
                       else unit
                       end
               OzDocToHTML, CheckNode(M Title ?X)
               OzDocToHTML, MakeTitle('Section&nbsp;'
                                      fun {$}
                                         Section <- @Section + 1
                                         SubSection <- 0
                                         if @Appendix then {Alpha @Chapter}
                                         else @Chapter
                                         end#'.'#@Section
                                      end
                                      ' ' Title
                                      fun {$ VS} '<H3>'#VS#'</H3>\n' end
                                      M 3)
               OzDocToHTML, Batch(M 2)
               OzDocToHTML, PopNode(X)
            [] subsection then Title X in
               Title = case {Label M.1} of title then
                          OzDocToHTML, Excursion(M.1 $)
                       else unit
                       end
               OzDocToHTML, CheckNode(M Title ?X)
               OzDocToHTML, MakeTitle('Section&nbsp;'
                                      fun {$}
                                         SubSection <- @SubSection + 1
                                         if @Appendix then {Alpha @Chapter}
                                         else @Chapter
                                         end#'.'#@Section#'.'#@SubSection
                                      end
                                      ' ' Title
                                      fun {$ VS} '<H4>'#VS#'</H4>\n' end
                                      M 4)
               OzDocToHTML, Batch(M 2)
               OzDocToHTML, PopNode(X)
            [] subsubsection then Title in
               Title = case {Label M.1} of title then
                          OzDocToHTML, Excursion(M.1 $)
                       else unit
                       end
               Out <- @Out#'<H5>'#Title#'</H5>\n'
               OzDocToHTML, Batch(M 2)
            %-----------------------------------------------------------
            % Paragraphs
            %-----------------------------------------------------------
            [] p then
               Out <- @Out#'<P>\n'
               OzDocToHTML, Batch(M 1)
               Out <- @Out#'</P>\n'
            [] para then
               %--** check for class=apropos?
               Out <- @Out#'<P class=margin>'
               OzDocToHTML, Batch(M.1=title(...) 1)
               Out <- @Out#'</P>\n<P>\n'
               OzDocToHTML, Batch(M 2)
               Out <- @Out#'</P>\n'
            [] 'div' then
               %--** do more here?
               % E.g., format some of these with a different background
               % color
               OzDocToHTML, Batch(M 1)
            %-----------------------------------------------------------
            % Lists
            %-----------------------------------------------------------
            [] list then
               %--** display attribute
               if {HasFeature M continues} then
                  {Exception.raiseError
                   ozDoc(sgmlToHTML notImplemented M continues)}   %--**
               end
               if {Label M.1} == entry then X in
                  Out <- @Out#'</P><DL>\n'
                  X = @InDescription
                  InDescription <- true
                  OzDocToHTML, Batch(M 1)
                  InDescription <- X
                  Out <- @Out#'</DL><P>\n'
               elseif {HasFeature M enum} then X in
                  %--** should the type be predefined so that HTML and
                  %--** LaTeX and HTML versions match? (type="[1aAiI]")
                  Out <- @Out#('</P><OL'#
                               case {CondSelect M n unit} of unit then ""
                               elseof I then ' start='#I
                               end#'>\n')
                  X = @InDescription
                  InDescription <- false
                  OzDocToHTML, Batch(M 1)
                  InDescription <- X
                  Out <- @Out#'</OL><P>\n'
               elseif {HasFeature M n} then
                  {Exception.raiseError
                   ozDoc(sgmlToHTML illegalAttributes M)}
               else X in
                  Out <- @Out#'</P><UL>\n'
                  X = @InDescription
                  InDescription <- false
                  OzDocToHTML, Batch(M 1)
                  InDescription <- X
                  Out <- @Out#'</UL><P>\n'
               end
            [] entry then
               Out <- @Out#'<DT>'
               OzDocToHTML, Batch(M.1 1)
               Out <- @Out#'</DT>'
            [] synopsis then
               {Exception.raiseError ozDoc(sgmlToHTML unsupported M)}   %--**
            [] item then
               if @InDescription then
                  Out <- @Out#'<DD>\n'
                  OzDocToHTML, Batch(M 1)
                  Out <- @Out#'</DD>\n'
               else
                  Out <- @Out#'<LI>\n'
                  OzDocToHTML, Batch(M 1)
                  Out <- @Out#'</LI>\n'
               end
            %-----------------------------------------------------------
            % Math
            %-----------------------------------------------------------
            [] math then Display in
               Display = case @MathDisplay of unit then M.display
                         elseof X then X
                         end
               case Display of display then
                  Out <- @Out#'</P><BLOCKQUOTE>'
               [] inline then skip
               end
               case M.type of 'TEX' then
                  OzDocToHTML, BatchMathTex(M 1)
               [] 'LATEX' then
                  OzDocToHTML, BatchMathTex(M 1)
               [] 'HTML' then
                  Out <- @Out#M.1
               else
                  {Exception.raiseError
                   ozDoc(sgmlToHTML unsupportedMathNotation M)}   %--**
               end
               case Display of display then
                  Out <- @Out#'</BLOCKQUOTE><P>'
               [] inline then skip
               end
            [] 'math.extern' then
               {Exception.raiseError ozDoc(sgmlToHTML unsupported M)}   %--**
            [] 'math.choice' then
               MathDisplay <- M.display
               OzDocToHTML, Process(M.1)   %--** make better choice
               MathDisplay <- unit
            %-----------------------------------------------------------
            % Picture Element
            %-----------------------------------------------------------
            [] picture then
               {Exception.raiseError ozdoc(sgmlToHTML unsupported M)}   %--**
            [] 'picture.extern' then
               {Exception.raiseError ozDoc(sgmlToHTML unsupported M)}   %--**
            [] 'picture.choice' then
               {Exception.raiseError ozDoc(sgmlToHTML unsupported M)}   %--**
            %-----------------------------------------------------------
            % Code
            %-----------------------------------------------------------
            [] code then
               case M.display of display then
                  Out <- @Out#'</P><BLOCKQUOTE>'
               [] inline then skip
               end
               OzDocToHTML, BatchCode(M 1)
               case M.display of display then
                  Out <- @Out#'</BLOCKQUOTE><P>'
               [] inline then skip
               end
            [] 'code.extern' then
               %--** class=linenumbers
               Out <- @Out#'</P>'
               case M.display of display then
                  Out <- @Out#'<BLOCKQUOTE>'
               [] inline then skip
               end
               Out <- @Out#('<PRE>\n'#
                            {@MyFontifier
                             enqueueFile(@ProgLang.1 M.to '\n' $)}#
                            '</PRE>')
               case M.display of display then
                  Out <- @Out#'</BLOCKQUOTE>'
               [] inline then skip
               end
               Out <- @Out#'<P>'
            [] var then
               case M.type of prog then
                  Out <- @Out#"<CODE>"
                  Out <- @Out#case {CondSelect M mode unit} of unit then ""
                              [] 'in' then '+'
                              [] out then '?'
                              [] cin then '*'
                              [] cnin then '*'
                              end
                  OzDocToHTML, Batch(M 1)
                  Out <- @Out#"</CODE>"
               [] meta then
                  Out <- @Out#"<I>"
                  OzDocToHTML, Batch(M 1)
                  Out <- @Out#"</I>"
               [] env then
                  Out <- @Out#"<CODE>"
                  OzDocToHTML, Batch(M 1)
                  Out <- @Out#"</CODE>"
               [] grammar then
                  Out <- @Out#"&lt;<I>"
                  OzDocToHTML, Batch(M 1)
                  Out <- @Out#"</I>&gt;"
               end
            %-----------------------------------------------------------
            % Cross References
            %-----------------------------------------------------------
            [] ref then
               {Exception.raiseError ozDoc(sgmlToHTML unsupported M)}   %--**
            [] 'ref.extern' then
               %--** key attribute?
               Out <- @Out#'<A href='#{MakeCDATA M.to}#'>'
               OzDocToHTML, Batch(M 1)
               Out <- @Out#'</A>'
            [] ptr then Node VS in
               OzDocToHTML, ID(M.to ?Node ?VS)
               Out <- @Out#('<A href='#
                            thread {MakeCDATA Node#"#"#M.to} end#
                            '>'#VS#'</A>')
            [] 'ptr.extern' then
               {Exception.raiseError ozDoc(sgmlToHTML unsupported M)}   %--**
            %-----------------------------------------------------------
            % Phrasal Elements
            %-----------------------------------------------------------
            [] file then
               Out <- @Out#'<CODE>'
               OzDocToHTML, Batch(M 1)
               Out <- @Out#'</CODE>'
            [] kbd then
               Out <- @Out#'<KBD>'
               OzDocToHTML, Batch(M 1)
               Out <- @Out#'</KBD>'
            [] key then
               {Exception.raiseError ozDoc(sgmlToHTML unsupported M)}   %--**
            [] samp then
               Out <- @Out#'<CODE>'
               OzDocToHTML, Batch(M 1)
               Out <- @Out#'</CODE>'
            [] name then
               %--** format differently, depending on class?
               OzDocToHTML, Batch(M 1)
            [] q then
               %--** use different quotes, depending on class and/or lang?
               if {SGML.isOfClass M terminal} then
                  Out <- @Out#'"'
                  OzDocToHTML, Batch(M 1)
                  Out <- @Out#'"'
               else
                  Out <- @Out#'``'
                  OzDocToHTML, Batch(M 1)
                  Out <- @Out#'\'\''
               end
            [] span then
               if {SGML.isOfClass M ignore} then skip
               else
                  OzDocToHTML, Batch(M 1)
               end
            [] def then
               Out <- @Out#'<EM>'
               OzDocToHTML, Batch(M 1)
               Out <- @Out#'</EM>'
            [] em then   %--** Gump special
               Out <- @Out#'<EM>'
               OzDocToHTML, Batch(M 1)
               Out <- @Out#'</EM>'
            %-----------------------------------------------------------
            % Figure
            %-----------------------------------------------------------
            [] figure then
               if {HasFeature M float} then
                  Floats <- {Append @Floats [M]}
               else
                  OzDocToHTML, OutputFigure(M)
               end
            [] caption then
               {Exception.raiseError ozDoc(sgmlToHTML internalError M)}
            %-----------------------------------------------------------
            % Note
            %-----------------------------------------------------------
            [] note then
               {Exception.raiseError ozDoc(sgmlToHTML unsupported M)}   %--**
            %-----------------------------------------------------------
            % Index
            %-----------------------------------------------------------
            [] index then
               {Exception.raiseError ozDoc(sgmlToHTML unsupported M)}   %--**
            [] and then
               {Exception.raiseError ozDoc(sgmlToHTML unsupported M)}   %--**
            [] see then
               {Exception.raiseError ozDoc(sgmlToHTML unsupported M)}   %--**
            %-----------------------------------------------------------
            % BNF Markup
            %-----------------------------------------------------------
            [] 'grammar.rule' then
               %--** display attribute?
               Out <- @Out#'</P><PRE>\n'
               case M.1 of X=var(...) then Var in
                  OzDocToHTML, Excursion(X ?Var)
                  %--** this is only OK if type=grammar:
                  Out <- @Out#'  &lt;<I>'#Var#'</I>&gt; '
                  %--** try to guess the length better:
                  GrammarAltIndent <- {VirtualString.length Var} + 5
               [] 'grammar.head' then
                  Out <- @Out#'  '
                  OzDocToHTML, Batch(M.1 1)
                  Out <- @Out#'\n      '
                  GrammarAltIndent <- 6
               end
               OzDocToHTML, Batch(M 2)
               Out <- @Out#'</PRE><P>\n'
            [] 'grammar.head' then
               {Exception.raiseError ozDoc(sgmlToHTML unsupported M)}   %--**
            [] 'grammar.alt' then
               Out <- @Out#case {CondSelect M type unit} of def then '::= '
                           [] add then
                              GrammarAltIndent <- @GrammarAltIndent - 1
                              '+= '
                           [] 'or' then {Spaces @GrammarAltIndent}#' |  '
                           [] space then {Spaces @GrammarAltIndent}#'    '
                           [] unit then {Spaces @GrammarAltIndent}#'    '
                           end
               OzDocToHTML, Batch(M 1)
               Out <- @Out#'\n'
            [] 'grammar.note' then
               {Exception.raiseError ozDoc(sgmlToHTML unsupported M)}   %--**
            [] 'grammar' then
               OzDocToHTML, Batch(M 1)
            %-----------------------------------------------------------
            % Tables
            %-----------------------------------------------------------
            [] table then Mr in
               %--** display attribute
               case {SGML.getSubtree M title ?Mr} of unit then skip
               else
                  {Exception.raiseError
                   ozDoc(sgmlToHTML unsupportedTableTitle M)}
               end
               Out <- @Out#'</P><TABLE align=center border=1>\n'
               OzDocToHTML, Batch(Mr 1)
               Out <- @Out#'</TABLE><P>\n'
            [] tr then
               Out <- @Out#'<TR>\n'
               OzDocToHTML, Batch(M 1)
               Out <- @Out#'</TR>\n'
            [] th then
               Out <- @Out#('<TH'#
                            case {CondSelect M colspan unit} of unit then ""
                            elseof S then ' colspan='#{MakeCDATA S}
                            end#
                            '>\n')
               OzDocToHTML, Batch(M 1)
               Out <- @Out#'</TH>\n'
            [] td then
               Out <- @Out#('<TD'#
                            case {CondSelect M colspan unit} of unit then ""
                            elseof S then ' colspan='#{MakeCDATA S}
                            end#
                            '>\n')
               OzDocToHTML, Batch(M 1)
               Out <- @Out#'</TD>\n'
            else
               {Exception.raiseError ozDoc(sgmlToHTML unsupported M)}   %--**
            end
         end
         OzDocToHTML, PopCommon(M)
      end
      meth MakeTitle(PtrText FormatNumber Sep Title LayoutTitle M Level)
         Label Text Res
      in
         OzDocToHTML, FlushFloats()
         if {HasFeature M id} then
            Label = M.id
            Res = '<A name='#{MakeCDATA Label}#'>'#Text#'</A>'
         else
            ToGenerate <- Label|@ToGenerate
            Res = '<A name='#thread {MakeCDATA Label} end#'>'#Text#'</A>'
         end
         if {SGML.isOfClass M unnumbered} then
            if Title == unit then
               {Exception.raiseError ozDoc(sgmlToHTML emptySectionTitle M)}
            end
            Text = Title
         else NumberVS in
            NumberVS = {FormatNumber}
            Text = case Title of unit then NumberVS
                   else NumberVS#Sep#Title
                   end
            if {HasFeature M id} then
               OzDocToHTML, ID(M.id @CurrentNode PtrText#NumberVS)
            end
         end
         TOC <- {Append @TOC [Level#Label#@CurrentNode#Text]}
         Out <- @Out#{LayoutTitle Res}
      end
      meth ID(L Node VS)
         if {Dictionary.member @Labels L} then
            Node#VS = {Dictionary.get @Labels L}
         else
            {Dictionary.put @Labels L Node#VS}
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
      meth CheckNode(M Title ?X)
         if {Member {CondSelect M id unit}
             {Dictionary.condGet @Meta 'htmlfile' nil}}
         then
            X = @CurrentNode#@Out#@TOC#@TOCMode
            NodeCounter <- @NodeCounter + 1
            CurrentNode <- 'node'#@NodeCounter#'.html'
            OzDocToHTML, StartNode(Title)
            TOC <- nil
            TOCMode <- false
         else
            X = unit
            if @TOCMode then
               Out <- @Out#'<HR>\n'
               TOCMode <- false
            end
         end
      end
      meth PopNode(X)
         case X of unit then skip
         [] C#O#T#M then
            OzDocToHTML, EndNode()
            CurrentNode <- C
            if M then
               Out <- O#{FormatTOC @TOC}
            else
               Out <- O#'<HR>\n'#{FormatTOC @TOC}
            end
            TOC <- {Append T @TOC}
            TOCMode <- true
         end
      end
      meth StartNode(Title)
         Out <- ('<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 3.2//EN">\n'#
                 '<HTML>\n'#
                 '<HEAD>\n'#
                 '<TITLE>'#Title#'</TITLE>\n'#
                 '<LINK rel=stylesheet href="/css/page.css">\n'#
                 '</HEAD>\n'#
                 '<BODY>\n')
         %--** navigation panel
      end
      meth EndNode()
         OzDocToHTML, FlushFloats()
         Out <- @Out#('<HR>\n'#
                      '<ADDRESS>\n'#
                      case @Authors of nil then ""
                      else
                         OzDocToHTML, FormatAuthors($)#'<BR>\n'
                      end#
                      'Generated on '#{FormatDate {OS.localTime}}#'\n'#
                      '</ADDRESS>\n'#
                      '</BODY>\n'#
                      '</HTML>\n')
         ToWrite <- @Out#(@OutputDirectory#'/'#@CurrentNode)|@ToWrite
         Out <- ""
      end
      meth FlushFloats()
         case @Floats of F|Fr then
            OzDocToHTML, OutputFigure(F)
            Floats <- Fr
            OzDocToHTML, FlushFloats()
         [] nil then skip
         end
      end
      meth OutputFigure(M) Class N Number Title Mr1 Caption Mr2 in
         Class = case {CondSelect M 'class' 'figure'} of _|_ then
                    {Exception.raiseError
                     ozDoc(sgmlToHTML moreThanOneFigureClass M)} unit
                 elseof X then X
                 end
         N = {Dictionary.condGet @FigureCounters Class 0} + 1
         {Dictionary.put @FigureCounters Class N}
         Number = ({InitialCapital {Atom.toString Class}}#'&nbsp;'#
                   if @Appendix then {Alpha @Chapter}
                   else @Chapter
                   end#'.'#N)
         Out <- @Out#'<HR>\n'
         case {CondSelect M id unit} of unit then skip
         elseof L then
            OzDocToHTML, ID(L @CurrentNode Number)
            Out <- @Out#'<A name='#{MakeCDATA L}#'></A>'
         end
         Title = {SGML.getSubtree M title ?Mr1}
         Caption = {SGML.getSubtree Mr1 caption ?Mr2}
         case Title of unit then skip
         else
            Out <- @Out#'<P align=center><B>'
            OzDocToHTML, Batch(Title 1)
            Out <- @Out#'</B></P>\n'
         end
         OzDocToHTML, Batch(Mr2 1)
         case Caption of unit then skip
         else
            Out <- @Out#'<P><STRONG>'#Number#':</STRONG> '
            OzDocToHTML, Batch(Caption.1 1)
            Out <- @Out#'</P>'
            OzDocToHTML, Batch(Caption 2)
         end
         Out <- @Out#'<HR>\n'
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
               case H of unit then N#'\n'
               else '<A href='#{MakeCDATA H}#'>'#N#'</A>'
               end
            end
         in
            {FoldLTail Ar
             fun {$ In A|Ar}
                In#
                case Ar of nil then ' and\n'
                else ',\n'
                end#{FormatAuthor A}
             end {FormatAuthor A}}#'\n'
         end
      end
   end

   proc {Translate IsColor File OutputDirectory}
      {New OzDocToHTML
       init(IsColor {SGML.parse File} OutputDirectory) _}
   end
end
