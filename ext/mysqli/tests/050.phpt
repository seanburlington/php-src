--TEST--
non freed statement test
--SKIPIF--
<?php 
require_once('skipif.inc'); 
require_once('skipifconnectfailure.inc');
?>
--FILE--
<?php
	include "connect.inc";

	/************************
	 * non freed stamement
	 ************************/
	$link = mysqli_connect($host, $user, $passwd, $db, $port, $socket);

	$stmt = mysqli_prepare($link, "SELECT CURRENT_USER()");
	mysqli_execute($stmt);

	mysqli_close($link);
	printf("Ok\n");
?>
--EXPECT--
Ok
