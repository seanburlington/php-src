--TEST--
mysqli_get_metadata
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

	mysqli_query($link,"DROP TABLE IF EXISTS test_store_result");
	mysqli_query($link,"CREATE TABLE test_store_result (a int)");

	mysqli_query($link, "INSERT INTO test_store_result VALUES (1),(2),(3)");

	$stmt = mysqli_prepare($link, "SELECT * FROM test_store_result");
	mysqli_execute($stmt);

	/* this should produce an out of sync error */
	if ($result = mysqli_query($link, "SELECT * FROM test_store_result")) {
		mysqli_free_result($result);
		printf ("Query ok\n");
	}
	mysqli_stmt_close($stmt);

	/* now we should try mysqli_stmt_reset() */
	$stmt = mysqli_prepare($link, "SELECT * FROM test_store_result");
	var_dump(mysqli_execute($stmt));
	var_dump(mysqli_stmt_reset($stmt));

	var_dump($stmt = mysqli_prepare($link, "SELECT * FROM test_store_result"));
	if ($IS_MYSQLND && $stmt->affected_rows !== -1)
			printf("[001] Expecting -1, got %d\n", $stmt->affected_rows);

	var_dump(mysqli_execute($stmt));
	var_dump($stmt = @mysqli_prepare($link, "SELECT * FROM test_store_result"), mysqli_error($link));
	var_dump(mysqli_stmt_reset($stmt));

	$stmt = mysqli_prepare($link, "SELECT * FROM test_store_result");
	mysqli_execute($stmt);
	$result1 = mysqli_get_metadata($stmt);
	mysqli_stmt_store_result($stmt);

	printf ("Rows: %d\n", mysqli_stmt_affected_rows($stmt));

	/* this should show an error, cause results are not buffered */
	if ($result = mysqli_query($link, "SELECT * FROM test_store_result")) {
		$row = mysqli_fetch_row($result);
		mysqli_free_result($result);
	}

	var_dump($row);

	mysqli_free_result($result1);
	mysqli_stmt_close($stmt);
	mysqli_close($link);
	echo "done!";
?>
--EXPECTF--
bool(true)
bool(true)
object(mysqli_stmt)#%d (%d) {
  [%u|b%"affected_rows"]=>
  int(%i)
  [%u|b%"insert_id"]=>
  int(0)
  [%u|b%"num_rows"]=>
  int(0)
  [%u|b%"param_count"]=>
  int(0)
  [%u|b%"field_count"]=>
  int(1)
  [%u|b%"errno"]=>
  int(0)
  [%u|b%"error"]=>
  %unicode|string%(0) ""
  [%u|b%"sqlstate"]=>
  %unicode|string%(5) "00000"
  [%u|b%"id"]=>
  int(3)
}
bool(true)
bool(false)
%unicode|string%(0) ""

Warning: mysqli_stmt_reset() expects parameter 1 to be mysqli_stmt, boolean given in %s on line %d
NULL
Rows: 3
array(1) {
  [0]=>
  %unicode|string%(1) "1"
}
done!