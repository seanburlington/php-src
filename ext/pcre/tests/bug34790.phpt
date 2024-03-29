--TEST--
Bug #34790 (preg_match_all(), named capturing groups, variable assignment/return => crash)
--FILE--
<?php
function func1(){
        $string = 'what the word and the other word the';
        preg_match_all('/(?P<word>the)/', $string, $matches);
        return $matches['word'];
}
$words = func1();
var_dump($words);
?>
--EXPECT--
array(4) {
  [0]=>
  unicode(3) "the"
  [1]=>
  unicode(3) "the"
  [2]=>
  unicode(3) "the"
  [3]=>
  unicode(3) "the"
}
