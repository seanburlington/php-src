--TEST--
Test sprintf() function : basic functionality - integer format
--FILE--
<?php
/* Prototype  : string sprintf(string $format [, mixed $arg1 [, mixed ...]])
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/

echo "*** Testing sprintf() : basic functionality - using integer format ***\n";


// Initialise all required variables
$format = "format";
$format1 = "%d";
$format2 = "%d %d";
$format3 = "%d %d %d";
$arg1 = 111;
$arg2 = 222;
$arg3 = 333;

// Calling sprintf() with default arguments
var_dump( sprintf($format) );

// Calling sprintf() with two arguments
var_dump( sprintf($format1, $arg1) );

// Calling sprintf() with three arguments
var_dump( sprintf($format2, $arg1, $arg2) );

// Calling sprintf() with four arguments
var_dump( sprintf($format3, $arg1, $arg2, $arg3) );

echo "Done";
?>
--EXPECT--
*** Testing sprintf() : basic functionality - using integer format ***
unicode(6) "format"
unicode(3) "111"
unicode(7) "111 222"
unicode(11) "111 222 333"
Done
