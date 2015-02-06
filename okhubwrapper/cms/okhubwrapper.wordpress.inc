<?php

/**
 * Wordpress-specific variables and functions.
 *
 */

global $okhub_assets;
global $okhub_datasets; 
global $okhub_categories;
global $okhub_exclude;
global $okhub_sources;

$okhub_datasets = array('hub');
$okhub_assets = array('documents','organisations');
$okhub_categories = array('countries', 'regions');
$okhub_sources = array('opendocs','eldis','observaction','bridge','heart','pids','ccccc','ella','serpp');
global $okhubapi_errors;
$okhubapi_errors = array();

// Call the URL.
function okhubapi_get_data($url) {
  $response = wp_remote_get($url, array('timeout' => 60));
  if (is_wp_error($response)) {
    $data = $response->get_error_message();
  }
  else {
    $data = json_decode($response['body'], true);
  }
  return $data;
}

//------------------------------ Error processing --------------------------------

// Register a new error or notification.
function okhubapi_register_error($component, $message, $function, $error_code, $verbose = TRUE) {
  global $okhubapi_errors;
  $text_message = $message;
  if ($verbose) {
    if ($component) {
      $text_message = $component . ' reported: ' . $text_message;
    }
    if ($function) {
      $text_message .= ' In: ' . $function;
    }
  }
  if (isset($okhubapi_errors[$error_code])) {
    $okhubapi_errors[$error_code]->add($error_code, $text_message); 
  }
  else {
    $okhubapi_errors[$error_code] = new WP_Error($error_code, $text_message);
  }
}

// Display previously registered errors.
function okhubapi_report_errors() {
  global $okhubapi_errors;
  $okhubapi_show_error_codes =  explode(',', OKHUB_API_SHOW_ERROR_CODES);
  $output = '';
  foreach ($okhubapi_show_error_codes as $error_code) {
    if (isset($okhubapi_errors[$error_code])) {
      $error_messages = $okhubapi_errors[$error_code]->get_error_messages();
      if (OKHUB_API_HTML_ERRORS) {
        $output .= '<div class="ids-' . $error_code . '"><ul>';
      }
      foreach ($error_messages as $error_message) {
        if (OKHUB_API_HTML_ERRORS) {
          $output .= '<li><strong>' . ucfirst($error_code) . ':</strong> ' . $error_message . '</li>';
        }
        else {
          $output .= ucfirst($error_code) . ": $error_message\n";
        }
      }
      if (OKHUB_API_HTML_ERRORS) {
        $output .= '</ul></div>';
      }
    }
  }
  if ($output) {
    echo $output;
  }
}

//------------------------------ Options handling --------------------------------

// Set the value of a parameter.
function okhubapi_variable_set($group, $variable_name, $value) {
  $options_name = $group . '_options';
  if ($options = get_option($options_name)) {
    $options[$variable_name] = $value;
    update_option($options_name, $options);
  }
  else {
    $options[$variable_name] = $value;
    add_option($options_name, $options);
  }
}

// Retrieve the value of a saved parameter
function okhubapi_variable_get($group, $variable_name, $default = '') {
  $value = $default;
  $options_name = $group . '_options';
  $options = get_option($options_name);

  
  if (isset($options[$variable_name])) {
    $value = $options[$variable_name];
  }
  return $value;
}

//------------------------------ Helper functions --------------------------------

function okhubapi_exclude($dataset, $type) {
  global $ids_exclude;
  if (isset($ids_exclude[$dataset]) && ($ids_exclude[$dataset] === $type)) {
    return TRUE;
  }
  else {
    return FALSE;
  }
}



