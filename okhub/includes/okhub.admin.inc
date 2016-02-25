<?php

// Return available languages. TODO: Retrieve and save!
// Use count: http://api.okhub.org/v1/hub/count/themes/language/
function okhub_languages() {
  $languages = array(
                'sq' => __('Albanian'),
                'ar' => __('Arabic'),
                'zh' => __('Chinese'),
                'nl' => __('Dutch'),
                'en' => __('English'),
                'fr' => __('French'),
                'de' => __('German'),
                'it' => __('Italian'),
                'pt' => __('Portuguese'),
                'ru' => __('Russian'),
                'es' => __('Spanish'),
              );
  return $languages;
}

function okhub_datasets_names($force_update=FALSE) {
  $okhub_datasets = okhubapi_variable_get('okhub', 'okhub_datasets', array());
  if ($force_update || empty($okhub_datasets)) {
    $okhubapi = new OkhubApiWrapper;
    $api_key = okhubapi_variable_get('okhub', 'api_key', '');
    $response = $okhubapi->getAll('sources', OKHUB_HUB_SOURCE_CODE, $api_key, 'full');
    if (!$response->isEmpty()) {
      $sources = $response->getResults();
      foreach ($sources as $source) {
        $code = okhub_get_api_value($source, OKHUB_API_FN_SOURCE_CODE, OKHUB_OKHUB_SOURCE_CODE);
        $name = okhub_get_api_value($source, OKHUB_API_FN_SOURCE_NAME, OKHUB_OKHUB_SOURCE_CODE);
        if ($code && $name) {
          $okhub_datasets[$code] = $name;
        }
      }
      asort($okhub_datasets);
      okhubapi_variable_set('okhub', 'okhub_datasets', $okhub_datasets);
    }
  }
  return $okhub_datasets;
}

function okhub_datasets_names_with_content() {
  return array_intersect_key(okhub_datasets_names(), array_flip(okhub_datasets_with_content()));
}

function okhub_datasets() {
  return array_keys(okhub_datasets_names());
}

function okhub_selected_datasets($default=array()) {
  return okhubapi_variable_get('okhub', 'selected_datasets', $default);
}

// Links WP custom post types and taxonomies with the OKHub types.
function okhub_types() { 
  return array('countries' => 'Country', 'documents' => 'Document', 'documenttypes' => 'DocumentType', 'organisations' => 'Organisation', 'regions' => 'Region', 'subjects' => 'Subject', 'themes' => 'Theme');
}

function okhub_get_type_name($type, $plural=FALSE) {
  $singular_names = array('countries' => 'Country', 'documents' => 'Document', 'documenttypes' => 'Document Type', 'organisations' => 'Organisation', 'regions' => 'Region', 'subjects' => 'Subject', 'themes' => 'Theme');
  $name = $singular_names[$type];
  if ($plural) {
    $name = ($type == 'countries') ? 'Countries' : $name.'s';
  }
  return $name;
}

function okhub_relitems_type($hub_type) {
  $type = '';
  $types = array('Country' => 'country_focus', 'Document' => 'document', 'DocumentType' => 'document_type', 'Organisation' => 'publisher', 'Region' => 'region', 'Subject' => 'subject', 'Theme' => 'theme');
  if (isset($types[$hub_type])) {
    $type = $types[$hub_type];
  }
  return $type;
}

// WP post types' single labels.
function okhub_post_types() {
  return array('documents' => 'Document', 'organisations' => 'Organisation');
}

function okhub_assets() {
  return array_keys(okhub_post_types());
}

// WP taxonomies' single labels.
function okhub_taxonomies() {
  return array('countries' => 'Country', 'regions' => 'Region', 'themes' => 'Theme', 'documenttypes' => 'Document type');
}

function okhub_categories() {
  return array_keys(okhub_taxonomies());
}

function okhub_content_sources($force_update = FALSE) {
  $content_sources = okhubapi_variable_get('okhub', 'content_sources', array());
  if ($force_update || empty($content_sources)) {
    $okhubapi = new OkhubApiWrapper;
    $api_key = okhubapi_variable_get('okhub', 'api_key', '');
    $content_types = array_merge(okhub_assets(), okhub_categories());
    foreach ($content_types as $content_type) {
      $response = $okhubapi->count($content_type, 'hub', $api_key, 'source');
      if (!$response->isEmpty()) {
        foreach ($response->getResults() as $result) {
          $count = okhub_get_api_value($result, 'count');
          $source = okhub_get_api_value($result, 'object_name');
          if ($count && $source) {
            $content_sources[$source][$content_type] = $count;
          }
        }
      }
    }
    okhubapi_variable_set('okhub', 'content_sources', $content_sources);
  }
  return $content_sources;
}

function okhub_datasets_with_content($force_update=FALSE) {
  return array_keys(okhub_content_sources($force_update));
}

function okhub_has_content($dataset, $type='', $force_update=FALSE) {
  $content_sources = okhub_content_sources($force_update);
  if ($type) {
    return !empty($content_sources[$dataset][$type]);
  }
  else {
    return !empty($content_sources[$dataset]);
  }
}

function okhub_is_hierarchical($type) { // TODO: Generalize.
  return in_array($type, array_map('trim', explode(',', OKHUB_HIERARCHICAL_HUB_TYPES)));
}

function okhub_custom_type_name($type) {
  return OKHUB_CUSTOM_PREFIX . '_' . $type;
}

function okhub_custom_taxonomy_name($dataset, $name) {
  return OKHUB_CUSTOM_PREFIX . '_' . $dataset . '_' . $name;
}

function okhub_source_field_name($field_name, $dataset, $language='') {
  $field_name = $dataset . '_' . $field_name;
  if ($language) {
    $field_name .= '_' . $language;
  }
  return $field_name;
}

function okhub_taxonomy_short_name($taxonomy_name) {
  $short_name = '';
  $datasets = implode('|', okhub_datasets());
  $taxonomies = implode('|', okhub_categories());
  if (preg_match('/('.$datasets.')_('.$taxonomies.')/', $taxonomy_name, $matches)) {
    $short_name = $matches[2];
  }
  return $short_name;
}

function okhub_get_post_sources($post_id) { 
  return get_post_meta($post_id, OKHUB_WP_FN_HIDDEN_SOURCES);
}

function okhub_get_hub_relitems_field($object, $field_name, $dataset='hub', $rel_field) {
  // For some reason the API is returning always an array with one element, which is an array with one element indexed by the lower-case name of the rel. item type.
  $items_ids = array();
  $rel_items_api = okhub_get_api_value($object, $field_name, $dataset);
  if (($rel_items = okhub_get_api_value($object, $field_name, $dataset)) && is_array($rel_items) && ($rel_items_api = current($rel_items))) {    
    if (is_array($rel_items_api)) {     
      if (is_numeric(key($rel_items_api))) {
        foreach ($rel_items_api as $rel_item) {
          if (isset($rel_item[$rel_field])) {
            $items_ids[] = $rel_item[$rel_field];
          }
        }
      }
      elseif (isset($rel_items_api[$rel_field])) {
        $items_ids[] = $rel_items_api[$rel_field];
      }
    }
    else {
      $items_ids[] = $rel_items_api;
    }
  }
  return $items_ids;
}

// $alternative_languages in priority order.
function okhub_api_value_languages($object, $field_name, $dataset='', $alternative_languages=array(), $return_single=TRUE) {
  $value_language = array(); // if not found in one of the passed languages, return an empty array.
  $language = '';
  if ($api_values = okhub_get_api_value($object, $field_name, $dataset, '', FALSE)) {
    foreach ($alternative_languages as $language) {
      if (isset($api_values[$language])) {
        $value_language[$language] = $api_values[$language];
        break; // We get the value for the first language for which it's found.
      }
    }
    if (isset($value_language[$language]) && is_array($value_language[$language]) && $return_single) {
      $value_language[$language] = current($value_language[$language]);
    }
  }
  return $value_language;
}

// Get values. Languages in order of preference.
// $dataset can be empty as there are a few fields ('item_id', 'sources', 'timestamp') that don't have a related source code.
// If the $dataset parameter is empty, $return_single=FALSE and there are values for several source, it will return the array with values for all the sources.
// If the $language parameter is not empty, it will return results only the the passed source and language.
// If the $language is empty, $return_single=FALSE and there are values for several languages, all will be returned.
// if the $dataset parameter is passed, only values corresponding to this dataset will be returned.
/* if (in_array('all', $accepted_languages)) {
    ...
  }
*/
function okhub_get_api_value($object, $field_name, $dataset='', $language='', $return_single=TRUE) {
  $value = FALSE;
  if (is_a($object, 'OkhubApiObject')) {
    $valid_language_codes = array_keys(okhub_languages());
    //$default_language = okhubapi_variable_get('okhub', 'default_language', OKHUB_DEFAULT_LANGUAGE);
    if ($api_value = $object->get_value_object($field_name)) {
      if ($dataset) {
        if (isset($api_value[$dataset])) {
          if (is_scalar($api_value[$dataset])) {
            $value = $api_value[$dataset];
          }
          elseif (is_array($api_value[$dataset]) && !empty($api_value[$dataset])) {
            $keys = array_keys($api_value[$dataset]);
            if (is_int($keys[0])) {  // it's enough to look at the first key to know if it's indexed by language.
              $value = $api_value[$dataset];
            }
            else { // it's indexed by language.
              if ($language) {
                if (isset($api_value[$dataset][$language]) && !empty($api_value[$dataset][$language])) {
                  $value = $api_value[$dataset][$language];
                }
              }
              else {
                $value = $api_value[$dataset];
              }
            }
          }
        }
      }
      else {
        $value = $api_value;
      }
      if (is_array($value) && $return_single)  {
        $value = current($value); // returns FALSE if the array is empty.
        if (is_array($value) && count($value) == 1 && isset($value[0]) && is_scalar($value[0])) {
          $value = $value[0];
        }
      }
    }
  }
  return $value;
}

function okhub_get_dataset_name($dataset) {
  return $dataset;
}

function okhub_get_dataset_acronym($dataset) {
  return ucfirst($dataset);
}

function okhub_get_translatable_fields($post_type) {
  switch ($post_type) {
    case 'okhub_documents':
      $additional_fields = OKHUB_ADDITIONAL_TRANSLATABLE_FIELDS_DOCUMENTS;
      break;
    case 'okhub_organisations':
      $additional_fields = OKHUB_ADDITIONAL_TRANSLATABLE_FIELDS_ORGANISATIONS;
      break;
  }
  return array_unique(array_map('trim', explode(',', OKHUB_REQUIRED_TRANSLATABLE_FIELDS . ',' . $additional_fields)));
}

function okhub_has_language_content($asset, $dataset, $okhub_language_filters_codes) {
  $has_language = TRUE;
  $required_fields_lang = array_map('trim', explode(',', OKHUB_REQUIRED_TRANSLATABLE_FIELDS));
  foreach ($okhub_language_filters_codes as $lang) {
    $has_language = TRUE;
    foreach ($required_fields_lang as $field_name) {
      $field_value = $asset->get_value_object($field_name);
      $has_language = $has_language && !empty($field_value[$dataset][$lang]);
    }
    if ($has_language) {
      break; // No need to keep looking.
    }
  }
  return $has_language;
}

function okhub_excerpt_post($content) {
  $okhub_excerpt_length = okhubapi_variable_get('okhub', 'excerpt_length', 0);
  if(strlen($content) > $okhub_excerpt_length) {
    $excerpt = substr($content, 0, $okhub_excerpt_length-3);
    $space = strrpos($excerpt, ' ');
    $excerpt = substr($excerpt, 0, $space);
    $excerpt .= '...';
  } else {
    $excerpt = $content;
  }
  return $excerpt;
}

function okhub_exclude_asset($asset, $dataset, $exclude_uris) {
  $exclude = FALSE;
  $urls = okhub_get_api_value($asset, OKHUB_API_FN_URLS, $dataset);
  if ($exclude_uris && isset($asset->urls)) {
    $uris = explode(',', $exclude_uris);
    foreach($uris as $uri) {
      $uri = preg_quote(trim($uri), '/');
      $matches = preg_grep("/$uri/i", $asset->urls);
      if ($exclude = !empty($matches)) {
        break;
      }
    }
  }
  return $exclude;
}

//------------------------- Generate admin settings page ------------------------

// Checks if it has to display the admin settings or if it has to initialize the plugin's data, etc.
function okhub_admin_main() {
  if (!current_user_can( 'manage_options')) {
    wp_die(__('You do not have sufficient permissions to manage the plugin settings.'));
  }
  $okhub_setup_done = okhubapi_variable_get('okhub', 'setup_done', FALSE);
  if ($okhub_setup_done) {
    okhub_after_config();
  }
  okhub_render_form();
  $importer_url = 'import.php?import=okhub_importer';
  $message = '<div class="okhub_message">You can change the plugin configuration options above or proceed to the <input type="button" class="button-primary" value="manual import page" onclick="window.location=\''.$importer_url.'\';"></div>';
  okhub_show_message($message);
}

// Display registered error messages
function okhub_admin_notices() {
  settings_errors('okhub_options', FALSE, TRUE);
  okhubapi_report_errors();
}

// Run plugin initialization tasks after configuration.
function okhub_after_config() {
  // Schedule periodic imports
  okhub_schedule_imports();
  // Create/update author of imported posts.
  $import_user = okhubapi_variable_get('okhub', 'import_user', OKHUB_IMPORT_USER);
  $user_id = okhubapi_variable_get('okhub', 'import_user_id', 0);
  $okhub_user = get_user_by('email', OKHUB_USER_EMAIL);
  if ((!$okhub_user) && (!empty($import_user))) {
    $user_id = okhub_new_user($import_user);
    okhubapi_variable_set('okhub', 'import_user_id', $user_id);
  }
  if ($user_info = get_userdata($user_id)) {
    okhubapi_variable_set('okhub', 'import_user', $user_info->user_login);
  }
}

// Create new user (author of imported content)
function okhub_new_user($username) {
  $password = wp_generate_password( $length=12, $include_standard_special_chars=false );
  $user_data = array (
    'user_login' => $username,
    'user_pass' => $password,
    'user_email' => OKHUB_USER_EMAIL,
    'role' => 'editor',
    'description' => 'Default author for the OKHub Content.',
    'user_url' => OKHUB_API_URL
  );
  $user_id = wp_insert_user( $user_data );
  if (is_wp_error($user_id)) {
    okhubapi_variable_set('okhub', 'import_user', OKHUB_IMPORT_USER);
    okhubapi_register_error('okhub', __('The user could not be created. Error message: ') . $user_id->get_error_message(), 'okhub_new_user', 'warning');
    $user_id = 0;
  }
  return $user_id;
}

function okhub_clear_cache() {
  $okhubapi = new OkhubApiWrapper;
  $okhubapi->cacheFlush();
  // Reset content from the API saved in options.
  okhubapi_variable_set('okhub', 'okhub_datasets', array());
  okhubapi_variable_set('okhub', 'content_sources', array());
  $categories_datasets = okhubapi_variable_get('okhub', 'categories_datasets', array());
  foreach ($categories_datasets as $var_name) {
    okhubapi_variable_set('okhub', $var_name, array());
  }    
}

//------------------------------ Options validation -----------------------------

// Validate input and the API key.
function okhub_validate_options($input) {
  $okhubapi = new OkhubApiWrapper;
  $error = FALSE;
  $error_message = '';
  $okhub_api_key_validated = okhubapi_variable_get('okhub', 'api_key_validated', FALSE);
  $reg_api_key = '/^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$/';
  $reg_api_url = '/^http:\/\/[\/0-9a-z\.]+$/';
  $reg_path = '/^\/?([\w\-]+\/?)*\/?$/';
  $allowed_html = array();
  if ((isset($input['api_key'])) && preg_match($reg_api_key, $input['api_key'])) {
    if ($okhubapi->validateKey($input['api_key'])) {
        $input['api_key_validated'] = TRUE;
    }
    else {
      $input['api_key'] = '';
      $input['api_key_validated'] = FALSE;
      $error = TRUE;
      $error_message = __(' The OKHub API call did not retrieve results. Please check that you have entered a valid API key.');
    }
  }
  else {
    $input['api_key'] = '';
    $input['api_key_validated'] = FALSE;
    $error = TRUE;
    $error_message = __('Please enter a valid API key.');
  }
  if ($okhub_api_key_validated) {
    $input['include_imported_documents'] = (isset($input['include_imported_documents']) && $input['include_imported_documents']);
    $input['include_imported_organisations'] = (isset($input['include_imported_organisations']) && $input['include_imported_organisations']);
    $input['import_new_categories'] = (isset($input['import_new_categories']) && $input['import_new_categories']);
    $input['import_tags'] = (isset($input['import_tags']) && $input['import_tags']);
    if (empty($input['selected_datasets'])) {
      $error = TRUE;
      $error_message = 'Please select at least one source to retrieve content from.';
    }
    //If the API key had not been validated, the other fields had not been displayed, so they keep the default values.
    if (isset($input['num_items'])) {
      if (!is_numeric($input['num_items'])) {
        $error = TRUE;
        $error_message .= ' ' . $input['num_items'] . __(' is not a valid number of items to import. Please enter a numeric value.');
        $input['num_items'] = OKHUB_API_NUM_ITEMS;
      }
    }
    else {
      $input['num_items'] = OKHUB_API_NUM_ITEMS;
    }
    if (isset($input['excerpt_length'])) {
      if (!is_numeric($input['excerpt_length'])) {
        $error = TRUE;
        $error_message .= ' ' . $input['excerpt_length'] . __(' is not a valid length. Please enter a numeric value.');
        $input['excerpt_length'] = 0;
      }
    }
    else {
      $input['excerpt_length'] = 0;
    }
    if (isset($input['exclude_uris'])) {
      $input['exclude_uris'] = wp_kses($input['exclude_uris'], $allowed_html);
    }
    else {
      $input['exclude_uris'] = '';
    }
    if (isset($input['import_authors_assets'])) {
      $input['import_authors_assets'] = wp_kses($input['import_authors_assets'], $allowed_html);
    }
    else {
      $input['import_authors_assets'] = '';
    }
    if (isset($input['import_publishers_assets'])) {
      $input['import_publishers_assets'] = wp_kses($input['import_publishers_assets'], $allowed_html);
    }
    else {
      $input['import_publishers_assets'] = '';
    }
    if (isset($input['import_query'])) {
      $input['import_query'] = wp_kses($input['import_query'], $allowed_html);
    }
    else {
      $input['import_query'] = '';
    }
    if (isset($input['import_user'])) {
      $input['import_user'] =  sanitize_user($input['import_user'], TRUE);
    }
    else {
      $input['import_user'] = OKHUB_IMPORT_USER;
    }
  }
  else {
    $input['include_imported_documents'] = okhubapi_variable_get('okhub', 'include_imported_documents', OKHUB_INCLUDE_IMPORTED_DOCUMENTS);
    $input['include_imported_organisations'] = okhubapi_variable_get('okhub', 'include_imported_organisations', OKHUB_INCLUDE_IMPORTED_ORGANISATIONS);
    $input['import_new_categories'] = okhubapi_variable_get('okhub', 'import_new_categories', OKHUB_NEW_CATEGORIES);
    $input['import_tags'] = okhubapi_variable_get('okhub', 'import_tags', OKHUB_TAGS);
  }
  if ($error) {
    $input['setup_done'] = FALSE;
    add_settings_error('okhub_options', 'okhub_errors', $error_message);
  }
  else {
    $input['setup_done'] = TRUE;
  }
  return $input;
}
