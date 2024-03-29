--TEST--
Test htmlentities() function
--FILE--
<?php
/* Prototype: string htmlentities ( string $string [, int $quote_style [, string $charset]] );
   Description: Convert all applicable characters to HTML entities
*/

/* retrieving htmlentities from the ANSI character table */
echo "*** Retrieving htmlentities for 256 characters ***\n";
for($i=0; $i<256; $i++)
  var_dump( bin2hex( htmlentities(b"chr($i)")) );

/* giving arguments as NULL */
echo "\n*** Testing htmlentities() with NULL as first,second and third argument ***\n";
var_dump( htmlentities("\x82\x86\x99\x9f\x80\x82\x81", NULL, 'cp1252') );
var_dump( htmlentities("\x82\x86\x99\x9f\x80\x82\x81", ENT_QUOTES, NULL) );
var_dump( htmlentities("\x82\x86\x99\x9f\x80\x82\x81", ENT_NOQUOTES, NULL) );
var_dump( htmlentities("\x82\x86\x99\x9f\x80\x82\x81", ENT_COMPAT, NULL) );
var_dump( htmlentities(NULL, NULL, NULL) );

/* giving long string to check for proper memory re-allocation */
echo "\n*** Checking for proper memory allocation with long string ***\n";
var_dump( htmlentities("\x82\x86\x99\x9f\x80\x82\x86\x84\x80\x89\x85\x83\x86\x84\x80\x91\x83\x91\x86\x87\x85\x86\x88\x82\x89\x92\x91\x83", ENT_QUOTES, 'cp1252'));

/* giving a normal string */
echo "\n*** Testing a normal string with htmlentities() ***\n";
var_dump( htmlentities("<html> This is a test! </html>") );

/* checking behavior of quote */
echo "\n*** Testing htmlentites() on a quote ***\n";
$str = "A 'quote' is <b>bold</b>";
var_dump( htmlentities($str) );
var_dump( htmlentities($str, ENT_QUOTES) );
var_dump( htmlentities($str, ENT_NOQUOTES) );
var_dump( htmlentities($str, ENT_COMPAT) );

echo "\n*** Testing error conditions ***\n";
/* zero argument */
var_dump( htmlentities() );
/* arguments more than expected */
var_dump( htmlentities("\x84\x91",ENT_QUOTES, 'cp1252', "test1") );

echo "Done\n";
?>

--EXPECTF--
*** Retrieving htmlentities for 256 characters ***
unicode(12) "636872283029"
unicode(12) "636872283129"
unicode(12) "636872283229"
unicode(12) "636872283329"
unicode(12) "636872283429"
unicode(12) "636872283529"
unicode(12) "636872283629"
unicode(12) "636872283729"
unicode(12) "636872283829"
unicode(12) "636872283929"
unicode(14) "63687228313029"
unicode(14) "63687228313129"
unicode(14) "63687228313229"
unicode(14) "63687228313329"
unicode(14) "63687228313429"
unicode(14) "63687228313529"
unicode(14) "63687228313629"
unicode(14) "63687228313729"
unicode(14) "63687228313829"
unicode(14) "63687228313929"
unicode(14) "63687228323029"
unicode(14) "63687228323129"
unicode(14) "63687228323229"
unicode(14) "63687228323329"
unicode(14) "63687228323429"
unicode(14) "63687228323529"
unicode(14) "63687228323629"
unicode(14) "63687228323729"
unicode(14) "63687228323829"
unicode(14) "63687228323929"
unicode(14) "63687228333029"
unicode(14) "63687228333129"
unicode(14) "63687228333229"
unicode(14) "63687228333329"
unicode(14) "63687228333429"
unicode(14) "63687228333529"
unicode(14) "63687228333629"
unicode(14) "63687228333729"
unicode(14) "63687228333829"
unicode(14) "63687228333929"
unicode(14) "63687228343029"
unicode(14) "63687228343129"
unicode(14) "63687228343229"
unicode(14) "63687228343329"
unicode(14) "63687228343429"
unicode(14) "63687228343529"
unicode(14) "63687228343629"
unicode(14) "63687228343729"
unicode(14) "63687228343829"
unicode(14) "63687228343929"
unicode(14) "63687228353029"
unicode(14) "63687228353129"
unicode(14) "63687228353229"
unicode(14) "63687228353329"
unicode(14) "63687228353429"
unicode(14) "63687228353529"
unicode(14) "63687228353629"
unicode(14) "63687228353729"
unicode(14) "63687228353829"
unicode(14) "63687228353929"
unicode(14) "63687228363029"
unicode(14) "63687228363129"
unicode(14) "63687228363229"
unicode(14) "63687228363329"
unicode(14) "63687228363429"
unicode(14) "63687228363529"
unicode(14) "63687228363629"
unicode(14) "63687228363729"
unicode(14) "63687228363829"
unicode(14) "63687228363929"
unicode(14) "63687228373029"
unicode(14) "63687228373129"
unicode(14) "63687228373229"
unicode(14) "63687228373329"
unicode(14) "63687228373429"
unicode(14) "63687228373529"
unicode(14) "63687228373629"
unicode(14) "63687228373729"
unicode(14) "63687228373829"
unicode(14) "63687228373929"
unicode(14) "63687228383029"
unicode(14) "63687228383129"
unicode(14) "63687228383229"
unicode(14) "63687228383329"
unicode(14) "63687228383429"
unicode(14) "63687228383529"
unicode(14) "63687228383629"
unicode(14) "63687228383729"
unicode(14) "63687228383829"
unicode(14) "63687228383929"
unicode(14) "63687228393029"
unicode(14) "63687228393129"
unicode(14) "63687228393229"
unicode(14) "63687228393329"
unicode(14) "63687228393429"
unicode(14) "63687228393529"
unicode(14) "63687228393629"
unicode(14) "63687228393729"
unicode(14) "63687228393829"
unicode(14) "63687228393929"
unicode(16) "6368722831303029"
unicode(16) "6368722831303129"
unicode(16) "6368722831303229"
unicode(16) "6368722831303329"
unicode(16) "6368722831303429"
unicode(16) "6368722831303529"
unicode(16) "6368722831303629"
unicode(16) "6368722831303729"
unicode(16) "6368722831303829"
unicode(16) "6368722831303929"
unicode(16) "6368722831313029"
unicode(16) "6368722831313129"
unicode(16) "6368722831313229"
unicode(16) "6368722831313329"
unicode(16) "6368722831313429"
unicode(16) "6368722831313529"
unicode(16) "6368722831313629"
unicode(16) "6368722831313729"
unicode(16) "6368722831313829"
unicode(16) "6368722831313929"
unicode(16) "6368722831323029"
unicode(16) "6368722831323129"
unicode(16) "6368722831323229"
unicode(16) "6368722831323329"
unicode(16) "6368722831323429"
unicode(16) "6368722831323529"
unicode(16) "6368722831323629"
unicode(16) "6368722831323729"
unicode(16) "6368722831323829"
unicode(16) "6368722831323929"
unicode(16) "6368722831333029"
unicode(16) "6368722831333129"
unicode(16) "6368722831333229"
unicode(16) "6368722831333329"
unicode(16) "6368722831333429"
unicode(16) "6368722831333529"
unicode(16) "6368722831333629"
unicode(16) "6368722831333729"
unicode(16) "6368722831333829"
unicode(16) "6368722831333929"
unicode(16) "6368722831343029"
unicode(16) "6368722831343129"
unicode(16) "6368722831343229"
unicode(16) "6368722831343329"
unicode(16) "6368722831343429"
unicode(16) "6368722831343529"
unicode(16) "6368722831343629"
unicode(16) "6368722831343729"
unicode(16) "6368722831343829"
unicode(16) "6368722831343929"
unicode(16) "6368722831353029"
unicode(16) "6368722831353129"
unicode(16) "6368722831353229"
unicode(16) "6368722831353329"
unicode(16) "6368722831353429"
unicode(16) "6368722831353529"
unicode(16) "6368722831353629"
unicode(16) "6368722831353729"
unicode(16) "6368722831353829"
unicode(16) "6368722831353929"
unicode(16) "6368722831363029"
unicode(16) "6368722831363129"
unicode(16) "6368722831363229"
unicode(16) "6368722831363329"
unicode(16) "6368722831363429"
unicode(16) "6368722831363529"
unicode(16) "6368722831363629"
unicode(16) "6368722831363729"
unicode(16) "6368722831363829"
unicode(16) "6368722831363929"
unicode(16) "6368722831373029"
unicode(16) "6368722831373129"
unicode(16) "6368722831373229"
unicode(16) "6368722831373329"
unicode(16) "6368722831373429"
unicode(16) "6368722831373529"
unicode(16) "6368722831373629"
unicode(16) "6368722831373729"
unicode(16) "6368722831373829"
unicode(16) "6368722831373929"
unicode(16) "6368722831383029"
unicode(16) "6368722831383129"
unicode(16) "6368722831383229"
unicode(16) "6368722831383329"
unicode(16) "6368722831383429"
unicode(16) "6368722831383529"
unicode(16) "6368722831383629"
unicode(16) "6368722831383729"
unicode(16) "6368722831383829"
unicode(16) "6368722831383929"
unicode(16) "6368722831393029"
unicode(16) "6368722831393129"
unicode(16) "6368722831393229"
unicode(16) "6368722831393329"
unicode(16) "6368722831393429"
unicode(16) "6368722831393529"
unicode(16) "6368722831393629"
unicode(16) "6368722831393729"
unicode(16) "6368722831393829"
unicode(16) "6368722831393929"
unicode(16) "6368722832303029"
unicode(16) "6368722832303129"
unicode(16) "6368722832303229"
unicode(16) "6368722832303329"
unicode(16) "6368722832303429"
unicode(16) "6368722832303529"
unicode(16) "6368722832303629"
unicode(16) "6368722832303729"
unicode(16) "6368722832303829"
unicode(16) "6368722832303929"
unicode(16) "6368722832313029"
unicode(16) "6368722832313129"
unicode(16) "6368722832313229"
unicode(16) "6368722832313329"
unicode(16) "6368722832313429"
unicode(16) "6368722832313529"
unicode(16) "6368722832313629"
unicode(16) "6368722832313729"
unicode(16) "6368722832313829"
unicode(16) "6368722832313929"
unicode(16) "6368722832323029"
unicode(16) "6368722832323129"
unicode(16) "6368722832323229"
unicode(16) "6368722832323329"
unicode(16) "6368722832323429"
unicode(16) "6368722832323529"
unicode(16) "6368722832323629"
unicode(16) "6368722832323729"
unicode(16) "6368722832323829"
unicode(16) "6368722832323929"
unicode(16) "6368722832333029"
unicode(16) "6368722832333129"
unicode(16) "6368722832333229"
unicode(16) "6368722832333329"
unicode(16) "6368722832333429"
unicode(16) "6368722832333529"
unicode(16) "6368722832333629"
unicode(16) "6368722832333729"
unicode(16) "6368722832333829"
unicode(16) "6368722832333929"
unicode(16) "6368722832343029"
unicode(16) "6368722832343129"
unicode(16) "6368722832343229"
unicode(16) "6368722832343329"
unicode(16) "6368722832343429"
unicode(16) "6368722832343529"
unicode(16) "6368722832343629"
unicode(16) "6368722832343729"
unicode(16) "6368722832343829"
unicode(16) "6368722832343929"
unicode(16) "6368722832353029"
unicode(16) "6368722832353129"
unicode(16) "6368722832353229"
unicode(16) "6368722832353329"
unicode(16) "6368722832353429"
unicode(16) "6368722832353529"

*** Testing htmlentities() with NULL as first,second and third argument ***
unicode(7) ""
unicode(7) ""
unicode(7) ""
unicode(7) ""
unicode(0) ""

*** Checking for proper memory allocation with long string ***
unicode(28) ""

*** Testing a normal string with htmlentities() ***
unicode(42) "&lt;html&gt; This is a test! &lt;/html&gt;"

*** Testing htmlentites() on a quote ***
unicode(36) "A 'quote' is &lt;b&gt;bold&lt;/b&gt;"
unicode(46) "A &#039;quote&#039; is &lt;b&gt;bold&lt;/b&gt;"
unicode(36) "A 'quote' is &lt;b&gt;bold&lt;/b&gt;"
unicode(36) "A 'quote' is &lt;b&gt;bold&lt;/b&gt;"

*** Testing error conditions ***

Warning: htmlentities() expects at least 1 parameter, 0 given in %s on line %d
NULL
unicode(2) ""
Done
