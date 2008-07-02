<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
		xmlns="http://www.w3.org/TR/REC-html40"
                xmlns:date="http://exslt.org/dates-and-times"
                extension-element-prefixes="date" 
		result-ns="" version="1.0">

<!-- problems: text that starts ', ' or '. '-->


<xsl:output method="text" indent="no"/>

<xsl:strip-space elements="doc
  section subsection subsubsection
  optlist optentry option 
  eqlist eqentry tool
  dlist dentry usage usagelist item"/>

<xsl:param name="OZMAKEVERSION" select="''"/>
<xsl:param name="OZMAKEOUTPUT" select="'text'"/>

<!-- reusable replace-string function -->
<!-- taken from http://aspn.activestate.com/ASPN/Cookbook/XSLT/Recipe/65426 -->
<!-- licence is public domain: http://aspn.activestate.com/ASPN/Cookbook/XSLT/license -->
 <xsl:template name="replace-string">
    <xsl:param name="text"/>
    <xsl:param name="from"/>
    <xsl:param name="to"/>

    <xsl:choose>
      <xsl:when test="contains($text, $from)">

	<xsl:variable name="before" select="substring-before($text, $from)"/>
	<xsl:variable name="after" select="substring-after($text, $from)"/>
	<xsl:variable name="prefix" select="concat($before, $to)"/>

	<xsl:value-of select="$before"/>
	<xsl:value-of select="$to"/>
        <xsl:call-template name="replace-string">
	  <xsl:with-param name="text" select="$after"/>
	  <xsl:with-param name="from" select="$from"/>
	  <xsl:with-param name="to" select="$to"/>
	</xsl:call-template>
      </xsl:when> 
      <xsl:otherwise>
        <xsl:value-of select="$text"/>  
      </xsl:otherwise>
    </xsl:choose>            
 </xsl:template>


<xsl:template match="/doc">
<xsl:text>.\"                                      Hey, EMACS: -*- nroff -*-
.\" First parameter, NAME, should be all caps
.\" Second parameter, SECTION, should be 1-8, maybe w/ subsection
.\" other parameters are allowed: see man(7), man(1)
.TH OZMAKE 1 "</xsl:text>
  <xsl:value-of select="date:month-name()"/> 
  <xsl:text> </xsl:text>
  <xsl:value-of select="date:day-in-month()"/> 
  <xsl:text>, </xsl:text>
  <xsl:value-of select="date:year()"/> 
<xsl:text>"
.\" Please adjust this date whenever revising the manpage.
.\"
.\" Some roff macros, for reference:
.\" .nh        disable hyphenation
.\" .hy        enable hyphenation
.\" .ad l      left justify
.\" .ad b      justify to both left and right margins
.\" .nf        disable filling
.\" .fi        enable filling
.\" .br        insert line break
.\" .sp &lt;n&gt;    insert n+1 empty lines
.\" for manpage-specific macros, see man(7)
.SH NAME
Ozmake \- Make for Oz
</xsl:text>
<!-- Pick out synopsis child first -->
<xsl:apply-templates select="section[contains(title/text(),'SYNOPSIS')]"/>
<!-- Now the Description -->
<xsl:apply-templates select="section[contains(title/text(),'USAGE')]"/>
<xsl:apply-templates select="section[contains(title/text(),'OPTIONS')]"/>
<xsl:apply-templates select="section[contains(title/text(),'MAKEFILE')]"/>
<xsl:apply-templates select="section[contains(title/text(),'CONTACTS')]"/>
.SH AUTHOR
This man page has been automatically generated from the \fBozmake\fR help file. The
\fBozmake\fR help file is maintained by Denys Duchier.
.SH SEE ALSO
Full documentation of the Mozart system and the Oz programming
language is available through the
the \fImozart-doc\fP package, or from the mozart web page
\fIwww.mozart-oz.org\fP.
See in particular the document \fIThe Oz Programming Interface\fP.

.P
.BR ozc (1),
.BR ozd (1),
.BR ozengine (1),
.BR ozl (1),
.BR oztool (1),
.BR convertTextPickle (1).
</xsl:template>

<!-- Sections and Subsections -->
<xsl:template match="section">
<xsl:apply-templates select="title" mode="SECTION"/>
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="subsection">
<xsl:apply-templates select="title" mode="SUBSECTION"/>
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="subsubsection">
<xsl:apply-templates select="title" mode="SUBSUBSECTION"/>
<xsl:apply-templates/>
</xsl:template>

<!-- Section headings -->
<!-- Change USAGE -> DESCRIPTION -->
<xsl:template match="title" mode="SECTION">
.SH
<xsl:choose>
<xsl:when test="contains(text(),'USAGE')">
DESCRIPTION
</xsl:when>
<xsl:otherwise>
<xsl:apply-templates/>
</xsl:otherwise>
</xsl:choose>
</xsl:template>

<xsl:template match="title" mode="SUBSECTION">
.SS
<xsl:apply-templates/>
</xsl:template>

<xsl:template match="title" mode="SUBSUBSECTION">
.B
<xsl:apply-templates/>
</xsl:template>

<!-- And do not reprocess -->
<xsl:template match="title"/>

<!-- Usage list -->
<xsl:template match="usagelist">
<xsl:apply-templates mode="USAGELIST"/>
</xsl:template>

<xsl:template match="usage" mode="USAGELIST">
\fB<xsl:apply-templates/>\fR
.br </xsl:template>


<xsl:template match="dlist">
<xsl:apply-templates />
</xsl:template>

<xsl:template match="dentry" >
<xsl:apply-templates />
</xsl:template>

<xsl:template match="dleft" >
.P
 <xsl:apply-templates mode="PARA"/>
</xsl:template>

<xsl:template match="dleft" mode="PARA">
.P
  <xsl:text>   </xsl:text> <xsl:apply-templates mode="PARA"/>
</xsl:template>

<xsl:template match="dright" >
.IP
<xsl:apply-templates mode="PARA"/>
</xsl:template>

<xsl:template match="dright" mode="PARA">
.IP
<xsl:apply-templates mode="PARA"/>
</xsl:template>

<xsl:template match="optlist">
<xsl:apply-templates />
</xsl:template>

<xsl:template match="optentry" >
<xsl:apply-templates />
</xsl:template>

<xsl:template match="option" >
.TP
.SM
<xsl:apply-templates mode="OPTION" />
.br 
</xsl:template>

<xsl:template match="opt" mode="OPTION">
<xsl:apply-templates mode="OPTION"/> <xsl:text>  </xsl:text>
</xsl:template>

<xsl:template match="opt">
  <xsl:text> </xsl:text><xsl:apply-templates/><xsl:text> </xsl:text>
</xsl:template>

<xsl:template match="opt" mode="PARA">\fB<xsl:apply-templates/>\fR</xsl:template>
<xsl:template match="opt" mode="ITEM"> \fB<xsl:apply-templates/>\fR </xsl:template>

<xsl:template match="item">
<xsl:apply-templates mode="ITEM"/>
</xsl:template>

<xsl:template match="item" mode="USAGELIST">
.RS
<xsl:apply-templates mode="ITEM"/>
.RE
</xsl:template>

<xsl:template match="tool">\fB<xsl:value-of select = "." />\fP</xsl:template>

<xsl:template match="tool" mode="PARA">\fB<xsl:value-of select = "." />\fP</xsl:template>

<xsl:template match="meta" mode="OPTION">\fI<xsl:text>&lt;</xsl:text><xsl:value-of select = "." /><xsl:text>&gt;</xsl:text>\fP</xsl:template>

<xsl:template match="meta" mode="DEFAULT">\fI<xsl:text>&lt;</xsl:text><xsl:value-of select = "." /><xsl:text>&gt;</xsl:text>\fP</xsl:template>

<xsl:template match="meta" mode="PARA">\fI<xsl:text>&lt;</xsl:text><xsl:apply-templates/><xsl:text>&gt;</xsl:text>\fR</xsl:template>

<xsl:template match="meta">\fI<xsl:text>&lt;</xsl:text><xsl:value-of select = "." /><xsl:text>&gt;</xsl:text>\fR</xsl:template>

<xsl:template match="em" mode="PARA">\fI<xsl:value-of select = "." />\fP</xsl:template>

<xsl:template match="default">\fBdefault<xsl:if test="@type = 'glob'"> glob patterns</xsl:if>:\fP <xsl:apply-templates mode="DEFAULT"/>
.br 
</xsl:template>

<xsl:template match="oz">
\fB<xsl:apply-templates/>\fP</xsl:template>

<xsl:template match="oz" mode="PARA">\fB<xsl:apply-templates/>\fP</xsl:template>

<xsl:template match="file" mode="PARA">\fI<xsl:apply-templates/>\fR</xsl:template>

<xsl:template match="p">
.P
<xsl:apply-templates mode="PARA"/>
<xsl:text>
</xsl:text>
</xsl:template>

<xsl:template match="text()" mode="PARA">
  <xsl:variable name="newtext"> 
    <xsl:call-template name="replace-string">
      <xsl:with-param name="text" select="."/>
      <xsl:with-param name="from" select='"\n"'/>
      <xsl:with-param name="to" select='" "'/>
    </xsl:call-template>
  </xsl:variable>
  <!-- keep first and last characters, strip all other whitespace -->
  <xsl:value-of select = "concat(substring($newtext,1,1),normalize-space(substring($newtext,2,string-length($newtext)-2)),substring($newtext,string-length($newtext),1))" />
</xsl:template>

<xsl:template match="text()" mode="ITEM">
  <xsl:variable name="newtext"> 
    <xsl:call-template name="replace-string">
      <xsl:with-param name="text" select="."/>
      <xsl:with-param name="from" select='"\"'/>
      <xsl:with-param name="to" select='"kk"'/>
    </xsl:call-template>
  </xsl:variable>
  <!-- keep first and last characters, strip all other whitespace -->
  <xsl:value-of select = "concat(substring($newtext,1,1),normalize-space(substring($newtext,2,string-length($newtext)-2)),substring($newtext,string-length($newtext),1))" />
</xsl:template>

<xsl:template match="example">
</xsl:template>

</xsl:stylesheet>