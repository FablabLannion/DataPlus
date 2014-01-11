<?php
// Q&D parser for maree.info

$port = 68;

function err($str = "") {
   print ("err: $str\n");
   exit();
} // err

$fp = fopen ("http://horloge.maree.frbateaux.net/wd$port.js",'r');
// $fp = fopen ("/tmp/wd$port.js",'r');
if ($fp == FALSE) {
   err('fopen');
}
$page = fread ($fp, 4096);
if ($page == FALSE) {
   err('fread');
}
fclose($fp);

$ret = preg_match ( '/PostIt.*innerHTML="(.*)";/' , strip_tags($page), $arr);
if ($ret != 1) {
   err('no match');
}
// print_r ($arr);
print (substr ($arr[1], 0, 8) . "\n");
print (substr ($arr[1], -8)   . "\n");

?>
