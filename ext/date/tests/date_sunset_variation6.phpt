--TEST--
Test date_sunset() function : usage variation - Passing unexpected values to sixth argument gmt_offset.
--FILE--
<?php
/* Prototype  : mixed date_sunset(mixed time [, int format [, float latitude [, float longitude [, float zenith [, float gmt_offset]]]]])
 * Description: Returns time of sunset for a given day and location 
 * Source code: ext/date/php_date.c
 * Alias to functions: 
 */

echo "*** Testing date_sunset() : usage variation ***\n";

// Initialise function arguments not being substituted (if any)
date_default_timezone_set("Asia/Calcutta");
$time = mktime(8, 8, 8, 8, 8, 2008);
$longitude = 88.21;
$latitude = 22.34;
$zenith = 90;

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

// loop through each element of the array for gmt_offset

foreach($inputs as $key =>$value) {
      echo "\n--$key--\n";
      var_dump( date_sunset($time, SUNFUNCS_RET_STRING, $latitude, $longitude, $zenith, $value) );
      var_dump( date_sunset($time, SUNFUNCS_RET_DOUBLE, $latitude, $longitude, $zenith, $value) );
      var_dump( date_sunset($time, SUNFUNCS_RET_TIMESTAMP, $latitude, $longitude, $zenith, $value) );
};

?>
===DONE===
--EXPECTF--
*** Testing date_sunset() : usage variation ***

--int 0--
unicode(5) "12:40"
float(12.681%d)
int(1218199253)

--int 1--
unicode(5) "13:40"
float(13.681%d)
int(1218199253)

--int 12345--
unicode(5) "21:40"
float(21.681%d)
int(1218199253)

--int -12345--
unicode(5) "19:40"
float(19.681%d)
int(1218199253)

--empty array--

Warning: date_sunset() expects parameter 6 to be double, array given in %s on line %d
bool(false)

Warning: date_sunset() expects parameter 6 to be double, array given in %s on line %d
bool(false)

Warning: date_sunset() expects parameter 6 to be double, array given in %s on line %d
bool(false)

--int indexed array--

Warning: date_sunset() expects parameter 6 to be double, array given in %s on line %d
bool(false)

Warning: date_sunset() expects parameter 6 to be double, array given in %s on line %d
bool(false)

Warning: date_sunset() expects parameter 6 to be double, array given in %s on line %d
bool(false)

--associative array--

Warning: date_sunset() expects parameter 6 to be double, array given in %s on line %d
bool(false)

Warning: date_sunset() expects parameter 6 to be double, array given in %s on line %d
bool(false)

Warning: date_sunset() expects parameter 6 to be double, array given in %s on line %d
bool(false)

--nested arrays--

Warning: date_sunset() expects parameter 6 to be double, array given in %s on line %d
bool(false)

Warning: date_sunset() expects parameter 6 to be double, array given in %s on line %d
bool(false)

Warning: date_sunset() expects parameter 6 to be double, array given in %s on line %d
bool(false)

--uppercase NULL--
unicode(5) "12:40"
float(12.681%d)
int(1218199253)

--lowercase null--
unicode(5) "12:40"
float(12.681%d)
int(1218199253)

--lowercase true--
unicode(5) "13:40"
float(13.681%d)
int(1218199253)

--lowercase false--
unicode(5) "12:40"
float(12.681%d)
int(1218199253)

--uppercase TRUE--
unicode(5) "13:40"
float(13.681%d)
int(1218199253)

--uppercase FALSE--
unicode(5) "12:40"
float(12.681%d)
int(1218199253)

--empty string DQ--

Warning: date_sunset() expects parameter 6 to be double, Unicode string given in %s on line %d
bool(false)

Warning: date_sunset() expects parameter 6 to be double, Unicode string given in %s on line %d
bool(false)

Warning: date_sunset() expects parameter 6 to be double, Unicode string given in %s on line %d
bool(false)

--empty string SQ--

Warning: date_sunset() expects parameter 6 to be double, Unicode string given in %s on line %d
bool(false)

Warning: date_sunset() expects parameter 6 to be double, Unicode string given in %s on line %d
bool(false)

Warning: date_sunset() expects parameter 6 to be double, Unicode string given in %s on line %d
bool(false)

--string DQ--

Warning: date_sunset() expects parameter 6 to be double, Unicode string given in %s on line %d
bool(false)

Warning: date_sunset() expects parameter 6 to be double, Unicode string given in %s on line %d
bool(false)

Warning: date_sunset() expects parameter 6 to be double, Unicode string given in %s on line %d
bool(false)

--string SQ--

Warning: date_sunset() expects parameter 6 to be double, Unicode string given in %s on line %d
bool(false)

Warning: date_sunset() expects parameter 6 to be double, Unicode string given in %s on line %d
bool(false)

Warning: date_sunset() expects parameter 6 to be double, Unicode string given in %s on line %d
bool(false)

--mixed case string--

Warning: date_sunset() expects parameter 6 to be double, Unicode string given in %s on line %d
bool(false)

Warning: date_sunset() expects parameter 6 to be double, Unicode string given in %s on line %d
bool(false)

Warning: date_sunset() expects parameter 6 to be double, Unicode string given in %s on line %d
bool(false)

--heredoc--

Warning: date_sunset() expects parameter 6 to be double, Unicode string given in %s on line %d
bool(false)

Warning: date_sunset() expects parameter 6 to be double, Unicode string given in %s on line %d
bool(false)

Warning: date_sunset() expects parameter 6 to be double, Unicode string given in %s on line %d
bool(false)

--instance of classWithToString--

Warning: date_sunset() expects parameter 6 to be double, object given in %s on line %d
bool(false)

Warning: date_sunset() expects parameter 6 to be double, object given in %s on line %d
bool(false)

Warning: date_sunset() expects parameter 6 to be double, object given in %s on line %d
bool(false)

--instance of classWithoutToString--

Warning: date_sunset() expects parameter 6 to be double, object given in %s on line %d
bool(false)

Warning: date_sunset() expects parameter 6 to be double, object given in %s on line %d
bool(false)

Warning: date_sunset() expects parameter 6 to be double, object given in %s on line %d
bool(false)

--undefined var--
unicode(5) "12:40"
float(12.681%d)
int(1218199253)

--unset var--
unicode(5) "12:40"
float(12.681%d)
int(1218199253)
===DONE===
