<?xml version="1.0" encoding="iso-8859-1"?>
<!-- $Id: phpfunc.xsl,v 1.1 2009/05/23 14:49:29 felipe Exp $ -->
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
    xmlns:php="http://php.net/xsl"
>
    <xsl:output  method="text" encoding="iso-8859-1" indent="no"/>
<!--    <xsl:param name="foo" select="'bar'"/>-->
    <xsl:template match="/">
     <xsl:value-of select="php:function('ucwords','this is an example')"/>
    </xsl:template>
</xsl:stylesheet>
