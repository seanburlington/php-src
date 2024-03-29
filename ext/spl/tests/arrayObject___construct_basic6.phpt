--TEST--
SPL: ArrayObject::__construct: check impact of ArrayObject::STD_PROP_LIST on var_dump.
--FILE--
<?php
class MyArrayObject extends ArrayObject {
	private $priv1 = 'secret1';
	public $pub1 = 'public1';
} 

$ao = new ArrayObject(array(1,2,3));
$ao->p = 1;
var_dump($ao);

$ao = new ArrayObject(array(1,2,3), ArrayObject::STD_PROP_LIST);
$ao->p = 1;
var_dump($ao);

$ao = new MyArrayObject(array(1,2,3));
var_dump($ao);

$ao = new MyArrayObject(array(1,2,3), ArrayObject::STD_PROP_LIST);
var_dump($ao);
?>
--EXPECTF--
object(ArrayObject)#1 (2) {
  [u"p"]=>
  int(1)
  [u"storage":u"ArrayObject":private]=>
  array(3) {
    [0]=>
    int(1)
    [1]=>
    int(2)
    [2]=>
    int(3)
  }
}
object(ArrayObject)#2 (2) {
  [u"p"]=>
  int(1)
  [u"storage":u"ArrayObject":private]=>
  array(3) {
    [0]=>
    int(1)
    [1]=>
    int(2)
    [2]=>
    int(3)
  }
}
object(MyArrayObject)#1 (3) {
  [u"priv1":u"MyArrayObject":private]=>
  unicode(7) "secret1"
  [u"pub1"]=>
  unicode(7) "public1"
  [u"storage":u"ArrayObject":private]=>
  array(3) {
    [0]=>
    int(1)
    [1]=>
    int(2)
    [2]=>
    int(3)
  }
}
object(MyArrayObject)#2 (3) {
  [u"priv1":u"MyArrayObject":private]=>
  unicode(7) "secret1"
  [u"pub1"]=>
  unicode(7) "public1"
  [u"storage":u"ArrayObject":private]=>
  array(3) {
    [0]=>
    int(1)
    [1]=>
    int(2)
    [2]=>
    int(3)
  }
}
