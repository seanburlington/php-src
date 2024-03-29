--TEST--
Test sprintf() function : usage variations - scientific formats with array values
--FILE--
<?php
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/

echo "*** Testing sprintf() : scientific formats with array values ***\n";

// array of array values 
$array_values = array(
  array(),
  array(0),
  array(1),
  array(100000000000),
  array(.0000001),
  array(10e2),
  array(NULL),
  array(null),
  array("string"),
  array(true),
  array(TRUE),
  array(false),
  array(FALSE),
  array(1,2,3,4),
  array(1 => "One", "two" => 2)
);

// array of scientific formats
$scientific_formats = array( 
  "%e", "%he", "%le",
  "%Le", " %e", "%e ",
  "\t%e", "\n%e", "%4e", 
  "%30e", "%[0-1]", "%*e"
);

$count = 1;
foreach($array_values as $array_value) {
  echo "\n-- Iteration $count --\n";
  
  foreach($scientific_formats as $format) {
    var_dump( sprintf($format, $array_value) );
  }
  $count++;
};

echo "Done";
?>
--EXPECT--
*** Testing sprintf() : scientific formats with array values ***

-- Iteration 1 --
unicode(11) "0.000000e+0"
unicode(1) "e"
unicode(11) "0.000000e+0"
unicode(1) "e"
unicode(12) " 0.000000e+0"
unicode(12) "0.000000e+0 "
unicode(12) "	0.000000e+0"
unicode(12) "
0.000000e+0"
unicode(11) "0.000000e+0"
unicode(30) "                   0.000000e+0"
unicode(4) "0-1]"
unicode(1) "e"

-- Iteration 2 --
unicode(11) "1.000000e+0"
unicode(1) "e"
unicode(11) "1.000000e+0"
unicode(1) "e"
unicode(12) " 1.000000e+0"
unicode(12) "1.000000e+0 "
unicode(12) "	1.000000e+0"
unicode(12) "
1.000000e+0"
unicode(11) "1.000000e+0"
unicode(30) "                   1.000000e+0"
unicode(4) "0-1]"
unicode(1) "e"

-- Iteration 3 --
unicode(11) "1.000000e+0"
unicode(1) "e"
unicode(11) "1.000000e+0"
unicode(1) "e"
unicode(12) " 1.000000e+0"
unicode(12) "1.000000e+0 "
unicode(12) "	1.000000e+0"
unicode(12) "
1.000000e+0"
unicode(11) "1.000000e+0"
unicode(30) "                   1.000000e+0"
unicode(4) "0-1]"
unicode(1) "e"

-- Iteration 4 --
unicode(11) "1.000000e+0"
unicode(1) "e"
unicode(11) "1.000000e+0"
unicode(1) "e"
unicode(12) " 1.000000e+0"
unicode(12) "1.000000e+0 "
unicode(12) "	1.000000e+0"
unicode(12) "
1.000000e+0"
unicode(11) "1.000000e+0"
unicode(30) "                   1.000000e+0"
unicode(4) "0-1]"
unicode(1) "e"

-- Iteration 5 --
unicode(11) "1.000000e+0"
unicode(1) "e"
unicode(11) "1.000000e+0"
unicode(1) "e"
unicode(12) " 1.000000e+0"
unicode(12) "1.000000e+0 "
unicode(12) "	1.000000e+0"
unicode(12) "
1.000000e+0"
unicode(11) "1.000000e+0"
unicode(30) "                   1.000000e+0"
unicode(4) "0-1]"
unicode(1) "e"

-- Iteration 6 --
unicode(11) "1.000000e+0"
unicode(1) "e"
unicode(11) "1.000000e+0"
unicode(1) "e"
unicode(12) " 1.000000e+0"
unicode(12) "1.000000e+0 "
unicode(12) "	1.000000e+0"
unicode(12) "
1.000000e+0"
unicode(11) "1.000000e+0"
unicode(30) "                   1.000000e+0"
unicode(4) "0-1]"
unicode(1) "e"

-- Iteration 7 --
unicode(11) "1.000000e+0"
unicode(1) "e"
unicode(11) "1.000000e+0"
unicode(1) "e"
unicode(12) " 1.000000e+0"
unicode(12) "1.000000e+0 "
unicode(12) "	1.000000e+0"
unicode(12) "
1.000000e+0"
unicode(11) "1.000000e+0"
unicode(30) "                   1.000000e+0"
unicode(4) "0-1]"
unicode(1) "e"

-- Iteration 8 --
unicode(11) "1.000000e+0"
unicode(1) "e"
unicode(11) "1.000000e+0"
unicode(1) "e"
unicode(12) " 1.000000e+0"
unicode(12) "1.000000e+0 "
unicode(12) "	1.000000e+0"
unicode(12) "
1.000000e+0"
unicode(11) "1.000000e+0"
unicode(30) "                   1.000000e+0"
unicode(4) "0-1]"
unicode(1) "e"

-- Iteration 9 --
unicode(11) "1.000000e+0"
unicode(1) "e"
unicode(11) "1.000000e+0"
unicode(1) "e"
unicode(12) " 1.000000e+0"
unicode(12) "1.000000e+0 "
unicode(12) "	1.000000e+0"
unicode(12) "
1.000000e+0"
unicode(11) "1.000000e+0"
unicode(30) "                   1.000000e+0"
unicode(4) "0-1]"
unicode(1) "e"

-- Iteration 10 --
unicode(11) "1.000000e+0"
unicode(1) "e"
unicode(11) "1.000000e+0"
unicode(1) "e"
unicode(12) " 1.000000e+0"
unicode(12) "1.000000e+0 "
unicode(12) "	1.000000e+0"
unicode(12) "
1.000000e+0"
unicode(11) "1.000000e+0"
unicode(30) "                   1.000000e+0"
unicode(4) "0-1]"
unicode(1) "e"

-- Iteration 11 --
unicode(11) "1.000000e+0"
unicode(1) "e"
unicode(11) "1.000000e+0"
unicode(1) "e"
unicode(12) " 1.000000e+0"
unicode(12) "1.000000e+0 "
unicode(12) "	1.000000e+0"
unicode(12) "
1.000000e+0"
unicode(11) "1.000000e+0"
unicode(30) "                   1.000000e+0"
unicode(4) "0-1]"
unicode(1) "e"

-- Iteration 12 --
unicode(11) "1.000000e+0"
unicode(1) "e"
unicode(11) "1.000000e+0"
unicode(1) "e"
unicode(12) " 1.000000e+0"
unicode(12) "1.000000e+0 "
unicode(12) "	1.000000e+0"
unicode(12) "
1.000000e+0"
unicode(11) "1.000000e+0"
unicode(30) "                   1.000000e+0"
unicode(4) "0-1]"
unicode(1) "e"

-- Iteration 13 --
unicode(11) "1.000000e+0"
unicode(1) "e"
unicode(11) "1.000000e+0"
unicode(1) "e"
unicode(12) " 1.000000e+0"
unicode(12) "1.000000e+0 "
unicode(12) "	1.000000e+0"
unicode(12) "
1.000000e+0"
unicode(11) "1.000000e+0"
unicode(30) "                   1.000000e+0"
unicode(4) "0-1]"
unicode(1) "e"

-- Iteration 14 --
unicode(11) "1.000000e+0"
unicode(1) "e"
unicode(11) "1.000000e+0"
unicode(1) "e"
unicode(12) " 1.000000e+0"
unicode(12) "1.000000e+0 "
unicode(12) "	1.000000e+0"
unicode(12) "
1.000000e+0"
unicode(11) "1.000000e+0"
unicode(30) "                   1.000000e+0"
unicode(4) "0-1]"
unicode(1) "e"

-- Iteration 15 --
unicode(11) "1.000000e+0"
unicode(1) "e"
unicode(11) "1.000000e+0"
unicode(1) "e"
unicode(12) " 1.000000e+0"
unicode(12) "1.000000e+0 "
unicode(12) "	1.000000e+0"
unicode(12) "
1.000000e+0"
unicode(11) "1.000000e+0"
unicode(30) "                   1.000000e+0"
unicode(4) "0-1]"
unicode(1) "e"
Done
