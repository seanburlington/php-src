--TEST--
Test ^ operator : various numbers as strings
--FILE--
<?php

$strVals = array(
   b"0",b"65",b"-44", b"1.2", b"-7.7", b"abc", b"123abc", b"123e5", b"123e5xyz", b" 123abc", b"123 abc", b"123abc ", b"3.4a",
   b"a5.9"
);

error_reporting(E_ERROR);

foreach ($strVals as $strVal) {
   foreach($strVals as $otherVal) {
	   echo "--- testing: '$strVal' ^ '$otherVal' ---\n";   
      var_dump(bin2hex($strVal^$otherVal));
   }
}

   
?>
===DONE===
--EXPECT--
--- testing: '0' ^ '0' ---
unicode(2) "00"
--- testing: '0' ^ '65' ---
unicode(2) "06"
--- testing: '0' ^ '-44' ---
unicode(2) "1d"
--- testing: '0' ^ '1.2' ---
unicode(2) "01"
--- testing: '0' ^ '-7.7' ---
unicode(2) "1d"
--- testing: '0' ^ 'abc' ---
unicode(2) "51"
--- testing: '0' ^ '123abc' ---
unicode(2) "01"
--- testing: '0' ^ '123e5' ---
unicode(2) "01"
--- testing: '0' ^ '123e5xyz' ---
unicode(2) "01"
--- testing: '0' ^ ' 123abc' ---
unicode(2) "10"
--- testing: '0' ^ '123 abc' ---
unicode(2) "01"
--- testing: '0' ^ '123abc ' ---
unicode(2) "01"
--- testing: '0' ^ '3.4a' ---
unicode(2) "03"
--- testing: '0' ^ 'a5.9' ---
unicode(2) "51"
--- testing: '65' ^ '0' ---
unicode(2) "06"
--- testing: '65' ^ '65' ---
unicode(4) "0000"
--- testing: '65' ^ '-44' ---
unicode(4) "1b01"
--- testing: '65' ^ '1.2' ---
unicode(4) "071b"
--- testing: '65' ^ '-7.7' ---
unicode(4) "1b02"
--- testing: '65' ^ 'abc' ---
unicode(4) "5757"
--- testing: '65' ^ '123abc' ---
unicode(4) "0707"
--- testing: '65' ^ '123e5' ---
unicode(4) "0707"
--- testing: '65' ^ '123e5xyz' ---
unicode(4) "0707"
--- testing: '65' ^ ' 123abc' ---
unicode(4) "1604"
--- testing: '65' ^ '123 abc' ---
unicode(4) "0707"
--- testing: '65' ^ '123abc ' ---
unicode(4) "0707"
--- testing: '65' ^ '3.4a' ---
unicode(4) "051b"
--- testing: '65' ^ 'a5.9' ---
unicode(4) "5700"
--- testing: '-44' ^ '0' ---
unicode(2) "1d"
--- testing: '-44' ^ '65' ---
unicode(4) "1b01"
--- testing: '-44' ^ '-44' ---
unicode(6) "000000"
--- testing: '-44' ^ '1.2' ---
unicode(6) "1c1a06"
--- testing: '-44' ^ '-7.7' ---
unicode(6) "00031a"
--- testing: '-44' ^ 'abc' ---
unicode(6) "4c5657"
--- testing: '-44' ^ '123abc' ---
unicode(6) "1c0607"
--- testing: '-44' ^ '123e5' ---
unicode(6) "1c0607"
--- testing: '-44' ^ '123e5xyz' ---
unicode(6) "1c0607"
--- testing: '-44' ^ ' 123abc' ---
unicode(6) "0d0506"
--- testing: '-44' ^ '123 abc' ---
unicode(6) "1c0607"
--- testing: '-44' ^ '123abc ' ---
unicode(6) "1c0607"
--- testing: '-44' ^ '3.4a' ---
unicode(6) "1e1a00"
--- testing: '-44' ^ 'a5.9' ---
unicode(6) "4c011a"
--- testing: '1.2' ^ '0' ---
unicode(2) "01"
--- testing: '1.2' ^ '65' ---
unicode(4) "071b"
--- testing: '1.2' ^ '-44' ---
unicode(6) "1c1a06"
--- testing: '1.2' ^ '1.2' ---
unicode(6) "000000"
--- testing: '1.2' ^ '-7.7' ---
unicode(6) "1c191c"
--- testing: '1.2' ^ 'abc' ---
unicode(6) "504c51"
--- testing: '1.2' ^ '123abc' ---
unicode(6) "001c01"
--- testing: '1.2' ^ '123e5' ---
unicode(6) "001c01"
--- testing: '1.2' ^ '123e5xyz' ---
unicode(6) "001c01"
--- testing: '1.2' ^ ' 123abc' ---
unicode(6) "111f00"
--- testing: '1.2' ^ '123 abc' ---
unicode(6) "001c01"
--- testing: '1.2' ^ '123abc ' ---
unicode(6) "001c01"
--- testing: '1.2' ^ '3.4a' ---
unicode(6) "020006"
--- testing: '1.2' ^ 'a5.9' ---
unicode(6) "501b1c"
--- testing: '-7.7' ^ '0' ---
unicode(2) "1d"
--- testing: '-7.7' ^ '65' ---
unicode(4) "1b02"
--- testing: '-7.7' ^ '-44' ---
unicode(6) "00031a"
--- testing: '-7.7' ^ '1.2' ---
unicode(6) "1c191c"
--- testing: '-7.7' ^ '-7.7' ---
unicode(8) "00000000"
--- testing: '-7.7' ^ 'abc' ---
unicode(6) "4c554d"
--- testing: '-7.7' ^ '123abc' ---
unicode(8) "1c051d56"
--- testing: '-7.7' ^ '123e5' ---
unicode(8) "1c051d52"
--- testing: '-7.7' ^ '123e5xyz' ---
unicode(8) "1c051d52"
--- testing: '-7.7' ^ ' 123abc' ---
unicode(8) "0d061c04"
--- testing: '-7.7' ^ '123 abc' ---
unicode(8) "1c051d17"
--- testing: '-7.7' ^ '123abc ' ---
unicode(8) "1c051d56"
--- testing: '-7.7' ^ '3.4a' ---
unicode(8) "1e191a56"
--- testing: '-7.7' ^ 'a5.9' ---
unicode(8) "4c02000e"
--- testing: 'abc' ^ '0' ---
unicode(2) "51"
--- testing: 'abc' ^ '65' ---
unicode(4) "5757"
--- testing: 'abc' ^ '-44' ---
unicode(6) "4c5657"
--- testing: 'abc' ^ '1.2' ---
unicode(6) "504c51"
--- testing: 'abc' ^ '-7.7' ---
unicode(6) "4c554d"
--- testing: 'abc' ^ 'abc' ---
unicode(6) "000000"
--- testing: 'abc' ^ '123abc' ---
unicode(6) "505050"
--- testing: 'abc' ^ '123e5' ---
unicode(6) "505050"
--- testing: 'abc' ^ '123e5xyz' ---
unicode(6) "505050"
--- testing: 'abc' ^ ' 123abc' ---
unicode(6) "415351"
--- testing: 'abc' ^ '123 abc' ---
unicode(6) "505050"
--- testing: 'abc' ^ '123abc ' ---
unicode(6) "505050"
--- testing: 'abc' ^ '3.4a' ---
unicode(6) "524c57"
--- testing: 'abc' ^ 'a5.9' ---
unicode(6) "00574d"
--- testing: '123abc' ^ '0' ---
unicode(2) "01"
--- testing: '123abc' ^ '65' ---
unicode(4) "0707"
--- testing: '123abc' ^ '-44' ---
unicode(6) "1c0607"
--- testing: '123abc' ^ '1.2' ---
unicode(6) "001c01"
--- testing: '123abc' ^ '-7.7' ---
unicode(8) "1c051d56"
--- testing: '123abc' ^ 'abc' ---
unicode(6) "505050"
--- testing: '123abc' ^ '123abc' ---
unicode(12) "000000000000"
--- testing: '123abc' ^ '123e5' ---
unicode(10) "0000000457"
--- testing: '123abc' ^ '123e5xyz' ---
unicode(12) "00000004571b"
--- testing: '123abc' ^ ' 123abc' ---
unicode(12) "110301520301"
--- testing: '123abc' ^ '123 abc' ---
unicode(12) "000000410301"
--- testing: '123abc' ^ '123abc ' ---
unicode(12) "000000000000"
--- testing: '123abc' ^ '3.4a' ---
unicode(8) "021c0700"
--- testing: '123abc' ^ 'a5.9' ---
unicode(8) "50071d58"
--- testing: '123e5' ^ '0' ---
unicode(2) "01"
--- testing: '123e5' ^ '65' ---
unicode(4) "0707"
--- testing: '123e5' ^ '-44' ---
unicode(6) "1c0607"
--- testing: '123e5' ^ '1.2' ---
unicode(6) "001c01"
--- testing: '123e5' ^ '-7.7' ---
unicode(8) "1c051d52"
--- testing: '123e5' ^ 'abc' ---
unicode(6) "505050"
--- testing: '123e5' ^ '123abc' ---
unicode(10) "0000000457"
--- testing: '123e5' ^ '123e5' ---
unicode(10) "0000000000"
--- testing: '123e5' ^ '123e5xyz' ---
unicode(10) "0000000000"
--- testing: '123e5' ^ ' 123abc' ---
unicode(10) "1103015654"
--- testing: '123e5' ^ '123 abc' ---
unicode(10) "0000004554"
--- testing: '123e5' ^ '123abc ' ---
unicode(10) "0000000457"
--- testing: '123e5' ^ '3.4a' ---
unicode(8) "021c0704"
--- testing: '123e5' ^ 'a5.9' ---
unicode(8) "50071d5c"
--- testing: '123e5xyz' ^ '0' ---
unicode(2) "01"
--- testing: '123e5xyz' ^ '65' ---
unicode(4) "0707"
--- testing: '123e5xyz' ^ '-44' ---
unicode(6) "1c0607"
--- testing: '123e5xyz' ^ '1.2' ---
unicode(6) "001c01"
--- testing: '123e5xyz' ^ '-7.7' ---
unicode(8) "1c051d52"
--- testing: '123e5xyz' ^ 'abc' ---
unicode(6) "505050"
--- testing: '123e5xyz' ^ '123abc' ---
unicode(12) "00000004571b"
--- testing: '123e5xyz' ^ '123e5' ---
unicode(10) "0000000000"
--- testing: '123e5xyz' ^ '123e5xyz' ---
unicode(16) "0000000000000000"
--- testing: '123e5xyz' ^ ' 123abc' ---
unicode(14) "11030156541a1a"
--- testing: '123e5xyz' ^ '123 abc' ---
unicode(14) "00000045541a1a"
--- testing: '123e5xyz' ^ '123abc ' ---
unicode(14) "00000004571b59"
--- testing: '123e5xyz' ^ '3.4a' ---
unicode(8) "021c0704"
--- testing: '123e5xyz' ^ 'a5.9' ---
unicode(8) "50071d5c"
--- testing: ' 123abc' ^ '0' ---
unicode(2) "10"
--- testing: ' 123abc' ^ '65' ---
unicode(4) "1604"
--- testing: ' 123abc' ^ '-44' ---
unicode(6) "0d0506"
--- testing: ' 123abc' ^ '1.2' ---
unicode(6) "111f00"
--- testing: ' 123abc' ^ '-7.7' ---
unicode(8) "0d061c04"
--- testing: ' 123abc' ^ 'abc' ---
unicode(6) "415351"
--- testing: ' 123abc' ^ '123abc' ---
unicode(12) "110301520301"
--- testing: ' 123abc' ^ '123e5' ---
unicode(10) "1103015654"
--- testing: ' 123abc' ^ '123e5xyz' ---
unicode(14) "11030156541a1a"
--- testing: ' 123abc' ^ ' 123abc' ---
unicode(14) "00000000000000"
--- testing: ' 123abc' ^ '123 abc' ---
unicode(14) "11030113000000"
--- testing: ' 123abc' ^ '123abc ' ---
unicode(14) "11030152030143"
--- testing: ' 123abc' ^ '3.4a' ---
unicode(8) "131f0652"
--- testing: ' 123abc' ^ 'a5.9' ---
unicode(8) "41041c0a"
--- testing: '123 abc' ^ '0' ---
unicode(2) "01"
--- testing: '123 abc' ^ '65' ---
unicode(4) "0707"
--- testing: '123 abc' ^ '-44' ---
unicode(6) "1c0607"
--- testing: '123 abc' ^ '1.2' ---
unicode(6) "001c01"
--- testing: '123 abc' ^ '-7.7' ---
unicode(8) "1c051d17"
--- testing: '123 abc' ^ 'abc' ---
unicode(6) "505050"
--- testing: '123 abc' ^ '123abc' ---
unicode(12) "000000410301"
--- testing: '123 abc' ^ '123e5' ---
unicode(10) "0000004554"
--- testing: '123 abc' ^ '123e5xyz' ---
unicode(14) "00000045541a1a"
--- testing: '123 abc' ^ ' 123abc' ---
unicode(14) "11030113000000"
--- testing: '123 abc' ^ '123 abc' ---
unicode(14) "00000000000000"
--- testing: '123 abc' ^ '123abc ' ---
unicode(14) "00000041030143"
--- testing: '123 abc' ^ '3.4a' ---
unicode(8) "021c0741"
--- testing: '123 abc' ^ 'a5.9' ---
unicode(8) "50071d19"
--- testing: '123abc ' ^ '0' ---
unicode(2) "01"
--- testing: '123abc ' ^ '65' ---
unicode(4) "0707"
--- testing: '123abc ' ^ '-44' ---
unicode(6) "1c0607"
--- testing: '123abc ' ^ '1.2' ---
unicode(6) "001c01"
--- testing: '123abc ' ^ '-7.7' ---
unicode(8) "1c051d56"
--- testing: '123abc ' ^ 'abc' ---
unicode(6) "505050"
--- testing: '123abc ' ^ '123abc' ---
unicode(12) "000000000000"
--- testing: '123abc ' ^ '123e5' ---
unicode(10) "0000000457"
--- testing: '123abc ' ^ '123e5xyz' ---
unicode(14) "00000004571b59"
--- testing: '123abc ' ^ ' 123abc' ---
unicode(14) "11030152030143"
--- testing: '123abc ' ^ '123 abc' ---
unicode(14) "00000041030143"
--- testing: '123abc ' ^ '123abc ' ---
unicode(14) "00000000000000"
--- testing: '123abc ' ^ '3.4a' ---
unicode(8) "021c0700"
--- testing: '123abc ' ^ 'a5.9' ---
unicode(8) "50071d58"
--- testing: '3.4a' ^ '0' ---
unicode(2) "03"
--- testing: '3.4a' ^ '65' ---
unicode(4) "051b"
--- testing: '3.4a' ^ '-44' ---
unicode(6) "1e1a00"
--- testing: '3.4a' ^ '1.2' ---
unicode(6) "020006"
--- testing: '3.4a' ^ '-7.7' ---
unicode(8) "1e191a56"
--- testing: '3.4a' ^ 'abc' ---
unicode(6) "524c57"
--- testing: '3.4a' ^ '123abc' ---
unicode(8) "021c0700"
--- testing: '3.4a' ^ '123e5' ---
unicode(8) "021c0704"
--- testing: '3.4a' ^ '123e5xyz' ---
unicode(8) "021c0704"
--- testing: '3.4a' ^ ' 123abc' ---
unicode(8) "131f0652"
--- testing: '3.4a' ^ '123 abc' ---
unicode(8) "021c0741"
--- testing: '3.4a' ^ '123abc ' ---
unicode(8) "021c0700"
--- testing: '3.4a' ^ '3.4a' ---
unicode(8) "00000000"
--- testing: '3.4a' ^ 'a5.9' ---
unicode(8) "521b1a58"
--- testing: 'a5.9' ^ '0' ---
unicode(2) "51"
--- testing: 'a5.9' ^ '65' ---
unicode(4) "5700"
--- testing: 'a5.9' ^ '-44' ---
unicode(6) "4c011a"
--- testing: 'a5.9' ^ '1.2' ---
unicode(6) "501b1c"
--- testing: 'a5.9' ^ '-7.7' ---
unicode(8) "4c02000e"
--- testing: 'a5.9' ^ 'abc' ---
unicode(6) "00574d"
--- testing: 'a5.9' ^ '123abc' ---
unicode(8) "50071d58"
--- testing: 'a5.9' ^ '123e5' ---
unicode(8) "50071d5c"
--- testing: 'a5.9' ^ '123e5xyz' ---
unicode(8) "50071d5c"
--- testing: 'a5.9' ^ ' 123abc' ---
unicode(8) "41041c0a"
--- testing: 'a5.9' ^ '123 abc' ---
unicode(8) "50071d19"
--- testing: 'a5.9' ^ '123abc ' ---
unicode(8) "50071d58"
--- testing: 'a5.9' ^ '3.4a' ---
unicode(8) "521b1a58"
--- testing: 'a5.9' ^ 'a5.9' ---
unicode(8) "00000000"
===DONE===
