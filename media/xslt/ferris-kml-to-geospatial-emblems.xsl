<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"  version="1.0"
  xmlns:ferris="http://libferris.org"
  xmlns:kml="http://earth.google.com/kml/2.1"
  exclude-result-prefixes="ferris"
  >
  <xsl:output method="xml"/>


  <xsl:template match="kml:kml">
    <updates autocreate="1" strip="1" new="6">
      <xsl:apply-templates select="kml:Document/Folder"/>
      <xsl:apply-templates select="kml:Document/kml:Folder"/>
      <xsl:apply-templates select="kml:Folder/kml:Document/Folder"/>
    </updates>
  </xsl:template>



  <xsl:template match="Folder">
      <context name="{./name}">
        <xsl:apply-templates select="./Folder"/>
        <xsl:apply-templates select="./Placemark"/>
      </context>

  </xsl:template>

  <xsl:template match="Placemark">

<!--     <context name="{./name}" longitude="{./LookAt/longitude}" latitude="{./LookAt/latitude}" zoom="{./LookAt/range}" /> -->

    <xsl:variable name="long">
      <xsl:value-of select="substring-before(./Point/coordinates,',')"/>
    </xsl:variable>
    <xsl:variable name="lat">
      <xsl:value-of select="substring-before(substring-after(./Point/coordinates,','),',')"/>
    </xsl:variable>

    <context name="{./name}" latitude="{$lat}" longitude="{$long}" zoom="{./LookAt/range}" />
  </xsl:template>


  <!-- With namespaces everywhere too -->

  <xsl:template match="kml:Folder">
      <context name="{./kml:name}">
        <xsl:apply-templates select="./kml:Folder"/>
        <xsl:apply-templates select="./kml:Placemark"/>
      </context>

  </xsl:template>

  <xsl:template match="kml:Placemark">

    <xsl:variable name="long">
      <xsl:value-of select="substring-before(./kml:Point/kml:coordinates,',')"/>
    </xsl:variable>
    <xsl:variable name="lat">
      <xsl:value-of select="substring-before(substring-after(./kml:Point/kml:coordinates,','),',')"/>
    </xsl:variable>

    <context name="{./kml:name}" latitude="{$lat}" longitude="{$long}" zoom="{./kml:LookAt/kml:range}" />
  </xsl:template>


  <xsl:template match="*"></xsl:template>

</xsl:stylesheet>
