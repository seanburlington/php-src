--TEST--
XMLReader: libxml2 XML Reader, parser property set/get 
--SKIPIF--
<?php if (!extension_loaded("xmlreader")) print "skip"; ?>
--FILE--
<?php 
/* $Id: 005.phpt,v 1.2 2006/08/05 12:32:54 rrichards Exp $ */

$xmlstring = b'<?xml version="1.0" encoding="UTF-8"?>
<books></books>';

$reader = new XMLReader();

$reader->XML($xmlstring);


$a = $reader->setParserProperty(XMLReader::LOADDTD, false);
$b = $reader->getParserProperty(XMLReader::LOADDTD);

if (!$a && !$b) { 
	echo "ok\n";
}

$a = $reader->setParserProperty(XMLReader::SUBST_ENTITIES, true);
$b = $reader->getParserProperty(XMLReader::SUBST_ENTITIES);

if ($a && $b) { 
	echo "ok\n";
}
// Only go through
while ($reader->read());
$reader->close();
?>
===DONE===
--EXPECT--
ok
===DONE===
