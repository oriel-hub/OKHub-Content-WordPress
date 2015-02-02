<?php
// $Id;

/**
 * @file
 *   OKHUB Wordpress Import Plugin default parameters.
 */

// Environment in which we are using the API wrapper. Possible values: 'generic', 'wordpress', 'drupal'. Default is 'generic'.
if (!defined('OKHUB_API_LIBRARY_PATH')) define('OKHUB_API_LIBRARY_PATH', 'okhubapi/');
if (!defined('OKHUB_API_ENVIRONMENT')) define('OKHUB_API_ENVIRONMENT', 'wordpress');

define('OKHUB_IMPORT_DEFAULT_DATASET_PUBLIC', 'hub');
define('OKHUB_IMPORT_DEFAULT_DATASET_ADMIN', 'hub');
define('OKHUB_IMPORT_IMPORT_USER', 'OKHUB');
define('OKHUB_IMPORT_CATEGORY_LEVELS', 2);
define('OKHUB_IMPORT_IMPORT_RECURRENCE_ASSETS', 'daily');
define('OKHUB_IMPORT_IMPORT_RECURRENCE_CATEGORIES', 'monthly');
define('OKHUB_IMPORT_USER_EMAIL','api@okhub.org');
define('OKHUB_IMPORT_MENU_TITLE', 'OKHUB Import');
// This might be defined in IDS Expose, too.
if (!defined('OKHUB_IMPORT_TAXONOMY')) define('OKHUB_IMPORT_TAXONOMY', 'okhubimport_taxonomy');

define('OKHUB_IMPORT_DEFAULT_LANGUAGE', 'en');
define('OKHUB_IMPORT_UPDATED_ASSETS', 'keep');

define('OKHUB_IMPORT_NEW_CATEGORIES', TRUE);
define('OKHUB_IMPORT_MAP_CATEGORIES', FALSE);
define('OKHUB_IMPORT_VERBOSE_IMPORT', TRUE);
define('OKHUB_IMPORT_INCLUDE_IMPORTED_DOCUMENTS', TRUE);
define('OKHUB_IMPORT_INCLUDE_IMPORTED_ORGANISATIONS', TRUE);
define('OKHUB_IMPORT_TAGS', TRUE);



