<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"  version="1.0"
  xmlns:ferris="http://libferris.org"
  exclude-result-prefixes="ferris"
  >
  <xsl:output method="xml"/>

  <xsl:template match="/">
    <kml xmlns="http://earth.google.com/kml/2.1">
      <Document>
      <xsl:apply-templates/>
    </Document>
  </kml>
  </xsl:template>
  
  <xsl:template match="*">
    <xsl:if test="@is-dir&gt;0">
      <Folder>
        <name><xsl:value-of select="@name"/></name>
        <open>1</open>
        <xsl:apply-templates/>
      </Folder>
    </xsl:if>
    <xsl:if test="@is-dir&lt;1">
      <Placemark>
        <name><xsl:value-of select="@name"/></name>
        <LookAt>
          <longitude><xsl:value-of select="@longitude"/></longitude>
          <latitude><xsl:value-of select="@latitude"/></latitude>
          <range><xsl:value-of select="@zoom"/></range>
        </LookAt>
        <Point>
          <coordinates><xsl:value-of select="@longitude"/>,<xsl:value-of select="@latitude"/>,0</coordinates>
        </Point>
      </Placemark>
    </xsl:if>
  </xsl:template>

</xsl:stylesheet>
