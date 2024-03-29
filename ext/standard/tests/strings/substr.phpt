--TEST--
Testing substr() function
--INI--
unicode.script_encoding=ISO-8859-1
unicode.output_encoding=ISO-8859-1
--FILE--
<?php

/* Prototype: string substr( string str, int start[, int length] )
 * Description: Returns the portion of string specified by the start and length parameters.
 */

$strings_array = array( NULL, "", 12345, "abcdef", "123abc", "_123abc");


/* Testing for error conditions */
echo "*** Testing for error conditions ***\n";

/* Zero Argument */
var_dump( substr() );

/* NULL as Argument */
var_dump( substr(NULL) );

/* Single Argument */
var_dump( substr("abcde") );

/* Scalar Argument */
var_dump( substr(12345) );

/* more than valid number of arguments ( valid are 2 or 3 ) */
var_dump( substr("abcde", 2, 3, 4) );

$counter = 1;
foreach ($strings_array as $str) {
  /* variations with two arguments */
  /* start values >, < and = 0    */

  echo ("\n--- Iteration ".$counter." ---\n");
  echo ("\n-- Variations for two arguments --\n");
  var_dump ( substr($str, 1) );
  var_dump ( substr($str, 0) );
  var_dump ( substr($str, -2) );

  /* variations with three arguments */
  /* start value variations with length values  */

  echo ("\n-- Variations for three arguments --\n");
  var_dump ( substr($str, 1, 3) );
  var_dump ( substr($str, 1, 0) );
  var_dump ( substr($str, 1, -3) );
  var_dump ( substr($str, 0, 3) );
  var_dump ( substr($str, 0, 0) );
  var_dump ( substr($str, 0, -3) );
  var_dump ( substr($str, -2, 3) );
  var_dump ( substr($str, -2, 0 ) );
  var_dump ( substr($str, -2, -3) );

  $counter++;
}

/* variation of start and length to point to same element */
echo ("\n*** Testing for variations of start and length to point to same element ***\n");
var_dump (substr("abcde" , 2, -2) );
var_dump (substr("abcde" , -3, -2) );

/* Testing to return empty string when start denotes the position beyond the truncation (set by negative length) */
echo ("\n*** Testing for start > truncation  ***\n");
var_dump (substr("abcdef" , 4, -4) );

/* String with null character */
echo ("\n*** Testing for string with null characters ***\n");
var_dump (substr("abc\x0xy\x0z" ,2) );

/* String with international characters */
echo ("\n*** Testing for string with international characters ***\n");
var_dump (substr('\xIñtërnâtiônàlizætiøn',3) );

/* start <0 && -start > length */
echo "\n*** Start before the first char ***\n";
var_dump (substr("abcd" , -8) );
 
echo"\nDone";

?>
--EXPECTF--
*** Testing for error conditions ***

Warning: substr() expects at least 2 parameters, 0 given in %s on line %d
NULL

Warning: substr() expects at least 2 parameters, 1 given in %s on line %d
NULL

Warning: substr() expects at least 2 parameters, 1 given in %s on line %d
NULL

Warning: substr() expects at least 2 parameters, 1 given in %s on line %d
NULL

Warning: substr() expects at most 3 parameters, 4 given in %s on line %d
NULL

--- Iteration 1 ---

-- Variations for two arguments --
bool(false)
bool(false)
bool(false)

-- Variations for three arguments --
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)

--- Iteration 2 ---

-- Variations for two arguments --
bool(false)
bool(false)
bool(false)

-- Variations for three arguments --
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)

--- Iteration 3 ---

-- Variations for two arguments --
unicode(4) "2345"
unicode(5) "12345"
unicode(2) "45"

-- Variations for three arguments --
unicode(3) "234"
unicode(0) ""
unicode(1) "2"
unicode(3) "123"
unicode(0) ""
unicode(2) "12"
unicode(2) "45"
unicode(0) ""
unicode(0) ""

--- Iteration 4 ---

-- Variations for two arguments --
unicode(5) "bcdef"
unicode(6) "abcdef"
unicode(2) "ef"

-- Variations for three arguments --
unicode(3) "bcd"
unicode(0) ""
unicode(2) "bc"
unicode(3) "abc"
unicode(0) ""
unicode(3) "abc"
unicode(2) "ef"
unicode(0) ""
unicode(0) ""

--- Iteration 5 ---

-- Variations for two arguments --
unicode(5) "23abc"
unicode(6) "123abc"
unicode(2) "bc"

-- Variations for three arguments --
unicode(3) "23a"
unicode(0) ""
unicode(2) "23"
unicode(3) "123"
unicode(0) ""
unicode(3) "123"
unicode(2) "bc"
unicode(0) ""
unicode(0) ""

--- Iteration 6 ---

-- Variations for two arguments --
unicode(6) "123abc"
unicode(7) "_123abc"
unicode(2) "bc"

-- Variations for three arguments --
unicode(3) "123"
unicode(0) ""
unicode(3) "123"
unicode(3) "_12"
unicode(0) ""
unicode(4) "_123"
unicode(2) "bc"
unicode(0) ""
unicode(0) ""

*** Testing for variations of start and length to point to same element ***
unicode(1) "c"
unicode(1) "c"

*** Testing for start > truncation  ***
unicode(0) ""

*** Testing for string with null characters ***
unicode(6) "c xy z"

*** Testing for string with international characters ***
unicode(26) "ñtërnâtiônàlizætiøn"

*** Start before the first char ***
unicode(4) "abcd"

Done
