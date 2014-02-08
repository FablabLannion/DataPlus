<?php
date_default_timezone_set ('Europe/Paris');

require_once "simple_html_dom.php";

$html = new simple_html_dom();
$html->load_file("http://mer.meteoconsult.fr/meteo-marine/fr/high/C%F4tes-d%27Armor/bulletin-meteo-port-Perros-Guirec+%2822700%29-234-23-0.php");
// $html->load_file("test.html");
$a=$html->find('table',8);
// get coeficient
$a = preg_split("/[\s,]+/", $html->find('table',8)->last_child()->last_child()->children(0)->children(0)->children(3)->plaintext );
// init output with coeficient
$tide = $a[2];
// get tide hours, high split into an array
$a = preg_split("/[\s,]+/", str_replace('&nbsp;',' ',trim ($html->find('table',9)->plaintext) ) ) ;
foreach ($a as $e) {
   if (preg_match ('/\d{2}h\d{2}/', $e)) {
//       echo "heure $e\n";
      $ts = mktime ((int)substr($e,0,2), (int)substr($e,-2) );
      $tide .= ";$ts";
   } elseif (preg_match ('/(\d+\.\d+)m/', $e, $matches)) {
//       echo "hauteur $e\n";
//       print_r($matches);
      $tide .= ";$matches[1]";
   }
}
echo "$tide\n";
// print_r($a);
?>
