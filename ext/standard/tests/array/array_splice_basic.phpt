--TEST--
Test array_splice(): basic functionality
--FILE--
<?php
/* 
 * proto array array_splice(array input, int offset [, int length [, array replacement]])
 * Function is implemented in ext/standard/array.c
*/ 

echo "*** Testing array_splice() basic operations ***\n";
echo "test truncation \n";
$input = array("red", "green", "blue", "yellow");
var_dump (array_splice($input, 2));
var_dump ($input);
// $input is now array("red", "green")

echo "test removing entries from the middle \n";
$input = array("red", "green", "blue", "yellow");
var_dump (array_splice($input, 1, -1));
var_dump ($input);
// $input is now array("red", "yellow")

echo "test substitution at end \n";
$input = array("red", "green", "blue", "yellow");
var_dump (array_splice($input, 1, count($input), "orange"));
var_dump ($input);
// $input is now array("red", "orange")

$input = array("red", "green", "blue", "yellow");
var_dump (array_splice($input, -1, 1, array("black", "maroon")));
var_dump ($input);
// $input is now array("red", "green",
//          "blue", "black", "maroon")

echo "test insertion \n";
$input = array("red", "green", "blue", "yellow");
var_dump (array_splice($input, 3, 0, "purple"));
var_dump ($input);
// $input is now array("red", "green",
//          "blue", "purple", "yellow");


?>
--EXPECT--
*** Testing array_splice() basic operations ***
test truncation 
array(2) {
  [0]=>
  unicode(4) "blue"
  [1]=>
  unicode(6) "yellow"
}
array(2) {
  [0]=>
  unicode(3) "red"
  [1]=>
  unicode(5) "green"
}
test removing entries from the middle 
array(2) {
  [0]=>
  unicode(5) "green"
  [1]=>
  unicode(4) "blue"
}
array(2) {
  [0]=>
  unicode(3) "red"
  [1]=>
  unicode(6) "yellow"
}
test substitution at end 
array(3) {
  [0]=>
  unicode(5) "green"
  [1]=>
  unicode(4) "blue"
  [2]=>
  unicode(6) "yellow"
}
array(2) {
  [0]=>
  unicode(3) "red"
  [1]=>
  unicode(6) "orange"
}
array(1) {
  [0]=>
  unicode(6) "yellow"
}
array(5) {
  [0]=>
  unicode(3) "red"
  [1]=>
  unicode(5) "green"
  [2]=>
  unicode(4) "blue"
  [3]=>
  unicode(5) "black"
  [4]=>
  unicode(6) "maroon"
}
test insertion 
array(0) {
}
array(5) {
  [0]=>
  unicode(3) "red"
  [1]=>
  unicode(5) "green"
  [2]=>
  unicode(4) "blue"
  [3]=>
  unicode(6) "purple"
  [4]=>
  unicode(6) "yellow"
}
