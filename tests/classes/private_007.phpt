--TEST--
ZE2 A derived class does not know about privates of ancestors
--SKIPIF--
<?php if (version_compare(zend_version(), '2.0.0-dev', '<')) die('skip ZendEngine 2 needed'); ?>
--FILE--
<?php

class Bar {
	public static function pub() {
		Bar::priv();
	}
	private static function priv()	{
		echo "Bar::priv()\n";
	}
}
class Foo extends Bar {
	public static function priv()	{ 
		echo "Foo::priv()\n";
	}
}

Foo::pub();
Foo::priv();

echo "Done\n";
?>
--EXPECT--
Bar::priv()
Foo::priv()
Done
