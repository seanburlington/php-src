--TEST--
Test sprintf() function : usage variations - char formats with float values
--FILE--
<?php
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/

echo "*** Testing sprintf() : char formats with float values ***\n";

// array of float values 
$float_values = array(
  0.0,
  -0.1,
  1.0,
  1e5,
  -1e6,
  1E8,
  -1E9,
  10.5e+5
);

// array of char formats
$char_formats = array(
  "%c", "%hc", "%lc",
  "%Lc", " %c", "%c ",
  "\t%c", "\n%c", "%4c",
  "%30c", "%[a-bA-B@#$&]", "%*c"
);

$count = 1;
foreach($float_values as $float_value) {
  echo "\n-- Iteration $count --\n";
  
  foreach($char_formats as $format) {
    var_dump( sprintf($format, $float_value) );
  }
  $count++;
};

echo "Done";
?>
--EXPECT--
*** Testing sprintf() : char formats with float values ***

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
unicode(1) "ﾠ"
unicode(1) "c"
unicode(1) "ﾠ"
unicode(1) "c"
unicode(2) " ﾠ"
unicode(2) "ﾠ "
unicode(2) "	ﾠ"
unicode(2) "
ﾠ"
unicode(1) "ﾠ"
unicode(1) "ﾠ"
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 5 --
unicode(1) "￀"
unicode(1) "c"
unicode(1) "￀"
unicode(1) "c"
unicode(2) " ￀"
unicode(2) "￀ "
unicode(2) "	￀"
unicode(2) "
￀"
unicode(1) "￀"
unicode(1) "￀"
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 6 --
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

-- Iteration 7 --
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

-- Iteration 8 --
unicode(1) "ﾐ"
unicode(1) "c"
unicode(1) "ﾐ"
unicode(1) "c"
unicode(2) " ﾐ"
unicode(2) "ﾐ "
unicode(2) "	ﾐ"
unicode(2) "
ﾐ"
unicode(1) "ﾐ"
unicode(1) "ﾐ"
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"
Done
