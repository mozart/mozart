<!-- -*-xml-*- -->

<stylesheet xmlns="http://www.w3.org/XSL/Transform/1.0">

<strip-space elements="
	BOOK FRONT BACK BODY
	PART CHAPTER SECTION SUBSECTION SUBSUBSECTION APPENDIX
	LIST ENTRY SYNOPSIS
	MATH.CHOICE PICTURE.CHOICE
	CHUNK FIGURE INDEX SEE
	GRAMMAR.RULE GRAMMAR
	TABLE TR"/>

<template match="text()"/>

<template match="PICTURE.CHOICE">
  <choose>
    <when test="PICTURE[@TYPE='LATEX']"/>
    <when test="PICTURE.EXTERN[@TYPE='LATEX' or @TYPE='PS']"/>
    <when test="PICTURE.EXTERN[@TYPE='GIF']">
      <apply-templates select="PICTURE.EXTERN[@TYPE='GIF'][1]"/>
    </when>
  </choose>
</template>

<template match="PICTURE.EXTERN[@TYPE='GIF']">
  <value-of select="@TO"/>
  <text>
</text>
</template>

<template match="/|*">
  <apply-templates select="*"/>
</template>

</stylesheet>
