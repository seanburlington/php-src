--TEST--
Test serialize() & unserialize() functions: objects (variations)
--FILE--
<?php 
/* Prototype  : proto string serialize(mixed variable)
 * Description: Returns a string representation of variable (which can later be unserialized) 
 * Source code: ext/standard/var.c
 * Alias to functions: 
 */
/* Prototype  : proto mixed unserialize(string variable_representation)
 * Description: Takes a string representation of variable and recreates it 
 * Source code: ext/standard/var.c
 * Alias to functions: 
 */

echo "\n--- Testing Variations in objects ---\n";

class members 
{
  private $var_private = 10;
  protected $var_protected = "string";
  public $var_public = array(-100.123, "string", TRUE);
}

class nomembers { }

class C {
  var $a, $b, $c, $d, $e, $f, $g, $h;
  function __construct() {
    $this->a = 10;
    $this->b = "string";
    $this->c = TRUE;
    $this->d = -2.34444;
    $this->e = array(1, 2.22, "string", TRUE, array(), 
                     new members(), null);
    $this->f = new nomembers();
    $this->g = $GLOBALS['file_handle'];
    $this->h = NULL;
  }
}

class D extends C {
  function __construct( $w, $x, $y, $z ) {
    $this->a = $w;
    $this->b = $x;
    $this->c = $y;
    $this->d = $z;
  }
}

$variation_obj_arr = array(
  new C(),
  new D( 1, 2, 3333, 444444 ),
  new D( .5, 0.005, -1.345, 10.005e5 ),
  new D( TRUE, true, FALSE, false ),
  new D( "a", 'a', "string", 'string' ),
  new D( array(), 
         array(1, 2.222, TRUE, FALSE, "string"), 
         array(new nomembers(), $file_handle, NULL, ""),
         array(array(1,2,3,array()))
       ),
  new D( NULL, null, "", "\0" ),
  new D( new members, new nomembers, $file_handle, NULL),
);   

/* Testing serialization on all the objects through loop */
foreach( $variation_obj_arr as $object) {

  echo "After Serialization => ";
  $serialize_data = serialize( $object );
  var_dump( $serialize_data );
 
  echo "After Unserialization => ";
  $unserialize_data = unserialize( $serialize_data );
  var_dump( $unserialize_data );
}

echo "\nDone";
?>
--EXPECTF--
--- Testing Variations in objects ---

Notice: Undefined index: file_handle in %s on line 34

Notice: Undefined variable: file_handle in %s on line 56

Notice: Undefined variable: file_handle in %s on line 60
After Serialization => unicode(493) "O:1:"C":8:{U:1:"a";i:10;U:1:"b";U:6:"string";U:1:"c";b:1;U:1:"d";d:-2.344440000000000079438677857979200780391693115234375;U:1:"e";a:7:{i:0;i:1;i:1;d:2.220000000000000195399252334027551114559173583984375;i:2;U:6:"string";i:3;b:1;i:4;a:0:{}i:5;O:7:"members":3:{U:20:" members var_private";i:10;U:16:" * var_protected";U:6:"string";U:10:"var_public";a:3:{i:0;d:-100.1230000000000046611603465862572193145751953125;i:1;U:6:"string";i:2;b:1;}}i:6;N;}U:1:"f";O:9:"nomembers":0:{}U:1:"g";N;U:1:"h";N;}"
After Unserialization => object(C)#%d (8) {
  [u"a"]=>
  int(10)
  [u"b"]=>
  unicode(6) "string"
  [u"c"]=>
  bool(true)
  [u"d"]=>
  float(-2.34444)
  [u"e"]=>
  array(7) {
    [0]=>
    int(1)
    [1]=>
    float(2.22)
    [2]=>
    unicode(6) "string"
    [3]=>
    bool(true)
    [4]=>
    array(0) {
    }
    [5]=>
    object(members)#%d (3) {
      [u"var_private":u"members":private]=>
      int(10)
      [u"var_protected":protected]=>
      unicode(6) "string"
      [u"var_public"]=>
      array(3) {
        [0]=>
        float(-100.123)
        [1]=>
        unicode(6) "string"
        [2]=>
        bool(true)
      }
    }
    [6]=>
    NULL
  }
  [u"f"]=>
  object(nomembers)#%d (0) {
  }
  [u"g"]=>
  NULL
  [u"h"]=>
  NULL
}
After Serialization => unicode(108) "O:1:"D":8:{U:1:"a";i:1;U:1:"b";i:2;U:1:"c";i:3333;U:1:"d";i:444444;U:1:"e";N;U:1:"f";N;U:1:"g";N;U:1:"h";N;}"
After Unserialization => object(D)#%d (8) {
  [u"a"]=>
  int(1)
  [u"b"]=>
  int(2)
  [u"c"]=>
  int(3333)
  [u"d"]=>
  int(444444)
  [u"e"]=>
  NULL
  [u"f"]=>
  NULL
  [u"g"]=>
  NULL
  [u"h"]=>
  NULL
}
After Serialization => unicode(223) "O:1:"D":8:{U:1:"a";d:0.5;U:1:"b";d:0.005000000000000000104083408558608425664715468883514404296875;U:1:"c";d:-1.3449999999999999733546474089962430298328399658203125;U:1:"d";d:1000500;U:1:"e";N;U:1:"f";N;U:1:"g";N;U:1:"h";N;}"
After Unserialization => object(D)#%d (8) {
  [u"a"]=>
  float(0.5)
  [u"b"]=>
  float(0.005)
  [u"c"]=>
  float(-1.345)
  [u"d"]=>
  float(1000500)
  [u"e"]=>
  NULL
  [u"f"]=>
  NULL
  [u"g"]=>
  NULL
  [u"h"]=>
  NULL
}
After Serialization => unicode(100) "O:1:"D":8:{U:1:"a";b:1;U:1:"b";b:1;U:1:"c";b:0;U:1:"d";b:0;U:1:"e";N;U:1:"f";N;U:1:"g";N;U:1:"h";N;}"
After Unserialization => object(D)#%d (8) {
  [u"a"]=>
  bool(true)
  [u"b"]=>
  bool(true)
  [u"c"]=>
  bool(false)
  [u"d"]=>
  bool(false)
  [u"e"]=>
  NULL
  [u"f"]=>
  NULL
  [u"g"]=>
  NULL
  [u"h"]=>
  NULL
}
After Serialization => unicode(126) "O:1:"D":8:{U:1:"a";U:1:"a";U:1:"b";U:1:"a";U:1:"c";U:6:"string";U:1:"d";U:6:"string";U:1:"e";N;U:1:"f";N;U:1:"g";N;U:1:"h";N;}"
After Unserialization => object(D)#%d (8) {
  [u"a"]=>
  unicode(1) "a"
  [u"b"]=>
  unicode(1) "a"
  [u"c"]=>
  unicode(6) "string"
  [u"d"]=>
  unicode(6) "string"
  [u"e"]=>
  NULL
  [u"f"]=>
  NULL
  [u"g"]=>
  NULL
  [u"h"]=>
  NULL
}
After Serialization => unicode(300) "O:1:"D":8:{U:1:"a";a:0:{}U:1:"b";a:5:{i:0;i:1;i:1;d:2.221999999999999975131004248396493494510650634765625;i:2;b:1;i:3;b:0;i:4;U:6:"string";}U:1:"c";a:4:{i:0;O:9:"nomembers":0:{}i:1;N;i:2;N;i:3;U:0:"";}U:1:"d";a:1:{i:0;a:4:{i:0;i:1;i:1;i:2;i:2;i:3;i:3;a:0:{}}}U:1:"e";N;U:1:"f";N;U:1:"g";N;U:1:"h";N;}"
After Unserialization => object(D)#%d (8) {
  [u"a"]=>
  array(0) {
  }
  [u"b"]=>
  array(5) {
    [0]=>
    int(1)
    [1]=>
    float(2.222)
    [2]=>
    bool(true)
    [3]=>
    bool(false)
    [4]=>
    unicode(6) "string"
  }
  [u"c"]=>
  array(4) {
    [0]=>
    object(nomembers)#%d (0) {
    }
    [1]=>
    NULL
    [2]=>
    NULL
    [3]=>
    unicode(0) ""
  }
  [u"d"]=>
  array(1) {
    [0]=>
    array(4) {
      [0]=>
      int(1)
      [1]=>
      int(2)
      [2]=>
      int(3)
      [3]=>
      array(0) {
      }
    }
  }
  [u"e"]=>
  NULL
  [u"f"]=>
  NULL
  [u"g"]=>
  NULL
  [u"h"]=>
  NULL
}
After Serialization => unicode(103) "O:1:"D":8:{U:1:"a";N;U:1:"b";N;U:1:"c";U:0:"";U:1:"d";U:1:" ";U:1:"e";N;U:1:"f";N;U:1:"g";N;U:1:"h";N;}"
After Unserialization => object(D)#%d (8) {
  [u"a"]=>
  NULL
  [u"b"]=>
  NULL
  [u"c"]=>
  unicode(0) ""
  [u"d"]=>
  unicode(1) " "
  [u"e"]=>
  NULL
  [u"f"]=>
  NULL
  [u"g"]=>
  NULL
  [u"h"]=>
  NULL
}
After Serialization => unicode(303) "O:1:"D":8:{U:1:"a";O:7:"members":3:{U:20:" members var_private";i:10;U:16:" * var_protected";U:6:"string";U:10:"var_public";a:3:{i:0;d:-100.1230000000000046611603465862572193145751953125;i:1;U:6:"string";i:2;b:1;}}U:1:"b";O:9:"nomembers":0:{}U:1:"c";N;U:1:"d";N;U:1:"e";N;U:1:"f";N;U:1:"g";N;U:1:"h";N;}"
After Unserialization => object(D)#%d (8) {
  [u"a"]=>
  object(members)#%d (3) {
    [u"var_private":u"members":private]=>
    int(10)
    [u"var_protected":protected]=>
    unicode(6) "string"
    [u"var_public"]=>
    array(3) {
      [0]=>
      float(-100.123)
      [1]=>
      unicode(6) "string"
      [2]=>
      bool(true)
    }
  }
  [u"b"]=>
  object(nomembers)#%d (0) {
  }
  [u"c"]=>
  NULL
  [u"d"]=>
  NULL
  [u"e"]=>
  NULL
  [u"f"]=>
  NULL
  [u"g"]=>
  NULL
  [u"h"]=>
  NULL
}

Done
