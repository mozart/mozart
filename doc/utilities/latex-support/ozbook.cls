% Copyright (C) by Denys Duchier, April 1996, Universitaet des Saarlandes

\def\fileversion{v1.0}
\def\filedate{4 April 1996}
\def\docdate {4 April 1996}

% Identification
\NeedsTeXFormat{LaTeX2e}
\ProvidesClass{ozbook}[1996/04/04 Oz HandBook Class (Denys Duchier)]

% Initial Code
\RequirePackage{times}

% Declaration of Options
\newif\if@coverPage \@coverPagetrue
\newif\if@titlePage \@titlePagetrue
\newif\if@tableofcontents \@tableofcontentstrue
\newif\if@pdf \@pdffalse

\DeclareOption{nocover}{\@coverPagefalse}
\DeclareOption{notitle}{\@titlePagefalse}
\DeclareOption{notableofcontents}{\@tableofcontentsfalse}
\DeclareOption{pdf}{\@pdftrue}
\DeclareOption{html}{}
\DeclareOption*{\PassOptionsToClass{\CurrentOption}{book}}

% Execution of Options
\ProcessOptions

% Package Loading
\LoadClass[a4paper,11pt]{book}
\RequirePackage{index}
\RequirePackage{float}
\RequirePackage{verbatim}
\RequirePackage{multicol}
\RequirePackage{graphicx}% for \includegraphics
\RequirePackage{pstricks}
\RequirePackage{pst-node}
\RequirePackage{latexsym}
\RequirePackage{epsf}
\RequirePackage{html}

% Main Code
%
% LAYOUT PARAMETERS
%
\newlength{\fullwidth}
\newlength{\leftmarginwidth}
% this should depend on the paper size
\setlength{\leftmarginwidth}{1.7in}
\addtolength{\oddsidemargin}{-0.6cm}
\addtolength{\textwidth}{3.14cm}
\addtolength{\topmargin}{-1.5cm}
\addtolength{\textheight}{2.6cm}
\setlength{\fullwidth}{\textwidth}
\addtolength{\textwidth}{-\leftmarginwidth}
\addtolength{\oddsidemargin}{\leftmarginwidth}
\setlength{\evensidemargin}{\oddsidemargin}
\setlength{\parindent}{0pt}
\setlength{\parskip}{6pt plus 3pt minus 2pt}
\raggedbottom
\setlength{\marginparsep}{8mm}
\setlength\marginparwidth\leftmarginwidth
\addtolength{\marginparwidth}{-\marginparsep}

% SECTIONAL COMMANDS
%
% redefine \section, \subsection and \subsubsection to (1) hang out
% in the left margin, but in such a way that they may extend over
% several lines, and (2) so that the vertical space above and below
% is a little more compact than in bookstyle.

\newcommand{\hanging@section}{%
  \raggedright\leftskip-\leftmarginwidth}

\renewcommand\section{\@startsection
	{section}{1}{\z@}%
	{-3ex plus -1ex minus -.2ex}%
	{2ex plus .2ex}%
	{\hanging@section\normalfont\Large\bfseries}}

\renewcommand\subsection{\@startsection
	{subsection}{1}{\z@}%
	{-2ex plus -1ex minus -.2ex}%
	{1.3ex plus .2ex}%
	{\hanging@section\normalfont\large\bfseries}}

\renewcommand\subsubsection{\@startsection
	{subsubsection}{1}{\z@}%
% at Gert's request:
% Changed vertical spacing to match that of sections
%	{-.5ex plus -1ex minus -.2ex}%
%	{.3ex plus .2ex}%
	{-2ex plus -1ex minus -.2ex}%
	{1.3ex plus .2ex}%
	{\hanging@section\normalfont\bfseries}}

\renewcommand{\chaptermark}[1]{\markboth{#1}{}}

\renewcommand{\@makechapterhead}[1]{%
\thispagestyle{empty}%
  \vspace*{10\p@}%
  {\parindent \z@ \raggedleft \normalfont
    \ifnum \c@secnumdepth >\m@ne
      \if@mainmatter
        \huge\bfseries\thechapter\quad
        \par\nobreak
        \vskip 4\p@
      \fi
    \fi
    \interlinepenalty\@M
    \advance\leftskip-\leftmarginwidth
    \advance\linewidth\leftmarginwidth
    \rule{\linewidth}{1.3\p@}\par\nobreak
    \Huge \bfseries #1\par\nobreak
    \vskip 40\p@
  }}

\renewcommand{\@makeschapterhead}[1]{%
\thispagestyle{empty}%
  \vspace*{10\p@}%
  {\parindent \z@ \raggedleft
    \normalfont
    \interlinepenalty\@M
    \advance\leftskip-\leftmarginwidth
    \advance\linewidth\leftmarginwidth
    \rule{\linewidth}{1.3\p@}\par\nobreak
    \Huge \bfseries #1\par\nobreak
    \vskip 40\p@
  }}


% HEAD AND FOOT - PAGE STYLE

\newlength{\headrulewidth}
\setlength{\headrulewidth}{0.4pt}
\newcommand{\@ozhead}[1]{%
  \hbox{\hskip-\leftmarginwidth
	\advance\hsize\leftmarginwidth
	\vbox to\z@{\vss
	      \hbox to\fullwidth{\strut#1}%
              \hrule\@height\headrulewidth}}}

\newcommand{\ps@oz}{%
  \renewcommand{\@oddhead}{%
	\@ozhead{{\rm\rightmark}\hfil{\bf\thepage}}}%
  \renewcommand{\@evenhead}{%
	\@ozhead{{\bf\thepage}\hfil{\sl\leftmark}}}%
  \renewcommand{\@oddfoot}{}%
  \renewcommand{\@evenfoot}{}}

\renewcommand{\sectionmark}[1]{%
  \markright{\ifnum \c@secnumdepth >\z@
    \thesection. \ \fi #1}}

\ps@oz

% MISCELLANEOUS ENVIRONMENTS

% An environment that spans the full width of the page including the
% left margin

\newenvironment{maxipage}
  {\begin{list}{}{\linewidth\textwidth
	\leftmargin-\leftmarginwidth
	\rightmargin\z@}\item[]}
  {\end{list}}

% \begin{explain}{label}...\end{explain}
% has "label" flushed left in the left margin.  If there is enough
% space the body of the environment starts on the same line. You can use
% \\ in the label to obtain multiple lines.
\newcommand{\explain@par}{\egroup\hbox\bgroup\strut\ignorespaces}
\newcommand{\explain@makelabel}[1]{%
  \setbox\@tempboxa\hbox{\ttfamily %\large
	\vbox\bgroup\let\\\explain@par
	  \hbox\bgroup\strut\ignorespaces#1\egroup\egroup}%
  \@tempdima\leftmarginwidth
  \advance\@tempdima-\labelsep
  \ifdim\wd\@tempboxa<\@tempdima
	\box\@tempboxa\hfil
  \else
    \@tempdima\textwidth
    \advance\@tempdima\labelwidth
    \hbox to\@tempdima{\unhbox\@tempboxa\hfil}\fi}
\newenvironment{explain}[1]
  {\trivlist\let\makelabel\explain@makelabel
   \setlength{\labelwidth}{\leftmarginwidth}%
   \addtolength{\labelwidth}{-\labelsep}%
	\item[{#1}]}
  {\endtrivlist}

% \begin{multiexplain}{{label1}{desc1}...{labeln}{descn}}...\end{...}
% is similar to explain but introduces several labels with corresponding
% descriptions.  This is better than using \\ with explain because here
% labels are aligned with their descriptions.

\newcommand{\multiexplain@first}[2]{%
  \def\@tempa{#1}%
  \ifx\@tempa\@nnil \def\@tempa{}%
  \else
    \def\@tempa{\multiexplain@arg{#1}{#2}\multiexplain@next}\fi
  \@tempa}
\newcommand{\multiexplain@next}[2]{%
  \def\@tempa{#1}%
  \ifx\@tempa\@nnil \def\@tempa{}%
  \else
    \def\@tempa{\\\multiexplain@arg{#1}{#2}\multiexplain@next}\fi
  \@tempa}
\newcommand{\multiexplain@arg}[2]{%
  \ignorespaces#1&\ignorespaces#2}
\newlength{\@labelsep}
\newbox\explain@box
\newenvironment{multiexplain}[1]
  {\trivlist
   \setlength{\labelwidth}{\leftmarginwidth}%
% save value of \labelsep and set it to zero
   \setlength{\@labelsep}{\labelsep}%
   \setlength{\labelsep}{0pt}%
   \setbox\explain@box\hbox{\ttfamily
% use saved value of \labelsep
	\advance\labelwidth-\@labelsep
        \begin{tabular}%
	  {@{}p{\labelwidth}@{\hspace{\@labelsep}}p{\linewidth}@{}}%
        \multiexplain@first#1\@nil\@nil
	\end{tabular}}%
   \item[{\box\explain@box}]%
   \mbox{}\vspace*{\topsep}\\%
% restore value of \labelsep
   \setlength{\labelsep}{\@labelsep}%
   \ignorespaces}
  {\endtrivlist}

\newcommand{\ozmetafont}{\ttfamily\slshape\small}
%% Usage: \MV{Name of a meta variable}
\newcommand{\MV}[1]{\mbox{\ozmetafont #1\/}}
%% Usage: \IN{Name of a meta variable} for input variables
\newcommand{\IN}[1]{\mbox{\ozmetafont {+}#1\/}}
%% Usage: \OUT{Name of a meta variable} for output variables
\newcommand{\OUT}[1]{\mbox{\ozmetafont {?}#1\/}}


% ERROR REPORTING

\newcommand{\@ozbookclass}{ozbook}
\newcommand{\@ozerror}[2]{%
  \ClassError{\@ozbookclass}{#1}{#2}}
\newcommand{\@ozwarning}[1]{%
  \ClassWarning{\@ozbookclass}{#1}}

% CROSS REFERENCES

\newcommand{\@ozrefs}{}
\newcommand{\defozref}[2]{%
  \@namedef{ozdoc@#1}{#2}%
  \expandafter\def\expandafter\@ozrefs\expandafter{%
	\@ozrefs\space#1}}
\defozref{survey}{A Survey of Oz}
\defozref{kernel}{The Definition of Kernel Oz}
\defozref{prog}{Programming in Oz}
\defozref{constraints}{Constraint Programming in Oz}
\defozref{demos}{DFKI Oz Demos}
\defozref{crash}{An Oz Primer}
\defozref{tips}{Tips on Oz}
\defozref{tk}{Window Programming in DFKI Oz}
\defozref{open}{Open Programming in DFKI Oz}
\defozref{modules}{The Oz Standard Modules}
\defozref{system}{DFKI Oz User's Manual}
\defozref{fd}{The Constraint Propagator Interface of DFKI Oz}
\defozref{notation}{The Oz Notation}

\newcommand{\@ozreferr}[2]{%
  \@ozerror {Illegal Argument to \string#1: #2}%
	    {Legal Arguments are:^^J\@ozrefs}}

\newcommand{\ozref}[1]{%
  \@ifundefined{ozdoc@#1}{\@ozreferr{\ozref}{#1}}%
	{\cite{ozdoc:#1}}}
\newcommand{\ozrefplain}[1]{%
  \@ifundefined{ozdoc@#1}{\@ozreferr{\ozrefplain}{#1}}%
	{\texttt{\@nameuse{ozdoc@#1}}}}

% COVER PAGE

\newcommand{\titlefontone}{\sffamily\bfseries\LARGE}
\newcommand{\titlefonttwo}{\sffamily\bfseries\Large}
\newcommand{\titlefontthree}{\sffamily\bfseries\large}
\newcommand{\ps@oztitlepage}{%
  \renewcommand{\@oddhead}{%
	\@ozhead{\titlefontone
	  DFKI Oz Documentation Series\hfil}}%
  \renewcommand{\@evenhead}{}%
  \renewcommand{\@oddfoot}{}%
  \renewcommand{\@evenfoot}{}}

\newcommand{\front@one}{%
\if@coverPage
  \begin{titlepage}%
    \thispagestyle{oztitlepage}%
    \hbox{}\vfill
    \begin{maxipage}%
      \noindent
      \hbox to\fullwidth{%
%        \setlength{\epsfxsize}{0.5\fullwidth}%
%        $\vcenter{\epsffile{/home/ps-home/hydra/comics/ps/\@comics}}$%
	$\vcenter{\hbox{\includegraphics[width=0.5\fullwidth]
		{/home/ps-home/hydra/comics/ps/\@comics}}}$%
        \hfil
        \begin{minipage}[c]{0.5\fullwidth}%
  	\raggedleft
  	{\titlefontone\@bursttitle\\[1cm]}%
  	{\titlefonttwo\@burstauthor\\[1cm]}%
        \end{minipage}}%
      \vfill
      \titlefontthree
      \tabcolsep0pt%
      \begin{tabular*}{\fullwidth}%
	{@{}l@{\extracolsep\fill}r@{\extracolsep{0pt}}@{}}%
%	{%
	Programming Systems Lab\\
	German Research Center for Artificial Intelligence (DFKI)\\
        and Universit\"at des Saarlandes\\
        Postfach 15~11~50, D-66041 Saarbr\"ucken, Germany &
	\@date
      \end{tabular*}%
    \end{maxipage}%
  \end{titlepage}%
\fi
}

\newcommand{\ozhttp}{http://www.ps.uni-sb.de/oz}
\newcommand{\ozftp}{ftp://ftp.ps.uni-sb.de/pub/oz}
\if@pdf
  \newcommand{\@ozhref}[1]{\href{#1}{#1}}
\else
  \newcommand{\@ozhref}{}
\fi
\newcommand{\@ozhttp}{\@ozhref{\ozhttp}}
\newcommand{\@ozftp}{\@ozhref{\ozftp}}

\newcommand{\front@two}{%
\if@coverPage
\clearpage
\pagestyle{empty}%
\thispagestyle{empty}%
\begin{maxipage}%
  This document describes the programming language 
  Oz~\@ozversion\ and the programming system
  DFKI~Oz \hbox{(Version \@dfkiozversion)}.

\vfill
  More information about Oz and DFKI Oz
  can be obtained via WWW at
  \[\texttt{\@ozhttp}\]
  and via anonymous ftp at
  \[\texttt{\@ozftp}\]

\vfill
  For specific questions and bug reports, you may mail to
  \texttt{oz@ps.uni-sb.de}.  To join the Oz users mailing list,
  contact \texttt{oz-users-request@ps.uni-sb.de}.

\vfill
  The development of Oz was supported by the 
  Bundesminister f\"ur Bildung, Wissenschaft, Forschung und Technologie
  of the German Government (ITW 9105 and ITW 9601), 
  the Esprit Program of the European Communities
  (ACCLAIM, EP 7195; CCL, EP 6028; CCLII, EP 22457),
  the Deutsche For\-schungsgemeinschaft (SFB 378),
  and the Universit\"at des Saarlandes.
\vfill
  Cover Illustration by Andreas Schoch.

\vfill
  {\sffamily\footnotesize
   The trademarks used in this document are the properties of their
   respective owners.

   This work may not be copied or reproduced in whole or part for any
   commercial purpose. Permission to copy in whole or part without
   payment of fee is granted for nonprofit educational and research
   purposes provided that all such whole or partial copies include the
   following: a notice that such copying is by permission of DFKI,
   Saarbr\"ucken, Germany; an acknowledgement of
   the authors and individual contributors to the work; all applicable
   portions of this copyright notice. Copying, reproducing, or
   republishing for any other purpose shall require a licence with
   payment of fee to DFKI.

   \copyright{} DFKI 1994, 1995, 1996, 1997.}%
\end{maxipage}%
\fi
}

\newcommand{\front@three}{%
\if@titlePage
\clearpage
\thispagestyle{empty}%
\begin{maxipage}%
\vspace*{\fill}%
\begin{center}%
  {\Huge\@title\par}%
  \vspace{1.5em}%
  {\large\@date\par}%
  \vspace{3em}%
  {\large\begin{tabular}[t]{c}\@author\end{tabular}\par}%
\end{center}%
\if@titlePage
\ifx\@abstract\long@empty\else
     \vspace*{\fill}%
     \begin{center}\textbf{Abstract}\end{center}%
     \begin{list}{}{\listparindent 1.5em%
                    \itemindent\z@%\listparindent
                    \rightmargin\leftmargin
                    \parsep\z@\@plus\p@}\item\relax
     \@abstract
     \end{list}%
\fi
\fi
\@thanks
\vspace*{\fill}%
\end{maxipage}%
\fi
}


\newcommand{\front@four}{\clearpage\thispagestyle{empty}}

\newcommand{\front@five}{%
\if@tableofcontents
\cleardoublepage
\pagestyle{empty}%
  \thispagestyle{empty}%
{\let\ps@plain\ps@empty
 \tableofcontents
 \cleardoublepage}%
\fi
}

\AtBeginDocument{\ozfrontmatter}
\newcommand{\ozfrontmatter}{%
\ifx\lthtmlwrite\@undefined
\front@one\front@two\front@three\front@four\front@five
\pagestyle{oz}%
\thispagestyle{oz}%
\setcounter{page}{1}%
\setcounter{footnote}{0}%
\fi
}

\newcommand{\@ozversion}{%
  \@ozwarning{Missing \string\ozversion}??OZ VERSION??}
\newcommand{\ozversion}{\renewcommand{\@ozversion}}
\newcommand{\@dfkiozversion}{%
  \@ozwarning{Missing \string\ozversion}??DFKI OZ VERSION??}
\newcommand{\dfkiozversion}{\renewcommand{\@dfkiozversion}}
\newcommand{\@dfkiozreleasedate}{%
  \@ozwarning{Missing \string\dfkiozreleasedate}??OZ RELEASE DATE??}
\newcommand{\dfkiozreleasedate}{\renewcommand{\@dfkiozreleasedate}}
\renewcommand{\@author}{%
  \@ozauthor\@ozaddress}
\newcommand{\@ozauthor}{%
  \@ozwarning{Missing \string\author}??AUTHOR??}
\renewcommand{\author}{\gdef\@ozauthor}
\newcommand{\@ozaddress}{%
  \\[2mm]
  Programming Systems Lab\\
  German Research Center for Artificial Intelligence (DFKI)\\
  and Universit\"at des Saarlandes\\
  Postfach 15~11~50\\
  D-66041 Saarbr\"ucken, Germany\\
  \texttt{\@email{}@ps.uni-sb.de}}

\newcommand{\@bursttitle}{%
  \@ozwarning{Missing \string\bursttitle}}
\newcommand{\bursttitle}{\renewcommand{\@bursttitle}}

\newcommand{\@burstauthor}{%
  \@ozwarning{Missing \string\burstauthor}}
\newcommand{\burstauthor}{\renewcommand{\@burstauthor}}

\newcommand{\@email}{%
  \@ozwarning{Missing \string\email}{??EMAIL??}}
\newcommand{\email}{\renewcommand{\@email}}

\newcommand{\@reportnumber}{%
  \@ozwarning{Missing \string\reportnumber}}
\newcommand{\reportnumber}{\renewcommand{\@reportnumber}}

\newcommand{\@releasenumber}{}
\newcommand{\releasenumber}{\renewcommand{\@releasenumber}}

\newcommand{\@comics}{none}
\newcommand{\comics}{\renewcommand{\@comics}}

\newcommand{\@abstract}{}%
\newcommand{\abstract}{\renewcommand{\@abstract}}%

% BACK MATTER

\newcommand{\long@empty}{}
\AtEndDocument{%
  \ifx\@ozauthor\long@empty\else %\@input{../cover.tex}
\fi}

% INDEX

\renewenvironment{theindex}
 {\vspace{30pt}%
  \setlength{\hfuzz}{20pt}%
  \setlength{\columnseprule}{0pt}%
  \setlength{\columnsep}{35pt}%
  \begin{multicols}{2}\raggedright
    \setlength{\parindent}{0pt}%
    \setlength{\parskip}{0pt plus .3pt}%
    \let\item\@idxitem}%
 {\end{multicols}\clearpage}

% The index is divided into units, one for each initial letter.
% \indexletter begins such a section and places a title across
% the column: it displays the letter in question inside an oval
% box.

\newcommand{\indexletter}[1]{%
  \pagebreak[2]%
  \psframebox[framearc=0.5,linewidth=0.4pt]%
	{\makebox[\linewidth]{\Large\textbf{#1}}}%
  \markboth{INDEX}{#1}}

\newcommand{\doindex}{\clearpage\printindex}

\newcommand{\getindexintro}{%
\chapter*{Index}%
\addcontentsline{toc}{chapter}{Index}%
This Index contains different kinds of items. It consists of concept
entries (they are displayed in the usual font), and of references to
technical items. They are displayed in the respective font used
throughout the text. 
Thus, for instance, you may find a reference ``foo'' as well as a
reference ``{\ozmetafont Foo}''.\par
If you want to look up a method or a field of a module, you will
be directed to the object or the module where it belongs to. 
Page numbers are printed in \textbf{boldface}, whenever the item
referenced is defined on this page.}

\newindex{default}{idx}{ind}{Index%
   \protect\addcontentsline{toc}{chapter}{Index}}
% \proofmodetrue
\indexproofstyle{\sc}
\newcommand{\id}{\textbf}

% redefine \see so that it looks like a subitem inside an item, like
% a subsubitem inside a subitem, etc... This is done by letting the
% various levels of items redefine \@see to be the next lower level.

\renewcommand{\see}[2]{{\@see{$\rightarrow$~#1}}}
\let\old@idxitem\@idxitem
\let\old@subitem\subitem
\let\old@subsubitem\subsubitem
\newcommand{\@subsubsubitem}{\par\hangindent 40\p@ \hspace*{40\p@}}
\renewcommand{\@idxitem}{\let\@see\old@subitem\old@idxitem}
\renewcommand{\subitem}{\let\@see\old@subsubitem\old@subitem}
\renewcommand{\subsubitem}{\let\@see\@subsubsubitem\old@subsubitem}

% OZHYPERREF STUFF MUST COME AFTER OZBOOK'S OWN DEFS
\if@pdf
\RequirePackage{ozhyperref}
\fi

% REFERENCES

\newcommand{\bibliographies}{%
  ../references,/usr/share/gs/soft/query/data/bibtex-here/fbps}
\newcommand{\doreferences}{%
  \clearpage
  \addcontentsline{toc}{chapter}{\bibname}%
  \chaptermark{\MakeUppercase\bibname}%
  \let\em\sl
  \bibliographystyle{plain}%
  \bibliography{\bibliographies}}

% FLOATS

\renewcommand{\floatpagefraction}{.7}
\floatstyle{plain}

% floats should span the full width of the page: we redefine the
% following command to format a float in a box that spans the full
% width of the page and to move it left by the width of the left
% margin.

\renewcommand\float@makebox{%
  \vbox{\moveleft\leftmarginwidth
  \vbox{\hsize\fullwidth \@parboxrestore
    \@fs@pre\@fs@iftopcapt
      \ifvbox\@floatcapt\unvbox\@floatcapt\par\@fs@mid\fi
      \unvbox\@currbox
    \else\unvbox\@currbox
      \ifvbox\@floatcapt\par\@fs@mid\unvbox\@floatcapt\fi
    \fi\par\@fs@post\vskip\z@}}}

% this seems to do the trick.  otherwise we get loads of overfull warnings.
\renewcommand{\@floatboxreset}{%
  \hsize\fullwidth\columnwidth\hsize
  \reset@font
  \normalsize
  \@setnobreak
  \@setminipage
}

\newcommand{\@maxioption}[1]{%
  \@ifnextchar[{\@@maxioption{#1}}{\@@@maxioption{#1}}}
\def\@@maxioption#1[#2]{\begin{#1}[#2]\begin{minipage}{\fullwidth}}
\def\@@@maxioption#1{\begin{#1}\begin{minipage}{\fullwidth}}

\newfloat{@maxiprogram}{tbp}{lop}[chapter]
\floatname{@maxiprogram}{Program}
\newfloat{@maxifigure}{tbp}{lof}[chapter]
\floatname{@maxifigure}{Figure}
\newenvironment{maxifigure}
  {\@maxioption{@maxifigure}}
%  {\begin{@maxifigure}\begin{minipage}{\fullwidth}}
  {\end{minipage}\end{@maxifigure}}
\newenvironment{maxiprogram}
  {\@maxioption{@maxiprogram}}
%  {\begin{@maxiprogram}\begin{minipage}{\fullwidth}}
  {\end{minipage}\end{@maxiprogram}}
% we want the corresponding lists to be entered into the table of contents
\newcommand{\@chapter@acl}[1]{%
  \if@openright\cleardoublepage\else\clearpage\fi
  \addcontentsline{toc}{chapter}{#1}}
\newcommand{\dolistoffigures}{%
  \@chapter@acl{List of Figures}%
  \listof{@maxifigure}{List of Figures}}
\def\dolistofprograms{%
  \@chapter@acl{List of Oz Programs}%
  \listof{@maxiprogram}{List of Oz Programs}}
\let\dolistofozprogs\dolistofprograms

\newfloat{@maxitable}{tbp}{lot}[chapter]
\floatname{@maxitable}{Table}
\newenvironment{maxitable}
  {\@maxioption{@maxitable}}
  {\end{minipage}\end{@maxitable}}

% redefine the following to cause captions to be flushed left

\renewcommand\floatc@plain[2]{\setbox\@tempboxa\hbox{{\@fs@cfont #1:} #2}%
  \ifdim\wd\@tempboxa>\hsize {\@fs@cfont #1:} #2\par
    \else\hbox to\hsize{\box\@tempboxa\hfil}\fi}

% EXERCISES AND SOLUTIONS
%
% we modify theorem-like environments (i.e. defined using \newtheorem)
% so that they display their caption/label in the left margin.
% Furthermore, you can also refine a theorem-like environment foo by
% specifying code to be executed at the beginning and/or at the end:
% just supply definitions for \fooBegin and/or \fooEnd (see example
% below)


\renewcommand{\@begintheorem}[2]{%
  \trivlist
  \setlength{\labelwidth}{\leftmarginwidth}%
  \item[\hskip \labelsep{\bfseries #1\ #2}\hfil]%
  \@ifundefined{\@currenvir Begin}%
	{\itshape}%
	{\@nameuse{\@currenvir Begin}}}
\renewcommand{\@endtheorem}{%
  \@ifundefined{\@currenvir End}%
	{}%
	{\@nameuse{\@currenvir End}}%
  \endtrivlist}

% example is already defined in math.sty
\let\example\undefined
\newtheorem{example}{Example}[section]
\newcommand{\exampleBegin}{\rm}
\newcommand{\exampleEnd}{%
  \leavevmode\hbox{}\nobreak\hfill\nobreak$\Box$}

\newtheorem{exercise}{Exercise}[section]
\newcommand{\exerciseBegin}{\rm}

% The solution environment writes its body to file \jobname.sol
% its implementation is based on that of \comment in Rainer Schoepf's
% verbatim package.

\newwrite\solution@out
\immediate\openout\solution@out\jobname.sol
\newenvironment{solution}
  {\@bsphack
   \let\do\@makeother\dospecials\catcode`\^^M\active
   \immediate\write\solution@out{%
	\noexpand\begin{showsolution}{\theexercise}}
   \def\verbatim@processline{%
	\immediate\write\solution@out{\the\verbatim@line}}%
   \verbatim@start}
  {\immediate\write\solution@out{%
	\noexpand\end{showsolution}}%
   \@esphack}

\newenvironment{showsolution}[1]
  {\setlength{\topsep}{12pt plus 12pt}\trivlist
   \setlength{\labelwidth}{\leftmarginwidth}%
   \item[\hskip\labelsep{\bfseries Exercise~#1}\hfil]}
  {\endtrivlist}

\newcommand{\makesolutions}{%
  \immediate\closeout\solution@out
  \@input{\jobname.sol}}

% MISC FONTS &STUFF

\let\ozmenufont\sf
\font\manual=manfnt
\def\dbend{{\manual\char127}}
\newenvironment{danger}{\Marginpar{\dbend}}{}
\newenvironment{Danger}{\Marginpar{\dbend\dbend}}{}
% I am not sure why there is an \mbox{} here.  It's probably
% the wrong hack.
\newcommand{\Marginpar}{\mbox{}\marginpar}
\@mparswitchfalse
\reversemarginpar
\let\old@marginpar\marginpar
\renewcommand{\marginpar}[1]{\old@marginpar{\raggedright\small\it#1}}

% \hsmash{X} centers X and gives it no width
% this is useful for very long subscripts, etc...

\newcommand{\hsmash}{\relax
  \ifmmode\expandafter\mathpalette\expandafter\mathHSMASH
  \else\expandafter\makeHSMASH\fi}
\newcommand{\mathHSMASH}[2]{%
  \hbox to0pt{\hss$\m@th#1{#2}$\hss}}
\newcommand{\makeHSMASH}[1]{%
  \hbox to0pt{\hss\color@begingroup#1\color@endgroup\hss}}

\newcommand{\HTMLNOLINEBREAK}{}
