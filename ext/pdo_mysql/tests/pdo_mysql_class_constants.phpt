--TEST--
PDO MySQL specific class constants
--SKIPIF--
<?php
require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'skipif.inc');
?>
--FILE--
<?php
	require_once(dirname(__FILE__) . DIRECTORY_SEPARATOR . 'mysql_pdo_test.inc');

	$expected = array(
		'MYSQL_ATTR_USE_BUFFERED_QUERY'		=> true,
		'MYSQL_ATTR_LOCAL_INFILE'					=> true,
		'MYSQL_ATTR_DIRECT_QUERY'					=> true,
		'MYSQL_ATTR_FOUND_ROWS'							=> true,
		'MYSQL_ATTR_IGNORE_SPACE'					=> true,
	);

	if (!MySQLPDOTest::isPDOMySQLnd()) {
		$expected['MYSQL_ATTR_MAX_BUFFER_SIZE'] 		= true;
		$expected['MYSQL_ATTR_INIT_COMMAND'] 				= true;
		$expected['MYSQL_ATTR_READ_DEFAULT_FILE'] 	= true;
		$expected['MYSQL_ATTR_READ_DEFAULT_GROUP'] 	= true;
		$expected['MYSQL_ATTR_COMPRESS']			= true;
	}

	/*
	TODO

		MYSQLI_OPT_CONNECT_TIMEOUT != PDO::ATTR_TIMEOUT  (integer)
    Sets the timeout value in seconds for communications with the database.
		^  Potential BUG, PDO::ATTR_TIMEOUT is used in pdo_mysql_handle_factory

		MYSQLI_SET_CHARSET_NAME -> DSN/charset=<charset_name>
		^ Undocumented and pitfall for ext/mysqli users

		Assorted mysqlnd settings missing
	*/
	$ref = new ReflectionClass('PDO');
	$constants = $ref->getConstants();
	$values = array();

	foreach ($constants as $name => $value)
		if (substr($name, 0, 11) == 'MYSQL_ATTR_') {
			if (!isset($values[$value]))
				$values[$value] = array($name);
			else
				$values[$value][] = $name;

			if (isset($expected[$name])) {
				unset($expected[$name]);
				unset($constants[$name]);
			}

		} else {
			unset($constants[$name]);
		}

	if (!empty($constants)) {
		printf("[001] Dumping list of unexpected constants\n");
		var_dump($constants);
	}

	if (!empty($expected)) {
		printf("[002] Dumping list of missing constants\n");
		var_dump($expected);
	}

	if (!empty($values)) {
		foreach ($values as $value => $constants) {
			if (count($constants) > 1) {
				printf("[003] Several constants share the same value '%s'\n", $value);
				var_dump($constants);
			}
		}
	}

	print "done!";
--EXPECT--
done!
