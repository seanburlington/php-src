--TEST--
SPL: FixedArray: overloading
--INI--
allow_call_time_pass_reference=1
--FILE--
<?php
class A extends SplFixedArray {
    public function count() {
        return 2;
    }

    public function offsetGet($n) {
        echo "A::offsetGet\n";
        return parent::offsetGet($n);
    }
    public function offsetSet($n, $v) {
        echo "A::offsetSet\n";
        return parent::offsetSet($n, $v);
    }
    public function offsetUnset($n) {
        echo "A::offsetUnset\n";
        return parent::offsetUnset($n);
    }
    public function offsetExists($n) {
        echo "A::offsetExists\n";
        return parent::offsetExists($n);
    }
}

$a = new A;

// errors
try {
    $a[0] = "value1";
} catch (RuntimeException $e) {
    echo "Exception: ".$e->getMessage()."\n";
}
try {
    var_dump($a["asdf"]);
} catch (RuntimeException $e) {
    echo "Exception: ".$e->getMessage()."\n";
}
try {
    unset($a[-1]);
} catch (RuntimeException $e) {
    echo "Exception: ".$e->getMessage()."\n";
}
$a->setSize(10);


$a[0] = "value0";
$a[1] = "value1";
$a[2] = "value2";
$a[3] = "value3";
$ref = "value4";
$ref2 =&$ref;
$a[4] = $ref;
$ref = "value5";

unset($a[1]);
var_dump(isset($a[1]), isset($a[2]), empty($a[1]), empty($a[2]));

var_dump($a[0], $a[2], $a[3], $a[4]);

// countable

var_dump(count($a), $a->getSize(), count($a) == $a->getSize());
?>
===DONE===
--EXPECTF--
A::offsetSet
Exception: Index invalid or out of range
A::offsetGet
Exception: Index invalid or out of range
A::offsetUnset
Exception: Index invalid or out of range
A::offsetSet
A::offsetSet
A::offsetSet
A::offsetSet
A::offsetSet
A::offsetUnset
A::offsetExists
A::offsetExists
A::offsetExists
A::offsetExists
bool(false)
bool(true)
bool(true)
bool(false)
A::offsetGet
A::offsetGet
A::offsetGet
A::offsetGet
unicode(6) "value0"
unicode(6) "value2"
unicode(6) "value3"
unicode(6) "value4"
int(2)
int(10)
bool(false)
===DONE===
