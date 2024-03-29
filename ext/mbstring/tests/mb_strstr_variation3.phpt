--TEST--
Test mb_strstr() function : usage variation - different values for part
--SKIPIF--
<?php
extension_loaded('mbstring') or die('skip');
function_exists('mb_strstr') or die("skip mb_strstr() is not available in this build");
?>
--FILE--
<?php
/* Prototype  : string mb_strstr(string haystack, string needle[, bool part[, string encoding]])
 * Description: Finds first occurrence of a string within another 
 * Source code: ext/mbstring/mbstring.c
 * Alias to functions: 
 */

echo "*** Testing mb_strstr() : usage variation ***\n";

// Define error handler
function test_error_handler($err_no, $err_msg, $filename, $linenum, $vars) {
	if (error_reporting() != 0) {
		// report non-silenced errors
		echo "Error: $err_no - $err_msg, $filename($linenum)\n";
	}
}
set_error_handler('test_error_handler');

// Initialise function arguments not being substituted (if any)
$haystack = b'string_val';
$needle = b'_';
$encoding = 'utf-8';

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// define some classes
class classWithToString
{
	public function __toString() {
		return b"Class A object";
	}
}

class classWithoutToString
{
}

// heredoc string
$heredoc = b<<<EOT
hello world
EOT;

// get a resource variable
$fp = fopen(__FILE__, "r");

// add arrays
$index_array = array (1, 2, 3);
$assoc_array = array ('one' => 1, 'two' => 2);

//array of values to iterate over
$inputs = array(

      // int data
      'int 0' => 0,
      'int 1' => 1,
      'int 12345' => 12345,
      'int -12345' => -2345,

      // float data
      'float 10.5' => 10.5,
      'float -10.5' => -10.5,
      'float 12.3456789000e10' => 12.3456789000e10,
      'float -12.3456789000e10' => -12.3456789000e10,
      'float .5' => .5,

      // array data
      'empty array' => array(),
      'int indexed array' => $index_array,
      'associative array' => $assoc_array,
      'nested arrays' => array('foo', $index_array, $assoc_array),

      // null data
      'uppercase NULL' => NULL,
      'lowercase null' => null,

      // boolean data
      'lowercase true' => true,
      'lowercase false' =>false,
      'uppercase TRUE' =>TRUE,
      'uppercase FALSE' =>FALSE,

      // empty data
      'empty string DQ' => "",
      'empty string SQ' => '',

      // string data
      'string DQ' => "string",
      'string SQ' => 'string',
      'mixed case string' => "sTrInG",
      'heredoc' => $heredoc,

      // object data
      'instance of classWithToString' => new classWithToString(),
      'instance of classWithoutToString' => new classWithoutToString(),

      // undefined data
      'undefined var' => @$undefined_var,

      // unset data
      'unset var' => @$unset_var,
      
      // resource variable
      'resource' => $fp      
);

// loop through each element of the array for part

foreach($inputs as $key =>$value) {
      echo "\n--$key--\n";
      $res = mb_strstr($haystack, $needle, $value, $encoding);
      if ($res === false) {
         var_dump($res);
      }
      else {
         var_dump(bin2hex($res));
      }      
};

fclose($fp);

?>
===DONE===
--EXPECTF--
*** Testing mb_strstr() : usage variation ***

--int 0--
unicode(8) "5f76616c"

--int 1--
unicode(12) "737472696e67"

--int 12345--
unicode(12) "737472696e67"

--int -12345--
unicode(12) "737472696e67"

--float 10.5--
unicode(12) "737472696e67"

--float -10.5--
unicode(12) "737472696e67"

--float 12.3456789000e10--
unicode(12) "737472696e67"

--float -12.3456789000e10--
unicode(12) "737472696e67"

--float .5--
unicode(12) "737472696e67"

--empty array--
Error: 2 - mb_strstr() expects parameter 3 to be boolean, array given, %s(%d)
bool(false)

--int indexed array--
Error: 2 - mb_strstr() expects parameter 3 to be boolean, array given, %s(%d)
bool(false)

--associative array--
Error: 2 - mb_strstr() expects parameter 3 to be boolean, array given, %s(%d)
bool(false)

--nested arrays--
Error: 2 - mb_strstr() expects parameter 3 to be boolean, array given, %s(%d)
bool(false)

--uppercase NULL--
unicode(8) "5f76616c"

--lowercase null--
unicode(8) "5f76616c"

--lowercase true--
unicode(12) "737472696e67"

--lowercase false--
unicode(8) "5f76616c"

--uppercase TRUE--
unicode(12) "737472696e67"

--uppercase FALSE--
unicode(8) "5f76616c"

--empty string DQ--
unicode(8) "5f76616c"

--empty string SQ--
unicode(8) "5f76616c"

--string DQ--
unicode(12) "737472696e67"

--string SQ--
unicode(12) "737472696e67"

--mixed case string--
unicode(12) "737472696e67"

--heredoc--
unicode(12) "737472696e67"

--instance of classWithToString--
Error: 2 - mb_strstr() expects parameter 3 to be boolean, object given, %s(%d)
bool(false)

--instance of classWithoutToString--
Error: 2 - mb_strstr() expects parameter 3 to be boolean, object given, %s(%d)
bool(false)

--undefined var--
unicode(8) "5f76616c"

--unset var--
unicode(8) "5f76616c"

--resource--
Error: 2 - mb_strstr() expects parameter 3 to be boolean, resource given, %s(%d)
bool(false)
===DONE===
