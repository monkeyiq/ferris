<?xml version="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="text" version="1.0" standalone="no"   
    doctype-system = "glade-2.0.dtd" />
    
    <xsl:key name="etkey" match="elementType" use="@name" />
    <xsl:key name="etrkey" match="elementTypeRef" use="@name" />

    <xsl:variable name="nl"><xsl:text>
</xsl:text></xsl:variable>

	<xsl:template match="schema">
          <xsl:apply-templates select="//elementTypeRef" />
	</xsl:template>

        <xsl:template match="//elementTypeRef">
          <xsl:value-of select="@name" />
          <xsl:value-of select="$nl" />
        </xsl:template>

</xsl:stylesheet>
