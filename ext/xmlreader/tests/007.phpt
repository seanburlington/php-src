--TEST--
XMLReader: libxml2 XML Reader, setRelaxNGSchema
--SKIPIF--
<?php if (!extension_loaded("xmlreader")) print "skip";
$reader = new XMLReader();
if (!method_exists($reader, 'setRelaxNGSchema')) print "skip";
?>
--FILE--
<?php 
/* $Id: 007.phpt,v 1.3 2006/08/05 12:32:54 rrichards Exp $ */

$xmlstring = b'<TEI.2>hello</TEI.2>';
$relaxngfile = dirname(__FILE__) . '/relaxNG.rng'; 
$file = dirname(__FILE__) . '/__007.xml';
file_put_contents($file, $xmlstring);

$reader = new XMLReader();
$reader->open($file);

if ($reader->setRelaxNGSchema($relaxngfile)) {
  while ($reader->read());
}
if ($reader->isValid()) {
  print "file relaxNG: ok\n";
} else {
  print "file relaxNG: failed\n";
}
$reader->close();
unlink($file);


$reader = new XMLReader();
$reader->XML($xmlstring);

if ($reader->setRelaxNGSchema($relaxngfile)) {
  while ($reader->read());
}
if ($reader->isValid()) {
  print "string relaxNG: ok\n";
} else {
  print "string relaxNG: failed\n";
}

$reader->close();

$reader = new XMLReader();
$reader->XML($xmlstring);

if ($reader->setRelaxNGSchema('')) {
	echo 'failed';
}
$reader->close();
?>
===DONE===
--EXPECTF--
file relaxNG: ok
string relaxNG: ok

Warning: XMLReader::setRelaxNGSchema(): Schema data source is required in %s on line %d
===DONE===
