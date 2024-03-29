--TEST--
DOM removeChild : Basic Functionality
--SKIPIF--
<?php
require_once('skipif.inc');
?>
--CREDITS--
Simon Hughes <odbc3@hotmail.com>
--FILE--
<?php

$xml = <<< EOXML
<?xml version="1.0" encoding="ISO-8859-1"?>
<courses>
	<course title="one">
		<notes>
			<note>c1n1</note>
			<note>c1n2</note>
		</notes>
	</course>
	<course title="two">
		<notes>
			<note>c2n1</note>
			<note>c2n2</note>
		</notes>
	</course>
</courses>
EOXML;

function dumpcourse($current) {
	$title = ($current->nodeType != XML_TEXT_NODE && $current->hasAttribute('title')) ? $current->getAttribute('title'):"no title"; 
	echo "Course: $title:";var_dump($current);
	echo "~";var_dump($current->textContent);
}

$dom = new DOMDocument();
$dom->loadXML($xml);
$root = $dom->documentElement;

$children = $root->childNodes;
$len = $children->length;
echo "orignal has $len nodes\n";
for ($index = $children->length - 1; $index >=0; $index--) {
	echo "node $index\n";
	$current = $children->item($index);
	dumpcourse($current);
	if ($current->nodeType == XML_TEXT_NODE) {
		$noderemoved = $root->removeChild($current);
	}
}
$children = $root->childNodes;
$len = $children->length;
echo "after text removed it now has $len nodes\n";
for ($index = 0; $index < $children->length; $index++) {
	echo "node $index\n";
	$current = $children->item($index);
	dumpcourse($current);
}

--EXPECTF--
orignal has 5 nodes
node 4
Course: no title:object(DOMText)#4 (0) {
}
~unicode(1) "
"
node 3
Course: two:object(DOMElement)#5 (0) {
}
~unicode(24) "
		
			c2n1
			c2n2
		
	"
node 2
Course: no title:object(DOMText)#6 (0) {
}
~unicode(2) "
	"
node 1
Course: one:object(DOMElement)#4 (0) {
}
~unicode(24) "
		
			c1n1
			c1n2
		
	"
node 0
Course: no title:object(DOMText)#5 (0) {
}
~unicode(2) "
	"
after text removed it now has 2 nodes
node 0
Course: one:object(DOMElement)#3 (0) {
}
~unicode(24) "
		
			c1n1
			c1n2
		
	"
node 1
Course: two:object(DOMElement)#4 (0) {
}
~unicode(24) "
		
			c2n1
			c2n2
		
	"