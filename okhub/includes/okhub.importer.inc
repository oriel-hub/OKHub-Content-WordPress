<?php

// Load Importer API
require_once ABSPATH . 'wp-admin/includes/import.php';

if ( !class_exists( 'WP_Importer' ) ) {
	$class_wp_importer = ABSPATH . 'wp-admin/includes/class-wp-importer.php';
	if ( file_exists( $class_wp_importer ) ) {
		require_once $class_wp_importer;
  }
}

//------------------------ OkhubImporter class definition -------------------------

if ( class_exists( 'WP_Importer' ) ) {
class OkhubImporter extends WP_Importer {

  // Posts and taxonomy terms created/updated
  var $terms = array();

//--------------------------- User interface methods -----------------------------

  // Determine what to display to the user, depending on the step.
	function dispatch() {
		if (empty ($_GET['step'])) {
			$step = 0;
    }
		else  {
			$step = (int) $_GET['step'];
    }
		$this->header();
		switch ($step) {
			case 0:
				$this->greet();
				break;
			case 1:
        check_admin_referer('okhub_import', 'okhub_import_check_referrer');
        $importer_url = admin_url() . 'admin.php?import=okhub_importer';
        printf('<div class="okhub-title-output-importer">' . __('Go back to the <a href="%s">OKHub Importer</a>') . '</div>', $importer_url);
        $this->import();
        $this->display_results();
        okhubapi_report_errors();
				break;
		}
		$this->footer();
	}

  // Display header.
	function header() {
		echo '<div class="okhub-wrap">';
		screen_icon();
		echo '<h2>'.__('OKHub Content - Importer', 'okhub-importer').'</h2>';
	}

  // Display footer.
	function footer() {
		echo '</div>';
	}

  // Display main import page. Selection of types of items to import.
	function greet() {
		$okhub_setup_done = okhubapi_variable_get('okhub', 'setup_done', FALSE);
		?>
		<div class="narrow">
		<?php 
		if (!$okhub_setup_done) {
      echo '<p>'.sprintf(__('Before starting to import OKHub content, you need you need to set a number of parameters, including the OKHub API key, at the <a href="%s">OKHub Plugin administrative page</a>.', 'okhub_importer'), 'options-general.php?page=okhub').'</p>'; 
		}
    else {
      okhub_importer_form();
    }
  }

  // Display a summary of the importing process.
  function display_results() {
    $total = 0;
    foreach ($this->terms as $dataset => $results_dataset) {
      $exclude = FALSE;
      $dataset_acronym = okhub_get_dataset_acronym($dataset);
      foreach ($results_dataset as $type => $results) {
        $type_name = okhub_get_type_name($type, TRUE);
        $exclude = !okhub_has_content($dataset, $type);
        if (!$exclude) {
          if ($results['new']) {
            $total += $results['new'];
            $output = __('Created ') . $results['new'] . ' ' . $dataset_acronym . ' ' . __($type_name);
            echo okhub_div_field_wrapper('notice notice-success okhub-results-import', '', '', $output);
          }
          if ($results['updated']) {
            $total += $results['updated'];
            $output = __('Updated ') . $results['updated'] . ' ' . $dataset_acronym . ' ' . __($type_name);
            echo okhub_div_field_wrapper('notice notice-success okhub-results-import', '', '', $output);
          }
          if (isset($results['skipped']) && $results['skipped']) {
            $output = __('Skipped ') . $results['skipped'] . ' ' . $dataset_acronym . ' ' . __($type_name);
            echo okhub_div_field_wrapper('notice notice-success okhub-results-import', '', '', $output);
          }
        }
        if (!$total && !$exclude) {
          $output = __('No '). strtolower($type_name) . __(' imported from ') . $dataset_acronym;
          echo okhub_div_field_wrapper('notice notice-success okhub-results-import', '', '', $output);
        }
      }
    }
  }
//-------------------------------- Import methods ------------------------------

  // Check which types of content to import and call to import methods. If selected, categories are imported first.
  // Keeps track of latest dates and results for each type, in order to display this information on the admin page (periodic imports do the same).
  function import() {
    $datasets = okhub_selected_datasets();
    $okhub_categories = okhub_categories();
    $okhub_assets = okhub_assets();
    $date = date(DATE_RFC822);
    $okhub_new_categories = okhubapi_variable_get('okhub', 'import_new_categories', OKHUB_NEW_CATEGORIES);
    $okhub_import_dates = okhubapi_variable_get('okhub', 'import_dates', array());
    $okhub_import_results = okhubapi_variable_get('okhub', 'import_results', array());
    $force_update = !empty($_POST['okhub_force_update']);
    foreach ($datasets as $dataset) {
      // Import selected categories (if the option is enabled).
      if ($okhub_new_categories) {
        foreach ($okhub_categories as $category) {
          $parameter_name = 'okhub_import_' . $category;
          if (!empty($_POST[$parameter_name])) {
            $this->import_categories($dataset, $category);
            @$okhub_import_dates[$dataset][$category] = $date;
            @$okhub_import_results[$dataset][$category] = $this->terms[$dataset][$category];
          }
        }
      }
      // Import selected assets.
      foreach ($okhub_assets as $asset) {
        $parameter_name = 'okhub_import_' . $asset;
        if (!empty($_POST[$parameter_name])) {
          $this->import_assets($dataset, $asset, $force_update);
          @$okhub_import_dates[$dataset][$asset] = $date;
          @$okhub_import_results[$dataset][$asset] = $this->terms[$dataset][$asset];
        }
      }
    }
    okhubapi_variable_set('okhub', 'import_dates', $okhub_import_dates);
    okhubapi_variable_set('okhub', 'import_results', $okhub_import_results);
  }

  // Import assets.
  function import_assets($dataset, $type, $force_update=FALSE) {
    $type_name = okhub_get_type_name($type, TRUE);
    $dataset_acronym = okhub_get_dataset_acronym($dataset);
    $default_language = okhubapi_variable_get('okhub', 'default_language', OKHUB_DEFAULT_LANGUAGE);
    $language_filters = okhubapi_variable_get('okhub', 'language_filters_codes', array_keys(okhub_languages())); 
    $preferred_languages = array_unique(array_merge(array($default_language), $language_filters));
 		$updated_assets = okhubapi_variable_get('okhub', 'updated_assets', OKHUB_UPDATED_ASSETS);
    $this->terms[$dataset][$type] = array('new' => 0, 'updated' => 0, 'skipped' => 0, 'duplicated' => 0);
    if (okhub_has_content($dataset, $type)) {
      if ($assets = $this->retrieve_content($dataset, $type)) {
        if (!empty($assets)) {
          $post_type = okhub_custom_type_name($type);
          if (post_type_exists($post_type)) {
            if (is_admin() && OKHUB_VERBOSE_IMPORT) {
              $output = __('Importing') . " $dataset_acronym " . __($type_name) . '...';
              echo okhub_div_field_wrapper('notice notice-success okhub-title-output-importer', '', '', $output);
              echo '<ul>';
            }
            foreach ($assets as $asset) {
              $titles = okhub_api_value_languages($asset, OKHUB_API_FN_TITLE, $dataset, $preferred_languages);
              $hub_id = okhub_get_api_value($asset, OKHUB_API_FN_ITEM_ID);
              if ($titles && $hub_id) {
                $title = current($titles);
                if ($post_id = okhub_get_id_by_meta('post', OKHUB_WP_FN_HUB_ITEM_ID, $hub_id)) { // The item already exists in WP (maybe not for the same dataset).
                  $post_status = get_post_status($post_id);
                  $post_sources = okhub_get_post_sources($post_id);
                  if (in_array($dataset, $post_sources) && ($post_status == 'pending' || $post_status == 'publish')) { // There's content for this source and the status is pending or publish.
                    // Compare latest hub timestamp in WP for the source with the hub timestamp in the API response.
                    // It's necessary to consider the source because the post could have been updated more recently for another source.
                    if (!$force_update) {
                      $old_hub_timestamp = get_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_HUB_TIMESTAMP, $dataset), TRUE);
                      if (!is_numeric($old_hub_timestamp)) {
                        $old_hub_timestamp = strtotime($old_hub_timestamp);
                      }
                      $current_hub_timestamp = okhub_get_api_value($asset, OKHUB_API_FN_TIMESTAMP);
                      if (!is_numeric($current_hub_timestamp)) {
                        $current_hub_timestamp = strtotime($current_hub_timestamp);
                      }
                      $force_update = (!$old_hub_timestamp || !$current_hub_timestamp || $current_hub_timestamp > $old_hub_timestamp);
                    }
                    if ($force_update) { // The date is not defined, the new version is more recent or the update was forced.
                      switch ($updated_assets) {
                        case 'overwrite':   
                          $post_id = okhub_update_asset($post_id, $dataset, $post_type, $asset);
                          if ($post_id) {
                            if (is_admin() && OKHUB_VERBOSE_IMPORT) {
                              $output = '<li> <a href="'.get_edit_post_link($post_id).'">'.$title.'</a>' . ' - ' . __('updated');
                              echo okhub_div_field_wrapper('okhub-item-output-importer', '', '', $output);
                            }
                            $this->terms[$dataset][$type]['updated']++;
                          }
                          else {
                            okhubapi_register_error('okhub', __('The post could not be updated for item with id ') . $hub_id, 'OkhubImporter::import_assets', 'warning');
                          }
                          break;
                        case 'skip':
                          if (is_admin() && OKHUB_VERBOSE_IMPORT) {
                            $output = '<li> <a href="'.get_edit_post_link($post_id).'">'.$title.'</a>' . ' - ' . __('skipped (most recent version already imported)');
                            echo okhub_div_field_wrapper('okhub-item-output-importer', '', '', $output);
                          }
                          $this->terms[$dataset][$type]['skipped']++;
                          break;
                      }
                    }
                    else { // There's content for this source and the version being imported is not more recent and it's currently published or pending. We always skip it in this case.
                      if (is_admin() && OKHUB_VERBOSE_IMPORT) {
                        $output = '<li> <a href="'.get_edit_post_link($post_id).'">'.$title.'</a>' . ' - ' . __('skipped (most recent version already imported)');
                        echo okhub_div_field_wrapper('okhub-item-output-importer', '', '', $output);
                      }
                      $this->terms[$dataset][$type]['skipped']++;
                    }
                  }
                  else { // The existing version is not published or pending - or there's no content for this source. We update it.
                    if ($post_id = okhub_update_asset($post_id, $dataset, $post_type, $asset)) {
                      if (is_admin() && OKHUB_VERBOSE_IMPORT) {
                        $output = '<li> <a href="'.get_edit_post_link($post_id).'">'.$title.'</a>' . ' - ' . __(' updated');
                        echo okhub_div_field_wrapper('okhub-item-output-importer', '', '', $output);
                      }
                      $this->terms[$dataset][$type]['updated']++;
                    }
                    else {
                      okhubapi_register_error('okhub', __('The post could not be updated for item with id ') . $hub_id, 'OkhubImporter::import_assets', 'warning');
                    }
                  }
                }
                else { // The item does not exist. Create it.
                  if ($post_id = okhub_insert_asset($dataset, $post_type, $asset)) {
                    if (is_admin() && OKHUB_VERBOSE_IMPORT) {
                      $output = '<li> <a href="'.get_edit_post_link($post_id).'">'.$title.'</a>' . ' - ' . __('new');
                      echo okhub_div_field_wrapper('okhub-item-output-importer', '', '', $output);
                    }
                    $this->terms[$dataset][$type]['new']++ ;
                  }
                  else {
                    okhubapi_register_error('okhub', __('A new post could not be created for item with id ') . $hub_id, 'OkhubImporter::import_assets', 'warning');
                  }
                }
              }
            }
            echo '</ul>';
          }
        }
        else {
          okhubapi_register_error('okhub', __('Sorry, we cannot find any content that matches your criteria. Please remove some filters or search more generally and try again.'), 'OkhubImporter::import_assets', 'notification', FALSE);
        }
      }
    }
    else {
      $output = __('No ') . __(strtolower($type_name)) . ' found in the hub for the source "' .$dataset_acronym. '"';
      echo okhub_div_field_wrapper('notice notice-warning okhub-title-output-importer', '', '', $output);
    }
  }

  // Import categories.
  function import_categories($dataset, $type) {
    $type_name = okhub_get_type_name($type, TRUE);
    $default_language = okhubapi_variable_get('okhub', 'default_language', OKHUB_DEFAULT_LANGUAGE);
    $language_filters = okhubapi_variable_get('okhub', 'language_filters_codes',  array_keys(okhub_languages())); 
    $preferred_languages = array_unique(array_merge(array($default_language), $language_filters));
    if (okhub_has_content($dataset, $type)) {
      $this->terms[$dataset][$type] = array('new' => 0, 'updated' => 0);
      if ($categories = $this->retrieve_content($dataset, $type)) {
        if (!empty($categories)) {
          $taxonomy_name = okhub_custom_taxonomy_name($dataset, $type);
          if (taxonomy_exists($taxonomy_name)) {
            $dataset_acronym = okhub_get_dataset_acronym($dataset);
            if (is_admin() && OKHUB_VERBOSE_IMPORT) {
              $output = __('Importing') . " $dataset_acronym " . __($type_name) . '...';
              echo okhub_div_field_wrapper('notice notice-success okhub-title-output-importer', '', '', $output);
            }
            echo '<ul>';
            foreach ($categories as $category) {
              $hub_id = okhub_get_api_value($category, OKHUB_API_FN_ITEM_ID);
              $category_names = okhub_get_category_names_language($category, $dataset, $preferred_languages);
              $category_name = current($category_names);
              if ($hub_id && $category_name) {
                if ($term_id = okhub_get_id_by_meta(OKHUB_TAXONOMY, OKHUB_WP_FN_HUB_ITEM_ID, $hub_id)) { // The taxonomy term exists.
                  // Update the term corresponding to the category.
                  if (is_admin() && OKHUB_VERBOSE_IMPORT) {
                    $output = '<li>' . $category_name . ' - ' . __('updated');
                    echo okhub_div_field_wrapper('okhub-item-output-importer', '', '', $output);
                  }
                  if (okhub_update_category($term_id, $dataset, $taxonomy_name, $category)) {
                    $this->terms[$dataset][$type]['updated']++;
                  }
                  else {
                    okhubapi_register_error('okhub', __('The term could not be updated for category with id ') . $hub_id, 'import_categories', 'warning');
                  }
                }
                else {
                  // Create a new term for the category.
                  if (is_admin() && OKHUB_VERBOSE_IMPORT) {
                    $output = '<li>' . $category_name . ' - ' . __('new');
                    echo okhub_div_field_wrapper('okhub-item-output-importer', '', '', $output);
                  }
                  $term_id = okhub_insert_category($dataset, $taxonomy_name, $category);
                  if ($term_id) {
                    $this->terms[$dataset][$type]['new']++ ;
                  }
                  else {
                    okhubapi_register_error('okhub', __('A new term could not be created for category with id ') . $hub_id, 'import_categories', 'warning');
                  }
                }
              }
            }
            echo '</ul>';
          }
        }
        else {
          okhubapi_register_error('okhub', __('No categories were retrieved by the API call.'), 'import_categories', 'warning');
        }
      }
    }
  }

  // Call the API to retrieve content of the indicated type, using the the plugin settings.
  function retrieve_content($dataset, $type) {
    $content = array();
    $okhubapi = new OkhubApiWrapper;
    $api_key = okhubapi_variable_get('okhub', 'api_key', '');
    $format = 'full';
    $filter_params = array();
    // Source filter
    $filter_params[OKHUB_API_PARAMETER_SOURCES] = $dataset;
    $is_asset = ($type == 'documents' || $type == 'organisations');
    if ($is_asset) {
      $num_requested = okhubapi_variable_get('okhub', 'num_items', OKHUB_API_NUM_ITEMS);
      $age_results = okhubapi_variable_get('okhub', 'age_new_assets', OKHUB_API_AGE_NEW_ASSETS); // Always 0 now as it's currently not supported and it's commented in the settings page.
      $language_filters = okhubapi_variable_get('okhub', 'language_filters_codes', array()); 
      $okhub_import_query = okhubapi_variable_get('okhub', 'import_query', '');
      $okhub_import_countries_assets = okhubapi_variable_get('okhub', $dataset . '_countries_assets', array());
      $okhub_import_regions_assets = okhubapi_variable_get('okhub', $dataset . '_regions_assets', array());
      $okhub_import_themes_assets = okhubapi_variable_get('okhub', $dataset . '_themes_assets', array());
      // Generic query
      if ($okhub_import_query) {
        $search_params = explode(',', $okhub_import_query);
        foreach ($search_params as $search_param) {
          $param = explode('=', trim($search_param));
          $key = $param[0];
          $value = $param[1];
          $filter_params[$key] = $value;
        }
      }
      // Category-based filters
      if (!empty($okhub_import_countries_assets)) {
        $filter_params[OKHUB_API_PARAMETER_COUNTRY_ID] = implode('|', $okhub_import_countries_assets);
      }
      if (!empty($okhub_import_regions_assets)) {
        $filter_params[OKHUB_API_PARAMETER_REGION_ID] = implode('|', $okhub_import_regions_assets);
      }
      if (!empty($okhub_import_themes_assets)) {
        $filter_params[OKHUB_API_PARAMETER_THEME_ID] = implode('|', $okhub_import_themes_assets);
      }
      // Document-specific filters
      if ($type == 'documents') {
        $okhub_import_authors_assets = okhubapi_variable_get('okhub', 'import_authors_assets', '');
        if ($okhub_import_authors_assets) {
          $authors = explode(',', $okhub_import_authors_assets);
          $list_authors = implode('|', array_map('trim', $authors));
          $filter_params[OKHUB_API_PARAMETER_AUTHOR] = $list_authors;
        }
        $okhub_import_publishers_assets = okhubapi_variable_get('okhub', 'import_publishers_assets', '');
        if ($okhub_import_publishers_assets) {
          $publishers = explode(',', $okhub_import_publishers_assets);
          $list_publishers = implode('|', array_map('trim', $publishers));
          $filter_params['publisher_name'] = $list_publishers;
        }
        // Only retrieve content in these languages.
        // The results are further refined (below) considering whether they have 'relevant' content (title, description) in these languages.
        if ($language_filters) {
          $language_codes = implode('|', $language_filters);
          $filter_params[OKHUB_API_PARAMETER_LANGUAGES] = $language_codes;
        }
      }
      // Keep only those assets whose URIs don't match the ones to exclude and that have 'relevant' content in the selected languages.
      // Paginate until we get the number requested.
      $num_responses = 0;
      $num_retrieved = 0;
      $total_results = 0;
      $offset = 0;
      $more_results = TRUE;
      $exclude_uris = okhubapi_variable_get('okhub', 'exclude_uris', '');
      while ($num_retrieved < $num_requested && $num_responses < OKHUB_HARD_LIMIT_ASSETS && $more_results) {
        $response = $okhubapi->search($type, OKHUB_HUB_SOURCE_CODE, $api_key, $format, $num_requested, $age_results, $offset, $filter_params);
        if ($response->isEmpty()) {
          $error_message = __('No content retrieved by the API call. Please check your active filters.');
          if ($response->isError()) {
            $error_message .= $response->getErrorMessage();
          }
          okhubapi_register_error('okhub', $error_message, 'OkhubImporter::retrieve_content', 'warning');
          $more_results = FALSE;
        }
        else {
          $assets_retrieved = $response->getResults();
          if (!empty($assets_retrieved)) {
            $num_responses += count($assets_retrieved);
            foreach ($assets_retrieved as $asset) {
              if (!okhub_exclude_asset($asset, $dataset, $exclude_uris) && okhub_has_language_content($asset, $dataset, $language_filters)) {
                $content[] = $asset;
                $num_retrieved++;
              }
              $offset++; 
              if ($num_retrieved == $num_requested) {
                break;
              }
            }
            $total_results = $response->getTotalResults();
            $more_results = ($total_results > $offset);
          }
          else {
            $more_results = FALSE;
          }
        }
      }
    } else { // is country, region, theme or document_type. - TODO: ADD PREFERRED/FILTER LANGUAGES FOR CATEGORY LABELS.
      if (okhub_is_hierarchical($type) && ($top_level_categories = okhubapi_variable_get('okhub', 'top_level_categories', OKHUB_DEFAULT_TOP_LEVEL))) {
          $filter_params[OKHUB_API_PARAMETER_TOP_LEVEL] = $top_level_categories;
      }
      $num_requested = 0;
      $age_results = 0;
      $start_offset = 0;
      $response = $okhubapi->search($type, OKHUB_HUB_SOURCE_CODE, $api_key, $format, $num_requested, $age_results, $start_offset, $filter_params);
      if ($response->isError()) {
        $error_message = __('No content retrieved by the API call. ') . $response->getErrorMessage();
        okhubapi_register_error('okhub', $error_message, 'OkhubImporter::retrieve_content', 'warning');
        $content = FALSE;
      }
      else {
        $content = $response->getResults();
      }
    }
    return $content;
  }

} // Class OkhubImport

} // If WP_Importer exists

//----------------------- End of OkhubImporter class definition -------------------------

//------------------------ Functions to manage imported content ------------------------

// Create new post with imported asset.
function okhub_insert_asset($dataset, $post_type, $asset) {
  // Get current settings for the importer.
  $import_keywords = okhubapi_variable_get('okhub', 'import_tags', OKHUB_TAGS);
  $default_language = okhubapi_variable_get('okhub', 'default_language', OKHUB_DEFAULT_LANGUAGE);
  $language_filters = okhubapi_variable_get('okhub', 'language_filters_codes',  array_keys(okhub_languages())); 
  $additional_languages = okhubapi_variable_get('okhub', 'additional_languages', array());
  // Accepted languages - in order of preference.
  $preferred_languages = array_unique(array_merge(array($default_language), $language_filters));
  $accepted_languages = array_unique(array_merge($preferred_languages, $additional_languages));
  $okhub_import_user_id = okhubapi_variable_get('okhub', 'import_user_id', '0');
  $okhub_api_key = okhubapi_variable_get('okhub', 'api_key', '');
  $okhub_pub_date = okhubapi_variable_get('okhub', 'pub_date', 'default');
  $post_status = okhubapi_variable_get('okhub', 'post_status', OKHUB_DEFAULT_POST_STATUS);
  // Add retrieved values to post.
  $post = array();
  // Post type (document, organisation)
  $post['post_type'] = $post_type;
  // Title
  $title_language = okhub_api_value_languages($asset, OKHUB_API_FN_TITLE, $dataset, $preferred_languages);
  $title = current($title_language);
  $title_lang = key($title_language);
  $post['post_title'] = $title;
  // Excerpt (description)
  $description_language = okhub_api_value_languages($asset, OKHUB_API_FN_DESCRIPTION, $dataset, $preferred_languages);
  $description = current($description_language);
  $description_lang = key($description_language);
  // Content (Description)
  $post['post_content'] = $description;
  $post['post_excerpt'] = okhub_excerpt_post($description);
  // Post status
  $post['post_status'] = $post_status;
  // Author
  if ($okhub_import_user_id) {
    $post['post_author'] = $okhub_import_user_id;
  }
  // Publication date
  if (!$hub_timestamp = okhub_get_api_value($asset, OKHUB_API_FN_TIMESTAMP)) {
    $hub_timestamp = okhub_get_api_value($asset, OKHUB_API_FN_INDEX_UPDATED);
  }
  if ($hub_timestamp && ($okhub_pub_date == 'date_created')) { 
    $post['post_date'] = date_i18n('Y-m-d H:i:s', strtotime($hub_timestamp));
  }
  if ($index_updated = okhub_get_api_value($asset, OKHUB_API_FN_INDEX_UPDATED) && ($okhub_pub_date == 'date_updated')) { 
    $post['post_date'] = date_i18n('Y-m-d H:i:s', strtotime($index_updated));
  }
  if ($post_id = wp_insert_post($post)) {
    // ---------- Hidden metadata - title, description, sources ----------
    add_post_meta($post_id, OKHUB_WP_FN_HIDDEN_TITLE_LANGUAGE, $title_lang, TRUE);
    add_post_meta($post_id, OKHUB_WP_FN_HIDDEN_CONTENT_LANGUAGE, $description_lang, TRUE);
    add_post_meta($post_id, OKHUB_WP_FN_HIDDEN_SOURCES, $dataset);
    // ---------- Hub fields ----------
    add_post_meta($post_id, OKHUB_WP_FN_HUB_ITEM_ID, okhub_get_api_value($asset, OKHUB_API_FN_ITEM_ID), TRUE);
    add_post_meta($post_id, OKHUB_WP_FN_HUB_METADATA_URL, okhub_get_api_value($asset, OKHUB_API_FN_METADATA_URL), TRUE);
    // We include the hub timestamp for each source to compare when importing.
    if ($hub_timestamp) { add_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_HUB_TIMESTAMP, $dataset), $hub_timestamp, TRUE); }
    // ---------- Source's fields ----------
    if ($object_id = okhub_get_api_value($asset, OKHUB_API_FN_OBJECT_ID, $dataset)) { add_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_OBJECT_ID, $dataset), $object_id, TRUE); }
    if ($website_url = okhub_get_api_value($asset, OKHUB_API_FN_WEBSITE_URL, $dataset)) { add_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_WEBSITE_URL, $dataset), $website_url, TRUE); }
    if ($date_created = okhub_get_api_value($asset, OKHUB_API_FN_DATE_CREATED, $dataset)) { add_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_DATE_CREATED, $dataset), $date_created, TRUE); }
    if ($date_updated = okhub_get_api_value($asset, OKHUB_API_FN_DATE_UPDATED, $dataset)) { add_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_DATE_UPDATED, $dataset), $date_updated, TRUE); }
    // ----- Categories -----
    if ($countries_ids = okhub_get_hub_relitems_field($asset, OKHUB_API_FN_COUNTRY_FOCUS_ARRAY, $dataset, OKHUB_API_FN_ITEM_ID)) { okhub_update_categories_post($post_id, $dataset, 'countries', $countries_ids); }
    if ($regions_ids = okhub_get_hub_relitems_field($asset, OKHUB_API_FN_REGION_ARRAY, $dataset, OKHUB_API_FN_ITEM_ID)) { okhub_update_categories_post($post_id, $dataset, 'regions', $regions_ids); }
    if ($themes_ids = okhub_get_hub_relitems_field($asset, OKHUB_API_FN_THEME_ARRAY, $dataset, OKHUB_API_FN_ITEM_ID)) { okhub_update_categories_post($post_id, $dataset, 'themes', $themes_ids); }
    if ($documenttypes_ids = okhub_get_hub_relitems_field($asset, OKHUB_API_FN_DOCUMENT_TYPE_ARRAY, $dataset, OKHUB_API_FN_ITEM_ID)) { okhub_update_categories_post($post_id, $dataset, 'documenttypes_ids', $themes_ids); }
    // ----- Add keywords as tags -----
    if ($import_keywords && $keywords_language = okhub_api_value_languages($asset, OKHUB_API_FN_KEYWORDS, $dataset, $accepted_languages, FALSE)) {
      foreach ($keywords_language as $keywords) {
        wp_set_post_tags($post_id, $keywords, TRUE);
      }
    }
    // ----- Document-specific fields -----
    if ($post_type == 'okhub_documents') {
      // -- Multi-valued --    TODO: Consider the possibility of serializing multiple values in one unique meta field.
      if ($authors = okhub_get_api_value($asset, OKHUB_API_FN_AUTHORS, $dataset, '', FALSE)) { okhub_add_post_array_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_AUTHORS, $dataset), $authors); }
      if ($urls = okhub_get_api_value($asset, OKHUB_API_FN_URLS, $dataset, '', FALSE)) { okhub_add_post_array_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_URLS, $dataset), $urls); }
      if ($language_names = okhub_get_api_value($asset, OKHUB_API_FN_LANGUAGE_NAME, $dataset, '', FALSE)) {
        foreach ($language_names as $language => $language_names_lang) {
          okhub_add_post_array_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_LANGUAGE_NAME, $dataset, $language), $language_names_lang); 
        }
      }
      // -- Single-valued --
      if ($publication_date = okhub_get_api_value($asset, OKHUB_API_FN_PUBLICATION_DATE, $dataset)) { add_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_PUBLICATION_DATE, $dataset), $publication_date, TRUE); }
      if ($publication_year = okhub_get_api_value($asset, OKHUB_API_FN_PUBLICATION_YEAR, $dataset)) { add_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_PUBLICATION_YEAR, $dataset), $publication_year, TRUE); }
      if ($licence_type = okhub_get_api_value($asset, OKHUB_API_FN_LICENCE_TYPE, $dataset)) { add_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_LICENSE_TYPE, $dataset), $licence_type, TRUE); }
      // These fields should in general be taxonomy terms, but they are not available as such in some sources, so if they are present we import them as fields anyway.
      // TODO: Get from related organisations and link to corresponding WP org if it exists.
      if ($publisher = okhub_get_api_value($asset, OKHUB_API_FN_PUBLISHER, $dataset)) { add_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_PUBLISHER, $dataset), $publisher, TRUE); }
      if ($publisher_country = okhub_get_api_value($asset, OKHUB_API_FN_PUBLISHER_COUNTRY, $dataset)) { add_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_PUBLISHER_COUNTRY, $dataset), $publisher_country, TRUE); }
      if ($document_type = okhub_get_api_value($asset, OKHUB_API_FN_DOCUMENT_TYPE, $dataset)) { add_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_DOCUMENT_TYPE, $dataset), $document_type, TRUE); }
    }
    // ----- Organisation-specific fields -----
    elseif ($post_type == 'okhub_organisations') {
      if ($acronym = okhub_get_api_value($asset, OKHUB_API_FN_ACRONYM, $dataset)) { add_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_ACRONYM, $dataset), $acronym, TRUE); }
      if ($alternative_acronym = okhub_get_api_value($asset, OKHUB_API_FN_ALTERNATIVE_ACRONYM, $dataset)) { add_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_ALTERNATIVE_ACRONYM, $dataset), $alternative_acronym, TRUE); }
      if ($organisation_type = okhub_get_api_value($asset, OKHUB_API_FN_ORGANISATION_TYPE, $dataset)) { add_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_ORGANISATION_TYPE, $dataset), $organisation_type, TRUE); }
      if ($role = okhub_get_api_value($asset, OKHUB_API_FN_ORGANISATION_ROLE, $dataset)) { add_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_ROLE, $dataset), $role, TRUE); }
      if ($organisation_url = okhub_get_api_value($asset, OKHUB_API_FN_ORGANISATION_URL, $dataset)) { add_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_ORGANISATION_URL, $dataset), $organisation_url, TRUE); }
      if ($location_country = okhub_get_api_value($asset, OKHUB_API_FN_LOCATION_COUNTRY, $dataset)) { add_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_LOCATION_COUNTRY, $dataset), $location_country, TRUE); }
    }
    // -- Add all languages versions for translatable fields (title/description) --
    $translatable_fields = okhub_get_translatable_fields($post_type);
    foreach ($accepted_languages as $lang_code) {
      foreach ($translatable_fields as $field) {
        $field_name = okhub_source_field_name($field, $dataset, $lang_code);
        $field_value = okhub_get_api_value($asset, $field, $dataset, $lang_code);
        if ($field_value) {
          if (is_scalar($field_value)) {
            add_post_meta($post_id, $field_name, $field_value, TRUE);
          }
          elseif (is_array($field_value)) {
            okhub_add_post_array_meta($post_id, $field_name, $field_value);
          }
        }
      }
    }
  }
  else {
    okhubapi_register_error('okhub', okhub_get_api_value($asset, OKHUB_API_FN_TITLE), 'okhub_insert_asset', 'warning');
  }
  return $post_id;
}

// Update post with imported asset.
function okhub_update_asset($post_id, $dataset, $post_type, $asset) {
  $import_keywords = okhubapi_variable_get('okhub', 'import_tags', '0');
  $additional_languages = okhubapi_variable_get('okhub', 'additional_languages', array());
  $okhub_import_user_id = okhubapi_variable_get('okhub', 'import_user_id', '0');
  $okhub_api_key = okhubapi_variable_get('okhub', 'api_key', '');
  $okhub_pub_date = okhubapi_variable_get('okhub', 'pub_date', 'default');
  $updated_post = array();
  $updated_post['ID'] = $post_id;
  $default_language = okhubapi_variable_get('okhub', 'default_language', OKHUB_DEFAULT_LANGUAGE);
  $language_filters = okhubapi_variable_get('okhub', 'language_filters_codes',  array_keys(okhub_languages())); 
  $additional_languages = okhubapi_variable_get('okhub', 'additional_languages', array());
  // Accepted languages - in order of preference.
  $preferred_languages = array_unique(array_merge(array($default_language), $language_filters));
  $accepted_languages = array_unique(array_merge($preferred_languages, $additional_languages));
  $title_language = okhub_api_value_languages($asset, OKHUB_API_FN_TITLE, $dataset, $preferred_languages);
  $title = current($title_language);
  $title_lang = key($title_language);
  $description_language = okhub_api_value_languages($asset, OKHUB_API_FN_DESCRIPTION, $dataset, $preferred_languages);
  $description = current($description_language);
  $description_lang = key($description_language);
  $current_title_lang = get_post_meta($post_id, OKHUB_WP_FN_HIDDEN_TITLE_LANGUAGE, TRUE);
  $current_content_lang = get_post_meta($post_id, OKHUB_WP_FN_HIDDEN_CONTENT_LANGUAGE, TRUE);
  $update_title = FALSE;
  $update_description = FALSE;
  if ($title_lang == $default_language && $current_title_lang <> $default_language) {
    $update_title = TRUE;
    $updated_post['post_title'] = $title;
  }
  if ($description_lang == $default_language && $current_content_lang <> $default_language) {
    $update_description = TRUE;
    $updated_post['post_excerpt'] = okhub_excerpt_post($description);
    $updated_post['post_content'] = $description;
  }
  if ($okhub_import_user_id) {
    $updated_post['post_author'] = $okhub_import_user_id;
  }
  // Publication date
  if (!$hub_timestamp = okhub_get_api_value($asset, OKHUB_API_FN_TIMESTAMP)) {
    $hub_timestamp = okhub_get_api_value($asset, OKHUB_API_FN_INDEX_UPDATED);
  }
  if ($hub_timestamp && ($okhub_pub_date == 'date_created')) { 
    $updated_post['post_date'] = date_i18n('Y-m-d H:i:s', strtotime($hub_timestamp));
  }
  if ($index_updated = okhub_get_api_value($asset, OKHUB_API_FN_INDEX_UPDATED) && ($okhub_pub_date == 'date_updated')) { 
    $updated_post['post_date'] = date_i18n('Y-m-d H:i:s', strtotime($index_updated));
  }
  if ($post_id = wp_update_post($updated_post)) {
    // ---------- Hidden metadata - title, description, sources ----------
    if ($update_title) { update_post_meta($post_id, OKHUB_WP_FN_HIDDEN_TITLE_LANGUAGE, $title_lang); }
    if ($update_description) { update_post_meta($post_id, OKHUB_WP_FN_HIDDEN_CONTENT_LANGUAGE, $description_lang); }
    add_post_meta($post_id, OKHUB_WP_FN_HIDDEN_SOURCES, $dataset);
    // ---------- Hub fields ----------
    //update_post_meta($post_id, OKHUB_WP_FN_HUB_ITEM_ID, okhub_get_api_value($asset, OKHUB_API_FN_ITEM_ID));
    update_post_meta($post_id, OKHUB_WP_FN_HUB_METADATA_URL, okhub_get_api_value($asset, OKHUB_API_FN_METADATA_URL));
    // Timestamp for source.
    if ($hub_timestamp) { update_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_HUB_TIMESTAMP, $dataset), $hub_timestamp); }
    // ---------- Source's fields ----------
    if ($object_id = okhub_get_api_value($asset, OKHUB_API_FN_OBJECT_ID, $dataset)) { update_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_OBJECT_ID, $dataset), $object_id); }
    if ($website_url = okhub_get_api_value($asset, OKHUB_API_FN_WEBSITE_URL, $dataset)) { update_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_WEBSITE_URL, $dataset), $website_url); }
    if ($date_created = okhub_get_api_value($asset, OKHUB_API_FN_DATE_CREATED, $dataset)) { update_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_DATE_CREATED, $dataset), $date_created); }
    if ($date_updated = okhub_get_api_value($asset, OKHUB_API_FN_DATE_UPDATED, $dataset)) { update_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_DATE_UPDATED, $dataset), $date_updated); }
    // ----- Categories -----
    if ($countries_ids = okhub_get_hub_relitems_field($asset, OKHUB_API_FN_COUNTRY_FOCUS_ARRAY, $dataset, OKHUB_API_FN_ITEM_ID)) { okhub_update_categories_post($post_id, $dataset, 'countries', $countries_ids); }
    if ($regions_ids = okhub_get_hub_relitems_field($asset, OKHUB_API_FN_REGION_ARRAY, $dataset, OKHUB_API_FN_ITEM_ID)) { okhub_update_categories_post($post_id, $dataset, 'regions', $regions_ids); }
    if ($themes_ids = okhub_get_hub_relitems_field($asset, OKHUB_API_FN_THEME_ARRAY, $dataset, OKHUB_API_FN_ITEM_ID)) { okhub_update_categories_post($post_id, $dataset, 'themes', $themes_ids); }
    if ($documenttypes_ids = okhub_get_hub_relitems_field($asset, OKHUB_API_FN_DOCUMENT_TYPE_ARRAY, $dataset, OKHUB_API_FN_ITEM_ID)) { okhub_update_categories_post($post_id, $dataset, 'documenttypes_ids', $themes_ids); }
    // ----- Add keywords as tags -----
    if ($import_keywords && $keywords_language = okhub_api_value_languages($asset, OKHUB_API_FN_KEYWORDS, $dataset, $accepted_languages, FALSE)) {
      foreach ($keywords_language as $keywords) {
        wp_set_post_tags($post_id, $keywords, TRUE);
      }
    }
    // ----- Document-specific fields -----
    if ($post_type == 'okhub_documents') {
      // -- Multi-valued --    TODO: Consider the possibility of serializing multiple values in one unique meta field.
      if ($authors = okhub_get_api_value($asset, OKHUB_API_FN_AUTHORS, $dataset, '', FALSE)) { okhub_update_post_array_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_AUTHORS, $dataset), $authors); }
      if ($urls = okhub_get_api_value($asset, OKHUB_API_FN_URLS, $dataset, '', FALSE)) { okhub_update_post_array_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_URLS, $dataset), $urls); }
      if ($language_names = okhub_get_api_value($asset, OKHUB_API_FN_LANGUAGE_NAME, $dataset, '', FALSE)) {
        foreach ($language_names as $language => $language_names_lang) {
          okhub_update_post_array_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_LANGUAGE_NAME, $dataset, $language), $language_names_lang); 
        }
      }
      // -- Single-valued --
      if ($publication_date = okhub_get_api_value($asset, OKHUB_API_FN_PUBLICATION_DATE, $dataset)) { update_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_PUBLICATION_DATE, $dataset), $publication_date); }
      if ($publication_year = okhub_get_api_value($asset, OKHUB_API_FN_PUBLICATION_YEAR, $dataset)) { update_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_PUBLICATION_YEAR, $dataset), $publication_year); }
      if ($licence_type = okhub_get_api_value($asset, OKHUB_API_FN_LICENCE_TYPE, $dataset)) { update_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_LICENSE_TYPE, $dataset), $licence_type); }
      if ($publisher = okhub_get_api_value($asset, OKHUB_API_FN_PUBLISHER, $dataset)) { update_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_PUBLISHER, $dataset), $publisher); }
      if ($publisher_country = okhub_get_api_value($asset, OKHUB_API_FN_PUBLISHER_COUNTRY, $dataset)) { update_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_PUBLISHER_COUNTRY, $dataset), $publisher_country); }
      if ($document_type = okhub_get_api_value($asset, OKHUB_API_FN_DOCUMENT_TYPE, $dataset)) { update_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_DOCUMENT_TYPE, $dataset), $document_type); }
    }
    // ----- Organisation-specific fields -----
    elseif ($post_type == 'okhub_organisations') {
      if ($acronym = okhub_get_api_value($asset, OKHUB_API_FN_ACRONYM, $dataset)) { update_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_ACRONYM, $dataset), $acronym); }
      if ($alternative_acronym = okhub_get_api_value($asset, OKHUB_API_FN_ALTERNATIVE_ACRONYM, $dataset)) { update_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_ALTERNATIVE_ACRONYM, $dataset), $alternative_acronym); }
      if ($organisation_type = okhub_get_api_value($asset, OKHUB_API_FN_ORGANISATION_TYPE, $dataset)) { update_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_ORGANISATION_TYPE, $dataset), $organisation_type); }
      if ($role = okhub_get_api_value($asset, OKHUB_API_FN_ORGANISATION_ROLE, $dataset)) { update_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_ROLE, $dataset), $role); }
      if ($organisation_url = okhub_get_api_value($asset, OKHUB_API_FN_ORGANISATION_URL, $dataset)) { update_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_ORGANISATION_URL, $dataset), $organisation_url); }
      if ($location_country = okhub_get_api_value($asset, OKHUB_API_FN_LOCATION_COUNTRY, $dataset)) { update_post_meta($post_id, okhub_source_field_name(OKHUB_WP_FN_LOCATION_COUNTRY, $dataset), $location_country); }
    }
    // -- Add all languages versions for translatable fields --
    $translatable_fields = okhub_get_translatable_fields($post_type);
    foreach ($accepted_languages as $lang_code) {
      foreach ($translatable_fields as $field) {
        $field_name = okhub_source_field_name($field, $dataset, $lang_code);
        $field_value = okhub_get_api_value($asset, $field, $dataset, $lang_code);
        if ($field_value) {
          if (is_scalar($field_value)) {
            update_post_meta($post_id, $field_name, $field_value);
          }
          elseif (is_array($field_value)) {
            okhub_update_post_array_meta($post_id, $field_name, $field_value);
          }
        }
      }
    }
  }
  else {
    okhubapi_register_error('okhub', okhub_get_api_value($asset, OKHUB_API_FN_TITLE), 'okhub_update_asset', 'warning');
  }
  return $post_id;
}

// Gets the category names in one of the preferred languages if available, looking into the retrieved fields.
function okhub_get_category_names_language($category, $dataset, $preferred_languages) {
  $category_names = array();
  if ($titles = okhub_api_value_languages($category, OKHUB_API_FN_TITLE, $dataset, $preferred_languages)) {
    $category_names = $titles;
  } elseif ($object_names = okhub_api_value_languages($category, OKHUB_API_FN_OBJECT_NAME, $dataset, $preferred_languages)) {
    $category_names = $object_names;
  } elseif ($names = okhub_api_value_languages($category, OKHUB_API_FN_NAME, $dataset, $preferred_languages)) {
    $category_names = $names;
  }
  return $category_names;
}

// Create new term with imported category.
// Returns the term id if everything goes ok, NULL if it could not be inserted.
// TODO - insert only categories not archived?
function okhub_insert_category($dataset, $taxonomy, $category) {
  $okhub_api_key = okhubapi_variable_get('okhub', 'api_key', '');
  $default_language = okhubapi_variable_get('okhub', 'default_language', OKHUB_DEFAULT_LANGUAGE);
  $language_filters = okhubapi_variable_get('okhub', 'language_filters_codes',  array_keys(okhub_languages())); 
  $additional_languages = okhubapi_variable_get('okhub', 'additional_languages', array());
  // Accepted languages - in order of preference.
  $preferred_languages = array_unique(array_merge(array($default_language), $language_filters));
  $accepted_languages = array_unique(array_merge($preferred_languages, $additional_languages));
  $description_lang = '';
  $description = '';
  $name_lang = $default_language;
  $term_id = NULL;
  // Add parent term if it exists
  $hub_type = okhub_get_api_value($category, OKHUB_API_FN_ITEM_TYPE);
  $parent_field_name = okhub_relitems_type($hub_type) . '_' . OKHUB_API_SUFFIX_PARENT_ARRAY;
  if ($parent_hub_id = current(okhub_get_hub_relitems_field($category, $parent_field_name, $dataset, OKHUB_API_FN_ITEM_ID))) {
    if ($terms_ids_parent = okhub_get_categories_terms_ids($dataset, $taxonomy, array($parent_hub_id))) {
      $term_args['parent'] = current($terms_ids_parent);
    }
  }
  if ($name_language = okhub_get_category_names_language($category, $dataset, $preferred_languages)) {
    if ($name = current($name_language)) {
      $name_lang = key($name_language);
      $term_args['name'] = $name;
      if ($description_language = okhub_api_value_languages($category, OKHUB_API_FN_DESCRIPTION, $dataset, $preferred_languages)) {
        if ($description = current($description_language)) {
          $description_lang = key($description_language);
          $term_args['description'] = $description;
        }
      }
      $new_term = wp_insert_term($term_args['name'], $taxonomy, $term_args);
      if (!is_wp_error($new_term)) {
        $term_id = $new_term['term_id'];
        // ---------- Hidden metadata - title, description, sources ----------
        okhub_add_term_meta($term_id, OKHUB_WP_FN_HIDDEN_NAME_LANGUAGE, $name_lang, TRUE);
        if ($description_lang) {
          okhub_add_term_meta($term_id, OKHUB_WP_FN_HIDDEN_DESCRIPTION_LANGUAGE, $description_lang, TRUE);
        }
        // ---------- Hub fields ----------
        okhub_add_term_meta($term_id, OKHUB_WP_FN_HUB_ITEM_ID, okhub_get_api_value($category, OKHUB_API_FN_ITEM_ID), TRUE);
        okhub_add_term_meta($term_id, OKHUB_WP_FN_HUB_ITEM_TYPE, $hub_type, TRUE);
        okhub_add_term_meta($term_id, OKHUB_WP_FN_HUB_TIMESTAMP, okhub_get_api_value($category, OKHUB_API_FN_TIMESTAMP), TRUE);
        okhub_add_term_meta($term_id, OKHUB_WP_FN_HUB_METADATA_URL, okhub_get_api_value($category, OKHUB_API_FN_METADATA_URL), TRUE);
        okhub_add_term_meta($term_id, OKHUB_WP_FN_HUB_SOURCE, $dataset, TRUE);
        okhub_add_term_meta($term_id, OKHUB_WP_FN_HUB_TOP_LEVEL, okhub_get_api_value($category, OKHUB_API_FN_TOP_LEVEL, OKHUB_HUB_SOURCE_CODE), TRUE);
        // ---------- Source's fields ----------
        if ($object_id = okhub_get_api_value($category, OKHUB_API_FN_OBJECT_ID, $dataset)) { okhub_add_term_meta($term_id, OKHUB_WP_FN_OBJECT_ID, $object_id, TRUE); }
        // ----- Country-specific fields -----
        if ($taxonomy == okhub_custom_taxonomy_name($dataset, 'countries')) {
          if ($alternative_name = okhub_get_api_value($category, OKHUB_API_FN_ALTERNATIVE_NAME, $dataset)) { okhub_add_term_meta($term_id, OKHUB_WP_FN_ALTERNATIVE_NAME, $alternative_name, TRUE); }
          if ($iso_number = okhub_get_api_value($category, OKHUB_API_FN_ISO_NUMBER, $dataset)) { okhub_add_term_meta($term_id, OKHUB_WP_FN_ISO_NUMBER, $iso_number, TRUE); }
          if ($iso_three_letter_code = okhub_get_api_value($category, OKHUB_API_FN_ISO_THREE_LETTER_CODE, $dataset)) { okhub_add_term_meta($term_id, OKHUB_WP_FN_ISO_THREE_LETTER_CODE, $iso_three_letter_code, TRUE); }
          if ($iso_two_letter_code = okhub_get_api_value($category, OKHUB_API_FN_ISO_TWO_LETTER_CODE, $dataset)) { okhub_add_term_meta($term_id, OKHUB_WP_FN_ISO_TWO_LETTER_CODE, $iso_two_letter_code, TRUE); }
          if ($regions_ids = okhub_get_hub_relitems_field($category, OKHUB_API_FN_REGION_ARRAY, $dataset, OKHUB_API_FN_ITEM_ID)) { okhub_update_categories_term($term_id, $dataset, okhub_custom_taxonomy_name($dataset,'regions'), $regions_ids); }
        // ----- Other taxonomies (region, theme, document type) fields -----
        } else {
          if ($archived = okhub_get_api_value($category, OKHUB_API_FN_ARCHIVED, $dataset)) { okhub_add_term_meta($term_id, OKHUB_WP_FN_ARCHIVED, $archived, TRUE); }
          if ($level = okhub_get_api_value($category, OKHUB_API_FN_LEVEL, $dataset)) { okhub_add_term_meta($term_id, OKHUB_WP_FN_LEVEL, $level, TRUE); }
          if ($cat_parent = okhub_get_api_value($category, OKHUB_API_FN_CAT_PARENT, $dataset)) { okhub_add_term_meta($term_id, OKHUB_WP_FN_CAT_PARENT, $cat_parent, TRUE); }
        }
        // TODO: Add "links" to terms in other OKHub taxonomies (in principle, it could be just the hub id).
      }
      else {
        okhubapi_register_error('okhub',  $new_term->get_error_message(), 'okhub_insert_category', 'warning');
      }
    }
  }
  return $term_id;
}

// Update term with imported category.
// Returns NULL if there was an error when updating.
function okhub_update_category($term_id, $dataset, $taxonomy, $category) {
  $okhub_api_key = okhubapi_variable_get('okhub', 'api_key', '');
  $default_language = okhubapi_variable_get('okhub', 'default_language', OKHUB_DEFAULT_LANGUAGE);
  $language_filters = okhubapi_variable_get('okhub', 'language_filters_codes',  array_keys(okhub_languages())); 
  $additional_languages = okhubapi_variable_get('okhub', 'additional_languages', array());
  // Accepted languages - in order of preference.
  $preferred_languages = array_unique(array_merge(array($default_language), $language_filters));
  $accepted_languages = array_unique(array_merge($preferred_languages, $additional_languages));
  $description_lang = '';
  $description = '';
  $name_lang = $default_language;
  $update_parent = FALSE;
  $new_term_id = NULL;
  $term_args = array();
  // Add/update parent term if it exists
  $hub_type = okhub_get_api_value($category, OKHUB_API_FN_ITEM_TYPE);
  $parent_field_name = okhub_relitems_type($hub_type) . '_' . OKHUB_API_SUFFIX_PARENT_ARRAY;
  if ($parent_hub_id = current(okhub_get_hub_relitems_field($category, $parent_field_name, $dataset, OKHUB_API_FN_ITEM_ID))) {
    if ($terms_ids_parent = okhub_get_categories_terms_ids($dataset, $taxonomy, array($parent_hub_id))) {
      $term_args['parent'] = current($terms_ids_parent);
    }
  }
  if ($name_language = okhub_get_category_names_language($category, $dataset, $preferred_languages)) {
    if ($name = current($name_language)) {
      $name_lang = key($name_language);
      $current_name_lang = okhub_get_term_meta($term_id, OKHUB_WP_FN_HIDDEN_NAME_LANGUAGE, TRUE);
      $update_name = FALSE;
      if ($name_lang == $default_language && $current_name_lang <> $default_language) {
        $update_name = TRUE;
        $term_args['name'] = $name;
      }
      $update_description = FALSE;
      if ($description_language = okhub_api_value_languages($category, OKHUB_API_FN_DESCRIPTION, $dataset, $preferred_languages)) {
        if ($description = current($description_language)) {
          $description_lang = key($description_language);
          $current_description_lang = get_post_meta($term_id, OKHUB_WP_FN_HIDDEN_DESCRIPTION_LANGUAGE, TRUE);
          if ($description_lang == $default_language && $current_description_lang <> $default_language) { // TODO: Consider source priority, if there is content for this source, etc.
            $update_description = TRUE;
            $term_args['description'] = $description;
          }
        }
      }
      $new_term  = wp_update_term($term_id, $taxonomy, $term_args);
      if (!is_wp_error($new_term)) {
        $new_term_id = $new_term['term_id'];
        // ---------- Hidden metadata ----------
        if ($update_name) { okhub_update_term_meta($term_id, OKHUB_WP_FN_HIDDEN_NAME_LANGUAGE, $name_lang, TRUE); }
        if ($update_description) { okhub_update_term_meta($term_id, OKHUB_WP_FN_HIDDEN_DESCRIPTION_LANGUAGE, $description_lang, TRUE); }
        // ---------- Hub fields ---------- // Unchanged, in general.
        okhub_update_term_meta($term_id, OKHUB_WP_FN_HUB_ITEM_ID, okhub_get_api_value($category, OKHUB_API_FN_ITEM_ID), TRUE);
        okhub_update_term_meta($term_id, OKHUB_WP_FN_HUB_ITEM_TYPE, $hub_type, TRUE);
        okhub_update_term_meta($term_id, OKHUB_WP_FN_HUB_TIMESTAMP, okhub_get_api_value($category, OKHUB_API_FN_TIMESTAMP), TRUE);
        okhub_update_term_meta($term_id, OKHUB_WP_FN_HUB_METADATA_URL, okhub_get_api_value($category, OKHUB_API_FN_METADATA_URL), TRUE);
        okhub_update_term_meta($term_id, OKHUB_WP_FN_HUB_SOURCE, $dataset, TRUE);
        okhub_update_term_meta($term_id, OKHUB_WP_FN_HUB_TOP_LEVEL, okhub_get_api_value($category, OKHUB_API_FN_TOP_LEVEL, OKHUB_HUB_SOURCE_CODE), TRUE);
        // ---------- Source's fields ----------
        if ($object_id = okhub_get_api_value($category, OKHUB_API_FN_OBJECT_ID, $dataset)) { okhub_update_term_meta($term_id, OKHUB_WP_FN_OBJECT_ID, $object_id, TRUE); }
        // ----- Country-specific fields -----
        if ($taxonomy == okhub_custom_taxonomy_name($dataset, 'countries')) {
          if ($alternative_name = okhub_get_api_value($category, OKHUB_API_FN_ALTERNATIVE_NAME, $dataset)) { okhub_update_term_meta($term_id, OKHUB_WP_FN_ALTERNATIVE_NAME, $alternative_name, TRUE); }
          if ($iso_number = okhub_get_api_value($category, OKHUB_API_FN_ISO_NUMBER, $dataset)) { okhub_update_term_meta($term_id, OKHUB_WP_FN_ISO_NUMBER, $iso_number, TRUE); }
          if ($iso_three_letter_code = okhub_get_api_value($category, OKHUB_API_FN_ISO_THREE_LETTER_CODE, $dataset)) { okhub_update_term_meta($term_id, OKHUB_WP_FN_ISO_THREE_LETTER_CODE, $iso_three_letter_code, TRUE); }
          if ($iso_two_letter_code = okhub_get_api_value($category, OKHUB_API_FN_ISO_TWO_LETTER_CODE, $dataset)) { okhub_update_term_meta($term_id, OKHUB_WP_FN_ISO_TWO_LETTER_CODE, $iso_two_letter_code, TRUE); }
          if ($regions_ids = okhub_get_hub_relitems_field($category, OKHUB_API_FN_REGION_ARRAY, $dataset, OKHUB_API_FN_ITEM_ID)) { okhub_update_categories_term($term_id, $dataset, okhub_custom_taxonomy_name($dataset,'regions'), $regions_ids); }
        // ----- Other taxonomies (region, theme, document type) fields -----
        } else {
          if ($archived = okhub_get_api_value($category, OKHUB_API_FN_ARCHIVED, $dataset)) { okhub_update_term_meta($term_id, OKHUB_WP_FN_ARCHIVED, $archived, TRUE); }
          if ($level = okhub_get_api_value($category, OKHUB_API_FN_LEVEL, $dataset)) { okhub_update_term_meta($term_id, OKHUB_WP_FN_LEVEL, $level, TRUE); }
          if ($cat_parent = okhub_get_api_value($category, OKHUB_API_FN_CAT_PARENT, $dataset)) { okhub_update_term_meta($term_id, OKHUB_WP_FN_CAT_PARENT, $cat_parent, TRUE); }
        }
      }
      else {
        //okhubapi_register_error('okhub',  __('Error updating category ') . $term_args['name'] . ': ' . $new_term->get_error_message(), 'okhub_update_category', 'warning');
        okhubapi_register_error('okhub',  $new_term->get_error_message(), 'okhub_update_category', 'warning');
      }  
    }
  }
  return $new_term_id;
}

// Relate a post with its OKHub categories. If the categories do not exist as terms, they are created.
// TODO. Get the arrays outside of this function. "Clean" previously assigned categories in the 'category' taxonomy in case they changed.
function okhub_update_categories_post($post_id, $dataset, $category_name, $categories_ids_array) {
  $okhub_new_categories = okhubapi_variable_get('okhub', 'import_new_categories', OKHUB_NEW_CATEGORIES);
  // Assign assets to new taxonomy terms corresponding to imported OKHub categories.
  if ($okhub_new_categories) {
    $taxonomy = okhub_custom_taxonomy_name($dataset, $category_name);
    $terms_okhub = okhub_get_categories_terms_ids($dataset, $taxonomy, $categories_ids_array);
    wp_set_post_terms($post_id, $terms_okhub, $taxonomy, FALSE);
  }
  // Assign assets to existing Wordpress categories (based on mappings).
  if ($mappings = okhub_get_mappings($dataset, $category_name)) {
    $tax_mappings = array();
    foreach ($categories_ids_array as $category_id) {
      if (isset($mappings[$category_id])) {
        $mappings_category = $mappings[$category_id];
        foreach ($mappings_category as $mapping) {
          $wp_term_id = '';
          $wp_taxonomy = '';
          if (preg_match('/(.+)\-(.+)$/', $mapping, $matches)) {
            if (isset($matches[1]) && isset($matches[2])) {
              $wp_taxonomy = $matches[1];
              $wp_term_id = $matches[2];
            }
          }
          if ($wp_taxonomy && $wp_term_id && is_numeric($wp_term_id)) {
            $tax_mappings[$wp_taxonomy][] = $wp_term_id;
          }
        }
      }
    }
    foreach ($tax_mappings as $tax_name => $mappings_okhub) {
      if (is_taxonomy_hierarchical($tax_name)) {
        wp_set_post_terms($post_id, $mappings_okhub, $tax_name, TRUE);
      }
      else {
        $mappings_names = array();
        foreach ($mappings_okhub as $mapping_id) {
          $mapped_term = get_term($mapping_id, $tax_name);
          $mappings_names[] = $mapped_term->name;
        }
        wp_set_post_terms($post_id, $mappings_names, $tax_name, TRUE);
      }
    }
  }
}

function okhub_get_mappings($dataset, $category_name) {
  $okhub_cats_wpterms = array();
  $mappings = okhubapi_variable_get('okhub', $dataset . '_' . $category_name . '_mappings', array());
  foreach ($mappings as $mapping) {
    $relation = explode(',', $mapping);
    if (isset($relation[0]) && isset($relation[1])) {
      $okhub_cats_wpterms[$relation[0]][] = $relation[1];
    }
  }
  return $okhub_cats_wpterms;
}

// Relate a term with its related OKHub categories. This currently only applies to countries (that are linked to regions).
function okhub_update_categories_term($term_id, $dataset, $taxonomy, $categories_ids_array) {
  $okhub_new_categories = okhubapi_variable_get('okhub', 'import_new_categories', OKHUB_NEW_CATEGORIES);
  if ($okhub_new_categories) {
    $terms_okhub = okhub_get_categories_terms_ids($dataset, $taxonomy, $categories_ids_array);
    wp_set_post_terms($term_id, $terms_okhub, $taxonomy, FALSE);
  }
}

// Given an array of categories ids, return an array of their corresponding term ids. New terms are created if they don't exist.
function okhub_get_categories_terms_ids($dataset, $taxonomy, $categories_ids_array) {
  $terms_okhub = array();
  foreach ($categories_ids_array as $category_id) {
    if ($term_id = okhub_get_id_by_meta(OKHUB_TAXONOMY, OKHUB_WP_FN_HUB_ITEM_ID, $category_id)) { // The taxonomy term exists.
      $terms_okhub[] = $term_id;
    }
    else {
      // If not in WP already, retrieve category from the API and add it to the taxonomy.
      $okhubapi = new OkhubApiWrapper;
      $api_key = okhubapi_variable_get('okhub', 'api_key', '');
      if ($type = okhub_taxonomy_short_name($taxonomy)) {
        $response = $okhubapi->get($type, OKHUB_HUB_SOURCE_CODE, $api_key, 'full', $category_id);
        if ($response->isError()) {
          okhubapi_register_error('okhub', __('No content retrieved by the API call. ') . $response->getErrorMessage(), 'okhub_get_categories_terms_ids', 'warning');
        }
        else {
          $categories = $response->getResults();
          if ($category = current($categories)) { // There's only one with the item id.
            if ($term_id = okhub_insert_category($dataset, $taxonomy, $category)) {
              $terms_okhub[] = $term_id;
            }
          }
        }
      }
    }
  }
  return $terms_okhub;
}

//------------------------ Functions to manage periodic imports ------------------------

// Define new cron intervals
function okhub_cron_intervals($schedules) {
	$schedules['twiceweekly'] = array(
		'interval' => 302400,
		'display' => __('Twice Weekly')
	);
	$schedules['weekly'] = array(
		'interval' => 604800,
		'display' => __('Once Weekly')
	);
	$schedules['monthly'] = array(
		'interval' => 2635200,
		'display' => __('Once Monthly')
	);
	return $schedules;
}

// Schedule periodic imports.
function okhub_schedule_imports() {
  $okhub_new_categories = okhubapi_variable_get('okhub', 'import_new_categories', OKHUB_NEW_CATEGORIES);
  foreach (okhub_assets() as $asset_type) {
    $import_assets[$asset_type] = okhubapi_variable_get('okhub', 'import_'.$asset_type, 0);
  }
  foreach (okhub_categories() as $category_type) {
    $import_categories[$category_type] = okhubapi_variable_get('okhub', 'import_',$category_type, 0);
  }
  $frequency_assets = okhubapi_variable_get('okhub', 'import_period_assets', OKHUB_IMPORT_RECURRENCE_ASSETS);
  $frequency_categories = okhubapi_variable_get('okhub', 'import_period_categories', OKHUB_IMPORT_RECURRENCE_CATEGORIES);
  okhub_unschedule_imports();
  if ($frequency_assets) {
    foreach ($import_assets as $assets_type => $enabled) {
      if ($enabled) {
        $schedule_result = wp_schedule_event(time(), $frequency_assets, 'okhub_scheduled_events', array($assets_type));
      }
    }
  }
  if ($frequency_categories && $okhub_new_categories) {
    foreach ($import_categories as $categories_type => $enabled) {
      if ($enabled) {
        $schedule_result = wp_schedule_event(time(), $frequency_categories, 'okhub_scheduled_events', array($categories_type));
      }
    }
  }
}

// Call to periodic imports.
function okhub_run_periodic_imports($type) {
  $date = date(DATE_RFC822);
  $okhub_import_dates = okhubapi_variable_get('okhub', 'import_dates', array());
  $okhub_import_results = okhubapi_variable_get('okhub', 'import_results', array());
  $okhub_new_categories = okhubapi_variable_get('okhub', 'import_new_categories', OKHUB_NEW_CATEGORIES);
  $datasets = okhub_selected_datasets();
  $categories = okhub_categories();
  $assets = okhub_assets();
  $importer = new OkhubImporter();
  if (in_array($type, $assets)) {
    foreach ($datasets as $dataset) {
      $importer->terms[$dataset][$type] = array('new' => 0, 'updated' => 0);
      $importer->import_assets($dataset, $type);
      $okhub_import_dates[$dataset][$type] = $date;
      $okhub_import_results[$dataset][$type] = $importer->terms[$dataset][$type];
    }
  }
  elseif (in_array($type, $categories) && $okhub_new_categories) {
    foreach ($datasets as $dataset) {
      $importer->terms[$dataset][$type] = array('new' => 0, 'updated' => 0);
      $importer->import_categories($dataset, $type);
      $okhub_import_dates[$dataset][$type] = $date;
      $okhub_import_results[$dataset][$type] = $importer->terms[$dataset][$type];
    }
  }
  okhubapi_variable_set('okhub', 'import_dates', $okhub_import_dates);
  okhubapi_variable_set('okhub', 'import_results', $okhub_import_results);
}

// Unschedule all the import events
function okhub_unschedule_imports() {
  foreach (array_keys(okhub_types()) as $type) {
    wp_clear_scheduled_hook('okhub_scheduled_events', array($type));
  }
}
