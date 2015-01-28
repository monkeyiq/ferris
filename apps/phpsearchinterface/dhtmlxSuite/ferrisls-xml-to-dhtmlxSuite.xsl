<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

  <xsl:output method="xml"/>

  <xsl:template match="/ferrisls">
    <xsl:variable name="number-of-columns">3</xsl:variable>

    <rows>
          <head>
              <column width="40" type="ro" align="right" color="lightgrey" sort="int">mtime</column>
              <column width="40" type="ro" align="right" color="lightgrey" sort="int">sz</column>
              <column width="40" type="link" align="left" color="lightgrey" sort="str">purl</column>
              <column width="40" type="link" align="left" color="lightgrey" sort="str">name</column>
              <column width="100" type="ro" align="right" color="lightgrey" sort="int">size</column>
              <column width="200" type="ro" align="right" color="lightgrey" sort="str">mtime-d</column>
              <column width="*" type="link" align="left" color="lightgrey" sort="str">url</column>
            </head>
          <xsl:for-each select="//context">
        <xsl:sort select="@name" /> 
        <xsl:apply-templates select=".">
        </xsl:apply-templates>
      </xsl:for-each>
    </rows>
  </xsl:template>

  <xsl:template match="context">

    <xsl:variable name="earl"><xsl:value-of select="@url" /></xsl:variable>

    <row>
      <cell><xsl:value-of select="@mtime" /></cell>
      <cell><xsl:value-of select="@size" /></cell>
      <cell><xsl:value-of select="@parent-url" /></cell>
      <cell><xsl:value-of select="@name-only" /></cell>
      <cell><xsl:value-of select="@size-human-readable" /></cell>
      <cell><xsl:value-of select="@mtime-display" /></cell>
      <cell><xsl:value-of select="$earl" />^<xsl:value-of select="$earl" /></cell>
    </row>
  </xsl:template>

</xsl:stylesheet>
