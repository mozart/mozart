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

\renewcommand{\encodingdefault}{T1}
\RequirePackage{times}
\RequirePackage{mathptm}

\newif\ifmozart@hyper \mozart@hypertrue
\DeclareOption{nohyper}{\mozart@hyperfalse}

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
\RequirePackage{pstricks}
\RequirePackage{xspace}
\RequirePackage{array}
\RequirePackage{colortbl}

% mozart info
\RequirePackage{ozdoc-info}

% layout parameter

\newlength{\mozart@fullwidth}
\newlength{\mozart@leftmarginwidth}

\setlength{\mozart@leftmarginwidth}{1in}

\addtolength{\oddsidemargin}{-0.6cm}
\addtolength{\textwidth}{3.14cm}
\addtolength{\topmargin}{-1.5cm}
\addtolength{\textheight}{2.6cm}
\setlength{\mozart@fullwidth}{\textwidth}
\newcommand{\mozartfullwidth}{\mozart@fullwidth}
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

\renewcommand{\chaptermark}[1]{%
  \markboth{\ifnum \c@secnumdepth >\z@
    \@chapapp\ \thechapter. \ \fi #1}{}}

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

\newenvironment{mozartFULLWIDTH}
  {\begin{list}{}{\linewidth\textwidth
     \leftmargin-\mozart@leftmarginwidth
     \rightmargin\z@}\item[]}
  {\end{list}}

\newenvironment{mozartFULLWIDTHLEFT}
  {\begin{mozartFULLWIDTH}\nopagebreak\begin{flushleft}\nopagebreak}
  {\end{flushleft}\end{mozartFULLWIDTH}}

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
    \mozart@head{\normalfont\sffamily\rightmark\hfil\bfseries\thepage}}%
  \renewcommand{\@evenhead}{%
    \mozart@head{\normalfont\sffamily{\bfseries\thepage}\hfil\leftmark}}%
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
  \mozart@contents
  \if@openright\cleardoublepage\else\clearpage\fi
  \pagestyle{mozart}\thispagestyle{mozart}%
  \setcounter{page}{1}\setcounter{footnote}{0}}

\ifmozart@hyper
\RequirePackage{hyperref}
\fi
\AtBeginDocument{\mozart@front}

\newcommand{\mozart@title@font@one}{%
  \mozart@section@font\huge}
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

\newcommand{\mozartComic}[1]{%
  \renewcommand{\@comic}{\mozart@comic{#1}}%
  \renewcommand{\@comicauthor}{Cover illustration
    by Andreas Schoch\\}}
\newcommand{\@comicauthor}{}
\newcommand{\@comic}{}
\let\@author\@empty
\let\@title\@empty

\newcommand{\mozart@comic@height}{10cm}
\newcommand{\mozart@comic}[1]{%
  \mozart@cut@extension{#1}%
  {\mozart@specials@other
   \epsfig{file=\mozart@basename,height=\mozart@comic@height}}}

\newcommand{\mozart@version}{\@mozartVersion}
\newcommand{\mozart@date}{\expandafter\@mozart@date\@mozartBuildDate}

\newcount\mozart@cnt
\newcommand{\@mozart@date}[8]{%
  \mozart@cnt=#1#2#3#4\relax
  \edef\mozart@year{\the\mozart@cnt}%
  \mozart@cnt=#5#6\relax
  \edef\mozart@month{\the\mozart@cnt}%
  \mozart@cnt=#7#8\relax
  \def\mozart@day{\the\mozart@cnt}%
  \ifcase\mozart@month
  \or January
  \or February
  \or March
  \or April
  \or May
  \or June
  \or July
  \or August
  \or September
  \or October
  \or November
  \or December
  \fi
  \mozart@day,
  \mozart@year}

\newcommand{\@mozartstyle}{J}
\newcommand{\mozartstyle}[1]{%
  \renewcommand{\@mozartstyle}{#1}}

\newcommand{\mozart@bottomcomic}{\mozart@logo}

\newcommand{\mozart@topcomic}{}
\newcommand{\mozartstyle@A}{%
  \renewcommand{\mozart@topcomic}{%
    \begin{flushleft}%
      \ifx\@comic\long@empty\vspace*{\mozart@comic@height}\else\@comic\fi
    \end{flushleft}}}

\newcommand{\mozartstyle@B}{%
  \renewcommand{\mozart@topcomic}{%
    \begin{flushleft}%
      \ifx\@comic\long@empty\vspace*{\mozart@comic@height}\else\fbox{\@comic}\fi
    \end{flushleft}}}

\newcommand{\mozartstyle@C}{%
  \renewcommand{\mozart@topcomic}{%
    \begin{tabular*}{\mozart@fullwidth}{@{}l@{\extracolsep{\fill}}l|p{1cm}@{}}%
      \ifx\@comic\long@empty\vbox{\kern\z@\vskip\mozart@comic@height}\else
        \@comic\fi&&\\\hline&&
    \end{tabular*}}}

\newcommand{\mozartstyle@D}{%
  \renewcommand{\mozart@topcomic}{%
    \begin{pspicture}(20,12)%
      \rput[tl](0,12){\@comic}%
      \psline(14,12)(14,-1)%
      \psline(2,1)(15.5,1)%
    \end{pspicture}}}

\newcommand{\mozartstyle@E}{%
  \renewcommand{\mozart@topcomic}{%
    \begin{pspicture}(20,12)%
      \rput[tl](0,12){\@comic}%
      \pstriangle[fillstyle=solid,fillcolor=lightgray,linestyle=none](14,-1)(0.5,15)%
      \psline[linecolor=gray,linewidth=1mm](2,1)(15.5,1)%
    \end{pspicture}}}

\newcommand{\mozartstyle@F}{%
  \renewcommand{\mozart@topcomic}{%
    \begin{pspicture}(20,12)%
      \rput[tl](0,12){\@comic}%
      \pstriangle[fillstyle=solid,fillcolor=lightgray,linestyle=none](14,-1)(0.5,15)%
      \psline[linecolor=gray,linewidth=1mm](10,1)(15.5,1)%
    \end{pspicture}}}

\newcommand{\mozartstyle@G}{%
  \renewcommand{\mozart@bottomcomic}{}%
  \renewcommand{\mozart@topcomic}{%
    \begin{pspicture}(20,12)%
      \rput[tl](0,12){\@comic}%
      \pstriangle[fillstyle=solid,fillcolor=lightgray,linestyle=none](14,-1)(0.5,15)%
      \rput[r](15.5,1){\mozart@logo}%
    \end{pspicture}}}

\newcommand{\mozartstyle@H}{%
  \renewcommand{\mozart@topcomic}{%
    \begin{pspicture}(20,12)%
      \rput[tl](0,12){\@comic}%
      \pstriangle[fillstyle=solid,fillcolor=lightgray,linestyle=none](14,-9)(0.5,23)%
    \end{pspicture}}}

\newrgbcolor{I@color}{0.52941 0.8078431 0.9803921}
%\newrgbcolor{I@color}{0.62941 0.8078431 0.9803921}
%\newrgbcolor{I@color}{0.7 0.9 1}
% cornflower:
%\newrgbcolor{I@color}{0.3921 0.58431 0.92941}
\newcommand{\mozartstyle@I}{%
  \renewcommand{\mozart@logo@color}{color}%
  \renewcommand{\mozart@topcomic}{%
    \begin{pspicture}(20,12)%
      \rput[tl](0,12){\@comic}%
      \pstriangle[fillstyle=solid,fillcolor=I@color,linestyle=none](14,-9)(0.5,23)%
    \end{pspicture}}}

% no frills, just the logo and the comic
\newcommand{\mozartstyle@J}{%
  \renewcommand{\mozart@logo@color}{color}%
  \renewcommand{\mozart@topcomic}{%
    \begin{pspicture}(20,12)%
      \rput[tl](0,12){\@comic}%
    \end{pspicture}}}

%\mozartstyle{I}

\newcommand{\mozart@titlepage}{%
\bgroup
  \pagestyle{empty}\thispagestyle{empty}%
  \@nameuse{mozartstyle@\@mozartstyle}%
  \begin{titlepage}%
    \begin{mozartFULLWIDTH}%
      \mozart@topcomic%
      \vspace*{2cm}%
      \begin{flushright}
        \ifx\@title\@empty\else{\mozart@title@font@one\@title\\[1cm]}\fi
        \ifx\@author\@empty\else{\mozart@title@font@two\@author\\[1cm]}\fi
      \end{flushright}%
      \vfill
      \mozart@title@font@three
      \noindent
      \mozart@titlepage@bottom@left\hfill\hbox{}%
      \mozart@titlepage@bottom@right
    \end{mozartFULLWIDTH}%
  \end{titlepage}%
\egroup}

\providecommand{\mozart@titlepage@bottom@left}{%
  \begin{tabular}{@{}l@{}}%
    Version \mozart@version\\\mozart@date
  \end{tabular}%
}

\providecommand{\mozart@titlepage@bottom@right}{%
  \mozart@bottomcomic}

\newcommand{\mozart@logo@color}{grey}
\newcommand{\mozart@logo}{%
  \lower10mm\hbox{\@PICEXT{mozart-\mozart@logo@color}{5cm}}%
}
\newcommand{\@logoauthor}{Mozart logo by Christian Lindig}

\newlength{\mozart@quote@indent}
\setlength{\mozart@quote@indent}{1cm}
\newenvironment{mozart@quote}
  {\begin{list}{}{%
     \leftmargin\mozart@quote@indent
     \rightmargin\mozart@quote@indent}
   \item[]\ignorespaces}
  {\end{list}}

\newif\if@mozartcredits

\newcommand{\mozart@license}{%
  \clearpage
  \pagestyle{empty}\thispagestyle{empty}%
  \begin{mozartFULLWIDTH}%
    \ifx\@abstract\long@empty\else
      \begin{center}\mozart@section@font\Large Abstract\end{center}%
      \begin{mozart@quote}\@abstract\end{mozart@quote}%
    \fi
    \vspace*{\fill}%
    \@mozartcreditsfalse
    \ifx\@comicauthor\long@empty\else\@mozartcreditstrue\fi
    \ifx\@logoauthor\long@empty\else\@mozartcreditstrue\fi
    \if@mozartcredits
    \begin{center}\mozart@section@font\Large Credits\end{center}%
    \begin{center}\footnotesize
      \@comicauthor
      \@logoauthor
    \end{center}%
    \fi
    \begin{center}\mozart@section@font
      \Large License Agreement%
    \end{center}%
    \begin{mozart@quote}\footnotesize%
      This software and its documentation are copyrighted by the
      German Research Center for
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
      PARTICULAR PURPOSE, AND NON-IN\-FRIN\-GE\-MENT.  THIS SOFTWARE
      AND ITS DOCUMENTATION ARE
      PROVIDED ON AN "AS IS" BASIS, AND THE AUTHORS AND DISTRIBUTORS
      HAVE NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
      ENHANCEMENTS, OR MODIFICATIONS.
    \end{mozart@quote}%
  \end{mozartFULLWIDTH}}

\long\def\@abstract{}
\newcommand{\mozartAbstract}[1]{%
  \long\def\@abstract{#1}}
\newcommand{\long@empty}{}

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

\newlength{\mozart@len}

\newcommand{\mozart@floatc@ruled}[2]{%
  \setlength{\mozart@len}{\mozart@fullwidth}%
  \addtolength{\mozart@len}{-2\fboxsep}%
  \colorbox{menucolor}{\parbox{\mozart@len}{\sffamily{\bfseries #1} #2}}}

\newcommand{\fs@mozart}{\def\@fs@cfont{\bfseries}\let\@fs@capt\mozart@floatc@ruled
  \def\@fs@pre{\hrule height.8pt depth0pt \kern2pt}%
  \def\@fs@post{\kern2pt\hrule\relax}%
  \def\@fs@mid{\kern2pt\hrule\kern2pt}%
  \let\@fs@iftopcapt\iftrue}
\floatstyle{mozart}
\newfloat{mozartFIGURE}{htbp}{lof}[chapter]
\floatname{mozartFIGURE}{Figure}

% \renewcommand{\floatc@ruled}[2]{
%   \setlength{\mozart@len}{\mozart@fullwidth}%
%   \addtolength{\mozart@len}{-2\fboxsep}%
%   \colorbox{menucolor}{\parbox{\mozart@len}{{\@fs@cfont #1} #2}}}

% XSL SUPPORT

% support for strange characters

\DeclareRobustCommand\mozart@text@HASH{\#}
\DeclareRobustCommand\mozart@text@DOLLAR{\$}
\DeclareRobustCommand\mozart@text@PERCENT{\%}
\DeclareRobustCommand\mozart@text@AMPERSAND{\&}
\DeclareRobustCommand\mozart@text@TILDE{\char`\~}
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
\DeclareRobustCommand\mozart@SPACE{\nobreakspace}
\newcommand{\mozartSPACE}[1]{\mozart@SPACE}
\newcommand{\mozartNEWLINE}{\mozart@NEWLINE}

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
  \let\mozart@UNDERSCORE\mozart@other@UNDERSCORE
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

\newcommand{\mozartPI}[1]{%
  \@ifundefined{usepi@#1}{%
    \@latex@error{unknown processing instruction: #1}\@ehc}%
    {\@nameuse{usepi@#1}}}
\newcommand{\usepi@eg}{e.g.\@\xspace}
\newcommand{\usepi@ie}{i.e.\@\xspace}
\newcommand{\usepi@nbsp}{~}
\newcommand{\usepi@NBSP}{~}
\newcommand{\usepi@LaTeX}{\LaTeX{}}
\newcommand{\usepi@LATEX}{\LaTeX{}}
\newcommand{\usepi@ellipsis}{\ldots}
\newcommand{\usepi@etc}{etc.}
\newcommand{\usepi@resp}{resp.\@\xspace}
\newcommand{\usepi@slash}{/}
\newcommand{\usepi@cdots}{\ensuremath{\cdots}}
% existing &emdash; should probably be changed to &endash;
% and \usepi@emdash should expand to ---
\newcommand{\usepi@emdash}{--}
\newcommand{\usepi@endash}{--}
\newcommand{\usepi@EG}{\usepi@eg}
\newcommand{\usepi@wrt}{wrt.\@\xspace}

% sectioning commands

\newcommand{\mozartPART}{\part}
\newcommand{\mozartCHAPTER}{\chapter}
\newcommand{\mozartAPPENDIX}{\mozart@appendix\chapter}
\newcommand{\mozartSECTION}{\section}
\newcommand{\mozartSUBSECTION}{\subsection}
\newcommand{\mozartSUBSUBSECTION}{\subsubsection}
\newcommand{\mozartPARA}{\paragraph}

\newcommand{\mozart@appendix}{%
  \appendix\global\let\mozart@appendix\relax}

% display

\newlength{\mozart@displaymargin}
\setlength{\mozart@displaymargin}{1cm}

\newenvironment{mozartDISPLAY}
  {\begin{list}{}{\leftmargin\mozart@display@use@margin}%
     \def\mozart@display@use@margin{\mozart@displaymargin}%
     \item[]\ignorespaces}
  {\end{list}}

\newcommand{\mozart@display@use@margin}{\mozart@displaymargin}

\newenvironment{mozartFIGURENODISPLAY}
  {\begin{mozartFIGURE}\def\mozart@display@use@margin{\z@}}
  {\end{mozartFIGURE}}

\newcommand{\mozart@code@family}{code}
\newcommand{\mozart@code@font}{%
  \fontencoding{T1}%
  \fontfamily{\mozart@code@family}%
  \fontshape{\shapedefault}\selectfont\hyphenchar\font\m@ne}
\newcommand{\@CODEINLINE}[1]{\mozart@code@font\mozart@specials@code@inline#1\egroup}
\DeclareRobustCommand{\mozartCODEINLINE}{\bgroup\let\mozartFACETEXT\mozartINLINEFACETEXT\@CODEINLINE}
\newcommand{\mozartCODEEXTERN}[1]{%
  \begin{mozartDISPLAY}\mozart@code@font\verbatiminput{#1}\end{mozartDISPLAY}}

%% all newlines in code are encoded using \mozartNEWLINE
%% remaining newlines are needed to keep lines
%% short enough to be processed by TeX.  Hence the
%% \let\obeylines\@empty.  Really, we don't want alltt
%% but something much like it that neither uses
%% obeyspaces nor obeylines... maybe some day.

\newenvironment{mozartCODEDISPLAY}
  {\let\obeylines\@empty
   \begin{mozartDISPLAY}\begin{alltt}\mozart@code@font\mozart@specials@code@display}
  {\end{alltt}\end{mozartDISPLAY}}

\definecolor{comment}{rgb}{0.698039,0.133333,0.133333}
\definecolor{keyword}{rgb}{0.627451,0.125490,0.941176}
\definecolor{string}{rgb}{0.737255,0.560784,0.560784}
\definecolor{function}{rgb}{0.000000,0.000000,1.000000}
\definecolor{type}{rgb}{0.133333,0.545098,0.133333}
\definecolor{variable}{rgb}{0.721569,0.525490,0.043137}
\definecolor{reference}{rgb}{0.372549,0.619608,0.627451}
\definecolor{builtin}{rgb}{0.854902,0.439216,0.839216}

\DeclareRobustCommand{\mozartFACECOMMENT}{\textcolor{comment}}
\DeclareRobustCommand{\mozartFACEKEYWORD}[1]{\textbf{\color{keyword}#1}}
\DeclareRobustCommand{\mozartFACESTRING}{\textcolor{string}}
\DeclareRobustCommand{\mozartFACEVARIABLE}{\textcolor{variable}}
\DeclareRobustCommand{\mozartFACEFUNCTION}{\textcolor{function}}
\DeclareRobustCommand{\mozartFACEBUILTIN}{\textcolor{builtin}}
\DeclareRobustCommand{\mozartFACEREFERENCE}{\textcolor{reference}}
\DeclareRobustCommand{\mozartFACETYPE}{\textcolor{type}}
\DeclareRobustCommand{\mozartFACETEXT}[1]{#1}%strip the braces

\definecolor{chunkborder}{rgb}{1.0,0.647,0}
\definecolor{chunktitle}{rgb}{0.4196,0.556862745,0.1372549}
\definecolor{chunkref}{rgb}{0.4156862745,0.352941,0.803921}

\definecolor{inlinecode}{rgb}{0.4,0.2,0.4}
\DeclareRobustCommand{\mozartINLINEFACETEXT}{\textcolor{inlinecode}}

\definecolor{menucolor}{rgb}{0.8,0.8,0.8}

% variables and annotations

\newcommand{\mozartMODE}[1]{\csname MODE@#1\endcsname}
\newcommand{\MODE@IN}{\ensuremath{+}}
\newcommand{\MODE@OUT}{\ensuremath{?}}
\newcommand{\MODE@CIN}{\ensuremath{*}}
\newcommand{\MODE@CNIN}{\ensuremath{\$}}
\newcommand{\mozartMETAVAR}[1]{{\normalfont\textit{#1}}}
\newcommand{\mozartENVVAR}[1]{{\normalfont\texttt{#1}}}
\newcommand{\mozartGRAMMARVAR}[1]{{\normalfont
  \ensuremath{\langle\textsf{#1}\rangle}}}
\newcommand{\mozartPROGVAR}[1]{{\normalfont\texttt{#1}}}

% informative elements

\newcommand{\mozartFILE}{\texttt}
\newcommand{\mozartSAMP}{\texttt}
\newcommand{\mozartEM}{\emph}
\newcommand{\mozartKBD}{\texttt}
\newcommand{\mozartKEY}{\texttt}

% references

\newcommand{\mozartREFEXTTO}{\textsl}
\newcommand{\mozartREFEXTKEY}[1]{ (#1)}
\newcommand{\mozartDEFAULTREFEXT}{\texttt}

\newcommand{\mozart@noweird}{\useochar@defs\def\usechar##1{##1}\def\protect{}}

\AtBeginDocument{%
\let\latex@ref\ref
\let\latex@pageref\pageref
\let\latex@label\label
\def\ref#1{{\mozart@specials@ref\latex@ref{#1}}}%
\def\pageref#1{{\mozart@specials@ref\latex@pageref{#1}}}%
\def\label#1{\@bsphack
  {\mozart@specials@ref\xdef\mozart@temp{#1}}%
  \expandafter\latex@label\expandafter{\mozart@temp}}}

%\def\mozart@label#1{%
%  \protected@write\@auxout{}%
%         {\string\newlabel{#1}{{\@currentlabel}{\thepage}}}%
%  \@esphack}

% figures

\newcommand{\mozartCAPTIONID}[1]{\caption{\label{#1}\kern0pt\relax}}
\newenvironment{mozartFIGTITLE}
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

\newcommand{\mozart@cut@extension}[1]{%
  \@mozartExtensionfalse
  \if@mozartExtension\else\mozart@hasExtension{#1}{.ps}\fi
  \if@mozartExtension\else\mozart@hasExtension{#1}{.eps}\fi
  \if@mozartExtension\else\mozart@hasExtension{#1}{.ps.gz}\fi
  \if@mozartExtension\else\mozart@hasExtension{#1}{.eps.gz}\fi
  \if@mozartExtension\else
    \def\mozart@basename{#1}%
    \def\mozart@extension{}%
  \fi}

\newcommand{\@PICEXT}[2]{% #1=filename #2=width
  \mozart@cut@extension{#1}%
  \def\mozart@temp{#2}%
  \ifx\mozart@temp\@empty
    \def\@@PICEXT{\epsfig{file=\mozart@basename}}%
  \else
    \def\@@PICEXT{\epsfig{file=\mozart@basename,width=#2}}%
  \fi
  {\mozart@specials@other\@@PICEXT}}

\newcommand{\mozartPICEXTINLINE}[2][]{\@PICEXT{#2}{#1}}
% \PICEXT[width]{file}
\newcommand{\mozartPICEXT}[2][0.7\linewidth]{\@PICEXT{#2}{#1}}

%\newcommand{\PICEXT}[1]{{\mozart@specials@other
%  \epsfig{file=#1,width=0.7\linewidth}}}
\newenvironment{mozartPICEXTDISPLAY}
  {\begin{center}\begin{minipage}{\linewidth}\begin{center}\mozart@specials@other}
  {\end{center}\end{minipage}\end{center}}
\newcommand{\mozartPICEXTFULL}[2][]{\@PICEXT{#2}{#1}}
%\newcommand{\PICEXTFULL}[1]{{\mozart@specials@other
%  \epsfig{file=#1,width=\linewidth}}}
\newcommand{\mozartNOTEGUIPIC}[1]{\@PICEXT{#1}{8mm}}
%\newcommand{\NOTEGUIPIC}[1]{{\mozart@specials@other
%  \epsfig{file=#1,width=8mm}}}
\newcommand{\mozartNOTEGUI}[2]{%
  \paragraph[#1]{#1 \mozartNOTEGUIPIC{#2}}}
%% different from \PICEXT because here we want to add a .ps
%% extension that doesn't get stripped off
\newcommand{\mozartPICEXTGIF}[2][0.7\linewidth]{{\mozart@specials@other
  \epsfig{file=#2.ps,width=#1}}}

% grammar stuff

\newcommand{\mozartGRAMDEF}{\ensuremath{{:}{:}{=}}}
\newcommand{\mozartGRAMADD}{\ensuremath{{+}{=}}}
\newcommand{\mozartGRAMOR}{\ensuremath{\mid}}
\newcommand{\mozartGRAMSPACE}{ }
\newcommand{\mozartGRAMMARNOTE}[1]{\textsl{\% #1}}

% literate programming stuff
% \begin{CHUNK}{TITLE}{CHUNK.ID}{CHUNK.NUM}

\newcounter{mozart@chunk}
\setcounter{mozart@chunk}{0}
\newcounter{mozart@chunk@subpage}
\setcounter{mozart@chunk@subpage}{0}

\newcommand{\mozart@chunk@prev@page}{}
\newcommand{\mozart@chunk@curr@page}{}

\ifmozart@hyper
\def\mozart@pick@first#1#2#3#4#5{#1}
\def\mozart@pick@second#1#2#3#4#5{#2}
\else
\def\mozart@pick@first#1#2{#1}
\def\mozart@pick@second#1#2{#2}
\fi

\newcommand{\mozart@chunk@init}{%
  \xdef\mozart@chunk@prev@page{\mozart@chunk@curr@page}%
  \stepcounter{mozart@chunk}%
  \@ifundefined{r@chunk.glob.\themozart@chunk}%
    {\xdef\mozart@chunk@curr@page{\thepage}}%
    {\xdef\mozart@chunk@curr@page{%
       \expandafter\expandafter
       \expandafter\mozart@pick@second
       \csname r@chunk.glob.\themozart@chunk\endcsname}}%
  \ifnum\themozart@chunk=\@ne
    \xdef\mozart@chunk@prev@page{\mozart@chunk@curr@page}%
  \fi
  \ifnum\mozart@chunk@prev@page=\mozart@chunk@curr@page
    \stepcounter{mozart@chunk@subpage}%
  \else
    \setcounter{mozart@chunk@subpage}{1}%
  \fi}

\newcommand{\chunk@ref@font}{\normalfont
  \sffamily\small}

\newcommand{\mozart@chunk@label}{%
  \llap{\hbox to1cm{\chunk@ref@font\hfil
    \mozart@chunk@curr@page\alph{mozart@chunk@subpage}\quad}}}

% \mozart@chunk@ref@bwd{CHUNK.ID}{CHUNK.NUM}

\newcommand{\mozart@chunk@ref}[2]{%
  \expandafter\expandafter\expandafter\mozart@chunk@ref@fmt
  \csname r@chunk.#1.#2\endcsname}
\newcommand{\mozart@chunk@ref@fmt}[2]{%
  {\chunk@ref@font\fontsize{8}{8pt}\selectfont#2\@alph{#1}}}

\newcommand{\mozart@chunk@ref@bwd}[2]{%
  \hbox to5mm{%
    \mozart@cnt#2\relax
    \advance\mozart@cnt\m@ne
    \@ifundefined{r@chunk.#1.\the\mozart@cnt}%
      {}%
      %{$\triangleleft$\chunk@ref@font\small\pageref{chunk.#1.\the\mozart@cnt}\@alph{\ref{chunk.#1.\the\mozart@cnt}}}%
      {$\triangleleft$\mozart@chunk@ref{#1}{\the\mozart@cnt}}%
    \hfil}}

\newcommand{\mozart@chunk@ref@fwd}[2]{%
  \hbox to5mm{%
    \hfil
    \mozart@cnt#2\relax
    \advance\mozart@cnt\@ne
    \@ifundefined{r@chunk.#1.\the\mozart@cnt}%
      {}%
      %{{\chunk@ref@font\small\pageref{chunk.#1.\the\mozart@cnt}\@alph{\ref{chunk.#1.\the\mozart@cnt}}}$\triangleright$}
      {\mozart@chunk@ref{#1}{\the\mozart@cnt}$\triangleright$}%
}}

\newcommand{\mozart@chunk@equiv}[1]{%
  \ifnum#1=1\else{+}\fi{\equiv}}

\newcommand{\mozart@chunk@ref@def}[1]{%
  \@ifundefined{r@chunk.#1.1}{}%
    {\edef\mozart@temp{\expandafter\expandafter
       \expandafter\mozart@pick@first\csname r@chunk.#1.1\endcsname}%
     ~~{\chunk@ref@font\pageref{chunk.#1.1}\@alph{\mozart@temp}}}}

\newenvironment{mozartCHUNK}[3]
  {\begin{list}{}{\leftmargin\mozart@displaymargin}%
     \mozart@chunk@init
     \item[\mbox{\mozart@chunk@label\bf$\color{chunkborder}\langle$%
       \textcolor{chunktitle}{#1\mozart@chunk@ref@def{#2}}%
       $\color{chunkborder}\rangle\mozart@chunk@equiv{#3}$}]%
     \edef\@currentlabel{\themozart@chunk@subpage}%
     \label{chunk.glob.\themozart@chunk}%
     \label{chunk.#2.#3}%
     \topsep\z@\parskip\z@\partopsep\z@\parsep\z@\itemsep\z@
     \let\obeylines\@empty
     \hfill\hbox{\rlap{\mozart@chunk@ref@bwd{#2}{#3}%
                       \hskip3mm
                       \mozart@chunk@ref@fwd{#2}{#3}}}%
     \begin{alltt}\mozart@code@font\mozart@specials@code@display}
  {\end{alltt}\end{list}}

\newenvironment{mozartCHUNKANON}[3]
  {\begin{mozartCODEDISPLAY}}
  {\end{mozartCODEDISPLAY}}

\newcommand{\mozartCHUNKREF}[2]{%
  \mbox{\normalfont\bf$\color{chunkborder}\langle$%
    \textcolor{chunkref}{#1\mozart@chunk@ref@def{#2}}%
    $\color{chunkborder}\rangle$}}

% (entry,synopsis*)+,item

%% for the documentation of the OPI it looks like the space
%% between the command and the keychord that invokes it has
%% disappeared, but in fact it is there; it is just very small
%% because the interword spacing in times in very small.

\newenvironment{mozartDESCRIPTION}
  {\begin{list}{}{%
     \leftmargin\z@\rightmargin\z@
     \itemindent\z@
     \labelwidth\z@
     \let\makelabel\DESCRIPTION@label
     \let\ENTRY@MBOX\relax\let\mozartITEM\@ITEM}}
  {\end{list}}

\newcommand{\DESCRIPTION@label}[1]{%
  \hspace{-\mozart@leftmarginwidth}%
  \hspace{\labelsep}\bf#1\hfill}

\newenvironment{mozartSYNOPSIS}
  {\noindent\begin{list}{}{%
     \topsep-\parskip\partopsep\z@
     \leftmargin\mozart@displaymargin}%
   \item[]\ignorespaces}
  {\end{list}\aftergroup\SYN@ITEM}

\def\SYN@ITEM{\let\mozartITEM\@ITEM}

\newcommand{\ENTRY@@MBOX}{%
  \mbox{}\vspace{-\parsep}\vspace{-\itemsep}}
\newcommand{\mozartENTRY}[1]{\ENTRY@MBOX\item[{\let\par\relax#1}]\let\ENTRY@MBOX\ENTRY@@MBOX}
\newcommand{\mozartENTRYWITHCATEGORY}[2]{%
  \mozartENTRY{\llap{\mozartCATEGORY{#1}}#2}%
}

\definecolor{category}{rgb}{0.415686,0.352941,0.80392}
\newcommand{\mozartCATEGORY}[1]{{\normalfont\footnotesize\color{category}\textsf{#1}\quad}}

\newcommand{\mozartENTRYHASCODE}{%
  \let\mozartITEM\ITEM@MBOX}
\newcommand{\ITEM@MBOX}{%
  \let\ENTRY@MBOX\relax\mbox{}\par\noindent}
\newcommand{\@ITEM}{%
  \let\ENTRY@MBOX\relax\vspace{\parsep}\noindent}
\newcommand{\mozartITEMnotes}{%
  \mozartITEM\let\mozartITEM\relax}

%\def\mozart@temp{\long\def\@item[##1]}
%\expandafter\mozart@temp\expandafter{\the\@item}

\let\latex@item\@item
\long\def\@item[#1]{\def\mozart@item@arg{#1}\latex@item[\mozart@item@arg]}

\newcommand{\mozartMENU}[3]{%
  \colorbox{menucolor}{%
    \setlength{\mozart@len}{\mozart@leftmarginwidth}%
    \addtolength{\mozart@len}{1cm}%
    \begin{tabular}[b]{@{}p{\mozart@len}p{1cm}@{}}%
      \normalfont\sffamily\small#1&#2%
    \end{tabular}}~#3}

\newcommand{\mozartMOUSE}[1]{\hbox{\lower3mm\hbox{\epsfig{file=mouse-#1.ps,width=1cm}}}}

% exercises and answers

\theoremheaderfont{\sffamily\bfseries}
\newtheorem{@EXERCISE}{Exercise}[chapter]
\newenvironment{mozartEXERCISE}[1]
  {\begin{@EXERCISE}\label{#1}\let\mozartPARA\mozart@internal@paragraph}
  {\end{@EXERCISE}}
% note the `*' in \paragraph*{... \ref{...}}
% to avoid messing up the \ref{...}.  There is probably
% a better more general way...
\newenvironment{mozartANSWER}[1]
  {\paragraph*{Answer to exercise \ref{#1}.}}
  {\par}

% rewrite rules

\newenvironment{mozartREWRITE}
  {\begin{flushleft}\begin{tabular}{|l@{\quad\ensuremath{\rightarrow}\quad}l|}\hline}
  {\end{tabular}\end{flushleft}}
\newcommand{\mozartREWRITEVAR}[1]{%
  \multicolumn{2}{|l|}{#1 \mozartGRAMDEF}\\\hline}
\newcommand{\mozartREWRITEFROM}[1]{\begin{mozartMINCODE}#1\end{mozartMINCODE}&}
\newcommand{\mozartREWRITETO}[1]{\begin{mozartMINCODE}#1\end{mozartMINCODE}\\\hline}
\newcommand{\mozartREWRITECONDITION}[1]{\multicolumn{2}{|@{\qquad}p{0.7\linewidth}|}{#1}\\\hline}

\newcommand{\mozartEOLN}{\\\mozartEMPTY}

\newenvironment{mozartMINCODE}
  {\vtop\bgroup\mincode@line}
  {\mincode@end\egroup}

\newcommand{\mincode@end}{}
\newcommand{\mincode@newline}{\egroup\mincode@line}

\newcommand{\mincode@line}{\hbox\bgroup\strut
  \let\mozartNEWLINE\mincode@newline
  \let\mozart@NEWLINE\mincode@newline
  \let\mozart@code@NEWLINE\mincode@newline
  \let\mozart@code@NEWLINE@display\mincode@newline
  \let\mincode@end\egroup}

% ozdoc external cross references

\newcommand{\mozartOZDOCREF}[3]{%
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

\newcommand{\mozartOZDOCREFTOP}[2]{%
  \def\mozart@temp{#2}%
  \ifx\mozart@temp\@empty
    \@latex@error{unknown cross ref: to=#1}\@ehc
  \else
    \@OZDOCREFTOP{#2}%
  \fi}

\newcommand{\@OZDOCREFTOP}[1]{%
  {\normalfont\textsl{``#1''}}}

%% table of contents should also use \mozart@section@font
\let\latex@l@part               \l@part
\let\latex@l@chapter            \l@chapter
\let\latex@l@section            \l@section
\let\latex@l@subsection         \l@subsection
\let\latex@l@subsubsection      \l@subsubsection
\let\latex@l@paragraph          \l@paragraph
\let\latex@subparagraph         \l@subparagraph

\newcommand{\mozart@toc@font}{\sffamily}

\renewcommand*\l@part[1]{\latex@l@part{\mozart@toc@font#1}}
\renewcommand*\l@chapter[1]{\latex@l@chapter{\mozart@toc@font#1}}
\renewcommand*\l@section[1]{\latex@l@section{\mozart@toc@font#1}}
\renewcommand*\l@subsection[1]{\latex@l@subsection{\mozart@toc@font#1}}
\renewcommand*\l@subsubsection[1]{\latex@l@subsubsection{\mozart@toc@font#1}}
\renewcommand*\l@paragraph[1]{\latex@l@paragraph{\mozart@toc@font#1}}
\renewcommand*\l@subparagraph[1]{\latex@l@subparagraph{\mozart@toc@font#1}}

%% DTD doc support

\newcommand{\mozartDTDTAG}{\texttt}
\newcommand{\mozartDTDATTRIB}{\texttt}
\newcommand{\mozartDTDPI}{\texttt}

%%

\newcommand{\mozartPAGEREF}{\protect\pageref}
\newcommand{\mozartREF}{\protect\ref}

%% indexing

\newcommand{\mozartINDEX}{\index}
\AtEndDocument{\mozart@printindex}
\newcommand{\mozart@printindex}{}
\newcommand{\mozartDOINDEX}{%
  \usepackage{makeidx}\makeindex
  \def\mozart@printindex{\clearpage\let\@mkboth\markboth\printindex}}

%% opi support

\newcommand{\mozartOPINAMEBUFFER}[1]{%
  {\mozart@code@font\mozart@specials@code@inline#1}}

%% funky array support

\providecommand{\tableHeaderBackground}{yellowgreen}
\providecommand{\tableBackground}{lightgoldenrodyellow}
\providecommand{\mozartTableHeader}{%
  \mozart@section@font\large}

%\definecolor{beige}{rgb}{0.96078,0.96078,0.86275}
\definecolor{yellowgreen}{rgb}{0.60392,0.80392,0.19608}
\definecolor{lightgoldenrodyellow}{rgb}{0.98039,0.98039,0.82353}
%\definecolor{gold}{rgb}{1,0.84314,0}

\newcolumntype{D}{>{\columncolor{\tableBackground}}}
\newcolumntype{H}{>{\columncolor{\tableHeaderBackground}\mozartTableHeader}}
\newcolumntype{L}{Dl}
\newcolumntype{R}{Dr}
\newcolumntype{C}{Dc}
\newcolumntype{P}{Dp}
\newcolumntype{T}{Dt}
\newcolumntype{B}{Db}

%\definecolor{tableTD}{rgb}{0.96078431,0.8741,0.70549}
%\definecolor{tableTH}{rgb}{0.3921,0.58431,0.92941}
%\newcolumntype{D}{>{\columncolor{tableTD}}l}
%\newcolumntype{H}{>{\columncolor{tableTH}\mozartTH}l}
%\newcommand{\TRBEGIN}{\rowcolor{tableTD}}

%% scaled fonts for code

\DeclareFontFamily{T1}{code}{}

\DeclareFontShape{T1}{code}{m}{n}{
   <-> s * [0.85] pcrr8t
}{}

\DeclareFontShape{T1}{code}{m}{sc}{
   <-> s * [0.85] pcrrc8t
}{}

\DeclareFontShape{T1}{code}{m}{sl}{
   <-> s * [0.85] pcrro8t
}{}

\DeclareFontShape{T1}{code}{b}{n}{
   <-> s * [0.85] pcrb8t
}{}

\DeclareFontShape{T1}{code}{b}{sc}{
   <-> s * [0.85] pcrbc8t
}{}

\DeclareFontShape{T1}{code}{b}{sl}{
   <-> s * [0.85] pcrbo8t
}{}

\DeclareFontShape{T1}{code}{m}{it}{<->ssub * code/m/sl}{}
\DeclareFontShape{T1}{code}{bx}{n}{<->ssub * code/b/n}{}
\DeclareFontShape{T1}{code}{bx}{sc}{<->ssub * code/b/sc}{}
\DeclareFontShape{T1}{code}{bx}{sl}{<->ssub * code/b/sl}{}
\DeclareFontShape{T1}{code}{b}{it}{<->ssub * code/b/sl}{}
\DeclareFontShape{T1}{code}{bx}{it}{<->ssub * code/b/it}{}

%% \mozartBibAdd{FILENAME}
%%      accumulates bibliography files
%% \mozartBibDo
%%      issues the corresponding
%%              \bibliographystyle{...}
%%              \bibliography{FILE1,...,FILEn}

\def\mozart@bibfiles{}
\newcommand{\mozartBibAdd}[1]{%
  \@ifundefined{mozbib@#1}%
    {\@namedef{mozbib@#1}{}%
     \ifx\mozart@bibfiles\@empty
       \gdef\mozart@bibfiles{#1}%
     \else
       \xdef\mozart@bibfiles{\mozart@bibfiles,#1}\fi}%
    {}}
\newcommand{\mozart@bibstyle}{plain}
\newcommand{\mozartBibDo}{%
  \bibliographystyle{\mozart@bibstyle}%
  \bibliography{\mozart@bibfiles}}
