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

<template match="@*|*|text()|processing-instruction()">
  <copy>
    <apply-templates select="@*|*|text()|processing-instruction()"/>
  </copy>
</template>

<template match="CODE">
  <element name="HILITE.MODE" namespace="">
    <attribute name="PROGLANG">
      <value-of select="ancestor-or-self::*[@PROGLANG][1]/@PROGLANG"/>
    </attribute>
    <apply-templates select="." mode="ok"/>
  </element>
</template>

<template match="@*|*|processing-instruction()" mode="ok">
  <copy>
    <apply-templates select="@*|*|text()|processing-instruction()" mode="ok"/>
  </copy>
</template>

<template match="text()" mode="ok">
  <element name="HILITE" namespace="">
    <attribute name="ID">
      <value-of select="generate-id(.)"/>
    </attribute>
    <value-of select="."/>
  </element>
</template>

</stylesheet>
