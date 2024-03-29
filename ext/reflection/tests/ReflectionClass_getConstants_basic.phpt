--TEST--
ReflectionClass::getConstants()
--FILE--
<?php
class C {
	const a = 'hello from C';
}
class D extends C {
}
class E extends D {
}
class F extends E {
	const a = 'hello from F';
}
class X {
}

$classes = array('C', 'D', 'E', 'F', 'X');
foreach($classes as $class) {
	echo "Constants from class $class: \n";
	$rc = new ReflectionClass($class);
	var_dump($rc->getConstants());
}
?>
--EXPECTF--
Constants from class C: 
array(1) {
  [u"a"]=>
  unicode(12) "hello from C"
}
Constants from class D: 
array(1) {
  [u"a"]=>
  unicode(12) "hello from C"
}
Constants from class E: 
array(1) {
  [u"a"]=>
  unicode(12) "hello from C"
}
Constants from class F: 
array(1) {
  [u"a"]=>
  unicode(12) "hello from F"
}
Constants from class X: 
array(0) {
}
