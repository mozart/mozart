% Copyright © by Denys Duchier, August 1999, Universität des
% Saarlandes

\newcommand{\mozart@release@number}{1.0}
\newcommand{\mozart@release@date}{\@date}

\def\fileversion{v1.0}
\def\filedate{4 August 1999}
\def\docdate {4 August 1999}

\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{ozdoc}[1999/08/04 Mozart Documentation Class (Denys Duchier)]

% aargh! currrent installation has wrong defaults
% this appears to fix the pb:
\lefthyphenmin=2
\righthyphenmin=3

\RequirePackage{times}

% pass options down to book class
\DeclareOption*{\PassOptionsToClass{\CurrentOption}{book}}
\ProcessOptions

% load packages
\LoadClass[a4paper,11pt]{book}
\RequirePackage{verbatim}
\RequirePackage{float}
\RequirePackage[dvips]{graphicx}
\RequirePackage[dvips]{epsfig}
\RequirePackage{alltt}
\RequirePackage{isolatin1}
\RequirePackage{color}
\RequirePackage{theorem}

% layout parameter

\newlength{\mozart@fullwidth}
\newlength{\mozart@leftmarginwidth}

\setlength{\mozart@leftmarginwidth}{1in}

\addtolength{\oddsidemargin}{-0.6cm}
\addtolength{\textwidth}{3.14cm}
\addtolength{\topmargin}{-1.5cm}
\addtolength{\textheight}{2.6cm}
\setlength{\mozart@fullwidth}{\textwidth}
\addtolength{\textwidth}{-\mozart@leftmarginwidth}
\addtolength{\oddsidemargin}{\mozart@leftmarginwidth}

\setlength{\evensidemargin}{\oddsidemargin}
\setlength{\parindent}{0pt}
\setlength{\parskip}{6pt plus 3pt minus 2pt}

\setlength{\marginparsep}{8mm}
\setlength\marginparwidth\mozart@leftmarginwidth
\addtolength{\marginparwidth}{-\marginparsep}

\raggedbottom

% sectional commands
%
% \(sub)*section are redefined to hang out in the left margin, but in
% such a way that they may extend over several lines, and so that the
% vertical space above and below is a little more compact that in
% bookstyle.

\newcommand{\mozart@section}{%
  \raggedright\leftskip-\mozart@leftmarginwidth
  \mozart@section@font}

\newcommand{\mozart@section@font}{%
  \normalfont\sffamily\bfseries}

\renewcommand{\section}{%
  \@startsection{section}{1}{\z@}%
        {-3ex plus -1ex minus -.2ex}%
        {2ex plus .2ex}%
        {\mozart@section\Large}}

\renewcommand{\subsection}{%
  \@startsection{subsection}{1}{\z@}%
        {-2ex plus -1ex minus -.2ex}%
        {1.3ex plus .2ex}%
        {\mozart@section\large}}

\renewcommand{\subsubsection}{%
  \@startsection{subsubsection}{1}{\z@}%
        {-2ex plus -1ex minus -.2ex}%
        {1.3ex plus .2ex}%
        {\mozart@section}}

\renewcommand{\chaptermark}[1]{\markboth{#1}{}}

\renewcommand{\@makechapterhead}[1]{%
  \thispagestyle{empty}%
  \vspace*{10\p@}%
  {\parindent\z@\raggedleft
    \ifnum \c@secnumdepth >\m@ne
      \if@mainmatter
        \mozart@section@font\huge
        \thechapter\quad
        \par\nobreak
        \vskip 4\p@
      \fi
    \fi
    \interlinepenalty\@M
    \advance\leftskip-\mozart@leftmarginwidth
    \advance\linewidth\mozart@leftmarginwidth
    \rule{\linewidth}{1.3\p@}\par\nobreak
    \mozart@section@font\Huge#1\par\nobreak
    \vskip 40\p@
  }}

\renewcommand{\@makeschapterhead}[1]{%
  \thispagestyle{empty}%
  \vspace*{10\p@}%
  {\parindent\z@\raggedleft
    \interlinepenalty\@M
    \advance\leftskip-\mozart@leftmarginwidth
    \advance\linewidth\mozart@leftmarginwidth
    \rule{\linewidth}{1.3\p@}\par\nobreak
    \mozart@section@font\Huge#1\par\nobreak
    \vskip 40\p@
  }}

\def\@part[#1]#2{%
    \ifnum \c@secnumdepth >-2\relax
      \refstepcounter{part}%
      \addcontentsline{toc}{part}{\thepart\hspace{1em}#1}%
    \else
      \addcontentsline{toc}{part}{#1}%
    \fi
    \markboth{}{}%
    {\centering
     \interlinepenalty \@M
     \normalfont
     \ifnum \c@secnumdepth >-2\relax
       \huge\mozart@section@font \partname~\thepart
       \par
       \vskip 20\p@
     \fi
     \Huge \bfseries #2\par}%
    \@endpart}
\def\@spart#1{%
    {\centering
     \interlinepenalty \@M
     \normalfont
     \Huge\mozart@section@font #1\par}%
    \@endpart}

\renewcommand\paragraph{\@startsection{paragraph}{4}{\z@}%
  {3.25ex \@plus1ex \@minus.2ex}%
  {-1em}%
  {\normalfont\mozart@section@font\normalsize}}

\newcommand{\mozart@internal@paragraph}[1]{%
  \ifvmode\else\par\fi
  \addpenalty\@secpenalty\addvspace{3.25ex \@plus1ex \@minus.2ex}%
  \noindent{\normalfont\mozart@section@font\normalsize#1}%
  \hspace{1em}\ignorespaces}

% \begin{FULLWIDTH}...\end{FULLWIDTH}
%       an environment that spans the full width of the page
% including the left margin

\newenvironment{FULLWIDTH}
  {\begin{list}{}{\linewidth\textwidth
     \leftmargin-\mozart@leftmarginwidth
     \rightmargin\z@}\item[]}
  {\end{list}}

% page style

\newlength{\mozart@headrulewidth}
\setlength{\mozart@headrulewidth}{0.4pt}

\newcommand{\mozart@head}[1]{%
  \hbox{\hskip-\mozart@leftmarginwidth
    \advance\hsize\mozart@leftmarginwidth
    \vbox to\z@{\vss
      \hbox to\mozart@fullwidth{\strut#1}%
      \hrule\@height\mozart@headrulewidth}}}

\newcommand{\ps@mozart}{%
  \renewcommand{\@oddhead}{%
    \mozart@head{\textrm{\rightmark}\hfil\textbf{\thepage}}}%
  \renewcommand{\@evenhead}{%
    \mozart@head{\textbf{\thepage}\hfil\textsl{\leftmark}}}%
  \renewcommand{\@oddfoot}{}%
  \renewcommand{\@evenfoot}{}}

\renewcommand{\sectionmark}[1]{%
  \markright{\ifnum \c@secnumdepth >\z@
    \thesection. \ \fi #1}}

\ps@mozart

% front matter

\newcommand{\mozart@front}{%
  \mozart@titlepage
  \mozart@license
  \mozart@abstract
  \mozart@contents
  \clearpage
  \pagestyle{mozart}\thispagestyle{mozart}%
  \setcounter{page}{1}\setcounter{footnote}{0}}

\AtBeginDocument{\mozart@front}

\newcommand{\mozart@title@font@one}{%
  \mozart@section@font\LARGE}
\newcommand{\mozart@title@font@two}{%
  \mozart@section@font\Large}
\newcommand{\mozart@title@font@three}{%
  \mozart@section@font\large}

\newcommand{\ps@mozart@front}{%
  \renewcommand{\@oddhead}{%
    \mozart@head{\mozart@title@font@one
      Mozart Documentation\hfil}}%
  \renewcommand{\@evenhead}{}%
  \renewcommand{\@oddfoot}{}%
  \renewcommand{\@evenfoot}{}}

\newcommand{\@comic}{}
\let\@author\@empty
\let\@title\@empty

\newcommand{\mozart@titlepage}{%
\bgroup
  \let\ps@empty\ps@mozart@front
  \pagestyle{empty}\thispagestyle{mozart@front}%
  \begin{titlepage}%
    \hbox{}\vfill
    \begin{FULLWIDTH}%
      \ifx\@comic\long@empty\else
        \begin{center}\@comic\end{center}\fi
      \begin{flushright}
        \ifx\@title\@empty\else{\mozart@title@font@one\@title\\[1cm]}\fi
        \ifx\@author\@empty\else{\mozart@title@font@two\@author\\[1cm]}\fi
      \end{flushright}%
      \vfill
      \mozart@title@font@three
      \noindent
      Mozart V\mozart@release@number\hfill\mozart@release@date
    \end{FULLWIDTH}%
  \end{titlepage}%
\egroup}

\newlength{\mozart@quote@indent}
\setlength{\mozart@quote@indent}{1cm}
\newenvironment{mozart@quote}
  {\begin{list}{}{%
     \leftmargin\mozart@quote@indent
     \rightmargin\mozart@quote@indent}
   \item[]\ignorespaces}
  {\end{list}}

\newcommand{\mozart@license}{%
  \clearpage
  \pagestyle{empty}\thispagestyle{empty}%
  \begin{FULLWIDTH}%
    \begin{center}\mozart@section@font
      \Large License Agreement%
    \end{center}%
    \begin{mozart@quote}%
      This software is copyrighted by the German Research Center for
      Artificial Intelligence (DFKI), the Swedish Institute of
      Computer Science (SICS), and other parties.  The following terms
      apply to all files associated with the software unless
      explicitly disclaimed in individual files.

      The authors hereby grant permission to use, copy, modify,
      distribute, and license this software and its documentation for
      any purpose, provided that existing copyright notices are
      retained in all copies and that this notice is included verbatim
      in any distributions. No written agreement, license, or royalty
      fee is required for any of the authorized uses. Modifications to
      this software may be copyrighted by their authors and need not
      follow the licensing terms described here, provided that the new
      terms are clearly indicated on the first page of each file where
      they apply.

      IN NO EVENT SHALL THE AUTHORS OR DISTRIBUTORS BE LIABLE TO ANY
      PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
      CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE,
      ITS DOCUMENTATION, OR ANY DERIVATIVES THEREOF, EVEN IF THE
      AUTHORS HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

      THE AUTHORS AND DISTRIBUTORS SPECIFICALLY DISCLAIM ANY
      WAR\-RAN\-TIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
      WAR\-RAN\-TIES OF MER\-CHAN\-TA\-B\-ILI\-TY, FITNESS FOR A
      PARTICULAR PURPOSE, AND NON-IN\-FRIN\-GE\-MENT.  THIS SOFTWARE IS
      PROVIDED ON AN "AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS
      HAVE NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
      ENHANCEMENTS, OR MODIFICATIONS.
    \end{mozart@quote}%
  \end{FULLWIDTH}}

\newcommand{\long@empty}{}
\newcommand{\mozart@abstract}{%
  \ifx\@abstract\@long@empty\else
    \clearpage
    \thispagestyle{empty}%
    \begin{FULLWIDTH}%
      \vspace*{\fill}%
      \begin{center}\mozart@section@font\Large Abstract\end{center}%
      \begin{mozart@quote}\@abstract\end{mozart@quote}%
      \vspace*{\fill}%
    \end{FULLWIDTH}%
  \fi}

\newcommand{\mozart@contents}{%
  \clearpage\pagestyle{empty}\thispagestyle{empty}%
  {\let\ps@plain\ps@empty\tableofcontents}}

% floats

\renewcommand{\floatpagefraction}{.7}
\floatstyle{plain}

% floats should span the full width of the page: we redefine the
% following command to format a float in a box that spans the full
% width of the page and to move it left by the width of the left
% margin.

\renewcommand\float@makebox{%
  \vbox{\moveleft\mozart@leftmarginwidth
  \vbox{\hsize\mozart@fullwidth \@parboxrestore
    \@fs@pre\@fs@iftopcapt
      \ifvbox\@floatcapt\unvbox\@floatcapt\par\@fs@mid\fi
      \unvbox\@currbox
    \else\unvbox\@currbox
      \ifvbox\@floatcapt\par\@fs@mid\unvbox\@floatcapt\fi
    \fi\par\@fs@post\vskip\z@}}}

% this seems to do the trick.  otherwise we get loads of overfull warnings.
\renewcommand{\@floatboxreset}{%
  \hsize\mozart@fullwidth\columnwidth\hsize
  \reset@font
  \normalsize
  \@setnobreak
  \@setminipage}

\floatstyle{ruled}
\newfloat{FIGURE}{htbp}{lof}[chapter]
\floatname{FIGURE}{Figure}

\newlength{\mozart@len}

\renewcommand{\floatc@ruled}[2]{
  \setlength{\mozart@len}{\mozart@fullwidth}%
  \addtolength{\mozart@len}{-2\fboxsep}%
  \colorbox{menucolor}{\parbox{\mozart@len}{{\@fs@cfont #1} #2}}}

% XSL SUPPORT

% support for strange characters

\DeclareRobustCommand\mozart@text@HASH{\#}
\DeclareRobustCommand\mozart@text@DOLLAR{\$}
\DeclareRobustCommand\mozart@text@PERCENT{\%}
\DeclareRobustCommand\mozart@text@AMPERSAND{\&}
\DeclareRobustCommand\mozart@text@TILDE{\~}
\DeclareRobustCommand\mozart@text@UNDERSCORE{\_}
\DeclareRobustCommand\mozart@text@CARET{\char`\^}
\DeclareRobustCommand\mozart@text@BSLASH{\ensuremath{\backslash}}
\DeclareRobustCommand\mozart@text@LBRACE{\ensuremath{\{}}
\DeclareRobustCommand\mozart@text@RBRACE{\ensuremath{\}}}
\DeclareRobustCommand\mozart@text@LT{\ensuremath{<}}
\DeclareRobustCommand\mozart@text@GT{\ensuremath{>}}
\DeclareRobustCommand\mozart@text@VBAR{\ensuremath{\vert}}

\newcommand{\mozartHASH}[1]{\mozart@HASH}
\newcommand{\mozartDOLLAR}[1]{\mozart@DOLLAR}
\newcommand{\mozartPERCENT}[1]{\mozart@PERCENT}
\newcommand{\mozartAMPERSAND}[1]{\mozart@AMPERSAND}
\newcommand{\mozartTILDE}[1]{\mozart@TILDE}
\newcommand{\mozartUNDERSCORE}[1]{\mozart@UNDERSCORE}
\newcommand{\mozartCARET}[1]{\mozart@CARET}
\newcommand{\mozartBSLASH}[1]{\mozart@BSLASH}
\newcommand{\mozartLBRACE}[1]{\mozart@LBRACE}
\newcommand{\mozartRBRACE}[1]{\mozart@RBRACE}
\newcommand{\mozartLT}[1]{\mozart@LT}
\newcommand{\mozartGT}[1]{\mozart@GT}
\newcommand{\mozartVBAR}[1]{\mozart@VBAR}
\newcommand{\mozartSPACE}[1]{\mozart@SPACE}
\newcommand{\mozartNEWLINE}[1]{\mozart@NEWLINE}

\newcommand{\mozartEMPTY}{}

\newcommand{\mozart@specials@text}{%
  \let\mozart@HASH      \mozart@text@HASH
  \let\mozart@DOLLAR    \mozart@text@DOLLAR
  \let\mozart@PERCENT   \mozart@text@PERCENT
  \let\mozart@AMPERSAND \mozart@text@AMPERSAND
  \let\mozart@TILDE     \mozart@text@TILDE
  \let\mozart@UNDERSCORE\mozart@text@UNDERSCORE
  \let\mozart@CARET     \mozart@text@CARET
  \let\mozart@BSLASH    \mozart@text@BSLASH
  \let\mozart@LBRACE    \mozart@text@LBRACE
  \let\mozart@RBRACE    \mozart@text@RBRACE
  \let\mozart@LT        \mozart@text@LT
  \let\mozart@GT        \mozart@text@GT
  \let\mozart@VBAR      \mozart@text@VBAR
}
\mozart@specials@text

\DeclareRobustCommand\mozart@code@HASH{\char`\#}
\DeclareRobustCommand\mozart@code@DOLLAR{\char`\$}
\DeclareRobustCommand\mozart@code@PERCENT{\char`\%}
\DeclareRobustCommand\mozart@code@AMPERSAND{\char`\&}
\DeclareRobustCommand\mozart@code@TILDE{\char`\~}
\DeclareRobustCommand\mozart@code@UNDERSCORE{\char`\_}
\DeclareRobustCommand\mozart@code@CARET{\char`\^}
\DeclareRobustCommand\mozart@code@BSLASH{\char`\\}
\DeclareRobustCommand\mozart@code@LBRACE{\char`\{}
\DeclareRobustCommand\mozart@code@RBRACE{\char`\}}
\DeclareRobustCommand\mozart@code@LT{<}
\DeclareRobustCommand\mozart@code@GT{>}
\DeclareRobustCommand\mozart@code@VBAR{|}
\DeclareRobustCommand\mozart@code@SPACE{\nobreakspace}
\DeclareRobustCommand\mozart@code@NEWLINE{\nobreakspace}

\newcommand{\mozart@specials@code@inline}{%
  \let\mozart@HASH      \mozart@code@HASH
  \let\mozart@DOLLAR    \mozart@code@DOLLAR
  \let\mozart@PERCENT   \mozart@code@PERCENT
  \let\mozart@AMPERSAND \mozart@code@AMPERSAND
  \let\mozart@TILDE     \mozart@code@TILDE
  \let\mozart@UNDERSCORE\mozart@code@UNDERSCORE
  \let\mozart@CARET     \mozart@code@CARET
  \let\mozart@BSLASH    \mozart@code@BSLASH
  \let\mozart@LBRACE    \mozart@code@LBRACE
  \let\mozart@RBRACE    \mozart@code@RBRACE
  \let\mozart@LT        \mozart@code@LT
  \let\mozart@GT        \mozart@code@GT
  \let\mozart@VBAR      \mozart@code@VBAR
  \let\mozart@SPACE     \mozart@code@SPACE
  \let\mozart@NEWLINE   \mozart@code@NEWLINE
}

\DeclareRobustCommand\mode@code@NEWLINE@display{\par}

\newcommand{\mozart@specials@code@display}{%
  \mozart@specials@code@inline
  \let\mozart@NEWLINE   \mode@code@NEWLINE@display}

\newcommand{\mozart@DOT}{.}

\newcommand{\mozart@specials@ref}{%
  \let\mozart@HASH      \mozart@DOT
  \let\mozart@DOLLAR    \mozart@DOT
  \let\mozart@PERCENT   \mozart@DOT
  \let\mozart@AMPERSAND \mozart@DOT
  \let\mozart@TILDE     \mozart@DOT
  \let\mozart@UNDERSCORE\mozart@DOT
  \let\mozart@CARET     \mozart@DOT
  \let\mozart@BSLASH    \mozart@DOT
  \let\mozart@LBRACE    \mozart@DOT
  \let\mozart@RBRACE    \mozart@DOT
  \let\mozart@LT        \mozart@DOT
  \let\mozart@GT        \mozart@DOT
  \let\mozart@VBAR      \mozart@DOT
  \let\mozart@SPACE     \mozart@DOT
  \let\mozart@NEWLINE   \mozart@DOT
}

\newcommand{\mozart@defochar}[2]{%
  \def\mozart@tmp##1##2{%
    \newcommand#1{##2}}
  \expandafter\mozart@tmp\string#2}
\mozart@defochar{\mozart@other@HASH}{\#}
\mozart@defochar{\mozart@other@DOLLAR}{\$}
\mozart@defochar{\mozart@other@PERCENT}{\%}
\mozart@defochar{\mozart@other@AMPERSAND}{\&}
\mozart@defochar{\mozart@other@TILDE}{\~}
\mozart@defochar{\mozart@other@UNDERSCORE}{\_}
\mozart@defochar{\mozart@other@CARET}{\^}
\mozart@defochar{\mozart@other@BSLASH}{\\}
\mozart@defochar{\mozart@other@LBRACE}{\{}
\mozart@defochar{\mozart@other@RBRACE}{\}}
\mozart@defochar{\mozart@other@LT}{\<}
\mozart@defochar{\mozart@other@GT}{\>}
\mozart@defochar{\mozart@other@VBAR}{\|}

\newcommand{\mozart@specials@other}{%
  \let\mozart@HASH      \mozart@other@HASH
  \let\mozart@DOLLAR    \mozart@other@DOLLAR
  \let\mozart@PERCENT   \mozart@other@PERCENT
  \let\mozart@AMPERSAND \mozart@other@AMPERSAND
  \let\mozart@TILDE     \mozart@other@TILDE
  \let\mozart@UNDERSCORE\mozart@other@UNDERSCORE
  \let\mozart@CARET     \mozart@other@CARET
  \let\mozart@BSLASH    \mozart@other@BSLASH
  \let\mozart@LBRACE    \mozart@other@LBRACE
  \let\mozart@RBRACE    \mozart@other@RBRACE
  \let\mozart@LT        \mozart@other@LT
  \let\mozart@GT        \mozart@other@GT
  \let\mozart@VBAR      \mozart@other@VBAR
  \let\mozart@SPACE     \mozart@other@SPACE
  \let\mozart@NEWLINE   \mozart@other@NEWLINE
}

\let\latex@citex\@citex
\def\@citex[#1]#2{\bgroup\mozart@specials@ref
  \latex@citex[{#1}]{#2}\egroup}

% processing instructions

\newcommand{\usepi}[1]{%
  \@ifundefined{usepi@#1}{%
    \@latex@error{unknown processing instruction: #1}\@ehc}%
    {\@nameuse{usepi@#1}}}
\newcommand{\usepi@eg}{e.g.\ }
\newcommand{\usepi@ie}{i.e.\ }
\newcommand{\usepi@nbsp}{~}
\newcommand{\usepi@LaTeX}{\LaTeX{}}
\newcommand{\usepi@ellipsis}{\ldots}
\newcommand{\usepi@etc}{etc.}
\newcommand{\usepi@resp}{resp.\ }
\newcommand{\usepi@slash}{/}
\newcommand{\usepi@cdots}{\ensuremath{\cdots}}
% existing &emdash; should probably be changed to &endash;
% and \usepi@emdash should expand to ---
\newcommand{\usepi@emdash}{--}
\newcommand{\usepi@endash}{--}
\newcommand{\usepi@EG}{\usepi@eg}
\newcommand{\usepi@wrt}{wrt.\ }

% sectioning commands

\newcommand{\PART}{\part}
\newcommand{\CHAPTER}{\chapter}
\newcommand{\APPENDIX}{\mozart@appendix\chapter}
\newcommand{\SECTION}{\section}
\newcommand{\SUBSECTION}{\subsection}
\newcommand{\SUBSUBSECTION}{\subsubsection}
\newcommand{\PARA}{\paragraph}

\newcommand{\mozart@appendix}{%
  \appendix\global\let\mozart@appendix\relax}

% display

\newlength{\mozart@displaymargin}
\setlength{\mozart@displaymargin}{1cm}

\newenvironment{DISPLAY}
  {\begin{list}{}{\leftmargin\mozart@displaymargin}\item[]\ignorespaces}
  {\end{list}}

\newcommand{\mozart@code@font}{%
  \fontencoding{\encodingdefault}%
  \fontfamily{\familydefault}%
  \fontshape{\shapedefault}\ttfamily\selectfont}
\newcommand{\@CODEINLINE}[1]{\mozart@code@font\mozart@specials@code@inline#1\egroup}
\DeclareRobustCommand{\CODEINLINE}{\bgroup\let\FACETEXT\INLINEFACETEXT\@CODEINLINE}
\newcommand{\CODEEXTERN}[1]{%
  \begin{DISPLAY}\mozart@code@font\verbatiminput{#1}\end{DISPLAY}}
\newenvironment{CODEDISPLAY}
  {\begin{DISPLAY}\begin{alltt}\mozart@code@font\mozart@specials@code@display}
  {\end{alltt}\end{DISPLAY}}

\definecolor{comment}{rgb}{0.698039,0.133333,0.133333}
\definecolor{keyword}{rgb}{0.627451,0.125490,0.941176}
\definecolor{string}{rgb}{0.737255,0.560784,0.560784}
\definecolor{function}{rgb}{0.000000,0.000000,1.000000}
\definecolor{type}{rgb}{0.133333,0.545098,0.133333}
\definecolor{variable}{rgb}{0.721569,0.525490,0.043137}
\definecolor{reference}{rgb}{0.372549,0.619608,0.627451}
\definecolor{builtin}{rgb}{0.854902,0.439216,0.839216}

\DeclareRobustCommand{\FACECOMMENT}{\textcolor{comment}}
\DeclareRobustCommand{\FACEKEYWORD}[1]{\textbf{\color{keyword}#1}}
\DeclareRobustCommand{\FACESTRING}{\textcolor{string}}
\DeclareRobustCommand{\FACEVARIABLE}{\textcolor{variable}}
\DeclareRobustCommand{\FACEFUNCTION}{\textcolor{function}}
\DeclareRobustCommand{\FACEBUILTIN}{\textcolor{builtin}}
\DeclareRobustCommand{\FACEREFERENCE}{\textcolor{reference}}
\DeclareRobustCommand{\FACETYPE}{\textcolor{type}}
\DeclareRobustCommand{\FACETEXT}{}

\definecolor{chunkborder}{rgb}{1.0,0.647,0}
\definecolor{chunktitle}{rgb}{0.4196,0.556862745,0.1372549}
\definecolor{chunkref}{rgb}{0.4156862745,0.352941,0.803921}

\definecolor{inlinecode}{rgb}{0.4,0.2,0.4}
\DeclareRobustCommand{\INLINEFACETEXT}{\textcolor{inlinecode}}

\definecolor{menucolor}{rgb}{0.8,0.8,0.8}

% variables and annotations

\newcommand{\MODE}[1]{\csname MODE@#1\endcsname}
\newcommand{\MODE@IN}{\ensuremath{+}}
\newcommand{\MODE@OUT}{\ensuremath{?}}
\newcommand{\MODE@CIN}{\ensuremath{*}}
\newcommand{\MODE@CNIN}{\ensuremath{\$}}
\newcommand{\METAVAR}[1]{{\normalfont\textit{#1}}}
\newcommand{\ENVVAR}[1]{{\normalfont\texttt{#1}}}
\newcommand{\GRAMMARVAR}[1]{{\normalfont
  \ensuremath{\langle\textsf{#1}\rangle}}}
\newcommand{\PROGVAR}[1]{{\normalfont\texttt{#1}}}

% informative elements

\newcommand{\FILE}{\texttt}
\newcommand{\SAMP}{\texttt}
\newcommand{\EM}{\emph}
\newcommand{\KBD}{\texttt}
\newcommand{\KEY}{\texttt}

% references

\newcommand{\REFEXTTO}{\textsl}
\newcommand{\REFEXTKEY}[1]{ (#1)}
\newcommand{\DEFAULTREFEXT}{\texttt}

\let\latex@ref\ref
\let\latex@pageref\pageref

\newcommand{\mozart@noweird}{\useochar@defs\def\usechar##1{##1}\def\protect{}}

\def\ref#1{{\mozart@specials@ref\latex@ref{#1}}}
\def\pageref#1{{\mozart@specials@ref\latex@pageref{#1}}}
\def\label#1{\@bsphack
  {\mozart@specials@ref\xdef\mozart@temp{#1}}%
  \expandafter\mozart@label\expandafter{\mozart@temp}}
\def\mozart@label#1{%
  \protected@write\@auxout{}%
         {\string\newlabel{#1}{{\@currentlabel}{\thepage}}}%
  \@esphack}

% figures

\newcommand{\CAPTIONID}[1]{\caption{\label{#1}\kern0pt\relax}}
\newenvironment{FIGTITLE}
  {\begin{center}\large\bf}
  {\end{center}}

\newif\if@mozartExtension

\def\mozart@hasExtension#1#2{%
  \def\mozart@@hasExtension##1#2\mozart@one##2\mozart@two{%
    \def\mozart@temp{##2}%
    \ifx\mozart@temp\@empty
      \@mozartExtensionfalse
    \else
      \def\mozart@basename{##1}%
      \def\mozart@extension{#2}%
      \@mozartExtensiontrue
    \fi}%
  \mozart@@hasExtension#1\mozart@one#2\mozart@one\mozart@two}

\newcommand{\@PICEXT}[2]{% #1=filename #2=width
  \@mozartExtensionfalse
  \if@mozartExtension\else\mozart@hasExtension{#1}{.ps}\fi
  \if@mozartExtension\else\mozart@hasExtension{#1}{.eps}\fi
  \if@mozartExtension\else\mozart@hasExtension{#1}{.ps.gz}\fi
  \if@mozartExtension\else\mozart@hasExtension{#1}{.eps.gz}\fi
  \if@mozartExtension\else
    \def\mozart@basename{#1}%
    \def\mozart@extension{}%
  \fi
  \def\mozart@temp{#2}%
  \ifx\mozart@temp\@empty
    \def\@@PICEXT{\epsfig{file=\mozart@basename}}%
  \else
    \def\@@PICEXT{\epsfig{file=\mozart@basename,width=#2}}%
  \fi
  {\mozart@specials@other\@@PICEXT}}

% \PICEXT[width]{file}
\newcommand{\PICEXT}[2][0.7\linewidth]{\@PICEXT{#2}{#1}}

%\newcommand{\PICEXT}[1]{{\mozart@specials@other
%  \epsfig{file=#1,width=0.7\linewidth}}}
\newenvironment{PICEXTDISPLAY}
  {\begin{center}\begin{minipage}{\linewidth}\begin{center}\mozart@specials@other}
  {\end{center}\end{minipage}\end{center}}
\newcommand{\PICEXTFULL}[2][]{\@PICEXT{#2}{#1}}
%\newcommand{\PICEXTFULL}[1]{{\mozart@specials@other
%  \epsfig{file=#1,width=\linewidth}}}
\newcommand{\NOTEGUIPIC}[1]{\@PICEXT{#1}{8mm}}
%\newcommand{\NOTEGUIPIC}[1]{{\mozart@specials@other
%  \epsfig{file=#1,width=8mm}}}
\newcommand{\NOTEGUI}[2]{%
  \paragraph[#1]{#1 \NOTEGUIPIC{#2}}}
%% different from \PICEXT because here we want to add a .ps
%% extension that doesn't get stripped off
\newcommand{\PICEXTGIF}[2][0.7\linewidth]{{\mozart@specials@other
  \epsfig{file=#2.ps,width=#1}}}

% grammar stuff

\newcommand{\GRAMDEF}{\ensuremath{{:}{:}{=}}}
\newcommand{\GRAMADD}{\ensuremath{{+}{=}}}
\newcommand{\GRAMOR}{\ensuremath{\mid}}
\newcommand{\GRAMSPACE}{ }
\newcommand{\GRAMMARNOTE}[1]{\textsl{\% #1}}

% literate programming stuff

\newenvironment{CHUNK}[1]
  {\begin{list}{}{\leftmargin\mozart@displaymargin}%
     \item[\mbox{\bf$\color{chunkborder}\langle$%
       \textcolor{chunktitle}{#1}$\color{chunkborder}\rangle\equiv$}]%
     \topsep\z@\parskip\z@\partopsep\z@\parsep\z@\itemsep\z@
     \hfill\hbox{}\begin{alltt}\mozart@code@font\mozart@specials@code@display}
  {\end{alltt}\end{list}}

\newcommand{\CHUNKREF}[1]{%
  \mbox{\normalfont\bf$\color{chunkborder}\langle$%
    \textcolor{chunkref}{#1}$\color{chunkborder}\rangle$}}

% (entry,synopsis*)+,item

\newenvironment{DESCRIPTION}
  {\begin{list}{}{%
     \leftmargin\z@\rightmargin\z@
     \itemindent\z@
     \labelwidth\z@
     \let\makelabel\DESCRIPTION@label
     \let\ENTRY@MBOX\relax\let\ITEM\@ITEM}}
  {\end{list}}

\newcommand{\DESCRIPTION@label}[1]{%
  \hspace{-\mozart@leftmarginwidth}%
  \hspace{\labelsep}\bf#1\hfill}

\newenvironment{SYNOPSIS}
  {\noindent\begin{list}{}{%
     \topsep-\parskip\partopsep\z@
     \leftmargin\mozart@displaymargin}%
   \item[]\ignorespaces}
  {\end{list}\aftergroup\SYN@ITEM}

\def\SYN@ITEM{\let\ITEM\@ITEM}

\newcommand{\ENTRY@@MBOX}{%
  \mbox{}\vspace{-\parsep}\vspace{-\itemsep}}
\newcommand{\ENTRY}[1]{\ENTRY@MBOX\item[{\let\par\relax#1}]\let\ENTRY@MBOX\ENTRY@@MBOX}

\newcommand{\ENTRYHASCODE}{%
  \let\ITEM\ITEM@MBOX}
\newcommand{\ITEM@MBOX}{%
  \let\ENTRY@MBOX\relax\mbox{}\par\noindent}
\newcommand{\@ITEM}{%
  \let\ENTRY@MBOX\relax\vspace{\parsep}\noindent}

%\def\mozart@temp{\long\def\@item[##1]}
%\expandafter\mozart@temp\expandafter{\the\@item}

\let\latex@item\@item
\long\def\@item[#1]{\def\mozart@item@arg{#1}\latex@item[\mozart@item@arg]}

\newcommand{\MENU}[3]{%
  \colorbox{menucolor}{%
    \begin{tabular}[b]{@{}p{\mozart@leftmarginwidth}p{1cm}p{1cm}@{}}%
      #1&#2&#3%
    \end{tabular}}}

\newcommand{\MOUSE}[1]{\hbox{\lower3mm\hbox{\epsfig{file=mouse-#1.ps,width=1cm}}}}

% exercises and answers

\theoremheaderfont{\sffamily\bfseries}
\newtheorem{@EXERCISE}{Exercise}[chapter]
\newenvironment{EXERCISE}[1]
  {\begin{@EXERCISE}\label{#1}\let\PARA\mozart@internal@paragraph}
  {\end{@EXERCISE}}
% note the `*' in \paragraph*{... \ref{...}}
% to avoid messing up the \ref{...}.  There is probably
% a better more general way...
\newenvironment{ANSWER}[1]
  {\paragraph*{Answer to exercise \ref{#1}.}}
  {\par}

% rewrite rules

\newenvironment{REWRITE}
  {\begin{DISPLAY}\begin{tabular}{|l@{\quad\ensuremath{\rightarrow}\quad}l|}\hline}
  {\end{tabular}\end{DISPLAY}}
\newcommand{\REWRITEVAR}[1]{%
  \multicolumn{2}{|l|}{#1 \GRAMDEF}\\\hline}
\newcommand{\REWRITEFROM}[1]{#1&}
\newcommand{\REWRITETO}[1]{#1\\\hline}
\newcommand{\REWRITECONDITION}[1]{\multicolumn{2}{|@{\qqad}p{0.7\linewidth}|}{#1}\\\hline}

\newcommand{\EOLN}{\\\mozartEMPTY}

% ozdoc external cross references

\newcommand{\OZDOCREF}[3]{%
  \def\mozart@temp{#3}%
  \ifx\mozart@temp\@empty
    \@latex@error{unknown cross ref: to=#1 key=#2}\@ehc
  \else
    \@OZDOCREF#3%
  \fi}

\newcommand{\mozart@SECNAME@PART}{Part}
\newcommand{\mozart@SECNAME@CHAPTER}{Chapter}
\newcommand{\mozart@SECNAME@APPENDIX}{Appendix}
\newcommand{\mozart@SECNAME@SECTION}{Section}
\newcommand{\mozart@SECNAME@SUBSECTION}{Section}
\newcommand{\mozart@SECNAME@SUBSUBSECTION}{Section}

\newcommand{\@OZDOCREF}[3]{%
  \@nameuse{mozart@SECNAME@#1}%
  {\normalfont\space\textsl{#2}, \textsl{(#3)}}}

\newcommand{\OZDOCREFTOP}[2]{%
  \def\mozart@temp{#2}%
  \ifx\mozart@temp\@empty
    \@latex@error{unknown cross ref: to=#1}\@ehc
  \else
    \@OZDOCREFTOP{#2}%
  \fi}

\newcommand{\@OZDOCREFTOP}[1]{%
  {\normalfont\textsl{``#1''}}}
