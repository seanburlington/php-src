--TEST--
Test unlink() function : usage variation different types for context
--CREDITS--
Dave Kelsey <d_kelsey@uk.ibm.com>
--FILE--
<?php
/* Prototype  : bool unlink(string filename[, context context])
 * Description: Delete a file 
 * Source code: ext/standard/file.c
 * Alias to functions: 
 */

echo "*** Testing unlink() : usage variation different types for context ***\n";

// Define error handler
function test_error_handler($err_no, $err_msg, $filename, $linenum, $vars) {
	if (error_reporting() != 0) {
		// report non-silenced errors
		echo "Error: $err_no - $err_msg, $filename($linenum)\n";
	}
}
set_error_handler('test_error_handler');

// Initialise function arguments not being substituted (if any)
$filename = __FILE__.'tmp';
$fileRes = fopen(__FILE__, 'r');

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
      
      // file resource
      'file resource' => $fileRes
);

// loop through each element of the array for context

foreach($inputs as $key =>$value) {
      echo "\n--$key--\n";
      touch($filename);  // create if it doesn't exist
      var_dump( unlink($filename, $value) );
};

if (file_exists($filename)) {
   unlink($filename);
}
fclose($fileRes);

?>
===DONE===
--EXPECTF--
*** Testing unlink() : usage variation different types for context ***

--int 0--
Error: 2 - unlink() expects parameter 2 to be resource, integer given, %s(%d)
bool(false)

--int 1--
Error: 2 - unlink() expects parameter 2 to be resource, integer given, %s(%d)
bool(false)

--int 12345--
Error: 2 - unlink() expects parameter 2 to be resource, integer given, %s(%d)
bool(false)

--int -12345--
Error: 2 - unlink() expects parameter 2 to be resource, integer given, %s(%d)
bool(false)

--float 10.5--
Error: 2 - unlink() expects parameter 2 to be resource, double given, %s(%d)
bool(false)

--float -10.5--
Error: 2 - unlink() expects parameter 2 to be resource, double given, %s(%d)
bool(false)

--float 12.3456789000e10--
Error: 2 - unlink() expects parameter 2 to be resource, double given, %s(%d)
bool(false)

--float -12.3456789000e10--
Error: 2 - unlink() expects parameter 2 to be resource, double given, %s(%d)
bool(false)

--float .5--
Error: 2 - unlink() expects parameter 2 to be resource, double given, %s(%d)
bool(false)

--empty array--
Error: 2 - unlink() expects parameter 2 to be resource, array given, %s(%d)
bool(false)

--int indexed array--
Error: 2 - unlink() expects parameter 2 to be resource, array given, %s(%d)
bool(false)

--associative array--
Error: 2 - unlink() expects parameter 2 to be resource, array given, %s(%d)
bool(false)

--nested arrays--
Error: 2 - unlink() expects parameter 2 to be resource, array given, %s(%d)
bool(false)

--uppercase NULL--
Error: 2 - unlink() expects parameter 2 to be resource, null given, %s(%d)
bool(false)

--lowercase null--
Error: 2 - unlink() expects parameter 2 to be resource, null given, %s(%d)
bool(false)

--lowercase true--
Error: 2 - unlink() expects parameter 2 to be resource, boolean given, %s(%d)
bool(false)

--lowercase false--
Error: 2 - unlink() expects parameter 2 to be resource, boolean given, %s(%d)
bool(false)

--uppercase TRUE--
Error: 2 - unlink() expects parameter 2 to be resource, boolean given, %s(%d)
bool(false)

--uppercase FALSE--
Error: 2 - unlink() expects parameter 2 to be resource, boolean given, %s(%d)
bool(false)

--empty string DQ--
Error: 2 - unlink() expects parameter 2 to be resource, Unicode string given, %s(%d)
bool(false)

--empty string SQ--
Error: 2 - unlink() expects parameter 2 to be resource, Unicode string given, %s(%d)
bool(false)

--string DQ--
Error: 2 - unlink() expects parameter 2 to be resource, Unicode string given, %s(%d)
bool(false)

--string SQ--
Error: 2 - unlink() expects parameter 2 to be resource, Unicode string given, %s(%d)
bool(false)

--mixed case string--
Error: 2 - unlink() expects parameter 2 to be resource, Unicode string given, %s(%d)
bool(false)

--heredoc--
Error: 2 - unlink() expects parameter 2 to be resource, Unicode string given, %s(%d)
bool(false)

--instance of classWithToString--
Error: 2 - unlink() expects parameter 2 to be resource, object given, %s(%d)
bool(false)

--instance of classWithoutToString--
Error: 2 - unlink() expects parameter 2 to be resource, object given, %s(%d)
bool(false)

--undefined var--
Error: 2 - unlink() expects parameter 2 to be resource, null given, %s(%d)
bool(false)

--unset var--
Error: 2 - unlink() expects parameter 2 to be resource, null given, %s(%d)
bool(false)

--file resource--
Error: 2 - unlink(): supplied resource is not a valid Stream-Context resource, %s(%d)
bool(true)
===DONE===
