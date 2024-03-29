--TEST--
Test strip_tags() function : usage variations - double quoted strings
--INI--
set short_open_tag = on
--FILE--
<?php
/* Prototype  : string strip_tags(string $str [, string $allowable_tags])
 * Description: Strips HTML and PHP tags from a string
 * Source code: ext/standard/string.c
*/

/*
 * testing functionality of strip_tags() by giving double quoted strings as values for $str argument
*/

echo "*** Testing strip_tags() : usage variations ***\n";

$double_quote_string = array (
  "<html> \$ -> This represents the dollar sign</html><?php echo hello ?>",
  "<html>\t\r\v The quick brown fo\fx jumped over the lazy dog</p>",
  "<a>This is a hyper text tag</a>",
  "<? <html>hello world\\t</html>?>",
  "<p>This is a paragraph</p>",
  "<b>This is \ta text in bold letters\r\s\malong with slashes\n</b>"
);

$quotes = "<html><a><p><b><?php";

//loop through the various elements of strings array to test strip_tags() functionality
$iterator = 1;
foreach($double_quote_string as $string_value)
{
      echo "-- Iteration $iterator --\n";
      var_dump( strip_tags($string_value, $quotes) );
      $iterator++;
}

echo "Done";
--EXPECT--
*** Testing strip_tags() : usage variations ***
-- Iteration 1 --
unicode(50) "<html> $ -> This represents the dollar sign</html>"
-- Iteration 2 --
unicode(59) "<html>	 The quick brown fox jumped over the lazy dog</p>"
-- Iteration 3 --
unicode(31) "<a>This is a hyper text tag</a>"
-- Iteration 4 --
unicode(0) ""
-- Iteration 5 --
unicode(26) "<p>This is a paragraph</p>"
-- Iteration 6 --
unicode(62) "<b>This is 	a text in bold letters\s\malong with slashes
</b>"
Done
