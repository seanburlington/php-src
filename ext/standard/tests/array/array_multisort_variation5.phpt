--TEST--
Test array_multisort() function : usage variation - testing with multiple array arguments 
--FILE--
<?php
/* Prototype  : bool array_multisort(array ar1 [, SORT_ASC|SORT_DESC [, SORT_REGULAR|SORT_NUMERIC|SORT_STRING]] [, array ar2 [, SORT_ASC|SORT_DESC [, SORT_REGULAR|SORT_NUMERIC|SORT_STRING]], ...])
 * Description: Sort multiple arrays at once similar to how ORDER BY clause works in SQL 
 * Source code: ext/standard/array.c
 * Alias to functions: 
 */

echo "*** Testing array_multisort() : Testing  all array sort specifiers ***\n";

$ar = array( 2, "aa" , "1");

array_multisort($ar, SORT_REGULAR, SORT_ASC);
var_dump($ar);

array_multisort($ar, SORT_STRING, SORT_ASC);
var_dump($ar);

array_multisort($ar, SORT_NUMERIC, SORT_ASC);
var_dump($ar);


?>
===DONE===
--EXPECTF--
*** Testing array_multisort() : Testing  all array sort specifiers ***
array(3) {
  [0]=>
  unicode(1) "1"
  [1]=>
  unicode(2) "aa"
  [2]=>
  int(2)
}
array(3) {
  [0]=>
  unicode(1) "1"
  [1]=>
  int(2)
  [2]=>
  unicode(2) "aa"
}
array(3) {
  [0]=>
  unicode(2) "aa"
  [1]=>
  unicode(1) "1"
  [2]=>
  int(2)
}
===DONE===
