--TEST--
Test str_split() function : usage variations - different double quoted strings for 'str' argument(Bug#42866) 
--FILE--
<?php
/* Prototype  : array str_split(string $str [, int $split_length])
 * Description: Convert a string to an array. If split_length is 
                specified, break the string down into chunks each 
                split_length characters long. 
 * Source code: ext/standard/string.c
 * Alias to functions: none
*/

/*
* passing different double quoted strings as 'str' argument to str_split()
* split_length is set to 7
*/

echo "*** Testing str_split() : double quoted strings for 'str' ***\n";

//Initialize variables
$split_length = 7;

// different values for 'str'
$values = array(
  "",  //empty
  " ",  //space
  "1234", //with only numbers
  "simple string",  //regular string
  "It's string with quote",  //string containing single quote
  "string\tcontains\rwhite space\nchars",
  "containing @ # $ % ^ & chars", 
  "with 1234 numbers",
  "with \0 and ".chr(0)."null chars",  //for binary safe
  "with    multiple     space char",
  "Testing invalid \k and \m escape char",
  "to check with \\n and \\t" //ignoring \n and \t results

);

//loop through each element of $values for 'str' argument
for($count = 0; $count < count($values); $count++) {
  echo "-- Iteration ".($count+1)." --\n";
  var_dump( str_split($values[$count], $split_length) );
}
echo "Done"
?>
--EXPECT--
*** Testing str_split() : double quoted strings for 'str' ***
-- Iteration 1 --
array(1) {
  [0]=>
  unicode(0) ""
}
-- Iteration 2 --
array(1) {
  [0]=>
  unicode(1) " "
}
-- Iteration 3 --
array(1) {
  [0]=>
  unicode(4) "1234"
}
-- Iteration 4 --
array(2) {
  [0]=>
  unicode(7) "simple "
  [1]=>
  unicode(6) "string"
}
-- Iteration 5 --
array(4) {
  [0]=>
  unicode(7) "It's st"
  [1]=>
  unicode(7) "ring wi"
  [2]=>
  unicode(7) "th quot"
  [3]=>
  unicode(1) "e"
}
-- Iteration 6 --
array(5) {
  [0]=>
  unicode(7) "string	"
  [1]=>
  unicode(7) "contain"
  [2]=>
  unicode(7) "swhite"
  [3]=>
  unicode(7) " space
"
  [4]=>
  unicode(5) "chars"
}
-- Iteration 7 --
array(4) {
  [0]=>
  unicode(7) "contain"
  [1]=>
  unicode(7) "ing @ #"
  [2]=>
  unicode(7) " $ % ^ "
  [3]=>
  unicode(7) "& chars"
}
-- Iteration 8 --
array(3) {
  [0]=>
  unicode(7) "with 12"
  [1]=>
  unicode(7) "34 numb"
  [2]=>
  unicode(3) "ers"
}
-- Iteration 9 --
array(4) {
  [0]=>
  unicode(7) "with   "
  [1]=>
  unicode(7) "and  nu"
  [2]=>
  unicode(7) "ll char"
  [3]=>
  unicode(1) "s"
}
-- Iteration 10 --
array(5) {
  [0]=>
  unicode(7) "with   "
  [1]=>
  unicode(7) " multip"
  [2]=>
  unicode(7) "le     "
  [3]=>
  unicode(7) "space c"
  [4]=>
  unicode(3) "har"
}
-- Iteration 11 --
array(6) {
  [0]=>
  unicode(7) "Testing"
  [1]=>
  unicode(7) " invali"
  [2]=>
  unicode(7) "d \k an"
  [3]=>
  unicode(7) "d \m es"
  [4]=>
  unicode(7) "cape ch"
  [5]=>
  unicode(2) "ar"
}
-- Iteration 12 --
array(4) {
  [0]=>
  unicode(7) "to chec"
  [1]=>
  unicode(7) "k with "
  [2]=>
  unicode(7) "\n and "
  [3]=>
  unicode(2) "\t"
}
Done
