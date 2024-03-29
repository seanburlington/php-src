--TEST--
mysqli fetch system variables
--SKIPIF--
<?php 
require_once('skipif.inc'); 
require_once('skipifconnectfailure.inc');
?>
--FILE--
<?php
	include "connect.inc";

	/*** test mysqli_connect 127.0.0.1 ***/
	$link = mysqli_connect($host, $user, $passwd, $db, $port, $socket);

	if (!mysqli_query($link, "SET AUTOCOMMIT=0"))
		printf("[001] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	if (!$stmt = mysqli_prepare($link, "SELECT @@autocommit"))
		printf("[001] [%d] %s\n", mysqli_errno($link), mysqli_error($link));

	mysqli_bind_result($stmt, $c0);
	mysqli_execute($stmt);

	mysqli_fetch($stmt);

	var_dump($c0);

	mysqli_close($link);
	print "done!";
?>
--EXPECT--
int(0)
done!