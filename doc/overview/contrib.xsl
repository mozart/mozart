<xsl:transform version="1.0"
	xmlns:xsl="http://www.w3.org/1999/XSL/Transform">

<xsl:strip-space elements="database topic package"/>
<xsl:output method="html"/>

<xsl:template match="/">
  <html>
    <head>
      <title>Mozart User Contributions Archive</title>
      <link href="ozdoc.css" rel="stylesheet" type="text/css"/>
      <style type="text/css">
<xsl:text>
span.package {
  font-weight:bold;
  color:steelblue;
}
span.import {
  font-size:small;
  color:steelblue;
}
</xsl:text>
      </style>
    </head>
    <body>
      <p class="margin"><a href="../">Top</a></p>
      <h1 align="center" class="title">User Contribution Archive</h1>
      <p>This is a page where users can lookup Oz packages contributed by
others, and register new ones.  In order to submit a package for
inclusion, send email to <a href="mailto:contrib@mozart-oz.org"
><tt>contrib@mozart-oz.org</tt></a> with the relevant info (see
examples below).  If you wish to develop a new contribution, check out
the <tt>ozskel</tt> script offered below.  It will get you started
fast and make your task easier.</p>
      <hr/>
      <xsl:apply-templates select="/database/topic"/>
      <hr/>
      <address>
        <a href="mailto:contrib@mozart-oz.org">
          <tt>contrib@mozart-oz.org</tt>
        </a>
      </address>
    </body>
  </html>
</xsl:template>

<xsl:template match="a|tt">
  <xsl:apply-templates select="." mode="copy"/>
</xsl:template>

<xsl:template match="@*|*|text()" mode="copy">
  <xsl:copy>
    <xsl:apply-templates select="@*|node()" mode="copy"/>
  </xsl:copy>
</xsl:template>

<xsl:template match="topic">
  <h2><xsl:apply-templates select="title"/></h2>
  <xsl:variable name="ID" select="string(@id)"/>
  <dl>
    <xsl:for-each select="/database/package[topic[1][@id=$ID]]">
      <dt>
        <span class="package">
          <xsl:apply-templates select="title"/>
        </span>
        <br/>
        <xsl:if test="doc">
          <xsl:text> [</xsl:text>
          <a href="{doc/@href}">documentation</a>
          <xsl:text>]</xsl:text>
        </xsl:if>
        <xsl:if test="src">
          <xsl:text> [</xsl:text>
          <a href="{src/@href}">download</a>
          <xsl:text>]</xsl:text>
        </xsl:if>
        <xsl:for-each select="author">
          <xsl:variable name="REF" select="string(@id)"/>
          <xsl:for-each select="/database/author[@id=$REF]">
            <xsl:text> [author: </xsl:text>
            <a href="{@www|@email}">
              <xsl:value-of select="@firstname"/>
              <xsl:text> </xsl:text>
              <xsl:value-of select="@lastname"/>
            </a>
            <xsl:text>]</xsl:text>
          </xsl:for-each>
        </xsl:for-each>
        <xsl:for-each select="import">
          <br/>
          <span class="import"><xsl:value-of select="@href"/></span>
        </xsl:for-each>
      </dt>
      <dd>
        <xsl:apply-templates select="abstract"/>
        <p/>
      </dd>
    </xsl:for-each>
  </dl>
</xsl:template>

</xsl:transform>