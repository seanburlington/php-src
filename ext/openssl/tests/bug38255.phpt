--TEST--
openssl key from zval leaks 
--SKIPIF--
<?php 
if (!extension_loaded("openssl")) die("skip"); 
?>
--FILE--
<?php
$pub_key_id = false; 
$signature = b'';
$ok = openssl_verify(b"foo", $signature, $pub_key_id, OPENSSL_ALGO_MD5);

class test {
	function __toString() {
		return "test object";
	}
}
$t = new test;


var_dump(openssl_verify(b"foo", $signature, $pub_key_id, OPENSSL_ALGO_MD5));
var_dump(openssl_verify(b"foo", $t, $pub_key_id, OPENSSL_ALGO_MD5));
var_dump(openssl_verify(b"foo", new stdClass, $pub_key_id, OPENSSL_ALGO_MD5));
var_dump(openssl_verify(b"foo", new stdClass, array(), OPENSSL_ALGO_MD5));
var_dump(openssl_verify(b"foo", array(), array(), OPENSSL_ALGO_MD5));
var_dump(openssl_verify());
var_dump(openssl_verify(new stdClass, new stdClass, array(), 10000));

echo "Done\n";

?>
--EXPECTF--
Warning: openssl_verify(): supplied key param cannot be coerced into a public key in %s on line %d

Warning: openssl_verify(): supplied key param cannot be coerced into a public key in %s on line %d
bool(false)

Warning: openssl_verify(): supplied key param cannot be coerced into a public key in %s on line %d
bool(false)

Warning: openssl_verify() expects parameter 2 to be binary string, object given in %s on line %d
bool(false)

Warning: openssl_verify() expects parameter 2 to be binary string, object given in %s on line %d
bool(false)

Warning: openssl_verify() expects parameter 2 to be binary string, array given in %s on line %d
bool(false)

Warning: openssl_verify() expects at least 3 parameters, 0 given in %s on line %d
bool(false)

Warning: openssl_verify() expects parameter 1 to be binary string, object given in %s on line %d
bool(false)
Done
