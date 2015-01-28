<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"  version="1.0"
  xmlns:ferris="http://libferris.org"
  xmlns:kml="http://earth.google.com/kml/2.1"
  exclude-result-prefixes="ferris"
  >
  <xsl:output method="xml"/>


  <xsl:template match="kml:kml">
    <updates autocreate="1" strip="1">
      <xsl:apply-templates select="Folder"/>
    </updates>
  </xsl:template>



  <xsl:template match="Folder">
      <context name="{./name}">
        <xsl:apply-templates select="./Folder"/>
        <xsl:apply-templates select="./Placemark"/>
      </context>

  </xsl:template>

  <xsl:template match="Placemark">
    <context name="{./name}" longitude="{./LookAt/longitude}" latitude="{./LookAt/latitude}" zoom="{./LookAt/range}" />
  </xsl:template>

  <xsl:template match="*"></xsl:template>

</xsl:stylesheet>
