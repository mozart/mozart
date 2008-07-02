<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
		xmlns="http://www.w3.org/TR/REC-html40"
		result-ns="" version="1.0">

<xsl:strip-space elements="doc usagelist
  section subsection subsubsection
  optlist optentry option
  eqlist eqentry
  dlist dentry"/>

<xsl:output method="html"/>

<xsl:param name="OZMAKEVERSION" select="''"/>
<xsl:param name="OZMAKEOUTPUT" select="'html'"/>

<xsl:template name="PREBUILT">
  <xsl:param name="VERSION" select="''"/>
  <xsl:variable name="EXTENSION">
    <xsl:choose>
      <xsl:when test="$VERSION=''"></xsl:when>
      <xsl:otherwise>-<xsl:value-of select="$VERSION"/></xsl:otherwise>
    </xsl:choose>
  </xsl:variable>
  <DL>
  <DT><xsl:choose>
        <xsl:when test="$VERSION=''">Current version</xsl:when>
        <xsl:otherwise>Version <xsl:value-of select="$VERSION"/></xsl:otherwise>
      </xsl:choose></DT>
  <DD><A
  HREF="http://www.lifl.fr/~duchier/mogul/pub/pkg/ozmake{$EXTENSION}"><SPAN
  CLASS="MODULE">ozmake<xsl:value-of select="$EXTENSION"/></SPAN> for Unix</A></DD>
  <DD><A
  HREF="http://www.lifl.fr/~duchier/mogul/pub/pkg/ozmake{$EXTENSION}.exe"><SPAN
  CLASS="MODULE">ozmake<xsl:value-of select="$EXTENSION"/>.exe</SPAN> for Windows</A></DD>
  <DD><A
  HREF="http://www.lifl.fr/~duchier/mogul/pub/pkg/ozmake{$EXTENSION}.ozf">non
  executable functor <SPAN
  CLASS="MODULE">ozmake<xsl:value-of select="$EXTENSION"/>.ozf</SPAN>
  for either</A></DD>
  </DL>
</xsl:template>

<xsl:template match="/doc">
<HTML>
<HEAD>
<TITLE>ozmake</TITLE>
<STYLE>
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
CODE {	color		: #663366; }
CODE,TT,PRE {
	font-family	: "lucida console",courier,monospace;
}
CODE.DISPLAY {
	display		: block;
	white-space	: pre;
	margin-left	: 2cm;
	margin-top	: 1em;
	margin-bottom	: 1em;
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
  color:steelblue;
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
</STYLE>
</HEAD>
<BODY>
<xsl:if test="$OZMAKEOUTPUT='html'">
<H1>ozmake</H1>
<P CLASS="AUTHOR">
  <A HREF="http://www.lifl.fr/~duchier/">Denys Duchier</A>
</P>
<DL>
  <DT><B>provides</B></DT>
  <DD><SPAN CLASS="MODULE">ozmake</SPAN></DD>
</DL>
<!--
<DL>
  <DT><B>download prebuilt tool</B></DT>
  <xsl:call-template name="PREBUILT">
    <xsl:with-param name="VERSION" select="''"/>
  </xsl:call-template>
  <xsl:call-template name="PREBUILT">
    <xsl:with-param name="VERSION" select="$OZMAKEVERSION"/>
  </xsl:call-template>
</DL>
-->
<HR/>
<!--
  <P CLASS="WARNING">ozmake is now in beta-release - feedback is extremely welcome</P>
-->
<P>see <A HREF="CHANGES">CHANGES</A> for a list of changes between
successive versions of <FILE>ozmake</FILE>.</P>
</xsl:if>
  <xsl:apply-templates/>
<xsl:if test="$OZMAKEOUTPUT='html'">
<HR/>
<ADDRESS>
<A HREF="http://www.lifl.fr/~duchier/">Denys Duchier</A>
</ADDRESS>
</xsl:if>
</BODY>
</HTML>
</xsl:template>

<xsl:template match="usagelist">
<DL>
  <xsl:apply-templates mode="USAGELIST"/>
</DL>
</xsl:template>

<xsl:template match="usage" mode="USAGELIST">
<DT>
<CODE CLASS="SHELL">
  <xsl:apply-templates/>
</CODE>
</DT>
</xsl:template>

<xsl:template match="item" mode="USAGELIST">
<DD><xsl:apply-templates/></DD>
</xsl:template>

<xsl:template match="p">
<P><xsl:apply-templates/></P>
</xsl:template>

<xsl:template match="tool">
<SPAN CLASS="TOOL"><xsl:apply-templates/></SPAN>
</xsl:template>

<xsl:template match="section">
  <H2><xsl:apply-templates select="title" mode="TITLE"/></H2>
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="subsection">
  <H3><xsl:apply-templates select="title" mode="TITLE"/></H3>
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="subsubsection">
  <H4><xsl:apply-templates select="title" mode="TITLE"/></H4>
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="title" mode="TITLE">
  <xsl:apply-templates/>
</xsl:template>

<xsl:template match="title"/>

<xsl:template match="meta">
<SPAN CLASS="META">&lt;<xsl:apply-templates/>&gt;</SPAN>
</xsl:template>

<xsl:template match="optlist">
<DL><xsl:apply-templates/></DL>
</xsl:template>

<xsl:template match="optentry">
  <xsl:apply-templates select="option"/>
  <xsl:apply-templates select="default"/>
  <xsl:apply-templates select="item" mode="OPTLIST"/>
</xsl:template>

<xsl:template match="option">
<DT>
  <xsl:for-each select="opt">
    <xsl:if test="position()>1">, </xsl:if>
    <xsl:apply-templates select="."/>
  </xsl:for-each>
</DT>
</xsl:template>

<xsl:template match="opt">
<CODE CLASS="SHELL"><xsl:apply-templates/></CODE>
</xsl:template>

<xsl:template match="default[@type='glob']">
<DD><SPAN CLASS="DEFAULT">default glob patterns: </SPAN>
<xsl:apply-templates/>
</DD>
</xsl:template>

<xsl:template match="default">
<DD><SPAN CLASS="DEFAULT">default: </SPAN>
<xsl:apply-templates/>
</DD>
</xsl:template>

<xsl:template match="default[@type='glob']" mode="USAGELIST">
<DD><SPAN CLASS="DEFAULT">default glob patterns: </SPAN>
<xsl:apply-templates/>
</DD>
</xsl:template>

<xsl:template match="default" mode="USAGELIST">
<DD><SPAN CLASS="DEFAULT">default: </SPAN>
<xsl:apply-templates/>
</DD>
</xsl:template>


<xsl:template match="item" mode="OPTLIST">
<DD><xsl:apply-templates/></DD>
</xsl:template>

<xsl:template match="file">
<CODE CLASS="FILE"><xsl:apply-templates/></CODE>
</xsl:template>

<xsl:template match="oz">
<CODE CLASS="OZ"><xsl:apply-templates/></CODE>
</xsl:template>

<xsl:template match="example">
<P><SPAN CLASS="EXAMPLE">example:</SPAN>
<xsl:apply-templates/>
</P>
</xsl:template>

<xsl:template match="eqlist">
<table BORDER="0">
<xsl:apply-templates/>
</table>
</xsl:template>

<xsl:template match="eqentry">
<TR>
  <TD><xsl:apply-templates select="eqleft"/></TD>
  <TD>=</TD>
  <TD><xsl:apply-templates select="eqright"/></TD>
</TR>
</xsl:template>

<xsl:template match="eqleft|eqright">
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="oz.display">
<PRE CLASS="OZDISPLAY"><xsl:apply-templates/></PRE>
</xsl:template>

<xsl:template match="dlist">
<DL COMPACT="COMPACT"><xsl:apply-templates/></DL>
</xsl:template>

<xsl:template match="dentry">
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="dleft">
<DT><xsl:apply-templates/></DT>
</xsl:template>

<xsl:template match="dright">
<DD><xsl:apply-templates/></DD>
</xsl:template>

</xsl:stylesheet>