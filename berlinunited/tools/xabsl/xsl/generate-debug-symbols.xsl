<?xml version="1.0" encoding="ISO-8859-1"?>
<xsl:stylesheet version="1.0" xmlns="http://www.w3.org/1999/xhtml" xmlns:xsl="http://www.w3.org/1999/XSL/Transform" xmlns:xabsl="http://www.xabsl.de">
	<xsl:output method="text"/>
	<xsl:key name="basic-behaviors" match="xabsl:agent-collection/xabsl:options/xabsl:option/xabsl:basic-behaviors/xabsl:basic-behavior" use="@name"/>
	<xsl:key name="output-symbols" match="xabsl:agent-collection/xabsl:options/xabsl:option/xabsl:symbols/xabsl:decimal-output-symbol | xabsl:agent-collection/xabsl:options/xabsl:option/xabsl:symbols/xabsl:boolean-output-symbol | xabsl:agent-collection/xabsl:options/xabsl:option/xabsl:symbols/xabsl:enumerated-output-symbol" use="@name"/>
	<xsl:key name="input-symbols" match="xabsl:agent-collection/xabsl:options/xabsl:option/xabsl:symbols/xabsl:decimal-input-symbol | xabsl:agent-collection/xabsl:options/xabsl:option/xabsl:symbols/xabsl:boolean-input-symbol | xabsl:agent-collection/xabsl:options/xabsl:option/xabsl:symbols/xabsl:enumerated-input-symbol" use="@name"/>
	<xsl:key name="enumerations" match="xabsl:agent-collection/xabsl:options/xabsl:option/xabsl:symbols/xabsl:enumeration" use="@name"/>
	<xsl:variable name="space">
		<xsl:text> </xsl:text>
	</xsl:variable>
	<xsl:variable name="line-break">
		<xsl:text>
</xsl:text>
	</xsl:variable>
	<xsl:template match="xabsl:agent-collection">
		<xsl:text>// Debug Symbols for the agents of </xsl:text>
		<xsl:value-of select="xabsl:title"/>
		<xsl:value-of select="$line-break"/>
		<xsl:value-of select="$line-break"/>
		<xsl:text>// number of enumerations</xsl:text>
		<xsl:value-of select="$line-break"/>
		<xsl:value-of select="count(xabsl:options/xabsl:option/xabsl:symbols/xabsl:*[local-name()='enumeration'][count(key('enumerations',@name)[1] | .)=1])"/>
		<xsl:value-of select="$line-break"/>
		<xsl:text>// enumerations</xsl:text>
		<xsl:value-of select="$line-break"/>
		<xsl:text>// (name number-of-elements (element)*)+</xsl:text>
		<xsl:value-of select="$line-break"/>
		<xsl:apply-templates select="xabsl:options/xabsl:option/xabsl:symbols/xabsl:enumeration"/>
		<xsl:value-of select="$line-break"/>
		<xsl:value-of select="$line-break"/>
		<xsl:text>// number of input symbols</xsl:text>
		<xsl:value-of select="$line-break"/>
		<xsl:value-of select="count(xabsl:options/xabsl:option/xabsl:symbols/xabsl:*[local-name()='decimal-input-symbol' or local-name()='boolean-input-symbol' or local-name()='enumerated-input-symbol'][count(key('input-symbols',@name)[1] | .)=1])"/>
		<xsl:value-of select="$line-break"/>
		<xsl:text>// input symbols</xsl:text>
		<xsl:value-of select="$line-break"/>
		<xsl:text>// (d decimal-input-symbol | b boolean-input-symbol | e enum-name enumerated-input-symbol)+</xsl:text>
		<xsl:value-of select="$line-break"/>
		<xsl:apply-templates select="xabsl:options/xabsl:option/xabsl:symbols/xabsl:decimal-input-symbol | xabsl:options/xabsl:option/xabsl:symbols/xabsl:boolean-input-symbol | xabsl:options/xabsl:option/xabsl:symbols/xabsl:enumerated-input-symbol"/>
		<xsl:value-of select="$line-break"/>
		<xsl:value-of select="$line-break"/>
		<xsl:text>// number of output symbols</xsl:text>
		<xsl:value-of select="$line-break"/>
		<xsl:value-of select="count(xabsl:options/xabsl:option/xabsl:symbols/xabsl:*[local-name()='decimal-output-symbol' or local-name()='boolean-output-symbol' or local-name()='enumerated-output-symbol'][count(key('output-symbols',@name)[1] | .)=1])"/>
		<xsl:value-of select="$line-break"/>
		<xsl:text>// output symbols</xsl:text>
		<xsl:value-of select="$line-break"/>
		<xsl:text>// (d decimal-output-symbol | b boolean-output-symbol | e enum-name enumerated-output-symbol)+</xsl:text>
		<xsl:value-of select="$line-break"/>
		<xsl:apply-templates select="xabsl:options/xabsl:option/xabsl:symbols/xabsl:decimal-output-symbol | xabsl:options/xabsl:option/xabsl:symbols/xabsl:boolean-output-symbol | xabsl:options/xabsl:option/xabsl:symbols/xabsl:enumerated-output-symbol"/>
		<xsl:value-of select="$line-break"/>
		<xsl:value-of select="$line-break"/>
		<xsl:text>// number of options</xsl:text>
		<xsl:value-of select="$line-break"/>
		<xsl:value-of select="count(xabsl:options/xabsl:option)"/>
		<xsl:value-of select="$line-break"/>
		<xsl:text>//options: (name num-of-parameters (d decimal-parameter | b boolean_parameter | e enum-name enumerated-parameter)*)+</xsl:text>
		<xsl:value-of select="$line-break"/>
		<xsl:apply-templates select="xabsl:options/xabsl:option"/>
		<xsl:value-of select="$line-break"/>
		<xsl:value-of select="$line-break"/>
		<xsl:text>// number of basic behaviors</xsl:text>
		<xsl:value-of select="$line-break"/>
		<xsl:value-of select="count(xabsl:options/xabsl:option/xabsl:basic-behaviors/xabsl:basic-behavior[count(key('basic-behaviors',@name)[1] | .)=1])"/>
		<xsl:value-of select="$line-break"/>
		<xsl:text>//basic behaviors: (name num-of-parameters (d decimal-parameter | b boolean_parameter | e enum-name enumerated-parameter)*)+</xsl:text>
		<xsl:value-of select="$line-break"/>
		<xsl:apply-templates select="xabsl:options/xabsl:option/xabsl:basic-behaviors/xabsl:basic-behavior"/>
		<xsl:value-of select="$line-break"/>
		<xsl:value-of select="$line-break"/>
	</xsl:template>
	<xsl:template match="xabsl:option">
			<xsl:variable name="name" select="@name"/>
			<xsl:value-of select="$name"/>
			<xsl:value-of select="$space"/>
			<xsl:value-of select="count(xabsl:option-definitions/xabsl:option-definition[@name=$name]/xabsl:*[local-name()='decimal-parameter' or local-name()='boolean-parameter' or local-name()='enumerated-parameter'])"/>
			<xsl:value-of select="$space"/>
			<xsl:apply-templates select="xabsl:option-definitions/xabsl:option-definition[@name=$name]/xabsl:*[local-name()='decimal-parameter' or local-name()='boolean-parameter' or local-name()='enumerated-parameter']"/>
			<xsl:value-of select="$line-break"/>
	</xsl:template>
	<xsl:template match="xabsl:decimal-parameter">
		<xsl:text>d</xsl:text>
		<xsl:value-of select="$space"/>
		<xsl:value-of select="@name"/>
		<xsl:value-of select="$space"/>
	</xsl:template>
	<xsl:template match="xabsl:boolean-parameter">
		<xsl:text>b</xsl:text>
		<xsl:value-of select="$space"/>
		<xsl:value-of select="@name"/>
		<xsl:value-of select="$space"/>
	</xsl:template>
	<xsl:template match="xabsl:enumerated-parameter">
		<xsl:text>e</xsl:text>
		<xsl:value-of select="$space"/>
		<xsl:value-of select="@enumeration"/>
		<xsl:value-of select="$space"/>
		<xsl:value-of select="@name"/>
		<xsl:value-of select="$space"/>
	</xsl:template>
	<xsl:template match="xabsl:basic-behavior">
		<xsl:if test="count(key('basic-behaviors',@name)[1] | .)=1">
			<xsl:variable name="name" select="@name"/>
			<xsl:value-of select="$name"/>
			<xsl:value-of select="$space"/>
			<xsl:value-of select="count(xabsl:*[local-name()='decimal-parameter' or local-name()='boolean-parameter' or local-name()='enumerated-parameter'])"/>
			<xsl:value-of select="$space"/>
			<xsl:apply-templates select="xabsl:*[local-name()='decimal-parameter' or local-name()='boolean-parameter' or local-name()='enumerated-parameter']"/>
			<xsl:value-of select="$line-break"/>
		</xsl:if>
	</xsl:template>
	<xsl:template match="xabsl:decimal-input-symbol">
		<xsl:if test="count(key('input-symbols',@name)[1] | .)=1">
			<xsl:text>d</xsl:text>
			<xsl:value-of select="$space"/>
			<xsl:value-of select="@name"/>
			<xsl:value-of select="$space"/>
			<xsl:value-of select="$line-break"/>
		</xsl:if>
	</xsl:template>
	<xsl:template match="xabsl:boolean-input-symbol">
		<xsl:if test="count(key('input-symbols',@name)[1] | .)=1">
			<xsl:text>b</xsl:text>
			<xsl:value-of select="$space"/>
			<xsl:value-of select="@name"/>
			<xsl:value-of select="$space"/>
			<xsl:value-of select="$line-break"/>
		</xsl:if>
	</xsl:template>
	<xsl:template match="xabsl:enumerated-input-symbol">
		<xsl:if test="count(key('input-symbols',@name)[1] | .)=1">
			<xsl:text>e</xsl:text>
			<xsl:value-of select="$space"/>
			<xsl:value-of select="@enumeration"/>
			<xsl:value-of select="$space"/>
			<xsl:value-of select="@name"/>
			<xsl:value-of select="$space"/>
			<xsl:value-of select="$line-break"/>
		</xsl:if>
	</xsl:template>
	<xsl:template match="xabsl:decimal-output-symbol">
		<xsl:if test="count(key('output-symbols',@name)[1] | .)=1">
			<xsl:text>d</xsl:text>
			<xsl:value-of select="$space"/>
			<xsl:value-of select="@name"/>
			<xsl:value-of select="$space"/>
			<xsl:value-of select="$line-break"/>
		</xsl:if>
	</xsl:template>
	<xsl:template match="xabsl:boolean-output-symbol">
		<xsl:if test="count(key('output-symbols',@name)[1] | .)=1">
			<xsl:text>b</xsl:text>
			<xsl:value-of select="$space"/>
			<xsl:value-of select="@name"/>
			<xsl:value-of select="$space"/>
			<xsl:value-of select="$line-break"/>
		</xsl:if>
	</xsl:template>
	<xsl:template match="xabsl:enumerated-output-symbol">
		<xsl:if test="count(key('output-symbols',@name)[1] | .)=1">
			<xsl:text>e</xsl:text>
			<xsl:value-of select="$space"/>
			<xsl:value-of select="@enumeration"/>
			<xsl:value-of select="$space"/>
			<xsl:value-of select="@name"/>
			<xsl:value-of select="$space"/>
			<xsl:value-of select="$line-break"/>
		</xsl:if>
	</xsl:template>
	<xsl:template match="xabsl:enumeration">
		<xsl:if test="count(key('enumerations',@name)[1] | .)=1">
			<xsl:value-of select="@name"/>
			<xsl:value-of select="$space"/>
			<xsl:value-of select="count(xabsl:enum-element)"/>
			<xsl:value-of select="$space"/>
			<xsl:for-each select="xabsl:enum-element">
				<xsl:value-of select="@name"/>
				<xsl:value-of select="$space"/>
			</xsl:for-each>
			<xsl:value-of select="$line-break"/>
		</xsl:if>
	</xsl:template>
	<xsl:template match="*"/>
	<xsl:template match="text()"/>
</xsl:stylesheet>
