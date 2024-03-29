--TEST--
mcrypt_cbf
--SKIPIF--
<?php if (!extension_loaded("mcrypt") || unicode_semantics()) print "skip"; ?>
--FILE--
<?php
$key      = "FooBar";
$secret   = "PHP Testfest 2008";
$cipher   = MCRYPT_RIJNDAEL_128;

$iv       = mcrypt_create_iv(mcrypt_get_iv_size($cipher, MCRYPT_MODE_ECB), MCRYPT_RAND);
$enc_data = mcrypt_cfb($cipher, $key, $secret, MCRYPT_ENCRYPT, $iv);

// we have to trim as AES rounds the blocks and decrypt doesnt detect that
echo trim(mcrypt_cfb($cipher, $key, $enc_data, MCRYPT_DECRYPT, $iv)) . "\n";

// a warning must be issued if we don't use a IV on a AES cipher, that usually requires an IV
mcrypt_cfb($cipher, $key, $enc_data, MCRYPT_DECRYPT);

--EXPECTF--
PHP Testfest 2008

Warning: mcrypt_cfb(): Attempt to use an empty IV, which is NOT recommended in %s on line %d
