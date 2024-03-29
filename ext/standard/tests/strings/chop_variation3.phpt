--TEST--
Test chop() function : usage variations - with heredoc string
--FILE--
<?php
/* Prototype  : string chop ( string $str [, string $charlist] )
 * Description: Strip whitespace (or other characters) from the end of a string
 * Source code: ext/standard/string.c
*/

/*
 * Testing chop() : with heredoc strings
*/

echo "*** Testing chop() : with heredoc strings ***\n";

// defining different heredoc strings
$empty_heredoc = <<<EOT
EOT;

$heredoc_with_newline = <<<EOT
\n

EOT;

$heredoc_with_characters = <<<EOT
first line of heredoc string
second line of heredoc string
third line of heredocstring
EOT;

$heredoc_with_newline_and_tabs = <<<EOT
hello\tworld\nhello\nworld\n
EOT;

$heredoc_with_alphanumerics = <<<EOT
hello123world456
1234hello\t1234
EOT;

$heredoc_with_embedded_nulls = <<<EOT
hello\0world\0hello
\0hello\0
EOT;

$heredoc_strings = array(
                   $empty_heredoc,
                   $heredoc_with_newline,
                   $heredoc_with_characters,
 		   $heredoc_with_newline_and_tabs,
 		   $heredoc_with_alphanumerics,
		   $heredoc_with_embedded_nulls
		   );
$i = 1;
foreach($heredoc_strings as $string)  {
  echo "\n--- Iteration $i ---\n";
  var_dump( chop($string) );
  var_dump( chop($string, "12345o\0\n\t") );
  $i++;
}

echo "Done\n";
?>
--EXPECT--
*** Testing chop() : with heredoc strings ***

--- Iteration 1 ---
unicode(0) ""
unicode(0) ""

--- Iteration 2 ---
unicode(0) ""
unicode(0) ""

--- Iteration 3 ---
unicode(86) "first line of heredoc string
second line of heredoc string
third line of heredocstring"
unicode(86) "first line of heredoc string
second line of heredoc string
third line of heredocstring"

--- Iteration 4 ---
unicode(23) "hello	world
hello
world"
unicode(23) "hello	world
hello
world"

--- Iteration 5 ---
unicode(31) "hello123world456
1234hello	1234"
unicode(25) "hello123world456
1234hell"

--- Iteration 6 ---
unicode(24) "hello world hello
 hello"
unicode(23) "hello world hello
 hell"
Done
