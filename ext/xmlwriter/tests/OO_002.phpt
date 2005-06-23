--TEST--
XMLWriter: libxml2 XML Writer, membuffer, flush
--SKIPIF--
<?php if (!extension_loaded("xmlwriter")) print "skip"; ?>
--FILE--
<?php 
/* $Id: OO_002.phpt,v 1.1 2005/06/23 11:29:03 pajoye Exp $ */

$xw = new XMLWriter();
$xw->openMemory();
$xw->startDocument('1.0', 'utf8', 'standalone');
$xw->startElement("tag1");
$xw->endDocument();

// Force to write and empty the buffer
echo $xw->flush(true);
echo "---Done---\n";
?>
--EXPECT--
<?xml version="1.0" encoding="utf8" standalone="standalone"?>
<tag1/>
---Done--- 
