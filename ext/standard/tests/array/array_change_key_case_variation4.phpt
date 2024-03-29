--TEST--
Test array_change_key_case() function : usage variations - different int values for $case
--FILE--
<?php
/* Prototype  : array array_change_key_case(array $input [, int $case])
 * Description: Retuns an array with all string keys lowercased [or uppercased] 
 * Source code: ext/standard/array.c
 */

/*
 * Pass different integer values as $case argument to array_change_key_case() to test behaviour
 */

echo "*** Testing array_change_key_case() : usage variations ***\n";

$input = array('One' => 'un', 'TWO' => 'deux', 'three' => 'trois');
for ($i = -5; $i <=5; $i += 1){
	echo "\n-- \$sort argument is $i --\n";
	$temp = $input;
	var_dump(array_change_key_case($temp, $i));
}

echo "Done";
?>

--EXPECTF--
*** Testing array_change_key_case() : usage variations ***

-- $sort argument is -5 --
array(3) {
  [u"ONE"]=>
  unicode(2) "un"
  [u"TWO"]=>
  unicode(4) "deux"
  [u"THREE"]=>
  unicode(5) "trois"
}

-- $sort argument is -4 --
array(3) {
  [u"ONE"]=>
  unicode(2) "un"
  [u"TWO"]=>
  unicode(4) "deux"
  [u"THREE"]=>
  unicode(5) "trois"
}

-- $sort argument is -3 --
array(3) {
  [u"ONE"]=>
  unicode(2) "un"
  [u"TWO"]=>
  unicode(4) "deux"
  [u"THREE"]=>
  unicode(5) "trois"
}

-- $sort argument is -2 --
array(3) {
  [u"ONE"]=>
  unicode(2) "un"
  [u"TWO"]=>
  unicode(4) "deux"
  [u"THREE"]=>
  unicode(5) "trois"
}

-- $sort argument is -1 --
array(3) {
  [u"ONE"]=>
  unicode(2) "un"
  [u"TWO"]=>
  unicode(4) "deux"
  [u"THREE"]=>
  unicode(5) "trois"
}

-- $sort argument is 0 --
array(3) {
  [u"one"]=>
  unicode(2) "un"
  [u"two"]=>
  unicode(4) "deux"
  [u"three"]=>
  unicode(5) "trois"
}

-- $sort argument is 1 --
array(3) {
  [u"ONE"]=>
  unicode(2) "un"
  [u"TWO"]=>
  unicode(4) "deux"
  [u"THREE"]=>
  unicode(5) "trois"
}

-- $sort argument is 2 --
array(3) {
  [u"ONE"]=>
  unicode(2) "un"
  [u"TWO"]=>
  unicode(4) "deux"
  [u"THREE"]=>
  unicode(5) "trois"
}

-- $sort argument is 3 --
array(3) {
  [u"ONE"]=>
  unicode(2) "un"
  [u"TWO"]=>
  unicode(4) "deux"
  [u"THREE"]=>
  unicode(5) "trois"
}

-- $sort argument is 4 --
array(3) {
  [u"ONE"]=>
  unicode(2) "un"
  [u"TWO"]=>
  unicode(4) "deux"
  [u"THREE"]=>
  unicode(5) "trois"
}

-- $sort argument is 5 --
array(3) {
  [u"ONE"]=>
  unicode(2) "un"
  [u"TWO"]=>
  unicode(4) "deux"
  [u"THREE"]=>
  unicode(5) "trois"
}
Done
