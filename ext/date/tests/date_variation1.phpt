--TEST--
Test date() function : usage variation - Passing unexpected values to first argument $format.
--FILE--
<?php
/* Prototype  : string date  ( string $format  [, int $timestamp  ] )
 * Description: Format a local time/date.
 * Source code: ext/date/php_date.c
 */

echo "*** Testing date() : usage variation -  unexpected values to first argument \$format***\n";

//Set the default time zone 
date_default_timezone_set("Europe/London");

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

// resource
$file_handle = fopen(__FILE__, 'r');

//array of values to iterate over
$inputs = array(

      // int data
      'int 0' => 0,
      'int 1' => 1,
      'int 12345' => 12345,
      'int -12345' => -12345,

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
      
      // resource 
      'resource' => $file_handle
);

$timestamp = mktime(10, 44, 30, 2, 27, 2009);

foreach($inputs as $variation =>$format) {
      echo "\n-- $variation --\n";
      var_dump( date($format, $timestamp) );
};

// closing the resource
fclose( $file_handle );

?>
===DONE===
--EXPECTF--
*** Testing date() : usage variation -  unexpected values to first argument $format***

-- int 0 --
unicode(1) "0"

-- int 1 --
unicode(1) "1"

-- int 12345 --
unicode(5) "12345"

-- int -12345 --
unicode(6) "-12345"

-- float 10.5 --
unicode(4) "10.5"

-- float -10.5 --
unicode(5) "-10.5"

-- float .5 --
unicode(3) "0.5"

-- empty array --

Warning: date() expects parameter 1 to be binary string, array given in %s on line %d
bool(false)

-- int indexed array --

Warning: date() expects parameter 1 to be binary string, array given in %s on line %d
bool(false)

-- associative array --

Warning: date() expects parameter 1 to be binary string, array given in %s on line %d
bool(false)

-- nested arrays --

Warning: date() expects parameter 1 to be binary string, array given in %s on line %d
bool(false)

-- uppercase NULL --
unicode(0) ""

-- lowercase null --
unicode(0) ""

-- lowercase true --
unicode(1) "1"

-- lowercase false --
unicode(0) ""

-- uppercase TRUE --
unicode(1) "1"

-- uppercase FALSE --
unicode(0) ""

-- empty string DQ --
unicode(0) ""

-- empty string SQ --
unicode(0) ""

-- string DQ --
unicode(40) "3028Fri, 27 Feb 2009 10:44:30 +000044210"

-- string SQ --
unicode(40) "3028Fri, 27 Feb 2009 10:44:30 +000044210"

-- mixed case string --
unicode(40) "30GMTFri, 27 Feb 2009 10:44:30 +00000210"

-- heredoc --
unicode(76) "10Europe/LondonFridayFriday2009 52009Fri, 27 Feb 2009 10:44:30 +0000Friday27"

-- instance of classWithToString --
unicode(64) "CFridayam3030 AM 2009b27Europe/London2009-02-27T10:44:30+00:0028"

-- instance of classWithoutToString --

Warning: date() expects parameter 1 to be binary string, object given in %s on line %d
bool(false)

-- undefined var --
unicode(0) ""

-- unset var --
unicode(0) ""

-- resource --

Warning: date() expects parameter 1 to be binary string, resource given in %s on line %d
bool(false)
===DONE===
