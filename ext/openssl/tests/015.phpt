--TEST--
openssl_public_encrypt() tests
--SKIPIF--
<?php if (!extension_loaded("openssl")) print "skip"; ?>
--FILE--
<?php
$data = b"Testing openssl_public_encrypt()";
$privkey = "file://" . dirname(__FILE__) . "/private.key";
$pubkey = "file://" . dirname(__FILE__) . "/public.key";
$wrong = b"wrong";
$invalid_ascii_string = "non-ascii-unicode\u0500";
class test {
        function __toString() {
                return "test";
        }
}
$obj = new test;

var_dump(openssl_public_encrypt($data, $encrypted, $pubkey));
var_dump(openssl_public_encrypt($data, $encrypted, $privkey));
var_dump(openssl_public_encrypt($data, $encrypted, $wrong));
var_dump(openssl_public_encrypt($invalid_ascii_string, $encrypted, $pubkey));
var_dump(openssl_public_encrypt($data, $encrypted, $invalid_ascii_string));
var_dump(openssl_public_encrypt($data, $encrypted, $obj));
var_dump(openssl_public_encrypt($obj, $encrypted, $pubkey));
openssl_private_decrypt($encrypted, $output, $privkey);
var_dump($output);
?>
--EXPECTF--
bool(true)

Warning: openssl_public_encrypt(): key parameter is not a valid public key in %s on line %d
bool(false)

Warning: openssl_public_encrypt(): key parameter is not a valid public key in %s on line %d
bool(false)

Warning: openssl_public_encrypt() expects parameter 1 to be strictly a binary string, Unicode string given in %s on line %d
NULL

Warning: openssl_public_encrypt(): Binary string expected, Unicode string received in %s on line %d

Warning: openssl_public_encrypt(): key parameter is not a valid public key in %s on line %d
bool(false)

Warning: openssl_public_encrypt(): key parameter is not a valid public key in %s on line %d
bool(false)
bool(true)
string(4) "test"

