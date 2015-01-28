<?xml version="1.0"?>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="html" indent="yes"/>
  
  <xsl:key name="fnamekey" match="context" use="@fname" />

  <xsl:template match="/*">
    <html>
      <head>
        <title>ferrisxsl query results</title>
        <style>
          td.light { background-color:#ddcccc; }
          td.dark  { background-color:lightgrey; }
        </style>
      </head>
      <body  bgcolor="#ccaaaa" text="#000000">
        <table border="1" >

          <tr bgcolor="338822" color="#000000" >
            <td><font color="#FFFFFF">name</font></td>
            <td><font color="#FFFFFF">description</font></td>
            <td><font color="#FFFFFF">primary-key</font></td>
            <td><font color="#FFFFFF">address</font></td>
            <td><font color="#FFFFFF">md5</font></td>
          </tr>
          
          <!-- page 144 ORA XSLT animal book -->
          <xsl:for-each select="//context[generate-id(.)=
                                  generate-id(key('fnamekey', @fname )[1])]">
            <xsl:sort select="@fname"/>    
            <xsl:variable name="fnamepos" select="position()"/>

            <xsl:for-each select="key('fnamekey', @fname )">
              <xsl:sort select="@description"/>

              <xsl:apply-templates select=".">
                <xsl:with-param name="fnamepos" select="$fnamepos"/>
                <xsl:with-param name="descpos" select="position()"/>
              </xsl:apply-templates>
              
            </xsl:for-each>
          </xsl:for-each>
          
        </table>
      </body>
    </html>
  </xsl:template>

  
  <xsl:template match="context">
    <xsl:param name="fnamepos"/>
    <xsl:param name="descpos"/>

              <tr BGCOLOR="#DDCCCC" >
                <xsl:if test="$descpos = 1">
                  <td valign="left" bgcolor="#999999">
                    <xsl:attribute name="rowspan">
                      <xsl:value-of select="count(key('fnamekey', @fname ))"/>
                    </xsl:attribute>
                    <b>
                      <xsl:value-of select="@fname"/>
                    </b>
                  </td>
                </xsl:if>

                <xsl:variable name="bgcolor">
                  <xsl:choose>
                    <xsl:when test="($descpos) mod 2">light</xsl:when> 
                    <xsl:otherwise>dark</xsl:otherwise>
                  </xsl:choose>
                </xsl:variable>

                <td class="{$bgcolor}"><xsl:value-of select="@description"/> </td>
                <td class="{$bgcolor}"><xsl:value-of select="@primary-key"/> </td>
                <td class="{$bgcolor}"><xsl:value-of select="@addr"/></td>
                <td class="{$bgcolor}"><xsl:value-of select="@md5"/>
              </td>
              </tr>
  </xsl:template>
  
</xsl:stylesheet>
