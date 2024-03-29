--TEST--
SimpleXML: Text data
--SKIPIF--
<?php if (!extension_loaded("simplexml")) print "skip"; ?>
--FILE--
<?php 

$sxe = simplexml_load_string(b<<<EOF
<?xml version='1.0'?>
<!DOCTYPE sxe SYSTEM "notfound.dtd">
<sxe id="elem1">
 Plain text.
 <elem1 attr1='first'>
  <!-- comment -->
  <elem2>
   Here we have some text data.
   <elem3>
    And here some more.
    <elem4>
     Wow once again.
    </elem4>
   </elem3>
  </elem2>
 </elem1>
</sxe>
EOF
);

var_dump(trim($sxe->elem1->elem2));
var_dump(trim($sxe->elem1->elem2->elem3));
var_dump(trim($sxe->elem1->elem2->elem3->elem4));

echo "---Done---\n";

?>
--EXPECT--
unicode(28) "Here we have some text data."
unicode(19) "And here some more."
unicode(15) "Wow once again."
---Done--- 
