<!-- -*-xml-*- -->

<stylesheet xmlns="http://www.w3.org/XSL/Transform/1.0">

<template match="text()"/>

<template match="BOOK">
  <element name="OZDOC.BOOK" namespace="">
    <copy-of select="FRONT/TITLE[1]"/>
    <text>
</text>
    <apply-templates select="BODY"/>
    <text>
</text>
  </element>
</template>

<template match="PART|CHAPTER|APPENDIX|SECTION|SUBSECTION|SUBSUBSECTION">
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

</stylesheet>
