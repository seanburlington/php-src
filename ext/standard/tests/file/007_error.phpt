--TEST--
Test fopen, fclose() & feof() functions: error conditions
--XFAIL--
Return values are inconsistent (and have changed from previous versions)
--CREDITS--
Dave Kelsey <d_kelsey@uk.ibm.com>
--FILE--
<?php
/*
 Prototype: resource fopen(string $filename, string $mode
                            [, bool $use_include_path [, resource $context]] );
 Description: Opens file or URL.

 Prototype: bool fclose ( resource $handle );
 Description: Closes an open file pointer

 Prototype: bool feof ( resource $handle )
 Description: Returns TRUE if the file pointer is at EOF or an error occurs 
   (including socket timeout); otherwise returns FALSE. 
*/

echo "*** Testing error conditions for fopen(), fclsoe() & feof() ***\n";
/* Arguments less than minimum no.of args */

// fopen ()
var_dump(fopen(__FILE__)); // one valid argument
var_dump(fopen());  // zero argument

// fclose()
$fp = fopen(__FILE__, "r");
fclose($fp);
var_dump( fclose($fp) ); // closed handle
var_dump( fclose(__FILE__) ); // invalid handle
var_dump( fclose() ); // zero argument 

//feof()
var_dump( feof($fp) );  // closed handle
var_dump( feof(__FILE__) );  // invalid handle
var_dump( feof() );  //zero argument

/* Arguments greater than maximum no.of ags */
var_dump(fopen(__FILE__, "r", TRUE, "www.example.com", 100));

$fp = fopen(__FILE__, "r");
var_dump( fclose($fp, "handle") );

var_dump( feof($fp, "handle"));
fclose($fp);

/* test invalid arguments : non-resources */
echo "-- Testing fopen(), fclose() & feof() with invalid arguments --\n";
$invalid_args = array (
  "string",
  10,
  10.5,
  true,
  array(1,2,3),
  NULL,
  ""
);

/* loop to test fclose with different invalid type of args */
for($loop_counter = 1; $loop_counter <= count($invalid_args); $loop_counter++) {
  echo "-- Iteration $loop_counter --\n";
  var_dump( fopen($invalid_args[$loop_counter - 1], "r") );
  var_dump( fclose($invalid_args[$loop_counter - 1]) );
  var_dump( feof($invalid_args[$loop_counter - 1]) );
}

?>
--EXPECTF--
*** Testing error conditions for fopen(), fclsoe() & feof() ***

Warning: fopen() expects at least 2 parameters, 1 given in %s on line %d
bool(false)

Warning: fopen() expects at least 2 parameters, 0 given in %s on line %d
bool(false)

Warning: fclose(): 5 is not a valid stream resource in %s on line %d
bool(false)

Warning: fclose() expects parameter 1 to be resource, Unicode string given in %s on line %d
NULL

Warning: fclose() expects exactly 1 parameter, 0 given in %s on line %d
NULL

Warning: feof(): 5 is not a valid stream resource in %s on line %d
bool(false)

Warning: feof() expects parameter 1 to be resource, Unicode string given in %s on line %d
NULL

Warning: feof() expects exactly 1 parameter, 0 given in %s on line %d
NULL

Warning: fopen() expects at most 4 parameters, 5 given in %s on line %d
bool(false)

Warning: fclose() expects exactly 1 parameter, 2 given in %s on line %d
NULL

Warning: feof() expects exactly 1 parameter, 2 given in %s on line %d
NULL
-- Testing fopen(), fclose() & feof() with invalid arguments --
-- Iteration 1 --

Warning: fopen(string): failed to open stream: No such file or directory in %s on line %d
bool(false)

Warning: fclose() expects parameter 1 to be resource, Unicode string given in %s on line %d
NULL

Warning: feof() expects parameter 1 to be resource, Unicode string given in %s on line %d
NULL
-- Iteration 2 --

Warning: fopen(10): failed to open stream: No such file or directory in %s on line %d
bool(false)

Warning: fclose() expects parameter 1 to be resource, integer given in %s on line %d
NULL

Warning: feof() expects parameter 1 to be resource, integer given in %s on line %d
NULL
-- Iteration 3 --

Warning: fopen(10.5): failed to open stream: No such file or directory in %s on line %d
bool(false)

Warning: fclose() expects parameter 1 to be resource, double given in %s on line %d
NULL

Warning: feof() expects parameter 1 to be resource, double given in %s on line %d
NULL
-- Iteration 4 --

Warning: fopen(1): failed to open stream: No such file or directory in %s on line %d
bool(false)

Warning: fclose() expects parameter 1 to be resource, boolean given in %s on line %d
NULL

Warning: feof() expects parameter 1 to be resource, boolean given in %s on line %d
NULL
-- Iteration 5 --

Notice: Array to string conversion in %s on line %d

Warning: fopen() expects parameter 1 to be string, array given in %s on line %d
bool(false)

Warning: fclose() expects parameter 1 to be resource, array given in %s on line %d
NULL

Warning: feof() expects parameter 1 to be resource, array given in %s on line %d
NULL
-- Iteration 6 --

Warning: fopen(): Filename cannot be empty in %s on line %d
bool(false)

Warning: fclose() expects parameter 1 to be resource, null given in %s on line %d
NULL

Warning: feof() expects parameter 1 to be resource, null given in %s on line %d
NULL
-- Iteration 7 --

Warning: fopen(): Filename cannot be empty in %s on line %d
bool(false)

Warning: fclose() expects parameter 1 to be resource, Unicode string given in %s on line %d
NULL

Warning: feof() expects parameter 1 to be resource, Unicode string given in %s on line %d
NULL
