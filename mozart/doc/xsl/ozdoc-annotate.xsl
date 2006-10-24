<!-- -*-xml-*- -->

<stylesheet xmlns="http://www.w3.org/XSL/Transform/1.0"
	    xmlns:meta="http://www.jclark.com/xt/java/Meta">

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

<!-- need to thread same-titled chunks.  all chunks with same title -->
<!-- are given the same CHUNK.ID and a different CHUNK.NUM which is an -->
<!-- integer starting at 1 and incremented each time -->

<template match="CHUNK">
  <copy>
    <apply-templates select="@*"/>
    <variable name="title">
      <value-of select="TITLE"/>
    </variable>
    <if test="not(meta:chunkExists(string($title)))">
      <if test="meta:chunkRegister(generate-id(),string($title))"/>
    </if>
    <attribute name="CHUNK.ID">
      <value-of select="meta:chunkGetID(string($title))"/>
    </attribute>
    <attribute name="CHUNK.NUM">
      <value-of select="meta:chunkGetNUMInc(string($title))"/>
    </attribute>
    <apply-templates select="*|text()|processing-instruction()"/>
  </copy>
</template>

<template match="CHUNK.REF" mode="ok">
  <copy>
    <apply-templates select="@*"/>
    <variable name="title">
      <value-of select="."/>
    </variable>
    <if test="not(meta:chunkExists(string($title)))">
      <if test="meta:chunkRegister(generate-id(),string($title))"/>
    </if>
    <attribute name="CHUNK.ID">
      <value-of select="meta:chunkGetID(string($title))"/>
    </attribute>
    <apply-templates select="*|text()|processing-instruction()"/>
  </copy>
</template>

<template match="CODE|CHUNK.SILENT|VAR[@TYPE='PROG']">
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

<template match="VAR[@TYPE!='PROG']" mode="ok">
  <copy-of select="."/>
</template>

</stylesheet>
