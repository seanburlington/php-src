--TEST--
function test: mysqli_get_host_info
--SKIPIF--
<?php
require_once('skipif.inc');
require_once('skipifemb.inc');
require_once('skipifconnectfailure.inc');
?>
--FILE--
<?php
	include "connect.inc";

	/*** test mysqli_connect 127.0.0.1 ***/
	$link = mysqli_connect($host, $user, $passwd, $db, $port, $socket);

	$hinfo = mysqli_get_host_info($link);

	var_dump(str_replace('/','', $hinfo));

	mysqli_close($link);
	print "done!";
?>
--EXPECTF--
%unicode|string%(%d) "%s via %s"
done!