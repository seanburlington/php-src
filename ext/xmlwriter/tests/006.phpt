--TEST--
XMLWriter: libxml2 XML Writer, startDTD/writeElementNS 
--SKIPIF--
<?php 
if (!extension_loaded("xmlwriter")) die("skip"); 
?>
--FILE--
<?php 
/* $Id: 006.phpt,v 1.2 2005/12/12 21:21:36 tony2001 Exp $ */

$doc_dest = '001.xml';
$xw = xmlwriter_open_uri($doc_dest);
xmlwriter_start_dtd($xw, 'foo', NULL, 'urn:bar');
xmlwriter_end_dtd($xw);
xmlwriter_start_element($xw, 'foo');
xmlwriter_write_element_ns($xw, 'foo', 'bar', 'urn:foo', 'dummy content');
xmlwriter_end_element($xw);

// Force to write and empty the buffer
$output_bytes = xmlwriter_flush($xw, true);
echo file_get_contents($doc_dest);
unset($xw);
unlink('001.xml');
?>
--EXPECT--
<!DOCTYPE foo SYSTEM "urn:bar"><foo><foo:bar xmlns:foo="urn:foo">dummy content</foo:bar></foo>
