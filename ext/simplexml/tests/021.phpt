--TEST--
SimpleXML: Element check
--SKIPIF--
<?php if (!extension_loaded("simplexml")) print "skip"; ?> 
--FILE--
<?php

$ok = 1;
$doc = simplexml_load_string(b'<root><exists>foo</exists></root>');
if(!isset($doc->exists)) {
	$ok *= 0;
}
if(isset($doc->doesnotexist)) {
	$ok *= 0;
}
if ($ok) {
         print "Works\n";
} else {
         print "Error\n";
}
?>
===DONE===
--EXPECT--
Works
===DONE===
