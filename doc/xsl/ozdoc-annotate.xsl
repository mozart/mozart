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

<template match="CODE|CHUNK.SILENT">
  <element name="HILITE.MODE" namespace="">
    <attribute name="PROGLANG">
      <value-of select="ancestor-or-self::*[@PROGLANG][1]/@PROGLANG"/>
    </attribute>
    <apply-templates select="." mode="ok"/>
  </element>
</template>

<template match="CODE.EXTERN">
  <element name="CODE" namespace="">
    <apply-templates select="@*"/>
    <element name="HILITE.FILE" namespace="">
      <attribute name="PROGLANG">
        <value-of select="ancestor-or-self::*[@PROGLANG][1]/@PROGLANG"/>
      </attribute>
      <attribute name="FILE">
        <value-of select="@TO"/>
      </attribute>
      <attribute name="ID">
        <value-of select="generate-id(.)"/>
      </attribute>
    </element>
  </element>
</template>

<template match="@*|*|processing-instruction()" mode="ok">
  <copy>
    <apply-templates select="@*|*|text()|processing-instruction()" mode="ok"/>
  </copy>
</template>

<template match="text()" mode="ok">
  <element name="HILITE.ITEM" namespace="">
    <attribute name="ID">
      <value-of select="generate-id(.)"/>
    </attribute>
    <value-of select="."/>
  </element>
</template>

<template match="CHUNK.REF" mode="ok">
  <copy-of select="."/>
</template>

</stylesheet>
