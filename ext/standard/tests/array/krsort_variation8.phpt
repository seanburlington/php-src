--TEST--
Test krsort() function : usage variations - sort mixed values, 'sort_flags' as default/SORT_REGULAR (OK to fail as result is unpredectable) 
--FILE--
<?php
/* Prototype  : bool krsort ( array &$array [, int $sort_flags] )
 * Description: Sort an array by key in reverse order, maintaining key to data correlation. 
 * Source code: ext/standard/array.c
*/

/*
 * testing krsort() by providing array of mixed values for $array argument 
 * with following flag  values:
 *  1.flag value as default
 *  2.SORT_REGULAR - compare items normally
*/

echo "*** Testing krsort() : usage variations ***\n";

// mixed value array with different key values
$mixed_values = array (
  "array1" => array(), 
  "array2" => array ( "sub_array[2,1]" => array(33,-5,6), "sub_array[2,2]" => array(11), 
                      "sub_array[2,3]" => array(22,-55), "sub_array[2,4]" => array() 
                    ),
  4 => 4, "4" => "4", 4.01 => 4.01, "b" => "b", "5" => "5", -2 => -2, -2.01 => -2.01, 
  -2.98989 => -2.98989, "-.9" => "-.9", "True" => "True", "" =>  "", NULL => NULL,
  "ab" => "ab", "abcd" => "abcd", 0.01 => 0.01, -0 => -0, '' => '' ,
  "abcd\x00abcd\x00abcd" => "abcd\x00abcd\x00abcd", 0.001 => 0.001
);

echo "\n-- Testing krsort() by supplying mixed value array, 'flag' value is default --\n";
$temp_array = $mixed_values;
var_dump( krsort($temp_array) );
var_dump($temp_array);

echo "\n-- Testing krsort() by supplying mixed value array, 'flag' value is SORT_REGULAR --\n";
$temp_array = $mixed_values;
var_dump( krsort($temp_array, SORT_REGULAR) );
var_dump($temp_array);

echo "Done\n";
?>
--EXPECT--
*** Testing krsort() : usage variations ***

-- Testing krsort() by supplying mixed value array, 'flag' value is default --
bool(true)
array(13) {
  [5]=>
  unicode(1) "5"
  [4]=>
  float(4.01)
  [0]=>
  float(0.001)
  [u"b"]=>
  unicode(1) "b"
  [u"array2"]=>
  array(4) {
    [u"sub_array[2,1]"]=>
    array(3) {
      [0]=>
      int(33)
      [1]=>
      int(-5)
      [2]=>
      int(6)
    }
    [u"sub_array[2,2]"]=>
    array(1) {
      [0]=>
      int(11)
    }
    [u"sub_array[2,3]"]=>
    array(2) {
      [0]=>
      int(22)
      [1]=>
      int(-55)
    }
    [u"sub_array[2,4]"]=>
    array(0) {
    }
  }
  [u"array1"]=>
  array(0) {
  }
  [u"abcd abcd abcd"]=>
  unicode(14) "abcd abcd abcd"
  [u"abcd"]=>
  unicode(4) "abcd"
  [u"ab"]=>
  unicode(2) "ab"
  [u"True"]=>
  unicode(4) "True"
  [u"-.9"]=>
  unicode(3) "-.9"
  [u""]=>
  unicode(0) ""
  [-2]=>
  float(-2.98989)
}

-- Testing krsort() by supplying mixed value array, 'flag' value is SORT_REGULAR --
bool(true)
array(13) {
  [5]=>
  unicode(1) "5"
  [4]=>
  float(4.01)
  [0]=>
  float(0.001)
  [u"b"]=>
  unicode(1) "b"
  [u"array2"]=>
  array(4) {
    [u"sub_array[2,1]"]=>
    array(3) {
      [0]=>
      int(33)
      [1]=>
      int(-5)
      [2]=>
      int(6)
    }
    [u"sub_array[2,2]"]=>
    array(1) {
      [0]=>
      int(11)
    }
    [u"sub_array[2,3]"]=>
    array(2) {
      [0]=>
      int(22)
      [1]=>
      int(-55)
    }
    [u"sub_array[2,4]"]=>
    array(0) {
    }
  }
  [u"array1"]=>
  array(0) {
  }
  [u"abcd abcd abcd"]=>
  unicode(14) "abcd abcd abcd"
  [u"abcd"]=>
  unicode(4) "abcd"
  [u"ab"]=>
  unicode(2) "ab"
  [u"True"]=>
  unicode(4) "True"
  [u"-.9"]=>
  unicode(3) "-.9"
  [u""]=>
  unicode(0) ""
  [-2]=>
  float(-2.98989)
}
Done
