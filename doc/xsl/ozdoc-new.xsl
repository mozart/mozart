<!-- -*-xml-*- -->

<stylesheet
  xmlns ="http://www.w3.org/XSL/Transform/1.0"
  xmlns:id      ="http://www.jclark.com/xt/java/ID"
  xmlns:msg     ="http://www.jclark.com/xt/java/Msg"
  xmlns:meta    ="http://www.jclark.com/xt/java/Meta"
  xmlns:txt     ="java:com.jclark.xsl.sax.TextOutputHandler"
  result-ns     ="txt">

<!-- for elements that only contain other elements but never data,
     all spaces should be stripped away -->

<strip-space elements="
        BOOK FRONT BACK BODY
        PART CHAPTER SECTION SUBSECTION SUBSUBSECTION APPENDIX
        LIST ENTRY SYNOPSIS
        MATH.CHOICE PICTURE.CHOICE
        CHUNK FIGURE INDEX SEE
        GRAMMAR.RULE GRAMMAR
        TABLE TR
        OZDOC.DB OZDOC.DOCUMENT OZDOC.BOOK OZDOC.ENTRY"/>

<!-- root processing -->

<template match="/">
  <call-template name="build.tables"/>
  <txt:document>
    <call-template name="declare.maps"/>
    <txt:usemap name="text">
      <apply-templates/>
    </txt:usemap>
  </txt:document>
</template>

<!-- build tables -->

<template name="build.tables">
  <if test="msg:saynl('BUILDING TABLES...')"/>
  <if test="msg:saynl('BUILDING TABLE: ID')"/>
  <call-template name="build.table.id"/>
  <call-template name="build.table.tabspec"/>
  <call-template name="build.table.picwid"/>
  <call-template name="build.table.category"/>
  <call-template name="build.table.fullwidth"/>
  <if test="msg:saynl('BUILDING TABLES...DONE')"/>
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
  <if test="msg:saynl('BUILDING TABLE: LATEX.TABLE.SPEC')"/>
  <for-each select="/BOOK/FRONT/META[@NAME='LATEX.TABLE.SPEC']">
    <if test="meta:latexTableSpecPut((string(@ARG1)),(string(@ARG2)))"/>
  </for-each>
</template>

<!-- build table mapping id to requested picture width -->

<template name="build.table.picwid">
  <if test="msg:saynl('BUILDING TABLE: LATEX.PICTURE.WIDTH')"/>
  <for-each select="/BOOK/FRONT/META[@NAME='LATEX.PICTURE.WIDTH']">
    <if test="meta:pictureWidthPut(string(@ARG1),string(@ARG2))"/>
  </for-each>
</template>

<!-- build table mapping entry categories to names -->

<template name="build.table.category">
  <if test="msg:saynl('BUILDING TABLE: ENTRY.CATEGORY')"/>
  <for-each select="/BOOK/FRONT/META[@NAME='ENTRY.CATEGORY']">
    <choose>
      <when test="@VALUE">
        <if test="meta:entryCategoryPut(string(@VALUE),string(@VALUE))"/>
      </when>
      <when test="@ARG1 and @ARG2">
        <if test="meta:entryCategoryPut(string(@ARG1),string(@ARG2))"/>
      </when>
      <otherwise>
        <if test="msg:saynl('ILL-FORMED META ENTRY.CATEGORY')"/>
      </otherwise>
    </choose>
  </for-each>
</template>

<!-- build table mapping id to bool indicating if it has fullwidth -->
<!-- prop -->

<template name="build.table.fullwidth">
  <if test="msg:saynl('BUILDING TABLE: LATEX.FULLWIDTH')"/>
  <for-each select="/BOOK/FRONT/META[@NAME='LATEX.FULLWIDTH']">
    <if test="@VALUE and meta:fullwidthPut(string(@VALUE))"/>
  </for-each>
</template>

<!-- declare maps -->

<template name="declare.maps">
  <call-template name="declare.map.text"/>
  <call-template name="declare.map.code"/>
  <call-template name="declare.map.index.text"/>
  <call-template name="declare.map.index.code"/>
</template>

<template name="declare.map.common">
  <txt:enter char="#">\mozartHASH{}</txt:enter>
  <txt:enter char="$">\mozartDOLLAR{}</txt:enter>
  <txt:enter char="%">\mozartPERCENT{}</txt:enter>
  <txt:enter char="&amp;">\mozartAMPERSAND{}</txt:enter>
  <txt:enter char="~">\mozartTILDE{}</txt:enter>
  <txt:enter char="_">\mozartUNDERSCORE{}</txt:enter>
  <txt:enter char="^">\mozartCARET{}</txt:enter>
  <txt:enter char="\">\mozartBSLASH{}</txt:enter>
  <txt:enter char="{{">\mozartLBRACE{}</txt:enter>
  <txt:enter char="}}">\mozartRBRACE{}</txt:enter>
  <txt:enter char="&lt;">\mozartLT{}</txt:enter>
  <txt:enter char="&gt;">\mozartGT{}</txt:enter>
  <txt:enter char="|">\mozartVBAR{}</txt:enter>
  <txt:enter char="&#10;"> \mozartEMPTY
</txt:enter>
</template>


<template name="declare.map.text">
  <txt:defmap name="text">
    <call-template name="declare.map.common"/>
  </txt:defmap>
</template>

<template name="declare.map.code">
  <txt:defmap name="code">
    <call-template name="declare.map.common"/>
    <txt:enter char=" ">\mozartSPACE{}</txt:enter>
    <txt:enter char="&#10;">\mozartNEWLINE
</txt:enter>
  </txt:defmap>
</template>

<template name="declare.map.index.common">
  <txt:enter char="!">"!</txt:enter>
  <txt:enter char='"'>""</txt:enter>
  <txt:enter char="@">"@</txt:enter>
  <txt:enter char="|">"|</txt:enter>
  <txt:enter char="&#10;">\mozartEMPTY
</txt:enter>
</template>

<template name="declare.map.index.text">
  <txt:defmap name="index.text">
    <call-template name="declare.map.common"/>
    <call-template name="declare.map.index.common"/>
  </txt:defmap>
</template>

<template name="declare.map.index.code">
  <txt:defmap name="index.code">
    <call-template name="declare.map.common"/>
    <call-template name="declare.map.index.common"/>
    <txt:enter char=" ">\mozartSPACE{}</txt:enter>
    <txt:enter char="&#10;">\mozartNEWLINE
</txt:enter>
  </txt:defmap>
</template>

<!-- book -->

<template match="BOOK">
  <txt:usemap>\documentclass{ozdoc}
</txt:usemap>
  <if test=".//INDEX">
    <txt:usemap>\DOINDEX
</txt:usemap>
  </if>
  <apply-templates select="FRONT"/>
  <txt:usemap>\begin{document}
</txt:usemap>
  <apply-templates select="BODY"/>
  <call-template name="answers.to.exercises"/>
  <apply-templates select="BACK"/>
  <txt:usemap>\end{document}
</txt:usemap>
</template>

<template match="BOOK/FRONT">
  <for-each select="META[@NAME='LATEX.PACKAGE']">
    <txt:usemap>\usepackage{<value-of select="@VALUE"/>}
</txt:usemap>
  </for-each>
  <for-each select="META[@NAME='LATEX.INPUT']">
    <txt:usemap>\input{<value-of select="@VALUE"/>}
</txt:usemap>
  </for-each>
  <if test="TITLE">
    <txt:usemap>\title{</txt:usemap>
    <apply-templates select="TITLE" mode="ok"/>
    <txt:usemap>}
</txt:usemap>
  </if>
  <if test="AUTHOR|AUTHOR.EXTERN">
    <txt:usemap>\author{</txt:usemap>
    <apply-templates select="AUTHOR|AUTHOR.EXTERN"/>
    <txt:usemap>}
</txt:usemap>
  </if>
  <if test="ABSTRACT">
    <txt:usemap>\mozAbstract{</txt:usemap>
    <apply-templates select="ABSTRACT" mode="abstract"/>
    <txt:usemap>}
</txt:usemap>
  </if>
  <apply-templates select="COMIC[1]" mode="comic"/>
</template>

<!-- ignore FRONT otherwise -->
<template match="FRONT"/>

<!-- for some elements: just recurse through -->

<template match="
        TITLE | BODY | ITEM/P.SILENT | ENTRY/P.SILENT |
        SYNOPSIS/P.SILENT | TD/P.SILENT | TH/P.SILENT |
        DEF | SPAN | FIGURE/CAPTION/P.SILENT | GRAMMAR.HEAD |
        CHUNK/TITLE | CHUNK/TITLE/P.SILENT | CHUNK.SILENT |
        NAME | NOTE/P.SILENT | EXERCISE/P.SILENT | ANSWER/P.SILENT |
        REWRITE.FROM/P.SILENT | REWRITE.TO/P.SILENT |
        REWRITE.CONDITION/P.SILENT | ABSTRACT/P.SILENT">
  <apply-templates/>
</template>

<!-- only go through on purpose, as indicated by the
     choice of mode="abstract" -->
<template match="ABSTRACT" mode="abstract">
  <apply-templates/>
</template>

<!-- default node template: output an error message -->

<template match="*" priority="-1.0">
  <if test="msg:say('UNMATCHED ELEMENT: ') and msg:saynl((local-part(.)))"/>
  <txt:usemap>\UNMATCHED{<value-of select="local-part(.)"/>}</txt:usemap>
</template>

<!-- processing instructions -->

<template match="processing-instruction()">
  <txt:usemap>\usepi{<value-of select="local-part(.)"/>}</txt:usemap>
</template>

<!-- format author. for the time being, we don't actually
     consult the author database but just use the key
     a key is for example Denys.Duchier and we just replace
     the period by a space -->

<template match="AUTHOR.EXTERN">
  <if test="not(position()=1)"><txt:usemap>\EOLN
</txt:usemap></if>
  <value-of select="translate((string(@KEY)),'.',' ')"/>
</template>

<template match="AUTHOR">
  <if test="not(position()=1)"><txt:usemap>\EOLN
</txt:usemap></if>
  <apply-templates/>
</template>

<!-- format a section-like element.
     the preamble defines a sectionning command with the same uppercase
     name as the element. we also add a \label command if the element
     has an id -->

<template match="PART|CHAPTER|APPENDIX|SECTION|SUBSECTION|SUBSUBSECTION|PARA">
  <txt:usemap><text>

</text>\<value-of select="local-part(.)"/></txt:usemap>
  <if test="@CLASS='UNNUMBERED'">
    <txt:usemap>*</txt:usemap>
  </if>
  <txt:usemap>{</txt:usemap>
  <apply-templates select="TITLE | FRONT/TITLE" mode="ok"/>
  <txt:usemap>}</txt:usemap>
  <if test="@ID">
    <txt:usemap>\label{</txt:usemap>
    <value-of select="@ID"/>
    <txt:usemap>}</txt:usemap>
  </if>
  <apply-templates select="FRONT/TITLE//NOTE[@FOOT='FOOT']
                          |      TITLE//NOTE[@FOOT='FOOT']
                          |FRONT/TITLE//REF.EXTERN
                          |      TITLE//REF.EXTERN"
                   mode="footnotetext"/>
  <txt:usemap><text>

</text></txt:usemap>
  <apply-templates/>
</template>

<template match="TITLE"/>
<template match="TITLE" mode="ok">
  <apply-templates/>
</template>

<!-- format a paragraph -->

<template match="P">
  <txt:usemap><text>

</text></txt:usemap>
  <call-template name="maybe.label"/>
  <apply-templates/>
</template>

<template name="maybe.label">
  <if test="@ID">
    <txt:usemap>\label{</txt:usemap>
    <value-of select="@ID"/>
    <txt:usemap>}\ignorespaces
</txt:usemap>
  </if>
</template>

<!-- LIST -->
<!-- 1. simple enumerated list -->

<template match="LIST[@ENUM='ENUM' and not(ENTRY)]">
  <txt:usemap>\begin{enumerate}</txt:usemap>
  <for-each select="ITEM">
    <txt:usemap><text>
\item{}</text></txt:usemap>
    <call-template name="maybe.label"/>
    <apply-templates/>
  </for-each>
  <txt:usemap><text>
\end{enumerate}
</text></txt:usemap>
</template>

<!-- 2. simple itemized list -->

<template match="LIST[not(@ENUM) and not(ENTRY)]">
  <txt:usemap>\begin{itemize}</txt:usemap>
  <for-each select="ITEM">
    <txt:usemap><text>
\item{}</text></txt:usemap>
    <call-template name="maybe.label"/>
    <apply-templates/>
  </for-each>
  <txt:usemap><text>
\end{itemize}
</text></txt:usemap>
</template>

<!-- 3. enumerated description -->

<template match="LIST[@ENUM='ENUM' and ENTRY]">
  <txt:usemap>\begin{enumerate}</txt:usemap>
  <apply-templates mode="list.enum.entry"/>
  <txt:usemap>\end{enumerate}
</txt:usemap>
</template>

<template mode="list.enum.entry" match="ITEM">
  <apply-templates/>
</template>

<template mode="list.enum.entry" match="ENTRY">
  <txt:usemap>\item{}</txt:usemap>
  <call-template name="maybe.label"/>
  <txt:usemap>\textit{</txt:usemap>
  <apply-templates/>
  <txt:usemap>\quad}</txt:usemap>
</template>

<!-- 4. itemized description -->

<template match="LIST[not(@ENUM) and ENTRY]">
  <txt:usemap>\begin{DESCRIPTION}
</txt:usemap>
  <apply-templates mode="list.desc"/>
  <txt:usemap>\end{DESCRIPTION}
</txt:usemap>
</template>

<template mode="list.desc" match="ENTRY">
  <choose>
    <when test="@CLASS and meta:entryCategoryExists(string(@CLASS))">
      <txt:usemap>\ENTRYWITHCATEGORY{</txt:usemap>
      <value-of select="meta:entryCategoryGet(string(@CLASS))"/>
      <txt:usemap>}{</txt:usemap>
    </when>
    <otherwise>
      <txt:usemap>\ENTRY{</txt:usemap>
    </otherwise>
  </choose>
  <apply-templates/>
  <txt:usemap>}</txt:usemap>
  <if test="@ID">
    <txt:usemap>\label{</txt:usemap>
    <value-of select="@ID"/>
    <txt:usemap>}</txt:usemap>
  </if>
  <if test=".//CODE|.//MENU">
    <txt:usemap>\ENTRYHASCODE</txt:usemap>
  </if>
  <txt:usemap><text>
</text></txt:usemap>
</template>

<template mode="list.desc" match="ITEM">
  <txt:usemap>\ITEM </txt:usemap>
  <apply-templates/>
</template>

<template mode="list.desc" match="SYNOPSIS">
  <txt:usemap>\begin{SYNOPSIS}
</txt:usemap>
  <apply-templates/>
  <txt:usemap>\end{SYNOPSIS}
</txt:usemap>
</template>

<!-- menu/mouse -->

<template match="MENU">
  <txt:usemap>\MENU{</txt:usemap>
  <apply-templates/>
  <txt:usemap>}{</txt:usemap>
  <if test="@KEY">
    <txt:usemap>\KEY{</txt:usemap>
    <txt:usemap name="code">
      <value-of select="@KEY"/>
    </txt:usemap>
    <txt:usemap>}</txt:usemap>
  </if>
  <txt:usemap>}{</txt:usemap>
  <if test="@MOUSE">
    <txt:usemap>\MOUSE{<value-of select="@MOUSE"/>}</txt:usemap>
  </if>
  <txt:usemap>}</txt:usemap>
</template>

<!-- format code -->

<template match="REWRITE.FROM/P.SILENT/CODE|REWRITE.TO/P.SILENT/CODE" priority="2.0">
  <txt:usemap name="code">
    <apply-templates/>
  </txt:usemap>
</template>

<template match="CODE[@DISPLAY='INLINE']">
  <txt:usemap>\CODEINLINE{</txt:usemap>
  <txt:usemap name="code">
    <apply-templates/>
  </txt:usemap>
  <txt:usemap>}</txt:usemap>
</template>

<template match="CODE.EXTERN[@DISPLAY='DISPLAY']">
  <txt:usemap>\CODEEXTERN{<value-of select="@TO"/>}</txt:usemap>
</template>

<template match="CODE[@DISPLAY='DISPLAY']">
  <txt:usemap>\begin{CODEDISPLAY}</txt:usemap>
  <txt:usemap name="code">
    <apply-templates/>
  </txt:usemap>
  <txt:usemap>\end{CODEDISPLAY}
</txt:usemap>
</template>

<template match="HILITE">
  <apply-templates/>
</template>

<template match="HILITE.FACE">
  <txt:usemap>\FACE<value-of select="@NAME"/>{</txt:usemap>
  <apply-templates/>
  <txt:usemap>}</txt:usemap>
</template>

<template match="CODE/SPAN[@CLASS='IGNORE']"/>

<template match="SPAN[@CLASS='INDEX']">
  <!-- first process normally -->
  <apply-templates/>
  <!-- then create indexing command -->
  <call-template name="process.as.index"/>
</template>

<!-- format variables -->

<template match="VAR">
  <variable name="type"><value-of select="@TYPE"/></variable>
  <if test="$type='PROG' and @MODE">
    <txt:usemap>\MODE{<value-of select="@MODE"/>}</txt:usemap>
  </if>
  <txt:usemap>\<value-of select="$type"/>VAR{</txt:usemap>
  <apply-templates/>
  <txt:usemap>}</txt:usemap>
</template>

<!-- format code text: preserve newlines and escape weird chars.
     for efficiency: don't use CODE//text(), just look 1, 2 or
     3 elements up -->

<!--
<template match=" CODE/text()
                 |CODE/*/text()
                 |CODE/*/*/text()
                 |CHUNK.SILENT/text()
                 |CHUNK.SILENT/*/text()
                 |CHUNK.SILENT/*/*/text()">
  <tex:verb><value-of select="."/></tex:verb>
</template>
-->

<!-- miscellaneous commands -->

<template match="FILE | SAMP | KBD | KEY">
  <call-template name="maybe.display.begin"/>
  <txt:usemap>\<value-of select="local-part(.)"/>{</txt:usemap>
  <txt:usemap name="code">
    <apply-templates/>
  </txt:usemap>
  <txt:usemap>}</txt:usemap>
  <call-template name="maybe.label"/>
  <call-template name="maybe.display.end"/>
</template>

<template match="EM">
  <call-template name="maybe.display.begin"/>
  <txt:usemap>\<value-of select="local-part(.)"/>{</txt:usemap>
  <apply-templates/>
  <txt:usemap>}</txt:usemap>
  <call-template name="maybe.display.end"/>
</template>

<template match="Q">`<apply-templates/>'</template>

<!-- references -->

<template match="REF.EXTERN">
  <apply-templates/>
  <choose>
    <when test="ancestor::TITLE">
      <txt:usemap>\protect\footnotemark</txt:usemap>
    </when>
    <otherwise>
      <txt:usemap>\footnote{</txt:usemap>
      <call-template name="ref.extern" select="."/>
      <txt:usemap>}</txt:usemap>
    </otherwise>
  </choose>
</template>

<template match="PTR.EXTERN">
  <call-template name="ref.extern" select="."/>
</template>

<template name="ref.extern">
  <variable name="to"><value-of select="@TO"/></variable>
  <choose>
    <when test="starts-with($to,'ozdoc:')">
      <choose>
        <when test="@KEY">
          <variable name="key"><value-of select="translate(string(@KEY),'abcdefghijklmnopqrstuvwxyz','ABCDEFGHIJKLMNOPQRSTUVWXYZ')"/></variable>
          <txt:usemap>\OZDOCREF{</txt:usemap>
          <value-of select="$to"/>
          <txt:usemap>}{</txt:usemap>
          <value-of select="$key"/>
          <txt:usemap>}{</txt:usemap>
          <apply-templates select="/BOOK/OZDOC.DB/OZDOC.DOCUMENT[@NAME=$to]/OZDOC.BOOK/OZDOC.ENTRY[@KEY=$key]"/>
          <txt:usemap>}</txt:usemap>
        </when>
        <otherwise>
          <txt:usemap>\OZDOCREFTOP{</txt:usemap>
          <value-of select="$to"/>
          <txt:usemap>}{</txt:usemap>
          <apply-templates
          select="/BOOK/OZDOC.DB/OZDOC.DOCUMENT[@NAME=$to]/OZDOC.BOOK/TITLE"
          mode="ok"/>
          <txt:usemap>}</txt:usemap>
        </otherwise>
      </choose>
    </when>
    <otherwise>
      <txt:usemap>\DEFAULTREFEXT{</txt:usemap>
      <value-of select="$to"/>
      <txt:usemap>}</txt:usemap>
    </otherwise>
  </choose>
</template>

<template match="OZDOC.DB"/>
<template match="OZDOC.ENTRY">
  <!-- {TYPE}{SECTITLE}{DOCTITLE} -->
  <txt:usemap>{</txt:usemap>
  <value-of select="@TYPE"/>
  <txt:usemap>}{</txt:usemap>
  <apply-templates select="TITLE[1]" mode="ok"/>
  <txt:usemap>}{</txt:usemap>
  <apply-templates select="../TITLE[1]" mode="ok"/>
  <txt:usemap>}</txt:usemap>
</template>

<template match="PTR">
  <apply-templates mode="ptr.ref" select="id:get((string(@TO)))"/>
</template>

<template match="REF">
  <apply-templates/>
  <apply-templates mode="ref.ref" select="id:get((string(@TO)))"/>
</template>

<template mode="ptr.ref" match="CHAPTER">
  <txt:usemap name="text">Chapter</txt:usemap>
  <call-template name="ref.to.id"/>
</template>

<template mode="ptr.ref" match="PART">
  <txt:usemap name="text">Part</txt:usemap>
  <call-template name="ref.to.id"/>
</template>

<template mode="ptr.ref" match="APPENDIX">
  <txt:usemap name="text">Appendix</txt:usemap>
  <call-template name="ref.to.id"/>
</template>

<template mode="ptr.ref" match="SECTION | SUBSECTION | SUBSUBSECTION">
  <txt:usemap name="text">Section</txt:usemap>
  <call-template name="ref.to.id"/>
</template>

<template mode="ptr.ref" match="PARA|ENTRY|P|CHUNK">
  <call-template name="pageref.to.id"/>
</template>

<template mode="ptr.ref" match="FIGURE">
  <txt:usemap name="text">Figure</txt:usemap>
  <call-template name="ref.to.id"/>
</template>

<template mode="ptr.ref" match="BIB.EXTERN">
  <txt:usemap>\cite{</txt:usemap>
  <value-of select="@KEY"/>
  <txt:usemap>}</txt:usemap>
</template>

<template mode="ptr.ref" match="ITEM">
  <txt:usemap name="text">item</txt:usemap>
  <call-template name="ref.to.id"/>
  <call-template name="pageref.to.id"/>
</template>

<template mode="ptr.ref" match="KEY">
  <txt:usemap>\KEY{</txt:usemap>
  <txt:usemap name="code">
    <value-of select="."/>
  </txt:usemap>
  <txt:usemap>}</txt:usemap>
  <call-template name="pageref.to.id"/>
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
  <txt:usemap>~\REF{</txt:usemap>
  <value-of select="@ID"/>
  <txt:usemap>}</txt:usemap>
</template>

<template mode="ref.ref" match="BIB.EXTERN">
  <txt:usemap>\cite{</txt:usemap>
  <value-of select="@KEY"/>
  <txt:usemap>}</txt:usemap>
</template>

<template mode="ref.ref" match="*">
  <call-template name="pageref.to.id"/>
</template>

<template name="pageref.to.id">
  <txt:usemap> (page~\PAGEREF{</txt:usemap>
  <value-of select="@ID"/>
  <txt:usemap>})</txt:usemap>
</template>

<!-- format tables -->

<template match="TABLE">
  <call-template name="maybe.display.begin.table"/>
  <txt:usemap>\begin{tabular}{</txt:usemap>
  <choose>
    <when test="@ID and meta:latexTableSpecExists((string(@ID)))">
      <txt:usemap>
        <value-of select="meta:latexTableSpecGet((string(@ID)))"/>
      </txt:usemap>
    </when>
    <otherwise>
      <for-each select="TR[position()=1]/*">
        <apply-templates mode="colspan" select="."/>
      </for-each>
    </otherwise>
  </choose>
  <txt:usemap>}</txt:usemap>
  <apply-templates/>
  <txt:usemap>\end{tabular}</txt:usemap>
  <call-template name="maybe.display.end.table"/>
</template>

<template match="TD[@COLSPAN]|TH[@COLSPAN]" mode="colspan">
  <txt:usemap>*{</txt:usemap>
  <value-of select="@COLSPAN"/>
  <txt:usemap>}{Dl}</txt:usemap>
</template>

<template match="TD[not(@COLSPAN)]|TH[not(@COLSPAN)]" mode="colspan">
  <txt:usemap>Dl</txt:usemap>
</template>

<template name="maybe.display.begin.table">
  <if test="not(last()=1 and parent::P.SILENT/parent::TD)">
    <call-template name="maybe.display.begin"/>
  </if>
</template>

<template name="maybe.display.end.table">
  <if test="not(last()=1 and parent::P.SILENT/parent::TD)">
    <call-template name="maybe.display.end"/>
  </if>
</template>

<template name="maybe.display.begin">
  <if test="@DISPLAY='DISPLAY'">
    <choose>
      <when test="@ID and meta:fullwidthGet(string(@ID))">
        <txt:usemap>\begin{FULLWIDTHLEFT}</txt:usemap>
      </when>
      <otherwise>
        <txt:usemap>\begin{center}</txt:usemap>
      </otherwise>
    </choose>
  </if>
</template>

<template name="maybe.display.end">
  <if test="@DISPLAY='DISPLAY'">
    <choose>
      <when test="@ID and meta:fullwidthGet(string(@ID))">
        <txt:usemap>\end{FULLWIDTHLEFT}</txt:usemap>
      </when>
      <otherwise>
        <txt:usemap>\end{center}</txt:usemap>
      </otherwise>
    </choose>
  </if>
</template>

<template name="maybe.really.display.begin">
  <if test="@DISPLAY='DISPLAY'">
    <txt:usemap>\begin{DISPLAY}</txt:usemap>
  </if>
</template>

<template name="maybe.really.display.end">
  <if test="@DISPLAY='DISPLAY'">
    <txt:usemap>\end{DISPLAY}</txt:usemap>
  </if>
</template>

<template match="TR">
  <apply-templates/>
  <if test="not(position()=last())">
    <txt:usemap>\EOLN
</txt:usemap>
  </if>
</template>

<!-- TD that just contains a picture:
     suppress the color background
  -->
<template match="TD[node()[last()=1 and
                           local-part()='P.SILENT' and
                           node()[last()=1 and
                                  local-part()='PICTURE.EXTERN']]
                    and not(@ID and meta:latexTableSpecExists(string(@ID)))]">
  <txt:usemap>\multicolumn{1}{l}{</txt:usemap>
  <apply-templates/>
  <txt:usemap>}</txt:usemap>
</template>

<template match="TD | TH">
  <if test="not(position()=1)">
    <txt:usemap>&amp;</txt:usemap>
  </if>
  <choose>
    <when test="@COLSPAN">
      <txt:usemap>\multicolumn{<value-of select="@COLSPAN"/>}{</txt:usemap>
      <choose>
        <when test="@ID and meta:latexTableSpecExists((string(@ID)))">
          <txt:usemap>
            <value-of select="meta:latexTableSpecExists((string(@ID)))"/>
          </txt:usemap>
        </when>
        <when test="local-part()='TD'">Dl</when>
        <otherwise>Hl</otherwise>
      </choose>
      <txt:usemap>}{</txt:usemap>
    </when>
  </choose>
  <apply-templates/>
  <if test="@COLSPAN">
    <txt:usemap>}</txt:usemap>
  </if>
</template>

<!-- figures -->

<template match="FIGURE">
  <if test="msg:saynl('FOUND NON LEIF EXAMPLE')"/>
  <txt:usemap>\begin{FIGURE}</txt:usemap>
  <apply-templates/>
  <if test="@ID and not(CAPTION)">
    <txt:usemap>\CAPTIONID{</txt:usemap>
    <value-of select="@ID"/>
    <txt:usemap>}</txt:usemap>
  </if>
  <txt:usemap>\end{FIGURE}</txt:usemap>
</template>

<template match="FIGURE/TITLE">
  <txt:usemap>\begin{FIGTITLE}</txt:usemap>
  <apply-templates/>
  <txt:usemap>\end{FIGTITLE}</txt:usemap>
</template>

<template match="FIGURE/CAPTION">
  <txt:usemap>\caption{</txt:usemap>
  <if test="../@ID">
    <txt:usemap>\label{</txt:usemap>
    <value-of select="../@ID"/>
    <txt:usemap>}</txt:usemap>
  </if>
  <apply-templates/>
  <txt:usemap>}</txt:usemap>
</template>

<!--  At Leif's request: try to identify figures
      whose sole content is one displayed element
      and don't indent it.
  -->

<template match="FIGURE[node()[not(self::TITLE  ) and
                               not(self::CAPTION) and
                               not(self::INDEX  )]
                              [last()=1]
                              [self::P[node()[last()=1 and
                                              @DISPLAY='DISPLAY']]
                               or
                               node()[@DISPLAY='DISPLAY']]]"
          priority="2.0">
  <if test="msg:say('FOUND LEIF EXAMPLE ID=') and
            msg:saynl(string(@ID))"/>
  <txt:usemap>\begin{FIGURENODISPLAY}</txt:usemap>
  <apply-templates/>
  <if test="@ID and not(CAPTION)">
    <txt:usemap>\CAPTIONID{</txt:usemap>
    <value-of select="@ID"/>
    <txt:usemap>}</txt:usemap>
  </if>
  <txt:usemap>\end{FIGURENODISPLAY}</txt:usemap>
</template>

<!-- comic pictures -->

<template match="COMIC" mode="comic">
  <apply-templates mode="comic"
                   select="PICTURE.CHOICE/PICTURE.EXTERN[@TYPE='PS'][1]"/>
</template>

<template match="PICTURE.EXTERN[@TYPE='PS']" mode="comic">
  <txt:usemap>\mozComic{</txt:usemap>
  <value-of select="@TO"/>
  <txt:usemap>}
</txt:usemap>
</template>

<template match="*" mode="comic" priority="-2.0">
  <if test="msg:say('UNEXPECTED ELEMENT IN mode=comic: ') and
            msg:saynl(string(local-part()))"/>
</template>

<!-- pictures -->

<template match="PICTURE.CHOICE">
  <call-template name="maybe.display.begin"/>
  <choose>
    <when test="PICTURE[@TYPE='LATEX']">
      <txt:usemap><value-of select="PICTURE[@TYPE='LATEX']"/></txt:usemap>
    </when>
    <when test="PICTURE.EXTERN[@TYPE='PS']">
      <apply-templates select="PICTURE.EXTERN[@TYPE='PS'][1]" mode="pic.choice"/>
    </when>
    <when test="PICTURE.EXTERN[@TYPE='GIF']">
      <apply-templates select="PICTURE.EXTERN[@TYPE='GIF'][1]" mode="pic.choice"/>
    </when>
    <otherwise>
      <if test="msg:saynl('UNMATCHED PICTURE')"/>
      <text>!!!UNMATCHED PICTURE!!!</text>
    </otherwise>
  </choose>
  <call-template name="maybe.display.end"/>
</template>

<template match="PICTURE.EXTERN[@TYPE='PS']" mode="pic.choice">
  <txt:usemap>\PICEXT</txt:usemap>
  <if test="@ID and meta:pictureWidthExists(string(@ID))">
    <txt:usemap>[<value-of select="meta:pictureWidthGet(string(@ID))"/>]</txt:usemap>
  </if>
  <txt:usemap>{</txt:usemap>
  <value-of select="@TO"/>
  <txt:usemap>}</txt:usemap>
</template>

<template match="PICTURE.EXTERN[@TYPE='GIF']" mode="pic.choice">
  <!-- for foo.gif, we will create foo.gif.ps -->
  <txt:usemap>\PICEXTGIF</txt:usemap>
  <if test="@ID and meta:pictureWidthExists(string(@ID))">
    <txt:usemap>[<value-of select="meta:pictureWidthGet(string(@ID))"/>]</txt:usemap>
  </if>
  <txt:usemap>{</txt:usemap>
  <value-of select="@TO"/>
  <txt:usemap>}</txt:usemap>
</template>

<template match="PICTURE.EXTERN[@DISPLAY='INLINE']|TD/P.SILENT/PICTURE.EXTERN" priority="2.0">
  <txt:usemap>\PICEXT</txt:usemap>
  <if test="@ID and meta:pictureWidthExists(string(@ID))">
    <txt:usemap>[<value-of select="meta:pictureWidthGet(string(@ID))"/>]</txt:usemap>
  </if>
  <txt:usemap>{</txt:usemap>
  <value-of select="@TO"/>
  <txt:usemap>}</txt:usemap>
</template>

<template match="PICTURE[@TYPE='LATEX']">
  <txt:usemap><value-of select="."/></txt:usemap>
</template>

<template match="PICTURE.EXTERN[@DISPLAY='DISPLAY']">
  <variable name="type"><value-of select="@TYPE"/></variable>
  <variable name="to"><value-of select="@TO"/></variable>
  <txt:usemap>\begin{PICEXTDISPLAY}</txt:usemap>
  <choose>
    <when test="$type='LATEX'">
      <txt:usemap>\input{</txt:usemap>
      <value-of select="$to"/>
      <txt:usemap>}</txt:usemap>
    </when>
    <when test="$type='GIF' or $type='PS'">
      <txt:usemap>\PICEXTFULL</txt:usemap>
      <if test="@ID and meta:pictureWidthExists(string(@ID))">
        <txt:usemap>[<value-of select="meta:pictureWidthGet(string(@ID))"/>]</txt:usemap>
      </if>
      <txt:usemap>{</txt:usemap>
      <value-of select="$to"/>
      <txt:usemap>}</txt:usemap>
    </when>
    <otherwise>
      <if test="msg:say('UNMATCHED PICTURE.EXTERN: ') and msg:saynl(string($to))"/>
      <text>!!!UNMATCHED PICTURE.EXTERN!!!</text>
      <value-of select="$to"/>
      <text>!!!</text>
    </otherwise>
  </choose>
  <txt:usemap>\end{PICEXTDISPLAY}</txt:usemap>
</template>

<!-- grammar thingies -->

<template match="GRAMMAR.RULE | GRAMMAR">
  <call-template name="maybe.really.display.begin"/>
  <txt:usemap>\begin{tabular}{lrll}
</txt:usemap>
  <apply-templates/>
  <txt:usemap>\end{tabular}
</txt:usemap>
  <call-template name="maybe.really.display.end"/>
</template>

<template match="GRAMMAR/GRAMMAR.RULE" priority="2.0">
  <if test="position()>1"><txt:usemap>\EOLN
</txt:usemap></if>
  <apply-templates/>
</template>

<template match="GRAMMAR.ALT">
  <txt:usemap>&amp;</txt:usemap>
  <txt:usemap>
    <choose>
      <when test="not(@TYPE) and position()=2 or @TYPE='DEF'">\GRAMDEF</when>
      <when test="@TYPE='ADD'">\GRAMADD</when>
      <when test="not(@TYPE) and position()>2 or @TYPE='OR'">\GRAMOR</when>
      <when test="@TYPE='SPACE'">\GRAMSPACE</when>
    </choose>
  </txt:usemap>
  <txt:usemap>&amp;</txt:usemap>
  <apply-templates/>
  <if test="not(child::GRAMMAR.NOTE)">
    <txt:usemap>&amp;</txt:usemap>
  </if>
  <if test="not(position()=last())"><txt:usemap>\EOLN
</txt:usemap></if>
</template>

<template match="GRAMMAR.NOTE">
  <txt:usemap>&amp;\GRAMMARNOTE{</txt:usemap>
  <apply-templates/>
  <txt:usemap>}</txt:usemap>
</template>

<!-- math -->

<template match="MATH[@TYPE='LATEX']">
  <txt:usemap>
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
  </txt:usemap>
</template>

<template match="MATH.CHOICE[MATH[@TYPE='LATEX']]">
  <apply-templates select="MATH[@TYPE='LATEX']"/>
</template>

<!-- literate programming: chunks -->

<template match="CHUNK">
  <variable name="env">
    <choose>
      <when test="@CLASS and @CLASS='ANONYMOUS'">CHUNKANON</when>
      <otherwise>CHUNK</otherwise>
    </choose>
  </variable>
  <txt:usemap>\begin{<value-of select="$env"/>}{</txt:usemap>
  <apply-templates select="TITLE"/>
  <txt:usemap>}</txt:usemap>
  <call-template name="maybe.label"/>
  <txt:usemap name="code">
    <apply-templates select="CHUNK.SILENT"/>
  </txt:usemap>
  <txt:usemap>\end{<value-of select="$env"/>}</txt:usemap>
</template>

<template match="CHUNK.REF">
  <txt:usemap>\CHUNKREF{</txt:usemap>
  <txt:usemap name="text">
    <apply-templates/>
  </txt:usemap>
  <txt:usemap>}</txt:usemap>
</template>

<!-- notes and footnotes -->

<template match="NOTE[@FOOT='FOOT']" mode="footnotetext">
  <txt:usemap>
\footnotetext{</txt:usemap>
  <apply-templates/>
  <txt:usemap>}</txt:usemap>
</template>

<template match="REF.EXTERN" mode="footnotetext">
  <txt:usemap>
\footnotetext{</txt:usemap>
  <call-template name="ref.extern" select="."/>
  <txt:usemap>}</txt:usemap>
</template>

<template match="TITLE//NOTE[@FOOT='FOOT']" priority="3.0">
  <txt:usemap>\protect\footnotemark{}</txt:usemap>
</template>

<template match="NOTE[@FOOT='FOOT']" priority="2.0">
  <txt:usemap>\footnote{</txt:usemap>
  <apply-templates/>
  <txt:usemap>}</txt:usemap>
</template>

<template match="NOTE">
  <txt:usemap>\paragraph{Note:}</txt:usemap>
  <apply-templates/>
</template>

<template match="NOTE.GUI">
  <txt:usemap>\NOTEGUI{</txt:usemap>
  <apply-templates/>
  <txt:usemap>}{</txt:usemap>
  <choose>
    <when test="@ICON='note-gui-l1.gif'">mouse-L1</when>
    <when test="@ICON='note-gui-l2.gif'">mouse-L2</when>
    <when test="@ICON='note-gui-m1.gif'">mouse-M1</when>
    <when test="@ICON='note-gui-m2.gif'">mouse-M2</when>
    <when test="@ICON='note-gui-r1.gif'">mouse-R1</when>
    <when test="@ICON='note-gui-r2.gif'">mouse-R2</when>
    <when test="@ICON='note-gui-lm1.gif'">mouse-LM1</when>
    <otherwise>
      <if test="msg:say('UNKNOWN NOTE.GUI ICON: ') and msg:saynl(string(@ICON))"/>
      <text>!!!UNKNOWN NOTE.GUI ICON!!!</text>
      <value-of select="@ICON"/>
      <text>!!!</text>
    </otherwise>
  </choose>
  <txt:usemap>}</txt:usemap>
</template>

<!-- exercises -->

<template match="EXERCISE[@ID]">
  <txt:usemap>\begin{EXERCISE}{</txt:usemap>
  <value-of select="@ID"/>
  <txt:usemap>}</txt:usemap>
  <apply-templates/>
  <txt:usemap>\end{EXERCISE}
</txt:usemap>
</template>

<!-- answer to exercises -->

<template match="ANSWER"/>

<template name="answers.to.exercises">
  <if test="//ANSWER">
    <txt:usemap><text>
\APPENDIX{Answers To Exercises}
</text></txt:usemap>
    <apply-templates select="//ANSWER" mode="answer"/>
  </if>
</template>

<template match="ANSWER" mode="answer">
  <txt:usemap>\begin{ANSWER}{</txt:usemap>
    <value-of select="@TO"/>
    <txt:usemap>}</txt:usemap>
    <apply-templates/>
    <txt:usemap>\end{ANSWER}
</txt:usemap>
</template>

<!-- back matter -->

<template match="BACK">
  <for-each select="BIB.EXTERN[position()=1]">
    <txt:usemap><text>
\bibliographystyle{plain}
\bibliography{</text></txt:usemap>
    <value-of select="substring-before((string(@TO)),'.bib')"/>
    <txt:usemap>}</txt:usemap>
  </for-each>
</template>

<!-- rewrite elements -->

<template match="REWRITE">
  <txt:usemap>\begin{REWRITE}</txt:usemap>
  <for-each select="VAR">
    <txt:usemap>\REWRITEVAR{</txt:usemap>
    <apply-templates select="."/>
    <txt:usemap>}
</txt:usemap>
  </for-each>
  <apply-templates select="REWRITE.FROM[1]"/>
  <apply-templates select="REWRITE.TO[1]"/>
  <apply-templates select="REWRITE.CONDITION[1]"/>
  <txt:usemap>\end{REWRITE}</txt:usemap>
</template>

<template match="REWRITE.FROM">
  <txt:usemap>\REWRITEFROM{</txt:usemap>
  <apply-templates/>
  <txt:usemap>}</txt:usemap>
</template>

<template match="REWRITE.TO">
  <txt:usemap>\REWRITETO{</txt:usemap>
  <apply-templates/>
  <txt:usemap>}</txt:usemap>
</template>

<template match="REWRITE.CONDITION">
  <txt:usemap>\REWRITECONDITION{</txt:usemap>
  <apply-templates/>
  <txt:usemap>}</txt:usemap>
</template>

<!-- DTD doc support -->

<template match="TAG|
                 NAME[@CLASS and meta:equal(string(@CLASS),'TAG')]|
                 NAME[@TYPE and meta:equal(string(@TYPE),'TAG')]"
          priority="2.0">
  <txt:usemap>\DTDTAG{</txt:usemap>
  <apply-templates/>
  <txt:usemap>}</txt:usemap>
</template>

<template match="ATTRIB">
  <txt:usemap>\DTDATTRIB{</txt:usemap>
  <apply-templates/>
  <txt:usemap>}</txt:usemap>
</template>

<template match="NAME[@TYPE and meta:equal(string(@TYPE),'PI')]"
          priority="2.0">
  <txt:usemap>\DTDPI{</txt:usemap>
  <apply-templates/>
  <txt:usemap>}</txt:usemap>
</template>

<!-- OPI support -->

<template match="NAME[@TYPE and meta:equal(string(@TYPE),'BUFFER')]"
          priority="2.0">
  <txt:usemap>\OPINAMEBUFFER{</txt:usemap>
  <txt:usemap name="code">
    <apply-templates/>
  </txt:usemap>
  <txt:usemap>}</txt:usemap>
</template>

<!-- indexing -->

<template match="INDEX">
  <call-template name="process.as.index"/>
</template>

<template name="process.as.index">
  <txt:usemap>\INDEX{</txt:usemap>
  <txt:alias from="text" to="index.text">
    <txt:alias from="code" to="index.code">
      <txt:usemap name="text">
        <apply-templates mode="index.key"/>
        <txt:usemap>@</txt:usemap>
        <apply-templates/>
      </txt:usemap>
    </txt:alias>
  </txt:alias>
  <txt:usemap>}</txt:usemap>
</template>

<template match="AND" mode="index.key">
  <if test="position()&gt;1"><txt:usemap>!</txt:usemap></if>
  <apply-templates mode="index.key"/>
</template>

<template match="processing-instruction()" mode="index.key">
  <apply-templates select="."/>
</template>

<template match="AND">
  <if test="position()&gt;1"><txt:usemap>, </txt:usemap></if>
  <apply-templates/>
</template>

<template match="SEE">
  <if test="msg:saynl('IGNORING SEE')"/>
</template>

</stylesheet>
