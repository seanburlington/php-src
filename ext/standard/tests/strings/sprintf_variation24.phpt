--TEST--
Test sprintf() function : usage variations - char formats with array values
--FILE--
<?php
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/

echo "*** Testing sprintf() : char formats with array values ***\n";

// array of array values 
$array_values = array(
  array(),
  array(0),
  array(1),
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

// array of char formats
$char_formats = array( 
  "%c", "%hc", "%lc", 
  "%Lc", " %c", "%c ",
  "\t%c", "\n%c", "%4c",
  "%30c", "%[a-bA-B@#$&]", "%*c"
);

$count = 1;
foreach($array_values as $array_value) {
  echo "\n-- Iteration $count --\n";
  
  foreach($char_formats as $format) {
    var_dump( sprintf($format, $array_value) );
  }
  $count++;
};

echo "Done";
?>
--EXPECT--
*** Testing sprintf() : char formats with array values ***

-- Iteration 1 --
unicode(1) " "
unicode(1) "c"
unicode(1) " "
unicode(1) "c"
unicode(2) "  "
unicode(2) "  "
unicode(2) "	 "
unicode(2) "
 "
unicode(1) " "
unicode(1) " "
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 2 --
unicode(1) ""
unicode(1) "c"
unicode(1) ""
unicode(1) "c"
unicode(2) " "
unicode(2) " "
unicode(2) "	"
unicode(2) "
"
unicode(1) ""
unicode(1) ""
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 3 --
unicode(1) ""
unicode(1) "c"
unicode(1) ""
unicode(1) "c"
unicode(2) " "
unicode(2) " "
unicode(2) "	"
unicode(2) "
"
unicode(1) ""
unicode(1) ""
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 4 --
unicode(1) ""
unicode(1) "c"
unicode(1) ""
unicode(1) "c"
unicode(2) " "
unicode(2) " "
unicode(2) "	"
unicode(2) "
"
unicode(1) ""
unicode(1) ""
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 5 --
unicode(1) ""
unicode(1) "c"
unicode(1) ""
unicode(1) "c"
unicode(2) " "
unicode(2) " "
unicode(2) "	"
unicode(2) "
"
unicode(1) ""
unicode(1) ""
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 6 --
unicode(1) ""
unicode(1) "c"
unicode(1) ""
unicode(1) "c"
unicode(2) " "
unicode(2) " "
unicode(2) "	"
unicode(2) "
"
unicode(1) ""
unicode(1) ""
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 7 --
unicode(1) ""
unicode(1) "c"
unicode(1) ""
unicode(1) "c"
unicode(2) " "
unicode(2) " "
unicode(2) "	"
unicode(2) "
"
unicode(1) ""
unicode(1) ""
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 8 --
unicode(1) ""
unicode(1) "c"
unicode(1) ""
unicode(1) "c"
unicode(2) " "
unicode(2) " "
unicode(2) "	"
unicode(2) "
"
unicode(1) ""
unicode(1) ""
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 9 --
unicode(1) ""
unicode(1) "c"
unicode(1) ""
unicode(1) "c"
unicode(2) " "
unicode(2) " "
unicode(2) "	"
unicode(2) "
"
unicode(1) ""
unicode(1) ""
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 10 --
unicode(1) ""
unicode(1) "c"
unicode(1) ""
unicode(1) "c"
unicode(2) " "
unicode(2) " "
unicode(2) "	"
unicode(2) "
"
unicode(1) ""
unicode(1) ""
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 11 --
unicode(1) ""
unicode(1) "c"
unicode(1) ""
unicode(1) "c"
unicode(2) " "
unicode(2) " "
unicode(2) "	"
unicode(2) "
"
unicode(1) ""
unicode(1) ""
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 12 --
unicode(1) ""
unicode(1) "c"
unicode(1) ""
unicode(1) "c"
unicode(2) " "
unicode(2) " "
unicode(2) "	"
unicode(2) "
"
unicode(1) ""
unicode(1) ""
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"
Done
