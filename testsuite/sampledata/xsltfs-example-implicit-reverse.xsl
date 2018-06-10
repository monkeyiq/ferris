<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"  version="1.0"
  xmlns:ferris="http://libferris.org"
  exclude-result-prefixes="ferris"
  >
  <xsl:output method="xml"/>

  <xsl:template match="/">
    <updates>
        <xsl:apply-templates/>
      </updates>
  </xsl:template>

  <xsl:template match="context">

    <xsl:variable name="bar2v">
      <xsl:value-of select="@bar2"/>
    </xsl:variable>

    <context name="root">
    <context name="file3" foo1=" XYZ foo1content " bar2="Augmented...{$bar2v}">IReversePrefix..CONTENT:<xsl:value-of select="."/>..IReversePostfix.</context>
  </context>
  </xsl:template>

</xsl:stylesheet>
