--TEST--
Test array_diff_uassoc() function : usage variation - Passing null,unset and undefined variable indexed array
--FILE--
<?php
/* Prototype  : array array_diff_uassoc(array arr1, array arr2 [, array ...], callback key_comp_func)
 * Description: Computes the difference of arrays with additional index check which is performed by a 
 * 				user supplied callback function
 * Source code: ext/standard/array.c
 */

echo "*** Testing array_diff_uassoc() : usage variation ***\n";

// Initialise function arguments not being substituted (if any)
$input_array = array(10 => '10', "" => ''); 

//get an unset variable
$unset_var = 10;
unset ($unset_var);

$input_arrays = array(
      'null indexed' => array(NULL => NULL, null => null),
      'undefined indexed' => array(@$undefined_var => @$undefined_var),
      'unset indexed' => array(@$unset_var => @$unset_var),
);

foreach($input_arrays as $key =>$value) {
      echo "\n--$key--\n";
      var_dump( array_diff_uassoc($input_array, $value, "strcasecmp") );
      var_dump( array_diff_uassoc($value, $input_array, "strcasecmp") );
}      
    
?>
===DONE===
--EXPECTF--
*** Testing array_diff_uassoc() : usage variation ***

--null indexed--
array(1) {
  [10]=>
  unicode(2) "10"
}
array(0) {
}

--undefined indexed--
array(1) {
  [10]=>
  unicode(2) "10"
}
array(0) {
}

--unset indexed--
array(1) {
  [10]=>
  unicode(2) "10"
}
array(0) {
}
===DONE===
