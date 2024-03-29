--TEST--
sqlite-oo: unbuffered
--INI--
sqlite.assoc_case=0
--SKIPIF--
<?php # vim:ft=php
if (!extension_loaded("sqlite")) print "skip"; 
?>
--FILE--
<?php 
include "blankdb_oo.inc";

$data = array(
	"one",
	"two",
	"three"
	);

$db->query("CREATE TABLE strings(a VARCHAR)");

foreach ($data as $str) {
	$db->query("INSERT INTO strings VALUES('$str')");
}

echo "====FOREACH====\n";
$r = $db->unbufferedQuery("SELECT a from strings", SQLITE_NUM);
foreach($r as $idx => $row) {
	var_dump($row[0]);
	var_dump($row[0]);
}
echo "====FOR====\n";
$r = $db->unbufferedQuery("SELECT a from strings", SQLITE_NUM);
for(;$r->valid(); $r->next()) {
	$v = $r->column(0);
	var_dump($v);
	$c = $r->column(0);
	var_dump(is_null($c) || $c==$v);
}
echo "===DONE===\n";
?>
--EXPECT--
====FOREACH====
unicode(3) "one"
unicode(3) "one"
unicode(3) "two"
unicode(3) "two"
unicode(5) "three"
unicode(5) "three"
====FOR====
unicode(3) "one"
bool(true)
unicode(3) "two"
bool(true)
unicode(5) "three"
bool(true)
===DONE===
