<?xml version="1.0"?>
<!--

/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2001-2003 Ben Martin

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: eainverted_file_to_html_table.xsl,v 1.1 2005/07/04 09:16:56 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

Usage:

     feaindexquery &minus;&minus;dump-index >|/tmp/eaindex.xml
     xsltproc eainverted_file_to_html_table.xsl /tmp/eaindex.xml >|/tmp/output.html

-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="html" indent="yes"/>
  
  <xsl:key name="attrkey"   match="/index/lexicon[@name=attribute]/lexterm" use="@id" />
  <xsl:key name="valuekey"  match="/index/lexicon[@name='value']"     use="@id" />
  <xsl:key name="dockey"    match="doc"     use="@id" />
  <xsl:key name="invertedref" match="/index/invertedfile/term/d" use="@id" />


  <!-- space seperated list of document ids to use for left to right -->
  <!-- column names and for gathering the data in that order         -->
  <xsl:variable name="document-ids">
    <xsl:for-each select="/index/documentmap/doc">
      <xsl:sort select="@url" />
      <xsl:value-of select="@id" /><xsl:text> </xsl:text>
    </xsl:for-each>
  </xsl:variable>

  <xsl:template match="/index">
    <html>
      <head>
        <title>Ferris attribute index</title>
        <style>
          td.light { background-color:#ddcccc; }
          td.dark  { background-color:lightgrey; }
        </style>
      </head>
      <body  bgcolor="#ccaaaa" text="#000000">
        <font color="black">

            <xsl:variable name="number-of-columns">
              <xsl:value-of select="count($document-ids) + 4"/>
            </xsl:variable>

          <table border="0" columns="{$number-of-columns}" >
            
            <colgroup>
              <col width="5"></col>
              <col width="5"></col>
              <col width="5"></col>
              <col width="5"></col>

              <xsl:for-each select="/index/documentmap/doc">
                <col width="5"></col>
              </xsl:for-each>
            </colgroup>

            <!-- header for table -->
            <font color="white">
              <tr bgcolor="pink" color="#FFFFFF" >
                <td colspan="4">Index</td>
                <xsl:for-each select="/index/documentmap/doc">
                  <xsl:sort select="@url" />
                  <xsl:apply-templates select="." />
                </xsl:for-each>
              </tr>

              <tr bgcolor="pink" color="#FFFFFF" >
                <td colspan="4">inverted list refcount (given)</td>
                <xsl:for-each select="/index/documentmap/doc">
                  <xsl:sort select="@url" />
                  <td>
                    <xsl:value-of select="format-number(@irc,'###0')" />
                  </td>
                </xsl:for-each>
              </tr>

              <tr bgcolor="pink" color="#FFFFFF" >
                <td colspan="4">inverted list refcount (calculated)</td>
                <xsl:for-each select="/index/documentmap/doc">
                  <xsl:sort select="@url" />
                  <td>
                    <xsl:value-of select="count(key('invertedref',@id))" />
                  </td>
                </xsl:for-each>
              </tr>

              <!-- has this document been revoked from the index -->
              <tr bgcolor="pink" color="#FFFFFF" >
                <td colspan="4">is revoked</td>
                <xsl:for-each select="/index/documentmap/doc">
                  <xsl:sort select="@url" />
                  <td>
                    <xsl:value-of select="@revoked" />
                  </td>
                </xsl:for-each>
              </tr>

            </font>

            <!-- describe rest of table -->
            <tr bgcolor="pink" color="#000000" >
              <td>aid</td>
              <td>attr name</td>
              <td>vid</td>
              <td>value</td>
              <xsl:for-each select="/index/documentmap/doc">
                <xsl:sort select="@url" />
                <td>
                  match
                </td>
              </xsl:for-each>
            </tr>

            
            <!-- loop through each term in lexicon and make a row -->
            <xsl:for-each select="/index/invertedfile/term" >
              <xsl:sort select="." />
              <xsl:apply-templates select="." >
                <xsl:with-param name="pos" select="position()"/>              
              </xsl:apply-templates>
            </xsl:for-each>          
            
          </table>
        </font>
      </body>
    </html>
  </xsl:template>


  <xsl:template match="doc">
    <td colspan="1">
      <xsl:element name="a">
        <xsl:attribute name="href">
          <xsl:value-of select="@url"/>
        </xsl:attribute>
        <xsl:value-of select="@id" />
      </xsl:element>
    </td>
  </xsl:template>

  <xsl:template match="invertedfile">
    invertedfile!
    <xsl:apply-templates select="./*" />
  </xsl:template>
  <xsl:template match="xterm">
  </xsl:template>

  <xsl:template match="d">
    <xsl:choose>
      <xsl:when test="@freq > 0">
        <xsl:value-of select="@freq"/>
      </xsl:when> 
      <xsl:otherwise>
        <xsl:text> </xsl:text>
      </xsl:otherwise>
    </xsl:choose>
    w_d_t: <xsl:value-of select="@w_d_t" />
  </xsl:template>


  <!-- this loops through a space seperated list of document ids -->
  <!-- and generates an html table cell for each one using data  -->
  <!-- looked up in the inverted file                            -->
  <xsl:template name="lextermcell">
    <xsl:param name="list-of-document-ids"/>
    <xsl:param name="matching-document-ids"/>
    <xsl:param name="aid"/>
    <xsl:param name="vid"/>
    <xsl:param name="bgcolor"/>
    <xsl:variable name="next-document-id">
      <xsl:value-of select="substring-before($list-of-document-ids,' ')"/>
    </xsl:variable>
    <xsl:variable name="remaining-document-ids">
      <xsl:value-of select="substring-after($list-of-document-ids,' ')"/>
    </xsl:variable>

    <!--
    <td class="{$bgcolor}">
x
    </td>
-->

    <xsl:variable name="idcomma"><xsl:value-of select="$next-document-id"/><xsl:text>,</xsl:text></xsl:variable>
      <xsl:choose>
        <xsl:when test="contains( $matching-document-ids, $idcomma )" >
          <td class="{$bgcolor}">
            x
          </td>
        </xsl:when>
        <xsl:otherwise>
          <td class="{$bgcolor}">
          </td>
        </xsl:otherwise>
      </xsl:choose>


    <!--
      <xsl:choose>
        <xsl:when test="count(key('invkey',$termid)/doclist/d[@id=$next-document-id]) > 0" >
          <xsl:for-each select="key('invkey',$termid)/doclist/d[@id=$next-document-id]">
            <td class="{$bgcolor}">
              <xsl:choose>
                <xsl:when test="@freq > 0">
                  <xsl:value-of select="@freq"/>
                </xsl:when> 
                <xsl:otherwise>
                  <xsl:text> </xsl:text>
                </xsl:otherwise>
              </xsl:choose>
            </td>
            <td class="{$bgcolor}">
              <xsl:value-of select="format-number(@w_d_t,'0.000')" /> 
            </td>
          </xsl:for-each>
        </xsl:when>
        <xsl:otherwise>
          <td class="{$bgcolor}" colspan="2">
          </td>
        </xsl:otherwise>
      </xsl:choose>
      -->

             
    <xsl:if test="normalize-space($remaining-document-ids)">
      <xsl:call-template name="lextermcell">
        <xsl:with-param name="list-of-document-ids" select="$remaining-document-ids" />
        <xsl:with-param name="matching-document-ids" select="$matching-document-ids" />
        <xsl:with-param name="aid" select="$aid" />
        <xsl:with-param name="vid" select="$vid" />
        <xsl:with-param name="bgcolor" select="$bgcolor" />
      </xsl:call-template>
    </xsl:if>
    
  </xsl:template>

  <!-- show the aid,vid lookup both of these in the lexicons and show which -->
  <!-- documents match for this aid,vid combination                         -->
  <xsl:template match="term">
    <xsl:param name="pos"/>

    <xsl:variable name="aid"><xsl:value-of select="@aid"/></xsl:variable>
    <xsl:variable name="vid"><xsl:value-of select="@vid"/></xsl:variable>

    <xsl:variable name="bgcolor">
      <xsl:choose>
        <xsl:when test="($pos) mod 2">light</xsl:when> 
        <xsl:otherwise>dark</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>

      <xsl:variable name="matching-document-ids">
        <xsl:for-each select="d">
          <xsl:sort select="@id" />
          <xsl:value-of select="@id" /><xsl:text>,</xsl:text>
        </xsl:for-each>
      </xsl:variable>
    
    <tr bgcolor="#DDCCCC" >
      <td class="{$bgcolor}">
        <xsl:value-of select="$aid"/>
      </td>
      <td class="{$bgcolor}" >
        <xsl:value-of select="/index/lexicon[@name='attribute']/lexterm[@id=$aid]"/>
      </td>
      <td class="{$bgcolor}">
        <xsl:value-of select="$vid" />
      </td>

      <td class="{$bgcolor}" >
      <xsl:choose>
        <xsl:when test="$vid = 1">
          <b>null</b>
        </xsl:when> 
        <xsl:when test="$vid = 2">
          <b>can't read</b>
        </xsl:when> 
        <xsl:otherwise>
          <xsl:value-of select="/index/lexicon[@name='value']/lexterm[@id=$vid]"/>
        </xsl:otherwise>
      </xsl:choose>      
      </td>



      
      <xsl:call-template name="lextermcell">
        <xsl:with-param name="list-of-document-ids" select="$document-ids" />
        <xsl:with-param name="matching-document-ids" select="$matching-document-ids" />
        <xsl:with-param name="aid" select="$aid" />
        <xsl:with-param name="vid" select="$vid" />
        <xsl:with-param name="bgcolor" select="$bgcolor" />
      </xsl:call-template>

      <!-- the below is correct but very slow -->
      <!--
          <xsl:for-each select="//index/documentmap/doc">
            <xsl:sort select="@url" />
            <xsl:variable name="docid"><xsl:value-of select="@id"/></xsl:variable>
            
            <td>
                <xsl:for-each select="key('invkey',$termid)/doclist/d[@id=$docid]">
                  <xsl:apply-templates select="." />
                </xsl:for-each>
            </td>
          </xsl:for-each>
          -->

    </tr>
  </xsl:template>
    
</xsl:stylesheet>
