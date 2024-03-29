--TEST--
Test strip_tags() function : usage variations - valid value for 'str' and invalid values for 'allowable_tags', binary variant
--INI--
short_open_tag = on
--FILE--
<?php
/* Prototype  : string strip_tags(string $str [, string $allowable_tags])
 * Description: Strips HTML and PHP tags from a string
 * Source code: ext/standard/string.c
*/

/*
 * testing functionality of strip_tags() by giving valid value for $str and invalid values for $allowable_tags argument
*/

echo "*** Testing strip_tags() : usage variations ***\n";

$strings = b"<html>hello</html> \tworld... <p>strip_tags_test\v\f</p><?php hello\t wo\rrld?>";

$quotes = array (
  b"<nnn>",
  b'<nnn>',
  b"<abc>",
  b'<abc>',
  b"<%?php",
  b'<%?php',
  b"<<html>>",
  b'<<html>>'
);

//loop through the various elements of strings array to test strip_tags() functionality
$iterator = 1;
foreach($quotes as $string_value)
{
      echo "-- Iteration $iterator --\n";
      var_dump( strip_tags($strings, $string_value) );
      $iterator++;
}

echo "Done";
--EXPECT--
*** Testing strip_tags() : usage variations ***
-- Iteration 1 --
string(33) "hello 	world... strip_tags_test"
-- Iteration 2 --
string(33) "hello 	world... strip_tags_test"
-- Iteration 3 --
string(33) "hello 	world... strip_tags_test"
-- Iteration 4 --
string(33) "hello 	world... strip_tags_test"
-- Iteration 5 --
string(33) "hello 	world... strip_tags_test"
-- Iteration 6 --
string(33) "hello 	world... strip_tags_test"
-- Iteration 7 --
string(46) "<html>hello</html> 	world... strip_tags_test"
-- Iteration 8 --
string(46) "<html>hello</html> 	world... strip_tags_test"
Done
