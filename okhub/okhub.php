<?php
/*
Plugin Name: OKHub Content (Beta)
Plugin URI: http://developer.okhub.org/okhub_plugins/
Description: Imports and/or displays content from the Open Knowledge Hub via the OKHub API (http://api.okhub.org/).
Version: 1.0.0
Author: Institute of Development Studies (IDS)
Author URI: http://www.ids.ac.uk/
License: GPLv3

    Copyright 2016  Institute of Development Studies (IDS)

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

require_once('includes/okhub.default.inc');

if (!defined('OKHUB_API_ENVIRONMENT')) define('OKHUB_API_ENVIRONMENT', 'wordpress');

if (!defined('OKHUB_API_LIBRARY_DIR')) {
  if ($wrapper = glob(WP_PLUGIN_DIR . '{/*/*/*/,/*/*/,/*/,/}okhubwrapper.wrapper.inc', GLOB_BRACE)) {
    $dir_wrapper = str_replace('okhubwrapper.wrapper.inc', '', $wrapper[0]);
    define('OKHUB_API_LIBRARY_DIR', $dir_wrapper);
    define('OKHUB_API_LIBRARY_PATH', plugin_basename($dir_wrapper));
        
  }
  else {
    wp_die(__('OKHub Content: The PHP OKHub Wrapper was not found. Please download it from https://github.com/okhub-API/PHP-wrapper/archive/master.zip'));
    deactivate_plugins(plugin_basename( __FILE__ ));
  }
}
if (file_exists(OKHUB_API_LIBRARY_DIR) && is_readable(OKHUB_API_LIBRARY_DIR)) {
  require_once(OKHUB_API_LIBRARY_DIR . 'okhubwrapper.wrapper.inc');
} else {
  wp_die(__('OKHub Content: The PHP OKHub Wrapper\'s directory or its contents could not be read.'));
}
if (!file_exists(OKHUB_API_LIBRARY_DIR.'/cache') || !is_writable(OKHUB_API_LIBRARY_DIR.'/cache')) {
  wp_die(__('OKHub Content: Please make sure that the directory '.OKHUB_API_LIBRARY_DIR.'/cache exists and is writable by the server user.'));
}

define('OKHUB_SCRIPTS_PATH', plugins_url('scripts', __FILE__));
define('OKHUB_IMAGES_PATH', plugins_url('images', __FILE__));

require_once('includes/okhub.settings.inc');
require_once('includes/okhub.admin.inc');
require_once('includes/okhub.customtypes.inc');
require_once('includes/okhub.html.inc');
require_once('includes/okhub.interface.inc');
require_once('includes/okhub.metadata.inc');
require_once('includes/okhub.importer.inc');
//require_once('includes/okhub.importer_request.inc');
require_once('includes/okhub.widget.inc');

//-------------------------------- Set-up hooks ---------------------------------

register_activation_hook(__FILE__, 'okhub_activate');
add_action('init', 'okhub_init');
add_action('admin_init', 'okhub_admin_init');
add_action('admin_menu', 'okhub_add_options_page');
add_action('admin_menu', 'okhub_add_menu', 9);
add_action('widgets_init', 'okhub_register_widget');
add_action('admin_notices', 'okhub_admin_notices');
add_action('wp_enqueue_scripts', 'okhub_add_stylesheet', 5);
add_action('admin_enqueue_scripts', 'okhub_add_admin_stylesheet', 5);
add_action('admin_enqueue_scripts', 'okhub_add_javascript', 6);
add_filter('plugin_action_links', 'okhub_plugin_action_links', 10, 2);
add_filter('manage_okhub_documents_posts_columns', 'okhub_posts_columns');
add_filter('manage_okhub_organisations_posts_columns', 'okhub_posts_columns');
add_action('manage_okhub_documents_posts_custom_column', 'okhub_populate_posts_columns');
add_action('manage_okhub_organisations_posts_custom_column', 'okhub_populate_posts_columns');
add_filter('manage_post_posts_columns', 'okhub_posts_columns');
add_action('manage_post_posts_custom_column', 'okhub_populate_posts_columns');
add_filter('cron_schedules', 'okhub_cron_intervals'); 
add_action('okhub_scheduled_events', 'okhub_run_periodic_imports', 10, 1);
add_filter('request', 'okhub_filter_imported_tags');
add_filter('query_vars', 'okhub_query_vars');
add_action('pre_get_posts', 'okhub_include_assets_loop');
add_filter('pre_get_posts', 'okhub_filter_posts');
add_action('delete_term', 'okhub_delete_term', 10, 3);
add_filter('get_previous_post_where', 'okhub_adjacent_post_where');
add_filter('get_previous_post_join', 'okhub_adjacent_post_join');
add_filter('get_next_post_where', 'okhub_adjacent_post_where');
add_filter('get_next_post_join', 'okhub_adjacent_post_join');
add_action('template_redirect', 'okhub_javascript_trigger');
add_filter('single_template', 'okhub_single_templates');
add_filter('content_template', 'okhub_content_templates');

//--------------------------- Set-up / init functions ----------------------------

// OKHub Content plugin activation.
function okhub_activate() {
  $okhubapi = new OkhubApiWrapper;
  $okhubapi->cacheFlush();
  okhub_init();
  flush_rewrite_rules();
}

// Register custom types, taxonomies and importer.
function okhub_init() {
  //global $wp_rewrite;
  // Register post types
  okhub_post_types_init();
  // Register taxonomies.
  okhub_taxonomies_init();
  // Register importer
  if (class_exists('OkhubImporter')) {
    $okhub_importer = new OkhubImporter();
    register_importer('okhub_importer', __('OKHub Content', 'okhub-importer'), __('Import posts from the Open Knowledge Hub.', 'okhub-importer'), array ($okhub_importer, 'dispatch'));
  }
  add_rewrite_tag('%okhub_source%', '([^&]+)');
}

// Clean up on deactivation
function okhub_deactivate() {
  $okhubapi = new OkhubApiWrapper;
  $okhubapi->cacheFlush();
  okhub_delete_plugin_options();
  okhub_unschedule_imports();
  flush_rewrite_rules();
}

// Clean up on uninstall
function okhub_uninstall() {
  okhub_delete_taxonomy_metadata();
}

// Delete options entries
function okhub_delete_plugin_options() {
	delete_option('okhub_options');
}

// Initialize the plugin's admin options
function okhub_admin_init(){
  register_setting('okhub', 'okhub_options', 'okhub_validate_options');
  $options = get_option('okhub_options');
  if (!is_array($options)) { // The options are corrupted.
    okhub_delete_plugin_options();
  }
  okhub_edit_post_form();
  register_deactivation_hook(__FILE__, 'okhub_deactivate');
  register_uninstall_hook(__FILE__, 'okhub_uninstall');
}

// Register OKHub Widget
function okhub_register_widget() {
  register_widget('OKHub_Widget');
}

//------------------------ New post types and taxonomies -------------------------

function okhub_single_templates($single) {
  return okhub_templates($single, 'single');
}

function okhub_content_templates($content) {
  return okhub_templates($content, 'content');
}

function okhub_templates($template, $type) {
  global $post;
  $file_name = plugin_dir_path( __FILE__ ) . OKHUB_TEMPLATES_DIR .'/'. $type .'-'. $post->post_type . '.php';
  if (preg_match('/^'.OKHUB_CUSTOM_PREFIX.'/', $post->post_type) && file_exists($file_name)) {
    return $file_name;
  }
  return $template;
}

// Create new custom taxonomies for the OKHub categories.
function okhub_taxonomies_init() {
  $datasets = okhub_selected_datasets();
  $okhub_taxonomies = okhub_taxonomies();
  okhub_create_taxonomy_metadata();
  foreach ($datasets as $dataset) {
    // "Regular" OKHub taxonomies.
    $dataset_acronym = okhub_get_dataset_acronym($dataset);
    foreach ($okhub_taxonomies as $taxonomy => $singular_name) {
      $taxonomy_name = okhub_custom_taxonomy_name($dataset, $taxonomy);
      $taxonomy_label = $dataset_acronym . ' ' . okhub_get_type_name($taxonomy, TRUE);
      $singular_name = $dataset_acronym . ' ' . $singular_name;
      okhub_new_taxonomy($taxonomy_name, $taxonomy_label, $singular_name, TRUE);
      okhub_register_new_categories($taxonomy_name);
      // Add additional fields in the imported categories' 'edit' page.
      $form_fields_hook = $taxonomy_name . '_edit_form_fields';
      $new_metabox = 'okhub_' . $taxonomy . '_metabox_edit';
      add_action($form_fields_hook, $new_metabox, 10, 1);
      // Add additional columns in edit-tags.php.
      $manage_columns_filter = 'manage_edit-' . $taxonomy_name . '_columns';
      $manage_columns_action = 'manage_' . $taxonomy_name . '_custom_column';
      add_filter($manage_columns_filter, 'okhub_edit_categories_header', 10, 2);
      add_action($manage_columns_action, 'okhub_edit_categories_populate_rows', 10, 3);
    }
  }
  okhub_new_taxonomy(OKHUB_TAGS_TAXONOMY, 'OKHub Tags', 'OKHub Tag', FALSE);
  foreach (okhub_assets() as $short_type) {
    $post_type_name = okhub_custom_type_name($short_type); //'okhub_documents', 'okhub_organisations'
    register_taxonomy_for_object_type(OKHUB_TAGS_TAXONOMY, $post_type_name);
  }
}

function okhub_post_link($url) {
  global $post;
  $post_type = get_post_type($post);
  $site = get_query_var('okhub_source');
  if ((($post_type == 'okhub_documents') || ($post_type == 'okhub_organisations')) && ($new_path = okhubapi_variable_get('okhub', $post_type . '_path', ''))) {
    if ($site) {
      $new_path .= "/$site";
    }
    $url = str_replace("/$post_type", "/$new_path", $url);
  }
  elseif ($site) {
    $url = add_query_arg('okhub_source', $site, $url);
  }
  return $url;
}

function okhub_adjacent_post_join($join) {
  global $wpdb;
  $site = get_query_var('okhub_source');
  if ($site) {
    $join .= ", $wpdb->postmeta AS m";
  }
  return $join;
}

function okhub_adjacent_post_where($where) {
  $site = get_query_var('okhub_source');
  if ($site) {
    $where .= " AND p.ID = m.post_id AND m.meta_key='".OKHUB_WP_FN_HIDDEN_SOURCES."' AND m.meta_value='$site'";
  }
  return $where;
}

function okhub_pre_post_link($link) {
  $site = get_query_var('okhub_source');
  if ($site) {   
    $link .= "&okhub_source=$site";
  }
  return $link;
}

// If selected, include imported documents and organisations in the loop.
function okhub_include_assets_loop($query) {
  if ((is_home() && $query->is_main_query()) || is_category() || is_feed() ) {
    $post_types = $query->get('post_type');
    $okhub_include_imported_documents = okhubapi_variable_get('okhub', 'include_imported_documents', OKHUB_INCLUDE_IMPORTED_DOCUMENTS);
    $okhub_include_imported_organisations = okhubapi_variable_get('okhub', 'include_imported_organisations', OKHUB_INCLUDE_IMPORTED_ORGANISATIONS);
    if (empty($post_types) && ($okhub_include_imported_documents || $okhub_include_imported_organisations)) {
      $all_post_types = array('post');
      if ($okhub_include_imported_documents) {
        $all_post_types[] = 'okhub_documents';
      }
      if ($okhub_include_imported_organisations) {
        $all_post_types[] = 'okhub_organisations';
      }
      $query->set('post_type', $all_post_types);
    }
  }
}

function okhub_filter_imported_tags($request) {
    if ( isset($request['tag']) && !isset($request['post_type']) )
    $request['post_type'] = 'any';
    return $request;
} 

function okhub_query_vars($query_vars) {
  $query_vars[] = 'okhub_source';
  $query_vars[] = 'okhub_category';
  $query_vars[] = 'okhub_javascript';
  $query_vars[] = 'okhub_get_item';
  $query_vars[] = 'okhub_type';
  return $query_vars;
}

function okhub_filter_posts($query) {
  $site = (get_query_var('okhub_source')) ? get_query_var('okhub_source') : '';
  if ($site) {
    $query->set('meta_key', OKHUB_WP_FN_HIDDEN_SOURCES);
    $query->set('meta_value', $site);
  }
}

//---------------------- Admin interface (links, menus, etc -----------------------

// Add settings link
function okhub_add_options_page() {
  add_options_page('OKHub Content Settings Page', 'OKHub Content', 'manage_options', 'okhub', 'okhub_admin_main');
}

// Add menu
function okhub_add_menu() {
  $okhub_categories = okhub_categories();
  $okhub_assets = okhub_assets();
  $okhub_menu_title = 'OKHub Content';
  if (okhubapi_variable_get('okhub', 'api_key_validated', FALSE)) {
    $okhub_new_categories = okhubapi_variable_get('okhub', 'import_new_categories', OKHUB_NEW_CATEGORIES);
    $datasets = okhub_selected_datasets();
    add_menu_page('OKHub API', $okhub_menu_title, 'manage_options', 'okhub_menu', 'okhub_general_page', OKHUB_IMAGES_PATH . '/okhub.png');
    add_submenu_page( 'okhub_menu', 'Control panel', 'Control panel', 'manage_options', 'okhub_menu');
    add_submenu_page( 'okhub_menu', 'Settings', 'Settings', 'manage_options', 'options-general.php?page=okhub');
    add_submenu_page( 'okhub_menu', 'Importer', 'Importer', 'manage_options', 'admin.php?import=okhub_importer');
    foreach ($okhub_assets as $asset) {
      $name = 'OKHub'  . ' ' .  okhub_get_type_name($asset, TRUE);
      $type = okhub_custom_type_name($asset);
      add_submenu_page( 'okhub_menu', $name, $name, 'manage_options', 'edit.php?post_type='.$type);
    }
  }
  else {
    add_menu_page('OKHub API', $okhub_menu_title, 'manage_options', 'okhub_menu', 'okhub_admin_main', OKHUB_IMAGES_PATH . '/okhub.png');
  }
  add_submenu_page( 'okhub_menu', 'Help', 'Help', 'manage_options', 'okhub_help', 'okhub_help_page');
}

// Remove links to OKHub categories from the posts admin menu.
// Not necessary if taxonomies are not registered for regular posts.
function okhub_remove_submenu_pages() {
  $okhub_categories = okhub_categories();
  $okhub_datasets = okhub_datasets();
  foreach ($okhub_datasets as $dataset) {
    foreach ($okhub_categories as $category) {
      $link = 'edit-tags.php?taxonomy=' . okhub_custom_taxonomy_name($dataset, $category);
      remove_submenu_page( 'edit.php', $link );
    }
  }
}

// Display a 'Settings' link on the main Plugins page
function okhub_plugin_action_links($links, $file) {
	if ($file == plugin_basename(dirname(__FILE__))) {
		$okhubapi_links = '<a href="' . get_admin_url() . 'options-general.php?page=okhub">' . __('Settings') . '</a>';
		array_unshift($links, $okhubapi_links);
	}
	return $links;
}

// Enqueue stylesheet. We keep separate functions as in the future we might want to use different stylesheets for each plugin.
function okhub_add_stylesheet() {
    wp_register_style('okhub_style', OKHUB_SCRIPTS_PATH . '/okhub.css');
    wp_enqueue_style('okhub_style');
}

// Enqueue stylesheet
function okhub_add_admin_stylesheet() {
    okhub_add_stylesheet();
    wp_register_style('okhub_chosen_style', OKHUB_SCRIPTS_PATH . '/chosen/chosen.css');
    wp_enqueue_style('okhub_chosen_style');
    wp_register_style('okhub_jqwidgets_style', OKHUB_SCRIPTS_PATH . '/jqwidgets/styles/jqx.base.css');
    wp_enqueue_style('okhub_jqwidgets_style');
}

// Enqueue javascript
function okhub_add_javascript($hook) {
  $okhub_datasets = okhub_datasets();
  if ($hook == 'settings_page_okhub') { // Only in the plugin's admin page.
    wp_print_scripts( 'jquery' );
    wp_print_scripts( 'jquery-ui-tabs' );
    wp_register_script('okhub_chosen_javascript', OKHUB_SCRIPTS_PATH . '/chosen/chosen.jquery.js');
    wp_enqueue_script('okhub_chosen_javascript');
    wp_register_script('okhub_blockui_javascript', OKHUB_SCRIPTS_PATH . '/blockui/jquery.blockUI.js');
    wp_enqueue_script('okhub_blockui_javascript');    
    wp_register_script('okhub_jqwidgets_jqxcore_javascript', OKHUB_SCRIPTS_PATH . '/jqwidgets/jqwidgets/jqxcore.js');
    wp_enqueue_script('okhub_jqwidgets_jqxcore_javascript');
    wp_register_script('okhub_jqwidgets_jqxbuttons_javascript', OKHUB_SCRIPTS_PATH . '/jqwidgets/jqwidgets/jqxbuttons.js');
    wp_enqueue_script('okhub_jqwidgets_jqxbuttons_javascript');
    wp_register_script('okhub_jqwidgets_jqxdropdownbutton_javascript', OKHUB_SCRIPTS_PATH . '/jqwidgets/jqwidgets/jqxdropdownbutton.js');
    wp_enqueue_script('okhub_jqwidgets_jqxdropdownbutton_javascript');
    wp_register_script('okhub_jqwidgets_jqxscrollbar_javascript', OKHUB_SCRIPTS_PATH . '/jqwidgets/jqwidgets/jqxscrollbar.js');
    wp_enqueue_script('okhub_jqwidgets_jqxscrollbar_javascript');
    wp_register_script('okhub_jqwidgets_jqxpanel_javascript', OKHUB_SCRIPTS_PATH . '/jqwidgets/jqwidgets/jqxpanel.js');
    wp_enqueue_script('okhub_jqwidgets_jqxpanel_javascript');
    wp_register_script('okhub_jqwidgets_jqxtree_javascript', OKHUB_SCRIPTS_PATH . '/jqwidgets/jqwidgets/jqxtree.js');
    wp_enqueue_script('okhub_jqwidgets_jqxtree_javascript');
    wp_register_script('okhub_jqwidgets_jqxcheckbox_javascript', OKHUB_SCRIPTS_PATH . '/jqwidgets/jqwidgets/jqxcheckbox.js');
    wp_enqueue_script('okhub_jqwidgets_jqxcheckbox_javascript');
    wp_register_script('okhub_javascript', OKHUB_SCRIPTS_PATH . '/okhub.js');
    wp_enqueue_script('okhub_javascript');
    okhub_init_javascript();
  }
  elseif ($hook == 'widgets.php') {
    wp_register_script('okhub_widget_javascript', OKHUB_SCRIPTS_PATH . '/okhub_widget.js');
    wp_enqueue_script('okhub_widget_javascript');
  }
}



