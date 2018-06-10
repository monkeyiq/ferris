<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"  version="1.0"
  xmlns:ferris="http://libferris.org"
  exclude-result-prefixes="ferris"
  >
  <xsl:output method="xml"/>

  <xsl:template match="/">
    <root>
      <xsl:apply-templates/>
      </root>
  </xsl:template>
  
  <xsl:template match="file3">
    <context original-url="{@url}" bar1="{@bar1}" bar2="{@foo1}">File3Prefix<xsl:value-of select="@content"/>File3Postfix.</context>
  </xsl:template>

</xsl:stylesheet>
