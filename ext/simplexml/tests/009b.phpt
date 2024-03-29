--TEST--
SimpleXML: foreach 
--SKIPIF--
<?php if (!extension_loaded("simplexml")) print "skip"; ?>
--FILE--
<?php 
$sxe = simplexml_load_string(b<<<EOF
<?xml version='1.0'?>
<!DOCTYPE sxe SYSTEM "notfound.dtd">
<sxe id="elem1">
 Plain text.
 <elem1 attr1='first'>Bla bla 1.<!-- comment --><elem2>
   Here we have some text data.
  </elem2></elem1>
 <elem11 attr2='second'>Bla bla 2.</elem11>
</sxe>
EOF
);
var_dump($sxe->children());
?>
===DONE===
<?php exit(0); ?>
--EXPECTF--
object(SimpleXMLElement)#%d (3) {
  [u"@attributes"]=>
  array(1) {
    [u"id"]=>
    unicode(5) "elem1"
  }
  [u"elem1"]=>
  unicode(10) "Bla bla 1."
  [u"elem11"]=>
  unicode(10) "Bla bla 2."
}
===DONE===
