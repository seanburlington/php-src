--TEST--
Test session_encode() function : error functionality
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php

ob_start();

/* 
 * Prototype : string session_encode(void)
 * Description : Encodes the current session data as a string
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_encode() : error functionality ***\n";

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

$iterator = 1;
foreach($inputs as $input) {
    echo "\n-- Iteration $iterator --\n";
    var_dump(session_start());
    $_SESSION[$input] = "Hello World!";
    var_dump(session_encode());
    var_dump(session_destroy());
    $iterator++;
};

fclose($fp);
echo "Done";
ob_end_flush();
?>
--EXPECTF--
*** Testing session_encode() : error functionality ***

-- Iteration 1 --
bool(true)
bool(false)
bool(true)

-- Iteration 2 --
bool(true)
bool(false)
bool(true)

-- Iteration 3 --
bool(true)
bool(false)
bool(true)

-- Iteration 4 --
bool(true)
bool(false)
bool(true)

-- Iteration 5 --
bool(true)
bool(false)
bool(true)

-- Iteration 6 --
bool(true)
bool(false)
bool(true)

-- Iteration 7 --
bool(true)
bool(false)
bool(true)

-- Iteration 8 --
bool(true)
bool(false)
bool(true)

-- Iteration 9 --
bool(true)
bool(false)
bool(true)

-- Iteration 10 --
bool(true)
unicode(21) "|U:12:"Hello World!";"
bool(true)

-- Iteration 11 --
bool(true)
unicode(21) "|U:12:"Hello World!";"
bool(true)

-- Iteration 12 --
bool(true)
bool(false)
bool(true)

-- Iteration 13 --
bool(true)
bool(false)
bool(true)

-- Iteration 14 --
bool(true)
bool(false)
bool(true)

-- Iteration 15 --
bool(true)
bool(false)
bool(true)

-- Iteration 16 --
bool(true)
unicode(21) "|U:12:"Hello World!";"
bool(true)

-- Iteration 17 --
bool(true)
unicode(21) "|U:12:"Hello World!";"
bool(true)

-- Iteration 18 --
bool(true)
unicode(28) "Nothing|U:12:"Hello World!";"
bool(true)

-- Iteration 19 --
bool(true)
unicode(28) "Nothing|U:12:"Hello World!";"
bool(true)

-- Iteration 20 --
bool(true)
unicode(33) "Hello World!|U:12:"Hello World!";"
bool(true)

-- Iteration 21 --
bool(true)

Warning: Illegal offset type in %s on line %d
bool(false)
bool(true)

-- Iteration 22 --
bool(true)
unicode(21) "|U:12:"Hello World!";"
bool(true)

-- Iteration 23 --
bool(true)
unicode(21) "|U:12:"Hello World!";"
bool(true)

-- Iteration 24 --
bool(true)

Strict Standards: Resource ID#%d used as offset, casting to integer (%d) in %s on line %d
bool(false)
bool(true)
Done

