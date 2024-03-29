--TEST--
SPL: ArrayObject::__construct basic usage with ArrayObject::STD_PROP_LIST|ArrayObject::ARRAY_AS_PROPS. Currently fails on php.net due to bug 45622.
--FILE--
<?php
class C {
	public $prop = 'C::prop.orig';
}

class MyArrayObject extends ArrayObject {
	public $prop = 'MyArrayObject::prop.orig';
}	

echo "\n--> Access prop on instance of ArrayObject with ArrayObject::STD_PROP_LIST|ArrayObject::ARRAY_AS_PROPS:\n";
$c = new C;
$ao = new ArrayObject($c, ArrayObject::STD_PROP_LIST|ArrayObject::ARRAY_AS_PROPS);
testAccess($c, $ao);

echo "\n--> Access prop on instance of MyArrayObject with ArrayObject::STD_PROP_LIST|ArrayObject::ARRAY_AS_PROPS:\n";
$c = new C;
$ao = new MyArrayObject($c, ArrayObject::STD_PROP_LIST|ArrayObject::ARRAY_AS_PROPS);
testAccess($c, $ao);

function testAccess($c, $ao) {
	echo "  - Iteration:\n";
	foreach ($ao as $key=>$value) {
		echo "      $key=>$value\n";
	}

	echo "  - Read:\n";
	@var_dump($ao->prop, $ao['prop']);
	
	echo "  - Write:\n";
	$ao->prop = 'changed1';
	$ao['prop'] = 'changed2';
	var_dump($ao->prop, $ao['prop']);
	
	echo "  - Isset:\n";
	var_dump(isset($ao->prop), isset($ao['prop']));
	
	echo "  - Unset:\n";
	unset($ao->prop);
	unset($ao['prop']);
	var_dump($ao->prop, $ao['prop']);
	
	echo "  - After:\n";
	var_dump($ao, $c);
}
?>
--EXPECTF--
--> Access prop on instance of ArrayObject with ArrayObject::STD_PROP_LIST|ArrayObject::ARRAY_AS_PROPS:
  - Iteration:
      prop=>C::prop.orig
  - Read:
unicode(12) "C::prop.orig"
unicode(12) "C::prop.orig"
  - Write:
unicode(8) "changed2"
unicode(8) "changed2"
  - Isset:
bool(true)
bool(true)
  - Unset:

Notice: Undefined index:  prop in %s on line 39

Notice: Undefined index:  prop in %s on line 40

Notice: Undefined index:  prop in %s on line 40
NULL
NULL
  - After:
object(ArrayObject)#2 (1) {
  [u"storage":u"ArrayObject":private]=>
  object(C)#1 (0) {
  }
}
object(C)#1 (0) {
}

--> Access prop on instance of MyArrayObject with ArrayObject::STD_PROP_LIST|ArrayObject::ARRAY_AS_PROPS:
  - Iteration:
      prop=>C::prop.orig
  - Read:
unicode(24) "MyArrayObject::prop.orig"
unicode(12) "C::prop.orig"
  - Write:
unicode(8) "changed1"
unicode(8) "changed2"
  - Isset:
bool(true)
bool(true)
  - Unset:

Notice: Undefined index:  prop in %s on line 40

Notice: Undefined index:  prop in %s on line 40
NULL
NULL
  - After:
object(MyArrayObject)#3 (1) {
  [u"storage":u"ArrayObject":private]=>
  object(C)#4 (0) {
  }
}
object(C)#4 (0) {
}
