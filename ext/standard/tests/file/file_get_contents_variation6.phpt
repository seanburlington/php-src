--TEST--
Test file_get_contents() function : usage variation 
--XFAIL--
Pending completion of Unicode streams
--CREDITS--
Dave Kelsey <d_kelsey@uk.ibm.com>
--FILE--
<?php
/* Prototype  : string file_get_contents(string filename [, bool use_include_path [, resource context [, long offset [, long maxlen]]]])
 * Description: Read the entire file into a string 
 * Source code: ext/standard/file.c
 * Alias to functions: 
 */

echo "*** Testing file_get_contents() : usage variation ***\n";

// Define error handler
function test_error_handler($err_no, $err_msg, $filename, $linenum, $vars) {
	if (error_reporting() != 0) {
		// report non-silenced errors
		echo "Error: $err_no - $err_msg, $filename($linenum)\n";
	}
}
set_error_handler('test_error_handler');

// Initialise function arguments not being substituted (if any)
$filename = 'FileGetContentsVar5.tmp';
$absFile = dirname(__FILE__).'/'.$filename;
$h = fopen($absFile,"w");
fwrite($h, "contents read");
fclose($h);

//get an unset variable
$unset_var = 10;
unset ($unset_var);

// define some classes
class classWithToString
{
	public function __toString() {
		return "Class A object";
	}
}

class classWithoutToString
{
}

// heredoc string
$heredoc = <<<EOT
hello world
EOT;

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
);

// loop through each element of the array for maxlen

foreach($inputs as $key =>$value) {
      echo "\n--$key--\n";
      var_dump( file_get_contents($absFile, false, null, 0, $value) );
};

unlink($absFile);

?>
===DONE===
--EXPECTF--
*** Testing file_get_contents() : usage variation ***

--int 0--
unicode(%d) ""

--int 1--
unicode(%d) "c"

--int 12345--
unicode(%d) "contents read"

--int -12345--
Error: 2 - file_get_contents(): length must be greater than or equal to zero, %s(%d)
bool(false)

--float 10.5--
unicode(%d) "contents r"

--float -10.5--
Error: 2 - file_get_contents(): length must be greater than or equal to zero, %s(%d)
bool(false)

--float .5--
unicode(%d) ""

--empty array--
Error: 2 - file_get_contents() expects parameter 5 to be long, array given, %s(%d)
NULL

--int indexed array--
Error: 2 - file_get_contents() expects parameter 5 to be long, array given, %s(%d)
NULL

--associative array--
Error: 2 - file_get_contents() expects parameter 5 to be long, array given, %s(%d)
NULL

--nested arrays--
Error: 2 - file_get_contents() expects parameter 5 to be long, array given, %s(%d)
NULL

--uppercase NULL--
unicode(%d) ""

--lowercase null--
unicode(%d) ""

--lowercase true--
unicode(%d) "c"

--lowercase false--
unicode(%d) ""

--uppercase TRUE--
unicode(%d) "c"

--uppercase FALSE--
unicode(%d) ""

--empty string DQ--
Error: 2 - file_get_contents() expects parameter 5 to be long, Unicode string given, %s(%d)
NULL

--empty string SQ--
Error: 2 - file_get_contents() expects parameter 5 to be long, Unicode string given, %s(%d)
NULL

--string DQ--
Error: 2 - file_get_contents() expects parameter 5 to be long, Unicode string given, %s(%d)
NULL

--string SQ--
Error: 2 - file_get_contents() expects parameter 5 to be long, Unicode string given, %s(%d)
NULL

--mixed case string--
Error: 2 - file_get_contents() expects parameter 5 to be long, Unicode string given, %s(%d)
NULL

--heredoc--
Error: 2 - file_get_contents() expects parameter 5 to be long, Unicode string given, %s(%d)
NULL

--instance of classWithToString--
Error: 2 - file_get_contents() expects parameter 5 to be long, object given, %s(%d)
NULL

--instance of classWithoutToString--
Error: 2 - file_get_contents() expects parameter 5 to be long, object given, %s(%d)
NULL

--undefined var--
unicode(%d) ""

--unset var--
unicode(%d) ""
===DONE===
