<?xml version="1.0"?>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="html" indent="yes"/>
    
  <xsl:template match="/ferrisxsl">
    <html>
      <head><title>ferrisxsl query results</title></head>
      <body  bgcolor="#ccaaaa" text="#000000">
        <table border="1" >
          <tr bgcolor="338822" color="#000000" >
            <td><font color="#FFFFFF">pk</font></td>
            <td><font color="#FFFFFF">id</font></td>
            <td><font color="#FFFFFF">name</font></td>
            <td><font color="#FFFFFF">addr</font></td>
          </tr>
          <xsl:apply-templates/>
        </table>
      </body>
    </html>
  </xsl:template>

  <xsl:template match="/mount-sql-as-xml">
    <html>
      <head><title>ferrisxsl query results</title></head>
      <body  bgcolor="#ccaaaa" text="#000000">
        <table border="1" >
          <tr bgcolor="338822" color="#000000" >
            <td><font color="#FFFFFF">pk</font></td>
            <td><font color="#FFFFFF">userid</font></td>
            <td><font color="#FFFFFF">uname</font></td>
            <td><font color="#FFFFFF">addr</font></td>
          </tr>
          <xsl:apply-templates/>
        </table>
      </body>
    </html>
  </xsl:template>
  
  <xsl:template match="context">

    <tr BGCOLOR="#DDCCCC" >
      <td><xsl:value-of select="@pk"/> </td>
      <td><xsl:value-of select="@userid"/> </td>
      <td><xsl:value-of select="@username"/> </td>
      <td><xsl:value-of select="@addr"/> </td>
    </tr>

    <xsl:apply-templates/>
  </xsl:template>
  
</xsl:stylesheet>
