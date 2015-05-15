<?php

//------------------------- Generate admin settings page ------------------------

// Checks if it has to display the admin settings or if it has to initialize the plugin's data, etc.
function okhubimport_admin_main() {
  if (!current_user_can( 'manage_options')) {
    wp_die(__('You do not have sufficient permissions to manage the plugin settings.'));
  }
  $okhubimport_setup_done = okhubapi_variable_get('okhubimport', 'setup_done', FALSE);
  if ($okhubimport_setup_done) {
    $importer_url = 'import.php?import=okhubimport_importer';
    $message = __("You can change the plugin configuration options below or proceed to the <input type=\"button\" class=\"button-primary\" value=\"manual import page\" onclick=\"window.location='$importer_url';\">");
    okhub_show_message($message);
    okhubimport_after_config();
  }
  okhubimport_render_form();
}

// Display registered error messages
function okhubimport_admin_notices() {
  settings_errors('okhubimport_options', FALSE, TRUE);
  okhubapi_report_errors();
}

// Run plugin initialization tasks after configuration.
function okhubimport_after_config() {
  // Schedule periodic imports
  okhubimport_schedule_imports();
  // Create/update author of imported posts.
  $import_user = okhubapi_variable_get('okhubimport', 'import_user', OKHUB_IMPORT_IMPORT_USER);
  $user_id = okhubapi_variable_get('okhubimport', 'import_user_id', 0);
  $okhub_user = get_user_by('email', OKHUB_IMPORT_USER_EMAIL);
  if ((!$okhub_user) && (!empty($import_user))) {
    $user_id = okhubimport_new_user($import_user);
    okhubapi_variable_set('okhubimport', 'import_user_id', $user_id);
  }
  if ($user_info = get_userdata($user_id)) {
    okhubapi_variable_set('okhubimport', 'import_user', $user_info->user_login);
  }
  okhubapi_report_errors();
}

// Create new user (author of imported content)
function okhubimport_new_user($username) {
  $password = wp_generate_password( $length=12, $include_standard_special_chars=false );
  $user_data = array (
    'user_login' => $username,
    'user_pass' => $password,
    'user_email' => OKHUB_IMPORT_USER_EMAIL,
    'role' => 'editor',
    'description' => 'Default author for the OKHUB imported content.',
    'user_url' => OKHUB_API_URL
  );
  $user_id = wp_insert_user( $user_data );
  if (is_wp_error($user_id)) {
    okhubapi_variable_set('okhubimport', 'import_user', OKHUB_IMPORT_IMPORT_USER);
    okhubapi_register_error('okhubimport', __('The user could not be created. Error message: ') . $user_id->get_error_message(), 'okhubimport_new_user', 'warning');
    $user_id = 0;
  }
  return $user_id;
}

function okhubimport_get_datasets() {
  global $okhub_datasets;
  $default_dataset = okhubapi_variable_get('okhubimport', 'default_dataset', OKHUB_IMPORT_DEFAULT_DATASET_ADMIN);
  if ($default_dataset == 'all') {
    $datasets = $okhub_datasets;
  }
  else {
    $datasets = array($default_dataset);
  }
  return $datasets;
}

function okhubimport_display_datasets($area) {
  global $okhub_datasets;
  $display_dataset = array();
  $datasets = $okhub_datasets;
  //$display_dataset['admin'] = okhubapi_variable_get('okhubimport', 'display_dataset_admin', OKHUB_IMPORT_DEFAULT_DATASET_ADMIN);
  $display_dataset['admin'] = okhubapi_variable_get('okhubimport', 'default_dataset', OKHUB_IMPORT_DEFAULT_DATASET_ADMIN);
  $display_dataset['public'] = okhubapi_variable_get('okhubimport', 'display_dataset_public', OKHUB_IMPORT_DEFAULT_DATASET_PUBLIC);
  if (isset($display_dataset[$area])) {
    switch($display_dataset[$area]) {
      case 'all':
        $datasets = $okhub_datasets;
        break;
      case 'none':
        $datasets = array();
        break;
      default:
        $datasets = array($display_dataset[$area]);
    }
  }
  else {
    $datasets = array();
  }
  return $datasets;
}

//------------------------------ Options validation -----------------------------

// Validate input and the API key.
function okhubimport_validate_options($input) {
  global $okhub_datasets;
  $idsapi = new OkhubApiWrapper;
  $error = FALSE;
  $error_message = '';
  $okhubimport_api_key_validated = okhubapi_variable_get('okhubimport', 'api_key_validated', FALSE);
  $reg_api_key = '/^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$/';
  $reg_api_url = '/^http:\/\/[\/0-9a-z\.]+$/';
  $reg_path = '/^\/?([\w\-]+\/?)*\/?$/';
  $allowed_html = array();
  $current_permalinks = get_option('permalink_structure');
  $input['permalink_structure'] = okhubapi_variable_get('okhubimport', 'permalink_structure', $current_permalinks);
  if ((isset($input['api_key'])) && preg_match($reg_api_key, $input['api_key'])) {
    if ($idsapi->validateKey($input['api_key'])) {
        $input['api_key_validated'] = TRUE;
    }
    else {
      $input['api_key'] = '';
      $input['api_key_validated'] = FALSE;
      $error = TRUE;
      $error_message = __(' The OKHUB API call did not retrieve results. Please check that you have entered a valid API key.');
    }
  }
  else {
    $input['api_key'] = '';
    $input['api_key_validated'] = FALSE;
    $error = TRUE;
    $error_message = __(' The OKHUB API key format is not valid. Please enter a valid API key.');
  }
  if ($okhubimport_api_key_validated) {
    $input['include_imported_documents'] = (isset($input['include_imported_documents']) && $input['include_imported_documents']);
    //$input['include_imported_organisations'] = (isset($input['include_imported_organisations']) && $input['include_imported_organisations']);
    $input['import_new_categories'] = (isset($input['import_new_categories']) && $input['import_new_categories']);
    $input['import_map_categories'] = (isset($input['import_map_categories']) && $input['import_map_categories']);
    $input['import_tags'] = (isset($input['import_tags']) && $input['import_tags']);

    //If the API key had not been validated, the other fields had not been displayed, so they keep the default values.
    if (isset($input['num_items'])) {
      if (!is_numeric($input['num_items'])) {
        $error = TRUE;
        $error_message .= ' ' . $input['num_items'] . __(' is not a valid number of items to import. Please enter a numeric value.');
        $input['num_items'] = OKHUB_API_NUM_ITEMS;
      }
    }
    if (isset($input['excerpt_length'])) {
      if (!is_numeric($input['excerpt_length'])) {
        $error = TRUE;
        $error_message .= ' ' . $input['excerpt_length'] . __(' is not a valid length. Please enter a numeric value.');
        $input['excerpt_length'] = 0;
      }
    }
    if (isset($input['exclude_uris'])) {
      $input['exclude_uris'] = wp_kses($input['exclude_uris'], $allowed_html);
    }
    if (isset($input['import_authors_assets'])) {
      $input['import_authors_assets'] = wp_kses($input['import_authors_assets'], $allowed_html);
    }
    if (isset($input['import_publishers_assets'])) {
      $input['import_publishers_assets'] = wp_kses($input['import_publishers_assets'], $allowed_html);
    }
    if (isset($input['import_query'])) {
      $input['import_query'] = wp_kses($input['import_query'], $allowed_html);
    }
    if (isset($input['import_user'])) {
      $input['import_user'] =  sanitize_user($input['import_user'], TRUE);
    }
    foreach ($okhub_datasets as $dataset) {
      if (($input['default_dataset'] != 'all') && ($input['default_dataset'] != $dataset)) {
        $input['import_document_type_'.$dataset] = 'field';
        $input['import_document_type_taxonomy_label_singular_'.$dataset] = '';
        $input['import_document_type_taxonomy_label_plural_'.$dataset] = '';
      }
      else {
        if ($input['import_document_type_'.$dataset] == 'new_taxonomy') {
          if (isset($input['import_document_type_taxonomy_label_singular_'.$dataset]) && isset($input['import_document_type_taxonomy_label_plural_'.$dataset]) && preg_match('/^\w[\w ]+\w$/', $input['import_document_type_taxonomy_label_singular_'.$dataset]) && preg_match('/^\w[\w ]+\w$/', $input['import_document_type_taxonomy_label_plural_'.$dataset])) {
            // We don't change the existing taxonomy internal name.
            if ($current_taxonomy_name = okhubapi_variable_get('okhubimport', 'import_document_type_taxonomy_'.$dataset, '')) {
              $input['import_document_type_taxonomy_'.$dataset] = $current_taxonomy_name;
            }
            else {
              $input['import_document_type_taxonomy_'.$dataset] = str_replace('-', '_', sanitize_title($input['import_document_type_taxonomy_label_singular_'.$dataset]));
              if (($current_taxonomy != $input['import_document_type_taxonomy_'.$dataset]) && taxonomy_exists($input['import_document_type_taxonomy_'.$dataset])) {
                $error = TRUE;
                $error_message .= __(' Invalid taxonomy name for ') . ucfirst($dataset) . __(' document types.');
                $error_message .= __('There already is a taxonomy with that name. Please choose another name or select the existing taxonomy.');
                $input['import_document_type_taxonomy_label_singular_'.$dataset] = '';
                $input['import_document_type_taxonomy_label_plural_'.$dataset] = '';
              }
            }
          }
          else {
            $error = TRUE;
            $error_message .= __(' Invalid taxonomy label for ') . ucfirst($dataset) . __(' document types.');
            $input['import_document_type_taxonomy_label_singular_'.$dataset] = '';
            $input['import_document_type_taxonomy_label_plural_'.$dataset] = '';
          }
        }
      }
    }
    $warning_paths = '';
    //foreach (array('documents', 'organisations') as $asset_type) {
    foreach (array('documents') as $asset_type) {
      if (isset($input['okhub_'.$asset_type.'_path']) && !$error) {
        if (!isset($input['okhub_'.$asset_type.'_old_path']) || ($input['okhub_'.$asset_type.'_path'] !== $input['okhub_'.$asset_type.'_old_path'])) { 
          if (preg_match($reg_path, $input['okhub_'.$asset_type.'_path'])) { // We always validate a new path.
            if (!$warning_paths) {
              $input['okhub_'.$asset_type.'_path'] = trim($input['okhub_'.$asset_type.'_path'], '/ ');
              $path = array_shift(explode('/', $input['okhub_'.$asset_type.'_path']));
              $old_path = array_shift(explode('/', $input['okhub_'.$asset_type.'_old_path']));
              if (($path !== $old_path) && okhubimport_check_existing_rules($path)) { // If the warning has already been displayed and it has the same beginning as the old path, we don not show it.
                $warning_paths = sprintf(__('Warning: Rewrite rule(s) beginning with \'%s\' already exist. This might create a conflict in Wordpress\' URL rewriting process. In this case, please consider using other paths.'), $path);
              }
            }
          }
          else {
            $error = TRUE;
            $error_message .= ' ' . $input['okhub_'.$asset_type.'_path'] . __(' is not a valid path.');
            $input['okhub_'.$asset_type.'_path'] = $input['okhub_'.$asset_type.'_old_path'];
          }
        }
        if (!$error) {
          $input['okhub_'.$asset_type.'_old_path'] = $input['okhub_'.$asset_type.'_path'];
        }
      }
    }
    if ($warning_paths) {
      add_settings_error('okhubimport_options', 'okhubimport_errors', $warning_paths, 'updated');
    }
  }
  else {
    $input['include_imported_documents'] = okhubapi_variable_get('okhubimport', 'include_imported_documents', OKHUB_IMPORT_INCLUDE_IMPORTED_DOCUMENTS);
    //$input['include_imported_organisations'] = okhubapi_variable_get('okhubimport', 'include_imported_organisations', OKHUB_IMPORT_INCLUDE_IMPORTED_ORGANISATIONS);
    $input['import_new_categories'] = okhubapi_variable_get('okhubimport', 'import_new_categories', OKHUB_IMPORT_NEW_CATEGORIES);
    $input['import_map_categories'] = okhubapi_variable_get('okhubimport', 'import_map_categories', OKHUB_IMPORT_MAP_CATEGORIES);
    $input['import_tags'] = okhubapi_variable_get('okhubimport', 'import_tags', OKHUB_IMPORT_TAGS);
  }
  if ($error) {
    $input['setup_done'] = FALSE;
    add_settings_error('okhubimport_options', 'okhubimport_errors', $error_message);
  }
  else {
    $input['setup_done'] = TRUE;
  }
  return $input;
}

function okhubimport_check_existing_rules($path) {
  global $wp_rewrite;
  if ($path && !empty($wp_rewrite->rules)) {
    return preg_grep('#^'.$path.'/#', array_keys($wp_rewrite->rules));
  }
  else {
    return FALSE;
  }
}

