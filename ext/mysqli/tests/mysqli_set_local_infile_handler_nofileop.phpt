--TEST--
mysqli_set_local_infile_handler() - do not use the file pointer
--SKIPIF--
<?php
require_once('skipif.inc');
require_once('skipifemb.inc');
require_once('skipifconnectfailure.inc');

if (!function_exists('mysqli_set_local_infile_handler'))
	die("skip - function not available.");

require_once('connect.inc');
if (!$TEST_EXPERIMENTAL)
	die("skip - experimental (= unsupported) feature");

if (!$link = mysqli_connect($host, $user, $passwb, $db, $port, $socket))
	die("skip Cannot connect to MySQL");

if (!$res = mysqli_query($link, 'SHOW VARIABLES LIKE "local_infile"')) {
	mysqli_close($link);
	die("skip Cannot check if Server variable 'local_infile' is set to 'ON'");
}

$row = mysqli_fetch_assoc($res);
mysqli_free_result($res);
mysqli_close($link);

if ('ON' != $row['Value'])
	die(sprintf("skip Server variable 'local_infile' seems not set to 'ON', found '%s'",
		$row['Value']));
?>
--INI--
mysqli.allow_local_infile=1
--FILE--
<?php
	require_once('connect.inc');
	require_once('local_infile_tools.inc');
	require_once('table.inc');

	function callback_nofileop($fp, &$buffer, $buflen, &$error) {
		static $invocation = 0;

		printf("Callback: %d\n", $invocation++);
		flush();

		$buffer = "1;'a';\n";
		if ($invocation > 10)
			return 0;

		return strlen($buffer);
	}

	$file = create_standard_csv(1);
	$expected = array(array('id' => 1, 'label' => 'a'));
	try_handler(20, $link, $file, 'callback_nofileop', $expected);

	mysqli_close($link);
	print "done!";
?>
--EXPECTF--
Callback set to 'callback_nofileop'
Callback: 0
Callback: 1
Callback: 2
Callback: 3
Callback: 4
Callback: 5
Callback: 6
Callback: 7
Callback: 8
Callback: 9
Callback: 10
done!