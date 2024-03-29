--TEST--
mysqli_fetch_array()
--SKIPIF--
<?php
require_once('skipif.inc');
require_once('skipifemb.inc');
require_once('skipifconnectfailure.inc');
?>
--FILE--
<?php
	include "connect.inc";
	require('table.inc');

	if (!$res = mysqli_query($link, "SELECT * FROM test ORDER BY id LIMIT 5")) {
		printf("[001] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}
	print "[002]\n";
	var_dump(mysqli_fetch_array($res, MYSQLI_ASSOC));
	mysqli_free_result($res);

	if (!$res = mysqli_query($link, "SELECT id, label FROM test ORDER BY id LIMIT 5")) {
		printf("[003] [%d] %s\n", mysqli_errno($link), mysqli_error($link));
	}
	print "[004]\n";
	var_dump(mysqli_fetch_array($res, MYSQLI_ASSOC));
	mysqli_free_result($res);

	mysqli_close($link);
	print "done!";
?>
--EXPECTF--
[002]
array(2) {
  [%u|b%"id"]=>
  %unicode|string%(1) "1"
  [%u|b%"label"]=>
  %unicode|string%(1) "a"
}
[004]
array(2) {
  [%u|b%"id"]=>
  %unicode|string%(1) "1"
  [%u|b%"label"]=>
  %unicode|string%(1) "a"
}
done!