--TEST--
Test strftime() function : usage variation - Checking date related formats which are not supported on Windows.
--SKIPIF--
<?php
if (strtoupper(substr(PHP_OS, 0, 3)) != 'WIN') {
    die("skip Test is valid for Windows");
}
?>
--FILE--
<?php
/* Prototype  : string strftime(string format [, int timestamp])
 * Description: Format a local time/date according to locale settings 
 * Source code: ext/date/php_date.c
 * Alias to functions: 
 */

echo "*** Testing strftime() : usage variation ***\n";

// Initialise function arguments not being substituted (if any)
locale_set_default("en_US");
date_default_timezone_set("Asia/Calcutta");
$timestamp = mktime(8, 8, 8, 8, 8, 2008);

//array of values to iterate over
$inputs = array(
	  'Century number' => "%C",
	  'Month Date Year' => "%D",
	  'Year with century' => "%G",
	  'Year without century' => "%g",
);

// loop through each element of the array for timestamp

foreach($inputs as $key =>$value) {
      echo "\n--$key--\n";
	  var_dump( strftime($value) );
	  var_dump( strftime($value, $timestamp) );
}	  

?>
===DONE===
--EXPECTF--
*** Testing strftime() : usage variation ***

--Century number--
bool(false)
bool(false)

--Month Date Year--
bool(false)
bool(false)

--Year with century--
bool(false)
bool(false)

--Year without century--
bool(false)
bool(false)
===DONE===
