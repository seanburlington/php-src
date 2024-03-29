--TEST--
Test session_start() function : variation
--SKIPIF--
<?php include('skipif.inc'); ?>
--FILE--
<?php

ob_start();

/* 
 * Prototype : bool session_start(void)
 * Description : Initialize session data
 * Source code : ext/session/session.c 
 */

echo "*** Testing session_start() : variation ***\n";

session_start();

$_SESSION['colour'] = 'green';
$_SESSION['animal'] = 'cat';
$_SESSION['person'] = 'julia';
$_SESSION['age'] = 6;

var_dump($_SESSION);
var_dump(session_write_close());
var_dump($_SESSION);
session_start();
var_dump($_SESSION);

session_destroy();
echo "Done";
ob_end_flush();
?>
--EXPECTF--
*** Testing session_start() : variation ***
array(4) {
  [u"colour"]=>
  unicode(5) "green"
  [u"animal"]=>
  unicode(3) "cat"
  [u"person"]=>
  unicode(5) "julia"
  [u"age"]=>
  int(6)
}
NULL
array(4) {
  [u"colour"]=>
  unicode(5) "green"
  [u"animal"]=>
  unicode(3) "cat"
  [u"person"]=>
  unicode(5) "julia"
  [u"age"]=>
  int(6)
}
array(4) {
  [u"colour"]=>
  unicode(5) "green"
  [u"animal"]=>
  unicode(3) "cat"
  [u"person"]=>
  unicode(5) "julia"
  [u"age"]=>
  int(6)
}
Done

