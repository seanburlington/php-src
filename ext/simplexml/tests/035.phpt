--TEST--
SimpleXML: __toString
--SKIPIF--
<?php if (!extension_loaded("simplexml")) print "skip"; ?>
--FILE--
<?php
$string = '<?xml version="1.0"?>
<foo><bar>
   <p>Blah 1</p>
   <p>Blah 2</p>
   <p>Blah 3</p>
   <tt>Blah 4</tt>
</bar></foo>
';
$foo = simplexml_load_string($string);
$p = $foo->bar->p;
echo $p."\n";
echo $p->__toString()."\n";
?>
==Done==
--EXPECT--
Blah 1
Blah 1
==Done==
