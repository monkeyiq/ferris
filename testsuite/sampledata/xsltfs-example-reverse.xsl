<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet
  xmlns:xsl="http://www.w3.org/1999/XSL/Transform"  version="1.0"
  xmlns:ferris="http://libferris.org"
  exclude-result-prefixes="ferris"
  >
  <xsl:output method="xml"/>

  <xsl:template match="/">
    <explicit-updates>
        <xsl:apply-templates/>
      </explicit-updates>
  </xsl:template>

  <xsl:template match="context">
    <context url="{@original-url}">ReversePrefix..CONTENT:<xsl:value-of select="."/>..ReversePostfix.</context>
    <attribute url="{@original-url}" name="foo1"> XYZ foo1content </attribute>
    <attribute url="{@original-url}" name="bar2">Augmented...<xsl:value-of select="@bar1"/></attribute>
  </xsl:template>

</xsl:stylesheet>
