--TEST--
Test count() function 
--SKIPIF--
<?php if (!extension_loaded("spl")) die("skip no SPL extension"); ?>
--FILE--
<?php
/* Prototype: int count ( mixed $var [, int $mode] );
   Discription: Count elements in an array, or properties in an object
*/

echo "*** Testing basic functionality of count() function ***\n";
print "-- Testing NULL --\n";
$arr = NULL;
print "COUNT_NORMAL: should be 0, is ".count($arr, COUNT_NORMAL)."\n";
print "COUNT_RECURSIVE: should be 0, is ".count($arr, COUNT_RECURSIVE)."\n";

print "-- Testing arrays --\n";
$arr = array(1, array(3, 4, array(6, array(8))));
print "COUNT_NORMAL: should be 2, is ".count($arr, COUNT_NORMAL)."\n";
print "COUNT_RECURSIVE: should be 8, is ".count($arr, COUNT_RECURSIVE)."\n";

print "-- Testing hashes --\n";
$arr = array("a" => 1, "b" => 2, array("c" => 3, array("d" => 5)));
print "COUNT_NORMAL: should be 3, is ".count($arr, COUNT_NORMAL)."\n";
print "COUNT_RECURSIVE: should be 6, is ".count($arr, COUNT_RECURSIVE)."\n";

print "-- Testing strings --\n";
print "COUNT_NORMAL: should be 1, is ".count("string", COUNT_NORMAL)."\n";
print "COUNT_RECURSIVE: should be 1, is ".count("string", COUNT_RECURSIVE)."\n";

print "-- Testing various types with no second argument --\n";
print "COUNT_NORMAL: should be 1, is ".count("string")."\n";
print "COUNT_NORMAL: should be 2, is ".count(array("a", array("b")))."\n";

$arr = array('a'=>array(NULL, NULL, NULL), 1=>array(NULL=>1, 1=>NULL),
	array(array(array(array(array(NULL))))));
print "-- Testing really cool arrays --\n";
print "COUNT_NORMAL: should be 3, is ".count($arr, COUNT_NORMAL)."\n";
print "COUNT_RECURSIVE: should be 13, is ".count($arr, COUNT_RECURSIVE)."\n";

echo "\n*** Testing possible variations of count() function on arrays ***";
$count_array = array(
  array(),
  array( 1 => "string"),
  array( "" => "string", 0 => "a", NULL => "b", -1.00 => "c",
         array(array(array(NULL)))),
  array( -2.44444 => 12, array(array(1, 2, array(array("0"))))),
  array( "a" => 1, "b" => -2.344, "b" => "string", "c" => NULL, "d" => -2.344),
  array( 4 => 1, 3 => -2.344, "3" => "string", "2" => NULL,
         1 => -2.344, array()),
  array( TRUE => TRUE, FALSE => FALSE, "" => "", " " => " ", 
	 NULL => NULL, "\x000" => "\x000", "\000" => "\000"),
  array( NULL, 1.23 => "Hi", "string" => "hello", 
         array("" => "World", "-2.34" => "a", "0" => "b"))
);

$i = 0;
foreach ($count_array as $count_value) {
  echo "\n-- Iteration $i --\n";
  print "COUNT_NORMAL is ".count($count_value, COUNT_NORMAL)."\n";
  print "COUNT_RECURSIVE is ".count($count_value, COUNT_RECURSIVE)."\n";  
  $i++;
}


/* Testing count() by passing constant with no second argument */
print "\n-- Testing count() on constants with no second argument --\n";
print "COUNT_NORMAL: should be 1, is ".count(100)."\n"; 
print "COUNT_NORMAL: should be 1, is ".count(-23.45)."\n";

print "\n-- Testing count() on NULL and Unset variables --\n";
print "COUNT_NORMAL: should be 0, is ".count(NULL)."\n";
print "COUNT_NORMAL: should be 1, is ".count("")."\n";
print "COUNT_NORMAL: should be 0, is ".@count($a)."\n";


print "\n-- Testing count() on an empty sub-array --\n";
$arr = array(1, array(3, 4, array()));
print "COUNT_NORMAL: should be 2, is ".count($arr, COUNT_NORMAL)."\n";
print "COUNT_RECURSIVE: should be 5, is ".count($arr, COUNT_RECURSIVE)."\n";

echo "\n-- Testing count() on objects with Countable interface --\n";
class count_class implements Countable {
  private $var_private;
  public $var_public;
  protected $var_protected;

  public function count() {
    return 3;
  }
}

$obj = new count_class();
print "COUNT_NORMAL: should be 3, is ".count($obj)."\n";


echo "\n-- Testing count() on resource type --\n";
$resource1 = fopen( __FILE__, "r" );  // Creating file(stream type) resource
$resource2 = opendir( "." );  // Creating dir resource

/* creating an array with resources as elements */
$arr_resource = array("a" => $resource1, "b" => $resource2);
var_dump(count($arr_resource));

echo "\n-- Testing count() on arrays containing references --\n";
$arr = array(1, array("a", "b", "c"));
$arr[2] = &$arr[1];

$mode_arr = array( COUNT_NORMAL, COUNT_RECURSIVE, 0, 1, -1, -1.45, 2, TRUE, 
                   FALSE, NULL);
for( $i =0; $i < count( $mode_arr ); $i++) {
  echo "For mode '$mode_arr[$i]' count is => ";
  var_dump(count($arr, $mode_arr[$i]));
}
  

echo "\n-- Testing error conditions --";
var_dump( count() );  // No. of args = 0
var_dump( count(array(), COUNT_NORMAL, 100) );  // No. of args > expected

/* Testing Invalid type arguments */
var_dump( count("string", ABCD) );
var_dump( count(100, "string") );
var_dump( count(array(), "") );

echo "\nDone";

--CLEAN--
/* closing the resource handles */
fclose( $resource1 );
closedir( $resource2 );
?>
--EXPECTF--
*** Testing basic functionality of count() function ***
-- Testing NULL --
COUNT_NORMAL: should be 0, is 0
COUNT_RECURSIVE: should be 0, is 0
-- Testing arrays --
COUNT_NORMAL: should be 2, is 2
COUNT_RECURSIVE: should be 8, is 8
-- Testing hashes --
COUNT_NORMAL: should be 3, is 3
COUNT_RECURSIVE: should be 6, is 6
-- Testing strings --
COUNT_NORMAL: should be 1, is 1
COUNT_RECURSIVE: should be 1, is 1
-- Testing various types with no second argument --
COUNT_NORMAL: should be 1, is 1
COUNT_NORMAL: should be 2, is 2
-- Testing really cool arrays --
COUNT_NORMAL: should be 3, is 3
COUNT_RECURSIVE: should be 13, is 13

*** Testing possible variations of count() function on arrays ***
-- Iteration 0 --
COUNT_NORMAL is 0
COUNT_RECURSIVE is 0

-- Iteration 1 --
COUNT_NORMAL is 1
COUNT_RECURSIVE is 1

-- Iteration 2 --
COUNT_NORMAL is 4
COUNT_RECURSIVE is 7

-- Iteration 3 --
COUNT_NORMAL is 2
COUNT_RECURSIVE is 8

-- Iteration 4 --
COUNT_NORMAL is 4
COUNT_RECURSIVE is 4

-- Iteration 5 --
COUNT_NORMAL is 5
COUNT_RECURSIVE is 5

-- Iteration 6 --
COUNT_NORMAL is 6
COUNT_RECURSIVE is 6

-- Iteration 7 --
COUNT_NORMAL is 4
COUNT_RECURSIVE is 7

-- Testing count() on constants with no second argument --
COUNT_NORMAL: should be 1, is 1
COUNT_NORMAL: should be 1, is 1

-- Testing count() on NULL and Unset variables --
COUNT_NORMAL: should be 0, is 0
COUNT_NORMAL: should be 1, is 1
COUNT_NORMAL: should be 0, is 0

-- Testing count() on an empty sub-array --
COUNT_NORMAL: should be 2, is 2
COUNT_RECURSIVE: should be 5, is 5

-- Testing count() on objects with Countable interface --
COUNT_NORMAL: should be 3, is 3

-- Testing count() on resource type --
int(2)

-- Testing count() on arrays containing references --
For mode '0' count is => int(3)
For mode '1' count is => int(9)
For mode '0' count is => int(3)
For mode '1' count is => int(9)
For mode '-1' count is => int(3)
For mode '-1.45' count is => int(3)
For mode '2' count is => int(3)
For mode '1' count is => int(9)
For mode '' count is => int(3)
For mode '' count is => int(3)

-- Testing error conditions --
Warning: count() expects at least 1 parameter, 0 given in %s on line %d
NULL

Warning: count() expects at most 2 parameters, 3 given in %s on line %d
NULL

Notice: Use of undefined constant ABCD - assumed 'ABCD' in %s on line %d

Warning: count() expects parameter 2 to be long, %s given in %s on line %d
NULL

Warning: count() expects parameter 2 to be long, %s given in %s on line %d
NULL

Warning: count() expects parameter 2 to be long, %s given in %s on line %d
NULL

Done
