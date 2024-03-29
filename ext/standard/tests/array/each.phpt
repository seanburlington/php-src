--TEST--
Test each() function
--FILE--
<?php
/* Return the current key and value pair from an array 
   and advance the array cursor */

echo "*** Testing each() : basic functionality ***\n";
$arrays = array (
	    array(0),
	    array(1),
	    array(-1),
	    array(1, 2, 3, 4, 5),
	    array(-1, -2, -3, 6, 7, 8),
 	    array("a", "ab", "abc", "abcd"),
	    array("1" => "one", "2" => "two", "3" => "three", "4" => "four"),
	    array("one" => 1, "two" => 2, 3 => "three", 4 => 4, "" => 5, 
		  "  " => 6, "\x00" => "\x000", "\0" => "\0", "" => "",
		  TRUE => TRUE, FALSE => FALSE, NULL => NULL
		 ),
	    array("1.5" => "one.5", "-2.0" => "negative2.0"),
	    array(-5 => "negative5", -.05  => "negative.05")
	  );

/* loop through to check working of each() on different arrays */
$i = 0;
while( list( $key, $sub_array) = each($arrays) )  {
  echo "-- Iteration $i --\n";
  $c = count ($sub_array);
  $c++; 		// increment by one to create the situation 
			// of accessing beyond array size
  while ( $c ) {
    var_dump( each($sub_array) );
    $c --;
  }
  /* assignment of an array to another variable resets the internal 
     pointer of the array. check this and ensure that each returns 
     the first element after the assignment */
  $new_array = $sub_array;
  var_dump( each($sub_array) );
  echo "\n";
  $i++;
}

echo "\n*** Testing each() : possible variations ***\n";
echo "-- Testing each() with reset() function --\n";
/* reset the $arrays and use each to get the first element */ 
var_dump( reset($arrays) );
var_dump( each($arrays) );  // first element
list($key, $sub_array) = each($arrays);  // now second element
var_dump( each($sub_array) );


echo "-- Testing each() with resources --\n";
$fp = fopen(__FILE__, "r");
$dfp = opendir(".");

$resources = array (
  "file" => $fp,
  "dir" => $dfp
);

for( $i = 0; $i < count($resources); $i++) {
  var_dump( each($resources) );
}

echo "-- Testing each with objects --\n";

class each_class {
  private $var_private = 100;
  protected $var_protected = "string";
  public $var_public = array(0, 1, TRUE, NULL);
}
$each_obj = new each_class();
for( $i = 0; $i <= 2; $i++ ) {
  var_dump( each($each_obj) );
}

echo "-- Testing each() with null array --\n";
$null_array = array();
var_dump( each($null_array) );


echo "\n*** Testing error conditions ***\n";

/* unexpected number of arguments */
var_dump( each() );  // args = 0
var_dump( each($null_array, $null_array) );  // args > expected

/* unexpected argument type */
$var=1;
$str ="string";
$fl = "15.5";
var_dump( each($var) );
var_dump( each($str) );
var_dump( each($fl) );

// close resourses used
fclose($fp);
closedir($dfp);

echo "Done\n";
?>

--EXPECTF--
*** Testing each() : basic functionality ***
-- Iteration 0 --
array(4) {
  [1]=>
  int(0)
  [u"value"]=>
  int(0)
  [0]=>
  int(0)
  [u"key"]=>
  int(0)
}
bool(false)
array(4) {
  [1]=>
  int(0)
  [u"value"]=>
  int(0)
  [0]=>
  int(0)
  [u"key"]=>
  int(0)
}

-- Iteration 1 --
array(4) {
  [1]=>
  int(1)
  [u"value"]=>
  int(1)
  [0]=>
  int(0)
  [u"key"]=>
  int(0)
}
bool(false)
array(4) {
  [1]=>
  int(1)
  [u"value"]=>
  int(1)
  [0]=>
  int(0)
  [u"key"]=>
  int(0)
}

-- Iteration 2 --
array(4) {
  [1]=>
  int(-1)
  [u"value"]=>
  int(-1)
  [0]=>
  int(0)
  [u"key"]=>
  int(0)
}
bool(false)
array(4) {
  [1]=>
  int(-1)
  [u"value"]=>
  int(-1)
  [0]=>
  int(0)
  [u"key"]=>
  int(0)
}

-- Iteration 3 --
array(4) {
  [1]=>
  int(1)
  [u"value"]=>
  int(1)
  [0]=>
  int(0)
  [u"key"]=>
  int(0)
}
array(4) {
  [1]=>
  int(2)
  [u"value"]=>
  int(2)
  [0]=>
  int(1)
  [u"key"]=>
  int(1)
}
array(4) {
  [1]=>
  int(3)
  [u"value"]=>
  int(3)
  [0]=>
  int(2)
  [u"key"]=>
  int(2)
}
array(4) {
  [1]=>
  int(4)
  [u"value"]=>
  int(4)
  [0]=>
  int(3)
  [u"key"]=>
  int(3)
}
array(4) {
  [1]=>
  int(5)
  [u"value"]=>
  int(5)
  [0]=>
  int(4)
  [u"key"]=>
  int(4)
}
bool(false)
array(4) {
  [1]=>
  int(1)
  [u"value"]=>
  int(1)
  [0]=>
  int(0)
  [u"key"]=>
  int(0)
}

-- Iteration 4 --
array(4) {
  [1]=>
  int(-1)
  [u"value"]=>
  int(-1)
  [0]=>
  int(0)
  [u"key"]=>
  int(0)
}
array(4) {
  [1]=>
  int(-2)
  [u"value"]=>
  int(-2)
  [0]=>
  int(1)
  [u"key"]=>
  int(1)
}
array(4) {
  [1]=>
  int(-3)
  [u"value"]=>
  int(-3)
  [0]=>
  int(2)
  [u"key"]=>
  int(2)
}
array(4) {
  [1]=>
  int(6)
  [u"value"]=>
  int(6)
  [0]=>
  int(3)
  [u"key"]=>
  int(3)
}
array(4) {
  [1]=>
  int(7)
  [u"value"]=>
  int(7)
  [0]=>
  int(4)
  [u"key"]=>
  int(4)
}
array(4) {
  [1]=>
  int(8)
  [u"value"]=>
  int(8)
  [0]=>
  int(5)
  [u"key"]=>
  int(5)
}
bool(false)
array(4) {
  [1]=>
  int(-1)
  [u"value"]=>
  int(-1)
  [0]=>
  int(0)
  [u"key"]=>
  int(0)
}

-- Iteration 5 --
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
array(4) {
  [1]=>
  unicode(2) "ab"
  [u"value"]=>
  unicode(2) "ab"
  [0]=>
  int(1)
  [u"key"]=>
  int(1)
}
array(4) {
  [1]=>
  unicode(3) "abc"
  [u"value"]=>
  unicode(3) "abc"
  [0]=>
  int(2)
  [u"key"]=>
  int(2)
}
array(4) {
  [1]=>
  unicode(4) "abcd"
  [u"value"]=>
  unicode(4) "abcd"
  [0]=>
  int(3)
  [u"key"]=>
  int(3)
}
bool(false)
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

-- Iteration 6 --
array(4) {
  [1]=>
  unicode(3) "one"
  [u"value"]=>
  unicode(3) "one"
  [0]=>
  int(1)
  [u"key"]=>
  int(1)
}
array(4) {
  [1]=>
  unicode(3) "two"
  [u"value"]=>
  unicode(3) "two"
  [0]=>
  int(2)
  [u"key"]=>
  int(2)
}
array(4) {
  [1]=>
  unicode(5) "three"
  [u"value"]=>
  unicode(5) "three"
  [0]=>
  int(3)
  [u"key"]=>
  int(3)
}
array(4) {
  [1]=>
  unicode(4) "four"
  [u"value"]=>
  unicode(4) "four"
  [0]=>
  int(4)
  [u"key"]=>
  int(4)
}
bool(false)
array(4) {
  [1]=>
  unicode(3) "one"
  [u"value"]=>
  unicode(3) "one"
  [0]=>
  int(1)
  [u"key"]=>
  int(1)
}

-- Iteration 7 --
array(4) {
  [1]=>
  int(1)
  [u"value"]=>
  int(1)
  [0]=>
  unicode(3) "one"
  [u"key"]=>
  unicode(3) "one"
}
array(4) {
  [1]=>
  int(2)
  [u"value"]=>
  int(2)
  [0]=>
  unicode(3) "two"
  [u"key"]=>
  unicode(3) "two"
}
array(4) {
  [1]=>
  unicode(5) "three"
  [u"value"]=>
  unicode(5) "three"
  [0]=>
  int(3)
  [u"key"]=>
  int(3)
}
array(4) {
  [1]=>
  int(4)
  [u"value"]=>
  int(4)
  [0]=>
  int(4)
  [u"key"]=>
  int(4)
}
array(4) {
  [1]=>
  NULL
  [u"value"]=>
  NULL
  [0]=>
  unicode(0) ""
  [u"key"]=>
  unicode(0) ""
}
array(4) {
  [1]=>
  int(6)
  [u"value"]=>
  int(6)
  [0]=>
  unicode(2) "  "
  [u"key"]=>
  unicode(2) "  "
}
array(4) {
  [1]=>
  unicode(1) " "
  [u"value"]=>
  unicode(1) " "
  [0]=>
  unicode(1) " "
  [u"key"]=>
  unicode(1) " "
}
array(4) {
  [1]=>
  bool(true)
  [u"value"]=>
  bool(true)
  [0]=>
  int(1)
  [u"key"]=>
  int(1)
}
array(4) {
  [1]=>
  bool(false)
  [u"value"]=>
  bool(false)
  [0]=>
  int(0)
  [u"key"]=>
  int(0)
}
bool(false)
array(4) {
  [1]=>
  int(1)
  [u"value"]=>
  int(1)
  [0]=>
  unicode(3) "one"
  [u"key"]=>
  unicode(3) "one"
}

-- Iteration 8 --
array(4) {
  [1]=>
  unicode(5) "one.5"
  [u"value"]=>
  unicode(5) "one.5"
  [0]=>
  unicode(3) "1.5"
  [u"key"]=>
  unicode(3) "1.5"
}
array(4) {
  [1]=>
  unicode(11) "negative2.0"
  [u"value"]=>
  unicode(11) "negative2.0"
  [0]=>
  unicode(4) "-2.0"
  [u"key"]=>
  unicode(4) "-2.0"
}
bool(false)
array(4) {
  [1]=>
  unicode(5) "one.5"
  [u"value"]=>
  unicode(5) "one.5"
  [0]=>
  unicode(3) "1.5"
  [u"key"]=>
  unicode(3) "1.5"
}

-- Iteration 9 --
array(4) {
  [1]=>
  unicode(9) "negative5"
  [u"value"]=>
  unicode(9) "negative5"
  [0]=>
  int(-5)
  [u"key"]=>
  int(-5)
}
array(4) {
  [1]=>
  unicode(11) "negative.05"
  [u"value"]=>
  unicode(11) "negative.05"
  [0]=>
  int(0)
  [u"key"]=>
  int(0)
}
bool(false)
array(4) {
  [1]=>
  unicode(9) "negative5"
  [u"value"]=>
  unicode(9) "negative5"
  [0]=>
  int(-5)
  [u"key"]=>
  int(-5)
}


*** Testing each() : possible variations ***
-- Testing each() with reset() function --
array(1) {
  [0]=>
  int(0)
}
array(4) {
  [1]=>
  array(1) {
    [0]=>
    int(0)
  }
  [u"value"]=>
  array(1) {
    [0]=>
    int(0)
  }
  [0]=>
  int(0)
  [u"key"]=>
  int(0)
}
array(4) {
  [1]=>
  int(1)
  [u"value"]=>
  int(1)
  [0]=>
  int(0)
  [u"key"]=>
  int(0)
}
-- Testing each() with resources --
array(4) {
  [1]=>
  resource(5) of type (stream)
  [u"value"]=>
  resource(5) of type (stream)
  [0]=>
  unicode(4) "file"
  [u"key"]=>
  unicode(4) "file"
}
array(4) {
  [1]=>
  resource(6) of type (stream)
  [u"value"]=>
  resource(6) of type (stream)
  [0]=>
  unicode(3) "dir"
  [u"key"]=>
  unicode(3) "dir"
}
-- Testing each with objects --
array(4) {
  [1]=>
  int(100)
  [u"value"]=>
  int(100)
  [0]=>
  unicode(23) " each_class var_private"
  [u"key"]=>
  unicode(23) " each_class var_private"
}
array(4) {
  [1]=>
  unicode(6) "string"
  [u"value"]=>
  unicode(6) "string"
  [0]=>
  unicode(16) " * var_protected"
  [u"key"]=>
  unicode(16) " * var_protected"
}
array(4) {
  [1]=>
  array(4) {
    [0]=>
    int(0)
    [1]=>
    int(1)
    [2]=>
    bool(true)
    [3]=>
    NULL
  }
  [u"value"]=>
  array(4) {
    [0]=>
    int(0)
    [1]=>
    int(1)
    [2]=>
    bool(true)
    [3]=>
    NULL
  }
  [0]=>
  unicode(10) "var_public"
  [u"key"]=>
  unicode(10) "var_public"
}
-- Testing each() with null array --
bool(false)

*** Testing error conditions ***

Warning: each() expects exactly 1 parameter, 0 given in %s on line %d
NULL

Warning: each() expects exactly 1 parameter, 2 given in %s on line %d
NULL

Warning: Variable passed to each() is not an array or object in %s on line %d
NULL

Warning: Variable passed to each() is not an array or object in %s on line %d
NULL

Warning: Variable passed to each() is not an array or object in %s on line %d
NULL
Done
