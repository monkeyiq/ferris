<?xml version="1.0"?>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="html" indent="yes"/>
  
  <xsl:key name="addrkey" match="context" use="@addr" />

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

        <h1>Select address to query for</h1>

        <form action="query.fxsql" METHOD="GET">
          <select name="address" size="6" >

            <xsl:for-each select="//context[generate-id(.)=
                                  generate-id(key('addrkey', @addr )[1])]">
              <xsl:sort select="@addr"/>    
              <xsl:apply-templates select="." />
            </xsl:for-each>            
            </select>
          <br></br>
          <input type="submit" name="subbutton" value="Submit"></input>
        </form>

      </body>
    </html>
  </xsl:template>

  
  <xsl:template match="context">
    <option value="{@addr}" ><xsl:value-of select="@addr"/></option>
  </xsl:template>
  
</xsl:stylesheet>
