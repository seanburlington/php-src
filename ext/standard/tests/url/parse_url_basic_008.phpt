--TEST--
Test parse_url() function: Parse a load of URLs without specifying PHP_URL_QUERY as the URL component 
--FILE--
<?php
/* Prototype  : proto mixed parse_url(string url, [int url_component])
 * Description: Parse a URL and return its components 
 * Source code: ext/standard/url.c
 * Alias to functions: 
 */

/*
 * Parse a load of URLs without specifying PHP_URL_QUERY as the URL component
 */
include_once(dirname(__FILE__) . '/urls.inc');

foreach ($urls as $url) {
	echo "--> $url   : ";
	var_dump(parse_url($url, PHP_URL_QUERY));
}

echo "Done";
?>
--EXPECTF--
--> 64.246.30.37   : NULL
--> http://64.246.30.37   : NULL
--> http://64.246.30.37/   : NULL
--> 64.246.30.37/   : NULL
--> 64.246.30.37:80/   : NULL
--> php.net   : NULL
--> php.net/   : NULL
--> http://php.net   : NULL
--> http://php.net/   : NULL
--> www.php.net   : NULL
--> www.php.net/   : NULL
--> http://www.php.net   : NULL
--> http://www.php.net/   : NULL
--> www.php.net:80   : NULL
--> http://www.php.net:80   : NULL
--> http://www.php.net:80/   : NULL
--> http://www.php.net/index.php   : NULL
--> www.php.net/?   : NULL
--> www.php.net:80/?   : NULL
--> http://www.php.net/?   : NULL
--> http://www.php.net:80/?   : NULL
--> http://www.php.net:80/index.php   : NULL
--> http://www.php.net:80/foo/bar/index.php   : NULL
--> http://www.php.net:80/this/is/a/very/deep/directory/structure/and/file.php   : NULL
--> http://www.php.net:80/this/is/a/very/deep/directory/structure/and/file.php?lots=1&of=2&parameters=3&too=4&here=5   : unicode(37) "lots=1&of=2&parameters=3&too=4&here=5"
--> http://www.php.net:80/this/is/a/very/deep/directory/structure/and/   : NULL
--> http://www.php.net:80/this/is/a/very/deep/directory/structure/and/file.php   : NULL
--> http://www.php.net:80/this/../a/../deep/directory   : NULL
--> http://www.php.net:80/this/../a/../deep/directory/   : NULL
--> http://www.php.net:80/this/is/a/very/deep/directory/../file.php   : NULL
--> http://www.php.net:80/index.php   : NULL
--> http://www.php.net:80/index.php?   : NULL
--> http://www.php.net:80/#foo   : NULL
--> http://www.php.net:80/?#   : NULL
--> http://www.php.net:80/?test=1   : unicode(6) "test=1"
--> http://www.php.net/?test=1&   : unicode(7) "test=1&"
--> http://www.php.net:80/?&   : unicode(1) "&"
--> http://www.php.net:80/index.php?test=1&   : unicode(7) "test=1&"
--> http://www.php.net/index.php?&   : unicode(1) "&"
--> http://www.php.net:80/index.php?foo&   : unicode(4) "foo&"
--> http://www.php.net/index.php?&foo   : unicode(4) "&foo"
--> http://www.php.net:80/index.php?test=1&test2=char&test3=mixesCI   : unicode(31) "test=1&test2=char&test3=mixesCI"
--> www.php.net:80/index.php?test=1&test2=char&test3=mixesCI#some_page_ref123   : unicode(31) "test=1&test2=char&test3=mixesCI"
--> http://secret@www.php.net:80/index.php?test=1&test2=char&test3=mixesCI#some_page_ref123   : unicode(31) "test=1&test2=char&test3=mixesCI"
--> http://secret:@www.php.net/index.php?test=1&test2=char&test3=mixesCI#some_page_ref123   : unicode(31) "test=1&test2=char&test3=mixesCI"
--> http://:hideout@www.php.net:80/index.php?test=1&test2=char&test3=mixesCI#some_page_ref123   : unicode(31) "test=1&test2=char&test3=mixesCI"
--> http://secret:hideout@www.php.net/index.php?test=1&test2=char&test3=mixesCI#some_page_ref123   : unicode(31) "test=1&test2=char&test3=mixesCI"
--> http://secret@hideout@www.php.net:80/index.php?test=1&test2=char&test3=mixesCI#some_page_ref123   : unicode(31) "test=1&test2=char&test3=mixesCI"
--> http://secret:hid:out@www.php.net:80/index.php?test=1&test2=char&test3=mixesCI#some_page_ref123   : unicode(31) "test=1&test2=char&test3=mixesCI"
--> nntp://news.php.net   : NULL
--> ftp://ftp.gnu.org/gnu/glic/glibc.tar.gz   : NULL
--> zlib:http://foo@bar   : NULL
--> zlib:filename.txt   : NULL
--> zlib:/path/to/my/file/file.txt   : NULL
--> foo://foo@bar   : NULL
--> mailto:me@mydomain.com   : NULL
--> /foo.php?a=b&c=d   : unicode(7) "a=b&c=d"
--> foo.php?a=b&c=d   : unicode(7) "a=b&c=d"
--> http://user:passwd@www.example.com:8080?bar=1&boom=0   : unicode(12) "bar=1&boom=0"
--> file:///path/to/file   : NULL
--> file://path/to/file   : NULL
--> file:/path/to/file   : NULL
--> http://1.2.3.4:/abc.asp?a=1&b=2   : unicode(7) "a=1&b=2"
--> http://foo.com#bar   : NULL
--> scheme:   : NULL
--> foo+bar://baz@bang/bla   : NULL
--> gg:9130731   : NULL
--> http://user:@pass@host/path?argument?value#etc   : unicode(14) "argument?value"
--> http://10.10.10.10/:80   : NULL
--> http://x:?   : NULL
--> x:blah.com   : NULL
--> x:/blah.com   : NULL
--> x://::abc/?   : NULL
--> http://::?   : NULL
--> x://::6.5   : NULL
--> http://?:/   : NULL
--> http://@?:/   : NULL
--> file:///:   : NULL
--> file:///a:/   : NULL
--> file:///ab:/   : NULL
--> file:///a:/   : NULL
--> file:///@:/   : NULL
--> file:///:80/   : NULL
--> []   : NULL
--> http://[x:80]/   : NULL
-->    : NULL
--> /   : NULL
--> http:///blah.com   : 
Warning: parse_url(http:///blah.com): Unable to parse URL in %s on line 15
bool(false)
--> http://:80   : 
Warning: parse_url(http://:80): Unable to parse URL in %s on line 15
bool(false)
--> http://user@:80   : 
Warning: parse_url(http://user@:80): Unable to parse URL in %s on line 15
bool(false)
--> http://user:pass@:80   : 
Warning: parse_url(http://user:pass@:80): Unable to parse URL in %s on line 15
bool(false)
--> http://:   : 
Warning: parse_url(http://:): Unable to parse URL in %s on line 15
bool(false)
--> http://@/   : 
Warning: parse_url(http://@/): Unable to parse URL in %s on line 15
bool(false)
--> http://@:/   : 
Warning: parse_url(http://@:/): Unable to parse URL in %s on line 15
bool(false)
--> http://:/   : 
Warning: parse_url(http://:/): Unable to parse URL in %s on line 15
bool(false)
--> http://?   : 
Warning: parse_url(http://?): Unable to parse URL in %s on line 15
bool(false)
--> http://?:   : 
Warning: parse_url(http://?:): Unable to parse URL in %s on line 15
bool(false)
--> http://:?   : 
Warning: parse_url(http://:?): Unable to parse URL in %s on line 15
bool(false)
--> http://blah.com:123456   : 
Warning: parse_url(http://blah.com:123456): Unable to parse URL in %s on line 15
bool(false)
--> http://blah.com:abcdef   : 
Warning: parse_url(http://blah.com:abcdef): Unable to parse URL in %s on line 15
bool(false)
Done
