<?php

/*
*
* NOTE: A valid API key is needed in order to run these examples.
* Please go to http://www.okhub.org/ and request an API key. Once you get it, set the $valid_api_key variable with it.
*
*/

/*
* STEP 1: The IdsApiWrapper class definition is included.
*/
require_once('okhubwrapper.wrapper.inc');
/*
* STEP 2: A new OkhubApiWrapper object is created.
*/
$okhubapi = new OkhubApiWrapper;

/*your api key*/
$valid_api_key = '77d11dd5-c3c5-4e27-b3f4-8227bccbed01';

header('Content-Type: text/html');

echo "<b>A new wrapper object is created</b>\n";
echo '<pre>';
echo '$okhubapi = new OkhubApiWrapper;';
echo '</pre>';
echo '<hr>';


/*
* Example 1: Get titles, with filters.
*/
echo "<b>Example 1:</b> Get an array, indexed by object_id, with the titles of Okhub documents about poverty in the Philippines.\n\n";
echo '<pre>';
echo '$response = $okhubapi->search(\'themes\', \'hub\', $valid_api_key, \'short\', 0, 0, array(\'q\' => \'Poverty\', \'country\' => \'Philippines\'))' . "\n";
$response = $okhubapi->search('themes', 'hub', $valid_api_key,'short');
echo '$response->getArrayTitles(): ';
print_r($response);
echo '</pre>';
echo '<hr>';

//$okhubapi->cacheFlush();

