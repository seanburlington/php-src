--TEST--
Bug #27287 (segfault with unserializing object data)
--SKIPIF--
<?php if (!extension_loaded("wddx")) print "skip"; ?>
--FILE--
<?php
	
	class foo {
	}
	$foo = new foo();
	$foo->abc = 'def';
	
	$string = wddx_serialize_value($foo);
	$bar = wddx_unserialize($string);

	echo "OK\n";

?>
--EXPECT--
OK
