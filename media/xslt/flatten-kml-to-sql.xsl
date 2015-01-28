<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"  version="1.0"
  xmlns:ferris="http://libferris.org"
  xmlns:kml="http://earth.google.com/kml/2.2"
  exclude-result-prefixes="ferris"
  >
  <xsl:output method="txt" omit-xml-declaration="yes"/>


    <xsl:variable name="cat">11</xsl:variable>

  <xsl:template match="/">
      <xsl:apply-templates select="kml:kml"/>
  </xsl:template>

  <xsl:template match="kml:kml">
      <xsl:apply-templates select="kml:Document//kml:Placemark"/>
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
    <xsl:variable name="d">
       <xsl:apply-templates select=".." mode="name"/>
    </xsl:variable>
    <xsl:variable name="desc">
       <xsl:value-of select="concat(./kml:name,'',$d)" />
    </xsl:variable>


     insert into poi (lat,lon,label,desc,cat_id) values ( 
         <xsl:value-of select="$lat"/>,
         <xsl:value-of select="$long"/>,
         "<xsl:value-of select="$desc"/>",
         "<xsl:value-of select="$desc"/>",
         <xsl:value-of select="$cat"/> );



  </xsl:template>


  <xsl:template match="*"></xsl:template>

</xsl:stylesheet>
