--TEST--
basename
--CREDITS--
Dave Kelsey <d_kelsey@uk.ibm.com>
--FILE--
<?php
/* 
 * proto string basename(string path [, string suffix])
 * Function is implemented in ext/standard/string.c
 */ 
$file_paths = array (
  /* simple paths */
  array("bar"),
  array("/foo/bar"),
  array("foo/bar"),
  array("/bar"),

  /* simple paths with trailing slashes */
  array("bar/"),
  array("/bar/"),
  array("/foo/bar/"),
  array("foo/bar/"),
  array("/bar/"),

  /* paths with suffix removal */
  array("bar.gz", ".gz"),
  array("bar.gz", "bar.gz"),
  array("/foo/bar.gz", ".gz"),
  array("foo/bar.gz", ".gz"),
  array("/bar.gz", ".gz"),  

  /* paths with suffix and trailing slashes with suffix removal*/
  array("bar.gz/", ".gz"),
  array("/bar.gz/", ".gz"),
  array("/foo/bar.gz/", ".gz"),
  array("foo/bar.gz/", ".gz"),
  array("/bar.gz/", ".gz"),  
  
  /* paths with basename only suffix, with suffix removal*/
  array("/.gz", ".gz"),
  array(".gz", ".gz"),
  array("/foo/.gz", ".gz"),

  /* paths with basename only suffix & trailing slashes, with suffix removal*/
  array(".gz/", ".gz"),
  array("/foo/.gz/", ".gz"),
  array("foo/.gz/", ".gz"),

  /* paths with binary value to check if the function is binary safe*/
  array("foo".chr(0)."bar"),
  array("/foo".chr(0)."bar"),
  array("/foo".chr(0)."bar/"),
  array("foo".chr(0)."bar/"),
  array("foo".chr(0)."bar/test"),
  array("/foo".chr(0)."bar/bar.gz", ".gz"),
  array("/foo".chr(0)."bar/bar.gz")
);

$file_path_variations = array (
  /* paths with shortcut home dir char, with suffix variation */
  array("~/home/user/bar"),
  array("~/home/user/bar", ""),
  array("~/home/user/bar", NULL),
  array("~/home/user/bar", ' '),
  array("~/home/user/bar.tar", ".tar"),
  array("~/home/user/bar.tar", "~"),
  array("~/home/user/bar.tar/", "~"),
  array("~/home/user/bar.tar/", ""),
  array("~/home/user/bar.tar", NULL),
  array("~/home/user/bar.tar", ''),
  array("~/home/user/bar.tar", " "),

  /* paths with hostname:dir notation, with suffix variation */
  array("hostname:/home/usr/bar.tar"),
  array("hostname:/home/user/bar.tar", "home"),
  array("hostname:/home/user/tbar.gz", "bar.gz"),
  array("hostname:/home/user/tbar.gz", "/bar.gz"),
  array("hostname:/home/user/tbar.gz", "/bar.gz/"),
  array("hostname:/home/user/tbar.gz/", "/bar.gz/"),
  array("hostname:/home/user/tbar.gz/", "/bar.gz/"),
  array("hostname:/home/user/My Pics.gz/", "/bar.gz/"),
  array("hostname:/home/user/My Pics.gz/", "Pics.gz/"),
  array("hostname:/home/user/My Pics.gz/", "Pics.gz"),
  array("hostname:/home/user/My Pics.gz/", ".gz"),
  array("hostname:/home/user/My Pics.gz/"),
  array("hostname:/home/user/My Pics.gz/", NULL),
  array("hostname:/home/user/My Pics.gz/", ' '),
  array("hostname:/home/user/My Pics.gz/", ''),
  array("hostname:/home/user/My Pics.gz/", "My Pics.gz"),

  /* paths with numeirc strings */
  array("10.5"),
  array("10.5", ".5"),
  array("10.5", "10.5"),
  array("10"),
  array("105", "5"),
  array("/10.5"),
  array("10.5/"),
  array("10/10.gz"),
  array("0"),
  array('0'),

  /* paths and suffix given as same */
  array("bar.gz", "bar.gz"),
  array("/bar.gz", "/bar.gz"),
  array("/bar.gz/", "/bar.gz/"),
  array(" ", " "),
  array(' ', ' '),
  array(NULL, NULL),

  /* path with spaces */
  array(" "),
  array(' '),
  
  /* empty paths */
  array(""),
  array(''),
  array(NULL)
);

function check_basename( $path_arrays ) {
   $loop_counter = 1;
   foreach ($path_arrays as $path) {
     echo "\n--Iteration $loop_counter--\n"; $loop_counter++;
     if( 1 == count($path) ) { // no suffix provided
       var_dump( basename($path[0]) );
     } else { // path as well as suffix provided,
       var_dump( basename($path[0], $path[1]) );    
     } 
   }
}

echo "*** Testing basic operations ***\n";
check_basename( $file_paths );

echo "\n*** Testing possible variations in path and suffix ***\n";
check_basename( $file_path_variations );

echo "\n*** Testing error conditions ***\n";
// zero arguments 
var_dump( basename() );

// more than expected no. of arguments
var_dump( basename("/var/tmp/bar.gz", ".gz", ".gz") );

// passing invalid type arguments 
$object = new stdclass;
var_dump( basename( array("string/bar") ) );
var_dump( basename( array("string/bar"), "bar" ) );
var_dump( basename( "bar", array("string/bar") ) );
var_dump( basename( $object, "bar" ) );
var_dump( basename( $object ) );
var_dump( basename( $object, $object ) );
var_dump( basename( "bar", $object ) );

echo "Done\n";
?>
--EXPECTF--
*** Testing basic operations ***

--Iteration 1--
unicode(3) "bar"

--Iteration 2--
unicode(3) "bar"

--Iteration 3--
unicode(3) "bar"

--Iteration 4--
unicode(3) "bar"

--Iteration 5--
unicode(3) "bar"

--Iteration 6--
unicode(3) "bar"

--Iteration 7--
unicode(3) "bar"

--Iteration 8--
unicode(3) "bar"

--Iteration 9--
unicode(3) "bar"

--Iteration 10--
unicode(3) "bar"

--Iteration 11--
unicode(6) "bar.gz"

--Iteration 12--
unicode(3) "bar"

--Iteration 13--
unicode(3) "bar"

--Iteration 14--
unicode(3) "bar"

--Iteration 15--
unicode(3) "bar"

--Iteration 16--
unicode(3) "bar"

--Iteration 17--
unicode(3) "bar"

--Iteration 18--
unicode(3) "bar"

--Iteration 19--
unicode(3) "bar"

--Iteration 20--
unicode(3) ".gz"

--Iteration 21--
unicode(3) ".gz"

--Iteration 22--
unicode(3) ".gz"

--Iteration 23--
unicode(3) ".gz"

--Iteration 24--
unicode(3) ".gz"

--Iteration 25--
unicode(3) ".gz"

--Iteration 26--
unicode(7) "foo bar"

--Iteration 27--
unicode(7) "foo bar"

--Iteration 28--
unicode(7) "foo bar"

--Iteration 29--
unicode(7) "foo bar"

--Iteration 30--
unicode(4) "test"

--Iteration 31--
unicode(3) "bar"

--Iteration 32--
unicode(6) "bar.gz"

*** Testing possible variations in path and suffix ***

--Iteration 1--
unicode(3) "bar"

--Iteration 2--
unicode(3) "bar"

--Iteration 3--
unicode(3) "bar"

--Iteration 4--
unicode(3) "bar"

--Iteration 5--
unicode(3) "bar"

--Iteration 6--
unicode(7) "bar.tar"

--Iteration 7--
unicode(7) "bar.tar"

--Iteration 8--
unicode(7) "bar.tar"

--Iteration 9--
unicode(7) "bar.tar"

--Iteration 10--
unicode(7) "bar.tar"

--Iteration 11--
unicode(7) "bar.tar"

--Iteration 12--
unicode(7) "bar.tar"

--Iteration 13--
unicode(7) "bar.tar"

--Iteration 14--
unicode(1) "t"

--Iteration 15--
unicode(7) "tbar.gz"

--Iteration 16--
unicode(7) "tbar.gz"

--Iteration 17--
unicode(7) "tbar.gz"

--Iteration 18--
unicode(7) "tbar.gz"

--Iteration 19--
unicode(10) "My Pics.gz"

--Iteration 20--
unicode(10) "My Pics.gz"

--Iteration 21--
unicode(3) "My "

--Iteration 22--
unicode(7) "My Pics"

--Iteration 23--
unicode(10) "My Pics.gz"

--Iteration 24--
unicode(10) "My Pics.gz"

--Iteration 25--
unicode(10) "My Pics.gz"

--Iteration 26--
unicode(10) "My Pics.gz"

--Iteration 27--
unicode(10) "My Pics.gz"

--Iteration 28--
unicode(4) "10.5"

--Iteration 29--
unicode(2) "10"

--Iteration 30--
unicode(4) "10.5"

--Iteration 31--
unicode(2) "10"

--Iteration 32--
unicode(2) "10"

--Iteration 33--
unicode(4) "10.5"

--Iteration 34--
unicode(4) "10.5"

--Iteration 35--
unicode(5) "10.gz"

--Iteration 36--
unicode(1) "0"

--Iteration 37--
unicode(1) "0"

--Iteration 38--
unicode(6) "bar.gz"

--Iteration 39--
unicode(6) "bar.gz"

--Iteration 40--
unicode(6) "bar.gz"

--Iteration 41--
unicode(1) " "

--Iteration 42--
unicode(1) " "

--Iteration 43--
unicode(0) ""

--Iteration 44--
unicode(1) " "

--Iteration 45--
unicode(1) " "

--Iteration 46--
unicode(0) ""

--Iteration 47--
unicode(0) ""

--Iteration 48--
unicode(0) ""

*** Testing error conditions ***

Warning: basename() expects at least 1 parameter, 0 given in %s on line %d
NULL

Warning: basename() expects at most 2 parameters, 3 given in %s on line %d
NULL

Warning: basename() expects parameter 1 to be string (Unicode or binary), array given in %s on line %d
NULL

Warning: basename() expects parameter 1 to be string (Unicode or binary), array given in %s on line %d
NULL

Warning: basename() expects parameter 2 to be string (Unicode or binary), array given in %s on line %d
NULL

Warning: basename() expects parameter 1 to be string (Unicode or binary), object given in %s on line %d
NULL

Warning: basename() expects parameter 1 to be string (Unicode or binary), object given in %s on line %d
NULL

Warning: basename() expects parameter 1 to be string (Unicode or binary), object given in %s on line %d
NULL

Warning: basename() expects parameter 2 to be string (Unicode or binary), object given in %s on line %d
NULL
Done
