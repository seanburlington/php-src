--TEST--
Test strrev() function : usage variations - heredoc strings
--FILE--
<?php
/* Prototype  : string strrev(string $str);
 * Description: Reverse a string 
 * Source code: ext/standard/string.c
*/

/* Test strrev() function with various heredoc strings for 'str' */

echo "*** Testing strrev() function: with heredoc strings ***\n";
$multi_line_str = <<<EOD
Example of string
spanning multiple lines
using heredoc syntax.
EOD;

$special_chars_str = <<<EOD
Ex'ple of h'doc st'g, contains
$#%^*&*_("_")!#@@!$#$^^&*(special)
chars.
EOD;

$control_chars_str = <<<EOD
Hello, World\n
Hello\0World
EOD;

$quote_chars_str = <<<EOD
it's bright o'side
"things in double quote"
'things in single quote'
this\line is /with\slashs
EOD;

$blank_line = <<<EOD

EOD;

$empty_str = <<<EOD
EOD;

$strings = array(
  $multi_line_str,
  $special_chars_str,
  $control_chars_str,
  $quote_chars_str,
  $blank_line,
  $empty_str
);

$count = 1;
for( $index = 0; $index < count($strings); $index++ ) {
  echo "\n-- Iteration $count --\n";
  var_dump( strrev($strings[$index]) );
  $count ++;
}

echo "*** Done ***";
?>
--EXPECT--
*** Testing strrev() function: with heredoc strings ***

-- Iteration 1 --
unicode(63) ".xatnys codereh gnisu
senil elpitlum gninnaps
gnirts fo elpmaxE"

-- Iteration 2 --
unicode(72) ".srahc
)laiceps(*&^^$#$!@@#!)"_"(_*&*^%#$
sniatnoc ,g'ts cod'h fo elp'xE"

-- Iteration 3 --
unicode(25) "dlroW olleH

dlroW ,olleH"

-- Iteration 4 --
unicode(94) "shsals\htiw/ si enil\siht
'etouq elgnis ni sgniht'
"etouq elbuod ni sgniht"
edis'o thgirb s'ti"

-- Iteration 5 --
unicode(0) ""

-- Iteration 6 --
unicode(0) ""
*** Done ***
