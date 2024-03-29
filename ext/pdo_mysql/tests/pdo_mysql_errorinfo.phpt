--TEST--
MySQL PDO->errorInfo()
--SKIPIF--
<?php
require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'skipif.inc');
require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc');
MySQLPDOTest::skip();
$db = MySQLPDOTest::factory();
?>
--FILE--
<?php
	require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc');
	$db = MySQLPDOTest::factory();
	MySQLPDOTest::createTestTable($db);

	function check_error($offset, &$obj, $expected = '00000') {

		$info = $obj->errorInfo();
		if (count($info) != 3)
			printf("[%03d] Info should have three fields, got %s\n",
				$offset, var_export($info, true));

		$code = $info[0];
		if (($code != $expected) && (($expected != '00000') && ($code != ''))) {
			printf("[%03d] Expecting error code '%s' got code '%s'\n",
				$offset, $expected, $code);
		}

		if ($expected != '00000') {
			if (!isset($info[1]) || $info[1] == '')
				printf("[%03d] Driver-specific error code not set\n", $offset);
			if (!isset($info[2]) || $info[2] == '')
				printf("[%03d] Driver-specific error message.not set\n", $offset);
		}

	}

	function pdo_mysql_errorinfo($db, $offset) {

		try {

			/*
			If you create a PDOStatement object through PDO->prepare()
			or PDO->query() and invoke an error on the statement handle,
			PDO->errorCode() will not reflect that error. You must call
			PDOStatement->errorCode() to return the error code for an
			operation performed on a particular statement handle.
			*/
			$code = $db->errorCode();
			check_error($offset + 2, $db);

			$stmt = $db->query('SELECT id, label FROM test');
			$stmt2 = &$stmt;
			check_error($offset + 3, $db);
			check_error($offset + 4, $stmt);

			$db->exec('DROP TABLE IF EXISTS test');
			@$stmt->execute();
			check_error($offset + 5, $db);
			check_error($offset + 6, $stmt, '42S02');
			check_error($offset + 7, $stmt2, '42S02');

			@$stmt = $db->query('SELECT id, label FROM unknown');
			check_error($offset + 8, $db, '42S02');

			MySQLPDOTest::createTestTable($db);
			$stmt = $db->query('SELECT id, label FROM test');
			check_error($offset + 9, $db);
			check_error($offset + 10, $stmt);

			$db2 = &$db;
			$db->exec('DROP TABLE IF EXISTS unknown');
			@$db->query('SELECT id, label FROM unknown');
			check_error($offset + 11, $db, '42S02');
			check_error($offset + 12, $db2, '42S02');
			check_error($offset + 13, $stmt);
			check_error($offset + 14, $stmt2);

			// lets hope this is an invalid attribute code
			$invalid_attr = -1 * PHP_INT_MAX + 3;
			$tmp = @$db->getAttribute($invalid_attr);
			check_error($offset + 15, $db, 'IM001');
			check_error($offset + 16, $db2, 'IM001');
			check_error($offset + 17, $stmt);
			check_error($offset + 18, $stmt2);

		} catch (PDOException $e) {
			printf("[%03d] %s [%s] %s\n",
				$offset + 19, $e->getMessage(), $db->errorCode(), implode(' ', $db->errorInfo()));
		}
	}

	$db->setAttribute(PDO::ATTR_EMULATE_PREPARES, 1);
	printf("Emulated Prepared Statements...\n");
	pdo_mysql_errorinfo($db, 0);

	$db->setAttribute(PDO::ATTR_EMULATE_PREPARES, 0);
	printf("Native Prepared Statements...\n");
	pdo_mysql_errorinfo($db, 20);

	$db->exec('DROP TABLE IF EXISTS test');
	print "done!";
--EXPECTF--
Emulated Prepared Statements...
[002] Info should have three fields, got array (
  0 => '00000',
)
[003] Info should have three fields, got array (
  0 => '00000',
)
[004] Info should have three fields, got array (
  0 => '00000',
)
[005] Info should have three fields, got array (
  0 => '00000',
)
[009] Info should have three fields, got array (
  0 => '00000',
)
[010] Info should have three fields, got array (
  0 => '00000',
)
[013] Info should have three fields, got array (
  0 => '00000',
)
[014] Info should have three fields, got array (
  0 => '00000',
)
[015] Info should have three fields, got array (
  0 => 'IM001',
)
[015] Driver-specific error code not set
[015] Driver-specific error message.not set
[016] Info should have three fields, got array (
  0 => 'IM001',
)
[016] Driver-specific error code not set
[016] Driver-specific error message.not set
[017] Info should have three fields, got array (
  0 => '00000',
)
[018] Info should have three fields, got array (
  0 => '00000',
)
Native Prepared Statements...
[022] Info should have three fields, got array (
  0 => '00000',
)
[023] Info should have three fields, got array (
  0 => '00000',
)
[024] Info should have three fields, got array (
  0 => '00000',
)
[025] Info should have three fields, got array (
  0 => '00000',
)
[030] Info should have three fields, got array (
  0 => '00000',
)
[033] Info should have three fields, got array (
  0 => '00000',
)
[034] Info should have three fields, got array (
  0 => '00000',
)
[037] Info should have three fields, got array (
  0 => '00000',
)
[038] Info should have three fields, got array (
  0 => '00000',
)
done!