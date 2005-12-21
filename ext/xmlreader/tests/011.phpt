--TEST--
XMLReader: libxml2 XML Reader, string data 
--SKIPIF--
<?php if (!extension_loaded("xmlreader")) print "skip"; ?>
--FILE--
<?php 
/* $Id: 011.phpt,v 1.1 2005/12/21 13:25:02 pajoye Exp $ */

$xmlstring = '<?xml version="1.0" encoding="UTF-8"?>
<books><book>test</book></books>';

$reader = new XMLReader();
$reader->XML($xmlstring);
$reader->read();
echo $reader->readInnerXml();
echo "\n";
$reader->close();


$reader = new XMLReader();
$reader->XML($xmlstring);
$reader->read();
echo $reader->readOuterXml();
echo "\n";
$reader->close();
?>
===DONE===
--EXPECT--
<book>test</book>
<books><book>test</book></books>
===DONE===
