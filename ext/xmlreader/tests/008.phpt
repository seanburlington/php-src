--TEST--
XMLReader: libxml2 XML Reader, DTD 
--SKIPIF--
<?php if (!extension_loaded("xmlreader")) print "skip"; ?>
--FILE--
<?php 
/* $Id: 008.phpt,v 1.4 2007/07/11 16:47:30 dmitry Exp $ */

$xmlstring = '<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<!DOCTYPE LIST SYSTEM "dtdexample.dtd">
<LIST>
<MOVIE ID="x200338360">
<TITLE>Move Title 1</TITLE>
<ORGTITLE/><LOC>Location 1</LOC>
<INFO/>
</MOVIE>
<MOVIE ID="m200338361">
<TITLE>Move Title 2</TITLE>
<ORGTITLE/>
<LOC>Location 2</LOC>
<INFO/>
</MOVIE>
</LIST>';

$dtdfile = rawurlencode((binary)dirname(__FILE__)) . b'/dtdexample.dtd'; 
$file = dirname(__FILE__) . '/__008.xml';
file_put_contents($file, $xmlstring);


$reader = new XMLReader();
$reader->open($file);
$reader->setParserProperty(XMLREADER::LOADDTD, TRUE);
$reader->setParserProperty(XMLREADER::VALIDATE, TRUE);
while($reader->read());
if ($reader->isValid()) {
	echo "file DTD: ok\n";
}
$reader->close();
unlink($file);

$xmlstring = b'<?xml version="1.0" encoding="UTF-8" standalone="no"?>
<!DOCTYPE LIST SYSTEM "file:///' . $dtdfile. b'">
<LIST>
<MOVIE ID="x200338360">
<TITLE>Move Title 1</TITLE>
<ORGTITLE/><LOC>Location 1</LOC>
<INFO/>
</MOVIE>
<MOVIE ID="m200338361">
<TITLE>Move Title 2</TITLE>
<ORGTITLE/>
<LOC>Location 2</LOC>
<INFO/>
</MOVIE>
</LIST>';

$reader = new XMLReader();
$reader->XML($xmlstring);

$reader->setParserProperty(XMLREADER::LOADDTD, TRUE);
$reader->setParserProperty(XMLREADER::VALIDATE, TRUE);
while($reader->read());
if ($reader->isValid()) {
	echo "string DTD: ok\n";
}
?>
===DONE===
--EXPECTF--
file DTD: ok
string DTD: ok
===DONE===
