--TEST--
ReflectionMethod::getDocComment() uses wrong comment block
--FILE--
<?php

function strip_doc_comment($c)
{
	if (!strlen($c) || $c === false) return $c;
	return trim(substr($c, 3, -2));
}

/** Comment for class A */
class A
{
	/** Method A::bla()
	 */
	function bla()
	{
	}

    function foo() {
        /**
        * This is a valid comment inside a method
        */
    }

    function bar() {
        // I don't have a doc comment....
    }

    /**
     * Comment for A::baz()
     */
    function baz() {
    }
}

$r = new ReflectionClass('A');
var_dump(strip_doc_comment($r->getDocComment()));

foreach($r->getMethods() as $m)
{
	var_dump(strip_doc_comment($m->getDocComment()));
}

?>
===DONE===
--EXPECT--
unicode(19) "Comment for class A"
unicode(15) "Method A::bla()"
bool(false)
bool(false)
unicode(22) "* Comment for A::baz()"
===DONE===
