--TEST--
Bug #45705 test #2 (imap rfc822_parse_adrlist() modifies passed address parameter)
--SKIPIF--
<?php
	if (!extension_loaded("imap")) { 
		die("skip imap extension not available");  
	}
?>
--FILE--
<?php

$envelope = array('return_path' => 'John Doe <john@example.com>',
                  'from'        => 'John Doe <john@example.com>',
                  'reply_to'    => 'John Doe <john@example.com>',
                  'to'          => 'John Doe <john@example.com>',
                  'cc'          => 'John Doe <john@example.com>',
                  'bcc'         => 'John Doe <john@example.com>',
);

var_dump($envelope);
imap_mail_compose($envelope, array(1 => array()));
var_dump($envelope);

?>
--EXPECT--
array(6) {
  [u"return_path"]=>
  unicode(27) "John Doe <john@example.com>"
  [u"from"]=>
  unicode(27) "John Doe <john@example.com>"
  [u"reply_to"]=>
  unicode(27) "John Doe <john@example.com>"
  [u"to"]=>
  unicode(27) "John Doe <john@example.com>"
  [u"cc"]=>
  unicode(27) "John Doe <john@example.com>"
  [u"bcc"]=>
  unicode(27) "John Doe <john@example.com>"
}
array(6) {
  [u"return_path"]=>
  unicode(27) "John Doe <john@example.com>"
  [u"from"]=>
  unicode(27) "John Doe <john@example.com>"
  [u"reply_to"]=>
  unicode(27) "John Doe <john@example.com>"
  [u"to"]=>
  unicode(27) "John Doe <john@example.com>"
  [u"cc"]=>
  unicode(27) "John Doe <john@example.com>"
  [u"bcc"]=>
  unicode(27) "John Doe <john@example.com>"
}
