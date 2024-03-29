--TEST--
Bug #26458 (var_dump(), var_export() & debug_zval_dump() are not binary safe for array keys)
--FILE--
<?php
$test = array("A\x00B" => "Hello world");
var_dump($test);
var_export($test);
debug_zval_dump($test);
?>
--EXPECT--
array(1) {
  [u"A B"]=>
  unicode(11) "Hello world"
}
array (
  'A B' => 'Hello world',
)array(1) refcount(2){
  [u"A B" { 0041 0000 0042 }]=>
  unicode(11) "Hello world" { 0048 0065 006c 006c 006f 0020 0077 006f 0072 006c 0064 } refcount(1)
}
