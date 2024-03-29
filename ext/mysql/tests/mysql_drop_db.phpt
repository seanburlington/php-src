--TEST--
mysql_drop_db()
--SKIPIF--
<?php
require_once('skipif.inc');
if (!function_exists('mysql_drop_db'))
	die("Skip function is deprecated and not available");
?>
--FILE--
<?php
include_once "connect.inc";

$tmp    = NULL;
$link   = NULL;

// NOTE: again this test does not test all of the behaviour of the function

if (NULL !== ($tmp = mysql_drop_db()))
	printf("[001] Expecting NULL/NULL, got %s/%s\n", gettype($tmp), $tmp);

require('table.inc');
if (!mysql_query('DROP DATABASE IF EXISTS mysqldropdb'))
	printf("[004] [%d] %s\n", mysql_errno($link), mysql_error($link));

if (!mysql_query('CREATE DATABASE mysqldropdb'))
	die(sprintf("[005] Skipping, can't create test database. [%d] %s\n", mysql_errno($link), mysql_error($link)));

if (true !== ($tmp = mysql_drop_db('mysqldropdb', $link)))
	printf("[006] Can't drop, got %s/%s. [%d] %s\n",
		gettype($tmp), $tmp,
		mysql_errno($link), mysql_error($link));

if (false !== ($tmp = mysql_drop_db('mysqldropdb', $link)))
	printf("[007] Expecting boolean/false, got %s/%s. [%d] %s\n",
		gettype($tmp), $tmp,
		mysql_errno($link), mysql_error($link));

mysql_close($link);

print "done!\n";
?>
--EXPECTF--
done!