--TEST--
Explode/memnstr bug
--INI--
error_reporting=2047
memory_limit=256M
--FILE--
<?php
$res = explode(str_repeat(b"A",145999999),1);
var_dump($res);
?>
--EXPECTF--
array(1) {
  [0]=>
  string(1) "1"
}
