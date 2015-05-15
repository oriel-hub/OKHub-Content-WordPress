<?php
/*
Plugin Name: OKHub Import
Plugin URI: http://api.ids.ac.uk/category/plugins/
Description: Imports content from the OKHUB collection via the OKHub Site.
Version: 1.1
Author: Modified by Ronald Yacat for the Institute of Development Studies' Open Knowledge Hub (OKHUB) from the original IDS Knowledge Service plugin developed by Pablo Accousto.
Author URI: http://www.okhub.org/
License: GPLv3

    Copyright 2014  Institute of Development Studies - Open Knowledge Hub (OKHUB)

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
//error_reporting(-1);
if (!defined('OKHUB_API_ENVIRONMENT')) define('OKHUB_API_ENVIRONMENT', 'wordpress');

if (!defined('OKHUB_API_LIBRARY_PATH')) define('OKHUB_API_LIBRARY_PATH', dirname(dirname(__FILE__)) . '/okhubwrapper/');
if (file_exists(OKHUB_API_LIBRARY_PATH) && is_readable(OKHUB_API_LIBRARY_PATH)) {
  require_once(OKHUB_API_LIBRARY_PATH . 'okhubwrapper.wrapper.inc');
} else {
  wp_die(__('OKHUB Import: The OKHUB API library directory was not found or could not be read.'));
}

if (!defined('OKHUB_COMMON_FILES_PATH')) define('OKHUB_COMMON_FILES_PATH', dirname(dirname(__FILE__)) . '/okhubplugins_common/');
if (file_exists(OKHUB_COMMON_FILES_PATH) && is_readable(OKHUB_COMMON_FILES_PATH)) {
  require_once(OKHUB_COMMON_FILES_PATH . 'okhubplugins.customtypes.inc');
  require_once(OKHUB_COMMON_FILES_PATH . 'okhubplugins.functions.inc');
  require_once(OKHUB_COMMON_FILES_PATH . 'okhubplugins.html.inc');
} else {
  wp_die(__('OKHUB Import: A directory with shared files OKHUB plugins files was not found or could not be read.'));
}

require_once('okhubimport.includes/okhubimport.default.inc');
require_once('okhubimport.includes/okhubimport.interface.inc');
require_once('okhubimport.includes/okhubimport.metadata.inc');
require_once('okhubimport.includes/okhubimport.admin.inc');
require_once('okhubimport.includes/okhubimport.importer.inc');

//-------------------------------- Set-up hooks ---------------------------------

register_activation_hook(__FILE__, 'okhubimport_activate');
add_action('init', 'okhubimport_init');
add_action('admin_init', 'okhubimport_admin_init');
add_action('admin_menu', 'okhubimport_add_options_page');
add_action('admin_menu', 'okhubimport_add_menu', 9);
add_action('admin_menu', 'okhubimport_remove_submenu_pages');
add_action('admin_notices', 'okhubimport_admin_notices');
add_action('wp_enqueue_scripts', 'okhubimport_add_stylesheet');
add_action('admin_enqueue_scripts', 'okhubimport_add_admin_stylesheet');
add_action('admin_enqueue_scripts', 'okhubimport_add_javascript');
add_filter('plugin_action_links', 'okhubimport_plugin_action_links', 10, 2);
add_filter('manage_okhub_documents_posts_columns', 'okhubimport_posts_columns');
add_filter('manage_okhub_organisations_posts_columns', 'okhubimport_posts_columns');
add_action('manage_okhub_documents_posts_custom_column', 'okhubimport_populate_posts_columns');
add_action('manage_okhub_organisations_posts_custom_column', 'okhubimport_populate_posts_columns');
add_filter('manage_post_posts_columns', 'okhubimport_posts_columns');
add_action('manage_post_posts_custom_column', 'okhubimport_populate_posts_columns');
add_filter('cron_schedules', 'okhubimport_cron_intervals'); 
add_action('okhubimport_scheduled_events', 'okhubimport_run_periodic_imports', 10, 1);
add_filter('wp_get_object_terms', 'okhubimport_filter_get_object_terms');
add_filter('the_category', 'okhubimport_filter_the_category');
add_filter('single_term_title', 'okhubimport_filter_the_category');
add_filter('list_cats', 'okhubimport_filter_list_cats');
add_filter('request', 'okhubimport_filter_imported_tags');
add_filter('query_vars', 'okhubimport_query_vars');
add_action('pre_get_posts', 'okhubimport_include_idsassets_loop');
add_filter('pre_get_posts', 'okhubimport_filter_posts');
add_filter('get_term', 'okhubimport_filter_get_term');
add_filter('get_terms', 'okhubimport_filter_get_terms');
add_action('delete_term', 'okhubimport_delete_term', 10, 3);
add_filter('get_previous_post_where', 'okhubimport_adjacent_post_where');
add_filter('get_previous_post_join', 'okhubimport_adjacent_post_join');
add_filter('get_next_post_where', 'okhubimport_adjacent_post_where');
add_filter('get_next_post_join', 'okhubimport_adjacent_post_join');
add_action('generate_rewrite_rules', 'okhubimport_create_rewrite_rules');
add_filter('post_type_link', 'okhubimport_post_link');

//--------------------------- Set-up / init functions ----------------------------

// OKHUB Import plugin activation.
function okhubimport_activate() {
  $okhubapi = new OkhubApiWrapper;
  $okhubapi->cacheFlush();  
}

// Register custom types, taxonomies and importer.
function okhubimport_init() {
  global $wp_rewrite;
  // Register post types
  okhub_post_types_init();
  // Register taxonomies.
  okhubimport_taxonomies_init();
  // Register importer
  if (class_exists('OKHUB_Importer')) {
    $okhub_importer = new OKHUB_Importer();
    register_importer('okhubimport_importer', __('OKHUB Importer', 'okhubimport-importer'), __('Import posts from the OKHUB collection (Open Knowledge Hub).', 'okhubimport-importer'), array ($okhub_importer, 'dispatch'));
  }
  add_rewrite_tag('%okhub_site%', '([^&]+)');
  $changed_path_documents = okhubimport_changed_path('okhub_documents');
  //$changed_path_organisations = okhubimport_changed_path('okhub_organisations');
  if ($changed_path_documents ||  okhub_check_permalinks_changed('okhubimport')) {
    $wp_rewrite->flush_rules();
  }
}

// Clean up on deactivation
function okhubimport_deactivate() {
  global $wp_rewrite;
  $okhubapi = new OkhubApiWrapper;
  okhubimport_delete_plugin_options();
  okhubimport_unschedule_imports();
  $okhubapi->cacheFlush();
  $wp_rewrite->flush_rules();
}

// Clean up on uninstall
function okhubimport_uninstall() {
  okhubimport_delete_taxonomy_metadata();
}

// Delete options entries
function okhubimport_delete_plugin_options() {
	delete_option('okhubimport_options');
}

// Initialize the plugin's admin options
function okhubimport_admin_init(){
  register_setting('okhubimport', 'okhubimport_options', 'okhubimport_validate_options');
  $options = get_option('okhubimport_options');
  if(!is_array($options)) { // The options are corrupted.
    okhubimport_delete_plugin_options();
  }
  okhubimport_edit_post_form();
  register_deactivation_hook(dirname(__FILE__), 'okhubimport_deactivate');
  register_uninstall_hook(dirname(__FILE__), 'okhubimport_uninstall');
}

//------------------------ New post types and taxonomies -------------------------

// Create new custom taxonomies for the OKHUB categories.
function okhubimport_taxonomies_init() {
  global $okhub_datasets;
  $okhub_taxonomies = array('countries' => 'Country', 'regions' => 'Region', 'themes' => 'Theme');
  okhubimport_create_taxonomy_metadata();
  $default_dataset = okhubapi_variable_get('okhubimport', 'default_dataset', OKHUB_API_DEFAULT_DATASET);
  if ($default_dataset == 'all') {
    $datasets = $okhub_datasets;
  }
  else {
    $datasets = array($default_dataset);
  }
  foreach ($datasets as $dataset) {
    // "Regular" OKHUB taxonomies.
    foreach ($okhub_taxonomies as $taxonomy => $singular_name) {
      $taxonomy_name = $dataset . '_' . $taxonomy;
      $taxonomy_label = ucfirst($dataset) . ' ' . ucfirst($taxonomy);
      $singular_name = ucfirst($dataset) . ' ' . $singular_name;
      okhubimport_new_taxonomy($taxonomy_name, $taxonomy_label, $singular_name, TRUE);
      okhubimport_register_new_categories($taxonomy_name);
      // Add additional fields in the imported categories' 'edit' page.
      $form_fields_hook = $taxonomy_name . '_edit_form_fields';
      $new_metabox = 'okhubimport_' . $taxonomy . '_metabox_edit';
      add_action($form_fields_hook, $new_metabox, 10, 1);
      // Add additional columns in edit-tags.php.
      $manage_columns_filter = 'manage_edit-' . $taxonomy_name . '_columns';
      $manage_columns_action = 'manage_' . $taxonomy_name . '_custom_column';
      add_filter($manage_columns_filter, 'okhubimport_edit_categories_header', 10, 2);
      add_action($manage_columns_action, 'okhubimport_edit_categories_populate_rows', 10, 3);
    }
    // New document types taxonomies.    
    if ('new_taxonomy' == okhubapi_variable_get('okhubimport', 'import_document_type_'.$dataset, 'field')) {
      $document_type_taxonomy_name = okhubapi_variable_get('okhubimport', 'import_document_type_taxonomy_'.$dataset, $dataset.'_document_type');
      $document_type_taxonomy_label_singular = okhubapi_variable_get('okhubimport', 'import_document_type_taxonomy_label_singular_'.$dataset, ucfirst($dataset). ' document type');
      $document_type_taxonomy_label_plural = okhubapi_variable_get('okhubimport', 'import_document_type_taxonomy_label_plural_'.$dataset, ucfirst($dataset). ' document types');
      okhubimport_new_taxonomy($document_type_taxonomy_name, $document_type_taxonomy_label_plural, $document_type_taxonomy_label_singular, TRUE);
      okhubimport_register_taxonomy($document_type_taxonomy_name);
      $manage_columns_filter = 'manage_edit-' . $document_type_taxonomy_name . '_columns';
      $manage_columns_action = 'manage_' . $document_type_taxonomy_name . '_custom_column';
      add_filter($manage_columns_filter, 'okhubimport_edit_categories_header', 10, 2);
      add_action($manage_columns_action, 'okhubimport_edit_categories_populate_rows', 10, 3);
    }
  }
}

function okhubimport_post_link($url) {
  global $post;
  $post_type = get_post_type($post);
  $site = get_query_var('okhub_site');
  if ((($post_type == 'okhub_documents') || ($post_type == 'okhub_organisations')) && ($new_path = okhubapi_variable_get('okhubimport', $post_type . '_path', ''))) {
    if ($site) {
      $new_path .= "/$site";
    }
    $url = str_replace("/$post_type", "/$new_path", $url);
  }
  elseif ($site) {
    $url = add_query_arg('okhub_site', $site, $url);
  }
  return $url;
}

function okhubimport_adjacent_post_join($join) {
  global $wpdb;
  $site = get_query_var('okhub_site');
  if ($site) {
    $join .= ", $wpdb->postmeta AS m";
  }
  return $join;
}

function okhubimport_adjacent_post_where($where) {
  $site = get_query_var('okhub_site');
  if ($site) {
    $where .= " AND p.ID = m.post_id AND m.meta_key='site' AND m.meta_value='$site'";
  }
  return $where;
}

function okhubimport_pre_post_link($link) {
  $site = get_query_var('okhub_site');
  if ($site) {   
    $link .= "&okhub_site=$site";
  }
  return $link;
}

function okhubimport_changed_path($post_type) {
  return (okhubapi_variable_get('okhubimport', $post_type . '_path', '') !== okhubapi_variable_get('okhubimport', $post_type . '_old_path', ''));
}

function okhubimport_create_rewrite_rules() {
   okhubimport_rewrite_path('okhub_documents');
   //okhubimport_rewrite_path('okhub_organisations');
}

function okhubimport_rewrite_path($post_type){
  global $wp_rewrite;
  global $okhub_datasets;
  $new_path = okhubapi_variable_get('okhubimport', $post_type . '_path', '');
  $permalink_structure = get_option('permalink_structure');
  if ($new_path && $permalink_structure) {
    $datasets = (okhubapi_variable_get('okhubimport', 'default_dataset', OKHUB_IMPORT_DEFAULT_DATASET_ADMIN) == 'all') ? implode('|', $okhub_datasets) : '';
    if ($datasets) {
      add_rewrite_rule("{$new_path}/({$datasets})/?$", "{$wp_rewrite->index}?post_type=$post_type" . '&okhub_site=$matches[1]', 'top');
      add_rewrite_rule("{$new_path}/({$datasets})/{$wp_rewrite->pagination_base}/([0-9]{1,})/?$", "{$wp_rewrite->index}?post_type=$post_type" . '&okhub_site=$matches[1]&paged=$matches[2]', 'top');
    }
    add_rewrite_rule("{$new_path}/?$", "{$wp_rewrite->index}?post_type=$post_type", 'top');
    add_rewrite_rule("{$new_path}/{$wp_rewrite->pagination_base}/([0-9]{1,})/?$", "{$wp_rewrite->index}?post_type=$post_type" . '&paged=$matches[1]', 'top');
    if ($wp_rewrite->feeds) {
      $feeds = '(' . trim(implode( '|', $wp_rewrite->feeds)) . ')';
      if ($datasets) {
        add_rewrite_rule("{$new_path}/({$datasets})/{$wp_rewrite->feed_base}/$feeds/?$", "{$wp_rewrite->index}?post_type=$post_type" . '&okhub_site=$matches[1]&feed=$matches[2]', 'top');
        add_rewrite_rule("{$new_path}/({$datasets})/$feeds/?$", "{$wp_rewrite->index}?post_type=$post_type" . '&okhub_site=$matches[1]&feed=$matches[2]', 'top');
      }
      if ($new_path !== $post_type) {
        add_rewrite_rule("{$new_path}/feed/$feeds/?$", "{$wp_rewrite->index}?post_type=$post_type" . '&feed=$matches[1]', 'top');
        add_rewrite_rule("{$new_path}/$feeds/?$", "{$wp_rewrite->index}?post_type=$post_type" . '&feed=$matches[1]', 'top');
      }
    }
    // These have to be the last ones.
    if ($datasets) {
      add_rewrite_rule("{$new_path}/({$datasets})/([a-z0-9\-]+)/?$", "{$wp_rewrite->index}?$post_type=" . '$matches[2]&okhub_site=$matches[1]', 'top');
    }
    if ($new_path !== $post_type) {
      add_rewrite_rule("{$new_path}/([a-z0-9\-]+)/?$", "{$wp_rewrite->index}?$post_type=" . '$matches[1]', 'top');
    }
  }
}

// When a custom term is deleted, it also deletes it's metadata.
function okhubimport_delete_term($term_id, $tt_id, $taxonomy) {
  if ($taxonomy == 'hub_countries' || $taxonomy == 'hub_regions' || $taxonomy == 'hub_themes') {
    okhubimport_delete_all_term_meta($term_id);
  }
}

// Deletes all imported terms of OKHUB categories.
function okhubimport_delete_taxonomy_terms($taxonomy) {
  $res = TRUE;
  $terms = get_categories(array('taxonomy' => $taxonomy, 'hide_empty' => FALSE));
	foreach ($terms as $term) {
	    if (isset($term->term_id)) {
	      $res = ($res && wp_delete_term($term->term_id, $taxonomy));
	    }
  }
  return $res;
}

// Create a new taxonomy.
function okhubimport_new_taxonomy($taxonomy_name, $taxonomy_label, $singular_name, $is_hierarchical) {
  global $wp_rewrite;
  if (!taxonomy_exists($taxonomy_name)) {
    $labels = array(
      'name' => _x( $taxonomy_label, 'taxonomy general name' ),
      'singular_name' => _x( $singular_name, 'taxonomy singular name' ),
      'search_items' =>  __( 'Search ') . __( $taxonomy_label ),
      'all_items' => __( 'All ') . __( $taxonomy_label ),
      'parent_item' => __( 'Parent ') . __( $singular_name ),
      'parent_item_colon' => __( 'Parent ') . __( $singular_name ) . ':',
      'edit_item' => __( 'Edit ') . __( $singular_name ), 
      'update_item' => __( 'Update ') . __( $singular_name ),
      'add_new_item' => __( 'Add New ') . __( $singular_name ),
      'new_item_name' => __( 'New ') . __( $singular_name ) . __( ' Name' ),
    );
    $args = array(
        'hierarchical' => $is_hierarchical,
        'labels' => $labels,
        'query_var' => true,
        'show_ui' => true,
        'show_in_nav_menus' => false,
        'show_in_menu' => false,
        'rewrite' => array('slug' => $taxonomy_name)
      );
    register_taxonomy(
      $taxonomy_name,
      array ('okhub_documents'),
      $args
    );
    $wp_rewrite->flush_rules();
  }
}

// Make the new taxonomies support existing types.
function okhubimport_register_new_categories($taxonomy_name) {
  $okhubimport_new_categories = okhubapi_variable_get('okhubimport', 'import_new_categories', OKHUB_IMPORT_NEW_CATEGORIES);
  if (($okhubimport_new_categories) && taxonomy_exists($taxonomy_name)) {
    okhubimport_register_taxonomy($taxonomy_name);
  }
}

function okhubimport_register_taxonomy($taxonomy_name) {
  $post_types = array('okhub_documents',  'post');
  foreach ($post_types as $post_type) {
    register_taxonomy_for_object_type($taxonomy_name, $post_type);
  }
  $post_types = get_post_types(array('_builtin' => false), 'objects');
  foreach ($post_types as $type) {
    $registered_taxonomies = get_object_taxonomies($type->name);
    if (in_array($taxonomy_name, $type->taxonomies) && !in_array($taxonomy_name, $registered_taxonomies)) {
      register_taxonomy_for_object_type($taxonomy_name, $type->name);
    }
  }
}


// Mark posts as pending.
function okhubimport_unpublish_assets($post_type, $dataset) {
  $res = TRUE;
  $posts = get_posts(array('numberposts' => -1, 'post_type' => $post_type, 'meta_key' => 'site', 'meta_value' => $dataset));
  $new_post = array();
	foreach ($posts as $post) {
    $new_post['ID'] = $post->ID;
    $new_post['post_status'] = 'pending';
    $res  = $res && wp_update_post($new_post);
  }
  return $res;
}

// If selected, include imported documents and organisations in the loop.
function okhubimport_include_idsassets_loop($query) {
  if ((is_home() && $query->is_main_query()) || is_category() || is_feed() ) {
    $post_types = $query->get('post_type');
    $okhubimport_include_imported_documents = okhubapi_variable_get('okhubimport', 'include_imported_documents', OKHUB_IMPORT_INCLUDE_IMPORTED_DOCUMENTS);
    //$okhubimport_include_imported_organisations = okhubapi_variable_get('okhubimport', 'include_imported_organisations', OKHUB_IMPORT_INCLUDE_IMPORTED_ORGANISATIONS);
    if (empty($post_types) && ($okhubimport_include_imported_documents || $okhubimport_include_imported_organisations)) {
      $all_post_types = array('post');
      if ($okhubimport_include_imported_documents) {
        $all_post_types[] = 'okhub_documents';
      }
      /*if ($okhubimport_include_imported_organisations) {
        $all_post_types[] = 'okhub_organisations';
      }*/
      $query->set('post_type', $all_post_types);
    }
  }
}

// Removes "(object_id") from the category names before displaying them.
function okhubimport_filter_get_object_terms($categories) {
  $new_categories = array();
  foreach ($categories as $category) {
    if (is_object($category)) {
      if (preg_match('/[hub]/', $category->taxonomy)) {
        $category->name = okhubimport_filter_the_category($category->name);
      }
    }
    $new_categories[] = $category;
  }
  return $new_categories;
}

// Removes "(object_id") from the category names before displaying them.
function okhubimport_filter_get_term($term) {
  if (is_object($term)) {
    if (preg_match('/[hub]/', $term->taxonomy)) {
      $term->name = okhubimport_filter_the_category($term->name);
    }
  }
  return $term;
}

function okhubimport_filter_get_terms($terms) {
  $filtered_terms = array();
  foreach ($terms as $term) {
    $filtered_terms[] = okhubimport_filter_get_term($term);
  }
  return $filtered_terms;
}

function okhubimport_filter_list_cats($cat_name) {
  return okhubimport_filter_the_category($cat_name);
}

function okhubimport_filter_the_category($category_name) {
  $category_name = preg_replace( '/\s*\([AC](\d+)\)/', '', $category_name);
  return $category_name;
} 

function okhubimport_filter_imported_tags($request) {
    if ( isset($request['tag']) && !isset($request['post_type']) )
    $request['post_type'] = 'any';
    return $request;
} 

function okhubimport_query_vars($query_vars) {
  $query_vars[] = 'okhub_site';
  return $query_vars;
}

function okhubimport_filter_posts($query) {
  $site = (get_query_var('okhub_site')) ? get_query_var('okhub_site') : '';
  if ($site == 'hub') {
    $query->set('meta_key', 'site');
    $query->set('meta_value', $site);
  }
}

//---------------------- Admin interface (links, menus, etc -----------------------

// Add settings link
function okhubimport_add_options_page() {
  add_options_page('OKHUB Import Settings Page', 'OKHUB Import', 'manage_options', 'okhubimport', 'okhubimport_admin_main');
}

// Add menu
function okhubimport_add_menu() {
  global $okhub_categories;
  global $okhub_assets;
  $okhubimport_menu_title = okhubapi_variable_get('okhubimport', 'menu_title', 'OKHUB Import');
  if (okhubapi_variable_get('okhubimport', 'api_key_validated', FALSE)) {
    $okhubimport_new_categories = okhubapi_variable_get('okhubimport', 'import_new_categories', OKHUB_IMPORT_NEW_CATEGORIES);
    $datasets = okhubimport_display_datasets('admin');
    add_menu_page('OKHUB API', $okhubimport_menu_title, 'manage_options', 'okhubimport_menu', 'okhubimport_general_page', plugins_url(OKHUB_IMAGES_PATH . '/ids.png', dirname(__FILE__)));
    add_submenu_page( 'okhubimport_menu', 'OKHUB Import', 'OKHUB Import', 'manage_options', 'okhubimport_menu');
    add_submenu_page( 'okhubimport_menu', 'Settings', 'Settings', 'manage_options', 'options-general.php?page=okhubimport');
    add_submenu_page( 'okhubimport_menu', 'Importer', 'Importer', 'manage_options', 'admin.php?import=okhubimport_importer');
    if (in_array('hub', $datasets)){
      add_submenu_page( 'okhubimport_menu', 'All OKHUB Documents', 'All OKHUB Documents', 'manage_options', 'edit.php?post_type=okhub_documents');
    /*}
    if (in_array('hub', $datasets)){*/
      //add_submenu_page( 'okhubimport_menu', 'All OKHUB Organisations', 'All OKHUB Organisations', 'manage_options', 'edit.php?post_type=okhub_organisations');
    }
    foreach ($datasets as $dataset) {
      foreach ($okhub_assets as $asset) {
        if (!okhubapi_exclude($dataset, $asset)) {
          $name = ucfirst($dataset) . ' ' . ucfirst($asset);
          $function = 'edit.php?post_type=okhub_' . $asset . '&okhub_site=' . $dataset;
          add_submenu_page( 'okhubimport_menu', $name, $name, 'manage_options', $function);
        }
      }
      if ($okhubimport_new_categories) {
        foreach ($okhub_categories as $category) {
          if (!okhubapi_exclude($dataset, $category)) {
            $slug = $dataset . '_' . $category;
            $name = ucfirst($dataset) . ' ' . ucfirst($category);
            $function = 'okhubimport_' . $dataset . '_' . $category . '_page';
            add_submenu_page('okhubimport_menu', $name, $name, 'manage_options', $slug, $function);
          }
        }
      }
      if ('new_taxonomy' == okhubapi_variable_get('okhubimport', 'import_document_type_'.$dataset, 'field')) {
        $document_type_taxonomy_name = okhubapi_variable_get('okhubimport', 'import_document_type_taxonomy_'.$dataset, $dataset.'_document_type');
        $document_type_taxonomy_label_plural = okhubapi_variable_get('okhubimport', 'import_document_type_taxonomy_label_plural_'.$dataset, ucfirst($dataset). ' document type');
        $function = 'okhubimport_' . $dataset . '_document_type_page';
        add_submenu_page('okhubimport_menu', $document_type_taxonomy_label_plural, $document_type_taxonomy_label_plural, 'manage_options', $document_type_taxonomy_name, $function);
      }
    }
  }
  else {
    add_menu_page('OKHUB API', $okhubimport_menu_title, 'manage_options', 'okhubimport_menu', 'okhubimport_admin_main', plugins_url(OKHUB_IMAGES_PATH . '/ids.png', dirname(__FILE__)));
  }
  add_submenu_page( 'okhubimport_menu', 'Help', 'Help', 'manage_options', 'okhubimport_help', 'okhubimport_help_page');
}

// Remove links to OKHUB categories from the posts admin menu.
// Not necessary if taxonomies are not registered for regular posts.
function okhubimport_remove_submenu_pages() {
  global $okhub_categories;
  global $okhub_datasets;
  foreach ($okhub_datasets as $dataset) {
    foreach ($okhub_categories as $category) {
      $link = 'edit-tags.php?taxonomy=' . $dataset . '_' . $category;
      remove_submenu_page( 'edit.php', $link );
    }
  }
}

// Display a 'Settings' link on the main Plugins page
function okhubimport_plugin_action_links($links, $file) {
	if ($file == plugin_basename(dirname(__FILE__))) {
		$okhubapi_links = '<a href="' . get_admin_url() . 'options-general.php?page=okhubimport">' . __('Settings') . '</a>';
		array_unshift($links, $okhubapi_links);
	}
	return $links;
}

// Enqueue stylesheet. We keep separate functions as in the future we might want to use different stylesheets for each plugin.
function okhubimport_add_stylesheet() {
    wp_register_style('okhubimport_style', plugins_url(OKHUB_PLUGINS_SCRIPTS_PATH . 'okhubplugins.css', dirname(__FILE__)));
    wp_enqueue_style('okhubimport_style');
}

// Enqueue stylesheet
function okhubimport_add_admin_stylesheet() {
    okhubimport_add_stylesheet();
    wp_register_style('okhubimport_chosen_style', plugins_url(OKHUB_PLUGINS_SCRIPTS_PATH . 'chosen/chosen.css', dirname(__FILE__)));
    wp_enqueue_style('okhubimport_chosen_style');
    wp_register_style('okhubimport_jqwidgets_style', plugins_url(OKHUB_PLUGINS_SCRIPTS_PATH . 'jqwidgets/styles/jqx.base.css', dirname(__FILE__)));
    wp_enqueue_style('okhubimport_jqwidgets_style');
}

// Enqueue javascript
function okhubimport_add_javascript($hook) {
  global $okhub_datasets;
  if ($hook == 'settings_page_okhubimport') { // Only in the admin page.
    wp_print_scripts( 'jquery' );
    wp_print_scripts( 'jquery-ui-tabs' );
    wp_register_script('okhubimport_chosen_javascript', plugins_url(OKHUB_PLUGINS_SCRIPTS_PATH . 'chosen/chosen.jquery.js', dirname(__FILE__)));
    wp_enqueue_script('okhubimport_chosen_javascript');
    wp_register_script('okhubimport_jqwidgets_jqxcore_javascript', plugins_url(OKHUB_PLUGINS_SCRIPTS_PATH . 'jqwidgets/jqwidgets/jqxcore.js', dirname(__FILE__)));
    wp_enqueue_script('okhubimport_jqwidgets_jqxcore_javascript');
    wp_register_script('okhubimport_jqwidgets_jqxbuttons_javascript', plugins_url(OKHUB_PLUGINS_SCRIPTS_PATH . 'jqwidgets/jqwidgets/jqxbuttons.js', dirname(__FILE__)));
    wp_enqueue_script('okhubimport_jqwidgets_jqxbuttons_javascript');
    wp_register_script('okhubimport_jqwidgets_jqxdropdownbutton_javascript', plugins_url(OKHUB_PLUGINS_SCRIPTS_PATH . 'jqwidgets/jqwidgets/jqxdropdownbutton.js', dirname(__FILE__)));
    wp_enqueue_script('okhubimport_jqwidgets_jqxdropdownbutton_javascript');
    wp_register_script('okhubimport_jqwidgets_jqxscrollbar_javascript', plugins_url(OKHUB_PLUGINS_SCRIPTS_PATH . 'jqwidgets/jqwidgets/jqxscrollbar.js', dirname(__FILE__)));
    wp_enqueue_script('okhubimport_jqwidgets_jqxscrollbar_javascript');
    wp_register_script('okhubimport_jqwidgets_jqxpanel_javascript', plugins_url(OKHUB_PLUGINS_SCRIPTS_PATH . 'jqwidgets/jqwidgets/jqxpanel.js', dirname(__FILE__)));
    wp_enqueue_script('okhubimport_jqwidgets_jqxpanel_javascript');
    wp_register_script('okhubimport_jqwidgets_jqxtree_javascript', plugins_url(OKHUB_PLUGINS_SCRIPTS_PATH . 'jqwidgets/jqwidgets/jqxtree.js', dirname(__FILE__)));
    wp_enqueue_script('okhubimport_jqwidgets_jqxtree_javascript');
    wp_register_script('okhubimport_jqwidgets_jqxcheckbox_javascript', plugins_url(OKHUB_PLUGINS_SCRIPTS_PATH . 'jqwidgets/jqwidgets/jqxcheckbox.js', dirname(__FILE__)));
    wp_enqueue_script('okhubimport_jqwidgets_jqxcheckbox_javascript');
    wp_register_script('okhubimport_javascript', plugins_url(OKHUB_PLUGINS_SCRIPTS_PATH . 'okhubplugins.js', dirname(__FILE__)));
    wp_enqueue_script('okhubimport_javascript');
    $api_key = okhubapi_variable_get('okhubimport', 'api_key', '');
    $api_key_validated = okhubapi_variable_get('okhubimport', 'api_key_validated', FALSE);
    $default_dataset = okhubapi_variable_get('okhubimport', 'default_dataset', OKHUB_API_DEFAULT_DATASET);
    foreach ($okhub_datasets as $dataset) {
      $countries[$dataset] = okhubapi_variable_get('okhubimport', $dataset . '_countries_assets', array());
      $regions[$dataset] = okhubapi_variable_get('okhubimport', $dataset . '_regions_assets', array());
      $themes[$dataset] = okhubapi_variable_get('okhubimport', $dataset . '_themes_assets', array());
      $countries_mappings[$dataset] = okhubapi_variable_get('okhubimport', $dataset . '_countries_mappings', array());
      $regions_mappings[$dataset] = okhubapi_variable_get('okhubimport', $dataset . '_regions_mappings', array());
      $themes_mappings[$dataset] = okhubapi_variable_get('okhubimport', $dataset . '_themes_mappings', array());
    }
    $categories = array('countries' => $countries, 'regions' => $regions, 'themes' => $themes);
    $categories_mappings = array('countries' => $countries_mappings, 'regions' => $regions_mappings, 'themes' => $themes_mappings);
    $default_user = okhubapi_variable_get('okhubimport', 'import_user', OKHUB_IMPORT_IMPORT_USER);
    okhub_init_javascript('okhubimport', $api_key, $api_key_validated, $default_dataset, $categories, $categories_mappings, $default_user);
  }
}
