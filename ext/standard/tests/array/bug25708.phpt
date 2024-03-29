--TEST--
Bug #25708 (extract($GLOBALS, EXTR_REFS) mangles $GLOBALS)
--FILE--
<?php
function foo($ref, $alt) {
	unset($GLOBALS['a']);
	unset($GLOBALS['b']);
	$GLOBALS['a'] = 1;
	$GLOBALS['b'] = 2;

	$org_a = $GLOBALS['a'];
	$org_b = $GLOBALS['b'];

	if ($ref) {
		global $a, $b;
	} else {
		/* zval temp_var(NULL); // refcount = 1
		 * a = temp_var[x] // refcount = 2
		 */
		$a = NULL;
		$b = NULL;
	}

	debug_zval_dump($a, $b, $GLOBALS['a'], $GLOBALS['b']);
	echo "--\n";
	if ($alt) {
		$a = &$GLOBALS['a'];
		$b = &$GLOBALS['b'];
	} else {
		extract($GLOBALS, EXTR_REFS);
	}
	debug_zval_dump($a, $b, $GLOBALS['a'], $GLOBALS['b']);
	echo "--\n";
	$a = &$GLOBALS['a'];
	$b = &$GLOBALS['b'];
	debug_zval_dump($a, $b, $GLOBALS['a'], $GLOBALS['b']);
	echo "--\n";
	$GLOBALS['b'] = 3;
	debug_zval_dump($a, $b, $GLOBALS['a'], $GLOBALS['b']);
	echo "--\n";
	$a = 4;
	debug_zval_dump($a, $b, $GLOBALS['a'], $GLOBALS['b']);
	echo "--\n";
	$c = $b;
	debug_zval_dump($b, $GLOBALS['b'], $c);
	echo "--\n";
	$b = 'x';
	debug_zval_dump($a, $b, $GLOBALS['a'], $GLOBALS['b'], $c);
	echo "--\n";
	debug_zval_dump($org_a, $org_b);
	echo "----";
	if ($ref) echo 'r';
	if ($alt) echo 'a';
	echo "\n";
}

$a = 'ok';
$b = 'ok';
$_a = $a;
$_b = $b;

foo(false, true);
foo(true, true);
foo(false, false);
foo(true, false);

debug_zval_dump($_a, $_b);
?>
--EXPECT--
NULL refcount(2)
NULL refcount(2)
long(1) refcount(3)
long(2) refcount(3)
--
long(1) refcount(1)
long(2) refcount(1)
long(1) refcount(1)
long(2) refcount(1)
--
long(1) refcount(1)
long(2) refcount(1)
long(1) refcount(1)
long(2) refcount(1)
--
long(1) refcount(1)
long(3) refcount(1)
long(1) refcount(1)
long(3) refcount(1)
--
long(4) refcount(1)
long(3) refcount(1)
long(4) refcount(1)
long(3) refcount(1)
--
long(3) refcount(1)
long(3) refcount(1)
long(3) refcount(2)
--
long(4) refcount(1)
unicode(1) "x" { 0078 } refcount(1)
long(4) refcount(1)
unicode(1) "x" { 0078 } refcount(1)
long(3) refcount(2)
--
long(1) refcount(2)
long(2) refcount(2)
----a
long(1) refcount(1)
long(2) refcount(1)
long(1) refcount(1)
long(2) refcount(1)
--
long(1) refcount(1)
long(2) refcount(1)
long(1) refcount(1)
long(2) refcount(1)
--
long(1) refcount(1)
long(2) refcount(1)
long(1) refcount(1)
long(2) refcount(1)
--
long(1) refcount(1)
long(3) refcount(1)
long(1) refcount(1)
long(3) refcount(1)
--
long(4) refcount(1)
long(3) refcount(1)
long(4) refcount(1)
long(3) refcount(1)
--
long(3) refcount(1)
long(3) refcount(1)
long(3) refcount(2)
--
long(4) refcount(1)
unicode(1) "x" { 0078 } refcount(1)
long(4) refcount(1)
unicode(1) "x" { 0078 } refcount(1)
long(3) refcount(2)
--
long(1) refcount(2)
long(2) refcount(2)
----ra
NULL refcount(2)
NULL refcount(2)
long(1) refcount(3)
long(2) refcount(3)
--
long(1) refcount(1)
long(2) refcount(1)
long(1) refcount(1)
long(2) refcount(1)
--
long(1) refcount(1)
long(2) refcount(1)
long(1) refcount(1)
long(2) refcount(1)
--
long(1) refcount(1)
long(3) refcount(1)
long(1) refcount(1)
long(3) refcount(1)
--
long(4) refcount(1)
long(3) refcount(1)
long(4) refcount(1)
long(3) refcount(1)
--
long(3) refcount(1)
long(3) refcount(1)
long(3) refcount(2)
--
long(4) refcount(1)
unicode(1) "x" { 0078 } refcount(1)
long(4) refcount(1)
unicode(1) "x" { 0078 } refcount(1)
long(3) refcount(2)
--
long(1) refcount(2)
long(2) refcount(2)
----
long(1) refcount(1)
long(2) refcount(1)
long(1) refcount(1)
long(2) refcount(1)
--
long(1) refcount(1)
long(2) refcount(1)
long(1) refcount(1)
long(2) refcount(1)
--
long(1) refcount(1)
long(2) refcount(1)
long(1) refcount(1)
long(2) refcount(1)
--
long(1) refcount(1)
long(3) refcount(1)
long(1) refcount(1)
long(3) refcount(1)
--
long(4) refcount(1)
long(3) refcount(1)
long(4) refcount(1)
long(3) refcount(1)
--
long(3) refcount(1)
long(3) refcount(1)
long(3) refcount(2)
--
long(4) refcount(1)
unicode(1) "x" { 0078 } refcount(1)
long(4) refcount(1)
unicode(1) "x" { 0078 } refcount(1)
long(3) refcount(2)
--
long(1) refcount(2)
long(2) refcount(2)
----r
unicode(2) "ok" { 006f 006b } refcount(2)
unicode(2) "ok" { 006f 006b } refcount(2)
