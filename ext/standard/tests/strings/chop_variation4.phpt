--TEST--
Test chop() function : usage variations - strings with embedded nulls
--FILE--
<?php
/* Prototype  : string chop ( string $str [, string $charlist] )
 * Description: Strip whitespace (or other characters) from the end of a string
 * Source code: ext/standard/string.c
*/

/*
 * Testing chop() : with nulls embedded in input string
*/

echo "*** Testing chop() : string with embedded nulls ***\n";

// defining varous strings with embedded nulls
$strings_with_nulls = array(
   		 	   "hello\0world",
			   "\0hello",
			   "hello\0",
			   "\0\0hello\tworld\0\0",
			   "\\0hello\\0",
			   'hello\0\0',
			   chr(0),
                           chr(0).chr(0),
                           chr(0).'hello'.chr(0),
                           'hello'.chr(0).'world'
			   );

$count = 1;
foreach($strings_with_nulls as $string)  {
  echo "\n--- Iteration $count ---\n";
  var_dump( chop($string) );
  var_dump( chop($string, "\0") );
  var_dump( chop($string, '\0') );
  $count++;
}

echo "Done\n";
?>
--EXPECT--
*** Testing chop() : string with embedded nulls ***

--- Iteration 1 ---
unicode(11) "hello world"
unicode(11) "hello world"
unicode(11) "hello world"

--- Iteration 2 ---
unicode(6) " hello"
unicode(6) " hello"
unicode(6) " hello"

--- Iteration 3 ---
unicode(5) "hello"
unicode(5) "hello"
unicode(6) "hello "

--- Iteration 4 ---
unicode(13) "  hello	world"
unicode(13) "  hello	world"
unicode(15) "  hello	world  "

--- Iteration 5 ---
unicode(9) "\0hello\0"
unicode(9) "\0hello\0"
unicode(7) "\0hello"

--- Iteration 6 ---
unicode(9) "hello\0\0"
unicode(9) "hello\0\0"
unicode(5) "hello"

--- Iteration 7 ---
unicode(0) ""
unicode(0) ""
unicode(1) " "

--- Iteration 8 ---
unicode(0) ""
unicode(0) ""
unicode(2) "  "

--- Iteration 9 ---
unicode(6) " hello"
unicode(6) " hello"
unicode(7) " hello "

--- Iteration 10 ---
unicode(11) "hello world"
unicode(11) "hello world"
unicode(11) "hello world"
Done
