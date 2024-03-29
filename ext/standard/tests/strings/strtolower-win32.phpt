--TEST--
Test strtolower() function
--SKIPIF--
<?php
if( (substr(PHP_OS, 0, 3) != "WIN") || (@setlocale(LC_CTYPE, "") != "English_United States.1252") )
  die('skip Run only on Windows with locale as "English_United States.1252"');
?>
--FILE--
<?php
/* Prototype: 
     string strtolower ( string $str );
   Description:
     Returns string with all alphabetic characters converted to lowercase.
*/

echo "*** Testing strtolower() with all 256 chars ***\n";
for ($i=0; $i<=255; $i++){
  $char = chr($i);
  print(bin2hex(b"$char"))." => ".(bin2hex(strtolower(b"$char")))."\n";
}

echo "*** Testing strlower() with basic strings ***\n";
$str = "Mary Had A liTTle LAmb and ShE loveD IT So\n";
var_dump(strtolower($str));

echo "\n*** Testing strtolower() with various strings ***";
/* strings to pass strtolower() */ 
$strings = array ( 
  "",
  "string",
  "stRINg0234",
  "1.233.344StrinG12333",
  "$$$$$$!!!!@@@@@@@ ABCDEF !!!***",
  "ABCD\0abcdABCD",
  NULL,
  TRUE,
  FALSE,
  array()
);

$count = 0;
/* loop through to check possible variations */
foreach ($strings as $string) {
  echo "\n-- Iteration $count --\n";
  var_dump( strtolower($string) );
  $count++;
}

echo "\n*** Testing strtolower() with two different case strings ***\n";
if (strtolower("HeLLo woRLd") === strtolower("hEllo WORLD"))
  echo "strings are same, with Case Insensitive\n";
else
  echo "strings are not same\n";

echo "\n*** Testing error conditions ***";
var_dump( strtolower() ); /* Zero arguments */
var_dump( strtolower("a", "b") ); /* Arguments > Expected */

echo "*** Done ***";
?>
--EXPECTF--
*** Testing strtolower() with all 256 chars ***
00 => 00
01 => 01
02 => 02
03 => 03
04 => 04
05 => 05
06 => 06
07 => 07
08 => 08
09 => 09
0a => 0a
0b => 0b
0c => 0c
0d => 0d
0e => 0e
0f => 0f
10 => 10
11 => 11
12 => 12
13 => 13
14 => 14
15 => 15
16 => 16
17 => 17
18 => 18
19 => 19
1a => 1a
1b => 1b
1c => 1c
1d => 1d
1e => 1e
1f => 1f
20 => 20
21 => 21
22 => 22
23 => 23
24 => 24
25 => 25
26 => 26
27 => 27
28 => 28
29 => 29
2a => 2a
2b => 2b
2c => 2c
2d => 2d
2e => 2e
2f => 2f
30 => 30
31 => 31
32 => 32
33 => 33
34 => 34
35 => 35
36 => 36
37 => 37
38 => 38
39 => 39
3a => 3a
3b => 3b
3c => 3c
3d => 3d
3e => 3e
3f => 3f
40 => 40
41 => 61
42 => 62
43 => 63
44 => 64
45 => 65
46 => 66
47 => 67
48 => 68
49 => 69
4a => 6a
4b => 6b
4c => 6c
4d => 6d
4e => 6e
4f => 6f
50 => 70
51 => 71
52 => 72
53 => 73
54 => 74
55 => 75
56 => 76
57 => 77
58 => 78
59 => 79
5a => 7a
5b => 5b
5c => 5c
5d => 5d
5e => 5e
5f => 5f
60 => 60
61 => 61
62 => 62
63 => 63
64 => 64
65 => 65
66 => 66
67 => 67
68 => 68
69 => 69
6a => 6a
6b => 6b
6c => 6c
6d => 6d
6e => 6e
6f => 6f
70 => 70
71 => 71
72 => 72
73 => 73
74 => 74
75 => 75
76 => 76
77 => 77
78 => 78
79 => 79
7a => 7a
7b => 7b
7c => 7c
7d => 7d
7e => 7e
7f => 7f
80 => 80
81 => 81
82 => 82
83 => 83
84 => 84
85 => 85
86 => 86
87 => 87
88 => 88
89 => 89
8a => 9a
8b => 8b
8c => 9c
8d => 8d
8e => 9e
8f => 8f
90 => 90
91 => 91
92 => 92
93 => 93
94 => 94
95 => 95
96 => 96
97 => 97
98 => 98
99 => 99
9a => 9a
9b => 9b
9c => 9c
9d => 9d
9e => 9e
9f => ff
a0 => a0
a1 => a1
a2 => a2
a3 => a3
a4 => a4
a5 => a5
a6 => a6
a7 => a7
a8 => a8
a9 => a9
aa => aa
ab => ab
ac => ac
ad => ad
ae => ae
af => af
b0 => b0
b1 => b1
b2 => b2
b3 => b3
b4 => b4
b5 => b5
b6 => b6
b7 => b7
b8 => b8
b9 => b9
ba => ba
bb => bb
bc => bc
bd => bd
be => be
bf => bf
c0 => e0
c1 => e1
c2 => e2
c3 => e3
c4 => e4
c5 => e5
c6 => e6
c7 => e7
c8 => e8
c9 => e9
ca => ea
cb => eb
cc => ec
cd => ed
ce => ee
cf => ef
d0 => f0
d1 => f1
d2 => f2
d3 => f3
d4 => f4
d5 => f5
d6 => f6
d7 => d7
d8 => f8
d9 => f9
da => fa
db => fb
dc => fc
dd => fd
de => fe
df => df
e0 => e0
e1 => e1
e2 => e2
e3 => e3
e4 => e4
e5 => e5
e6 => e6
e7 => e7
e8 => e8
e9 => e9
ea => ea
eb => eb
ec => ec
ed => ed
ee => ee
ef => ef
f0 => f0
f1 => f1
f2 => f2
f3 => f3
f4 => f4
f5 => f5
f6 => f6
f7 => f7
f8 => f8
f9 => f9
fa => fa
fb => fb
fc => fc
fd => fd
fe => fe
ff => ff
*** Testing strlower() with basic strings ***
unicode(43) "mary had a little lamb and she loved it so
"

*** Testing strtolower() with various strings ***
-- Iteration 0 --
unicode(0) ""

-- Iteration 1 --
unicode(6) "string"

-- Iteration 2 --
unicode(10) "string0234"

-- Iteration 3 --
unicode(20) "1.233.344string12333"

-- Iteration 4 --
unicode(31) "$$$$$$!!!!@@@@@@@ abcdef !!!***"

-- Iteration 5 --
unicode(13) "abcd abcdabcd"

-- Iteration 6 --
unicode(0) ""

-- Iteration 7 --
unicode(1) "1"

-- Iteration 8 --
unicode(0) ""

-- Iteration 9 --

Warning: strtolower() expects parameter 1 to be string (Unicode or binary), array given in %s on line %d
NULL

*** Testing strtolower() with two different case strings ***
strings are same, with Case Insensitive

*** Testing error conditions ***
Warning: strtolower() expects exactly 1 parameter, 0 given in %s on line %d
NULL

Warning: strtolower() expects exactly 1 parameter, 2 given in %s on line %d
NULL
*** Done ***
