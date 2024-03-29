--TEST--
Test strtok() function : usage variations - miscellaneous inputs
--FILE--
<?php
/* Prototype  : string strtok ( str $str, str $token )
 * Description: splits a string (str) into smaller strings (tokens), with each token being delimited by any character from token
 * Source code: ext/standard/string.c
*/

/*
 * Testing strtok() : with miscellaneous combinations of string and token
*/

echo "*** Testing strtok() : with miscellaneous inputs ***\n";

// defining arrays for input strings and tokens
$string_array = array(
   		       "HELLO WORLD",
 		       "hello world",
   		       "_HELLO_WORLD_",
		       "/thello/t/wor/ttld",
		       "hel/lo/t/world",
                       "one:$:two:!:three:#:four",
		       "\rhello/r/wor\rrld",
		       chr(0),
                       chr(0).chr(0),
                       chr(0).'hello'.chr(0),
                       'hello'.chr(0).'world'
 		     );
$token_array = array( 
		      "wr",
		      "hello world",
		      "__",
                      "t/",
		      '/t',
		      ":",
		      "\r",
                      "\0",		
                      "\0",		
                      "\0",		
                      "\0"
 		    );

// loop through each element of the array and check the working of strtok()
// when supplied with different string and token values

$counter =1;
foreach( $string_array as $string )  {
  echo "\n--- Iteration $counter ---\n";
  var_dump( strtok($string, $token_array[$counter-1]) ); 
  for( $count = 1; $count <=5; $count++ )  {
    var_dump( strtok($token_array[$counter-1]) );
  }
  $counter++;
}		      
		      

echo "Done\n";
?>
--EXPECT--
*** Testing strtok() : with miscellaneous inputs ***

--- Iteration 1 ---
unicode(11) "HELLO WORLD"
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)

--- Iteration 2 ---
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)

--- Iteration 3 ---
unicode(5) "HELLO"
unicode(5) "WORLD"
bool(false)
bool(false)
bool(false)
bool(false)

--- Iteration 4 ---
unicode(5) "hello"
unicode(3) "wor"
unicode(2) "ld"
bool(false)
bool(false)
bool(false)

--- Iteration 5 ---
unicode(3) "hel"
unicode(2) "lo"
unicode(5) "world"
bool(false)
bool(false)
bool(false)

--- Iteration 6 ---
unicode(3) "one"
unicode(1) "$"
unicode(3) "two"
unicode(1) "!"
unicode(5) "three"
unicode(1) "#"

--- Iteration 7 ---
unicode(11) "hello/r/wor"
unicode(3) "rld"
bool(false)
bool(false)
bool(false)
bool(false)

--- Iteration 8 ---
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)

--- Iteration 9 ---
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)

--- Iteration 10 ---
unicode(5) "hello"
bool(false)
bool(false)
bool(false)
bool(false)
bool(false)

--- Iteration 11 ---
unicode(5) "hello"
unicode(5) "world"
bool(false)
bool(false)
bool(false)
bool(false)
Done
