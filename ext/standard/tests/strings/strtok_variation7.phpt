--TEST--
Test strtok() function : usage variations - modifying the input string while tokenising
--FILE--
<?php
/* Prototype  : string strtok ( str $str, str $token )
 * Description: splits a string (str) into smaller strings (tokens), with each token being delimited by any character from token
 * Source code: ext/standard/string.c
*/

/*
 * Testing strtok() : modifying the input string while it is getting tokenised
*/

echo "*** Testing strtok() : with modification of input string in between tokenising ***\n";

$str = "this is a sample string";
$token = " ";

echo "\n*** Testing strtok() when string being tokenised is prefixed with another string in between the process ***\n";
var_dump( strtok($str, $token) ); 
// adding a string to the input string which is being tokenised
$str = "extra string ".$str;
for( $count = 1; $count <=6; $count++ )  {
  echo "\n-- Token $count is --\n";
  var_dump( strtok($token) );
  echo "\n-- Input str is \"$str\" --\n";
}
		      
echo "\n*** Testing strtok() when string being tokenised is suffixed with another string in between the process ***\n";
var_dump( strtok($str, $token) ); 
// adding a string to the input string which is being tokenised
$str = $str." extra string";
for( $count = 1; $count <=10; $count++ )  {
  echo "\n-- Token $count is --\n";
  var_dump( strtok($token) );
}

echo "Done\n";
?>
--EXPECT--
*** Testing strtok() : with modification of input string in between tokenising ***

*** Testing strtok() when string being tokenised is prefixed with another string in between the process ***
unicode(4) "this"

-- Token 1 is --
unicode(2) "is"

-- Input str is "extra string this is a sample string" --

-- Token 2 is --
unicode(1) "a"

-- Input str is "extra string this is a sample string" --

-- Token 3 is --
unicode(6) "sample"

-- Input str is "extra string this is a sample string" --

-- Token 4 is --
unicode(6) "string"

-- Input str is "extra string this is a sample string" --

-- Token 5 is --
bool(false)

-- Input str is "extra string this is a sample string" --

-- Token 6 is --
bool(false)

-- Input str is "extra string this is a sample string" --

*** Testing strtok() when string being tokenised is suffixed with another string in between the process ***
unicode(5) "extra"

-- Token 1 is --
unicode(6) "string"

-- Token 2 is --
unicode(4) "this"

-- Token 3 is --
unicode(2) "is"

-- Token 4 is --
unicode(1) "a"

-- Token 5 is --
unicode(6) "sample"

-- Token 6 is --
unicode(6) "string"

-- Token 7 is --
bool(false)

-- Token 8 is --
bool(false)

-- Token 9 is --
bool(false)

-- Token 10 is --
bool(false)
Done
