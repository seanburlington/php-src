--TEST--
Testing rtrim() function
--FILE--
<?php

/*  Testing for Error conditions  */

/*  Invalid Number of Arguments */

 echo "\n *** Output for Error Conditions ***\n";
 rtrim();
 rtrim("", " ", 1);

/* Testing the Normal behaviour of rtrim() function */

 echo "\n *** Output for Normal Behaviour ***\n";
 var_dump ( rtrim("rtrim test   \t\0 ") );                       /* without second Argument */
 var_dump ( rtrim("rtrim test   " , "") );                       /* no characters in second Argument */
 var_dump ( rtrim("rtrim test        ", NULL) );                 /* with NULL as second Argument */
 var_dump ( rtrim("rtrim test        ", true) );                 /* with boolean value as second Argument */
 var_dump ( rtrim("rtrim test        ", " ") );                  /* with single space as second Argument */
 var_dump ( rtrim("rtrim test \t\n\r\0\x0B", "\t\n\r\0\x0B") );  /* with multiple escape sequences as second Argument */
 var_dump ( rtrim("rtrim testABCXYZ", "A..Z") );                 /* with characters range as second Argument */
 var_dump ( rtrim("rtrim test0123456789", "0..9") );             /* with numbers range as second Argument */
 var_dump ( rtrim("rtrim test$#@", "#@$") );                     /* with some special characters as second Argument */


/* Use of class and objects */
echo "\n*** Checking with OBJECTS ***\n";
class string1 {
  public function __toString() {
    return "Object";
  }
}
$obj = new string1;
var_dump( rtrim($obj, "tc") );

/* String with embedded NULL */
echo "\n*** String with embedded NULL ***\n";
var_dump( rtrim("234\x0005678\x0000efgh\xijkl\x0n1", "\x0n1") );

/* heredoc string */
$str = <<<EOD
us
ing heredoc string
EOD;

echo "\n *** Using heredoc string ***\n";
var_dump( rtrim($str, "ing") );

echo "Done\n";
?>
--EXPECTF--
*** Output for Error Conditions ***

Warning: rtrim() expects at least 1 parameter, 0 given in %s on line %d

Warning: rtrim() expects at most 2 parameters, 3 given in %s on line %d

 *** Output for Normal Behaviour ***
unicode(10) "rtrim test"
unicode(13) "rtrim test   "
unicode(18) "rtrim test        "
unicode(18) "rtrim test        "
unicode(10) "rtrim test"
unicode(11) "rtrim test "
unicode(10) "rtrim test"
unicode(10) "rtrim test"
unicode(10) "rtrim test"

*** Checking with OBJECTS ***
unicode(4) "Obje"

*** String with embedded NULL ***
unicode(22) "234 05678 00efgh\xijkl"

 *** Using heredoc string ***
unicode(18) "us
ing heredoc str"
Done
