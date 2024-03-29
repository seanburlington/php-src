--TEST--
Test mb_encode_mimeheader() function : basic functionality
--SKIPIF--
<?php
extension_loaded('mbstring') or die('skip');
function_exists('mb_encode_mimeheader') or die("skip mb_encode_mimeheader() is not available in this build");
?>

--FILE--
<?php
/* Prototype  : string mb_encode_mimeheader(string $str [, string $charset 
 * [, string $transfer-encoding [, string $linefeed [, int $indent]]]])
 * Description: Converts the string to MIME "encoded-word" in the format of =?charset?(B|Q)?encoded_string?= 
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Test mb_encode_header() with different strings
 */

echo "*** Testing mb_encode_mimeheader() : basic2 ***\n";

//All strings are the same when displayed in their respective encodings
$sjis_string = base64_decode('k/qWe4zqg2WDTINYg2eCxYK3gUIwMTIzNIJUglWCVoJXgliBQg==');
$jis_string = base64_decode('GyRCRnxLXDhsJUYlLSU5JUgkRyQ5ISMbKEIwMTIzNBskQiM1IzYjNyM4IzkhIxsoQg==');
$euc_jp_string = base64_decode('xvzL3LjspcalraW5pcikx6S5oaMwMTIzNKO1o7ajt6O4o7mhow==');

$inputs = array('SJIS' => $sjis_string,
                'JIS' => $jis_string,
                'EUC_JP' => $euc_jp_string);

foreach ($inputs as $lang => $input) {
	echo "\nLanguage: $lang\n";
	echo "-- Base 64: --\n";
	mb_internal_encoding($lang);
	$outEncoding = $lang;
	var_dump(mb_encode_mimeheader($input, $outEncoding, 'B'));
	echo "-- Quoted-Printable --\n";
	var_dump(mb_encode_mimeheader($input, $outEncoding, 'Q'));
}

echo "Done";
?>
--EXPECTF--
*** Testing mb_encode_mimeheader() : basic2 ***

Language: SJIS
-- Base 64: --
string(68) "=?Shift_JIS?B?k/qWe4zqg2WDTINYg2eCxYK3gUIwMTIzNIJUglWCVoJXgliBQg==?="
-- Quoted-Printable --
string(124) "=?Shift_JIS?Q?=93=FA=96=7B=8C=EA=83e=83L=83X=83g=82=C5=82=B7=81B=30=31=32?=
 =?Shift_JIS?Q?=33=34=82T=82U=82V=82W=82X=81B?="

Language: JIS
-- Base 64: --
string(115) "=?ISO-2022-JP?B?GyRCRnxLXDhsJUYlLSU5JUgkRyQ5ISMbKEIwMTIzNBskQiM1IzYbKEI=?=
 =?ISO-2022-JP?B?GyRCIzcjOCM5ISMbKEI=?="
-- Quoted-Printable --
string(209) "=?ISO-2022-JP?Q?=1B=24BF=7CK=5C=38l=25F=25-=25=39=25H=24G=24=39=1B=28B?=
 =?ISO-2022-JP?Q?=1B=24B!=23=1B=28B=30=31=32=33=34=1B=24B=23=35=1B=28B?=
 =?ISO-2022-JP?Q?=1B=24B=23=36=23=37=23=38=23=39!=23=1B=28B?="

Language: EUC_JP
-- Base 64: --
string(65) "=?EUC-JP?B?xvzL3LjspcalraW5pcikx6S5oaMwMTIzNKO1o7ajt6O4o7mhow==?="
-- Quoted-Printable --
string(140) "=?EUC-JP?Q?=C6=FC=CB=DC=B8=EC=A5=C6=A5=AD=A5=B9=A5=C8=A4=C7=A4=B9=A1=A3?=
 =?EUC-JP?Q?=30=31=32=33=34=A3=B5=A3=B6=A3=B7=A3=B8=A3=B9=A1=A3?="
Done

