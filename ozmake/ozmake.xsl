<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
		xmlns="http://www.w3.org/TR/REC-html40"
		result-ns="">

<xsl:output method="html"/>
<xsl:strip-space elements="
    package head author abstract section dlist item
    align row
"/>

<xsl:template match="/package">
  <HTML>
    <HEAD>
      <TITLE><xsl:call-template name="title"/></TITLE>
      <STYLE><xsl:text>
BODY {
	background-color: white;
	margin-left	: 2cm;
	margin-right	: 2cm;
	font-family	: tahoma,arial,helvetica,sans-serif;
}
H1 {
	text-align	: center;
	color		: #9B0000;
}
H2 {	color		: #FF9933; }
H4 {	color		: slateblue; }
H3 {	color		: #881155; }
H5 {	color		: darkslateblue; }
CODE, SPAN.CMD {	color		: #663366; }
CODE,TT,PRE,SPAN.CMD,SPAN.FILE {
	font-family	: "lucida console",courier,monospace;
}
SPAN.FILE {
	color:maroon;
	white-space:pre;
}
CODE.DISPLAY {
	display		: block;
	white-space	: pre;
	margin-left	: 2cm;
	margin-top	: 1em;
	margin-bottom	: 1em;
}
SPAN.CMD {
	white-space	: pre;
}
P.BLURB {
	text-align	: center;
	font-style	: italic;
	margin-left	: 3cm;
	margin-right	: 3cm;
}
P.AUTHOR {
	text-align	: center;
	font-weight	: bold;
}
SPAN.MODULE {
	color		: steelblue;
}
A {	color		: steelblue; }
SPAN.TOOL {
  font-family:"lucida console",courier,monospace;
  color:steelblue;
}
SPAN.META {
  font-style:italic;
  font-family:"lucida console",courier,monospace;
  color:steelblue;
}
SPAN.DEFAULT {
  font-weight: bold;
  font-family:"lucida console",courier,monospace;
  color:gray; /*steelblue*/;
  margin-left: 1cm;
}
P.WARNING {
  color:red;
  font-weight:bold;
  text-align:center;
}
SPAN.COMMENT      { color: #B22222; }
SPAN.KEYWORD      { color: #A020F0; }
SPAN.STRING       { color: #BC8F8F; }
SPAN.FUNCTIONNAME { color: #0000FF; }
SPAN.TYPE         { color: #228B22; }
SPAN.VARIABLENAME { color: #B8860B; }
SPAN.REFERENCE    { color: #5F9EA0; }
SPAN.BUILTIN      { color: #DA70D6; }
TABLE.ALIGN {
	margin-left : 1cm;
}
</xsl:text>
      </STYLE>
    </HEAD>
    <BODY>
      <H1><xsl:call-template name="title"/></H1>
      <P CLASS="AUTHOR">
        <xsl:for-each select="head/author">
	  <xsl:if test="position()>1">, </xsl:if>
	  <xsl:apply-templates select="."/>
	</xsl:for-each>
      </P>
      <xsl:apply-templates select="head/blurb"/>
      <HR/>
      <xsl:apply-templates select="*[not(self::head)]"/>
    </BODY>
  </HTML>
</xsl:template>

<xsl:template name="title">
  <xsl:apply-templates select="title/node()|head/title/node()"/>
</xsl:template>

<xsl:template name="nonhead">
  <xsl:apply-templates select="*[not(self::title) and not(self::head)]"/>
</xsl:template>

<xsl:template match="author">
  <A>
    <xsl:choose>
      <xsl:when test="www">
        <xsl:attribute name="HREF"><xsl:value-of select="www"/></xsl:attribute>
      </xsl:when>
      <xsl:when test="email">
        <xsl:attribute name="HREF">mailto:<xsl:value-of select="email"/></xsl:attribute>
      </xsl:when>
    </xsl:choose>
    <xsl:apply-templates select="name/node()"/>
  </A>
</xsl:template>

<xsl:template match="blurb">
  <P CLASS="BLURB">
    <xsl:apply-templates/>
  </P>
</xsl:template>

<xsl:template match="tool">
  <SPAN CLASS="TOOL">
    <xsl:apply-templates/>
  </SPAN>
</xsl:template>

<xsl:template match="section/section/section/section">
  <H5>
    <xsl:call-template name="title"/>
  </H5>
  <xsl:call-template name="nonhead"/>
</xsl:template>

<xsl:template match="section/section/section">
  <H4>
    <xsl:call-template name="title"/>
  </H4>
  <xsl:call-template name="nonhead"/>
</xsl:template>

<xsl:template match="section/section">
  <H3>
    <xsl:call-template name="title"/>
  </H3>
  <xsl:call-template name="nonhead"/>
</xsl:template>

<xsl:template match="section">
  <H2>
    <xsl:call-template name="title"/>
  </H2>
  <xsl:call-template name="nonhead"/>
</xsl:template>

<!-- dlist -->

<xsl:template match="dlist">
  <DL>
    <xsl:apply-templates/>
  </DL>
</xsl:template>

<xsl:template match="dlist[@type='options']/item">
  <DT><xsl:apply-templates select="label" mode="options"/></DT>
  <DD><xsl:call-template name="item"/></DD>
</xsl:template>

<xsl:template match="dlist[not(@type='options')]/item">
  <xsl:apply-templates select="label"/>
  <DD><xsl:call-template name="item"/></DD>
</xsl:template>

<xsl:template match="label[@type='default']" mode='options'>
  <BR/><SPAN CLASS="DEFAULT">default:  </SPAN>
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="label[not(@type='default')]" mode="options">
  <xsl:if test="position()>1">, </xsl:if>
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="label">
  <DT>
    <xsl:apply-templates/>
  </DT>
</xsl:template>

<xsl:template name="item">
  <xsl:for-each select="p">
    <xsl:apply-templates/>
    <P/>
  </xsl:for-each>
</xsl:template>

<xsl:template match="p">
  <P>
    <xsl:apply-templates/>
  </P>
</xsl:template>

<xsl:template match="cmd">
  <SPAN CLASS="CMD">
    <xsl:apply-templates/>
  </SPAN>
</xsl:template>

<xsl:template match="meta">
  <SPAN CLASS="META">
    <xsl:text>&lt;</xsl:text>
    <xsl:apply-templates/>
    <xsl:text>&gt;</xsl:text>
  </SPAN>
</xsl:template>

<xsl:template match="dots">
  <xsl:text>...</xsl:text>
</xsl:template>

<xsl:template match="ie">
  <xsl:text>i.e.</xsl:text>
</xsl:template>

<xsl:template match="a">
  <A>
    <xsl:for-each select="@href">
      <xsl:attribute name="HREF">
        <xsl:value-of select="@href"/>
      </xsl:attribute>
    </xsl:for-each>
    <xsl:apply-templates/>
  </A>
</xsl:template>

<xsl:template match="file">
  <SPAN CLASS="FILE"><xsl:apply-templates/></SPAN>
</xsl:template>

<xsl:template match="align">
  <TABLE CLASS="ALIGN">
    <xsl:apply-templates/>
  </TABLE>
</xsl:template>

<xsl:template match="row">
  <TR>
    <xsl:apply-templates/>
  </TR>
</xsl:template>

<xsl:template match="col">
  <TD>
    <xsl:apply-templates/>
  </TD>
</xsl:template>

<!-- catch all elements that are not explicitly matched -->

<xsl:template match="*" mode="abstract">
  <xsl:message terminate="yes">**** OOPS abstract: <xsl:value-of select="local-name(.)"/> ****</xsl:message>
</xsl:template>

<xsl:template match="*">
  <xsl:message terminate="yes">**** OOPS: <xsl:value-of select="local-name(.)"/> ****</xsl:message>
</xsl:template>

</xsl:stylesheet>