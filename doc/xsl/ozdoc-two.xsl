<!-- -*-xml-*- -->

<stylesheet
 xmlns		= "http://www.w3.org/XSL/Transform/1.0"
 xmlns:txt	= "http://www.jclark.com/xt/java"
 extension-element-prefixes="txt"
 result-ns	= ""
 xmlns:key	= "http://www.jclark.com/xt/java/Key"
 xmlns:msg	= "http://www.jclark.com/xt/java/Msg"
>
<!-- for elements that only contain other elements but never data,
     all spaces should be stripped away -->

<strip-space elements="
	BOOK FRONT BACK BODY PART CHAPTER SECTION SUBSECTION
	SUBSUBSECTION APPENDIX LIST ENTRY SYNOPSIS MATH.CHOICE
	PICTURE.CHOICE CHUNK FIGURE INDEX SEE GRAMMAR.RULE
	GRAMMAR TABLE TR"/>

<!-- these parameters should be passed as args to the XSL processor -->

<param name="AUTHOR_FILE">
  <if test="moz:error('Missing AUTHOR_FILE parameter')"/>
</param>
<param name="XREF_FILE">
  <if test="moz:error('Missing XREF_FILE parameter')"/>
</param>
<param name="HILITE_FILE">
  <if test="moz:error('Missing HILITE_FILE paramter')"/>
</param>

<!-- root processing:

	1. load some other documents into variables
	1.a the author database
	1.b the cross ref database
	1.c the hilited code

	2. build some tables - this step might go away once
	   XT implements <key>, assuming it does so efficiently
	   for the above, and also for IDs

	3. transform the document to LaTeX
-->

<template match="/">
  <call-template name="build.table.author" />
  <call-template name="build.table.xref"   />
  <call-template name="build.table.hilite" />
  <call-template name="build.table.local   />
  <txt:document>
    <call-template name="declare.maps"/>
    <txt:use name="text">
      <apply-templates/>
    </txt:use>
  </txt:document>
</template>

<!-- author table -->
<template name="build.table.author">
  <apply-templates select="document($AUTHOR_FILE)" mode="build.table.author"/>
</template>

<template match="text()" mode="build.table.author"/>
<template match="author" mode="build.table.author">
  <if test="key:put('author',string(@key),.)"/>
</template>

<!-- xref table -->
<template name="build.table.xref">
  <apply-templates select="document($XREF_FILE)" mode="build.table.xref"/>
</template>

<template match="text()" mode="build.table.xref"/>
<template match="xref.document" mode="build.table.xref">
  <variable name="key">ozdoc:<value-of select="name"/></variable>
  <if test="key:put('xref',string($key),.)"/>
  <for-each select="xref.entry">
    <if test="key:put(string($key),string(@key),.)"/>
  </for-each>
</template>

<!-- hilite table -->
<template match="build.table.hilite">
  <apply-templates select="document($HILITE_FILE)" mode="build.table.hilite"/>
</template>

<template match="text()" mode="build.table.hilite"/>
<template match="HILITE" mode="build.table.hilite">
  <if test="key:put('hilite',string(@ID),.)"/>
</template>

<!-- meta, id, and code.id tables. also we save index
     elements just so we can know that there are any.
  -->
<template match="build.table.local">
  <apply-templates mode="build.table.local"/>
</template>

<template match="text()" mode="build.table.local"/>
<template match="*" mode="build.table.local">
  <if test="@ID and key:put('id',string(@ID),.)"/>
  <if test="@CODE.ID and key:put('code.id',string(@CODE.ID),.)"/>
  <if test="local-name()='INDEX' and key:put('index','all',.)"/>
  <if test="local-name()='ANSWER' and key:put('answer','all',.)"/>
  <if test="local-name()='META' and
     ((@ARG1  and key:put(string(@NAME),string(@ARG1),.)) or
      (@VALUE and key:put(string(@NAME),'all',.)))"/>
  <apply-templates mode="build.table.local"/>
</template>

<!-- escape maps -->

<template name="declare.maps">
  <!-- all LaTeX special characters must be escaped
       we must also escape newline because otherwise 2 in a row
       mean \par.
    -->
  <txt:define name="text">
    <txt:escape char="#"    >\mozHash </txt:escape>
    <txt:escape char="$"    >\mozDollar </txt:escape>
    <txt:escape char="%"    >\mozPercent </txt:escape>
    <txt:escape char="%amp;">\mozAmpersand </txt:escape>
    <txt:escape char="~"    >\mozTilde </txt:escape>
    <txt:escape char="_"    >\mozUnderscore </txt:escape>
    <txt:escape char="^"    >\mozCaret </txt:escape>
    <txt:escape char="\"    >\mozBackslash </txt:escape>
    <txt:escape char="{{"   >\mozLeftbrace </txt:escape>
    <txt:escape char="}}"   >\mozRightbrace </txt:escape>
    <txt:escape char="&lt;" >\mozLessthan </txt:escape>
    <txt:escape char="&gt;" >\mozGreaterthan </txt:escape>
    <txt:escape char="|"    >\mozVbar </txt:escape>
    <txt:escape char="&#10;"> \mozEmpty
</txt:escape>
  </txt:define>
  <!-- code escaping is the same except that space is
       also escaped because it must be preserved for
       proper indentation
    -->
  <txt:define name="code.inline">
    <txt:escape char="#"    >\mozCodeHash </txt:escape>
    <txt:escape char="$"    >\mozCodeDollar </txt:escape>
    <txt:escape char="%"    >\mozCodePercent </txt:escape>
    <txt:escape char="%amp;">\mozCodeAmpersand </txt:escape>
    <txt:escape char="~"    >\mozCodeTilde </txt:escape>
    <txt:escape char="_"    >\mozCodeUnderscore </txt:escape>
    <txt:escape char="^"    >\mozCodeCaret </txt:escape>
    <txt:escape char="\"    >\mozCodeBackslash </txt:escape>
    <txt:escape char="{{"   >\mozCodeLeftbrace </txt:escape>
    <txt:escape char="}}"   >\mozCodeRightbrace </txt:escape>
    <txt:escape char="&lt;" >\mozCodeLessthan </txt:escape>
    <txt:escape char="&gt;" >\mozCodeGreaterthan </txt:escape>
    <txt:escape char="|"    >\mozCodeVbar </txt:escape>
    <txt:escape char="&#10;">\mozCodeSpace
</txt:escape>
    <txt:escape char=" ">\mozCodeSpace </txt:escape>
  </txt:define>
  <!-- code display -->
  <txt:define name="code.display">
    <txt:include name="code.inline"/>
    <txt:escape char="&#10;">\mozCodeDisplayNewline
</txt:escape>
  </txt:define>
  <!-- within \index{...} additional characters meaningful
       to makeindex must be quoted with a preceding `"'
       every occurrence of a newline becomes a non escaped
       space.  However we must retain the newline so as
       not to exceed TeX's input buffer.
    -->
  <txt:define name="index.common">
    <txt:escape char="!">"!</txt:escape>
    <txt:escape char='"'>""</txt:escape>
    <txt:escape char="@">"@</txt:escape>
    <txt:escape char="|">"|</txt:escape>
    <txt:escape char="&#10;"> \mozEmpty
</txt:escape>
  </txt:define>
  <!-- index.text and index.code are like text and code
       but with the additional escaping required for
       \index{...}.  we will use <alias> to alias text and
       code to index.text and index.code while processing
       <index> elements
    -->
  <txt:define name="index.text">
    <txt:include name="text"/>
    <txt:include name="index.common"/>
  </txt:define>
  <txt:define name="index.code.inline">
    <txt:include name="code.inline"/>
    <txt:include name="index.common"/>
  </txt:define>
  <txt:define name="index.code.display">
    <txt:include name="code.display"/>
    <txt:include name="index.common"/>
  </txt:define>
</template>

<template match="BOOK">
  <txt:use>\documentclass{mozart}
</txt:use>
  <if test="key:get('index','all')">
    <txt:use>\mozDoindex
</txt:use>
  <apply-templates select="FRONT"/>
  <txt:use>\begin{document}
</txt:use>
  <apply-templates select="BODY"/>
  <call-template name="answer.to.exercises"/>
  <apply-templates select="BACK"/>
  <txt:use>\end{document}
</txt:use>
</template>

<template match="BOOK/FRONT">
  <for-each select="key:get('LATEX.PACKAGE','all')">
    <txt:use>\usepackage{<value-of select="@VALUE"/>}
</txt:use>
  </for-each>
  <for-each select="key:get('LATEX.INPUT,'all')">
    <txt:use>\input{<value-of select="@VALUE"/>}
</txt:use>
  </for-each>
  <if test="TITLE">
    <txt:use>\title{</txt:use>
    <apply-templates select="TITLE" mode="title"/>
    <txt:use>}</txt:use>
  </if>
  <if test="AUTHOR|AUTHOR.EXTERN">
    <txt:use>\author{</txt:use>
    <apply-templates select="AUTHOR|AUTHOR.EXTERN"/>
    <txt:use>}
</txt:use>
  </if>
</template>

<!-- ignore FRONT otherwise -->
<template match="FRONT"/>

<!-- we don't want a default template that silently
     recurses through.  Instead we want a message that
     informs us that there is a case which we have
     failed to anticipate.
  -->

<template match="*" priority="-1.0">
  <call-template name="error.element"/>
</template>

<template name="error.element">
  <param name="mode"/>
  <if test="msg:say('UNMATCHED ELEMENT: ') and
            msg:say(local-name()) and
            msg:say(' IN MODE ') and
            msg:saynl($mode)"/>
  <txt:use>\mozUnmatchedElement{<value-of select="local-name()"/>}</txt:use>
</template>

<!-- for some elements: just recurse through -->
<template match="
	TITLE | BODY | ITEM/P.SILENT | ENTRY/P.SILENT |
	SYNOPSIS/P.SILENT | TD/P.SILENT | TH/P.SILENT |
	DEF | SPAN | FIGURE/CAPTION/P.SILENT | GRAMMAR.HEAD |
	CHUNK/TITLE | CHUNK/TITLE/P.SILENT | CHUNK.SILENT |
	NAME | NOTE/P.SILENT | EXERCISE/P.SILENT | ANSWER/P.SILENT |
	REWRITE.FROM/P.SILENT | REWRITE.TO/P.SILENT |
	REWRITE.CONDITION/P.SILENT
">

<!-- processing instructions -->

<template match="processing-instruction()">
  <txt:use>\mozProcessingInstruction{<value-of select="local-name()"/>}</txt:use>
</template>

<!-- format author -->

<template match="AUTHOR">
  <if test="position()>1"><txt:use>\mozPar
</txt:use>
  <apply-templates/>
</templates>

<template match="AUTHOR.EXTERN">
  <if test="position()>1"><txt:use>\mozPar
</txt:use>
  <variable name="info" select="key:get('author',string(@TO))[1]"/>
  <choose>
    <when test="not($info)">
      <if test="msg:say('UNKNOWN AUTHOR: ') and
                msg:saynl(string(@TO))"/>
      <txt:use>\mozUnknownAuthor{<value-of select="@TO"/>}</txt:use>
    </when>
    <otherwise>
      <if test="$info/@firstname">
        <value-of select="$info/@firstname"/>
        <text>, </text>
      </if>
      <value-of select="$info/@lastname"/>
    </otherwise>
  </choose>
</template>

<!-- format a section-like element. we also add a \label command
     if the element has an id -->

<template match="PART|CHAPTER|SECTION|SUBSECTION|SUBSUBSECTION|PARA">
  <!-- an empty line before the new section -->
  <txt:use><text>

\moz</text>
    <value-of select="translate(local-name(),
                                'ABCDEFGHIJKLMNOPQRSTUVWXYZ',
                                'abcdefghijklmnopqrstuvwxyz')"/>
  </txt:use>
  <!-- add a star if the section should be unnumbered -->
  <if test="@CLASS='UNNUMBERED'">
    <txt:use>*</txt:use>
  </if>
  <!-- title argument -->
  <txt:use>{</txt:use>
  <apply-templates select="TITLE | FRONT/TITLE" mode="title"/>
  <txt:use>}</txt:use>
  <!-- add \label if element has id -->
  <call-template name="maybe.label"/>
  <!-- -->
  <apply-templates/>
</template>

<template name="maybe.label">
  <if test="@ID">
    <txt:use>\label{</txt:use>
    <value-of select="@ID"/>
    <txt:use>}\ignorespaces
</txt:use>
  </if>
</template>

<!-- don't process titles unless explicitly requested
     with mode="title" -->

<template match="TITLE"/>
<template match="TITLE" mode="title">
  <apply-templates/>
</template>

<!-- paragraph -->

<template match="P">
  <txt:use><text>

</text></txt:use>
  <call-template name="maybe.label"/>
  <apply-templates/>
</template>

<!-- LIST -->
<!-- 1. simple enumerated list -->

<template match="LIST[@ENUM='ENUM' and not(ENTRY)]">
  <txt:use>\begin{enumerate}
</txt:use>
  <apply-templates mode="simple.items"/>
  <txt:use>\end{enumerate}
</txt:use>
</template>

<template match="ITEM" mode="simple.items">
  <txt:use>\mozItem </txt:use>
  <call-template name="maybe.label"/>
  <apply-templates/>
  <txt:use><text>
</text></txt:use>
</template>

<template match="*" mode="simple.items">
  <call-template name="error.element">
    <with-param name="mode">simple.items</with-param>
  </call-template>
</template>

<!-- 2. simple itemized list -->

<template match="LIST[not(@ENUM) and not(ENTRY)]">
  <txt:use>\begin{itemize}
</txt:use>
  <apply-templates mode="simple.items"/>
  <txt:use>\end{itemize}
</txt:use>
</template>

<!-- 3. enumerated description -->

<template match="LIST[@ENUM='ENUM' and ENTRY]">
  <txt:use>\begin{enumerated.description}
</txt:use>
  <apply-templates mode="description"/>
  <txt:use>\end{enumerated.description}
</txt:use>
</template>

<template match="ENTRY" mode="description">
  <txt:use>\mozEntry{</txt:use>
  <apply-templates/>
  <txt:use>}</txt:use>
  <call-templates name="maybe.label"/>
  <if test=".//CODE|.//MENU">
    <txt:use>\mozEntryHasCode
</txt:use>
  </if>
</template>

<template match="SYNOPSIS" mode="description">
  <txt:use>\begin{synopsis}
</txt:use>
  <apply-templates/>
  <txt:use>\end{synopsis}
</txt:use>
</template>

<template match="ITEM" mode="description">
  <txt:use>\mozItem </txt:use>
  <apply-templates/>
</template>

<template match="*" mode="description">
  <call-template name="error.element">
    <with-param name="mode">description</with-param>
  </call-template>
</template>

<!-- itemized description -->

<template match="LIST[not(@ENUM) and ENTRY]">
  <txt:use>\begin{itemized.description}
</txt:use>
  <apply-templates mode="description"/>
  <txt:use>\end{itemized.description}
</txt:use>
</template>

<!-- menu/mouse -->

<template match="MENU">
  <txt:use>\mozMenu{</txt:use>
  <apply-templates/>
  <txt:use>}{</txt:use>
  <if test="@KEY">
    <txt:use>\mozKey{</txt:use>
    <value-of select="@KEY"/>
    <txt:use>}</txt:use>
  </if>
  <txt:use>}{</txt:use>
  <if test="@MOUSE">
    <txt:use>\mozMouse{</txt:use>
    <value-of select="@MOUSE"/>
    <txt:use>}</txt:use>
  </if>
  <txt:use>}</txt:use>
</template>

<!-- code -->

<template match="CODE[@DISPLAY='INLINE']">
  <txt:use>\mozCodeInline{</txt:use>
  <txt:use name="code">
    <apply-templates/>
  </txt:use>
</template>

<template match="CODE.EXTERN[@DISPLAY='INLINE']">
  <txt:use>\mozCodeInline{</txt:use>
  <txt:use name="code">
    <apply-templates select="key:get('hilite',string(@CODE.ID))"/>
  </txt:use>
  <txt:use>}</txt:use>
</template>

<template match="CODE.TEXT">
  <apply-templates select="key:get('hilite',string(@CODE.ID))"/>
</template>

<template match="CODE[@DISPLAY='DISPLAY']">
  <txt:use>\begin{code.display}
</txt:use>
  <txt:use name="code">
    <apply-templates/>
  </txt:use>
  <txt:use>\end{code.display}
</txt:use>
</template>

<template match="CODE.EXTERN[@DISPLAY='DISPLAY']">
  <txt:use>\begin{code.display}
</txt:use>
  <txt:use name="code">
    <apply-templates select="key:get('hilite',string(@CODE.ID))"/>
  </txt:use>
  <txt:use>\end{code.display}
</txt:use>
</template>

<template match="HILITE.FACE">
  <txt:use>\mozFace<value-of select="@NAME"/>{</txt:use>
  <apply-templates/>
  <txt:use>}</txt:use>
</template>

<template match="CODE/SPAN[@CLASS='IGNORE']"/>

<!-- variables -->

<template match="VAR">
  <if test="@TYPE='PROG' and @MODE">
    <txt:use>\mozMode<value-of select="@MODE"/> </value-of>
  </if>
  <txt:use>\mozVar<value-of select="@TYPE"/>{</txt:use>
  <apply-templates/>
  <txt:use>}</txt:use>
</template>

<!-- miscellaneous, usually inline, commands -->

<template match="FILE|SAMP|EM|KBD|KEY|Q">
  <variable name="kind">
    <choose>
      <when test="@DISPLAY='DISPLAY'">display</when>
      <otherwise>inline</otherwise>
    </choose>
    <text>.</text>
    <value-of select="local-name()"/>
  </variable>
  <txt:use>\begin{<value-of select="$kind"/>}</txt:use>
  <apply-templates/>
  <txt:use>\end{<value-of select="$kind"/>}</txt:use>
</template>

<!-- external references
     REF.EXTERN is converted to a footnote
     PTR.EXTERN is formatted inline
     when attribute TO begins with the string "ozdoc:", then
     this is a reference to another book in the mozart
     documentation and we format specially by looking it up
     in the global database of reference points.
  -->

<template match="REF.EXTERN">
  <apply-templates/>
  <if test="ancestor::TITLE">
    <txt:use>\protect</txt:use>
  </if>
  <txt:use>\footnote{</txt:use>
  <apply-templates select="." mode="ref.extern"/>
  <txt:use>}</txt:use>
</template>

<template match="PTR.EXTERN">
  <apply-templates select="." mode="ref.extern"/>
</template>

<template name="ref.extern">
  <choose>
    <!-- 1. ref to other mozart doc -->
    <when test="starts-with(string(@TO),'ozdoc:')">
      <variable name="xref.doc"
		select="key:get('xref',string(@TO))[1]"/>
      <choose>
        <!-- not in global database of ref points -->
        <when test="not($xref.doc)">
          <if test="msg:say('UNKNOWN OZDOC DOCUMENT: ') and
                    msg:saynl(string(@TO))"/>
          <txt:use>\mozXrefUnknown{</txt:use>
          <value-of select="@TO"/>
          <txt:use>}</txt:use>
        </when>
        <!-- ref to section in doc -->
        <when test="@KEY">
          <variable name="xref.sec"
		    select="key:get(string(@TO),string(@KEY))[1]"/>
          <choose>
            <!-- section not found in global db -->
            <when test="not($xref.sec)">
              <if test="msg:say('UNKNOWN OZDOC SECTION: ') and
                        msg:say(string(@KEY)) and
                        msg:say(' IN DOCUMENT ') and
                        msg:saynl(string(@TO))"/>
              <txt:use>\mozXrefUnknown{</txt:use>
              <value-of select="@KEY"/>
              <text> in </text>
              <value-of select="@TO"/>
              <txt:use>}</txt:use>
            </when>
            <!-- got document and section info from db -->
            <otherwise>
              <txt:use>\mozXrefSec</txt:use>
              <apply-templates select="$xref.sec"/>
              <apply-templates select="$xref.doc"/>
            </otherwise>
          </choose>
        </when>
        <!-- ref to doc as a whole -->
        <otherwise>
          <txt:use>\mozXrefDoc</txt:use>
          <apply-templates select="$xref.sec"/>
        </otherwise>
      </choose>
    </when>
    <!-- 2. arbitrary URL -->
    <otherwise>
      <txt:use>\mozXrefDefault{</txt:use>
      <value-of select="@TO"/>
      <txt:use>}</txt:use>
    </otherwise>
  </choose>
</template>

<template match="XREF.DOC">
  <txt:use>{</txt:use>
  <apply-templates select="TITLE" mode="title"/>
  <txt:use>}</txt:use>
</template>

<template match="XREF.SEC">
  <txt:use>{</txt:use>
  <value-of select="@TYPE"/>
  <txt:use>}{</txt:use>
  <apply-templates select="TITLE" mode="title"/>
  <txt:use>}</txt:use>
</template>

<!-- internal references -->

<template match="PTR">
  <call-template name="ref.intern">
    <with-param name="kind">Ptr</with-param>
  </call-template>
</template>

<template match="REF">
  <call-template name="ref.intern">
    <with-param name="kind">Ref</with-param>
  </call-template>
</template>

<template name="ref.intern">
  <param name="kind"/>
  <variable name="to" select="key:get('id',string(@TO))[1]"/>
  <choose>
    <when test="not($to)">
      <if test="msg:say('UNKNOWN INTERNAL REF: ') and
                msg:saynl(string(@TO))"/>
      <txt:use>\mozRefUnknown{</txt:use>
      <value-of select="@TO"/>
      <txt:use>}</txt:use>
    </when>
    <otherwise>
      <txt:use>\mozRef<value-of select="$kind"/></txt:use>
      <apply-templates select="$to" mode="ref.intern"/>
    </otherwise>
  </choose>
</template>

<template match="PART|CHAPTER|APPENDIX|SECTION|SUBSECTION|SUBSUBSECTION"
	  mode="ref.intern">
  <txt:use>{</txt:use>
  <value-of select="local-name()"/>
  <txt:use>}{</txt:use>
  <value-of select="@ID"/>
  <txt:use>}{</txt:use>
  <apply-templates select="FRONT/TITLE" mode="title"/>
  <txt:use>}</txt:use>
</template>

<template match="PARA|ENTRY|P|CHUNK|FIGURE|ITEM" mode="ref.intern">
  <txt:use>{</txt:use>
  <value-of select="local-name()"/>
  <txt:use>}{</txt:use>
  <value-of select="@ID"/>
  <txt:use>}{}</txt:use>
</template>

<template match="BIB.EXTERN" mode="ref.intern">
  <txt:use>{</txt:use>
  <value-of select="local-name()"/>
  <txt:use>}{</txt:use>
  <value-of select="@KEY"/>
  <txt:use>}{}</txt:use>
</template>

<template match="*" mode="ref.intern" priority="-1.0"/>
  <call-template name="error.element">
    <with-param name="mode">ref.intern</with-param>
  </call-template>
</template>
