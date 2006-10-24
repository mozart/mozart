<!-- -*-xml-*- -->

<stylesheet
  xmlns		="http://www.w3.org/XSL/Transform/1.0"
  xmlns:id	="http://www.jclark.com/xt/java/ID">

<strip-space elements="
	BOOK FRONT BACK BODY
	PART CHAPTER SECTION SUBSECTION SUBSUBSECTION APPENDIX
	LIST ENTRY SYNOPSIS
	MATH.CHOICE PICTURE.CHOICE
	CHUNK FIGURE INDEX SEE
	GRAMMAR.RULE GRAMMAR
	TABLE TR
	HILITE.BOOK HILITE.PREFACE HILITE.MODE HILITE"/>

<template match="/">
  <!-- build table mapping ID to HILITE element from
       the preface HILITE.PREFACE -->
  <for-each select="/HILITE.BOOK/HILITE.PREFACE/HILITE">
    <if test="id:put(string(@ID),.)"/>
  </for-each>
  <apply-templates select="HILITE.BOOK/BOOK"/>
</template>

<template match="@*|*|text()|processing-instruction()">
  <copy>
    <apply-templates select="@*|*|text()|processing-instruction()"/>
  </copy>
</template>

<template match="HILITE.MODE">
  <apply-templates select="@*|*|text()|processing-instruction()"/>
</template>

<template match="HILITE.ITEM|HILITE.FILE">
  <apply-templates select="id:get(string(@ID))"/>
</template>

</stylesheet>
