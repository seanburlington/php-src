--TEST--
Test serialize() & unserialize() functions: many types
--SKIPIF--
<?php
if (PHP_INT_SIZE != 4) {
	    die("skip this test is for 32bit platform only");
}
?>
--FILE--
<?php 
/* Prototype  : proto string serialize(mixed variable)
 * Description: Returns a string representation of variable (which can later be unserialized) 
 * Source code: ext/standard/var.c
 * Alias to functions: 
 */
/* Prototype  : proto mixed unserialize(string variable_representation)
 * Description: Takes a string representation of variable and recreates it 
 * Source code: ext/standard/var.c
 * Alias to functions: 
 */


echo "--- Testing Various Types ---\n";

/* unset variable */
$unset_var = 10;
unset($unset_var);
/* array declaration */
$arr_var = array(0, 1, -2, 3.333333, "a", array(), array(NULL));

$Variation_arr = array( 
   /* Integers */
   2147483647,
   -2147483647,
   2147483648,
   -2147483648,

   0xFF00123,  // hex integers
   -0xFF00123,
   0x7FFFFFFF,
   -0x7FFFFFFF, 
   0x80000000,
   -0x80000000,

   01234567,  // octal integers
   -01234567,

   /* arrays */
   array(),  // zero elements
   array(1, 2, 3, 12345666, -2344),
   array(0, 1, 2, 3.333, -4, -5.555, TRUE, FALSE, NULL, "", '', " ", 
         array(), array(1,2,array()), "string", new stdclass
        ),
   &$arr_var,  // Reference to an array
 
  /* nulls */
   NULL,
   null,

  /* strings */
   "",
   '',
   " ",
   ' ',
   "a",
   "string",
   'string',
   "hello\0",
   'hello\0',
   "123",
   '123',
   '\t',
   "\t",

   /* booleans */
   TRUE,
   true,
   FALSE,
   false,

   /* Mixed types */
   @TRUE123,
   "123string",
   "string123",
   "NULLstring",

   /* unset/undefined  vars */
   @$unset_var,
   @$undefined_var,
);

/* Loop through to test each element in the above array */
for( $i = 0; $i < count($Variation_arr); $i++ ) {

  echo "\n-- Iteration $i --\n";
  echo "after serialization => "; 
  $serialize_data = serialize($Variation_arr[$i]);
  var_dump( $serialize_data );

  echo "after unserialization => "; 
  $unserialize_data = unserialize($serialize_data);
  var_dump( $unserialize_data );
}

echo "\nDone";
?>
--EXPECT--
--- Testing Various Types ---

-- Iteration 0 --
after serialization => unicode(13) "i:2147483647;"
after unserialization => int(2147483647)

-- Iteration 1 --
after serialization => unicode(14) "i:-2147483647;"
after unserialization => int(-2147483647)

-- Iteration 2 --
after serialization => unicode(13) "d:2147483648;"
after unserialization => float(2147483648)

-- Iteration 3 --
after serialization => unicode(14) "d:-2147483648;"
after unserialization => float(-2147483648)

-- Iteration 4 --
after serialization => unicode(12) "i:267387171;"
after unserialization => int(267387171)

-- Iteration 5 --
after serialization => unicode(13) "i:-267387171;"
after unserialization => int(-267387171)

-- Iteration 6 --
after serialization => unicode(13) "i:2147483647;"
after unserialization => int(2147483647)

-- Iteration 7 --
after serialization => unicode(14) "i:-2147483647;"
after unserialization => int(-2147483647)

-- Iteration 8 --
after serialization => unicode(13) "d:2147483648;"
after unserialization => float(2147483648)

-- Iteration 9 --
after serialization => unicode(14) "d:-2147483648;"
after unserialization => float(-2147483648)

-- Iteration 10 --
after serialization => unicode(9) "i:342391;"
after unserialization => int(342391)

-- Iteration 11 --
after serialization => unicode(10) "i:-342391;"
after unserialization => int(-342391)

-- Iteration 12 --
after serialization => unicode(6) "a:0:{}"
after unserialization => array(0) {
}

-- Iteration 13 --
after serialization => unicode(57) "a:5:{i:0;i:1;i:1;i:2;i:2;i:3;i:3;i:12345666;i:4;i:-2344;}"
after unserialization => array(5) {
  [0]=>
  int(1)
  [1]=>
  int(2)
  [2]=>
  int(3)
  [3]=>
  int(12345666)
  [4]=>
  int(-2344)
}

-- Iteration 14 --
after serialization => unicode(303) "a:16:{i:0;i:0;i:1;i:1;i:2;i:2;i:3;d:3.3330000000000001847411112976260483264923095703125;i:4;i:-4;i:5;d:-5.55499999999999971578290569595992565155029296875;i:6;b:1;i:7;b:0;i:8;N;i:9;U:0:"";i:10;U:0:"";i:11;U:1:" ";i:12;a:0:{}i:13;a:3:{i:0;i:1;i:1;i:2;i:2;a:0:{}}i:14;U:6:"string";i:15;O:8:"stdClass":0:{}}"
after unserialization => array(16) {
  [0]=>
  int(0)
  [1]=>
  int(1)
  [2]=>
  int(2)
  [3]=>
  float(3.333)
  [4]=>
  int(-4)
  [5]=>
  float(-5.555)
  [6]=>
  bool(true)
  [7]=>
  bool(false)
  [8]=>
  NULL
  [9]=>
  unicode(0) ""
  [10]=>
  unicode(0) ""
  [11]=>
  unicode(1) " "
  [12]=>
  array(0) {
  }
  [13]=>
  array(3) {
    [0]=>
    int(1)
    [1]=>
    int(2)
    [2]=>
    array(0) {
    }
  }
  [14]=>
  unicode(6) "string"
  [15]=>
  object(stdClass)#2 (0) {
  }
}

-- Iteration 15 --
after serialization => unicode(129) "a:7:{i:0;i:0;i:1;i:1;i:2;i:-2;i:3;d:3.333333000000000101437080957111902534961700439453125;i:4;U:1:"a";i:5;a:0:{}i:6;a:1:{i:0;N;}}"
after unserialization => array(7) {
  [0]=>
  int(0)
  [1]=>
  int(1)
  [2]=>
  int(-2)
  [3]=>
  float(3.333333)
  [4]=>
  unicode(1) "a"
  [5]=>
  array(0) {
  }
  [6]=>
  array(1) {
    [0]=>
    NULL
  }
}

-- Iteration 16 --
after serialization => unicode(2) "N;"
after unserialization => NULL

-- Iteration 17 --
after serialization => unicode(2) "N;"
after unserialization => NULL

-- Iteration 18 --
after serialization => unicode(7) "U:0:"";"
after unserialization => unicode(0) ""

-- Iteration 19 --
after serialization => unicode(7) "U:0:"";"
after unserialization => unicode(0) ""

-- Iteration 20 --
after serialization => unicode(8) "U:1:" ";"
after unserialization => unicode(1) " "

-- Iteration 21 --
after serialization => unicode(8) "U:1:" ";"
after unserialization => unicode(1) " "

-- Iteration 22 --
after serialization => unicode(8) "U:1:"a";"
after unserialization => unicode(1) "a"

-- Iteration 23 --
after serialization => unicode(13) "U:6:"string";"
after unserialization => unicode(6) "string"

-- Iteration 24 --
after serialization => unicode(13) "U:6:"string";"
after unserialization => unicode(6) "string"

-- Iteration 25 --
after serialization => unicode(13) "U:6:"hello ";"
after unserialization => unicode(6) "hello "

-- Iteration 26 --
after serialization => unicode(18) "U:7:"hello\005c0";"
after unserialization => unicode(7) "hello\0"

-- Iteration 27 --
after serialization => unicode(10) "U:3:"123";"
after unserialization => unicode(3) "123"

-- Iteration 28 --
after serialization => unicode(10) "U:3:"123";"
after unserialization => unicode(3) "123"

-- Iteration 29 --
after serialization => unicode(13) "U:2:"\005ct";"
after unserialization => unicode(2) "\t"

-- Iteration 30 --
after serialization => unicode(8) "U:1:"	";"
after unserialization => unicode(1) "	"

-- Iteration 31 --
after serialization => unicode(4) "b:1;"
after unserialization => bool(true)

-- Iteration 32 --
after serialization => unicode(4) "b:1;"
after unserialization => bool(true)

-- Iteration 33 --
after serialization => unicode(4) "b:0;"
after unserialization => bool(false)

-- Iteration 34 --
after serialization => unicode(4) "b:0;"
after unserialization => bool(false)

-- Iteration 35 --
after serialization => unicode(14) "U:7:"TRUE123";"
after unserialization => unicode(7) "TRUE123"

-- Iteration 36 --
after serialization => unicode(16) "U:9:"123string";"
after unserialization => unicode(9) "123string"

-- Iteration 37 --
after serialization => unicode(16) "U:9:"string123";"
after unserialization => unicode(9) "string123"

-- Iteration 38 --
after serialization => unicode(18) "U:10:"NULLstring";"
after unserialization => unicode(10) "NULLstring"

-- Iteration 39 --
after serialization => unicode(2) "N;"
after unserialization => NULL

-- Iteration 40 --
after serialization => unicode(2) "N;"
after unserialization => NULL

Done
