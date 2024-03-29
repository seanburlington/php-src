--TEST--
Test stripslashes() function : usage variations - un-quote strings quoted with addslashes()
--FILE--
<?php
/* Prototype  : string stripslashes ( string $str )
 * Description: Returns an un-quoted string
 * Source code: ext/standard/string.c
*/

/*
 * Test stripslashes() with various strings containing characters thats can be backslashed.
 * First adding slashes using addslashes() and then removing the slashes using stripslashes() 
*/

echo "*** Testing stripslashes() : with various strings containing backslashed characters ***\n";

// initialising a heredoc string
$heredoc_string = <<<EOT
This is line 1 of 'heredoc' string
This is line 2 of "heredoc" string
EOT;

$heredoc_null_string =<<<EOT
EOT;
$heredoc_string_only_backslash =<<<EOT
\
EOT;
$heredoc_string_only_single_quote =<<<EOT
'
EOT;
$heredoc_string_only_double_quote =<<<EOT
"
EOT;
 
// initialising the string array

$str_array = array( 
                    // string without any characters that can be backslashed
                    'Hello world',
 
                    // string with single quotes
                    "how're you doing?", 
                    "don't disturb u'r neighbours",
                    "don't disturb u'r neighbours''",
                    '',
                    '\'',
                    "'",
                    $heredoc_string_only_single_quote,
                    
                    // string with double quotes
                    'he said, "he will be on leave"',
                    'he said, ""he will be on leave"',
                    '"""PHP"""',
                    "",
                    "\"",
                    '"',
 		    "hello\"",
                    $heredoc_string_only_double_quote,
                         
                    // string with backslash characters
                    'Is your name Ram\Krishna?',
                    '\\0.0.0.0',
                    'c:\php\testcase\stripslashes',
                    '\\',
                    $heredoc_string_only_backslash,

                    // string with nul characters
                    'hello'.chr(0).'world',
                    chr(0).'hello'.chr(0),
                    chr(0).chr(0).'hello',
                    chr(0),

                    // mixed strings
                    "'\\0.0.0.0'",
                    "'\\0.0.0.0'".chr(0),
                    chr(0)."'c:\php\'",
                    '"\\0.0.0.0"',
                    '"c:\php\"'.chr(0)."'",
                    '"hello"'."'world'".chr(0).'//',

		    // string with hexadecimal number
                    "0xABCDEF0123456789",
                    "\x00",
                    '!@#$%&*@$%#&/;:,<>',
                    "hello\x00world",

                    // heredoc strings
                    $heredoc_string,
                    $heredoc_null_string
                  );

$count = 1;
// looping to test for all strings in $str_array
foreach( $str_array as $str )  {
  echo "\n-- Iteration $count --\n";
  $str_addslashes = addslashes($str);
  var_dump("The string after addslashes is:", $str_addslashes);
  $str_stripslashes = stripslashes($str_addslashes);
  var_dump("The string after stripslashes is:", $str_stripslashes);
  if( strcmp($str, $str_stripslashes) != 0 )
    echo "\nError: Original string and string from stripslash() donot match\n";
  $count ++;
}

echo "Done\n";
?>
--EXPECT--
*** Testing stripslashes() : with various strings containing backslashed characters ***

-- Iteration 1 --
unicode(31) "The string after addslashes is:"
unicode(11) "Hello world"
unicode(33) "The string after stripslashes is:"
unicode(11) "Hello world"

-- Iteration 2 --
unicode(31) "The string after addslashes is:"
unicode(18) "how\'re you doing?"
unicode(33) "The string after stripslashes is:"
unicode(17) "how're you doing?"

-- Iteration 3 --
unicode(31) "The string after addslashes is:"
unicode(30) "don\'t disturb u\'r neighbours"
unicode(33) "The string after stripslashes is:"
unicode(28) "don't disturb u'r neighbours"

-- Iteration 4 --
unicode(31) "The string after addslashes is:"
unicode(34) "don\'t disturb u\'r neighbours\'\'"
unicode(33) "The string after stripslashes is:"
unicode(30) "don't disturb u'r neighbours''"

-- Iteration 5 --
unicode(31) "The string after addslashes is:"
unicode(0) ""
unicode(33) "The string after stripslashes is:"
unicode(0) ""

-- Iteration 6 --
unicode(31) "The string after addslashes is:"
unicode(2) "\'"
unicode(33) "The string after stripslashes is:"
unicode(1) "'"

-- Iteration 7 --
unicode(31) "The string after addslashes is:"
unicode(2) "\'"
unicode(33) "The string after stripslashes is:"
unicode(1) "'"

-- Iteration 8 --
unicode(31) "The string after addslashes is:"
unicode(2) "\'"
unicode(33) "The string after stripslashes is:"
unicode(1) "'"

-- Iteration 9 --
unicode(31) "The string after addslashes is:"
unicode(32) "he said, \"he will be on leave\""
unicode(33) "The string after stripslashes is:"
unicode(30) "he said, "he will be on leave""

-- Iteration 10 --
unicode(31) "The string after addslashes is:"
unicode(34) "he said, \"\"he will be on leave\""
unicode(33) "The string after stripslashes is:"
unicode(31) "he said, ""he will be on leave""

-- Iteration 11 --
unicode(31) "The string after addslashes is:"
unicode(15) "\"\"\"PHP\"\"\""
unicode(33) "The string after stripslashes is:"
unicode(9) """"PHP""""

-- Iteration 12 --
unicode(31) "The string after addslashes is:"
unicode(0) ""
unicode(33) "The string after stripslashes is:"
unicode(0) ""

-- Iteration 13 --
unicode(31) "The string after addslashes is:"
unicode(2) "\""
unicode(33) "The string after stripslashes is:"
unicode(1) """

-- Iteration 14 --
unicode(31) "The string after addslashes is:"
unicode(2) "\""
unicode(33) "The string after stripslashes is:"
unicode(1) """

-- Iteration 15 --
unicode(31) "The string after addslashes is:"
unicode(7) "hello\""
unicode(33) "The string after stripslashes is:"
unicode(6) "hello""

-- Iteration 16 --
unicode(31) "The string after addslashes is:"
unicode(2) "\""
unicode(33) "The string after stripslashes is:"
unicode(1) """

-- Iteration 17 --
unicode(31) "The string after addslashes is:"
unicode(26) "Is your name Ram\\Krishna?"
unicode(33) "The string after stripslashes is:"
unicode(25) "Is your name Ram\Krishna?"

-- Iteration 18 --
unicode(31) "The string after addslashes is:"
unicode(9) "\\0.0.0.0"
unicode(33) "The string after stripslashes is:"
unicode(8) "\0.0.0.0"

-- Iteration 19 --
unicode(31) "The string after addslashes is:"
unicode(31) "c:\\php\\testcase\\stripslashes"
unicode(33) "The string after stripslashes is:"
unicode(28) "c:\php\testcase\stripslashes"

-- Iteration 20 --
unicode(31) "The string after addslashes is:"
unicode(2) "\\"
unicode(33) "The string after stripslashes is:"
unicode(1) "\"

-- Iteration 21 --
unicode(31) "The string after addslashes is:"
unicode(2) "\\"
unicode(33) "The string after stripslashes is:"
unicode(1) "\"

-- Iteration 22 --
unicode(31) "The string after addslashes is:"
unicode(12) "hello\0world"
unicode(33) "The string after stripslashes is:"
unicode(11) "hello world"

-- Iteration 23 --
unicode(31) "The string after addslashes is:"
unicode(9) "\0hello\0"
unicode(33) "The string after stripslashes is:"
unicode(7) " hello "

-- Iteration 24 --
unicode(31) "The string after addslashes is:"
unicode(9) "\0\0hello"
unicode(33) "The string after stripslashes is:"
unicode(7) "  hello"

-- Iteration 25 --
unicode(31) "The string after addslashes is:"
unicode(2) "\0"
unicode(33) "The string after stripslashes is:"
unicode(1) " "

-- Iteration 26 --
unicode(31) "The string after addslashes is:"
unicode(13) "\'\\0.0.0.0\'"
unicode(33) "The string after stripslashes is:"
unicode(10) "'\0.0.0.0'"

-- Iteration 27 --
unicode(31) "The string after addslashes is:"
unicode(15) "\'\\0.0.0.0\'\0"
unicode(33) "The string after stripslashes is:"
unicode(11) "'\0.0.0.0' "

-- Iteration 28 --
unicode(31) "The string after addslashes is:"
unicode(15) "\0\'c:\\php\\\'"
unicode(33) "The string after stripslashes is:"
unicode(10) " 'c:\php\'"

-- Iteration 29 --
unicode(31) "The string after addslashes is:"
unicode(13) "\"\\0.0.0.0\""
unicode(33) "The string after stripslashes is:"
unicode(10) ""\0.0.0.0""

-- Iteration 30 --
unicode(31) "The string after addslashes is:"
unicode(17) "\"c:\\php\\\"\0\'"
unicode(33) "The string after stripslashes is:"
unicode(11) ""c:\php\" '"

-- Iteration 31 --
unicode(31) "The string after addslashes is:"
unicode(22) "\"hello\"\'world\'\0//"
unicode(33) "The string after stripslashes is:"
unicode(17) ""hello"'world' //"

-- Iteration 32 --
unicode(31) "The string after addslashes is:"
unicode(18) "0xABCDEF0123456789"
unicode(33) "The string after stripslashes is:"
unicode(18) "0xABCDEF0123456789"

-- Iteration 33 --
unicode(31) "The string after addslashes is:"
unicode(2) "\0"
unicode(33) "The string after stripslashes is:"
unicode(1) " "

-- Iteration 34 --
unicode(31) "The string after addslashes is:"
unicode(18) "!@#$%&*@$%#&/;:,<>"
unicode(33) "The string after stripslashes is:"
unicode(18) "!@#$%&*@$%#&/;:,<>"

-- Iteration 35 --
unicode(31) "The string after addslashes is:"
unicode(12) "hello\0world"
unicode(33) "The string after stripslashes is:"
unicode(11) "hello world"

-- Iteration 36 --
unicode(31) "The string after addslashes is:"
unicode(73) "This is line 1 of \'heredoc\' string
This is line 2 of \"heredoc\" string"
unicode(33) "The string after stripslashes is:"
unicode(69) "This is line 1 of 'heredoc' string
This is line 2 of "heredoc" string"

-- Iteration 37 --
unicode(31) "The string after addslashes is:"
unicode(0) ""
unicode(33) "The string after stripslashes is:"
unicode(0) ""
Done
