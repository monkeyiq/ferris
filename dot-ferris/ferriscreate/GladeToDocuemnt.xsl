<?xml version="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="xml" version="1.0" standalone="no"   
    doctype-system="http://www.monkeyiq.org/create.xsd" />

    <xsl:key name="widgetkey" match="widget" use="@class" />

	<xsl:template match="glade-interface">

	<xsl:variable name="outerpage"
select="//widget[@class='GtkNotebook']/property[@name='page']"/>

	   <xsl:variable name="elecount" select="count(//widget[@class='GtkNotebook']//widget[@class='GtkNotebook'])" />
	   <xsl:choose>
	   <xsl:when test="$elecount &lt; 5">
		<xsl:apply-templates 
select="//widget[@class='GtkNotebook']/child[1+$outerpage]"/>
	   </xsl:when>
	   <xsl:otherwise>

		<xsl:variable name="pagenum"
select="//widget[@class='GtkNotebook']/child[1+$outerpage]//widget[@class='GtkNotebook']/property[@name='page']"/>

<xsl:message>outer page:<xsl:value-of select="$outerpage" /></xsl:message>
<xsl:message>inner page: <xsl:value-of select="$pagenum" /></xsl:message>

		<xsl:apply-templates 
select="//widget[@class='GtkNotebook']/child[1+$outerpage]//widget[@class='GtkNotebook']/child[1+$pagenum]"/>
	   </xsl:otherwise>
	   </xsl:choose>

	</xsl:template>



	<xsl:template match="widget[@class='GtkVBox']">

	   <xsl:choose>
	   <xsl:when test="./property[@name='name']='no'">

		<xsl:apply-templates select="./child/widget"/> 

	   </xsl:when>
	   <xsl:otherwise>

		<xsl:text disable-output-escaping="yes">&lt;</xsl:text>
			<xsl:value-of select="@id"/>
		<xsl:text disable-output-escaping="yes">&gt;</xsl:text>

			<xsl:apply-templates select="./child/widget"/> 

		<xsl:text disable-output-escaping="yes">&lt;/</xsl:text>
			<xsl:value-of select="@id"/>
		<xsl:text disable-output-escaping="yes">&gt;</xsl:text>

	   </xsl:otherwise>
	   </xsl:choose>

	</xsl:template>

	<xsl:template match="widget[@class='GtkOptionMenu']">

		<xsl:text disable-output-escaping="yes">&lt;</xsl:text>
			<xsl:value-of select="./property[@name='name']"/>
		<xsl:text disable-output-escaping="yes">&gt;</xsl:text>

			<xsl:value-of select="./child/widget/property[@name='label']"/>  

		<xsl:text disable-output-escaping="yes">&lt;/</xsl:text>
			<xsl:value-of select="./property[@name='name']"/>
		<xsl:text disable-output-escaping="yes">&gt;</xsl:text>

		<xsl:apply-templates select="./child/widget"/> 

	</xsl:template>


	<xsl:template match="widget[@class='GtkEntry']">
		<xsl:variable name="pname" select="./property[@name='name']"/>


		<xsl:text disable-output-escaping="yes">&lt;</xsl:text>
			<xsl:value-of select="./property[@name='name']"/>
		<xsl:text disable-output-escaping="yes">&gt;</xsl:text>

			<xsl:value-of select="./property[@name='text']"/>  

		<xsl:text disable-output-escaping="yes">&lt;/</xsl:text>
			<xsl:value-of select="./property[@name='name']"/>
		<xsl:text disable-output-escaping="yes">&gt;</xsl:text>

		<xsl:apply-templates select="./child/widget"/> 

	</xsl:template>

	<xsl:template match="widget[@class='GtkCheckButton']">
		<xsl:variable name="pname" select="./property[@name='name']"/>


		<xsl:text disable-output-escaping="yes">&lt;</xsl:text>
			<xsl:value-of select="./property[@name='name']"/>
		<xsl:text disable-output-escaping="yes">&gt;</xsl:text>

			<xsl:value-of select="./property[@name='active']"/>  

		<xsl:text disable-output-escaping="yes">&lt;/</xsl:text>
			<xsl:value-of select="./property[@name='name']"/>
		<xsl:text disable-output-escaping="yes">&gt;</xsl:text>

		<xsl:apply-templates select="./child/widget"/> 

	</xsl:template>

	<xsl:template match="widget[@class='GtkSpinButton']">
		<xsl:variable name="pname" select="./property[@name='name']"/>


		<xsl:text disable-output-escaping="yes">&lt;</xsl:text>
			<xsl:value-of select="./property[@name='name']"/>
		<xsl:text disable-output-escaping="yes">&gt;</xsl:text>

			<xsl:value-of select="./property[@name='text']"/>  

		<xsl:text disable-output-escaping="yes">&lt;/</xsl:text>
			<xsl:value-of select="./property[@name='name']"/>
		<xsl:text disable-output-escaping="yes">&gt;</xsl:text>

		<xsl:apply-templates select="./child/widget"/> 

	</xsl:template>


	<xsl:template match="widget">
		<xsl:apply-templates select="./child/widget"/> 
	</xsl:template>
<!--	<xsl:template match="property"></xsl:template> -->

</xsl:stylesheet>
