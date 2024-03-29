--TEST--
ReflectionClass::getDefaultProperties (filtering parent privates)
--SKIPIF--
<?php extension_loaded('reflection') or die('skip'); ?>
--FILE--
<?php
class C1 {
	private   $p1 = 1;
	protected $p2 = 2;
	public    $p3 = 3;
}
class C2 extends C1 {
	private   $p4 = 4;
	protected $p5 = 5;
	public    $p6 = 6;
}
$class = new ReflectionClass("C2");
var_dump($class->getDefaultProperties());
?>
--EXPECT--	
array(5) {
  [u"p4"]=>
  int(4)
  [u"p5"]=>
  int(5)
  [u"p6"]=>
  int(6)
  [u"p2"]=>
  int(2)
  [u"p3"]=>
  int(3)
}
