--TEST--
Test mb_substr() function : usage variations - pass different integers to $start arg
--SKIPIF--
<?php
extension_loaded('mbstring') or die('skip');
function_exists('mb_substr') or die("skip mb_substr() is not available in this build");
?>
--FILE--
<?php
/* Prototype  : string mb_substr(string $str, int $start [, int $length [, string $encoding]])
 * Description: Returns part of a string 
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Test how mb_substr() behaves when passed a range of integers as $start argument
 */

echo "*** Testing mb_substr() : usage variations ***\n";

mb_internal_encoding('UTF-8');

$string_ascii = b'+Is an English string'; //21 chars

$string_mb = base64_decode('5pel5pys6Kqe44OG44Kt44K544OI44Gn44GZ44CCMDEyMzTvvJXvvJbvvJfvvJjvvJnjgII='); //21 chars

/*
 * Loop through integers as multiples of ten for $offset argument
 * 60 is larger than *BYTE* count for $string_mb
 */
for ($i = -60; $i <= 60; $i += 10) {
	if (@$a || @$b) {
		$a = null;
		$b = null;
	}
	echo "\n**-- Offset is: $i --**\n";
	echo "-- ASCII String --\n";
	$a = mb_substr($string_ascii, $i, 4);
	if ($a !== false) {
	   var_dump(bin2hex($a));
	}
	else {
	   var_dump($a);
	}		
	echo "--Multibyte String --\n";
	$b = mb_substr($string_mb, $i, 4, 'UTF-8');
	if (strlen($a) == mb_strlen($b, 'UTF-8')) { // should return same length
		var_dump(bin2hex($b));
	} else {
		echo "Difference in length of ASCII string and multibyte string\n";
	}
	
}

echo "Done";
?>
--EXPECT--
*** Testing mb_substr() : usage variations ***

**-- Offset is: -60 --**
-- ASCII String --
unicode(8) "2b497320"
--Multibyte String --
unicode(24) "e697a5e69cace8aa9ee38386"

**-- Offset is: -50 --**
-- ASCII String --
unicode(8) "2b497320"
--Multibyte String --
unicode(24) "e697a5e69cace8aa9ee38386"

**-- Offset is: -40 --**
-- ASCII String --
unicode(8) "2b497320"
--Multibyte String --
unicode(24) "e697a5e69cace8aa9ee38386"

**-- Offset is: -30 --**
-- ASCII String --
unicode(8) "2b497320"
--Multibyte String --
unicode(24) "e697a5e69cace8aa9ee38386"

**-- Offset is: -20 --**
-- ASCII String --
unicode(8) "49732061"
--Multibyte String --
unicode(24) "e69cace8aa9ee38386e382ad"

**-- Offset is: -10 --**
-- ASCII String --
unicode(8) "69736820"
--Multibyte String --
unicode(8) "31323334"

**-- Offset is: 0 --**
-- ASCII String --
unicode(8) "2b497320"
--Multibyte String --
unicode(24) "e697a5e69cace8aa9ee38386"

**-- Offset is: 10 --**
-- ASCII String --
unicode(8) "6c697368"
--Multibyte String --
unicode(8) "30313233"

**-- Offset is: 20 --**
-- ASCII String --
unicode(2) "67"
--Multibyte String --
unicode(6) "e38082"

**-- Offset is: 30 --**
-- ASCII String --
unicode(0) ""
--Multibyte String --
unicode(0) ""

**-- Offset is: 40 --**
-- ASCII String --
unicode(0) ""
--Multibyte String --
unicode(0) ""

**-- Offset is: 50 --**
-- ASCII String --
unicode(0) ""
--Multibyte String --
unicode(0) ""

**-- Offset is: 60 --**
-- ASCII String --
unicode(0) ""
--Multibyte String --
unicode(0) ""
Done