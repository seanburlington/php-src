--TEST--
Test htmlspecialchars_decode() function : basic functionality 
--FILE--
<?php
/* Prototype  : string htmlspecialchars_decode(string $string [, int $quote_style])
 * Description: Convert special HTML entities back to characters 
 * Source code: ext/standard/html.c
*/

echo "*** Testing htmlspecialchars_decode() : basic functionality ***\n";


// Initialise arguments 
//value initialized = Roy's height > Sam's height. 13 < 25. 1111 & 0000 = 0000. "double quoted string"
$single_quote_string = "Roy&#039;s height &gt; Sam&#039;s height. 13 &lt; 25. 1111 &amp; 0000 = 0000. &quot; double quoted string &quot;";
$double_quote_string = "Roy&#039;s height &gt; Sam&#039;s height. 13 &lt; 25. 1111 &amp; 0000 = 0000. &quot; double quoted string &quot;";

// Calling htmlspecialchars_decode() with default arguments
var_dump( htmlspecialchars_decode($single_quote_string) );
var_dump( htmlspecialchars_decode($double_quote_string) );

// Calling htmlspecialchars_decode() with optional 'quote_style' argument
var_dump( htmlspecialchars_decode($single_quote_string, ENT_COMPAT) );
var_dump( htmlspecialchars_decode($double_quote_string, ENT_COMPAT) );   
var_dump( htmlspecialchars_decode($single_quote_string, ENT_NOQUOTES) );
var_dump( htmlspecialchars_decode($double_quote_string, ENT_NOQUOTES) );
var_dump( htmlspecialchars_decode($single_quote_string, ENT_QUOTES) );
var_dump( htmlspecialchars_decode($double_quote_string, ENT_QUOTES) );

echo "Done";
?>
--EXPECT--
*** Testing htmlspecialchars_decode() : basic functionality ***
unicode(92) "Roy&#039;s height > Sam&#039;s height. 13 < 25. 1111 & 0000 = 0000. " double quoted string ""
unicode(92) "Roy&#039;s height > Sam&#039;s height. 13 < 25. 1111 & 0000 = 0000. " double quoted string ""
unicode(92) "Roy&#039;s height > Sam&#039;s height. 13 < 25. 1111 & 0000 = 0000. " double quoted string ""
unicode(92) "Roy&#039;s height > Sam&#039;s height. 13 < 25. 1111 & 0000 = 0000. " double quoted string ""
unicode(102) "Roy&#039;s height > Sam&#039;s height. 13 < 25. 1111 & 0000 = 0000. &quot; double quoted string &quot;"
unicode(102) "Roy&#039;s height > Sam&#039;s height. 13 < 25. 1111 & 0000 = 0000. &quot; double quoted string &quot;"
unicode(82) "Roy's height > Sam's height. 13 < 25. 1111 & 0000 = 0000. " double quoted string ""
unicode(82) "Roy's height > Sam's height. 13 < 25. 1111 & 0000 = 0000. " double quoted string ""
Done
