--TEST--
XMLReader: libxml2 XML Reader, string data 
--SKIPIF--
<?php if (!extension_loaded("xmlreader")) print "skip"; ?>
--FILE--
<?php 
/* $Id: 001.phpt,v 1.2 2006/08/05 12:32:54 rrichards Exp $ */

$xmlstring = b'<?xml version="1.0" encoding="UTF-8"?>
<books></books>';

$reader = new XMLReader();
$reader->XML($xmlstring);

// Only go through
while ($reader->read()) {
	echo $reader->name."\n";
}
$xmlstring = b'';
$reader = new XMLReader();
$reader->XML($xmlstring);
?>
===DONE===
--EXPECTF--
books
books

Warning: XMLReader::XML(): Empty string supplied as input in %s on line %d
===DONE===
