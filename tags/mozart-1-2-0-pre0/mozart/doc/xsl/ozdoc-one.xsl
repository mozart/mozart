<!-- -*-xml-*- -->

<!-- This stylesheet performs several tasks and creates several
     files for subsequent stages of processing.

     1. it extracts a script to create pictures from gifs when
        necessary

     2. it extracts a file of elisp to create code hilighting
        annotations.

     3. it outputs a copy of the input where code has been
        annotated to permit substitution by hilighted code
        in a later processing stage.
-->

<stylesheet
 xmlns		= "http://www.w3.org/XSL/Transform/1.0"
 xmlns:xt	= "http://www.jclark.com/xt"
 xmlns:txt	= "http://www.jclark.com/xt/java"
 extension-element-prefixes="xt txt"
 result-ns      = ""
>

<!-- for elements that only contain other elements but never data,
     all spaces should be stripped away -->

<strip-space elements="
	BOOK FRONT BACK BODY PART CHAPTER SECTION SUBSECTION
	SUBSUBSECTION APPENDIX LIST ENTRY SYNOPSIS MATH.CHOICE
	PICTURE.CHOICE CHUNK FIGURE INDEX SEE GRAMMAR.RULE
	GRAMMAR TABLE TR"/>

<!-- this is the basename of the main document -->

<param name="basename">MissingBasename</param>

<!-- process the document -->

<template match="/">
  <xt:document href="{$basename}.pics" method="txt:MapOutputHandler">
    <txt:document>
      <apply-templates select="/" mode="gif"/>
    </txt:document>
  </xt:document>
  <xt:document href="{$basename}.el" method="txt:MapOutputHandler">
    <txt:document>
      <call-template name="declare.escape.maps"/>
      <apply-templates select="/" mode="elisp"/>
    </txt:document>
  </xt:document>
  <apply-templates/>
</template>

<template name="declare.escape.maps">
  <call-template name="declare.escape.map.elisp.string"/>
</template>

<!-- PICTURE CONVERSION

     go over the document and extract the names of gif files
     that must be converted to postscript. this is realized
     by templates in mode="gif" -->

<template match="text()" mode="gif"/>

<template match="PICTURE.CHOICE" mode="gif">
  <choose>
    <when test="PICTURE[@TYPE='LATEX']"/>
    <when test="PICTURE.EXTERN[@TYPE='LATEX' or @TYPE='PS']"/>
    <when test="PICTURE.EXTERN[@TYPE='GIF']">
      <apply-templates select="PICTURE.EXTERN[@TYPE='GIF'][1]" mode="gif"/>
    </when>
  </choose>
</template>

<template match="PICTURE.EXTERN[@TYPE='GIF']" mode="gif">
  <value-of select="@TO"/>
  <text>
</text>
</template>

<!-- CODE HIGHLIGHTING

     go over the document and extract code to be highlighted.
     produce elisp code to compute the highlighting.  this is
     realized by templates in mode="elisp" -->

<template name="declare.escape.map.elisp.string">
  <!-- in elisp strings, " and \ need to be escaped -->
  <txt:define name="elisp.string">
    <txt:escape char='"'>\"</txt:escape>
    <txt:escape char="\">\\</txt:escape>
  </txt:define>
</template>

<template match="/" mode="elisp">
  <txt:use name="elisp.string">
    <apply-templates mode="elisp"/>
  </txt:use>
</template>

<template match="META[@NAME='EMACS.PACKAGE' and @VALUE]" mode="elisp">
  <txt:use>(load "</txt:use>
  <value-of select="@VALUE"/>
  <txt:use>")
</txt:use>
</template>

<template match="META[@NAME='PROGLANG.MODE' and @ARG1 and @ARG2]" mode="elisp">
  <txt:use>(ozdoc-declare-mode "</txt:use>
  <value-of select="@ARG1"/>
  <text>" '</text>
  <value-of select="@ARG2"/>
  <text>)
</text>
</template>

<template match="text()" mode="elisp"/>

<template match="CODE|CHUNK.SILENT|VAR[@TYPE='PROG']" mode="elisp">
  <txt:use>(ozdoc-fontify-alist "</txt:use>
  <value-of select="ancestor-or-self::*[@PROGLANG][1]/@PROGLANG"/>
  <txt:use>" '(</txt:use>
  <apply-templates mode="elisp.copy"/>
  <txt:use>))
</txt:use>
</template>

<template match="CODE.EXTERN" mode="elisp">
  <txt:use>(ozdoc-fontify-file "</txt:use>
  <value-of select="ancestor-or-self::*[@PROGLANG][1]/@PROGLANG"/>
  <txt:use>" '</txt:use>
  <value-of select="generate-id()"/>
  <txt:use> "</txt:use>
  <value-of select="@TO"/>
  <txt:use>")
</txt:use>
</template>

<template match="text()" mode="elisp.copy">
  <txt:use>(<value-of select="generate-id()"/> . "</txt:use>
  <value-of select="."/>
  <txt:use>")
</txt:use>
</template>

<template match="CHUNK.REF|VAR[@TYPE!='PROG']" mode="elisp.copy">
  <txt:use>(nil . " X ")
</txt:use>
</template>

<!-- DOCUMENT ANNOTATING

     the document is now essentially copied to the output, except
     that code text is replaced by elements with an appropriate
     id to permit substitution by highlighted code. -->

<template match="*|@*|text()|processing-instruction()">
  <copy>
    <apply-templates select="@*|node()"/>
  </copy>
</template>

<template match="CODE.EXTERN">
  <copy>
    <attribute name="CODE.ID">
      <value-of select="generate-id()"/>
    </attribute>
  </copy>
</template>

<template match="CODE|CHUNK.SILENT|VAR[@TYPE='PROG']" priority="2.0">
  <copy>
    <apply-templates select="@*|node()" mode="hilite"/>
  </copy>
</template>

<template match="*|@*|processing-instruction()" mode="hilite">
  <copy>
    <apply-templates select="@*|node()"/>
  </copy>
</template>

<template match="text()" mode="hilite">
  <!-- replace text node for subsequent substitution
       by hilighted code -->
  <element name="CODE.TEXT" namespace="">
    <attribute name="CODE.ID">
      <value-of select="generate-id()"/>
    </attribute>
  </element>
</template>

<template match="CHUNK.REF|VAR[@TYPE!='PROG']" mode="hilite" priority="2.0">
  <!-- just copy the contents without replacing text nodes for
       hilighting -->
  <copy>
    <apply-templates select="@*|node()"/>
  </copy>
</template>

</stylesheet>
