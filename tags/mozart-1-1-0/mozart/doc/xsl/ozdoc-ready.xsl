<!-- -*-xml-*- -->

<stylesheet xmlns="http://www.w3.org/XSL/Transform/1.0">

<template match="/">
  <apply-templates select="OZDOC.READY/BOOK"/>
</template>

<template match="@*|*|text()|processing-instruction()">
  <copy>
    <apply-templates select="@*|*|text()|processing-instruction()"/>
  </copy>
</template>

<template match="BOOK">
  <copy>
    <apply-templates select="@*|*|text()|processing-instruction()"/>
    <apply-templates select="/OZDOC.READY/OZDOC.DB"/>
  </copy>
</template>

</stylesheet>