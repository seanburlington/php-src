--TEST--
Test sprintf() function : usage variations - char formats with char values
--FILE--
<?php
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/


echo "*** Testing sprintf() : char formats with char values ***\n";

// array of char values 
$char_values = array( 'a', "a", 67, -67, 99, ' ', '', 'A', "A" );

// array of char formats
$char_formats = array( 
  "%c", "%hc", "%lc", 
  "%Lc", " %c", "%c ",
  "\t%c", "\n%c", "%4c",
  "%30c", "%[a-bA-B@#$&]", "%*c"
);

$count = 1;
foreach($char_values as $char_value) {
  echo "\n-- Iteration $count --\n";
  
  foreach($char_formats as $format) {
    var_dump( sprintf($format, $char_value) );
  }
  $count++;
};

echo "Done";
?>
--EXPECT--
*** Testing sprintf() : char formats with char values ***

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
unicode(1) "C"
unicode(1) "c"
unicode(1) "C"
unicode(1) "c"
unicode(2) " C"
unicode(2) "C "
unicode(2) "	C"
unicode(2) "
C"
unicode(1) "C"
unicode(1) "C"
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 4 --
unicode(1) "ﾽ"
unicode(1) "c"
unicode(1) "ﾽ"
unicode(1) "c"
unicode(2) " ﾽ"
unicode(2) "ﾽ "
unicode(2) "	ﾽ"
unicode(2) "
ﾽ"
unicode(1) "ﾽ"
unicode(1) "ﾽ"
unicode(11) "a-bA-B@#$&]"
unicode(1) "c"

-- Iteration 5 --
unicode(1) "c"
unicode(1) "c"
unicode(1) "c"
unicode(1) "c"
unicode(2) " c"
unicode(2) "c "
unicode(2) "	c"
unicode(2) "
c"
unicode(1) "c"
unicode(1) "c"
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

-- Iteration 9 --
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
Done
