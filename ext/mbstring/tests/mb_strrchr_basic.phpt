--TEST--
Test mb_strrchr() function : basic functionality 
--SKIPIF--
<?php
extension_loaded('mbstring') or die('skip');
function_exists('mb_strrchr') or die("skip mb_strrchr() is not available in this build");
?>
--FILE--
<?php
/* Prototype  : string mb_strrchr(string haystack, string needle[, bool part[, string encoding]])
 * Description: Finds the last occurrence of a character in a string within another 
 * Source code: ext/mbstring/mbstring.c
 * Alias to functions: 
 */

echo "*** Testing mb_strrchr() : basic functionality ***\n";

mb_internal_encoding('UTF-8');

$string_ascii = b'abc def';
//Japanese string in UTF-8
$string_mb = base64_decode('5pel5pys6Kqe44OG44Kt44K544OI44Gn44GZ44CCMDEyMzTvvJXvvJbvvJfvvJjvvJnjgII=');

echo "\n-- ASCII string: needle exists --\n";
var_dump(bin2hex(mb_strrchr($string_ascii, b'd', false, 'ISO-8859-1')));
var_dump(bin2hex(mb_strrchr($string_ascii, b'd')));
var_dump(bin2hex(mb_strrchr($string_ascii, b'd', true)));


echo "\n-- ASCII string: needle doesn't exist --\n";
var_dump(mb_strrchr($string_ascii, b'123'));

echo "\n-- Multibyte string: needle exists --\n";
$needle1 = base64_decode('5pel5pys6Kqe');
var_dump(bin2hex(mb_strrchr($string_mb, $needle1)));
var_dump(bin2hex(mb_strrchr($string_mb, $needle1, false, 'utf-8')));
var_dump(bin2hex(mb_strrchr($string_mb, $needle1, true)));


echo "\n-- Multibyte string: needle doesn't exist --\n";
$needle2 = base64_decode('44GT44KT44Gr44Gh44Gv44CB5LiW55WM');
var_dump(mb_strrchr($string_mb, $needle2));

?>
===DONE===
--EXPECT--
*** Testing mb_strrchr() : basic functionality ***

-- ASCII string: needle exists --
unicode(6) "646566"
unicode(6) "646566"
unicode(8) "61626320"

-- ASCII string: needle doesn't exist --
bool(false)

-- Multibyte string: needle exists --
unicode(106) "e697a5e69cace8aa9ee38386e382ade382b9e38388e381a7e38199e380823031323334efbc95efbc96efbc97efbc98efbc99e38082"
unicode(106) "e697a5e69cace8aa9ee38386e382ade382b9e38388e381a7e38199e380823031323334efbc95efbc96efbc97efbc98efbc99e38082"
unicode(0) ""

-- Multibyte string: needle doesn't exist --
bool(false)
===DONE===