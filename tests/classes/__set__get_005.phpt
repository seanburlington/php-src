--TEST--
ZE2 __set() and __get()
--SKIPIF--
<?php if (version_compare(zend_version(), '2.0.0-dev', '<')) die('skip ZendEngine 2 needed'); ?>
--FILE--
<?php
class Test
{
	protected $x;

	function __get($name) {
		echo __METHOD__ . "\n";
		if (isset($this->x[$name])) {
			return $this->x[$name];
		} 
		else
		{
			return NULL;
		}
	}

	function __set($name, $val) {
		echo __METHOD__ . "\n";
		$this->x[$name] = $val;
	}
}

class AutoGen
{
	protected $x;

	function __get($name) {
		echo __METHOD__ . "\n";
		if (!isset($this->x[$name])) {
			$this->x[$name] = new Test();
		}
		return $this->x[$name];
	}

	function __set($name, $val) {
		echo __METHOD__ . "\n";
		$this->x[$name] = $val;
	}
}

$foo = new AutoGen();
$foo->bar->baz = "Check";

var_dump($foo->bar);
var_dump($foo->bar->baz);

?>
===DONE===
--EXPECTF--
AutoGen::__get
Test::__set
AutoGen::__get
object(Test)#%d (1) {
  [u"x":protected]=>
  array(1) {
    [u"baz"]=>
    unicode(5) "Check"
  }
}
AutoGen::__get
Test::__get
unicode(5) "Check"
===DONE===
