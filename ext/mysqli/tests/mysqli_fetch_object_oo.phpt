--TEST--
mysqli_fetch_object()
--SKIPIF--
<?php
require_once('skipif.inc');
require_once('skipifemb.inc');
require_once('skipifconnectfailure.inc');
?>
--FILE--
<?php
	include "connect.inc";

	$tmp    = NULL;
	$link   = NULL;

	$mysqli = new mysqli();
	$res = @new mysqli_result($mysqli);
	if (!is_null($tmp = @$res->fetch_object()))
		printf("[001] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	require('table.inc');
	if (!$mysqli = new mysqli($host, $user, $passwd, $db, $port, $socket))
		printf("[002] Cannot connect to the server using host=%s, user=%s, passwd=***, dbname=%s, port=%s, socket=%s\n",
			$host, $user, $db, $port, $socket);

	if (!$res = $mysqli->query("SELECT id AS ID, label FROM test AS TEST ORDER BY id LIMIT 5")) {
		printf("[003] [%d] %s\n", $mysqli->errno, $mysqli->error);
	}

	if (!is_null($tmp = @$res->fetch_object($link)))
		printf("[004] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	if (!is_null($tmp = @$res->fetch_object($link, $link)))
		printf("[005] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	if (!is_null($tmp = @$res->fetch_object($link, $link, $link)))
		printf("[006] Expecting NULL, got %s/%s\n", gettype($tmp), $tmp);

	$obj = mysqli_fetch_object($res);
	if (($obj->ID !== "1") || ($obj->label !== "a") || (get_class($obj) != 'stdClass')) {
		printf("[007] Object seems wrong. [%d] %s\n", $mysqli->errno, $mysqli->error);
		var_dump($obj);
	}

	class mysqli_fetch_object_test {

		public $a = null;
		public $b = null;

		public function toString() {
			var_dump($this);
		}
	}

	$obj = $res->fetch_object('mysqli_fetch_object_test');
	if (($obj->ID !== "2") || ($obj->label !== "b") || ($obj->a !== NULL) || ($obj->b !== NULL) || (get_class($obj) != 'mysqli_fetch_object_test')) {
		printf("[008] Object seems wrong. [%d] %s\n", $mysqli->errno, $mysqli->error);
		var_dump($obj);
	}

	class mysqli_fetch_object_construct extends mysqli_fetch_object_test {

		public function __construct($a, $b) {
			$this->a = $a;
			$this->b = $b;
		}

	}

	$obj = $res->fetch_object('mysqli_fetch_object_construct', null);

	if (($obj->ID !== "3") || ($obj->label !== "c") || ($obj->a !== NULL) || ($obj->b !== NULL) || (get_class($obj) != 'mysqli_fetch_object_construct')) {
		printf("[009] Object seems wrong. [%d] %s\n", $mysqli->errno, $mysqli->error);
		var_dump($obj);
	}

	$obj = $res->fetch_object('mysqli_fetch_object_construct', array('a'));
	if (($obj->ID !== "4") || ($obj->label !== "d") || ($obj->a !== 'a') || ($obj->b !== NULL) || (get_class($obj) != 'mysqli_fetch_object_construct')) {
		printf("[010] Object seems wrong. [%d] %s\n", $mysqli->errno, $mysqli->error);
		var_dump($obj);
	}

	$obj = $res->fetch_object('mysqli_fetch_object_construct', array('a', 'b'));
	if (($obj->ID !== "5") || ($obj->label !== "e") || ($obj->a !== 'a') || ($obj->b !== 'b') || (get_class($obj) != 'mysqli_fetch_object_construct')) {
		printf("[011] Object seems wrong. [%d] %s\n", $mysqli->errno, $mysqli->error);
		var_dump($obj);
	}

	var_dump($res->fetch_object('mysqli_fetch_object_construct', array('a', 'b', 'c')));
	var_dump(mysqli_fetch_object($res));

	mysqli_free_result($res);

	if (!$res = mysqli_query($link, "SELECT id AS ID, label FROM test AS TEST")) {
		printf("[012] [%d] %s\n", $mysqli->errno, $mysqli->error);
	}

	mysqli_free_result($res);

	var_dump(mysqli_fetch_object($res));

	// Fatal error, script execution will end
	var_dump($res->fetch_object('this_class_does_not_exist'));

	$mysqli->close();
	print "done!";
?>
--EXPECTF--
Warning: Missing argument 1 for mysqli_fetch_object_construct::__construct() in %s on line %d

Warning: Missing argument 2 for mysqli_fetch_object_construct::__construct() in %s on line %d

Notice: Undefined variable: a in %s on line %d

Notice: Undefined variable: b in %s on line %d

Warning: Missing argument 2 for mysqli_fetch_object_construct::__construct() in %s on line %d

Notice: Undefined variable: b in %s on line %d
NULL
NULL

Warning: mysqli_fetch_object(): Couldn't fetch mysqli_result in %s on line %d
NULL

Fatal error: Class 'this_class_does_not_exist' not found in %s on line %d