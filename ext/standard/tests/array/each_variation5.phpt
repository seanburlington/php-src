--TEST--
Test each() function : usage variations - Multi-dimensional arrays
--FILE--
<?php
/* Prototype  : array each(array $arr)
 * Description: Return the currently pointed key..value pair in the passed array,
 * and advance the pointer to the next element 
 * Source code: Zend/zend_builtin_functions.c
 */

/*
 * Test behaviour of each() when passed:
 * 1. a two-dimensional array
 * 2. a sub-array
 */

echo "*** Testing each() : usage variations ***\n";

$arr = array ('zero',
              array(1, 2, 3),
              'one' => 'un',
              array('a', 'b', 'c')
              );

echo "\n-- Pass each() a two-dimensional array --\n";
for ($i = 1; $i < count($arr); $i++) {
	var_dump( each($arr) );
}

echo "\n-- Pass each() a sub-array --\n";
var_dump( each($arr[2]));

echo "Done";
?>

--EXPECTF--
*** Testing each() : usage variations ***

-- Pass each() a two-dimensional array --
array(4) {
  [1]=>
  unicode(4) "zero"
  [u"value"]=>
  unicode(4) "zero"
  [0]=>
  int(0)
  [u"key"]=>
  int(0)
}
array(4) {
  [1]=>
  array(3) {
    [0]=>
    int(1)
    [1]=>
    int(2)
    [2]=>
    int(3)
  }
  [u"value"]=>
  array(3) {
    [0]=>
    int(1)
    [1]=>
    int(2)
    [2]=>
    int(3)
  }
  [0]=>
  int(1)
  [u"key"]=>
  int(1)
}
array(4) {
  [1]=>
  unicode(2) "un"
  [u"value"]=>
  unicode(2) "un"
  [0]=>
  unicode(3) "one"
  [u"key"]=>
  unicode(3) "one"
}

-- Pass each() a sub-array --
array(4) {
  [1]=>
  unicode(1) "a"
  [u"value"]=>
  unicode(1) "a"
  [0]=>
  int(0)
  [u"key"]=>
  int(0)
}
Done