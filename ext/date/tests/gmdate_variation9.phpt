--TEST--
Test gmdate() function : usage variation - Passing Time format options to format argument.
--FILE--
<?php
/* Prototype  : string gmdate(string format [, long timestamp])
 * Description: Format a GMT date/time 
 * Source code: ext/date/php_date.c
 * Alias to functions: 
 */

echo "*** Testing gmdate() : usage variation ***\n";

// Initialise all required variables
date_default_timezone_set('UTC');
$timestamp = mktime(8, 8, 8, 8, 8, 2008);

$time_formats = array(

      'Lowercase Ante meridiem and post meridiem' => 'a',
	  'Uppercase Ante meridiem and post meridiem' => 'a',
	  'Swatch Internet time' => 'B',
	  '12-hour format without leading zeros' => 'g',
	  '24-hour format without leading zeros' => 'G',
	  '12-hour format with leading zeros' => 'h',
	  '24-hour format with leading zeros' => 'H',
	  'Minutes with leading zeros' => 'i',
	  'Seconds with leading zeros' => 's',
	  'Milliseconds' => 'u',
);

foreach($time_formats as $key =>$value) {
      echo "\n--$key--\n";
	  var_dump( gmdate($value) );
	  var_dump( gmdate($value, $timestamp) );
}	  

?>
===DONE===
--EXPECTF--
*** Testing gmdate() : usage variation ***

--Lowercase Ante meridiem and post meridiem--
unicode(2) "%s"
unicode(2) "am"

--Uppercase Ante meridiem and post meridiem--
unicode(2) "%s"
unicode(2) "am"

--Swatch Internet time--
unicode(3) "%d"
unicode(3) "380"

--12-hour format without leading zeros--
unicode(%d) "%d"
unicode(1) "8"

--24-hour format without leading zeros--
unicode(%d) "%d"
unicode(1) "8"

--12-hour format with leading zeros--
unicode(2) "%d"
unicode(2) "08"

--24-hour format with leading zeros--
unicode(2) "%d"
unicode(2) "08"

--Minutes with leading zeros--
unicode(2) "%d"
unicode(2) "08"

--Seconds with leading zeros--
unicode(2) "%d"
unicode(2) "08"

--Milliseconds--
unicode(6) "%d"
unicode(6) "000000"
===DONE===
