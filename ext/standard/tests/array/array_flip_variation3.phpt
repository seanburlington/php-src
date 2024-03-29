--TEST--
Test array_flip() function : usage variations - 'input' argument with different valid values
--FILE--
<?php
/* Prototype  : array array_flip(array $input)
 * Description: Return array with key <-> value flipped 
 * Source code: ext/standard/array.c
*/

/*
* In 'input' array argument, values are expected to be valid keys i.e. string/integer
* here testing for all different valid string and integer values
*/

echo "*** Testing array_flip() : different valid values in 'input' array argument ***\n";

$empty_heredoc = <<<EOT1
EOT1;

$simple_heredoc = <<<EOT4
simple
EOT4;

$multiline_heredoc = <<<EOT7
multiline heredoc with 123 and
speci@! ch@r$..checking\nwith\talso
EOT7;

$input = array(
  // numeric values
  'int_value' => 1,
  'negative_value' => -2,
  'zero_value' => 0,
  'octal_value' => 012,
  'hex_value' => 0x23,

  // single quoted string value
  'empty_value1' => '',
  'space_value1' => ' ',
  'char_value1' => 'a',
  'string_value1' => 'string1',
  'numeric_value1' => '123',
  'special_char_value1' => '!@#$%',
  'whitespace1_value1' => '\t',
  'whitespace2_value1' => '\n',
  'null_char_value1' => '\0',
  
  // double quoted string value
  'empty_value2' => "",
  'space_value2' => " ",
  'char_value2' => "b",
  'string_value2' => "string2",
  'numeric_value2' => "456",
  'special_char_value2' => "^&*",
  'whitespace1_value2' => "\t",
  'whitespace2_value2' => "\n",
  'null_char_value2' => "\0",
  'binary_value' => "a".chr(0)."b",

  // heredoc string value
  'empty_heredoc' => $empty_heredoc,
  'simple_heredoc' => $simple_heredoc,
  'multiline_heredoc' => $multiline_heredoc,
);
  
var_dump( array_flip($input) );

echo "Done"
?>
--EXPECT--
*** Testing array_flip() : different valid values in 'input' array argument ***
array(24) {
  [1]=>
  unicode(9) "int_value"
  [-2]=>
  unicode(14) "negative_value"
  [0]=>
  unicode(10) "zero_value"
  [10]=>
  unicode(11) "octal_value"
  [35]=>
  unicode(9) "hex_value"
  [u""]=>
  unicode(13) "empty_heredoc"
  [u" "]=>
  unicode(12) "space_value2"
  [u"a"]=>
  unicode(11) "char_value1"
  [u"string1"]=>
  unicode(13) "string_value1"
  [123]=>
  unicode(14) "numeric_value1"
  [u"!@#$%"]=>
  unicode(19) "special_char_value1"
  [u"\t"]=>
  unicode(18) "whitespace1_value1"
  [u"\n"]=>
  unicode(18) "whitespace2_value1"
  [u"\0"]=>
  unicode(16) "null_char_value1"
  [u"b"]=>
  unicode(11) "char_value2"
  [u"string2"]=>
  unicode(13) "string_value2"
  [456]=>
  unicode(14) "numeric_value2"
  [u"^&*"]=>
  unicode(19) "special_char_value2"
  [u"	"]=>
  unicode(18) "whitespace1_value2"
  [u"
"]=>
  unicode(18) "whitespace2_value2"
  [u" "]=>
  unicode(16) "null_char_value2"
  [u"a b"]=>
  unicode(12) "binary_value"
  [u"simple"]=>
  unicode(14) "simple_heredoc"
  [u"multiline heredoc with 123 and
speci@! ch@r$..checking
with	also"]=>
  unicode(17) "multiline_heredoc"
}
Done
