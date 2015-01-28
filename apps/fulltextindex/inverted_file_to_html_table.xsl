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

    $Id: inverted_file_to_html_table.xsl,v 1.1 2005/07/04 09:16:56 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

Usage:

     findexquery &minus;&minus;dump-index >|/tmp/index.xml
   xsltproc inverted_file_to_html_table.xsl /tmp/index.xml >|/tmp/output.html

-->

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="html" indent="yes"/>
  
  <xsl:key name="lexkey"      match="lexterm" use="@id" />
  <xsl:key name="invkey"      match="term"    use="@id" />
  <xsl:key name="dockey"      match="doc"     use="@id" />
  <xsl:key name="invertedref" match="/index/invertedfile/term/doclist/d" use="@id" />

  <xsl:variable name="lexiconfile">/tmp/lexicon.dump</xsl:variable>
  <xsl:variable name="invfile">/tmp/index.dump</xsl:variable>

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
        <title>Ferris index</title>
        <style>
          td.light { background-color:#ddcccc; }
          td.dark  { background-color:lightgrey; }
        </style>
      </head>
      <body  bgcolor="#ccaaaa" text="#000000">
        <font color="black">

            <xsl:variable name="number-of-columns">
              <xsl:value-of select="count($document-ids) + 3*8"/>
            </xsl:variable>

          <table border="0" columns="{$number-of-columns}" >
            
            <colgroup>
              <col width="6"></col>
              <col width="7"></col>
              <col width="4"></col>
              <col width="4"></col>
              <col width="5"></col>
              <col width="4"></col>
              <col width="4"></col>
              <col width="4"></col>

              <xsl:for-each select="/index/documentmap/doc">
                <col width="5"></col>
                <col width="5"></col>
              </xsl:for-each>
            </colgroup>

            <!-- header for table -->
            <font color="white">
              <tr bgcolor="pink" color="#FFFFFF" >
                <td colspan="8">Index</td>
                <xsl:for-each select="/index/documentmap/doc">
                  <xsl:sort select="@url" />
                  <xsl:apply-templates select="." />
                </xsl:for-each>
              </tr>

              <!-- show the document weights -->
              <tr bgcolor="pink" color="#000000" >
                <td colspan="8">doc weight</td>
                <xsl:for-each select="/index/documentmap/doc">
                  <xsl:sort select="@url" />
                  <td colspan="2">
                    <xsl:value-of select="format-number(@weight,'##0.0')" />
                  </td>
                </xsl:for-each>
              </tr>

              <!-- show the document inverted counts -->
              <tr bgcolor="pink" color="#000000" >
                <td colspan="8">inverted list refcount (given)</td>
                <xsl:for-each select="/index/documentmap/doc">
                  <xsl:sort select="@url" />
                  <td colspan="2">
                    <xsl:value-of select="format-number(@irc,'###0')" />
                  </td>
                </xsl:for-each>
              </tr>

              <!-- show the document inverted counts -->
              <tr bgcolor="pink" color="#000000" >
                <td colspan="8">inverted list refcount (calculated)</td>
                <xsl:for-each select="/index/documentmap/doc">
                  <xsl:sort select="@url" />
                  <td colspan="2">
                    <xsl:value-of select="count(key('invertedref',@id))" />
                  </td>
                </xsl:for-each>
              </tr>

              <!-- has this document been revoked from the index -->
              <tr bgcolor="pink" color="#000000" >
                <td colspan="8">is revoked</td>
                <xsl:for-each select="/index/documentmap/doc">
                  <xsl:sort select="@url" />
                  <td colspan="2">
                    <xsl:value-of select="@revoked" />
                  </td>
                </xsl:for-each>
              </tr>

            </font>

            <!-- describe rest of table -->
            <tr bgcolor="pink" color="#000000" >
              <td>term_id</td>
              <td>term</td>
              <td>f(t)</td>
              <td>w(t)</td>
              <td>list</td>
              <td>list</td>
              <td>avgb/</td>
              <td>avgb/</td>
              <xsl:for-each select="/index/documentmap/doc">
                <xsl:sort select="@url" />
                <td>
                  f(d,t)
                </td>
                <td>
                  w(d,t)
                </td>
              </xsl:for-each>
            </tr>
            <tr bgcolor="pink" color="#000000" >
              <td></td>
              <td></td>
              <td></td>
              <td></td>
              <td>size</td>
              <td>cnks</td>
              <td>docid</td>
              <td>f(d,t)</td>
              <xsl:for-each select="/index/documentmap/doc">
                <xsl:sort select="@url" />
                <td>
                </td>
                <td>
                </td>
              </xsl:for-each>
            </tr>

            
            <!-- loop through each term in lexicon and make a row -->
            <xsl:for-each select="/index/lexicon/lexterm" >
              <xsl:sort select="." />
              <xsl:apply-templates select="." >
                <xsl:with-param name="lexpos" select="position()"/>              
              </xsl:apply-templates>
            </xsl:for-each>          
            
          </table>
        </font>
      </body>
    </html>
  </xsl:template>


  <xsl:template match="doc">
    <td colspan="2">
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
  <xsl:template match="term">
    term!
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
    <xsl:param name="termid"/>
    <xsl:param name="bgcolor"/>
    <xsl:variable name="next-document-id">
      <xsl:value-of select="substring-before($list-of-document-ids,' ')"/>
    </xsl:variable>
    <xsl:variable name="remaining-document-ids">
      <xsl:value-of select="substring-after($list-of-document-ids,' ')"/>
    </xsl:variable>

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
        
             
    <xsl:if test="normalize-space($remaining-document-ids)">
      <xsl:call-template name="lextermcell">
        <xsl:with-param name="list-of-document-ids" select="$remaining-document-ids" />
        <xsl:with-param name="termid" select="$termid" />
        <xsl:with-param name="bgcolor" select="$bgcolor" />
      </xsl:call-template>
    </xsl:if>
    
  </xsl:template>


  <!-- we iterate down each row, visiting each term in the lexicon and then -->
  <!-- looking up which documents have hits for that term and their stats   -->
  <xsl:template match="lexterm">
    <xsl:param name="lexpos"/>

    <xsl:variable name="termid"><xsl:value-of select="@id"/></xsl:variable>

    <xsl:variable name="bgcolor">
      <xsl:choose>
        <xsl:when test="($lexpos) mod 2">light</xsl:when> 
        <xsl:otherwise>dark</xsl:otherwise>
      </xsl:choose>
    </xsl:variable>
    
    <xsl:variable name="matches">
      <xsl:value-of select="key('invkey',$termid)/@matches"/>
    </xsl:variable>

    <tr bgcolor="#DDCCCC" >
      <td class="{$bgcolor}">
        <xsl:value-of select="$termid"/>
      </td>
      <td class="{$bgcolor}" >
        <xsl:value-of select="current()"/>
      </td>
      <td class="{$bgcolor}">
        <xsl:value-of select="$matches" />
      </td>
      <td class="{$bgcolor}">
        <xsl:value-of select="format-number(key('invkey',$termid)/@w_t,'0.000')" />
      </td>
      <td class="{$bgcolor}">
        <xsl:value-of select="key('invkey',$termid)/@ondisksize" />
      </td>
      <td class="{$bgcolor}">
        <xsl:value-of select="key('invkey',$termid)/@chunkcount" />
      </td>
      <td class="{$bgcolor}">
        <xsl:value-of 
          select="format-number(key('invkey',$termid)/@coded_docid_size div $matches,'0.000')" />
      </td>
      <td class="{$bgcolor}">
        <xsl:value-of 
          select="format-number(key('invkey',$termid)/@coded_fdt_size div $matches,'0.000')" />
      </td>


      <xsl:call-template name="lextermcell">
        <xsl:with-param name="list-of-document-ids" select="$document-ids" />
        <xsl:with-param name="termid" select="$termid" />
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
