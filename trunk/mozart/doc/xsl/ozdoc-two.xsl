<!-- -*-xml-*- -->

<stylesheet
 xmlns		= "http://www.w3.org/XSL/Transform/1.0"
 xmlns:txt	= "http://www.jclark.com/xt/java"
 extension-element-prefixes="txt"
 result-ns	= ""
 xmlns:moz	= "http://www.jclark.com/xt/java/Mozart"
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
  <if test="moz:put('author',string(@key),.)"/>
</template>

<!-- xref table -->
<template name="build.table.xref">
  <apply-templates select="document($XREF_FILE)" mode="build.table.xref"/>
</template>

<template match="text()" mode="build.table.xref"/>
<template match="xref.document" mode="build.table.xref">
  <variable name="key">ozdoc:<value-of select="name"/></variable>
  <for-each select="xref.entry">
    <if test="moz:put('xref',string(@key),.)"/>
  </for-each>
</template>

<!-- hilite table -->
<template match="build.table.hilite">
  <apply-templates select="document($HILITE_FILE)" mode="build.table.hilite"/>
</template>

<template match="text()" mode="build.table.hilite"/>
<template match="HILITE" mode="build.table.hilite">
  <if test="moz:put('hilite',string(@ID),.)"/>
</template>

<!-- meta, id, and code.id tables
     as long as we are going through the whole document
     we should also record such info as whether there are
     any <index> elements.
  -->
<template match="build.table.local">
  <apply-templates mode="build.table.local"/>
</template>

<template match="text()" mode="build.table.local"/>
<template match="*[@ID or @CODE.ID]" mode="build.table.local">
  <if test="@ID and moz:put('id',string(@ID),.)"/>
  <if test="@CODE.ID and moz:put('code.id',string(@CODE.ID),.)"/>
  <if test="local-name()='INDEX' and moz:sset('has.index','yes')"/>
  <apply-templates mode="build.table.local"/>
</template>
<template match="META" mode="build.table.local">
  <choose>
    <when test="@NAME='LATEX.TABLE.SPEC' and @ARG1 and @ARG2">
      <if test="moz:ssput('latex.table.spec',string(@ARG1),string(@ARG2))"/>
    </when>
    <when test="@NAME='LATEX.PICTURE.WIDTH' and @ARG1 and @ARG2">
      <if test="moz:ssput('latex.picture.width',string(@ARG1),string(@ARG2))"/>
    </when>
  </choose>
</template>

<!-- escape maps -->

<template name="declare.maps">
  <!-- all LaTeX special characters must be escaped
       we must also escape newline because otherwise 2 in a row
       mean \par.
    -->
  <txt:define name="text">
    <txt:escape char="#">\mozHash </txt:escape>
    <txt:escape char="$">\mozDollar </txt:escape>
    <txt:escape char="%">\mozPercent </txt:escape>
    <txt:escape char="%amp;">\mozAmpersand </txt:escape>
    <txt:escape char="~">\mozTilde </txt:escape>
    <txt:escape char="_">\mozUnderscore </txt:escape>
    <txt:escape char="^">\mozCaret </txt:escape>
    <txt:escape char="\">\mozBackslash </txt:escape>
    <txt:escape char="{{">\mozLeftbrace </txt:escape>
    <txt:escape char="}}">\mozRightbrace </txt:escape>
    <txt:escape char="&lt;">\mozLessthan </txt:escape>
    <txt:escape char="&gt;">\mozGreaterthan </txt:escape>
    <txt:escape char="|">\mozVbar </txt:escape>
    <txt:escape char="&#10;">\mozNewline
</txt:escape>
  </txt:define>
  <!-- code escaping is the same except that space is
       also escaped because it must be preserved for
       proper indentation
    -->
  <txt:define name="code">
    <txt:include name="text"/>
    <txt:escape char=" ">\mozSpace </txt:escape>
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
       \index{...}.  we will use <alias> to alis text and
       code to index.text and index.code while processing
       <index> elements
    -->
  <txt:define name="index.text">
    <txt:include name="text"/>
    <txt:include name="index.common"/>
  </txt:define>
  <txt:define name="index.code">
    <txt:include name="code"/>
    <txt:include name="index.common"/>
  </txt:define>
</template>

<template match="BOOK">
  <txt:use>\documentclass{mozart}
</txt:use>
  <if test="moz:shaskey('has.index')">
    <txt:use>\mozDoindex
</txt:use>
  <apply-templates select="FRONT"/>
  <txt:use>\begin{document}
</txt:use>
  <apply-templates select="BODY"/>
<!-- for the answers to exercises, clearly we would like
     to keep a list of these answers that we accumulate
     during the pretraversal of the document and then
     process in document order at the end -->
  <call-template 