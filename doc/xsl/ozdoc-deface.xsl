<!-- -*-xml-*- -->

<stylesheet xmlns="http://www.w3.org/XSL/Transform/1.0">

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
  <variable name="id" select="@ID"/>
  <apply-templates select="/HILITE.BOOK/HILITE.PREFACE/HILITE[@ID=$id]"/>
</template>

</stylesheet>
