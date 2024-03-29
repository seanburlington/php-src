--TEST--
Test vsprintf() function : basic functionality - float format
--FILE--
<?php
/* Prototype  : string vsprintf(string $format , array $args)
 * Description: Return a formatted string 
 * Source code: ext/standard/formatted_print.c
*/

echo "*** Testing vsprintf() : basic functionality - using float format ***\n";

// Initialise all required variables

$format = "format";
$format1 = "%f";
$format2 = "%f %f";
$format3 = "%f %f %f";

$format11 = "%F";
$format22 = "%F %F";
$format33 = "%F %F %F";
$arg1 = array(11.11);
$arg2 = array(11.11,22.22);
$arg3 = array(11.11,22.22,33.33);

var_dump( vsprintf($format1,$arg1) );
var_dump( vsprintf($format11,$arg1) );

var_dump( vsprintf($format2,$arg2) );
var_dump( vsprintf($format22,$arg2) );

var_dump( vsprintf($format3,$arg3) );
var_dump( vsprintf($format33,$arg3) );

echo "Done";
?>
--EXPECT--
*** Testing vsprintf() : basic functionality - using float format ***
unicode(9) "11.110000"
unicode(9) "11.110000"
unicode(19) "11.110000 22.220000"
unicode(19) "11.110000 22.220000"
unicode(29) "11.110000 22.220000 33.330000"
unicode(29) "11.110000 22.220000 33.330000"
Done
