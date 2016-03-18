<?php
// $Id;

/**
 * @file
 *   OKHub Wordpress Import Plugin default user settings.
 */

/*------------------------------------- Default settings -----------------------------------*/

// Wordpress field names (meta keys) used when importing OKHub content into Wordpress.
// The name of these fields can be changed if needed.

/* ---- Hub fields ---- */
define('OKHUB_WP_FN_HUB_ITEM_ID', 'hub_item_id');
define('OKHUB_WP_FN_HUB_ITEM_TYPE', 'hub_item_type');
define('OKHUB_WP_FN_HUB_METADATA_URL', 'hub_metadata_url');
define('OKHUB_WP_FN_HUB_TIMESTAMP', 'hub_timestamp');
define('OKHUB_WP_FN_HUB_SOURCE', 'hub_source'); // For taxonomy terms only. This does not correspond to an API field.
define('OKHUB_WP_FN_HUB_TOP_LEVEL', 'hub_top_level');

/* ---- Sources' fields ---- */
define('OKHUB_WP_FN_OBJECT_ID', 'object_id');
define('OKHUB_WP_FN_WEBSITE_URL', 'website_url');
define('OKHUB_WP_FN_DATE_CREATED', 'date_created');
define('OKHUB_WP_FN_DATE_UPDATED', 'date_updated');
// Documents
define('OKHUB_WP_FN_AUTHORS', 'authors');
define('OKHUB_WP_FN_URLS', 'urls');
define('OKHUB_WP_FN_PUBLICATION_DATE', 'publication_date');
define('OKHUB_WP_FN_PUBLICATION_YEAR', 'publication_year');
define('OKHUB_WP_FN_LICENSE_TYPE', 'licence_type');
define('OKHUB_WP_FN_PUBLISHER', 'publisher');
define('OKHUB_WP_FN_DOCUMENT_TYPE', 'document_type');
define('OKHUB_WP_FN_PUBLISHER_COUNTRY', 'publisher_country');
define('OKHUB_WP_FN_LANGUAGE_NAME', 'language_name');
// Organisation
define('OKHUB_WP_FN_ACRONYM', 'acronym');
define('OKHUB_WP_FN_ALTERNATIVE_ACRONYM', 'alternative_acronym');
define('OKHUB_WP_FN_NAME', 'name');
define('OKHUB_WP_FN_ORGANISATION_TYPE', 'organisation_type');
define('OKHUB_WP_FN_ORGANISATION_URL', 'organisation_url');
define('OKHUB_WP_FN_ROLE', 'role');
define('OKHUB_WP_FN_LOCATION_COUNTRY', 'country');
// Country
define('OKHUB_WP_FN_ALTERNATIVE_NAME', 'alternative_name');
define('OKHUB_WP_FN_ISO_NUMBER', 'iso_number');
define('OKHUB_WP_FN_ISO_THREE_LETTER_CODE', 'iso_three_letter_code');
define('OKHUB_WP_FN_ISO_TWO_LETTER_CODE', 'iso_two_letter_code');
define('OKHUB_WP_FN_ARCHIVED', 'archived');
// Categories
define('OKHUB_WP_FN_LEVEL', 'level');
define('OKHUB_WP_FN_CAT_PARENT', 'cat_parent');

/* ---- Hidden meta fields ---- */
define('OKHUB_WP_FN_HIDDEN_SOURCES', '_current_sources');
define('OKHUB_WP_FN_HIDDEN_TITLE_LANGUAGE', '_current_title_language');
define('OKHUB_WP_FN_HIDDEN_CONTENT_LANGUAGE', '_current_content_language'); // For posts.
define('OKHUB_WP_FN_HIDDEN_DESCRIPTION_LANGUAGE', '_current_description_language'); // For taxonomy terms.
define('OKHUB_WP_FN_HIDDEN_NAME_LANGUAGE', '_current_name_language');

/* ---- Other settings. Most of these settings can and should be changed through the plugin's administrative page. ---- */

define('OKHUB_MAX_REQUESTED_ASSETS', 100); // Maximum number of documents/organisations that can be selected for each datasource to be imported at once.
define('OKHUB_HARD_LIMIT_ASSETS', 300); // Used in the widget.
define('OKHUB_BATCH_SIZE', 25); // Size batches to import content.
define('OKHUB_DEFAULT_DATASET', 'eldis');
define('OKHUB_DEFAULT_LANGUAGE', 'en');
define('OKHUB_DEFAULT_POST_STATUS', 'publish');
//define('OKHUB_DEFAULT_DATASET_ADMIN', 'all');
define('OKHUB_IMPORT_USER', 'OKHUB');
define('OKHUB_USER_EMAIL','okhub@ids.ac.uk');
define('OKHUB_SORT_ORDER', 'sort_desc');
define('OKHUB_SORT_FIELD', 'timestamp'); // Most recently added/updated first.
define('OKHUB_DEFAULT_TOP_LEVEL', 'true');
define('OKHUB_IMPORT_RECURRENCE_ASSETS', 'daily');
define('OKHUB_IMPORT_RECURRENCE_CATEGORIES', 'monthly');
define('OKHUB_REQUIRED_TRANSLATABLE_FIELDS', 'title,description'); // To be added to admin interface.
define('OKHUB_ADDITIONAL_TRANSLATABLE_FIELDS_DOCUMENTS', 'publisher_country');
define('OKHUB_ADDITIONAL_TRANSLATABLE_FIELDS_ORGANISATIONS', 'name');
define('OKHUB_UPDATED_ASSETS', 'overwrite');
define('OKHUB_NEW_CATEGORIES', TRUE);
//define('OKHUB_MAP_CATEGORIES', FALSE);
define('OKHUB_VERBOSE_IMPORT', TRUE);
define('OKHUB_INCLUDE_IMPORTED_DOCUMENTS', TRUE);
define('OKHUB_INCLUDE_IMPORTED_ORGANISATIONS', TRUE);
define('OKHUB_TAGS', TRUE);
define('OKHUB_TEMPLATES_DIR', 'templates');
// Widget default values.
define('OKHUB_WIDGET_ASSETS_TYPE', 'documents');
define('OKHUB_WIDGET_TITLE', 'OKHub updates');
define('OKHUB_WIDGET_NUM_ITEMS', 10);
define('OKHUB_WIDGET_MAX_ITEMS', 100);
define('OKHUB_WIDGET_ERROR_MESSAGES', TRUE); // Set to TRUE to allow the user to display widget's error messages. In general, it's only useful while in development.
define('OKHUB_WIDGET_TEMPLATE_DOCUMENTS', 'widget-okhub_documents.php');
define('OKHUB_WIDGET_TEMPLATE_ORGANISATIONS', 'widget-okhub_organisations.php');
define('OKHUB_WIDGET_ITEM_TEMPLATE_DOCUMENTS', 'widget-single-okhub_documents.php');
define('OKHUB_WIDGET_ITEM_TEMPLATE_ORGANISATIONS', 'widget-single-okhub_organisations.php');

/*--------------------------------- End default settings -----------------------------------*/
