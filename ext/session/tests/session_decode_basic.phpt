--TEST--
Test session_decode() function : basic functionality
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php

ob_start();

/* 
 * Prototype : string session_decode(void)
 * Description : Decodes session data from a string
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_decode() : basic functionality ***\n";

// Get an unset variable
$unset_var = 10;
unset($unset_var);

class classA
{
    public function __toString() {
        return "Hello World!";
    }
}

$heredoc = <<<EOT
Hello World!
EOT;

$fp = fopen(__FILE__, "r");

// Unexpected values to be passed as arguments
$inputs = array(

       // Integer data
/*1*/  0,
       1,
       12345,
       -2345,

       // Float data
/*5*/  10.5,
       -10.5,
       12.3456789000e10,
       12.3456789000E-10,
       .5,

       // Null data
/*10*/ NULL,
       null,

       // Boolean data
/*12*/ true,
       false,
       TRUE,
       FALSE,
       
       // Empty strings
/*16*/ "",
       '',

       // Invalid string data
/*18*/ "Nothing",
       'Nothing',
       $heredoc,
       
       // Object data
/*21*/ new classA(),

       // Undefined data
/*22*/ @$undefined_var,

       // Unset data
/*23*/ @$unset_var,

       // Resource variable
/*24*/ $fp
);

var_dump(session_start());
$iterator = 1;
foreach($inputs as $input) {
    echo "\n-- Iteration $iterator --\n";
    $_SESSION["data"] = $input;
    $encoded = session_encode();
    var_dump(session_decode($encoded));
    var_dump($_SESSION);
    $iterator++;
};

var_dump(session_destroy());
fclose($fp);
echo "Done";
ob_end_flush();
?>
--EXPECTF--
*** Testing session_decode() : basic functionality ***
bool(true)

-- Iteration 1 --
bool(true)
array(1) {
  [u"data"]=>
  int(0)
}

-- Iteration 2 --
bool(true)
array(1) {
  [u"data"]=>
  int(1)
}

-- Iteration 3 --
bool(true)
array(1) {
  [u"data"]=>
  int(12345)
}

-- Iteration 4 --
bool(true)
array(1) {
  [u"data"]=>
  int(-2345)
}

-- Iteration 5 --
bool(true)
array(1) {
  [u"data"]=>
  float(10.5)
}

-- Iteration 6 --
bool(true)
array(1) {
  [u"data"]=>
  float(-10.5)
}

-- Iteration 7 --
bool(true)
array(1) {
  [u"data"]=>
  float(123456789000)
}

-- Iteration 8 --
bool(true)
array(1) {
  [u"data"]=>
  float(1.23456789E-9)
}

-- Iteration 9 --
bool(true)
array(1) {
  [u"data"]=>
  float(0.5)
}

-- Iteration 10 --
bool(true)
array(1) {
  [u"data"]=>
  NULL
}

-- Iteration 11 --
bool(true)
array(1) {
  [u"data"]=>
  NULL
}

-- Iteration 12 --
bool(true)
array(1) {
  [u"data"]=>
  bool(true)
}

-- Iteration 13 --
bool(true)
array(1) {
  [u"data"]=>
  bool(false)
}

-- Iteration 14 --
bool(true)
array(1) {
  [u"data"]=>
  bool(true)
}

-- Iteration 15 --
bool(true)
array(1) {
  [u"data"]=>
  bool(false)
}

-- Iteration 16 --
bool(true)
array(1) {
  [u"data"]=>
  unicode(0) ""
}

-- Iteration 17 --
bool(true)
array(1) {
  [u"data"]=>
  unicode(0) ""
}

-- Iteration 18 --
bool(true)
array(1) {
  [u"data"]=>
  unicode(7) "Nothing"
}

-- Iteration 19 --
bool(true)
array(1) {
  [u"data"]=>
  unicode(7) "Nothing"
}

-- Iteration 20 --
bool(true)
array(1) {
  [u"data"]=>
  unicode(12) "Hello World!"
}

-- Iteration 21 --
bool(true)
array(1) {
  [u"data"]=>
  object(classA)#2 (0) {
  }
}

-- Iteration 22 --
bool(true)
array(1) {
  [u"data"]=>
  NULL
}

-- Iteration 23 --
bool(true)
array(1) {
  [u"data"]=>
  NULL
}

-- Iteration 24 --
bool(true)
array(1) {
  [u"data"]=>
  int(0)
}
bool(true)
Done

