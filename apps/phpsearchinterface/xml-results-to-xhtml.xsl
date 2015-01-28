<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

  <xsl:output method="html"/>

  <xsl:template match="/ferrisls">
    <xsl:variable name="number-of-columns">3</xsl:variable>

    <html>
      <head>
        <title>Ferris index</title>
        <style>
          td.light { background-color:#d5cccc; }
          td.dark  { background-color:lightgrey; }
          a:link {
          COLOR: #000055;
          }
          a:visited {
          COLOR: #000022;
          }
          a:hover {
          COLOR: #aa0000;
          }
          a:active {
          COLOR: #00FF00;
          }
        </style>
      </head>
      <body bgcolor="#bdbbbb">
        <table border="0" columns="{$number-of-columns}" >
          <!-- header for table -->
          <tr bgcolor="pink" color="#FFFFFF" >
            <td>size</td>
            <td>mtime</td>
            <td>url</td>
          </tr>

          <xsl:for-each select="//context">
             <xsl:sort select="@name" /> 
            <xsl:apply-templates select=".">
              <xsl:with-param name="lexpos" select="position()"/>              
            </xsl:apply-templates>
          </xsl:for-each>
        </table>
      </body>
    </html>
  </xsl:template>

  <xsl:template match="context">
    <xsl:param name="lexpos"/>

    <xsl:variable name="bgcolor">
      <xsl:choose>
        <xsl:when test="($lexpos) mod 2">light</xsl:when> 
        <xsl:otherwise>dark</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

    <xsl:variable name="earl"><xsl:value-of select="@url" /></xsl:variable>

    <tr bgcolor="#DDCCCC" >
      <td class="{$bgcolor}">
        <xsl:value-of select="@size-human-readable" /> 
      </td>
      <td class="{$bgcolor}">
        <xsl:value-of select="@mtime-display" />
      </td>
      <td class="{$bgcolor}">
        <a href="{$earl}"><xsl:value-of select="$earl" /></a>
      </td>
    </tr>
  </xsl:template>

</xsl:stylesheet>
