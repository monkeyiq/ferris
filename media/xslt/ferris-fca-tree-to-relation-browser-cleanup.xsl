<?xml version="1.0" encoding="utf-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
                version="1.0">

  <xsl:output method="xml"/>

  <xsl:key name="nkey"      match="node" use="@id" />
  <xsl:key name="rkey"      match="DirectedRelation" use="@id" />

  <xsl:template match="//RelationViewerData">
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
        <xsl:for-each select="key('nkey',*)">
          <xsl:sort select="@id" order="ascending" />
          id:<xsl:value-of select="@id"/> <xsl:text> </xsl:text>

        </xsl:for-each>
      </Nodes>

<!--       <Relations> -->
<!--         <xsl:apply-templates mode="//DirectedRelationX"/> -->
<!--       </Relations> -->

  </RelationViewerData>
  </xsl:template>


  <xsl:template match="*">
    <unmatchedraw/>
  </xsl:template>

</xsl:stylesheet>
