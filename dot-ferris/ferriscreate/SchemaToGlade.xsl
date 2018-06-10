<?xml version="1.0"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="xml" version="1.0" standalone="no"   
    doctype-system = "glade-2.0.dtd" />
    
    <xsl:key name="etkey" match="elementType" use="@name" />
    <xsl:key name="stkey" match="simpleType"  use="@name" />
      
<!--	<xsl:template match="elementType[@name='CreateSubContext']"> -->
<!--	<xsl:template match="//*[@id='root']"> -->

	<xsl:template match="schema">


		<glade-interface>
		  <widget class="GtkWindow" id="win">
		    <property name="title">Ferris Create</property>
		    <property name="default-width">800</property>
		    <property name="default-height">640</property>
		    <property name="type">GTK_WINDOW_TOPLEVEL</property>
		    <property name="allow_shrink">yes</property>
		    <property name="allow_grow">yes</property>
		    <property name="window-position">GTK_WIN_POS_NONE</property>
		    <property name="visible">no</property>

		    <signal name="destroy" handler="gtk_main_quit" />

		    <child>
		      <widget class="GtkVBox" id="no">
		        <property name="homogeneous">no</property>
		        <property name="spacing">0</property>
		        <property name="visible">yes</property>

			<xsl:apply-templates select="key('etkey','CreateSubContext')" />

                          
		        <child><widget class="GtkHBox" id="hbox1">
			        <property name="homogeneous">no</property>
			        <property name="spacing">0</property>
			        <property name="visible">yes</property>

			        <child><widget class="GtkButton" id="ok">
			                <property name="has_default">yes</property>
        			        <property name="can_focus">yes</property>
                			<property name="label">gtk-ok</property>
		        	        <property name="visible">yes</property>
		        	        <property name="use-stock">yes</property>
				</widget></child>

			        <child><widget class="GtkButton" id="cancel">
        			        <property name="can_focus">yes</property>
                			<property name="label">gtk-cancel</property>
		        	        <property name="visible">yes</property>
		        	        <property name="use-stock">yes</property>
				</widget></child>


			</widget>
			<packing>
		            <property name="padding">0</property>
		            <property name="expand">no</property>
		            <property name="fill">no</property>
			</packing>
			</child>
		      </widget>
		    </child></widget>
		</glade-interface>

	</xsl:template>

	<xsl:template match="key('etkey','CreateSubContext')">
<!--		hi: <xsl:value-of select="@name"/> id:<xsl:value-of select="@id"/> -->
		<xsl:apply-templates/>
	</xsl:template>


	<xsl:template match="sequence">

	   <xsl:variable name="elecount" select="count(/schema/elementType)" />
	   <xsl:choose>
	   <xsl:when test="$elecount &lt; 5">
 	          <child><widget class="GtkNotebook" id="globnb">
                    <property name="enable-popup">no</property>
                    <property name="tab-pos">left</property>
                    <property name="can_focus">yes</property>
	            <property name="show_border">yes</property>
	            <property name="scrollable">yes</property>
        	    <property name="tab_border">3</property>
                    <property name="visible">yes</property>
                    <xsl:for-each select="elementTypeRef">
                        <xsl:sort select="."/>
                        <!-- <xsl:apply-templates select="//elementType[@name=current()/@name]"/> -->
                        <xsl:apply-templates select="key('etkey',@name)" />

                    </xsl:for-each>
                  </widget></child>
	   </xsl:when>
	   <xsl:otherwise>

 	        <child><widget class="GtkNotebook" id="globnb">
                  <property name="enable-popup">no</property>
                  <property name="tab-pos">left</property>
                  <property name="can_focus">yes</property>
	          <property name="show_border">yes</property>
	          <property name="scrollable">yes</property>
        	  <property name="tab_border">3</property>
                  <property name="visible">yes</property>

 	          <child><widget class="GtkNotebook" id="_image">
                    <property name="enable-popup">no</property>
                    <property name="tab-pos">left</property>
                    <property name="can_focus">yes</property>
	            <property name="show_border">yes</property>
	            <property name="scrollable">yes</property>
        	    <property name="tab_border">3</property>
                    <property name="visible">yes</property>
                      <xsl:for-each select="elementTypeRef">
                          <xsl:sort select="."/>
                          <!-- <xsl:apply-templates
select="//elementType[@name=current()/@name][@mime-major='image']"/> -->
                          <xsl:apply-templates select="key('etkey',@name)[@mime-major='image']"/>

                      </xsl:for-each>
                  </widget></child>

 	          <child><widget class="GtkNotebook" id="_text">
                    <property name="enable-popup">no</property>
                    <property name="tab-pos">left</property>
                    <property name="can_focus">yes</property>
	            <property name="show_border">yes</property>
	            <property name="scrollable">yes</property>
        	    <property name="tab_border">3</property>
                    <property name="visible">yes</property>
                      <xsl:for-each select="elementTypeRef">
                          <xsl:sort select="."/>
                          <!-- <xsl:apply-templates
select="//elementType[@name=current()/@name][@mime-major='text']"/> -->
                          <xsl:apply-templates select="key('etkey',@name)[@mime-major='text']"/>
                      </xsl:for-each>
                  </widget></child>

 	          <child><widget class="GtkNotebook" id="_audio">
                    <property name="enable-popup">no</property>
                    <property name="tab-pos">left</property>
                    <property name="can_focus">yes</property>
	            <property name="show_border">yes</property>
	            <property name="scrollable">yes</property>
        	    <property name="tab_border">3</property>
                    <property name="visible">yes</property>
                      <xsl:for-each select="elementTypeRef">
                          <xsl:sort select="."/>
                          <!-- <xsl:apply-templates
select="//elementType[@name=current()/@name][@mime-major='audio']"/> -->
                          <xsl:apply-templates select="key('etkey',@name)[@mime-major='audio']"/>
                      </xsl:for-each>
                  </widget></child>

 	          <child><widget class="GtkNotebook" id="_video">
                    <property name="enable-popup">no</property>
                    <property name="tab-pos">left</property>
                    <property name="can_focus">yes</property>
	            <property name="show_border">yes</property>
	            <property name="scrollable">yes</property>
        	    <property name="tab_border">3</property>
                    <property name="visible">yes</property>
                      <xsl:for-each select="elementTypeRef">
                          <xsl:sort select="."/>
                          <!-- <xsl:apply-templates
select="//elementType[@name=current()/@name][@mime-major='video']"/> -->
                          <xsl:apply-templates select="key('etkey',@name)[@mime-major='video']"/>
                      </xsl:for-each>
                  </widget></child>

 	          <child><widget class="GtkNotebook" id="_misc">
                    <property name="enable-popup">no</property>
                    <property name="tab-pos">left</property>
                    <property name="can_focus">yes</property>
	            <property name="show_border">yes</property>
	            <property name="scrollable">yes</property>
        	    <property name="tab_border">3</property>
                    <property name="visible">yes</property>
                      <xsl:for-each select="elementTypeRef">
                          <xsl:sort select="."/>
                          <!-- <xsl:apply-templates
select="//elementType[@name=current()/@name][ not(@mime-major) ]"/> -->
                          <xsl:apply-templates select="key('etkey',@name)[ not(@mime-major) ]"/>
                      </xsl:for-each>
                  </widget></child>

                </widget></child>
	   </xsl:otherwise>
	   </xsl:choose>

	</xsl:template>


	<xsl:template match="elementType"> 
<!--		ele: <xsl:value-of select="@name"/> -->

		    <child><widget class="GtkVBox" id="{@name}">
		        <property name="homogeneous">no</property>
		        <property name="spacing">0</property>
		        <property name="visible">yes</property>


		<xsl:for-each select="./elementType">
		<xsl:sort select="."/>

		    <child><widget class="GtkHBox" id="gtkhboxen">
		        <property name="homogeneous">yes</property>
		        <property name="spacing">0</property>
		        <property name="visible">yes</property>

			<!-- ///////////////////////////////////////////// -->
			<!-- ////////////// Insert label ///////////////// -->
			<!-- ///////////////////////////////////////////// -->
			<xsl:choose>
			<xsl:when test="@name='sql'">
 			</xsl:when>
			<xsl:when test="@name='optimize-for-dbtype'">
 	                <child><widget class="GtkLabel" id="lab">
	                    <property name="label">Enable backend specific 
optimizations for your database</property>
                	    <property name="xalign">1</property>
	                    <property name="yalign">0.5</property>
        	            <property name="xpad">5</property>
                	    <property name="ypad">2</property>
	                    <property name="visible">yes</property>
        	        </widget></child>
                        </xsl:when>
			<xsl:when test="@name='use-odbc3-bulkload'">
 	                <child><widget class="GtkLabel" id="lab">
	                    <property name="label">Enable ODBC 3.0 bulkload usage
(gives you faster index building
if your driver supports it.)</property>
                	    <property name="xalign">1</property>
	                    <property name="yalign">0.5</property>
        	            <property name="xpad">5</property>
                	    <property name="ypad">2</property>
	                    <property name="visible">yes</property>
        	        </widget></child>
                        </xsl:when>

			<xsl:otherwise>
 	                <child><widget class="GtkLabel" id="lab">
	                    <property name="label"><xsl:value-of select="@name"/></property>
                	    <property name="xalign">1</property>
	                    <property name="yalign">0.5</property>
        	            <property name="xpad">5</property>
                	    <property name="ypad">2</property>
	                    <property name="visible">yes</property>
        	        </widget></child>
			</xsl:otherwise>
			</xsl:choose>

			<!-- ///////////////////////////////////////////// -->
			<!-- ////////////// Second element /////////////// -->
			<!-- ///////////////////////////////////////////// -->
	  	        
			<xsl:choose>
			<xsl:when test="dataTypeRef[@name='bool']">
					<child><widget class="GtkCheckButton" id="{@name}">
        	        		    <property name="visible">yes</property>
        	        		    <property name="active"><xsl:value-of select="@default"/></property>
	                		</widget></child>
                        </xsl:when>
                        <xsl:when test="dataTypeRef[@name='int']">
			<xsl:choose>
				<xsl:when test="@minInclusive" >
				<child><widget class="GtkSpinButton" id="{@name}">
		                    <property name="editable">yes</property>

<!-- value lower upper step page page_size -->
				    <property name="adjustment">
<xsl:value-of select="@default"/><xsl:text> </xsl:text>
<xsl:value-of select="@minInclusive"/> <xsl:text> </xsl:text>
<xsl:value-of select="@maxInclusive"/> <xsl:text> </xsl:text>
<xsl:value-of select="@step"/> <xsl:text> </xsl:text>
10 10</property>
	       		            <property name="visible">yes</property>
                		</widget></child>
				</xsl:when>
				<xsl:otherwise>
					<child><widget class="GtkEntry" id="{@name}">
			                    <property name="text">
						<xsl:value-of select="@default"/></property>
		        	            <property name="editable">yes</property>
        	        		    <property name="visible">yes</property>
        	        		    <property name="text"><xsl:value-of select="@default"/></property>
	                		</widget></child>
				</xsl:otherwise>
			</xsl:choose>
			</xsl:when>
			<xsl:when test="dataTypeRef[@name='real']">
				<xsl:variable name="def" select="@default"/>
				<xsl:variable name="mini" select="@minInclusive"/>
				<xsl:variable name="maxi" select="@maxInclusive"/>
				<xsl:variable name="step" select="@step"/>

				<child><widget class="GtkSpinButton" id="{@name}">
		                    <property name="editable">yes</property>
				    <property name="adjustment">
<xsl:value-of select="@default"/><xsl:text> </xsl:text>
<xsl:value-of select="@minInclusive"/> <xsl:text> </xsl:text>
<xsl:value-of select="@maxInclusive"/> <xsl:text> </xsl:text>
<xsl:value-of select="@step"/> <xsl:text> </xsl:text>
0.1 0.1</property>
	       		            <property name="digits">2</property>
	       		            <property name="visible">yes</property>
                		</widget></child>
			</xsl:when>

			<xsl:when test="dataTypeRef[@name='RedlandRepositoryListT']">

				<child><widget class="GtkEntry" id="{@name}">
		                    <property name="editable">yes</property>
		                    <property name="text">
<xsl:text>MAKE_INTO_OPMENU,</xsl:text>
<xsl:for-each select="key('stkey','RedlandRepositoryT')//enumeration" >
	<xsl:value-of select="@value" />
	<xsl:text>,</xsl:text>
</xsl:for-each>
				    </property>
	       		            <property name="visible">yes</property>
                		</widget></child>

			</xsl:when>

			<xsl:when test="dataTypeRef[@name='OptimizeForDBTypeT']">

				<child><widget class="GtkEntry" id="{@name}">
		                    <property name="editable">yes</property>
		                    <property name="text">
<xsl:text>MAKE_INTO_OPMENU,</xsl:text>
<xsl:for-each select="key('stkey','OptimizeForDBTypeT')//enumeration" >
	<xsl:value-of select="@value" />
	<xsl:text>,</xsl:text>
</xsl:for-each>
				    </property>
	       		            <property name="visible">yes</property>
                		</widget></child>

			</xsl:when>

			<xsl:when test="dataTypeRef[@name='OptimizeForEAIndexDBTypeT']">

				<child><widget class="GtkEntry" id="{@name}">
		                    <property name="editable">yes</property>
		                    <property name="text">
<xsl:text>MAKE_INTO_OPMENU,</xsl:text>
<xsl:for-each select="key('stkey','OptimizeForEAIndexDBTypeT')//enumeration" >
	<xsl:value-of select="@value" />
	<xsl:text>,</xsl:text>
</xsl:for-each>
				    </property>
	       		            <property name="visible">yes</property>
                		</widget></child>

			</xsl:when>

			<xsl:when test="dataTypeRef[@name='LicenseListT']">

				<child><widget class="GtkEntry" id="{@name}">
		                    <property name="editable">yes</property>
		                    <property name="text">
<xsl:text>MAKE_INTO_OPMENU,</xsl:text>
<xsl:for-each select="key('stkey','LicenseT')//enumeration" >
	<xsl:value-of select="@value" />
	<xsl:text>,</xsl:text>
</xsl:for-each>
				    </property>
	       		            <property name="visible">yes</property>
                		</widget></child>

			</xsl:when>

			<xsl:when test="dataTypeRef[@name='ColorSpaceListT']">

				<child><widget class="GtkEntry" id="{@name}">
		                    <property name="editable">yes</property>
		                    <property name="text">
<xsl:text>MAKE_INTO_OPMENU,</xsl:text>
<xsl:for-each select="key('stkey','ColorSpaceT')//enumeration" >
	<xsl:value-of select="@value" />
	<xsl:text>,</xsl:text>
</xsl:for-each>
				    </property>
	       		            <property name="visible">yes</property>
                		</widget></child>

<!--
				<child><widget class="GtkOptionMenu" id="x">
	       		            <property name="visible">yes</property>

 				    <child><widget class="GtkMenu" id="x">
                                        <property name="visible">yes</property>
                                        <child><widget class="GtkMenuItem" id="Quit">
                                            <property name="visible">yes</property>
                                            <child>
                                            <widget class="GtkAccelLabel" id="menu1">
                                              <property name="accel-object">Menu1</property>
                                              <property name="xalign">0.0</property>
                                              <property name="label" translatable="yes">_Menu1</property>
                                              <property name="use-underline">yes</property>
                                              <property name="visible">yes</property>
                                            </widget>
                                            </child>

                   		        </widget></child>
                 		    </widget></child>
                		</widget></child>
-->
			</xsl:when>

			<xsl:when test="dataTypeRef[@name='JPEGColorSpaceListT']">

				<child><widget class="GtkEntry" id="{@name}">
		                    <property name="editable">yes</property>
		                    <property name="text">
<xsl:text>MAKE_INTO_OPMENU,</xsl:text>
<xsl:for-each select="key('stkey','JPEGColorSpaceT')//enumeration" >
	<xsl:value-of select="@value" />
	<xsl:text>,</xsl:text>
</xsl:for-each>
				    </property>
	       		            <property name="visible">yes</property>
                		</widget></child>

			</xsl:when>

			<xsl:when test="dataTypeRef[@name='SocketSSLListT']">

				<child><widget class="GtkEntry" id="{@name}">
		                    <property name="editable">yes</property>
		                    <property name="text">
<xsl:text>MAKE_INTO_OPMENU,</xsl:text>
<xsl:for-each select="key('stkey','SocketSSLT')//enumeration" >
	<xsl:value-of select="@value" />
	<xsl:text>,</xsl:text>
</xsl:for-each>
				    </property>
	       		            <property name="visible">yes</property>
                		</widget></child>

			</xsl:when>

			<xsl:when test="dataTypeRef[@name='TCPIPProtocolListT']">

				<child><widget class="GtkEntry" id="{@name}">
		                    <property name="editable">yes</property>
		                    <property name="text">
<xsl:text>MAKE_INTO_OPMENU,</xsl:text>
<xsl:for-each select="key('stkey','TCPIPProtocolT')//enumeration" >
	<xsl:value-of select="@value" />
	<xsl:text>,</xsl:text>
</xsl:for-each>
				    </property>
	       		            <property name="visible">yes</property>
                		</widget></child>

			</xsl:when>

			<xsl:when test="dataTypeRef[@name='DB4TypeListT']">

				<child><widget class="GtkEntry" id="{@name}">
		                    <property name="editable">yes</property>
		                    <property name="text">
<xsl:text>MAKE_INTO_OPMENU,</xsl:text>
<xsl:for-each select="key('stkey','DB4TypeT')//enumeration" >
	<xsl:value-of select="@value" />
	<xsl:text>,</xsl:text>
</xsl:for-each>
				    </property>
	       		            <property name="visible">yes</property>
                		</widget></child>

			</xsl:when>

			<!-- ///////////////////////////////////////////// -->
			<!-- ////////////// Fulltext  //////////////////// -->
			<!-- ///////////////////////////////////////////// -->

			<xsl:when test="dataTypeRef[@name='LexiconClassListT']">

				<child><widget class="GtkEntry" id="{@name}">
		                    <property name="editable">yes</property>
		                    <property name="text">
<xsl:text>MAKE_INTO_OPMENU,</xsl:text>
<xsl:for-each select="key('stkey','LexiconClassT')//enumeration" >
	<xsl:value-of select="@value" />
	<xsl:text>,</xsl:text>
</xsl:for-each>
				    </property>
	       		            <property name="visible">yes</property>
                		</widget></child>

			</xsl:when>


			<xsl:when test="dataTypeRef[@name='LexiconClassUncompressedFirstListT']">

				<child><widget class="GtkEntry" id="{@name}">
		                    <property name="editable">yes</property>
		                    <property name="text">
<xsl:text>MAKE_INTO_OPMENU,</xsl:text>
<xsl:for-each select="key('stkey','LexiconClassUncompressedFirstT')//enumeration" >
	<xsl:value-of select="@value" />
	<xsl:text>,</xsl:text>
</xsl:for-each>
				    </property>
	       		            <property name="visible">yes</property>
                		</widget></child>

			</xsl:when>


			<xsl:when test="dataTypeRef[@name='ReverseLexiconClassUncompressedFirstListT']">

				<child><widget class="GtkEntry" id="{@name}">
		                    <property name="editable">yes</property>
		                    <property name="text">
<xsl:text>MAKE_INTO_OPMENU,</xsl:text>
<xsl:for-each select="key('stkey','ReverseLexiconClassUncompressedFirstT')//enumeration" >
	<xsl:value-of select="@value" />
	<xsl:text>,</xsl:text>
</xsl:for-each>
				    </property>
	       		            <property name="visible">yes</property>
                		</widget></child>

			</xsl:when>


			<xsl:when test="dataTypeRef[@name='StemmerListT']">

				<child><widget class="GtkEntry" id="{@name}">
		                    <property name="editable">yes</property>
		                    <property name="text">
<xsl:text>MAKE_INTO_OPMENU,</xsl:text>
<xsl:for-each select="key('stkey','StemmerT')//enumeration" >
	<xsl:value-of select="@value" />
	<xsl:text>,</xsl:text>
</xsl:for-each>
				    </property>
	       		            <property name="visible">yes</property>
                		</widget></child>

			</xsl:when>


			<xsl:when test="dataTypeRef[@name='LuceneStemmerListT']">

				<child><widget class="GtkEntry" id="{@name}">
		                    <property name="editable">yes</property>
		                    <property name="text">
<xsl:text>MAKE_INTO_OPMENU,</xsl:text>
<xsl:for-each select="key('stkey','LuceneStemmerT')//enumeration" >
	<xsl:value-of select="@value" />
	<xsl:text>,</xsl:text>
</xsl:for-each>
				    </property>
	       		            <property name="visible">yes</property>
                		</widget></child>

			</xsl:when>

			<xsl:when test="dataTypeRef[@name='DocumentNumberGapCodeListT']">

				<child><widget class="GtkEntry" id="{@name}">
		                    <property name="editable">yes</property>
		                    <property name="text">
<xsl:text>MAKE_INTO_OPMENU,</xsl:text>
<xsl:for-each select="key('stkey','DocumentNumberGapCodeT')//enumeration" >
	<xsl:value-of select="@value" />
	<xsl:text>,</xsl:text>
</xsl:for-each>
				    </property>
	       		            <property name="visible">yes</property>
                		</widget></child>

			</xsl:when>

			<xsl:when test="dataTypeRef[@name='EADocumentNumberGapCodeListT']">

				<child><widget class="GtkEntry" id="{@name}">
		                    <property name="editable">yes</property>
		                    <property name="text">
<xsl:text>MAKE_INTO_OPMENU,</xsl:text>
<xsl:for-each select="key('stkey','EADocumentNumberGapCodeT')//enumeration" >
	<xsl:value-of select="@value" />
	<xsl:text>,</xsl:text>
</xsl:for-each>
				    </property>
	       		            <property name="visible">yes</property>
                		</widget></child>

			</xsl:when>


			<xsl:when test="dataTypeRef[@name='FrequencyOfTermInDocumentCodeListT']">

				<child><widget class="GtkEntry" id="{@name}">
		                    <property name="editable">yes</property>
		                    <property name="text">
<xsl:text>MAKE_INTO_OPMENU,</xsl:text>
<xsl:for-each select="key('stkey','FrequencyOfTermInDocumentCodeT')//enumeration" >
	<xsl:value-of select="@value" />
	<xsl:text>,</xsl:text>
</xsl:for-each>
				    </property>
	       		            <property name="visible">yes</property>
                		</widget></child>

			</xsl:when>


			<xsl:when test="dataTypeRef[@name='StringListT']">

       		              <child><widget class="GtkVBox" id="no">
		                <property name="homogeneous">no</property>
		                <property name="spacing">0</property>
		                <property name="visible">yes</property>

				<child><widget class="GtkEntry" id="{@name}">
		                    <property name="editable">yes</property>
		                    <property name="text">
<xsl:text>MAKE_INTO_STRINGLIST,</xsl:text>
<xsl:value-of select="default" />
<!--
<xsl:for-each select="default" >
	<xsl:value-of select="." />
	<xsl:text>,</xsl:text>
</xsl:for-each>
-->
				    </property>
	       		            <property name="visible">yes</property>
                		</widget></child>
                              </widget></child>

			</xsl:when>

			<xsl:when test="dataTypeRef[@name='EditSQLColumnsT']">

       		              <child><widget class="GtkVBox" id="no">
		                <property name="homogeneous">no</property>
		                <property name="spacing">0</property>
		                <property name="visible">yes</property>

				<child><widget class="GtkEntry" id="{@name}">
		                    <property name="editable">yes</property>
		                    <property name="text">
<xsl:text>MAKE_INTO_EDITSQLCOLUMNS,</xsl:text>
<xsl:value-of select="default" />
<!--
<xsl:for-each select="default" >
	<xsl:value-of select="." />
	<xsl:text>,</xsl:text>
</xsl:for-each>
-->
				    </property>
	       		            <property name="visible">yes</property>
                		</widget></child>
                              </widget></child>

			</xsl:when>


			<!-- ///////////////////////////////////////////// -->
			<!-- ////////////// SQL table //////////////////// -->
			<!-- ///////////////////////////////////////////// -->

			<xsl:when test="@name='sql'">

       		              <child><widget class="GtkVBox" id="no">
		                <property name="homogeneous">no</property>
		                <property name="spacing">0</property>
		                <property name="visible">yes</property>
				

			  <xsl:choose>
			  <xsl:when test="./ancestor::elementType[@name='queryview']">
       	                        <child><widget class="GtkScrolledWindow" id="sw">
       	                        <child><widget class="GtkViewport" id="sw">
	     	                <child><widget class="GtkLabel" id="lab">
<property name="label">
SELECT [ ALL | DISTINCT [ ON ( expression [, ...] ) ] ]
    * | expression [ AS output_name ] [, ...]
    [ FROM from_item [, ...] ]
    [ WHERE condition ]
    [ GROUP BY expression [, ...] ]
    [ HAVING condition [, ...] ]
    [ { UNION | INTERSECT | EXCEPT } [ ALL ] select ]
    [ ORDER BY expression [ ASC | DESC | USING operator ] [, ...] ]
    [ LIMIT { count | ALL } ]
    [ OFFSET start ]
    [ FOR UPDATE [ OF table_name [, ...] ] ]

where from_item can be one of:

    [ ONLY ] table_name [ * ] [ [ AS ] alias [ ( column_alias [, ...] ) ] ]
    ( select ) [ AS ] alias [ ( column_alias [, ...] ) ]
    function_name ( [ argument [, ...] ] ) [ AS ] alias [ ( column_alias [, ...] | column_definition [, ...] ) ]
    function_name ( [ argument [, ...] ] ) AS ( column_definition [, ...] )
    from_item [ NATURAL ] join_type from_item [ ON join_condition | USING ( join_column [, ...] ) ]
</property>
	                          <property name="visible">yes</property>
        	                </widget></child>
        	                </widget></child>
        	                </widget>
				<packing>
		            		<property name="padding">0</property>
		            		<property name="expand">yes</property>
		            		<property name="fill">yes</property>
				</packing>
				</child>

	     	                <child><widget class="GtkLabel" id="lab">
				    <property name="label">Enter SQL select below:</property>
	                            <property name="visible">yes</property>
        	                </widget>
				<packing>
		            		<property name="padding">0</property>
		            		<property name="expand">no</property>
		            		<property name="fill">no</property>
				</packing>
				</child>

			  </xsl:when>
			  <xsl:otherwise>

       	                        <child><widget class="GtkScrolledWindow" id="sw">
       	                        <child><widget class="GtkViewport" id="sw">
	     	                <child><widget class="GtkLabel" id="lab">
<property name="label">
CREATE [ [ GLOBAL | LOCAL ] { TEMPORARY | TEMP } ] TABLE table_name (
  { column_name data_type [ DEFAULT default_expr ] [ column_constraint [ ... ] ]
    | table_constraint
    | LIKE parent_table [ { INCLUDING | EXCLUDING } DEFAULTS ] }  [, ... ]
)
[ INHERITS ( parent_table [, ... ] ) ]
[ WITH OIDS | WITHOUT OIDS ]
[ ON COMMIT { PRESERVE ROWS | DELETE ROWS | DROP } ]
[ TABLESPACE tablespace ]

where column_constraint is:

[ CONSTRAINT constraint_name ]
{ NOT NULL | 
  NULL | 
  UNIQUE [ USING INDEX TABLESPACE tablespace ] |
  PRIMARY KEY [ USING INDEX TABLESPACE tablespace ] |
  CHECK (expression) |
  REFERENCES reftable [ ( refcolumn ) ] [ MATCH FULL | MATCH PARTIAL | MATCH SIMPLE ]
    [ ON DELETE action ] [ ON UPDATE action ] }
[ DEFERRABLE | NOT DEFERRABLE ] [ INITIALLY DEFERRED | INITIALLY IMMEDIATE ]

and table_constraint is:

[ CONSTRAINT constraint_name ]
{ UNIQUE ( column_name [, ... ] ) [ USING INDEX TABLESPACE tablespace ] |
  PRIMARY KEY ( column_name [, ... ] ) [ USING INDEX TABLESPACE tablespace ] |
  CHECK ( expression ) |
  FOREIGN KEY ( column_name [, ... ] ) REFERENCES reftable [ ( refcolumn [, ... ] ) ]
    [ MATCH FULL | MATCH PARTIAL | MATCH SIMPLE ] [ ON DELETE action ] [ ON UPDATE action ] }
[ DEFERRABLE | NOT DEFERRABLE ] [ INITIALLY DEFERRED | INITIALLY IMMEDIATE ]
</property>
	                          <property name="visible">yes</property>
        	                </widget></child>
        	                </widget></child>
        	                </widget>
				<packing>
		            		<property name="padding">0</property>
		            		<property name="expand">yes</property>
		            		<property name="fill">yes</property>
				</packing>
				</child>

	     	                <child><widget class="GtkLabel" id="lab">
				    <property name="label">Enter SQL Create below:</property>
	                            <property name="visible">yes</property>
        	                </widget>
				<packing>
		            		<property name="padding">0</property>
		            		<property name="expand">no</property>
		            		<property name="fill">no</property>
				</packing>
				</child>
			  </xsl:otherwise>
			  </xsl:choose>

                                <child><widget class="GtkEntry" id="{@name}">
		                    <property name="text">
					<xsl:value-of select="@default"/>
				    </property>
		                    <property name="editable">yes</property>
        		            <property name="visible">yes</property>
                		</widget>
				<packing>
		            		<property name="padding">0</property>
		            		<property name="expand">no</property>
		            		<property name="fill">no</property>
				</packing>
				</child>



                	      </widget></child>
			</xsl:when>

			<xsl:otherwise>
				<child><widget class="GtkEntry" id="{@name}">
		                    <property name="text">
					<xsl:value-of select="@default"/>
				    </property>
		                    <property name="max-length">
					<xsl:value-of select="@maxLength"/>
				    </property>
		                    <property name="editable">yes</property>
        		            <property name="visible">yes</property>
                		</widget></child>
			</xsl:otherwise>
			</xsl:choose>

				  <xsl:choose>
				  <xsl:when test="./ancestor::elementType[@name='tuple']">
			          <xsl:variable name="mino" select="@minOccur"/>
 	                          <child><widget class="GtkLabel" id="labx">
	                              <property name="label">
<xsl:choose><xsl:when test="$mino > 0">required,</xsl:when></xsl:choose>
<xsl:choose><xsl:when test="dataTypeRef[@name='SQLDoubleT']">double,</xsl:when></xsl:choose>
<xsl:choose><xsl:when test="dataTypeRef[@name='SQLDateT']">(YYYY-MM-DD)</xsl:when></xsl:choose>
<xsl:choose><xsl:when test="dataTypeRef[@name='SQLTimeStampT']">NOW()
or YYYYMMDDHHMMSS</xsl:when></xsl:choose>
</property>
	                              <property name="visible">yes</property>
         	                  </widget></child>
				  </xsl:when>
			          <xsl:otherwise>
	      		          </xsl:otherwise>
				  </xsl:choose>


	
		      </widget>

			<!-- ///////////////////////////////////////////// -->
			<!-- ////////////// Packing control ////////////// -->
			<!-- ///////////////////////////////////////////// -->
			<xsl:choose>
			<xsl:when test="@name='sql'">
			<packing>
		            <property name="padding">0</property>
		            <property name="expand">yes</property>
		            <property name="fill">yes</property>
			</packing>
			</xsl:when>
			<xsl:otherwise>
			<packing>
		            <property name="padding">0</property>
		            <property name="expand">no</property>
		            <property name="fill">no</property>
			</packing>
			</xsl:otherwise>
			</xsl:choose>
		      </child>

		</xsl:for-each>

		      </widget>
			<packing>
		            <property name="padding">0</property>
		            <property name="expand">yes</property>
		            <property name="fill">yes</property>
			</packing>
		      </child>

<!--
  trying to set the label of the tabs 

 	                <child><widget class="GtkLabel" id="tab">
	                    <property name="label"><xsl:value-of select="@name"/></property>
        	            <property name="justify">GTK_JUSTIFY_LEFT</property>
                	    <property name="xalign">0.5</property>
	                    <property name="yalign">0.5</property>
        	            <property name="xpad">0</property>
                	    <property name="ypad">0</property>
	                    <property name="visible">yes</property>
	                    <property name="type">tab</property>
	                    <property name="name">tab</property>
        	        </widget></child>
-->


	</xsl:template>



</xsl:stylesheet>
