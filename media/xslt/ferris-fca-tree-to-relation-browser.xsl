<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version="1.0">

  <xsl:output method="xml"/>

  <xsl:key name="ferrislskey"      match="ferrisls" use="@id" />


  <xsl:template match="/">
    <RelationViewerData>

      <Settings appTitle="Formal Concept Analysis in Relation browser" startID=" " defaultRadius="150" maxRadius="180">
		<RelationTypes>
			<UndirectedRelation color="0x85CDE4" lineSize="4" labelText="association" letterSymbol="B"/>
			<DirectedRelation color="0xFF6666" lineSize="5"/>
			<MyCustomRelation color="0xB7C631" lineSize="4" labelText="created by" letterSymbol="C"/>
			<FooRelation color="0xAA0000" lineSize="14" labelText="foo!" letterSymbol="F"/>
		</RelationTypes>
		<NodeTypes>
			<Node/>
			<Comment/>
			<Person/>
			<Document/>
		</NodeTypes>

      </Settings>

      <Nodes>
        <xsl:apply-templates mode="nodes"/>
      </Nodes>

      <Relations>
        <xsl:apply-templates mode="links"/>
      </Relations>

  </RelationViewerData>
  </xsl:template>

  <xsl:template name="handle-self-s-nodes">
    <xsl:param name="arg1" select="'ff'"/>
    <xsl:for-each select="./context">
      <xsl:value-of select="@delegate-name"/>
<xsl:text> 
</xsl:text>
<!--       <xsl:element name="a"> -->
<!--         <xsl:attribute name="href"><xsl:value-of select="@name"/></xsl:attribute> -->
<!--         <xsl:value-of select="@delegate-name"/> -->
<!--       </xsl:element> -->
     
    </xsl:for-each>
  </xsl:template>

  <xsl:template match="ferrisls" mode="nodes">

    <xsl:choose>
      <xsl:when test="@id=' '">
        <xsl:element name="node">
          <xsl:attribute name="id">
            <xsl:value-of select="' '"/>
          </xsl:attribute>
          <xsl:attribute name="name">
            <xsl:value-of select="'ALL FILES'"/>
          </xsl:attribute>
          <xsl:attribute name="URL">
            <xsl:value-of select="@url"/>
          </xsl:attribute>
        </xsl:element>
      </xsl:when>
      <xsl:when test="@name!='-self-s'">
      </xsl:when>
      <xsl:otherwise>
    
        <xsl:element name="node">
          <xsl:attribute name="id">
            <xsl:value-of select="@id"/>
          </xsl:attribute>
          <xsl:attribute name="name">

            <xsl:choose>
              <xsl:when test="count(./*)=0">
                BOTTOM
              </xsl:when>
<!--               <xsl:when test="count(key('ferrislskey',@id)/ferrisls[@name!='-self-s'])=0">BOTTOM</xsl:when> -->
              <xsl:otherwise>
                <xsl:value-of select="@fisheye-label"/>
              </xsl:otherwise>
            </xsl:choose>
          </xsl:attribute>
          <xsl:attribute name="URL">
            <xsl:value-of select="@url"/>
          </xsl:attribute>
          <xsl:value-of select="@id"/>
<xsl:text> 
</xsl:text>
          
          <xsl:if test="@name='-self-s'">
            <xsl:call-template name="handle-self-s-nodes">
              <xsl:with-param name="arg1" select="'x'"/>
            </xsl:call-template>
          </xsl:if>
        </xsl:element>
      </xsl:otherwise>
    </xsl:choose>

    <xsl:apply-templates mode="nodes"/>
  </xsl:template>

  <xsl:template match="context" mode="nodes">
  </xsl:template>

  <xsl:template match="*" mode="nodes">
    <unmatched/>
  </xsl:template>

  <xsl:template match="ferrisls" mode="links">
    <xsl:apply-templates mode="links"/>
  </xsl:template>

  <xsl:template name="links-context-body">
    <xsl:param name="relationtype" select="'ff'"/>
    <xsl:param name="letterSymbol" select="'S'"/>

    <xsl:choose>
      <xsl:when test="@id=''"></xsl:when> 
      <xsl:when test="count(key('ferrislskey',../@id)/ferrisls[@name!='-self-s'])=0"></xsl:when>
      <xsl:otherwise>

        <xsl:element name="{$relationtype}">

          <xsl:attribute name="fromID">
            <xsl:value-of select="../@id"/>
          </xsl:attribute>
          <xsl:attribute name="toID">
            <xsl:value-of select="@id"/>
          </xsl:attribute>
          <xsl:attribute name="labelText">
            <xsl:choose>
              <xsl:when test="count(key('ferrislskey',@id)/ferrisls[@name!='-self-s'])=0">END</xsl:when>
<!--               <xsl:when test="@id=''">END -->
<!-- CHNUM:<xsl:value-of select="count(key('ferrislskey',@id)/ferrisls[@name!='-self-s'])"/> ... -->
<!-- </xsl:when> -->
              <xsl:otherwise>
                <xsl:value-of select="@name-label"/>
              </xsl:otherwise>
            </xsl:choose>
          </xsl:attribute>
          <xsl:attribute name="letterSymbol">
            <xsl:value-of select="$letterSymbol"/>
          </xsl:attribute>

        </xsl:element>
      </xsl:otherwise>
    </xsl:choose>
  </xsl:template>

  <xsl:template match="context" mode="links">

    <xsl:variable name="n"><xsl:value-of select="normalize-space(@name)"/></xsl:variable>
  <xsl:choose>
    <xsl:when test="../@id=@id">
    </xsl:when>
    <xsl:when test="@id=' '">
    </xsl:when>
    <xsl:otherwise>
      <xsl:choose>

        <xsl:when test="count(key('ferrislskey',@id)/ferrisls[@name!='-self-s'])=0">
          <xsl:call-template name="links-context-body">
            <xsl:with-param name="relationtype" select="'DirectedRelation'"/>
            <xsl:with-param name="letterSymbol" select="'B'"/>
          </xsl:call-template>
        </xsl:when>
        <xsl:when test="starts-with($n,'mtime') and (contains($n,'width') or contains($n,'height'))">
          <xsl:call-template name="links-context-body">
            <xsl:with-param name="relationtype" select="'MyCustomRelation'"/>
            <xsl:with-param name="letterSymbol" select="'A'"/>
          </xsl:call-template>
        </xsl:when>
        <xsl:when test="starts-with($n,'mtime')">
          <xsl:call-template name="links-context-body">
            <xsl:with-param name="relationtype" select="'DirectedRelation'"/>
            <xsl:with-param name="letterSymbol" select="'T'"/>
          </xsl:call-template>
        </xsl:when>
        <xsl:when test="contains($n,'Feb06Trip') or contains($n,'Oct05Trip')">
          <xsl:call-template name="links-context-body">
            <xsl:with-param name="relationtype" select="'DirectedRelation'"/>
            <xsl:with-param name="letterSymbol" select="'T'"/>
          </xsl:call-template>
        </xsl:when>
        <xsl:when test="contains($n,'Flash')">
          <xsl:call-template name="links-context-body">
            <xsl:with-param name="relationtype" select="'DirectedRelation'"/>
            <xsl:with-param name="letterSymbol" select="'F'"/>
          </xsl:call-template>
        </xsl:when>
        <xsl:when test="contains($n,'Expose')">
          <xsl:call-template name="links-context-body">
            <xsl:with-param name="relationtype" select="'DirectedRelation'"/>
            <xsl:with-param name="letterSymbol" select="'E'"/>
          </xsl:call-template>
        </xsl:when>
        <xsl:when test="contains($n,'width')">
          <xsl:call-template name="links-context-body">
            <xsl:with-param name="relationtype" select="'DirectedRelation'"/>
            <xsl:with-param name="letterSymbol" select="'D'"/>
          </xsl:call-template>
        </xsl:when>
        <xsl:otherwise>
          <xsl:call-template name="links-context-body">
            <xsl:with-param name="relationtype" select="'DirectedRelation'"/>
            <xsl:with-param name="letterSymbol" select="'X'"/>
          </xsl:call-template>
        </xsl:otherwise>
      </xsl:choose>
    </xsl:otherwise>
  </xsl:choose>
  
  </xsl:template>

  <xsl:template match="*" mode="links">
    <unmatched/>
  </xsl:template>


  <xsl:template match="*">
    <unmatchedraw/>
  </xsl:template>

</xsl:stylesheet>
