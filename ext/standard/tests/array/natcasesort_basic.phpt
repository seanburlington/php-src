--TEST--
Test natcasesort() function : basic functionality 
--FILE--
<?php
/* Prototype  : bool natcasesort(array &$array_arg)
 * Description: Sort an array using case-insensitive natural sort
 * Source code: ext/standard/array.c
 */

/*
 * Test basic functionality of natcasesort()
 */

echo "*** Testing natcasesort() : basic functionality ***\n";

$array = array ('A01', 'a1', 'b10',  'a01', 'b01');
echo "\n-- Before sorting: --\n";
var_dump($array);

echo "\n-- After Sorting: --\n";
var_dump(natcasesort($array));
var_dump($array);

echo "Done";
?>
--EXPECTF--
*** Testing natcasesort() : basic functionality ***

-- Before sorting: --
array(5) {
  [0]=>
  unicode(3) "A01"
  [1]=>
  unicode(2) "a1"
  [2]=>
  unicode(3) "b10"
  [3]=>
  unicode(3) "a01"
  [4]=>
  unicode(3) "b01"
}

-- After Sorting: --
bool(true)
array(5) {
  [3]=>
  unicode(3) "a01"
  [0]=>
  unicode(3) "A01"
  [1]=>
  unicode(2) "a1"
  [4]=>
  unicode(3) "b01"
  [2]=>
  unicode(3) "b10"
}
Done