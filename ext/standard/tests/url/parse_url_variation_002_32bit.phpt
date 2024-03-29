--TEST--
Test parse_url() function : usage variations  - unexpected type for arg 2
--SKIPIF--
<?php if (PHP_INT_SIZE != 4) die("skip this test is for 32bit platforms only"); ?>
--FILE--
<?php
/* Prototype  : proto mixed parse_url(string url, [int url_component])
 * Description: Parse a URL and return its components 
 * Source code: ext/standard/url.c
 * Alias to functions: 
 */

function test_error_handler($err_no, $err_msg, $filename, $linenum, $vars) {
	echo "Error: $err_no - $err_msg, $filename($linenum)\n";
}
set_error_handler('test_error_handler');

echo "*** Testing parse_url() : usage variations ***\n";

// Initialise function arguments not being substituted (if any)
$url = 'http://secret:hideout@www.php.net:80/index.php?test=1&test2=char&test3=mixesCI#some_page_ref123';

//get an unset variable
$unset_var = 10;
unset ($unset_var);

//array of values to iterate over
$values = array(

      // float data
      10.5,
      -10.5,
      10.1234567e10,
      10.7654321E-10,
      .5,

      // array data
      array(),
      array(0),
      array(1),
      array(1, 2),
      array('color' => 'red', 'item' => 'pen'),

      // null data
      NULL,
      null,

      // boolean data
      true,
      false,
      TRUE,
      FALSE,

      // empty data
      "",
      '',

      // string data
      "string",
      'string',

      // object data
      new stdclass(),

      // undefined data
      $undefined_var,

      // unset data
      $unset_var,
);

// loop through each element of the array for url_component

foreach($values as $value) {
      echo "\nArg value $value \n";
      var_dump( parse_url($url, $value) );
};

echo "Done";
?>
--EXPECTF--
*** Testing parse_url() : usage variations ***
Error: 8 - Undefined variable: undefined_var, %s(61)
Error: 8 - Undefined variable: unset_var, %s(64)

Arg value 10.5 
Error: 2 - parse_url(): Invalid URL component identifier 10, %s(71)
bool(false)

Arg value -10.5 
array(8) {
  [u"scheme"]=>
  unicode(4) "http"
  [u"host"]=>
  unicode(11) "www.php.net"
  [u"port"]=>
  int(80)
  [u"user"]=>
  unicode(6) "secret"
  [u"pass"]=>
  unicode(7) "hideout"
  [u"path"]=>
  unicode(10) "/index.php"
  [u"query"]=>
  unicode(31) "test=1&test2=char&test3=mixesCI"
  [u"fragment"]=>
  unicode(16) "some_page_ref123"
}

Arg value 101234567000 
array(8) {
  [u"scheme"]=>
  unicode(4) "http"
  [u"host"]=>
  unicode(11) "www.php.net"
  [u"port"]=>
  int(80)
  [u"user"]=>
  unicode(6) "secret"
  [u"pass"]=>
  unicode(7) "hideout"
  [u"path"]=>
  unicode(10) "/index.php"
  [u"query"]=>
  unicode(31) "test=1&test2=char&test3=mixesCI"
  [u"fragment"]=>
  unicode(16) "some_page_ref123"
}

Arg value 1.07654321E-9 
unicode(4) "http"

Arg value 0.5 
unicode(4) "http"
Error: 8 - Array to string conversion, %s(70)

Arg value Array 
Error: 2 - parse_url() expects parameter 2 to be long, array given, %s(71)
NULL
Error: 8 - Array to string conversion, %s(70)

Arg value Array 
Error: 2 - parse_url() expects parameter 2 to be long, array given, %s(71)
NULL
Error: 8 - Array to string conversion, %s(70)

Arg value Array 
Error: 2 - parse_url() expects parameter 2 to be long, array given, %s(71)
NULL
Error: 8 - Array to string conversion, %s(70)

Arg value Array 
Error: 2 - parse_url() expects parameter 2 to be long, array given, %s(71)
NULL
Error: 8 - Array to string conversion, %s(70)

Arg value Array 
Error: 2 - parse_url() expects parameter 2 to be long, array given, %s(71)
NULL

Arg value  
unicode(4) "http"

Arg value  
unicode(4) "http"

Arg value 1 
unicode(11) "www.php.net"

Arg value  
unicode(4) "http"

Arg value 1 
unicode(11) "www.php.net"

Arg value  
unicode(4) "http"

Arg value  
Error: 2 - parse_url() expects parameter 2 to be long, Unicode string given, %s(71)
NULL

Arg value  
Error: 2 - parse_url() expects parameter 2 to be long, Unicode string given, %s(71)
NULL

Arg value string 
Error: 2 - parse_url() expects parameter 2 to be long, Unicode string given, %s(71)
NULL

Arg value string 
Error: 2 - parse_url() expects parameter 2 to be long, Unicode string given, %s(71)
NULL
Error: 4096 - Object of class stdClass could not be converted to string, %s(70)

Arg value  
Error: 2 - parse_url() expects parameter 2 to be long, object given, %s(71)
NULL

Arg value  
unicode(4) "http"

Arg value  
unicode(4) "http"
Done
