<?php 

/*-------------------------------------  Generic template tags functions -------------------------------------*/

function okhub_get_sources() {
  $sources = array();
  $sources_names = okhub_datasets_names();
  if ($post_sources = get_post_meta(get_the_ID(), OKHUB_WP_FN_HIDDEN_SOURCES)) {
    foreach ($post_sources as $source_code) {
      if (!empty($sources_names[$source_code])) {
        $sources[$source_code] = $sources_names[$source_code];
      }
    }
  }
  return $sources;
}

// Prints the value (or values) of a field.
function okhub_field($field_name, $source='', $language='', $before='', $after='', $format='', $separator=', ') {
  $field = okhub_get_field($field_name, $source, $language, $format, $separator);
  if (is_scalar($field) && ($field !== '')) {
    echo $before . $field . $after;
  }
}

// Returns a string with the value (or values) of a field based on its name (joined by the separator).
function okhub_get_field($field_name, $source='', $language='', $format='', $separator=', ') {
  global $post;
  $fields = '';
  if ($source) {
    $field_name = $source . '_' . $field_name;
  }
  if ($language) {
    $field_name .= '_' . $language;
  }
  $fields = get_post_meta($post->ID, $field_name);
  if (!$language && $fields === array()) {
    $default_language = okhubapi_variable_get('okhub', 'default_language', OKHUB_DEFAULT_LANGUAGE);
    $additional_languages = okhubapi_variable_get('okhub', 'additional_languages', array());
    $languages = array_unique(array_merge(array($default_language), $additional_languages));
    foreach ($languages as $lang) {
      if ($fields = okhub_get_field($field_name, '', $lang, $format, $separator)) {
        break;
      }
    }
  }
  if (is_array($fields)) {
    $array_fields=array();
    foreach ($fields as $field) {
      $array_fields[] = okhub_format_field($field, $format);
    }
    $fields = implode($separator, $array_fields);
  }
  elseif ($fields) {
    $fields = okhub_format_field($fields, $format);
  }
  return $fields;
}

// Prints HTML with links to OKHub categories.
function okhub_terms($category, $source='', $before='', $after='', $separator=', ', $base_url='') {
  $categories = okhub_get_terms($category, $source, $before, $after, $separator, $base_url);
  if ($categories) {
    echo $categories;
  }
}

// Retrieves a string (with HTML format) with links to OKHub categories.
function okhub_get_terms($category, $source='', $before='', $after='', $separator=', ', $base_url='') {
  global $post;
  $taxonomy = "okhub_$category";
  $results = (isset($post->{$taxonomy}) ? $before. implode($separator, okhub_links_category($post->{$taxonomy}, $base_url)) . $after : okhub_get_list_categories($post->ID, $category, $source, $before, $separator, $after));
  if (!is_wp_error($results)) {
    return $results;
  }
  else {
    return '';
  }
}

function okhub_post_categories_source($source) {
  global $post;
  $categories = array();
  foreach (okhub_categories() as $category) {
    if ($terms = okhub_get_the_term_list($post->ID, $category, $source)) {
      $categories[$category] = $terms;
    }
  }
  return $categories;
}

function okhub_get_the_term_list($post_id, $category, $source, $before='', $sep='', $after='') {
  $terms ='';
  $taxonomy_name = okhub_custom_taxonomy_name($source, $category);
  if (taxonomy_exists($taxonomy_name)) {
    if ($list_terms = get_the_term_list($post_id, $taxonomy_name, $before, $sep, $after)) {
      $terms .= $list_terms;
    }
  }
  return $terms;
}

// Returns OKHub categories attached to post.
function okhub_get_list_categories($post_id, $category, $source, $before='', $sep='', $after='') {
  $terms='';
  if (!get_page_template_slug($post_id)) {
    if ($source) {
      $terms = okhub_get_the_term_list($post_id, $category, $source, $before, $sep, $after);
    }
    else {
      foreach (okhub_datasets_with_content() as $dataset) {
        $terms .= okhub_get_the_term_list($post_id, $category, $dataset, $before, $sep, $after);
      }
    }
  }
  return $terms;
}

/*----------------------------------------  Specific template tag functions -------------------------------- */
/*** These functions are deprecated and left only for backwards compatibility. Use the generic ones, above ***/

/*
// okhub_acronym
function okhub_acronym($before='', $after='') {
  $acronym = okhub_get_acronym();
  if ($acronym) {
    echo $before . $acronym . $after;
  }
}

// okhub_get_acronym
function okhub_get_acronym() {
  global $post;
  $acronym = (isset($post->acronym) ? $post->acronym : get_post_meta($post->ID, 'acronym', TRUE));
  return $acronym;
}

// okhub_location_country
function okhub_location_country($before='', $after='') {
  $location_country = okhub_get_location_country();
  if ($location_country) {
    echo $before . $location_country . $after;
  }
}

// okhub_get_location_country
function okhub_get_location_country() {
  global $post;
  $location_country = (isset($post->location_country) ? $post->location_country : get_post_meta($post->ID, 'location_country', TRUE));
  return $location_country;
}

// okhub_organisation_type
function okhub_organisation_type($before='', $after='') {
  $organisation_type = okhub_get_organisation_type();
  if ($organisation_type) {
    echo $before . $organisation_type . $after;
  }
}

// okhub_get_organisation_type
function okhub_get_organisation_type() {
  global $post;
  $organisation_type = (isset($post->organisation_type) ? $post->organisation_type : get_post_meta($post->ID, 'organisation_type', TRUE));
  return $organisation_type;
}

// okhub_authors
function okhub_authors($before='', $sep=', ', $after='') {
  $authors = okhub_get_authors();
  if ($authors) {
    echo $before . $authors . $after;
  }
}

// okhub_get_authors
function okhub_get_authors($sep=', ') {
  global $post;
  $authors = (isset($post->authors) ? $post->authors : get_post_meta($post->ID, 'authors', FALSE));
  if (is_array($authors)) {
    $authors = implode($sep, $authors);
  }
  return $authors;
}

// okhub_organisation_url
function okhub_organisation_url($before='', $after='') {
  $organisation_url = okhub_get_organisation_url();
  if ($organisation_url) {
    echo $before . okhub_new_html_link($organisation_url, $organisation_url) . $after;
  }
}

// okhub_get_organisation_url
function okhub_get_organisation_url() {
  global $post;
  $organisation_url = (isset($post->organisation_url) ? $post->organisation_url : get_post_meta($post->ID, 'organisation_url', TRUE));
  return $organisation_url;
}

// okhub_get_date_updated
function okhub_get_date_updated() {
  global $post;
  $date = (isset($post->date_updated) ? $post->date_updated : get_post_meta($post->ID, 'date_updated', TRUE));
  if ($date) {
    if (is_numeric($date)) { // for backwards compatibility.
      $date = date_i18n(get_option('date_format'), $date);
    }
  }
  return $date;
}

// okhub_date_created
function okhub_date_created($before='', $after='') {
  $date = okhub_get_date_created();
  if ($date) {
    echo $before . $date . $after;
  }
}

// okhub_get_date_created
function okhub_get_date_created() {
  global $post;
  $date = (isset($post->date_created) ? $post->date_created : get_post_meta($post->ID, 'date_created', TRUE));
  if ($date) {
    if (is_numeric($date)) { // for backwards compatibility.
      $date = date_i18n(get_option('date_format'), $date);
    }
  }
  return $date;
}

// okhub_date_updated
function okhub_date_updated($before='', $after='') {
  $date = okhub_get_date_updated();
  if ($date) {
    echo $before . $date . $after;
  }
}

// okhub_external_urls
function okhub_external_urls($before='', $sep=', ', $after='') {
  $urls = okhub_get_external_urls();
  if (is_array($urls)) {
    $urls = implode($sep, array_map('okhub_new_html_link', $urls, $urls));
  }
  else {
    $urls = okhub_new_html_link($urls, $urls);
  }
  if ($urls) {
    echo $before . $urls . $after;
  }
}

// okhub_get_external_urls
function okhub_get_external_urls() {
  global $post;
  $urls = (isset($post->urls) ? $post->urls : get_post_meta($post->ID, 'urls', FALSE));
  return $urls;
}


// okhub_countries
function okhub_countries($before='', $sep=', ', $after='', $base_url='') {
  $countries = okhub_get_countries($before, $sep, $after, $base_url);
  if ($countries) {
    echo $countries;
  }
}

// okhub_get_countries
function okhub_get_countries($before='', $sep=', ', $after='', $base_url='') {
  return okhub_categories_links('countries', $before='', $sep=', ', $after='', $base_url='');
}

// okhub_regions
function okhub_regions($before='', $sep=', ', $after='', $base_url='') {
  $regions = okhub_get_regions($before, $sep, $after, $base_url);
  if ($regions) {
    echo $regions;
  }
}

// okhub_get_regions
function okhub_get_regions($before='', $sep=', ', $after='', $base_url='') {
  return okhub_categories_links('regions', $before='', $sep=', ', $after='', $base_url='');
}

// okhub_themes
function okhub_themes($before='', $sep=', ', $after='', $base_url='') {
  $themes = okhub_get_themes($before, $sep, $after, $base_url);
  if ($themes) {
    echo $themes;
  }
}

// okhub_get_themes
function okhub_get_themes($before='', $sep=', ', $after='', $base_url='') {
  return okhub_categories_links('themes', $before='', $sep=', ', $after='', $base_url='');
}

*/

//-------------------- Functions used to generate/format the output --------------------

// Given a category object, returns a link to it.
function okhub_links_category($categories, $baseurl='') {
  global $post;
  $links = array();
  if (empty($baseurl)) {
    $baseurl = get_page_link($post->ID);
  }
  $par = ((preg_match('[\?]', $baseurl)) ? '&' : '?');
  foreach ($categories as $category) {
    if (isset($category->name) && isset ($category->object_type) && isset($category->object_id)) {
      $object_type = strtolower($category->object_type);
      $category_name = rtrim($category->name, " ($category->object_id)");
      $url = '<a href="'. $baseurl . $par . $object_type . "=$category->object_id" . '">' . $category_name . '</a>';
      $links[] = $url;
    }
  }
  return $links;
}

// okhub_format_field
function okhub_format_field($field, $format) {
  if (is_scalar($field)) {
    if (is_array($format)) {
      list($format_type, $format_data) = $format;
    }
    else {
      $format_type = $format;
      $format_data = '';
    }
    switch ($format_type) {
      case 'date':
        if (!is_numeric($field)) {
          $field = strtotime($field);
        }
        if (!$format_data) {
          $format_data = get_option('date_format');
        }
        $field = date_i18n($format_data, $field);
        break;
      case 'link':
        $field = okhub_new_html_link($field, $format_data);
        break;
      case 'tags':
        $terms = get_term_link($field, OKHUB_TAGS_TAXONOMY);
        if (is_string($terms)) {
          $field = okhub_new_html_link($terms, $field);
        }
        break;
    }
  }
  return $field;
}

// Create and return link
function okhub_new_html_link($link, $text='') {
  if (!$text) {
    $text = $link;
  }
  return '<a href="' . esc_url( $link ) . '" rel="tag">' . $text . '</a>';
}

// Generate array of categories that correspond to the indicated dataset.
// level indicates the number of levels to retrieve. 1 means only top-level categories. 0 means all categories, independently of their level.
function okhub_get_category_array($dataset, $category, $force_update=FALSE, $level=2, $padding_str='') {
  //$array_categories = array();
  $default_language = okhubapi_variable_get('okhub', 'default_language', OKHUB_DEFAULT_LANGUAGE); // TODO: Get currently selected language in settings.
  $var_name = 'okhub_categories_'.$dataset.'_'.$category.'_'.$default_language.'_'.$level;
  $array_categories = okhubapi_variable_get('okhub', $var_name, array());
  if (empty($array_categories) || $force_update) {
    $api_key = okhubapi_variable_get('okhub', 'api_key', '');
    $filter_params[OKHUB_API_PARAMETER_SOURCES] = $dataset;
    $extra_fields = array();
    $okhubapi = new OkhubApiWrapper;
    $show_children = FALSE;
    if (okhub_is_hierarchical($category)) {
      if ($level == 1) {
        $filter_params[OKHUB_API_PARAMETER_TOP_LEVEL] = 'true';
      }
      else {
        $show_children = TRUE;
        $hub_types = okhub_types();
        $hub_single_type = okhub_relitems_type($hub_types[$category]);
        $context_field = 'hub_'.$hub_single_type.'_context';
        $extra_fields[] = 'level';
        $extra_fields[] = $context_field;
      }
    }
    $response = $okhubapi->search($category, OKHUB_HUB_SOURCE_CODE, $api_key, 'short', 0, 0, 0, $filter_params, $extra_fields);
    if (!$response->isEmpty()) {
      $language_filters = okhubapi_variable_get('okhub', 'language_filters_codes', array_keys(okhub_languages())); 
      $preferred_languages = array_unique(array_merge(array($default_language), $language_filters, array_keys(okhub_languages())));
      $results = $response->getResults();
      foreach ($results as $object) {
        $item_id = okhub_get_api_value($object, OKHUB_API_FN_ITEM_ID);
        $object_level = okhub_get_api_value($object, OKHUB_API_FN_LEVEL, $dataset);
        if ($show_children) {
          if (!$level || !$object_level || $object_level <= $level) {
            if (!$title = okhub_get_api_value($object, $context_field, $dataset)) {
              $title_language = okhub_api_value_languages($object, OKHUB_API_FN_TITLE, $dataset, $preferred_languages);
              $title = current($title_language);
            }
          }
          else {
            $title = FALSE; // We exclude it.
          }
        }
        else {
          $title_language = okhub_api_value_languages($object, OKHUB_API_FN_TITLE, $dataset, $preferred_languages);
          $title = current($title_language);
        }
        if ($item_id && $title) {
          if ($show_children && $padding_str) {
            $padding = strlen($title) + strlen($padding_str)*($level - 1);
            $title = str_pad($title, $padding, $padding_str, STR_PAD_LEFT);
          }
          $array_categories[$item_id] = $title;
        }
      }
      asort($array_categories);
    }
    okhubapi_variable_set('okhub', $var_name, $array_categories);
    $categories_datasets = okhubapi_variable_get('okhub', 'categories_datasets', array()); // Keep track of saved categories.
    $categories_datasets[] = $var_name;
    okhubapi_variable_set('okhub', 'categories_datasets', $categories_datasets);
  }
  return $array_categories;
}

//-------------------- Generation of HTML and javascript code -------------------

// Shows a message to the user at the top of an admin page.
function okhub_show_message($message, $visibility='block') {
?>
  <div class="updated" style="display:<?php echo $visibility; ?>>;">
    <p>
    <?php
      echo $message;
    ?>
    </p>
  </div>
<?php
}

// Generate select boxes.
function  okhub_select_box($select_name, $id, $items, $selected_values = array(), $attributes=array()) {
	$html = "<select id='$id' name='$select_name'";
  if (!empty($attributes)) {
    foreach ($attributes as $attr_name => $attr_value) {
      $html .= ' ' . $attr_name . '="' . $attr_value . '"';
    }
  }
  $html .= '>';
	foreach($items as $key => $value) {
    $html .= "<option value='$key' ";
    if (in_array($key, $selected_values)) {
      $html .= 'selected';
    }
    $html .= '>';
    $html .= $value;
    $html .= '</option>';
	}
	$html .= '</select>';
  return $html;
}

// Generate input field.
function okhub_input_field($name, $size, $value, $id, $label, $description, $class_field, $required) {
	$html  = "<input name='$name' id='$id' class='$class_field' type='text' value='$value' size='$size' aria-required='$required' />";
	$html .= "<p class='description'>$description</p>";
  return $html;
}

// Generate <tr> field wrapper.
function okhub_tr_field_wrapper($class, $field_name, $label, $contents) {
	$html  = "<tr class='$class'>";
	$html .= "	<th scope='row' valign='top'>";
	$html .= "		<label for='$field_name'>$label</label>";
	$html .= "	</th>";
	$html .= "	<td>";
  $html .= "    $contents";
	$html .= "	</td>";
	$html .= "</tr>";
  return $html;
}

// Generate <div> field wrapper.
function okhub_div_field_wrapper($class, $field_name, $label, $contents) {
	$html  = "<div class='$class'>";
  if ($field_name && $label) {
    $html .= "	<label for='$field_name'>$label</label>";
  }
  $html .= "    $contents";
	$html .= "</div>";
  return $html;
}



// Creates javascript arrays from category arrays retrieved by the API.
function okhub_js_array($array_category) {
  $js_code = array();
  foreach ($array_category as $key => $value) {
    //$value = addslashes($value);
    $js_code[] = '["'.$key.'", "'.$value.'"]';
  }
  $js_array = "[" . implode(",", $js_code) . "]";
  return $js_array;
}

// Load themes to populate tree in the "settings" page. --- Not being used now.
function okhub_js_tree($type, $dataset, $api_key) {
  $default_language = okhubapi_variable_get('okhub', 'default_language', OKHUB_DEFAULT_LANGUAGE);
  $additional_languages = okhubapi_variable_get('okhub', 'additional_languages', array());
  $all_languages = okhub_languages();
  $preferred_languages = array_unique(array_merge(array($default_language), $additional_languages));
  $accepted_languages = array_unique(array_merge($preferred_languages, $all_languages));
  $okhubapi = new OkhubApiWrapper;
  $response = $okhubapi->search($type, OKHUB_HUB_SOURCE_CODE, $api_key, 'short', 0, 0, 0, array(OKHUB_API_PARAMETER_SOURCES => $dataset, OKHUB_API_PARAMETER_TOP_LEVEL => 'true'));
  $results = array();
  $url = plugins_url('scripts/okhub.themes.php', __FILE__);
  $url .= '?token_guid=' . $api_key . '&source=' . $dataset .'&type=' . $type;
  if (!$response->isError()) {
    $items = $response->getResults();
    foreach ($items as $item) {
      if ($titles = okhub_get_category_names_language($item, $dataset, $preferred_languages)) {
        $title = current($titles);
        $item_id = okhub_get_api_value($item, OKHUB_API_FN_ITEM_ID);
        if ($title && $item_id && is_scalar($title) && is_scalar($item_id)) {
          $results[] = "{ 'label': '$title', 'value': '$item_id', 'items': [{ 'value': '$url&id=$item_id', 'label': 'Loading...' }] }";
        }
      }
    }
  }
  $output = implode(",\n", $results);
  $output = "[\n" . $output . "\n]";
  return $output;
}

// Converts a PHP array into a javascript one.
function okhub_php2js_array($php_array = array()) {
  if ($js_array = implode("','", $php_array)) {
    $js_array = "['" . $js_array . "']";
  }
  else {
    $js_array = "[]";
  }
  return $js_array;
}

function okhub_php2js_objects($php_array = array()) {
  $objects = array();
  $values = array();
  foreach ($php_array as $element) {
    $relation = explode(',', $element);
    if (isset($relation[0]) && isset($relation[1])) {
      $values[$relation[0]][] = $relation[1];
    }   
  }
  foreach ($values as $value => $mapped) {
    $objects[] = '"'.$value.'":' . '["'.implode('","',$mapped).'"]';    
  }
  return "{" . implode(",", $objects) . "}";
}

// Initialize category arrays for each dataset in a javascript variable, default dataset and keep track of selected categories.
function okhub_init_javascript() {
  $api_key = okhubapi_variable_get('okhub', 'api_key', '');
  $api_key_validated = okhubapi_variable_get('okhub', 'api_key_validated', FALSE);
  $default_user = okhubapi_variable_get('okhub', 'import_user', OKHUB_IMPORT_USER);
  $okhub_datasets = okhub_datasets_with_content();
  $selected_datasets = implode("','", okhub_selected_datasets(array(OKHUB_DEFAULT_DATASET)));
  $selected_datasets = ($selected_datasets) ? "['$selected_datasets']" : '[]';
  $category_names = okhub_categories();
  $okhub_categories = implode("','", $category_names);
  $okhub_categories = ($okhub_categories) ? "['$okhub_categories']" : '[]';
  $datasets_array = implode("', '", $okhub_datasets);
  $datasets_array = "['$datasets_array']";
  $category_js_objects = implode("':[], '", $category_names);
  $category_js_objects = "{'$category_js_objects':[]}";
  $array_categories = array();
  $array_sel_categories = array();
  $selected_categories = array();
  $selected_categories_mappings = array();
  // Get current selections for filters and mappings.
  foreach ($okhub_datasets as $dataset) {
    $array_categories[] = "'$dataset':$category_js_objects";
    $array_sel_categories[] = "'$dataset':[]";
    $array_selected_categories[$dataset] = array();
    $array_categories_mappings[$dataset] = array();
    foreach ($category_names as $category) {
      if (okhub_has_content($dataset, $category)) {
        $array_selected_categories[$dataset][] = "'$category':".okhub_php2js_array(okhubapi_variable_get('okhub', $dataset .'_'.$category.'_'.'assets', array()));
        $array_categories_mappings[$dataset][] = "'$category':".okhub_php2js_objects(okhubapi_variable_get('okhub', $dataset .'_'.$category.'_'.'mappings', array()));
      }
    }
    if ($array_selected_categories[$dataset]) {
      $selected_categories[$dataset] = implode(', ', $array_selected_categories[$dataset]);
      $selected_categories[$dataset] = '{'.$selected_categories[$dataset].'}';
    }
    if ($array_categories_mappings[$dataset]) {
      $selected_categories_mappings[$dataset] = implode(', ', $array_categories_mappings[$dataset]);
      $selected_categories_mappings[$dataset] = '{'.$selected_categories_mappings[$dataset].'}';
    }
  }
  $okhub_array_categories = implode(",\n   ", $array_categories);
  $okhub_array_categories = "{\n   ".$okhub_array_categories."\n}";
  $sel_categories = implode(',', $array_sel_categories);
  $sel_categories = "{\n".$sel_categories."\n}";
  ?>
<script type="text/javascript">
// Initialization
var okhub_categories = <?php echo $okhub_categories; ?>;
var default_dataset = '<?php echo OKHUB_DEFAULT_DATASET; ?>';
var default_user = '<?php echo $default_user; ?>';
var okhub_plugin = 'okhub';
var datasets = <?php echo $datasets_array; ?>;
var selected_datasets = <?php echo $selected_datasets; ?>;
var home_url = '<?php echo get_site_url(); ?>';
var okhub_array_categories = [];
var okhub_array_categories_mappings = [];
var selected_categories_mappings = <?php echo $sel_categories; ?>;
var selected_categories = <?php echo $sel_categories; ?>;
var $jqorig = jQuery.noConflict();

function initCategoriesArrays() {
  //okhub_array_trees_themes = {'eldis':{}, 'bridge':{}};
  okhub_array_categories = <?php echo $okhub_array_categories; ?>;
  okhub_array_categories_mappings = <?php echo $okhub_array_categories; ?>;
  selected_categories_mappings;
  selected_categories;
  /* Arrays with previously selected category filters and mappings */
  <?php 
  foreach ($okhub_datasets as $dataset) {
    if (!empty($selected_categories[$dataset])) { ?>
  selected_categories['<?php echo $dataset; ?>'] = <?php echo $selected_categories[$dataset]; ?>;
  <?php
    }
    if (!empty($selected_categories_mappings[$dataset])) { ?>
  selected_categories_mappings['<?php echo $dataset; ?>'] = <?php echo $selected_categories_mappings[$dataset]; ?>;
  <?php
    }
  }
  ?>
}
</script>
<?php
}

function okhub_javascript_trigger() {
  if (get_query_var('okhub_javascript') && ($dataset=get_query_var('okhub_source')) && ($category=get_query_var('okhub_category'))) {
    $force_update = get_query_var('okhub_refresh');
    $categories = okhub_get_category_array($dataset, $category, $force_update);
    echo okhub_js_array($categories);
    exit;
  }
  elseif (($item_id=get_query_var('okhub_get_item')) && ($dataset=get_query_var('okhub_source')) && ($type=get_query_var('okhub_type'))) {
    OKHub_Widget::displayItem($item_id, $dataset, $type);
    exit;
  }
}

function okhub_init_javascript_widget() {
?>
<script type="text/javascript">
var home_url = '<?php echo get_site_url(); ?>';
var okhub_widget_assets = [];
</script>
<?php
}





