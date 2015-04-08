<?php

/**
* The following functions and variable provide an abstraction level so their implementation can be changed depending on the environment.
* For instance, in Drupal, we would use its internal caching mechanism instead (see cms/idsapi.drupal.inc).
*/

require_once('okhubwrapper.cache.inc');

global $okhubapi_cache;
$idsapi_cache = new OkhubApiCache(array('repository' => dirname(__FILE__) . '/' . OKHUB_API_CACHE_DIR));

/**
* Add data to the cache.
* before its idsapi
*/
function okhubapi_cache_set($cache_id, $data, $bin, $expire) {
  global $okhubapi_cache;
  $idsapi_cache->store($cache_id, $data, $expire, $bin);
}

/**
* Get data from the cache.
*/
function okhubapi_cache_get($cache_id, $bin) {
  global $okhubapi_cache;
  $ret = OKHUB_API_USE_CACHE;
  if ($ret) {
    $ret = $okhubapi_cache->get($cache_id);
  }
  return $ret;
}

/**
* Clear data from one cache id.
*/
function okhubapi_cache_clear($cache_id) {
  global $okhubapi_cache;
  $okhubapi_cache->clear($cache_id);
}

/**
* Flush data from the cache.
*/
function okhubapi_cache_flush() {
  global $okhubapi_cache;
  $okhubapi_cache->clear_all();
}

