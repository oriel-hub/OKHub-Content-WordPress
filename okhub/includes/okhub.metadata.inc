<?php

//---------------------------- Functions to handle metadata -------------------------------

// Given the type of object and a pair of metadata key and value, returns the corresponding object.
// Return NULL if not found.
// (Used to retrieve terms and posts by OKHub object id, for instance).
function okhub_get_id_by_meta($type, $meta_key, $meta_value) {
  $object_okhub = okhub_get_okhub_by_meta($type, $meta_key, $meta_value);
  if (!empty($object_okhub)) {
    return $object_okhub[0];
  }
  else {
    return NULL;
  }
}

// Given the type of object and a pair of metadata key and value, returns the all the found objects.
// Return NULL if not found.
function okhub_get_okhub_by_meta($type, $meta_key, $meta_value) {
  global $wpdb;
  $table = $wpdb->prefix . $type . 'meta';
  $id = $type . '_id';
  $select = "SELECT $id FROM $table WHERE meta_key = '%s' AND meta_value = '%s'";
  $object_okhub = $wpdb->get_col($wpdb->prepare($select, $meta_key, $meta_value));
  return $object_okhub;
}

// Add multiple-value metadata in posts.
function okhub_add_post_array_meta($post_id, $meta_key, $array_values) {
  foreach ($array_values as $meta_value) {
    add_post_meta($post_id, $meta_key, $meta_value, FALSE);
  }
}

// Update multiple-value metadata in posts.
function okhub_update_post_array_meta($post_id, $meta_key, $array_values) {
  delete_post_meta($post_id, $meta_key);
  foreach ($array_values as $meta_value) {
    add_post_meta($post_id, $meta_key, $meta_value, FALSE);
  }
}

//--------- New table and functions to handle OKHub categories fields as metadata -----------

// Create table for taxonomy metadata
function okhub_create_taxonomy_metadata() {
  global $wpdb;
  $table = $wpdb->prefix . OKHUB_TAXONOMY . 'meta';
  $charset_collate = '';	
	if (!empty($wpdb->charset))
		$charset_collate = "DEFAULT CHARACTER SET $wpdb->charset";
	if (!empty($wpdb->collate))
		$charset_collate .= " COLLATE $wpdb->collate";	
	$tables = $wpdb->get_results("show tables like '$table'");
  $taxonomy_id = OKHUB_TAXONOMY . '_id';
	if (!count($tables))
		$wpdb->query("CREATE TABLE $table (
			meta_id bigint(20) unsigned NOT NULL auto_increment,
			$taxonomy_id bigint(20) unsigned NOT NULL default '0',
			meta_key varchar(255) default NULL,
			meta_value longtext,
			PRIMARY KEY	(meta_id),
			KEY $taxonomy_id ($taxonomy_id),
			KEY meta_key (meta_key)
		) $charset_collate;");
	$wpdb->okhub_taxonomymeta = $table;
}

// Drops taxonomy metadata table
function okhub_delete_taxonomy_metadata() {
  global $wpdb;
  $table = $wpdb->prefix . OKHUB_TAXONOMY . 'meta';
  $sql = "DROP TABLE ". $table;
	$wpdb->query($sql);
}

// Add meta data field to a term.
function okhub_add_term_meta($term_id, $meta_key, $meta_value, $unique = false) {
	return add_metadata(OKHUB_TAXONOMY, $term_id, $meta_key, $meta_value, $unique);
}

// Remove metadata matching criteria from a term.
function okhub_delete_term_meta($term_id, $meta_key, $meta_value = '') {
	return delete_metadata(OKHUB_TAXONOMY, $term_id, $meta_key, $meta_value);
}

// Delete all metadata associated with a term.
function okhub_delete_all_term_meta($term_id) {
  global $wpdb;
  $table = $wpdb->prefix . OKHUB_TAXONOMY . 'meta';
  $id_field = OKHUB_TAXONOMY . '_id';
  $delete = "DELETE FROM $table WHERE $id_field = $term_id";
  $count = $wpdb->query($delete);
	return $count;
}

// Retrieve term meta field for a term.
function okhub_get_term_meta($term_id, $key, $single = false) {
	return get_metadata(OKHUB_TAXONOMY, $term_id, $key, $single);
}

// Update term meta field based on term ID.
function okhub_update_term_meta($term_id, $meta_key, $meta_value, $prev_value = '') {
	return update_metadata(OKHUB_TAXONOMY, $term_id, $meta_key, $meta_value, $prev_value);
}

/*
  See the filter and action hooks in okhub_taxonomies_init function in okhub.php
*/

// Add columns with number of documents/organisations linked to the custom taxonomy.
function okhub_edit_categories_header($columns) {
    //unset($columns['posts']);
    $columns['okhub_documents'] = 'OKHub Documents';
    $columns['okhub_organisations'] = 'OKHub Organisations';
    return $columns;
}

// For each term in the custom taxonomy, retrieve number of documents/organisations and display link to them.
function okhub_edit_categories_populate_rows($value, $column, $term_id) {
  $value = 0;
  if (isset($_GET['taxonomy'])) {
    $taxonomy = $_GET['taxonomy'];
    $term = get_term($term_id, $taxonomy);
    $value = okhub_get_term_post_count_by_type($term_id, $taxonomy, $column);
    if (isset($term->slug)) {
      $term_slug = $term->slug;
      $link = 'edit.php?post_type='. $column . '&' . $taxonomy . '=' . $term_slug;
      $value = '<a href="' . $link . '">' . $value . '</a>';
    }
  }
  return $value;
}

// Get number of posts associated with a term.
function okhub_get_term_post_count_by_type($term_id, $taxonomy, $type) {
  $args = array( 
    'post_type' => $type, 
    'numberposts' => -1,
    'tax_query' => array(
        array(
            'taxonomy' => $taxonomy,
            'field' => 'id',
            'terms' => $term_id
        )
      )
   );
  $ps = get_posts($args);
  return count($ps);
}

/*
 TODO: Define custom fields for assets and categories externaly and retrieve them here.
 The label and description could be part of the OKHub API Wrapper and the type of widget and size could be defined in the OKHub Content plugin.
*/

// Fields common to all OKHub categories.
function okhub_general_metabox_edit($term) {
  /* Site */
  $site = okhub_get_term_meta($term->term_id, OKHUB_WP_FN_HUB_SOURCE, true);
  $input_field = okhub_input_field(OKHUB_WP_FN_HUB_SOURCE, 10, $site, 'source', __('Source'), __('Code of the OKHub source to which this term belongs'), 'input-metabox', 'true');
  echo okhub_tr_field_wrapper('form-field', 'source', __('Source'), $input_field);
  /* Item ID */
  $item_id = okhub_get_term_meta($term->term_id, OKHUB_WP_FN_HUB_ITEM_ID, true);
  $input_field = okhub_input_field(OKHUB_WP_FN_HUB_ITEM_ID, 10, $item_id, 'item_id', __('Item ID'), __('Unique identifier of this term in the OKHub'), 'input-metabox', 'true');
  echo okhub_tr_field_wrapper('form-field', 'item_id', __('Item ID'), $input_field);
  /* Metadata URL */
  $metadata_url = okhub_get_term_meta($term->term_id, OKHUB_WP_FN_HUB_METADATA_URL, true);
  $input_field = okhub_input_field(OKHUB_WP_FN_HUB_METADATA_URL, 40, $metadata_url, 'metadata_url', __('Metadata URL'), __('Web-accessible uri for this item in the OKHub'), 'input-metabox', 'true');
  echo okhub_tr_field_wrapper('form-field', 'metadata_url', __('Metadata URL'), $input_field);
  /* Timestamp */
  $timestamp = okhub_get_term_meta($term->term_id, OKHUB_WP_FN_HUB_TIMESTAMP, true);
  $input_field = okhub_input_field(OKHUB_WP_FN_HUB_TIMESTAMP, 20, $timestamp, 'timestamp', __('Timestamp'), __('Indicates when record was indexed in the API'), 'input-metabox', 'true');
  echo okhub_tr_field_wrapper('form-field', 'timestamp', __('Timestamp'), $input_field);
}

// Fields present in regions and themes. If they are not set, okhub_get_term_meta gets an empty string.
function okhub_common_metabox_edit($term) {
  /* Archived */
  $archived = okhub_get_term_meta($term->term_id, 'archived', true);
  $input_field = okhub_input_field('archived', 10, $archived, 'archived', __('Archived'), __('The category is archived'), 'input-metabox', 'true');
  echo okhub_tr_field_wrapper('form-field', 'archived', __('Archived'), $input_field);
  /* Level */
  $level = okhub_get_term_meta($term->term_id, 'level', true);
  $input_field = okhub_input_field('level', 10, $level, 'level', __('Level'), __('Position in the hierarchy, starting at 1, the top level'), 'input-metabox', 'true');
  echo okhub_tr_field_wrapper('form-field', 'level', __('Level'), $input_field);
  
  // TODO: UPDATE THIS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  /* Category parent */
  $cat_parent = okhub_get_term_meta($term->term_id, 'cat_parent', true);
  $input_field = okhub_input_field('cat_parent', 10, $cat_parent, 'cat_parent', __('Category parent'), __('Parent item id'), 'input-metabox', 'true');
  echo okhub_tr_field_wrapper('form-field', 'cat_parent', __('Category parent'), $input_field);
}

// Add fields to OKHub regions edit page.
function okhub_regions_metabox_edit($term) {
  okhub_general_metabox_edit($term);
  okhub_common_metabox_edit($term);
  /* Website URL */
  $website_url = okhub_get_term_meta($term->term_id, 'website_url', true);
  $input_field = okhub_input_field('website_url', 40, $website_url, 'website_url', __('Website URL'), __('Link to the category page on the site'), 'input-metabox', 'true');
  echo okhub_tr_field_wrapper('form-field', 'website_url', __('Website URL'), $input_field);
}

// Add fields to OKHub themes edit page.
function okhub_themes_metabox_edit($term) {
  okhub_general_metabox_edit($term);
  okhub_common_metabox_edit($term);
}

// Add fields to OKHub document types edit page.
function okhub_documenttypes_metabox_edit($term) {
  okhub_general_metabox_edit($term);
  okhub_common_metabox_edit($term);
}

// Add fields to OKHub countries edit page.
function okhub_countries_metabox_edit($term) {
  okhub_general_metabox_edit($term);
  /* Website URL */
  $website_url = okhub_get_term_meta($term->term_id, 'website_url', true);
  $input_field = okhub_input_field('website_url', 40, $website_url, 'website_url', __('Website URL'), __('Link to the category page on the site'), 'input-metabox', 'true');
  echo okhub_tr_field_wrapper('form-field', 'website_url', __('Website URL'), $input_field);
  /* Alternative name */
  $alternative_name = okhub_get_term_meta($term->term_id, 'alternative_name', true);
  $input_field = okhub_input_field('alternative_name', 40, $alternative_name, 'alternative_name', __('Alternative name'), __('Alternative name of country'), 'input-metabox', 'true');
  echo okhub_tr_field_wrapper('form-field', 'alternative_name', __('Alternative name'), $input_field);
  /* Asset ID */ //It's not being used?
  $asset_id = okhub_get_term_meta($term->term_id, 'asset_id', true);
  if ($asset_id) {
    $input_field = okhub_input_field('asset_id', 10, $asset_id, 'asset_id', __('Asset ID'), __('Country numeric identifier'), 'input-metabox', 'true');
    echo okhub_tr_field_wrapper('form-field', 'asset_id', __('Asset ID'), $input_field);
  }
  /* ISO number */
  $iso_number = okhub_get_term_meta($term->term_id, 'iso_number', true);
  $input_field = okhub_input_field('iso_number', 10, sprintf("%03d", $iso_number), 'iso_number', __('ISO number'), __('Numeric country code (ISO 3166-1)'), 'input-metabox', 'true');
  echo okhub_tr_field_wrapper('form-field', 'iso_number', __('ISO number'), $input_field);
  /* ISO three-letter code */
  $iso_three_letter_code = okhub_get_term_meta($term->term_id, 'iso_three_letter_code', true);
  $input_field = okhub_input_field('iso_three_letter_code', 10, $iso_three_letter_code, 'iso_three_letter_code', __('ISO three-letter code'), __('Three-letter country code (ISO 3166-1 alpha-3)'), 'input-metabox', 'true');
  echo okhub_tr_field_wrapper('form-field', 'iso_three_letter_code', __('ISO three-letter code'), $input_field);
  /* ISO two-letter code */
  $iso_two_letter_code = okhub_get_term_meta($term->term_id, 'iso_two_letter_code', true);
  $input_field = okhub_input_field('iso_two_letter_code', 10, $iso_two_letter_code, 'iso_two_letter_code', __('ISO two-letter code'), __('Two-letter country code (ISO 3166-1 alpha-2)'), 'input-metabox', 'true');
  echo okhub_tr_field_wrapper('form-field', 'iso_two_letter_code', __('ISO two-letter code'), $input_field);
  
  /* Regions */
  if ($dataset = okhub_get_term_meta($term->term_id, OKHUB_WP_FN_HUB_SOURCE, true)) {
    $regions_taxonomy_name = okhub_custom_taxonomy_name($dataset, 'regions');
    $all_regions = array_merge(array(0 => 'None'), get_categories(array('taxonomy' => $regions_taxonomy_name, 'hide_empty' => FALSE)));
    $regions_country = wp_get_object_terms( $term->term_id, $regions_taxonomy_name );
    $regions_select = array(0 => 'None');
    $selected_region = 0;
    if ((!is_wp_error($regions_country)) && (!empty($regions_country))) {
      $country_region = $regions_country[0];
      $selected_region = $country_region->term_id;
    }
    foreach ($all_regions as $region) {
      if (isset($region->name)) {
        //$regions_select[$region->term_id] = okhub_filter_the_category($region->name);
        $regions_select[$region->term_id] = $region->name;
      }
    }
    $select_field = okhub_select_box('category_region_array[]', 'category_region_array', $regions_select, array($selected_region));
    echo okhub_tr_field_wrapper('form-field', 'category_region_array', __('Region'), $select_field);
  }
}



