<!-- -*-xml-*- -->

<stylesheet xmlns="http://www.w3.org/XSL/Transform/1.0">

<template match="text()"/>

<template match="BOOK" priority="2.0">
  <element name="OZDOC.BOOK" namespace="">
    <copy-of select="FRONT/TITLE[1]"/>
    <text>
</text>
    <apply-templates select="BODY"/>
    <text>
</text>
  </element>
</template>

<template match="PART|CHAPTER|APPENDIX|SECTION|SUBSECTION|SUBSUBSECTION" priority="2.0">
  <if test="@ID">
    <element name="OZDOC.ENTRY" namespace="">
      <attribute name="TYPE">
        <value-of select="local-part(.)"/>
      </attribute>
      <attribute name="KEY">
        <value-of select="@ID"/>
      </attribute>
      <copy-of select="FRONT/TITLE[1]"/>
    </element>
    <text>
</text>
  </if>
  <apply-templates/>
</template>

<template match="*[@ID]">
  <variable name="key" select="@ID"/>
  <variable name="subtype" select="local-part(.)"/>
  <for-each select="ancestor::*[self::PART
                               |self::CHAPTER
                               |self::APPENDIX
                               |self::SECTION
                               |self::SUBSECTION
                               |self::SUBSUBSECTION][1]">
    <element name="OZDOC.ENTRY" namespace="">
      <attribute name="TYPE">
        <value-of select="local-part(.)"/>
      </attribute>
      <attribute name="KEY">
        <value-of select="$key"/>
      </attribute>
      <attribute name="SUBTYPE">
        <value-of select="$subtype"/>
      </attribute>
      <copy-of select="FRONT/TITLE[1]"/>
    </element>
    <text>
</text>
  </for-each>
  <apply-templates/>
</template>

</stylesheet>
