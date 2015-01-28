<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"  version="1.0"
  xmlns:ferris="http://libferris.org"
  xmlns:kml="http://earth.google.com/kml/2.2"
  exclude-result-prefixes="ferris"
  >
  <xsl:output method="xml"/>

    <xsl:variable name="cat">11</xsl:variable>

  <xsl:template match="/">
      <xsl:apply-templates select="kml:kml"/>
  </xsl:template>

  <xsl:template match="kml:kml">
    <updates autocreate="1" strip="1" new="6">
      <xsl:apply-templates select="kml:Document//kml:Placemark"/>
    </updates>
  </xsl:template>

  <xsl:template match="kml:Folder">
      <context name="{./kml:name}">
        <xsl:apply-templates select="./kml:Folder"/>
        <xsl:apply-templates select="./kml:Placemark"/>
      </context>
  </xsl:template>



  <!-- termination condition -->
  <xsl:template match="kml:Document" mode="name">
  </xsl:template>

  <!-- grab the name of each successive parent folder 
       to form something like the URL -->
  <xsl:template match="kml:Folder" mode="name">

      <xsl:variable name="n">
         <xsl:value-of select="normalize-space(./kml:name)" />
      </xsl:variable>

      <xsl:if test="$n != 'My Places'">, <xsl:value-of select="$n" />
         <xsl:apply-templates select=".." mode="name"/>
      </xsl:if>
  </xsl:template>


  <xsl:template match="kml:Placemark">

    <xsl:variable name="long">
      <xsl:value-of select="substring-before(./kml:Point/kml:coordinates,',')"/>
    </xsl:variable>
    <xsl:variable name="lat">
      <xsl:value-of select="substring-before(substring-after(./kml:Point/kml:coordinates,','),',')"/>
    </xsl:variable>
    <xsl:variable name="desc">
       <xsl:apply-templates select=".." mode="name"/>
    </xsl:variable>

    <context name="{./kml:name}" lat="{$lat}" lon="{$long}" zoom="{./kml:LookAt/kml:range}" 
        desc="{concat(./kml:name,'',$desc)}" label="{concat(./kml:name,'',$desc)}" cat_id="{$cat}" 
/>
  </xsl:template>


  <xsl:template match="*"></xsl:template>

</xsl:stylesheet>
