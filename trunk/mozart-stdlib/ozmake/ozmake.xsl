<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
		xmlns="http://www.w3.org/TR/REC-html40"
		result-ns="" version="1.0">

<xsl:output method="html"/>
<xsl:strip-space elements="
    package head author section dlist item
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
CODE, SPAN.CMD, PRE.OZDISPLAY, PRE.CODEDISPLAY {	color		: #663366; }
CODE,TT,PRE,SPAN.CMD,SPAN.FILE,PRE.OZDISPLAY, PRE.CODEDISPLAY {
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
PRE.OZDISPLAY,PRE.CODEDISPLAY {
	margin-left : 2cm;
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
P.CENTER {
	text-align	: center;
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
      <DL>
        <xsl:call-template name="pkgversion"/>
	<xsl:call-template name="pkgprovides"/>
      </DL>
      <HR/>
      <xsl:apply-templates select="*[not(self::head)]"/>
      <HR/>
      <ADDRESS>
        <xsl:for-each select="head/author">
	  <xsl:if test="position()>1">, </xsl:if>
	  <xsl:apply-templates select="."/>
	</xsl:for-each>
      </ADDRESS>
    </BODY>
  </HTML>
</xsl:template>

<xsl:template name="pkgversion">
  <xsl:for-each select="/*/head/version[1]">
    <DT><B>version</B></DT>
    <DD><SPAN CLASS="MODULE"><xsl:apply-templates/></SPAN></DD>
  </xsl:for-each>
</xsl:template>

<xsl:template name="pkgprovides">
  <xsl:for-each select="/*/head/provides">
    <xsl:if test="position()=1">
      <DT><B>provides</B></DT>
    </xsl:if>
    <DD><SPAN CLASS="MODULE"><xsl:apply-templates/></SPAN></DD>
  </xsl:for-each>
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

<xsl:template match="section/section/section/section/section" priority="5.0">
  <H6>
    <xsl:call-template name="title"/>
  </H6>
  <xsl:call-template name="nonhead"/>
</xsl:template>

<xsl:template match="section/section/section/section" priority="4.0">
  <H5>
    <xsl:call-template name="title"/>
  </H5>
  <xsl:call-template name="nonhead"/>
</xsl:template>

<xsl:template match="section/section/section" priority="3.0">
  <H4>
    <xsl:call-template name="title"/>
  </H4>
  <xsl:call-template name="nonhead"/>
</xsl:template>

<xsl:template match="section/section" priority="2.0">
  <H3>
    <xsl:call-template name="title"/>
  </H3>
  <xsl:call-template name="nonhead"/>
</xsl:template>

<xsl:template match="section" priority="1.0">
  <H2>
    <xsl:call-template name="title"/>
  </H2>
  <xsl:call-template name="nonhead"/>
</xsl:template>

<!-- dlist -->

<xsl:template match="dlist">
  <!-- apparently konqueror requires a newline before the stag DL
       otherwise it won't properly display the list indented -->
  <xsl:text>
</xsl:text>
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

<xsl:template match="label[@type='default']">
  <DT>
    <SPAN CLASS="DEFAULT">
      <xsl:choose>
        <xsl:when test="@text"><xsl:value-of select="@text"/></xsl:when>
	<xsl:otherwise>default</xsl:otherwise>
      </xsl:choose>
      <xsl:text>:  </xsl:text>
    </SPAN>
    <xsl:apply-templates/>
  </DT>
</xsl:template>

<xsl:template match="label">
  <DT>
    <xsl:apply-templates/>
  </DT>
</xsl:template>

<!--
<xsl:template name="item">
  <xsl:variable name="compact" select="parent::*[@compact='yes']"/>
  <xsl:for-each select="*[not(self::label)]">
    <xsl:apply-templates/>
    <xsl:choose>
      <xsl:when test="$compact"/>
      <xsl:otherwise><P/></xsl:otherwise>
    </xsl:choose>
  </xsl:for-each>
</xsl:template>
-->
<xsl:template name="item">
  <xsl:apply-templates select="*[not(self::label)]"/>
  <xsl:choose>
    <xsl:when test="parent::*[@compact='yes']"/>
    <xsl:when test="position()=last()"/>
    <xsl:otherwise><P/></xsl:otherwise>
  </xsl:choose>
</xsl:template>

<xsl:template match="item/p[position()=1 or not(preceding-sibling::p)]" priority="1.0">
  <xsl:apply-templates/>
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

<xsl:template match="eg">
  <xsl:text>e.g.</xsl:text>
</xsl:template>

<xsl:template match="resp">
  <xsl:text>resp.</xsl:text>
</xsl:template>

<xsl:template match="cc">
  <CODE>
    <xsl:apply-templates/>
  </CODE>
</xsl:template>

<xsl:template match="a">
  <A>
    <xsl:if test="@href">
      <xsl:attribute name="HREF"><xsl:value-of select="@href"/></xsl:attribute>
    </xsl:if>
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

<xsl:template match="oz">
  <CODE>
    <xsl:apply-templates/>
  </CODE>
</xsl:template>

<xsl:template match="oz.display">
  <PRE CLASS="OZDISPLAY">
    <xsl:apply-templates/>
  </PRE>
</xsl:template>

<xsl:template match="code.display">
  <PRE CLASS="CODEDISPLAY">
    <xsl:apply-templates/>
  </PRE>
</xsl:template>

<xsl:template match="code">
  <CODE>
    <xsl:apply-templates/>
  </CODE>
</xsl:template>

<xsl:template match="em">
  <EM>
    <xsl:apply-templates/>
  </EM>
</xsl:template>

<xsl:template match="p.center">
   <P CLASS="CENTER">
     <xsl:apply-templates/>
   </P>
</xsl:template>

<xsl:template match="image.center">
  <P CLASS="CENTER">
    <IMG SRC="{@src}">
      <xsl:apply-templates/>
    </IMG>
  </P>
</xsl:template>

<!-- catch all elements that are not explicitly matched -->

<xsl:template match="*" mode="abstract">
  <xsl:message terminate="yes">**** OOPS abstract: <xsl:value-of select="local-name(.)"/> ****</xsl:message>
</xsl:template>

<xsl:template match="*">
  <xsl:message terminate="yes">**** OOPS: <xsl:value-of select="local-name(.)"/> ****</xsl:message>
</xsl:template>

</xsl:stylesheet>