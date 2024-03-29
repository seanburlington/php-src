--TEST--
SPL: SplPriorityQueue: var_dump
--FILE--
<?php
$pq = new SplPriorityQueue();

$pq->insert("a", 0);
$pq->insert("b", 1);
$pq->insert("c", 5);
$pq->insert("d", -2);

var_dump($pq);
?>
===DONE===
<?php exit(0); ?>
--EXPECTF--
object(SplPriorityQueue)#1 (3) {
  [u"flags":u"SplPriorityQueue":private]=>
  int(1)
  [u"isCorrupted":u"SplPriorityQueue":private]=>
  bool(false)
  [u"heap":u"SplPriorityQueue":private]=>
  array(4) {
    [0]=>
    array(2) {
      ["data"]=>
      unicode(1) "c"
      ["priority"]=>
      int(5)
    }
    [1]=>
    array(2) {
      ["data"]=>
      unicode(1) "a"
      ["priority"]=>
      int(0)
    }
    [2]=>
    array(2) {
      ["data"]=>
      unicode(1) "b"
      ["priority"]=>
      int(1)
    }
    [3]=>
    array(2) {
      ["data"]=>
      unicode(1) "d"
      ["priority"]=>
      int(-2)
    }
  }
}
===DONE===
