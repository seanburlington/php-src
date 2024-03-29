--TEST--
Test open_basedir configuration
--INI--
open_basedir=.
--FILE--
<?php
require_once "open_basedir.inc";
$initdir = getcwd();
test_open_basedir_before("scandir");
test_open_basedir_error("scandir");     

var_dump(scandir($initdir."/test/ok/"));
var_dump(scandir($initdir."/test/ok"));
var_dump(scandir($initdir."/test/ok/../ok"));

test_open_basedir_after("scandir");?>
--CLEAN--
<?php
require_once "open_basedir.inc";
delete_directories();
?>
--EXPECTF--
*** Testing open_basedir configuration [scandir] ***
bool(true)
bool(true)
bool(true)
bool(true)
bool(true)

Warning: scandir(): open_basedir restriction in effect. File(../bad) is not within the allowed path(s): (.) in %s on line %d

Warning: scandir(../bad): failed to open dir: %s in %s on line %d

Warning: scandir(): (errno 1): %s in %s on line %d
bool(false)

Warning: scandir(): open_basedir restriction in effect. File(../bad/bad.txt) is not within the allowed path(s): (.) in %s on line %d

Warning: scandir(../bad/bad.txt): failed to open dir: %s in %s on line %d

Warning: scandir(): (errno 1): %s in %s on line %d
bool(false)

Warning: scandir(): open_basedir restriction in effect. File(..) is not within the allowed path(s): (.) in %s on line %d

Warning: scandir(..): failed to open dir: %s in %s on line %d

Warning: scandir(): (errno 1): %s in %s on line %d
bool(false)

Warning: scandir(): open_basedir restriction in effect. File(../) is not within the allowed path(s): (.) in %s on line %d

Warning: scandir(../): failed to open dir: %s in %s on line %d

Warning: scandir(): (errno 1): %s in %s on line %d
bool(false)

Warning: scandir(): open_basedir restriction in effect. File(/) is not within the allowed path(s): (.) in %s on line %d

Warning: scandir(/): failed to open dir: %s in %s on line %d

Warning: scandir(): (errno 1): %s in %s on line %d
bool(false)

Warning: scandir(): open_basedir restriction in effect. File(../bad/.) is not within the allowed path(s): (.) in %s on line %d

Warning: scandir(../bad/.): failed to open dir: %s in %s on line %d

Warning: scandir(): (errno 1): %s in %s on line %d
bool(false)

Warning: scandir(): open_basedir restriction in effect. File(%s/test/bad/bad.txt) is not within the allowed path(s): (.) in %s on line %d

Warning: scandir(%s/test/bad/bad.txt): failed to open dir: %s in %s on line %d

Warning: scandir(): (errno 1): %s in %s on line %d
bool(false)

Warning: scandir(): open_basedir restriction in effect. File(%s/test/bad/../bad/bad.txt) is not within the allowed path(s): (.) in %s on line %d

Warning: scandir(%s/test/bad/../bad/bad.txt): failed to open dir: %s in %s on line %d

Warning: scandir(): (errno 1): %s in %s on line %d
bool(false)
array(3) {
  [0]=>
  unicode(1) "."
  [1]=>
  unicode(2) ".."
  [2]=>
  unicode(6) "ok.txt"
}
array(3) {
  [0]=>
  unicode(1) "."
  [1]=>
  unicode(2) ".."
  [2]=>
  unicode(6) "ok.txt"
}
array(3) {
  [0]=>
  unicode(1) "."
  [1]=>
  unicode(2) ".."
  [2]=>
  unicode(6) "ok.txt"
}
*** Finished testing open_basedir configuration [scandir] ***

