<!-- -*-xml-*- -->

<stylesheet
  xmlns ="http://www.w3.org/XSL/Transform/1.0"
  xmlns:id      ="http://www.jclark.com/xt/java/ID"
  xmlns:msg     ="http://www.jclark.com/xt/java/Msg"
  xmlns:meta    ="http://www.jclark.com/xt/java/Meta"
  xmlns:tex ="java:com.jclark.xsl.sax.LaTeXOutputHandler"
  result-ns ="tex">

<!-- for elements that only contain other elements but never data,
     all spaces should be stripped away -->

<strip-space elements="
        BOOK FRONT BACK BODY
        PART CHAPTER SECTION SUBSECTION SUBSUBSECTION APPENDIX
        LIST ENTRY SYNOPSIS
        MATH.CHOICE PICTURE.CHOICE
        CHUNK FIGURE INDEX SEE
        GRAMMAR.RULE GRAMMAR
        TABLE TR"/>

<!-- root processing -->

<template match="/">
  <call-template name="build.tables"/>
  <apply-templates/>
</template>

<!-- build tables -->

<template name="build.tables">
  <call-template name="build.table.id"/>
  <call-template name="build.table.tabspec"/>
</template>

<!-- build table mapping id to node -->

<template name="build.table.id">
  <if test="@ID and id:put((string(@ID)),.)"/>
  <for-each select="*">
    <call-template name="build.table.id"/>
  </for-each>
</template>

<!-- build table mapping id to latex table spec -->

<template name="build.table.tabspec">
  <for-each select="/BOOK/FRONT/META[@NAME='LATEX.TABLE.SPEC']">
    <if test="meta:latexTableSpecPut((string(@ARG1)),(string(@ARG2)))"/>
  </for-each>
</template>

<!-- book -->

<template match="BOOK">
  <tex:document>
  <tex:code>\documentclass{ozdoc}
</tex:code>
  <apply-templates select="FRONT"/>
  <tex:code>\begin{document}
</tex:code>
  <apply-templates select="BODY"/>
  <apply-templates select="BACK"/>
  <tex:code>\end{document}
</tex:code>
  </tex:document>
</template>

<template match="BOOK/FRONT">
  <for-each select="META[@NAME='LATEX.PACKAGE']">
    <tex:code>\usepackage{<value-of select="@VALUE"/>}
</tex:code>
  </for-each>
  <for-each select="META[@NAME='LATEX.INPUT']">
    <tex:code>\input{<value-of select="@VALUE"/>}
</tex:code>
  </for-each>
  <tex:code>\title{</tex:code>
  <apply-templates select="TITLE" mode="ok"/>
  <tex:code>}
\author{</tex:code>
  <apply-templates select="AUTHOR|AUTHOR.EXTERN"/>
  <tex:code>}
</tex:code>
</template>

<!-- ignore FRONT otherwise -->
<template match="FRONT"/>

<!-- for some elements: just recurse through -->

<template match="
        TITLE | BODY | ITEM/P.SILENT | ENTRY/P.SILENT |
        SYNOPSIS/P.SILENT | TD/P.SILENT | TH/P.SILENT |
        DEF | SPAN | FIGURE/CAPTION/P.SILENT | GRAMMAR.HEAD |
        CHUNK/TITLE | CHUNK/TITLE/P.SILENT | CHUNK.SILENT |
        NAME | NOTE/P.SILENT">
  <apply-templates/>
</template>

<!-- for some elements: just ignore them -->

<template match="INDEX">
  <if test="msg:say('IGNORING ') and msg:saynl((local-part(.)))"/>
</template>

<!-- default node template: output an error message -->

<template match="*" priority="-1.0">
  <if test="msg:say('UNMATCHED ELEMENT: ') and msg:saynl((local-part(.)))"/>
  <tex:code>\UNMATCHED{<value-of select="local-part(.)"/>}</tex:code>
</template>

<!-- processing instructions -->

<template match="processing-instruction()">
  <tex:code>\usepi{<value-of select="local-part(.)"/>}</tex:code>
</template>

<!-- format author. for the time being, we don't actually
     consult the author database but just use the key
     a key is for example Denys.Duchier and we just replace
     the period by a space -->

<template match="AUTHOR.EXTERN">
  <if test="not(position()=1)"><tex:code>\\
</tex:code></if>
  <value-of select="translate((string(@KEY)),'.',' ')"/>
</template>

<template match="AUTHOR">
  <if test="not(position()=1)"><tex:code>\\
</tex:code></if>
  <apply-templates/>
</template>

<!-- format a section-like element.
     the preamble defines a sectionning command with the same uppercase
     name as the element. we also add a \label command if the element
     has an id -->

<template match="PART|CHAPTER|APPENDIX|SECTION|SUBSECTION|SUBSUBSECTION|PARA">
  <tex:eoln/>
  <tex:eoln/>
  <tex:code>\<value-of select="local-part(.)"/>{</tex:code>
  <apply-templates select="TITLE | FRONT/TITLE" mode="ok"/>
  <tex:code>}</tex:code>
  <if test="@ID">
    <tex:code>\label{<value-of select="@ID"/>}</tex:code>
  </if>
  <tex:eoln/>
  <tex:eoln/>
  <apply-templates/>
</template>

<template match="TITLE"/>
<template match="TITLE" mode="ok">
  <apply-templates/>
</template>

<!-- format a paragraph -->

<template match="P">
  <tex:eoln/>
  <tex:eoln/>
  <apply-templates/>
</template>

<!-- LIST -->
<!-- 1. simple enumerated list -->

<template match="LIST[@ENUM='ENUM' and not(ENTRY)]">
  <tex:code>\begin{enumerate}</tex:code>
  <for-each select="ITEM">
    <tex:eoln/>
    <tex:code>\item{}</tex:code>
    <apply-templates/>
  </for-each>
  <tex:eoln/>
  <tex:code>\end{enumerate}</tex:code>
</template>

<!-- 2. simple itemized list -->

<template match="LIST[not(@ENUM) and not(ENTRY)]">
  <tex:code>\begin{itemize}</tex:code>
  <for-each select="ITEM">
    <tex:eoln/>
    <tex:code>\item{}</tex:code>
    <apply-templates/>
  </for-each>
  <tex:eoln/>
  <tex:code>\end{itemize}</tex:code>
</template>

<!-- 3. enumerated description -->

<template match="LIST[@ENUM='ENUM' and ENTRY]">
  <tex:code>\begin{enumerate}</tex:code>
  <apply-templates mode="list.enum.entry"/>
  <tex:code>\end{enumerate}</tex:code>
</template>

<template mode="list.enum.entry" match="ITEM">
  <apply-templates/>
</template>

<template mode="list.enum.entry" match="ENTRY">
  <tex:code>\item{}</tex:code>
  <apply-templates/>
</template>

<!-- 4. itemized description -->

<template match="LIST[not(@ENUM) and ENTRY]">
  <tex:code>\begin{DESCRIPTION}
</tex:code>
  <apply-templates mode="list.desc"/>
  <tex:code>\end{DESCRIPTION}
</tex:code>
</template>

<!--
<template mode="list.desc" match="ENTRY">
  <tex:code>\item[{</tex:code>
  <apply-templates/>
  <tex:code>}]</tex:code>
  <if test="@ID">
    <tex:code>\label{<value-of select="@ID"/>}</tex:code>
  </if>
</template>
-->
<template mode="list.desc" match="ENTRY">
  <tex:code>\ENTRY{</tex:code>
  <apply-templates/>
  <tex:code>}</tex:code>
  <if test="@ID">
    <tex:code>\label{<value-of select="@ID"/>}</tex:code>
  </if>
  <if test=".//CODE">
    <tex:code>\ENTRYHASCODE</tex:code>
  </if>
  <tex:eoln/>
</template>

<template mode="list.desc" match="ITEM">
  <tex:code>\ITEM </tex:code>
  <apply-templates/>
</template>

<template mode="list.desc" match="SYNOPSIS">
  <tex:code>\begin{SYNOPSIS}
</tex:code>
  <apply-templates/>
  <tex:code>\end{SYNOPSIS}
</tex:code>
</template>

<!-- format code -->

<template match="CODE[@DISPLAY='INLINE']">
  <tex:code>\CODEINLINE{</tex:code>
  <apply-templates/>
  <tex:code>}</tex:code>
</template>

<template match="CODE.EXTERN[@DISPLAY='DISPLAY']">
  <tex:code>\CODEEXTERN{<value-of select="@TO"/>}</tex:code>
</template>

<template match="CODE[@DISPLAY='DISPLAY']">
  <tex:code>\begin{CODEDISPLAY}</tex:code>
  <apply-templates/>
  <tex:code>\end{CODEDISPLAY}
</tex:code>
</template>

<template match="HILITE">
  <apply-templates/>
</template>

<template match="HILITE.FACE">
  <tex:code>\FACE<value-of select="@NAME"/>{</tex:code>
  <apply-templates/>
  <tex:code>}</tex:code>
</template>

<!-- format variables -->

<template match="VAR">
  <variable name="type"><value-of select="@TYPE"/></variable>
  <if test="$type='PROG' and @MODE">
    <tex:code>\MODE{<value-of select="@MODE"/>}</tex:code>
  </if>
  <tex:code>\<value-of select="$type"/>VAR{</tex:code>
  <apply-templates/>
  <tex:code>}</tex:code>
</template>

<!-- format code text: preserve newlines and escape weird chars -->

<template match="CODE/text()|CHUNK.SILENT/text()">
  <tex:verb><value-of select="."/></tex:verb>
</template>

<!-- miscellaneous commands -->

<template match="FILE | SAMP | EM | KBD | KEY">
  <tex:code>\<value-of select="local-part(.)"/>{</tex:code>
  <apply-templates/>
  <tex:code>}</tex:code>
</template>

<template match="Q">`<apply-templates/>'</template>

<!-- references -->

<template match="REF.EXTERN">
  <apply-templates/>
  <tex:code>\footnote{</tex:code>
  <call-template name="ref.extern" select="."/>
  <tex:code>}</tex:code>
</template>

<template match="PTR.EXTERN">
  <call-template name="ref.extern" select="."/>
</template>

<template name="ref.extern">
  <variable name="to"><value-of select="@TO"/></variable>
  <choose>
  <when test="starts-with($to,'ozdoc:')">
    <tex:code>\REFEXTTO{</tex:code>
    <choose>
      <when test="$to='ozdoc:system'"
            >The System Modules</when>
      <when test="$to='ozdoc:apptut'"
            >The Application Programming Tutorial</when>
      <when test="$to='ozdoc:opi'"
            >The Oz Programming Interface</when>
      <when test="$to='ozdoc:fdt'"
            >Finite Domain Constraint Programming</when>
      <when test="$to='ozdoc:notation'"
            >The Oz Notation</when>
      <when test="$to='ozdoc:base'"
            >The Oz Base Environment</when>
      <when test="$to='ozdoc:browser'"
            >The Oz Browser</when>
      <when test="$to='ozdoc:explorer'"
            >Oz Explorer - Visual Constraint Programming Support</when>
      <when test="$to='ozdoc:panel'"
            >Oz Panel</when>
      <when test="$to='ozdoc:ozcar'"
            >The Mozart Debugger</when>
      <when test="$to='ozdoc:profiler'"
            >The Mozart Profiler</when>
      <when test="$to='ozdoc:dstutorial'"
            >Distributed Programming in Mozart -
A Tutorial Introduction</when>
      <when test="$to='ozdoc:install'"
            >Installation Manual</when>
      <when test="$to='ozdoc:tools'"
            >Oz Shell Utilities</when>
      <when test="$to='ozdoc:foreign'"
            >Interfacing to C and C++</when>
      <when test="$to='ozdoc:cpiref'"
            >The Mozart Constraint Extensions Reference</when>
      <when test="$to='ozdoc:wp'"
            >Window Programming in Mozart</when>
      <when test="$to='ozdoc:compiler'"
            >The Mozart Compiler</when>
      <when test="$to='ozdoc:tutorial'"
            >The Tutorial of Oz</when>
      <when test="$to='ozdoc:op'"
            >Open Programming in Mozart</when>
      <otherwise>
        <if test="msg:say('UNRECOGNIZED OZDOC REF: ') and msg:saynl(string($to))"/>
        <tex:text>!!!UNRECOGNIZED REF!!!<value-of select="$to"/>!!!</tex:text>
      </otherwise>
    </choose>
    <tex:code>}</tex:code>
    <if test="@KEY">
      <tex:code> \REFEXTKEY{</tex:code>
      <value-of select="@KEY"/>
      <tex:code>}</tex:code>
    </if>
  </when>
  <otherwise>
    <tex:code>\DEFAULTREFEXT{</tex:code>
    <value-of select="$to"/>
    <tex:code>}</tex:code>
  </otherwise>
  </choose>
</template>

<template match="PTR">
  <apply-templates mode="ptr.ref" select="id:get((string(@TO)))"/>
</template>

<template match="REF">
  <apply-templates/>
  <apply-templates mode="ref.ref" select="id:get((string(@TO)))"/>
</template>

<template mode="ptr.ref" match="CHAPTER">
  <tex:text>Chapter</tex:text>
  <call-template name="ref.to.id"/>
</template>

<template mode="ptr.ref" match="PART">
  <tex:text>Part</tex:text>
  <call-template name="ref.to.id"/>
</template>

<template mode="ptr.ref" match="APPENDIX">
  <tex:text>Appendix</tex:text>
  <call-template name="ref.to.id"/>
</template>

<template mode="ptr.ref" match="SECTION | SUBSECTION | SUBSUBSECTION">
  <tex:text>Section</tex:text>
  <call-template name="ref.to.id"/>
</template>

<template mode="ptr.ref" match="PARA|ENTRY">
  <call-template name="pageref.to.id"/>
</template>

<template mode="ptr.ref" match="FIGURE">
  <tex:text>Figure</tex:text>
  <call-template name="ref.to.id"/>
</template>

<template mode="ptr.ref" match="BIB.EXTERN">
  <tex:code>\cite{</tex:code>
  <value-of select="@KEY"/>
  <tex:code>}</tex:code>
</template>

<template mode="ptr.ref" match="*" priority="-1.0">
  <if test="msg:say('UNEXPECTED PTR TO ') and
            msg:say(local-part(.)) and
            msg:say(': ') and
            msg:saynl((string(@ID)))"/>
  <text>!!!UNEXPECTED PTR TO </text>
  <value-of select="@ID"/>
  <text>!!!</text>
</template>

<template name="ref.to.id">
  <tex:code>~\ref{</tex:code>
  <value-of select="@ID"/>
  <tex:code>}</tex:code>
</template>

<template mode="ref.ref" match="BIB.EXTERN">
  <tex:code>\cite{</tex:code>
  <value-of select="@KEY"/>
  <tex:code>}</tex:code>
</template>

<template mode="ref.ref" match="*">
  <call-template name="pageref.to.id"/>
</template>

<template name="pageref.to.id">
  <tex:code> (page~\pageref{</tex:code>
  <value-of select="@ID"/>
  <tex:code>})</tex:code>
</template>

<!-- format tables -->

<template match="TABLE">
  <call-template name="maybe.display.begin"/>
  <tex:code>\begin{tabular}{</tex:code>
  <choose>
    <when test="@ID and meta:latexTableSpecExists((string(@ID)))">
      <tex:code>
        <value-of select="meta:latexTableSpecGet((string(@ID)))"/>
      </tex:code>
    </when>
    <otherwise>
      <for-each select="TR[position()=1]/*">l</for-each>
    </otherwise>
  </choose>
  <tex:code>}</tex:code>
  <apply-templates/>
  <tex:code>\end{tabular}</tex:code>
  <call-template name="maybe.display.end"/>
</template>

<template name="maybe.display.begin">
  <if test="@DISPLAY='DISPLAY'">
    <tex:code>\begin{center}</tex:code>
  </if>
</template>

<template name="maybe.display.end">
  <if test="@DISPLAY='DISPLAY'">
    <tex:code>\end{center}</tex:code>
  </if>
</template>

<template match="TR">
  <apply-templates/>
  <if test="not(position()=last())">
    <tex:code>\\</tex:code>
  </if>
</template>

<template match="TD | TH">
  <if test="not(position()=1)">
    <tex:code>&amp;</tex:code>
  </if>
  <if test="@COLSPAN">
    <tex:code>\multicolumn{<value-of select="@COLSPAN"/>}{l}{</tex:code>
  </if>
  <if test="local-part(.)='TH'">
    <tex:code>\bf{}</tex:code>
  </if>
  <apply-templates/>
  <if test="@COLSPAN">
    <tex:code>}</tex:code>
  </if>
</template>

<!-- figures -->

<template match="FIGURE">
  <tex:code>\begin{figure}</tex:code>
  <apply-templates/>
  <if test="@ID and not(CAPTION)">
    <tex:code>\CAPTIONID{</tex:code>
    <value-of select="@ID"/>
    <tex:code>}</tex:code>
  </if>
  <tex:code>\end{figure}</tex:code>
</template>

<template match="FIGURE/TITLE">
  <tex:code>\begin{FIGTITLE}</tex:code>
  <apply-templates/>
  <tex:code>\end{FIGTITLE}</tex:code>
</template>

<template match="FIGURE/CAPTION">
  <tex:code>\caption{</tex:code>
  <if test="../@ID">
    <tex:code>\label{</tex:code>
    <value-of select="../@ID"/>
    <tex:code>}</tex:code>
  </if>
  <apply-templates/>
  <tex:code>}</tex:code>
</template>

<!-- pictures -->

<template match="PICTURE.CHOICE">
  <call-template name="maybe.display.begin"/>
  <choose>
    <when test="PICTURE[@TYPE='LATEX']">
      <tex:code><value-of select="PICTURE[@TYPE='LATEX']"/></tex:code>
    </when>
    <when test="PICTURE.EXTERN[@TYPE='PS']">
      <tex:code>\PICEXT{</tex:code>
      <value-of select="PICTURE.EXTERN[@TYPE='PS']/@TO"/>
      <tex:code>}</tex:code>
    </when>
    <when test="PICTURE.EXTERN[@TYPE='GIF']">
      <!-- for foo.gif, we will create foo.gif.ps -->
      <tex:code>\PICEXT{</tex:code>
      <value-of select="PICTURE.EXTERN[@TYPE='GIF']/@TO"/>
      <tex:code>.ps}</tex:code>
    </when>
    <otherwise>
      <if test="msg:saynl('UNMATCHED PICTURE')"/>
      <text>!!!UNMATCHED PICTURE!!!</text>
    </otherwise>
  </choose>
  <call-template name="maybe.display.end"/>
</template>

<template match="PICTURE[@TYPE='LATEX']">
  <tex:code><value-of select="."/></tex:code>
</template>

<template match="PICTURE.EXTERN[@DISPLAY='DISPLAY']">
  <variable name="type"><value-of select="@TYPE"/></variable>
  <variable name="to"><value-of select="@TO"/></variable>
  <tex:code>\begin{PICEXTDISPLAY}</tex:code>
  <choose>
    <when test="$type='LATEX'">
      <tex:code>\input{</tex:code>
      <value-of select="$to"/>
      <tex:code>}</tex:code>
    </when>
    <when test="$type='GIF'">
      <tex:code>\PICEXTFULL{</tex:code>
      <value-of select="$to"/>
      <tex:code>}</tex:code>
    </when>
    <otherwise>
      <if test="msg:say('UNMATCHED PICTURE.EXTERN: ') and msg:saynl($to)"/>
      <text>!!!UNMATCHED PICTURE.EXTERN!!!</text>
      <value-of select="$to"/>
      <text>!!!</text>
    </otherwise>
  </choose>
  <tex:code>\end{PICEXTDISPLAY}</tex:code>
</template>

<!-- grammar thingies -->

<template match="GRAMMAR.RULE | GRAMMAR">
  <call-template name="maybe.display.begin"/>
  <tex:code>\begin{tabular}{lrll}
</tex:code>
  <apply-templates/>
  <tex:code>\end{tabular}
</tex:code>
  <call-template name="maybe.display.end"/>
</template>

<template match="GRAMMAR/GRAMMAR.RULE" priority="2.0">
  <if test="position()>1"><tex:code>\\
</tex:code></if>
  <apply-templates/>
</template>

<template match="GRAMMAR.ALT">
  <tex:code>&amp;</tex:code>
  <tex:code>
    <choose>
      <when test="not(@TYPE) and position()=2 or @TYPE='DEF'">\GRAMDEF</when>
      <when test="@TYPE='ADD'">\GRAMADD</when>
      <when test="not(@TYPE) and position()>2 or @TYPE='OR'">\GRAMOR</when>
      <when test="@TYPE='SPACE'">\GRAMSPACE</when>
    </choose>
  </tex:code>
  <tex:code>&amp;</tex:code>
  <apply-templates/>
  <if test="not(child::GRAMMAR.NOTE)">
    <tex:code>&amp;</tex:code>
  </if>
  <if test="not(position()=last())"><tex:code>\\
</tex:code></if>
</template>

<template match="GRAMMAR.NOTE">
  <tex:code>&amp;\GRAMMARNOTE{</tex:code>
  <apply-templates/>
  <tex:code>}</tex:code>
</template>

<!-- math -->

<template match="MATH[@TYPE='LATEX']">
  <tex:code>
    <choose>
      <when test="parent::MATH.CHOICE[@DISPLAY='DISPLAY']">\[</when>
      <when test="@DISPLAY='DISPLAY'">\[</when>
      <otherwise>\(</otherwise>
    </choose>
    <value-of select="."/>
    <choose>
      <when test="parent::MATH.CHOICE[@DISPLAY='DISPLAY']">\]</when>
      <when test="@DISPLAY='DISPLAY'">\]</when>
      <otherwise>\)</otherwise>
    </choose>
  </tex:code>
</template>

<template match="MATH.CHOICE[MATH[@TYPE='LATEX']]">
  <apply-templates select="MATH[@TYPE='LATEX']"/>
</template>

<!-- literate programming: chunks -->

<template match="CHUNK">
  <tex:code>\begin{CHUNK}{</tex:code>
  <apply-templates select="TITLE"/>
  <tex:code>}</tex:code>
  <apply-templates select="CHUNK.SILENT"/>
  <tex:code>\end{CHUNK}</tex:code>
</template>

<template match="CHUNK.REF">
  <tex:code>\CHUNKREF{</tex:code>
  <apply-templates/>
  <tex:code>}</tex:code>
</template>

<!-- notes and footnotes -->

<template match="NOTE[@FOOT='FOOT']" priority="2.0">
  <tex:code>\footnote{</tex:code>
  <apply-templates/>
  <tex:code>}</tex:code>
</template>

<template match="NOTE">
  <tex:code>\paragraph{Note:}</tex:code>
  <apply-templates/>
</template>

<!-- back matter -->

<template match="BACK">
  <for-each select="BIB.EXTERN[position()=1]">
    <tex:code>
\bibliographystyle{plain}
\bibliography{</tex:code>
    <value-of select="substring-before((string(@TO)),'.bib')"/>
    <tex:code>}</tex:code>
  </for-each>
</template>

</stylesheet>
