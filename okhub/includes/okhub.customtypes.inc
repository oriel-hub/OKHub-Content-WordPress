<?php

//----------------------------------- Custom post types ---------------------------

// Creates new post types for OKHub assets. - Currently, creating one post for each document / organisation in the OKHub (potentially, with multiple values for each source).
function okhub_post_types_init() {
  foreach (okhub_post_types() as $post_type => $singular_name) {
    $post_type_name = okhub_custom_type_name($post_type);
    $post_type_label = 'OKHub ' . okhub_get_type_name($post_type, TRUE);
    okhub_new_post_type($post_type_name, $post_type_label, $singular_name, TRUE);
  }
}

// Register a new post type.
function okhub_new_post_type($post_type_name, $post_type_label, $singular_name) {
  global $wp_rewrite;
  $okhub_datasets = okhub_datasets();
  $okhub_categories = okhub_categories();
  //$post_type_slug = okhubapi_variable_get('okhub', 'okhub_documents_path', '$post_type_name');
  if (!post_type_exists($post_type_name)) {
    $labels = array(
      'name' => _x( $post_type_label, 'post type general name' ),
      'singular_name' => _x( $singular_name, 'post type singular name' ),
      'add_new' => __( 'Add New ' ) . __( $singular_name ),
      'all_items' => __( $post_type_label ),
      'add_new_item' => __( 'Add New ' ) . __( $singular_name ),
      'edit_item' => __( 'Edit ' ) . __( $singular_name ),
      'new_item' => __( 'New ' ) . __( $singular_name ),
      'view_item' => __( 'View ' ) . __( $singular_name ),
      'search_items' => __( 'Search ' ) . __( $post_type_label ),
      'not_found' =>  __( 'No ' ) . __( $post_type_label  ) . __( ' found' ),
      'not_found_in_trash' => __( 'No ' ) . __( $post_type_label ) . __( ' found in trash' ),
      'parent_item_colon' => __( 'Parent ' ) . __( $singular_name  ) . ':',
      'menu_name' => __( $post_type_label )
    );
    $args = array(
      'labels' => $labels,
      'description' => __( 'Assets retrieved via the OKHub API'),
      'public' => true,
      'exclude_from_search' => false,
      'publicly_queryable' => true,
      'show_ui' => true, 
      'show_in_nav_menus' => true, 
      'show_in_menu' => false,
      'menu_icon' => '../images/icons32.png',
      'capability_type' => 'post',
      'hierarchical' => false,
      'supports' => array('title','editor','author','excerpt','custom-fields'),
      'has_archive' => true,
      'query_var' => true,
      'can_export' => true,
      //'rewrite' => array('slug' => $post_type_slug, 'with_front' => FALSE),
    );
    $args['taxonomies'] = array( 'post_tag', 'category' );
    register_post_type($post_type_name, $args);
    //$wp_rewrite->flush_rules();
  }
}

//----------------------------------- Custom taxonomies ---------------------------

// Create a new taxonomy.
function okhub_new_taxonomy($taxonomy_name, $taxonomy_label, $singular_name, $is_hierarchical) {
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
      array ('okhub_documents', 'okhub_organisations'),
      $args
    );
    $wp_rewrite->flush_rules();
  }
}

// Make the new taxonomies support existing types.
function okhub_register_new_categories($taxonomy_name) {
  $okhub_new_categories = okhubapi_variable_get('okhub', 'import_new_categories', OKHUB_NEW_CATEGORIES);
  if (($okhub_new_categories) && taxonomy_exists($taxonomy_name)) {
    okhub_register_taxonomy($taxonomy_name);
  }
}

function okhub_register_taxonomy($taxonomy_name) {
  register_taxonomy_for_object_type($taxonomy_name, 'post');
  foreach (okhub_assets() as $short_type) {
    $post_type_name = okhub_custom_type_name($short_type); //'okhub_documents', 'okhub_organisations'
    register_taxonomy_for_object_type($taxonomy_name, $post_type_name);
  }
  $post_types = get_post_types(array('_builtin' => false), 'objects');
  foreach ($post_types as $type) {
    $registered_taxonomies = get_object_taxonomies($type->name);
    if (in_array($taxonomy_name, $type->taxonomies) && !in_array($taxonomy_name, $registered_taxonomies)) {
      register_taxonomy_for_object_type($taxonomy_name, $type->name);
    }
  }
}

// When a custom term is deleted, it also deletes it's metadata.
function okhub_delete_term($term_id, $tt_id, $taxonomy) {
  if (preg_match('/'.OKHUB_CUSTOM_PREFIX.'_/', $taxonomy)) {
    okhub_delete_all_term_meta($term_id);
  }
}








