--TEST--
Test stripslashes() function : basic functionality 
--FILE--
<?php
/* Prototype  : string stripslashes ( string $str )
 * Description: Un-quotes a quoted string 
 * Source code: ext/standard/string.c
*/

/*
 * Testing stripslashes() with quoted strings 
*/

echo "*** Testing stripslashes() : basic functionality ***\n";

// Initialize all required variables
$str_array = array( "How's everybody",   // string containing single quote
                    'Are you "JOHN"?',   // string with double quotes
                    'c:\php\stripslashes',   // string with backslashes
                    'c:\\php\\stripslashes',   // string with double backslashes
                    "hello\0world"   // string with nul character
                  );

// Calling striplashes() with all arguments
foreach( $str_array as $str )  {
  $str_addslashes = addslashes($str);
  var_dump("The string after addslashes is:", $str_addslashes);
  $str_stripslashes = stripslashes($str_addslashes);
  var_dump("The string after stripslashes is:", $str_stripslashes);
  if( strcmp($str, $str_stripslashes) != 0 )
    echo "\nError: Original string and string after stripslashes donot match\n";
}

echo "Done\n";
?>
--EXPECT--
*** Testing stripslashes() : basic functionality ***
unicode(31) "The string after addslashes is:"
unicode(16) "How\'s everybody"
unicode(33) "The string after stripslashes is:"
unicode(15) "How's everybody"
unicode(31) "The string after addslashes is:"
unicode(17) "Are you \"JOHN\"?"
unicode(33) "The string after stripslashes is:"
unicode(15) "Are you "JOHN"?"
unicode(31) "The string after addslashes is:"
unicode(21) "c:\\php\\stripslashes"
unicode(33) "The string after stripslashes is:"
unicode(19) "c:\php\stripslashes"
unicode(31) "The string after addslashes is:"
unicode(21) "c:\\php\\stripslashes"
unicode(33) "The string after stripslashes is:"
unicode(19) "c:\php\stripslashes"
unicode(31) "The string after addslashes is:"
unicode(12) "hello\0world"
unicode(33) "The string after stripslashes is:"
unicode(11) "hello world"
Done
