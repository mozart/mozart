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

<template match="PICTURE.EXTERN[@TYPE='GIF']">
  <value-of select="@TO"/>
  <text>
</text>
</template>

<template match="/|*">
  <apply-templates select="*"/>
</template>

</stylesheet>
