<!-- -*-xml-*- -->

<stylesheet xmlns="http://www.w3.org/XSL/Transform/1.0"
	xmlns:nxml="java:com.jclark.xsl.sax.NXMLOutputHandler"
	result-ns="nxml">

<strip-space elements="
	BOOK FRONT BACK BODY
	PART CHAPTER SECTION SUBSECTION SUBSUBSECTION APPENDIX
	LIST ENTRY SYNOPSIS
	MATH.CHOICE PICTURE.CHOICE
	CHUNK FIGURE INDEX SEE
	GRAMMAR.RULE GRAMMAR
	TABLE TR"/>

<template match="/">
  <nxml:nxml>
    <nxml:escape char='"'>\"</nxml:escape>
    <nxml:escape char="\">\\</nxml:escape>
    <call-template name="emacs.packages"/>
    <apply-templates/>
  </nxml:nxml>
</template>

<template name="emacs.packages">
  <for-each select="/BOOK/FRONT/META[@NAME='EMACS.PACKAGE' and @VALUE]">
    <nxml:control>
      <text>(load "</text>
      <nxml:data><value-of select="@VALUE"/></nxml:data>
      <text>")
</text>
    </nxml:control>
  </for-each>
</template>

<template match="META[@NAME='PROGLANG.MODE' and @ARG1 and @ARG2]">
  <nxml:control>
    <text>(ozdoc-declare-mode "</text>
    <nxml:data><value-of select="@ARG1"/></nxml:data>
    <text>" '</text>
    <value-of select="@ARG2"/>
    <text>)
</text>
  </nxml:control>
</template>

<!-- ignore text, except in mode="ok" -->
<template match="text()"/>
<template match="text()" mode="ok">
  <nxml:data><value-of select="."/></nxml:data>
</template>

<template match="CHUNK.REF|VAR[@TYPE!='PROG']" mode="ok">
  <nxml:control>(nil . " X ")
</nxml:control>
</template>

<template match="HILITE.MODE">
  <nxml:control>
    <text>(ozdoc-fontify-alist "</text>
    <value-of select="@PROGLANG"/>
    <text>" '(</text>
    <apply-templates mode="ok"/>
    <text>))
</text>
  </nxml:control>
</template>

<template match="HILITE.FILE">
  <nxml:control>
    <text>(ozdoc-fontify-file "</text>
    <value-of select="@PROGLANG"/>
    <text>" '</text>
    <value-of select="@ID"/>
    <text> "</text>
    <value-of select="@FILE"/>
    <text>")
</text>
  </nxml:control>
</template>

<template match="HILITE.ITEM" mode="ok">
  <nxml:control>(<value-of select="@ID"/> . "</nxml:control>
  <nxml:data><value-of select="."/></nxml:data>
  <nxml:control>")
</nxml:control>
</template>

</stylesheet>
