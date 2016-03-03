<?php
// $Id;

/**
 * @file
 *   OKHub Wordpress Import Plugin default parameters.
 */

// Environment in which we are using the API wrapper. Possible values: 'generic', 'wordpress', 'drupal'. Default is 'generic'.
if (!defined('OKHUB_API_ENVIRONMENT')) define('OKHUB_API_ENVIRONMENT', 'wordpress');

// This might be defined in OKHub Expose, too.
if (!defined('OKHUB_TAXONOMY')) define('OKHUB_TAXONOMY', 'okhub_taxonomy');

define('OKHUB_TAGS_TAXONOMY', 'okhub_tags');
define('OKHUB_HIERARCHICAL_HUB_TYPES', 'themes,subjects');

define('OKHUB_HUB_SOURCE_CODE', 'hub'); // This is defined in the wrapper as OKHUB_API_FN_SITE, but the name is misleading and might change.
define('OKHUB_OKHUB_SOURCE_CODE', 'okhub');
define('OKHUB_CUSTOM_PREFIX', 'okhub'); // Prefix to use in the name of custom types and taxonomies.
define('OKHUB_DATAHUB_URL', 'http://data.okhub.org');

define('OKHUB_API_SEARCH_DOCUMENTATION', 'http://developer.okhub.org/hub-api-documentation/api-explorer/');

// Related items (themes, regions, publishers, etc) in API.
if (!defined('OKHUB_API_FN_REGION_ARRAY')) define('OKHUB_API_FN_REGION_ARRAY', 'region_array');
if (!defined('OKHUB_API_FN_THEME_ARRAY')) define('OKHUB_API_FN_THEME_ARRAY', 'theme_array');
if (!defined('OKHUB_API_FN_COUNTRY_FOCUS_ARRAY')) define('OKHUB_API_FN_COUNTRY_FOCUS_ARRAY', 'country_focus_array');
if (!defined('OKHUB_API_FN_DOCUMENT_TYPE_ARRAY')) define('OKHUB_API_FN_DOCUMENT_TYPE_ARRAY', 'document_type_array');
if (!defined('OKHUB_API_FN_PUBLISHER_ARRAY')) define('OKHUB_API_FN_PUBLISHER_ARRAY', 'publisher_array');
if (!defined('OKHUB_API_FN_PUBLISHER_ARRAY')) define('OKHUB_API_FN_DOCUMENT_TYPE_ARRAY', 'document_type_array');
if (!defined('OKHUB_API_SUFFIX_PARENT_ARRAY')) define('OKHUB_API_SUFFIX_PARENT_ARRAY', 'broader_array');
if (!defined('OKHUB_API_SUFFIX_CHILDREN_ARRAY')) define('OKHUB_API_SUFFIX_CHILDREN_ARRAY', 'narrower_array');

// API filters / parameters
define('OKHUB_API_PARAMETER_SOURCES', 'source');
define('OKHUB_API_PARAMETER_LANGUAGES', 'lang');
define('OKHUB_API_PARAMETER_ONLY_LANGUAGE', 'lang_only'); // Works only with one language code (can't use |).
define('OKHUB_API_PARAMETER_TOP_LEVEL', 'top_level');
define('OKHUB_API_PARAMETER_COUNTRY_NAME', 'country');
define('OKHUB_API_PARAMETER_COUNTRY_ID', 'country_id');
define('OKHUB_API_PARAMETER_THEME_NAME', 'theme');
define('OKHUB_API_PARAMETER_THEME_ID', 'theme_id');
define('OKHUB_API_PARAMETER_SUBJECT_NAME', 'subject');
define('OKHUB_API_PARAMETER_SUBJECT_ID', 'subject_id');
define('OKHUB_API_PARAMETER_REGION_NAME', 'region');
define('OKHUB_API_PARAMETER_REGION_ID', 'region_id');
define('OKHUB_API_PARAMETER_PUBLISHER_NAME', 'publisher');
define('OKHUB_API_PARAMETER_PUBLISHER_ID', 'publisher_id');
define('OKHUB_API_PARAMETER_PUBLISHER_COUNTRY_NAME', 'publisher_country'); // Not available for all sources. Includes both the 'publisher_country' field value in Eldis/BRIDGE and the related item's content when the relationship is defined in the OKHub.
define('OKHUB_API_PARAMETER_PUBLISHER_COUNTRY_ID', 'publisher_country_id'); // Only available for a few sources where the mapping is defined in the OKHub.
define('OKHUB_API_PARAMETER_DOCUMENT_TYPE_NAME', 'document_type');
define('OKHUB_API_PARAMETER_DOCUMENT_TYPE_ID', 'document_type_id');
define('OKHUB_API_PARAMETER_AUTHOR', 'author');
define('OKHUB_API_PARAMETER_GENERIC_SEARCH', 'q');


// Additional fields retrieved from the API.
if (!defined('OKHUB_API_FN_ACRONYM')) define('OKHUB_API_FN_ACRONYM', 'acronym');
if (!defined('OKHUB_API_FN_ALTERNATIVE_ACRONYM')) define('OKHUB_API_FN_ALTERNATIVE_ACRONYM', 'alternative_acronym');
if (!defined('OKHUB_API_FN_ALTERNATIVE_NAME')) define('OKHUB_API_FN_ALTERNATIVE_NAME', 'alternative_name');
if (!defined('OKHUB_API_FN_AUTHORS')) define('OKHUB_API_FN_AUTHORS', 'author');
if (!defined('OKHUB_API_FN_CAT_PARENT')) define('OKHUB_API_FN_CAT_PARENT', 'cat_parent'); // This is for Eldis/BRIDGE. The narrower/broader arrays should be used.
if (!defined('OKHUB_API_FN_CAT_SUPERPARENT')) define('OKHUB_API_FN_CAT_SUPERPARENT', 'cat_superparent'); // This is for Eldis/BRIDGE. It should be retrieved in general.
if (!defined('OKHUB_API_FN_INDEX_UPDATED')) define('OKHUB_API_FN_INDEX_UPDATED', 'timestamp');
if (!defined('OKHUB_API_FN_ISO_NUMBER')) define('OKHUB_API_FN_ISO_NUMBER', 'iso_number');
if (!defined('OKHUB_API_FN_ISO_THREE_LETTER_CODE')) define('OKHUB_API_FN_ISO_THREE_LETTER_CODE', 'iso_three_letter_code');
if (!defined('OKHUB_API_FN_ISO_TWO_LETTER_CODE')) define('OKHUB_API_FN_ISO_TWO_LETTER_CODE', 'iso_two_letter_code');
if (!defined('OKHUB_API_FN_KEYWORDS')) define('OKHUB_API_FN_KEYWORDS', 'keyword');
if (!defined('OKHUB_API_FN_LANGUAGE_NAME')) define('OKHUB_API_FN_LANGUAGE_NAME', 'language_name'); // This is available for Eldis/BRIDGE. The hub_metadata_languages fields should be used instead for all sources (now it's broken in the API).
if (!defined('OKHUB_API_FN_LEVEL')) define('OKHUB_API_FN_LEVEL', 'level');
if (!defined('OKHUB_API_FN_LICENCE_TYPE')) define('OKHUB_API_FN_LICENCE_TYPE', 'licence_type');
if (!defined('OKHUB_API_FN_LOCATION_COUNTRY')) define('OKHUB_API_FN_LOCATION_COUNTRY', 'country');
if (!defined('OKHUB_API_FN_NAME')) define('OKHUB_API_FN_NAME', 'name');
if (!defined('OKHUB_API_FN_OBJECT_ID')) define('OKHUB_API_FN_OBJECT_ID', 'object_id'); // Source's object ids like Eldis' of BRIDGE's.
if (!defined('OKHUB_API_FN_OBJECT_NAME')) define('OKHUB_API_FN_OBJECT_NAME', 'object_name');
if (!defined('OKHUB_API_FN_ORGANISATION_TYPE')) define('OKHUB_API_FN_ORGANISATION_TYPE', 'organisation_type');
if (!defined('OKHUB_API_FN_ORGANISATION_ROLE')) define('OKHUB_API_FN_ORGANISATION_ROLE', 'role');
if (!defined('OKHUB_API_FN_ORGANISATION_URL')) define('OKHUB_API_FN_ORGANISATION_URL', 'organisation_url');
if (!defined('OKHUB_API_FN_PARENT_OBJECT_ID')) define('OKHUB_API_FN_PARENT_OBJECT_ID', 'parent_object_id');
if (!defined('OKHUB_API_FN_PUBLICATION_DATE')) define('OKHUB_API_FN_PUBLICATION_DATE', 'publication_date');
if (!defined('OKHUB_API_FN_PUBLICATION_YEAR')) define('OKHUB_API_FN_PUBLICATION_YEAR', 'publication_year');
if (!defined('OKHUB_API_FN_PUBLISHER')) define('OKHUB_API_FN_PUBLISHER', 'publisher'); // Publisher name present in sources like BRIDGE and Eldis. Use OKHUB_API_FN_ORGANISATION for the relationships to items of type Organisation in the hub.
if (!defined('OKHUB_API_FN_PUBLISHER_COUNTRY')) define('OKHUB_API_FN_PUBLISHER_COUNTRY', 'publisher_country');
if (!defined('OKHUB_API_FN_TOP_LEVEL')) define('OKHUB_API_FN_TOP_LEVEL', 'top_level');
// Source fields
if (!defined('OKHUB_API_FN_SOURCE_CODE')) define('OKHUB_API_FN_SOURCE_CODE', 'code');
if (!defined('OKHUB_API_FN_SOURCE_NAME')) define('OKHUB_API_FN_SOURCE_NAME', 'title');
if (!defined('OKHUB_API_FN_SOURCE_CONTACT_NAME')) define('OKHUB_API_FN_SOURCE_CONTACT_EMAIL', 'contact_email');
if (!defined('OKHUB_API_FN_SOURCE_CONTACT_ROLE')) define('OKHUB_API_FN_SOURCE_CONTACT_ROLE', 'contact_role');
if (!defined('OKHUB_API_FN_SOURCE_DESCRIPTION')) define('OKHUB_API_FN_SOURCE_DESCRIPTION', 'description');
if (!defined('OKHUB_API_FN_SOURCE_EDITORIAL_POLICY')) define('OKHUB_API_FN_SOURCE_EDITORIAL_POLICY', 'editorial_policy'); // Link, no language.
if (!defined('OKHUB_API_FN_SOURCE_LICENSE_DESCRIPTION')) define('OKHUB_API_FN_SOURCE_LICENSE_DESCRIPTION', 'licence_description');
if (!defined('OKHUB_API_FN_SOURCE_LOGO')) define('OKHUB_API_FN_SOURCE_LOGO', 'logo');
if (!defined('OKHUB_API_FN_SOURCE_WEBSITE')) define('OKHUB_API_FN_SOURCE_WEBSITE', 'website');








