--TEST--
mysqli bind_param/bind_result date
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

	mysqli_select_db($link, $db);
	mysqli_query($link, "SET sql_mode=''");

	mysqli_query($link,"DROP TABLE IF EXISTS test_bind_result");

	$rc = @mysqli_query($link,"CREATE TABLE test_bind_result(
		c1 date,
		c2 time,
		c3 timestamp(14),
		c4 year,
		c5 datetime,
		c6 timestamp(4),
		c7 timestamp(6))");

	if (!$rc)
		$rc = mysqli_query($link,"CREATE TABLE test_bind_result(
		c1 date,
		c2 time,
		c3 timestamp,
		c4 year,
		c5 datetime,
		c6 timestamp,
		c7 timestamp)");

	$stmt = mysqli_prepare($link, "INSERT INTO test_bind_result VALUES (?,?,?,?,?,?,?)");
	mysqli_bind_param($stmt, "sssssss", $d1, $d2, $d3, $d4, $d5, $d6, $d7);

	$d1 = "2002-01-02";
	$d2 = "12:49:00";
	$d3 = "2002-01-02 17:46:59";
	$d4 = "2010";
	$d5 = "2010-07-10";
	$d6 = "2020";
	$d7 = "1999-12-29";

	mysqli_execute($stmt);
	mysqli_stmt_close($stmt);

	$stmt = mysqli_prepare($link, "SELECT c1, c2, c3, c4, c5, c6, c7 FROM test_bind_result");

	mysqli_bind_result($stmt,$c1, $c2, $c3, $c4, $c5, $c6, $c7);

	mysqli_execute($stmt);
	mysqli_fetch($stmt);

	$test = array($c1,$c2,$c3,$c4,$c5,$c6,$c7);

	var_dump($test);

	mysqli_stmt_close($stmt);
	mysqli_query($link, "DROP TABLE IF EXISTS test_bind_result");
	mysqli_close($link);

	print "done!";
?>
--EXPECTF--
array(7) {
  [0]=>
  %s(10) "2002-01-02"
  [1]=>
  %s(8) "12:49:00"
  [2]=>
  %s(19) "2002-01-02 17:46:59"
  [3]=>
  int(2010)
  [4]=>
  %s(19) "2010-07-10 00:00:00"
  [5]=>
  %s(19) "0000-00-00 00:00:00"
  [6]=>
  %s(19) "1999-12-29 00:00:00"
}
done!