--TEST--
Test image_type_to_mime_type() function : usage variations  - Pass different data types as imagetype
--FILE--
<?php
/* Prototype  : string image_type_to_mime_type(int imagetype)
 * Description: Get Mime-Type for image-type returned by getimagesize, exif_read_data, exif_thumbnail, exif_imagetype 
 * Source code: ext/standard/image.c
 */

echo "*** Testing image_type_to_mime_type() : usage variations ***\n";

//get an unset variable
$unset_var = 10;
unset ($unset_var);

class MyClass
{
  function __toString() {
    return "MyClass";
  }
}

//array of values to iterate over
$values = array(

      // float data
      100.5,
      -100.5,
      100.1234567e10,
      100.7654321E-10,
      .5,

      // array data
      array(),
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
      new MyClass(),

      // undefined data
      @$undefined_var,

      // unset data
      @$unset_var,
);

// loop through each element of the array for imagetype
$iterator = 1;
foreach($values as $value) {
      echo "\n-- Iteration $iterator --\n";
      var_dump( image_type_to_mime_type($value) );
      $iterator++;
};
?>
===DONE===
--EXPECTF--
*** Testing image_type_to_mime_type() : usage variations ***

-- Iteration 1 --
unicode(24) "application/octet-stream"

-- Iteration 2 --
unicode(24) "application/octet-stream"

-- Iteration 3 --
unicode(24) "application/octet-stream"

-- Iteration 4 --
unicode(24) "application/octet-stream"

-- Iteration 5 --
unicode(24) "application/octet-stream"

-- Iteration 6 --

Warning: image_type_to_mime_type() expects parameter 1 to be long, array given in %s on line %d
NULL

-- Iteration 7 --

Warning: image_type_to_mime_type() expects parameter 1 to be long, array given in %s on line %d
NULL

-- Iteration 8 --
unicode(24) "application/octet-stream"

-- Iteration 9 --
unicode(24) "application/octet-stream"

-- Iteration 10 --
unicode(9) "image/gif"

-- Iteration 11 --
unicode(24) "application/octet-stream"

-- Iteration 12 --
unicode(9) "image/gif"

-- Iteration 13 --
unicode(24) "application/octet-stream"

-- Iteration 14 --

Warning: image_type_to_mime_type() expects parameter 1 to be long, Unicode string given in %s on line %d
NULL

-- Iteration 15 --

Warning: image_type_to_mime_type() expects parameter 1 to be long, Unicode string given in %s on line %d
NULL

-- Iteration 16 --

Warning: image_type_to_mime_type() expects parameter 1 to be long, Unicode string given in %s on line %d
NULL

-- Iteration 17 --

Warning: image_type_to_mime_type() expects parameter 1 to be long, Unicode string given in %s on line %d
NULL

-- Iteration 18 --

Warning: image_type_to_mime_type() expects parameter 1 to be long, object given in %s on line %d
NULL

-- Iteration 19 --
unicode(24) "application/octet-stream"

-- Iteration 20 --
unicode(24) "application/octet-stream"
===DONE===
