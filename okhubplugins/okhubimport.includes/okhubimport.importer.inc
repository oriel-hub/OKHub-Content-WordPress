<?php

// Load Importer API
require_once ABSPATH . 'wp-admin/includes/import.php';

if ( !class_exists( 'WP_Importer' ) ) {
	$class_wp_importer = ABSPATH . 'wp-admin/includes/class-wp-importer.php';
	if ( file_exists( $class_wp_importer ) ) {
		require_once $class_wp_importer;
  }
}

//------------------------ OKHUB_Importer class definition -------------------------

if ( class_exists( 'WP_Importer' ) ) {
class OKHUB_Importer extends WP_Importer {

  // Posts and taxonomy terms created/updated
  var $terms = array('hub' => array());

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
        check_admin_referer('okhubimport_import', 'okhubimport_import_check_referrer');
        $importer_url = admin_url() . 'admin.php?import=okhubimport_importer';
        printf(__('<br/>Go back to the <a href="%s">OKHUB Importer page</a>.'), $importer_url);
        $this->import();
        $this->display_results();
        okhubapi_report_errors();
				break;
		}
		$this->footer();
	}

  // Display header.
	function header() {
		echo '<div class="wrap">';
		screen_icon();
		echo '<h2>'.__('OKHUB Importer', 'okhubimport-importer').'</h2>';
	}

  // Display footer.
	function footer() {
		echo '</div>';
	}

  // Display main import page. Selection of types of items to import.
	function greet() {
		$okhubimport_setup_done = okhubapi_variable_get('okhubimport', 'setup_done', FALSE);
		?>
		<div class="narrow">
		<?php 
		if (!$okhubimport_setup_done) {
      echo '<p>'.sprintf(__('Before starting to import OKHUB content, you need you need to set a number of parameters, including the OKHUB API key, at the <a href="%s">OKHUB API administrative page</a>.', 'okhubimport_import'), 'options-general.php?page=okhubimport').'</p>'; 
		}
    else {
      okhubimport_importer_form();
    }
  }

  // Display a summary of the importing process.
  function display_results() {
    $total = 0;
    foreach ($this->terms as $dataset => $results_dataset) {
      $exclude = FALSE;
      foreach ($results_dataset as $type => $results) {
        $exclude = okhubapi_exclude($dataset, $type);
        if (!$exclude) {
          if ($results['new']) {
            $total += $results['new'];
            $output = __('Created ') . $results['new'] . ' ' . ucfirst($dataset) . ' ' . __($type);
            echo okhub_div_field_wrapper('ids-results-import', '', '', $output);
          }
          if ($results['updated']) {
            $total += $results['updated'];
            $output = __('Updated ') . $results['updated'] . ' ' . ucfirst($dataset) . ' ' . __($type);
            echo okhub_div_field_wrapper('ids-results-import', '', '', $output);
          }
          if (isset($results['duplicated']) && $results['duplicated']) {
            $total += $results['duplicated'];
            $output = __('New versions created for ') . $results['duplicated'] . ' ' . ucfirst($dataset) . ' ' . __($type);
            echo okhub_div_field_wrapper('ids-results-import', '', '', $output);
          }
          if (isset($results['skipped']) && $results['skipped']) {
            $output = __('Skipped ') . $results['skipped'] . ' ' . ucfirst($dataset) . ' ' . __($type);
            echo okhub_div_field_wrapper('ids-results-import', '', '', $output);
          }
        }
        if (!$total && !$exclude) {
          $output = __('Nothing imported from ') . ucfirst($dataset);
          echo okhub_div_field_wrapper('ids-results-import', '', '', $output);
        }
      }
    }
  }
//-------------------------------- Import methods ------------------------------

  // Check which types of content to import and call to import methods. If selected, categories are imported first.
  // Keeps track of latest dates and results for each type, in order to display this information on the admin page (periodic imports do the same).
  function import() {
    global $okhub_categories;
    global $okhub_assets;
    global $okhub_exclude;
    $date = date(DATE_RFC822);
    $okhubimport_new_categories = okhubapi_variable_get('okhubimport', 'import_new_categories', OKHUB_IMPORT_NEW_CATEGORIES);
    $okhubimport_import_dates = okhubapi_variable_get('okhubimport', 'import_dates', array('eldis' => array(), 'bridge' => array()));
    $okhubimport_import_results = okhubapi_variable_get('okhubimport', 'import_results', array('eldis' => array(), 'bridge' => array()));
    $datasets = okhubimport_get_datasets();
    foreach ($datasets as $dataset) {
      // Import selected categories (if the option is enabled).
      if ($okhubimport_new_categories) {
        foreach ($okhub_categories as $category) {
          $parameter_name = 'okhubimport_import_' . $category;
          if (isset($_POST[$parameter_name]) && ($_POST[$parameter_name])) {
            $this->import_categories($dataset, $category);
            $okhubimport_import_dates[$dataset][$category] = $date;
            $okhubimport_import_results[$dataset][$category] = $this->terms[$dataset][$category];
          }
        }
      }
      // Import selected assets.
      foreach ($okhub_assets as $asset) {
        $parameter_name = 'okhubimport_import_' . $asset;
        if (isset($_POST[$parameter_name]) && ($_POST[$parameter_name])) {
          $this->import_assets($dataset, $asset);
          $okhubimport_import_dates[$dataset][$asset] = $date;
          $okhubimport_import_results[$dataset][$asset] = $this->terms[$dataset][$asset];
        }
      }
    }
    okhubapi_variable_set('okhubimport', 'import_dates', $okhubimport_import_dates);
    okhubapi_variable_set('okhubimport', 'import_results', $okhubimport_import_results);
  }

  // Import assets.
  function import_assets($dataset, $type) { 
    global $okhub_sources;
    $updated_assets = okhubapi_variable_get('okhubimport', 'updated_assets', OKHUB_IMPORT_UPDATED_ASSETS);
    $this->terms[$dataset][$type] = array('new' => 0, 'updated' => 0, 'skipped' => 0, 'duplicated' => 0);
    if (!okhubapi_exclude($dataset, $type)) {
      if ($assets = $this->retrieve_content($dataset, $type)) {
        if (!empty($assets)) {
          $post_type = 'okhub_' . $type;
          $object_id_field = $dataset . '_object_id';
          if (post_type_exists($post_type)) {
            if (is_admin() && OKHUB_IMPORT_VERBOSE_IMPORT) {
              $output = __('Importing') . ' ' . ucfirst($dataset) . ' ' . __($type) . '...';
              echo okhub_div_field_wrapper('ids-title-output-importer', '', '', $output);
            }
            foreach ($assets as $id=>$ass) {
            	if ($id == "results"){ //make sure that its the results not the metadata
		    foreach($ass as $i=>$asset){
		    	      foreach($okhub_sources as $sources){ //quick hack for getting info from different sources
					if ($asset[$sources]['title'] !=""){
						$source = $sources;
						break;
					}
			      } 
		      $asset[$source]['object_id'] = $i;
              if ($post_ids = okhubimport_get_okhub_by_meta('post', $object_id_field, $i)) { // The post already exists in the dataset.
                $post_id = $post_ids[0];
                $post_status = get_post_status($post_id);
                $old_date_updated = get_post_meta($post_id, 'date_updated', TRUE);
                if (!is_numeric($old_date_updated)) {
                  $old_date_updated = strtotime($old_date_updated);
                }
                if ($post_status == 'pending' || $post_status == 'publish') {
                  if ((strtotime($asset['source']['date_updated']) > $old_date_updated)) { // the new version is more recent
                    switch ($updated_assets) {
                      case 'overwrite':                 
                        $post_id = okhubimport_update_asset($post_id, $dataset, $post_type, $asset, $source);
                        if ($post_id) {
                          if (is_admin() && OKHUB_IMPORT_VERBOSE_IMPORT) {
                            $output = $asset[$source]['title'] . ' - ' . __('updated');
                            echo okhub_div_field_wrapper('ids-item-output-importer', '', '', $output);
                          }
                          $this->terms[$dataset][$type]['updated']++;
                        }
                        else {
                          okhubapi_register_error('okhubimport', __('The post could not be updated for ') . $asset->object_id, 'OKHUB_Importer::import_assets', 'warning');
                        }
                        break;
                      case 'keep': 
                        if (count($post_ids) == 1) {
                          $post_id = okhubimport_insert_asset($dataset, $post_type, $asset, $source, TRUE);
                          if ($post_id) {
                            $this->terms[$dataset][$type]['duplicated']++ ;
                          }
                          else {
                            okhubapi_register_error('okhubimport', __('A new version of the post could not be created for ') . $asset->object_id, 'OKHUB_Importer::import_assets', 'warning');
                          }
                        }
                        else {
                          if (is_admin() && OKHUB_IMPORT_VERBOSE_IMPORT) {
                            $output = $asset[$source]['title'] . ' - ' . __('skipped (there are already two versions for this asset)');
                            echo okhub_div_field_wrapper('ids-item-output-importer', '', '', $output);
                          }
                          $this->terms[$dataset][$type]['skipped']++;
                        }
                        break;
                      case 'skip':
                        if (is_admin() && OKHUB_IMPORT_VERBOSE_IMPORT) {
                          $output = $asset->title . ' - ' . __('skipped (most recent version already imported)');
                          echo okhub_div_field_wrapper('ids-item-output-importer', '', '', $output);
                        }
                        $this->terms[$dataset][$type]['skipped']++;
                        break;
                    }
                  }
                  else { // The version being imported is not more recent and it's currently published or pending. we always skip it in this case.
                    if (is_admin() && OKHUB_IMPORT_VERBOSE_IMPORT) {
                      $output = $asset->title . ' - ' . __('skipped (most recent version already imported)');
                      echo okhub_div_field_wrapper('ids-item-output-importer', '', '', $output);
                    }
                    $this->terms[$dataset][$type]['skipped']++;
                  }
                }
                else { // The existing version is not published or pending - we update it.
                  $post_id = okhubimport_update_asset($post_id, $dataset, $post_type, $asset);
                  if ($post_id) {
                    if (is_admin() && OKHUB_IMPORT_VERBOSE_IMPORT) {
                      $output = $asset->title . ' - ' . $post_status . __(' updated');
                      echo okhub_div_field_wrapper('ids-item-output-importer', '', '', $output);
                    }
                    $this->terms[$dataset][$type]['updated']++;
                  }
                  else {
                    okhubapi_register_error('okhubimport', __('The post could not be updated for ') . $asset->object_id, 'OKHUB_Importer::import_assets', 'warning');
                  }
                }
              }
              else { // The asset does not exist in the dataset. Create it.
                $post_id = okhubimport_insert_asset($dataset, $post_type, $asset);
                if ($post_id) {
                  $this->terms[$dataset][$type]['new']++ ;
                }
                else {
                  okhubapi_register_error('okhubimport', __('A new post could not be created for ') . $asset->object_id, 'OKHUB_Importer::import_assets', 'warning');
                }
              }
         	 }} //foreach and if
            }
          }
        }
        else {
          okhubapi_register_error('okhubimport', __('Sorry, we cannot find any content that matches your criteria. Please remove some filters or search more generally and try again.'), 'OKHUB_Importer::import_assets', 'notification', FALSE);
        }
      }
    }
  }

  // Import categories.
  function import_categories($dataset, $type) {
    if (!okhubapi_exclude($dataset, $type)) {
      $this->terms[$dataset][$type] = array('new' => 0, 'updated' => 0);
      $update_children = array();
      $levels = okhubapi_variable_get('okhubimport', 'level_categories', OKHUB_IMPORT_CATEGORY_LEVELS);
      if ($categories = $this->retrieve_content($dataset, $type, $levels)) {
        if (!empty($categories)) {
          $taxonomy_name = $dataset . '_' . $type;
          if (taxonomy_exists($taxonomy_name)) {
            $object_id_field = $dataset . '_object_id';
            if (is_admin() && OKHUB_IMPORT_VERBOSE_IMPORT) {
              $output = __('Importing') . ' ' . ucfirst($dataset) . ' ' . __($type) . '...';
              echo okhub_div_field_wrapper('ids-title-output-importer', '', '', $output);
            }
            foreach ($categories as $category) {
              $update_parent = FALSE;
              $category_name_display = okhubimport_get_category_name($category);
              $category_name = $category_name_display . " ($category->object_id)";
              $term_args = array('name' => $category_name);
              $term = FALSE;
              if ((isset($category->level)) && ($category->level > 1) && (isset($category->parent_object_id))) {
                if ($term_id_parent = okhubimport_get_id_by_meta(OKHUB_IMPORT_TAXONOMY, $object_id_field, $category->parent_object_id)) { // The parent is a WP term.
                  $term_args['parent'] = $term_id_parent;
                }
                else { // It has a parent, but the parent term has not been created in the dataset yet.
                  $update_parent = TRUE;
                }
              }
              $term = term_exists($category_name, $taxonomy_name);
              if ($term) {
                if (is_array($term)) {
                  $term_id = $term['term_id'];
                }
                else {
                  $term_id = $term;
                }
                if ($update_parent) {
                  if (isset($update_children[$category->parent_object_id])) {
                  $update_children[$category->parent_object_id][] = $term_id;
                  }
                  else {
                    $update_children[$category->parent_object_id] = array($term_id);
                  }
                }
                // Update the term corresponding to the category.
                if (is_admin() && OKHUB_IMPORT_VERBOSE_IMPORT) {
                  $output = $category_name_display . ' - ' . __('updated');
                  echo okhub_div_field_wrapper('ids-item-output-importer', '', '', $output);
                }
                if (okhubimport_update_category($term_id, $dataset, $taxonomy_name, $category, $term_args)) {
                  $this->terms[$dataset][$type]['updated']++;
                }
                else {
                  okhubapi_register_error('okhubimport', __('The term could not be updated for category ') . $category->object_id, 'import_categories', 'warning');
                }
              }
              else {
                // Create a new term for the category.
                if (is_admin() && OKHUB_IMPORT_VERBOSE_IMPORT) {
                  $output = $category_name_display . ' - ' . __('new');
                  echo okhub_div_field_wrapper('ids-item-output-importer', '', '', $output);
                }
                $term_id = okhubimport_insert_category($dataset, $taxonomy_name, $category, $term_args);
                if ($term_id) {
                  $this->terms[$dataset][$type]['new']++ ;
                }
                else {
                  okhubapi_register_error('okhubimport', __('A new term could not be created for category ') . $category->object_id, 'import_categories', 'warning');
                }
              }
            }
            okhubimport_update_children($dataset, $update_children, $taxonomy_name);
          }
        }
        else {
          okhubapi_register_error('okhubimport', __('No categories were retrieved by the API call.'), 'import_categories', 'warning');
        }
      }
    }
  }

  // Call the API to retrieve content of the indicated type, using the the plugin settings.
  function retrieve_content($dataset, $type, $levels = 0) {
    global $okhub_sources;
    $content = array();
    $okhubapi = new OkhubApiWrapper;
    $api_key = okhubapi_variable_get('okhubimport', 'api_key', '');
    $format = 'full';
    $filter_params = array();
    $is_asset = ($type == 'documents' || $type == 'organisations');
    if ($is_asset) {
      $num_requested = okhubapi_variable_get('okhubimport', 'num_items', OKHUB_API_NUM_ITEMS);
      $age_results = okhubapi_variable_get('okhubimport', 'age_new_assets', OKHUB_API_AGE_NEW_ASSETS);
      $okhubimport_import_query = okhubapi_variable_get('okhubimport', 'import_query', '');
      $okhubimport_import_countries_assets = okhubapi_variable_get('okhubimport', $dataset . '_countries_assets', array());
      $okhub_import_regions_assets = okhubapi_variable_get('okhubimport', $dataset . '_regions_assets', array());
      $okhubimport_import_themes_assets = okhubapi_variable_get('okhubimport', $dataset . '_themes_assets', array());
      if ($okhubimport_import_query) {
        $search_params = explode(',', $okhubimport_import_query);
        foreach ($search_params as $search_param) {
          $param = explode('=', trim($search_param));
          $key = $param[0];
          $value = $param[1];
          $filter_params[$key] = $value;
        }
      }
      if (!empty($okhubimport_import_countries_assets)) {
        $filter_params['country'] = implode('|', $okhubimport_import_countries_assets);
      }
      if (!empty($okhub_import_regions_assets)) {
        $filter_params['region'] = implode('|', $okhub_import_regions_assets);
      }
      if (!empty($okhubimport_import_themes_assets)) {
        $filter_params['theme'] = implode('|', $okhubimport_import_themes_assets);
      }
      if ($type == 'documents') { // Document-specific filters
        $okhubimport_import_authors_assets = okhubapi_variable_get('okhubimport', 'import_authors_assets', '');
        $okhubimport_import_publishers_assets = okhubapi_variable_get('okhubimport', 'import_publishers_assets', '');
        $okhubimport_language_name_codes = okhubapi_variable_get('okhubimport', 'language_name_codes', array());
        if ($okhubimport_import_authors_assets) {
          $authors = explode(',', $okhubimport_import_authors_assets);
          $list_authors = implode('|', array_map('trim', $authors));
          $filter_params['author'] = $list_authors;
        }
        if ($okhubimport_import_publishers_assets) {
          $publishers = explode(',', $okhubimport_import_publishers_assets);
          $list_publishers = implode('|', array_map('trim', $publishers));
          $filter_params['publisher_name'] = $list_publishers;
        }
        if ($okhubimport_language_name_codes) {
          $okhub_languages = okhub_languages();
          $list_language_names = array();
          foreach ($okhubimport_language_name_codes as $language_code) {
            if (isset($okhub_languages[$language_code])) {
              $list_language_names[] = $okhub_languages[$language_code]; 
            }
            if ($list_language_names) {
              $language_names = implode('|', $list_language_names);
              $filter_params['language_name'] = $language_names;
            }
          }
        }
      }
      // Keep only those assets whose URIs don't match the ones to exclude. Paginate until we get the number requested.
      $num_retrieved = 0;
      $total_results = 0;
      $offset = 0;
      $more_results = TRUE;
      while ((!$num_requested || ($num_retrieved < $num_requested)) && $more_results) {
        /*if ($offset) {
          $filter_params['start_offset'] = $offset;
        }*/
        $response = $response = $okhubapi->search($type, $dataset, $api_key, $format, $num_requested, $age_results, $offset, $filter_params);
        if ($response->isError()) {
          $error_message = __('No content retrieved by the API call. ') . $response->getErrorMessage();
          okhubapi_register_error('okhubimport', $error_message, 'OKHUB_Importer::retrieve_content', 'warning');
          $more_results = FALSE;
        }
        else {
          //$assets_retrieved = $response->getResults(); //before
          $assets_retrieved = $response->getDocumentVersion($okhub_sources);
          if (!empty($assets_retrieved)) {
            foreach ($assets_retrieved as $asset) {
              if (!okhub_exclude_asset('okhubimport', $asset)) {
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
    } else { // is country, region or theme
      if ($type == 'themes') {
        if ($levels > 0) {
          $include_levels = array();
          for ($i = 1; $i <= $levels; $i++) {
            $include_levels[] = $i;
          }
          $filter_params['level'] = implode('|', $include_levels);
        }
      }
      $num_requested = 0;
      $age_results = 0;
      $response = $okhubapi->search($type, $dataset, $api_key, $format, $num_requested, $age_results,$offset, $filter_params);
      if ($response->isError()) {
        $error_message = __('No content retrieved by the API call. ') . $response->getErrorMessage();
        okhubapi_register_error('okhubimport', $error_message, 'OKHUB_Importer::retrieve_content', 'warning');
        $content = FALSE;
      }
      else {
        //$content = $response->getResults();
        $content = $response->getDocumentVersion($okhub_sources);
      }
    }
    return $content;
  }

} // Class OKHUB_Import

} // If WP_Importer exists

//----------------------- End of OKHUB_Importer class definition -------------------------

//------------------------ Functions to manage imported content ------------------------

// Create new post with imported asset.
function okhubimport_insert_asset($dataset, $post_type, $asset, $source, $duplicated = FALSE) {
  global $okhub_sources;
  $import_keywords = okhubapi_variable_get('okhubimport', 'import_tags', OKHUB_IMPORT_TAGS);
  $import_additional_languages = okhubapi_variable_get('okhubimport', 'additional_languages', array());
  $okhubimport_import_user_id = okhubapi_variable_get('okhubimport', 'import_user_id', '0');
  $okhub_api_key = okhubapi_variable_get('okhubimport', 'api_key', '');
  $okhubimport_pub_date = okhubapi_variable_get('okhubimport', 'pub_date', 'default');
  $okhubimport_import_document_type = okhubapi_variable_get('okhubimport', 'import_document_type_'.$dataset, 'field');
  $display_sites = okhubimport_display_datasets('public');
  $countries_taxonomy = $dataset . '_countries';
  $regions_taxonomy = $dataset . '_regions';
  $themes_taxonomy = $dataset . '_themes';
  $object_id_field = $dataset . '_object_id';
  $post = array();
  /*$default_language = okhubapi_variable_get('okhubimport', 'language', OKHUB_IMPORT_DEFAULT_LANGUAGE);
  $title = okhub_get_translation($asset, 'title', $default_language);
  $description = okhub_get_translation($asset, 'description', $default_language*/
  foreach($okhub_sources as $sources){
  	if ($asset[$sources]['title'] != ""){
  		$source = $sources;	
  	}
  }
  $asset = $asset[$source];
  $post['post_title'] = $asset['title'];
  $post['post_name'] = sanitize_title($asset['title']);
  if ('hub' == okhubapi_variable_get('okhubimport', 'default_dataset', OKHUB_IMPORT_DEFAULT_DATASET_ADMIN)) {
    $post['post_name'] = $dataset . '-' . $post['post_name'];
  }
  if ($duplicated) {
    $post['post_title'] .= ' [UPDATED]';
    $post['post_name'] .= '-2';
  }
  $post['post_excerpt'] = okhub_excerpt_post('okhubimport', $asset['description']);
  $post['post_content'] = $asset['description'];
  $post['post_type'] = $post_type;
  if (!$duplicated && in_array($dataset, $display_sites)) {
    $post['post_status'] = 'publish';
  }
  else {
    $post['post_status'] = 'pending';
  }
  if ($okhubimport_import_user_id) {
    $post['post_author'] = $okhubimport_import_user_id;
  }
  if (isset($asset[$source]['date_created']) && ($okhubimport_pub_date == 'date_created')) { 
    $post['post_date'] = date_i18n('Y-m-d H:i:s', strtotime($asset['date_created']));
  }
  if (isset($asset[$source]['date_updated']) && ($okhubimport_pub_date == 'date_updated')) { 
    //$post['post_date'] = date_i18n('Y-m-d H:i:s', strtotime($asset->date_updated));
     $post['post_date'] = date_i18n('Y-m-d H:i:s', strtotime($asset['date_updated']));
  }
  $post_id  = wp_insert_post($post);
  if (is_admin() && OKHUB_IMPORT_VERBOSE_IMPORT) {
    if ($duplicated) {
      $output = $title . ' - ' . __('new version of existing post');
    }
    else {
      $output = $title . ' - ' . __('new');
    }
    //echo okhub_div_field_wrapper('ids-item-output-importer', '', '', $output);
    echo okhub_div_field_wrapper('okhub-item-output-importer', '', '', $output);
  }
  if ($post_id) {
  	  
    add_post_meta($post_id, $object_id_field, $asset[$source]['object_id'], TRUE);
    // Extra fields that could be not present in 'short' responses.
    if (isset($asset['site'])) { add_post_meta($post_id, 'site', $asset['site'], TRUE); }
    if (isset($asset['timestamp'])) { add_post_meta($post_id, 'timestamp', $asset['timestamp'], TRUE); }
    if (isset($asset['website_url'])) { add_post_meta($post_id, 'website_url', $asset['website_url'], TRUE); }
    if (isset($asset['asset_id'])) { add_post_meta($post_id, 'asset_id', $asset['asset_id'], TRUE); }
    if (isset($asset['date_created'])) { add_post_meta($post_id, 'date_created', $asset['date_created'], TRUE); }
    if (isset($asset['date_updated'])) { add_post_meta($post_id, 'date_updated', $asset['date_updated'], TRUE); }
    /*if (isset($asset->country_focus_array)) { okhubimport_update_categories_post($post_id, $dataset, $countries_taxonomy, $asset->country_focus_array); }
    if (isset($asset->category_region_array)) { okhubimport_update_categories_post($post_id, $dataset, $regions_taxonomy, $asset->category_region_array); }
    if (isset($asset->category_theme_array)) { okhubimport_update_categories_post($post_id, $dataset, $themes_taxonomy, $asset->category_theme_array); }
    */
    if (isset($asset->metadata_url)) {
      $metadata_url = $asset->metadata_url;
      if ($okhub_api_key) {
        $metadata_url .= '?' . OKHUB_API_KEY_PAR . '=' . $okhub_api_key;
      }
      add_post_meta($post_id, 'metadata_url', $metadata_url, TRUE);
    }
    if (($import_keywords) && (isset($asset->keywords))) {
      wp_set_post_tags($post_id, $asset->keywords, TRUE);
    }
    if ($post_type == 'okhub_documents') {
    // Document-specific fields.
      if (isset($asset['authors'])) { add_post_meta($post_id, 'authors', $asset['authors']); }
      if (isset($asset['language_name'])) { add_post_meta($post_id, 'language_name', $asset['language_name'], TRUE); }
      if (isset($asset['publication_date'])) { add_post_meta($post_id, 'publication_date', $asset['publication_date'], TRUE); }
      if (isset($asset['publication_year'])) { add_post_meta($post_id, 'publication_year', $asset['publication_year'], TRUE); }
      if (isset($asset['licence_type'])) { add_post_meta($post_id, 'licence_type', $asset['licence_type'], TRUE); }
      if (isset($asset['publisher'])) { add_post_meta($post_id, 'publisher', $asset['publisher'], TRUE); }
      if (isset($asset['publisher_country'])) { add_post_meta($post_id, 'publisher_country', $asset['publisher_country'], TRUE); }
      if (isset($asset['document_type'])) { add_post_meta($post_id, 'document_type', $asset['document_type'], TRUE); }
      if (isset($asset['url'])) { add_post_meta($post_id, 'urls', $asset['url']); }
      
       /*if (isset($asset->authors)) { okhubimport_add_post_array_meta($post_id, 'authors', $asset->authors); }
      if (isset($asset->language_name)) { add_post_meta($post_id, 'language_name', $asset->language_name, TRUE); }
      if (isset($asset->publication_date)) { add_post_meta($post_id, 'publication_date', $asset->publication_date, TRUE); }
      if (isset($asset->publication_year)) { add_post_meta($post_id, 'publication_year', $asset->publication_year, TRUE); }
      if (isset($asset->licence_type)) { add_post_meta($post_id, 'licence_type', $asset->licence_type, TRUE); }
      if (isset($asset->publisher)) { add_post_meta($post_id, 'publisher', $asset->publisher, TRUE); }
      if (isset($asset->publisher_country)) { add_post_meta($post_id, 'publisher_country', $asset->publisher_country, TRUE); }
      if (isset($asset->document_type)) { add_post_meta($post_id, 'document_type', $asset->document_type, TRUE); }
      if (isset($asset->urls)) { okhubimport_add_post_array_meta($post_id, 'urls', $asset->urls); }
     foreach ($import_additional_languages as $lang_code) {
        foreach (array('title', 'description') as $field) { // Iterate through the translatable fields. This could be generalised.
          $field_name = $field . '_' . $lang_code;
          $field_value = okhub_get_translation($asset, $field, $lang_code, FALSE);
          if ($field_value) {
            add_post_meta($post_id, $field_name, $field_value, TRUE);
          }
        }
      }*/
      if (isset($asset->document_type)) {
        if ($okhubimport_import_document_type == 'field') {
          okhubimport_add_post_array_meta($post_id, 'document_type', $asset->document_type);
        }
        else {
          okhubimport_update_document_types_post($post_id, $dataset, $asset->document_type);
        }
      }
    }
    elseif ($post_type == 'okhub_organisations') {
    // Organisation-specific fields.
      if (isset($asset->acronym)) { add_post_meta($post_id, 'acronym', $asset->acronym, TRUE); }
      if (isset($asset->alternative_acronym)) { add_post_meta($post_id, 'alternative_acronym', $asset->alternative_acronym, TRUE); }
      if (isset($asset->organisation_type)) { add_post_meta($post_id, 'organisation_type', $asset->organisation_type, TRUE); }
      if (isset($asset->organisation_url)) { add_post_meta($post_id, 'organisation_url', $asset->organisation_url, TRUE); }
      if (isset($asset->location_country)) { add_post_meta($post_id, 'location_country', $asset->location_country, TRUE); }
    }
  }
  else {
    okhubapi_register_error('okhubimport',  __('Error inserting asset ') . $asset->title, 'okhubimport_insert_asset', 'warning');
  }
  return array($post_id);
}

// Update post with imported asset.
function okhubimport_update_asset($post_id, $dataset, $post_type, $asset, $source) {
  global $okhub_sources;
  $import_keywords = okhubapi_variable_get('okhubimport', 'import_tags', '0');
  $import_additional_languages = okhubapi_variable_get('okhubimport', 'additional_languages', array());
  $okhubimport_import_user_id = okhubapi_variable_get('okhubimport', 'import_user_id', '0');
  $okhub_api_key = okhubapi_variable_get('okhubimport', 'api_key', '');
  $okhubimport_pub_date = okhubapi_variable_get('okhubimport', 'pub_date', 'default');
  $okhubimport_import_document_type = okhubapi_variable_get('okhubimport', 'import_document_type_'.$dataset, 'field');
  $countries_taxonomy = $dataset . '_countries';
  $regions_taxonomy = $dataset . '_regions';
  $themes_taxonomy = $dataset . '_themes';
  $object_id_field = $dataset . '_object_id';
  $updated_post = array();
  $updated_post['ID'] = $post_id;
  /*$default_language = okhubapi_variable_get('okhubimport', 'language', OKHUB_IMPORT_DEFAULT_LANGUAGE);
  $title = okhub_get_translation($asset, 'title', $default_language);
  $description = okhub_get_translation($asset, 'description', $default_language);*/
  foreach($okhub_sources as $sources){
  	if ($asset[$sources]['title'] != ""){
  		$source = $sources;	
  	}
  }
  $asset = $asset[$source];
  $updated_post['post_title'] = $asset['title'];
  $updated_post['post_excerpt'] = okhub_excerpt_post('okhubimport', $asset['description']);
  $updated_post['post_content'] = $asset['description'];
  if ($okhubimport_import_user_id) {
    $updated_post['post_author'] = $okhubimport_import_user_id;
  }
  if (isset($asset->date_created) && ($okhubimport_pub_date == 'date_created')) { 
    $updated_post['post_date'] = date_i18n('Y-m-d H:i:s', strtotime($asset['date_created']));
  }
  if (isset($asset->date_updated) && ($okhubimport_pub_date == 'date_updated')) { 
    $updated_post['post_date'] = date_i18n('Y-m-d H:i:s', strtotime($asset['date_updated']));
  }
  $new_post_id  = wp_update_post($updated_post);
  if ($new_post_id) {
    update_post_meta($post_id, $object_id_field, $asset['object_id']);
    // Extra fields that could not be present in 'short' responses.
    if (isset($asset['site'])) { update_post_meta($post_id, 'site', $asset['site']); }
    if (isset($asset['timestamp'])) { update_post_meta($post_id, 'timestamp', $asset['timestamp']); }
    if (isset($asset['website_url'])) { update_post_meta($post_id, 'website_url', $asset['website_url']); }
    if (isset($asset['asset_id'])) { update_post_meta($post_id, 'asset_id', $asset['asset_id']); }
    if (isset($asset['date_created'])) { update_post_meta($post_id, 'date_created', $asset['date_created']); }
    if (isset($asset['date_updated'])) { update_post_meta($post_id, 'date_updated', $asset['date_updated']); }
    /*if (isset($asset->country_focus_array)) { okhubimport_update_categories_post($post_id, $dataset, $countries_taxonomy, $asset->country_focus_array); }
    if (isset($asset->category_region_array)) { okhubimport_update_categories_post($post_id, $dataset, $regions_taxonomy, $asset->category_region_array); }
    if (isset($asset->category_theme_array)) { okhubimport_update_categories_post($post_id, $dataset, $themes_taxonomy, $asset->category_theme_array); }*/
    if (isset($asset->metadata_url)) {
      $metadata_url = $asset->metadata_url;
      if ($okhub_api_key) {
        $metadata_url .= '?' . OKHUB_API_KEY_PAR . '=' . $okhub_api_key;
      }
      update_post_meta($post_id, 'metadata_url', $metadata_url);
    }
    if (($import_keywords) && (isset($asset->keywords))) {
      wp_set_post_tags($post_id, $asset->keywords, TRUE);
    }
    if ($post_type == 'okhub_documents') {
    // Document-specific fields.
      //if (isset($asset['authors'])) { okhubimport_update_post_array_meta($post_id, 'authors', $asset->authors); }
      if (isset($asset['authors'])) { update_post_meta($post_id, 'authors', $asset['authors']); }
      if (isset($asset['language_name'])) { update_post_meta($post_id, 'language_name', $asset['language_name']); }
      if (isset($asset['publication_date'])) { update_post_meta($post_id, 'publication_date', $asset['publication_date']); }
      if (isset($asset['publication_year'])) { update_post_meta($post_id, 'publication_year', $asset['publication_year']); }
      if (isset($asset['licence_type'])) { update_post_meta($post_id, 'licence_type', $asset['licence_type']); }
      if (isset($asset['publisher'])) { update_post_meta($post_id, 'publisher', $asset['publisher']); }
      if (isset($asset['publisher_country'])) { update_post_meta($post_id, 'publisher_country', $asset['publisher_country']); }
      if (isset($asset['document_type'])) { update_post_meta($post_id, 'document_type', $asset['document_type']); }
      if (isset($asset['url'])) { update_post_meta($post_id, 'urls', $asset['url']); }
      /*foreach ($import_additional_languages as $lang_code) {
        foreach (array('title', 'description') as $field) { // Iterate through the translatable fields. This could be generalised.
          $field_name = $field . '_' . $lang_code;
          $field_value = okhub_get_translation($asset, $field, $lang_code, FALSE);
          if ($field_value) {
            update_post_meta($post_id, $field_name, $field_value);
          }
        }
      }*/
      if (isset($asset->document_type)) {
        if ($okhubimport_import_document_type == 'field') {
          okhubimport_update_post_array_meta($post_id, 'document_type', $asset->document_type);
        }
        else {
          okhubimport_update_document_types_post($post_id, $dataset, $asset->document_type);
        }
      }
    }
    elseif ($post_type == 'okhub_organisations') {
    // Organisation-specific fields.
      if (isset($asset->acronym)) { update_post_meta($post_id, 'acronym', $asset->acronym); }
      if (isset($asset->alternative_acronym)) { update_post_meta($post_id, 'alternative_acronym', $asset->alternative_acronym); }
      if (isset($asset->organisation_type)) { update_post_meta($post_id, 'organisation_type', $asset->organisation_type); }
      if (isset($asset->organisation_url)) { update_post_meta($post_id, 'organisation_url', $asset->organisation_url); }
      if (isset($asset->location_country)) { update_post_meta($post_id, 'location_country', $asset->location_country); }
    }
  }
  else {
    okhubapi_register_error('okhubimport',  __('Error updating asset ') . $asset->title, 'okhubimport_update_asset', 'warning');
  }
  return array($new_post_id);
}

// Gets the category name, looking into the retrieved fields.
function okhubimport_get_category_name($category) {
  $category_name = '';
  if (isset($category->title)) {
    $category_name = $category->title;
  } elseif (isset($category->object_name)) {
    $category_name = $category->object_name;
  } elseif (isset($category->name)) {
    $category_name = $category->name;
  }
  return trim($category_name);
}

// This is necessary if parents don't come before their children, which is not the case now with the OKHUB API, but better to make sure.
function okhubimport_update_children($dataset, $update_children, $taxonomy_name) {
  $object_id_field = $dataset . '_object_id';
  foreach ($update_children as $parent_object_id => $array_children_term_ids) {
    if ($term_id_parent = okhubimport_get_id_by_meta(OKHUB_IMPORT_TAXONOMY, $object_id_field, $parent_object_id)) {
      foreach ($array_children_term_ids as $term_id) {
        wp_update_term($term_id, $taxonomy_name, array('parent' => $term_id_parent));
      }
    }
  }
}

// Create new term with imported category.
// Returns the term id if everything goes ok, NULL if it could not be inserted.
// TODO - insert only categories not archived?
function okhubimport_insert_category($dataset, $taxonomy, $category, $term_args) {
  $okhub_api_key = okhubapi_variable_get('okhubimport', 'api_key', '');
  $countries_taxonomy = $dataset . '_countries';
  $regions_taxonomy = $dataset . '_regions';
  $object_id_field = $dataset . '_object_id';
  $term_id = NULL;
  $term_slug = sanitize_title($term_args['name']);
  if ('all' == okhubapi_variable_get('okhubimport', 'default_dataset', OKHUB_IMPORT_DEFAULT_DATASET_ADMIN)) {
    $term_slug = $dataset . '-' . $term_slug;
  }
  $term_args['slug'] = $term_slug;
  $new_term = wp_insert_term($term_args['name'], $taxonomy, $term_args);
  if (!is_wp_error($new_term)) {
    $term_id = $new_term['term_id'];
    okhubimport_add_term_meta($term_id, $object_id_field, $category->object_id, TRUE);
    // There might be some fields not available when inserting categories (in the process of inserting an asset or with 'short' responses).
    if (isset($category->site)) { okhubimport_add_term_meta($term_id, 'site', $category->site, TRUE); }
    if (isset($category->timestamp)) { okhubimport_add_term_meta($term_id, 'timestamp', $category->timestamp, TRUE); }
    if (isset($category->website_url)) { okhubimport_add_term_meta($term_id, 'website_url', $category->website_url, TRUE); }
    if ($taxonomy == $countries_taxonomy) {
      if (isset($category->alternative_name)) { okhubimport_add_term_meta($term_id, 'alternative_name', $category->alternative_name, TRUE); }
      if (isset($category->asset_id)) { okhubimport_add_term_meta($term_id, 'asset_id', $category->asset_id, TRUE); }
      if (isset($category->category_region_array)) { okhubimport_update_categories_term($term_id, $dataset, $regions_taxonomy, $category->category_region_array); }
      if (isset($category->iso_number)) { okhubimport_add_term_meta($term_id, 'iso_number', $category->iso_number, TRUE); }
      if (isset($category->iso_three_letter_code)) { okhubimport_add_term_meta($term_id, 'iso_three_letter_code', $category->iso_three_letter_code, TRUE); }
      if (isset($category->iso_two_letter_code)) { okhubimport_add_term_meta($term_id, 'iso_two_letter_code', $category->iso_two_letter_code, TRUE); }
    } else {
      if (isset($category->archived)) { okhubimport_add_term_meta($term_id, 'archived', $category->archived, TRUE); }
      if (isset($category->level)) { okhubimport_add_term_meta($term_id, 'level', $category->level, TRUE); }
      if (isset($category->cat_parent)) { okhubimport_add_term_meta($term_id, 'cat_parent', $category->cat_parent, TRUE); }
      if (isset($category->cat_superparent)) { okhubimport_add_term_meta($term_id, 'cat_superparent', $category->cat_superparent, TRUE); }
      if (isset($category->category_id)) { okhubimport_add_term_meta($term_id, 'category_id', $category->category_id, TRUE); }
    }
    if (isset($category->metadata_url)) {
      $metadata_url = $category->metadata_url;
      if ($okhub_api_key) {
        $metadata_url .= '?' . OKHUB_API_KEY_PAR . '=' . $okhub_api_key;
      }
      okhubimport_add_term_meta($term_id, 'metadata_url', $metadata_url, TRUE);
    }
  }
  else {
    okhubapi_register_error('okhubimport',  __('Error inserting category ') . $term_args['name'] . ': ' . $new_term->get_error_message(), 'okhubimport_insert_category', 'warning');
  }
  return $term_id;
}

// Update term with imported category.
// Returns NULL if there was an error when updating.
// TODO - process categories that are now archived. Delete them?
function okhubimport_update_category($term_id, $dataset, $taxonomy, $category, $term_args) {
  $okhub_api_key = okhubapi_variable_get('okhubimport', 'api_key', '');
  $countries_taxonomy = $dataset . '_countries';
  $regions_taxonomy = $dataset . '_regions';
  $object_id_field = $dataset . '_object_id';
  $update_parent = FALSE;
  $new_term_id = NULL;
  $new_term  = wp_update_term($term_id, $taxonomy, $term_args);
  if (!is_wp_error($new_term)) {
    $new_term_id = $new_term['term_id'];
    okhubimport_update_term_meta($term_id, $object_id_field, $category->object_id, TRUE);
    if (isset($category->site)) { okhubimport_update_term_meta($term_id, 'site', $category->site, TRUE); }
    if (isset($category->timestamp)) { okhubimport_update_term_meta($term_id, 'timestamp', $category->timestamp, TRUE); }
    if (isset($category->website_url)) { okhubimport_update_term_meta($term_id, 'website_url', $category->website_url, TRUE); }
    if ($taxonomy == $countries_taxonomy) {
      if (isset($category->alternative_name)) { okhubimport_add_term_meta($term_id, 'alternative_name', $category->alternative_name, TRUE); }
      if (isset($category->asset_id)) { okhubimport_add_term_meta($term_id, 'asset_id', $category->asset_id, TRUE); }
      if (isset($category->category_region_array)) { okhubimport_update_categories_term($term_id, $dataset, $regions_taxonomy, $category->category_region_array); }
      if (isset($category->iso_number)) { okhubimport_add_term_meta($term_id, 'iso_number', $category->iso_number, TRUE); }
      if (isset($category->iso_three_letter_code)) { okhubimport_add_term_meta($term_id, 'iso_three_letter_code', $category->iso_three_letter_code, TRUE); }
      if (isset($category->iso_two_letter_code)) { okhubimport_add_term_meta($term_id, 'iso_two_letter_code', $category->iso_two_letter_code, TRUE); }
    } else {
      if (isset($category->archived)) { okhubimport_update_term_meta($term_id, 'archived', $category->archived, TRUE); }
      if (isset($category->level)) { okhubimport_update_term_meta($term_id, 'level', $category->level, TRUE); }
      if (isset($category->cat_parent)) { okhubimport_update_term_meta($term_id, 'cat_parent', $category->cat_parent, TRUE); }
      if (isset($category->cat_superparent)) { okhubimport_update_term_meta($term_id, 'cat_superparent', $category->cat_superparent, TRUE); }
      if (isset($category->category_id)) { okhubimport_update_term_meta($term_id, 'category_id', $category->category_id, TRUE); }
    }
    if (isset($category->metadata_url)) {
      $metadata_url = $category->metadata_url;
      if ($okhub_api_key) {
        $metadata_url .= '?' . OKHUB_API_KEY_PAR . '=' . $okhub_api_key;
      }
      okhubimport_update_term_meta($term_id, 'metadata_url', $metadata_url, TRUE);
    }
  }
  else {
    okhubapi_register_error('okhubimport',  __('Error updating category ') . $term_args['name'] . ': ' . $new_term->get_error_message(), 'okhubimport_update_category', 'warning');
  }  
  return $new_term_id;
}

// Generic function to retrieve a term id - creating it if it does not exist.
function okhubimport_insert_term($taxonomy_name, $term_label) {
  $term_id = 0;
  if ($term = term_exists($term_label, $taxonomy_name)) {
    $term_id = $term['term_id'];
  }
  else {
    $term = wp_insert_term($term_label, $taxonomy_name);
    if (!is_wp_error($term)) {
      $term_id = $term['term_id'];
    }
    else {
      okhubapi_register_error('okhubimport',  __('Error inserting term ') . $term_label . ': ' . $term->get_error_message(), 'okhubimport_insert_term', 'warning');
    }
  }
  return $term_id;
}

// Adds document types to posts (when document type is a taxonomy - new or existing).
function okhubimport_update_document_types_post($post_id, $dataset, $document_types) {
  $document_type_taxonomy_name = okhubapi_variable_get('okhubimport', 'import_document_type_taxonomy_'.$dataset, $dataset.'_document_type');
  if ($document_type_taxonomy = get_taxonomy($document_type_taxonomy_name)) {
    if ($document_type_taxonomy->hierarchical) {
      $terms_ids = array();
      foreach ($document_types as $document_type) {
        if ($document_type) {
          $terms_ids[] = okhubimport_insert_term($document_type_taxonomy_name, $document_type);
        }
      }
      if ($terms_ids) {
        $append = ('existing_taxonomy' == okhubapi_variable_get('okhubimport', 'import_document_type_'.$dataset, 'field'));
        wp_set_post_terms($post_id, $terms_ids, $document_type_taxonomy_name, $append);
      }
    }
    else {
      wp_set_post_terms($post_id, $document_types, $document_type_taxonomy_name, TRUE);
    }
  }
}

// Relate a post with its OKHUB categories. If the categories do not exist as terms, they are created.
// TODO. Get the arrays outside of this function. "Clean" previously assigned categories in the 'category' taxonomy in case they changed.
function okhubimport_update_categories_post($post_id, $dataset, $taxonomy, $categories) {
  $okhubimport_new_categories = okhubapi_variable_get('okhubimport', 'import_new_categories', OKHUB_IMPORT_NEW_CATEGORIES);
  $okhubimport_map_categories = okhubapi_variable_get('okhubimport', 'import_map_categories', OKHUB_IMPORT_MAP_CATEGORIES);
  if ($okhubimport_new_categories) {
    // Assign assets to new taxonomy terms corresponding to imported OKHUB categories.
    $terms_ids = okhubimport_get_okhub_categories_terms_ids($dataset, $taxonomy, $categories);
    wp_set_post_terms($post_id, $terms_ids, $taxonomy, FALSE);
  }
  if ($okhubimport_map_categories) {
    // Assign assets to existing Wordpress categories (based on mappings).
    $mappings = okhubimport_get_mappings($taxonomy);
    $tax_mappings = array();
    foreach ($categories as $category) {
      if (isset($mappings[$category->object_id])) {
        $mappings_category = $mappings[$category->object_id];
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
          elseif (is_numeric($mapping)) { // For backwards compatibility
            $tax_mappings['category'][] = $mapping;
          }
        }
      }
    }
    foreach ($tax_mappings as $tax_name => $mappings_ids) {
      if (is_taxonomy_hierarchical($tax_name)) {
        wp_set_post_terms($post_id, $mappings_ids, $tax_name, TRUE);
      }
      else {
        $mappings_names = array();
        foreach ($mappings_ids as $mapping_id) {
          $mapped_term = get_term($mapping_id, $tax_name);
          $mappings_names[] = $mapped_term->name;
        }
        wp_set_post_terms($post_id, $mappings_names, $tax_name, TRUE);
      }
    }
  }
}

function okhubimport_get_mappings($taxonomy_name) {
  $idscats_wpterms = array();
  $mappings = okhubapi_variable_get('okhubimport', $taxonomy_name . '_mappings', array());
  foreach ($mappings as $mapping) {
    $relation = explode(',', $mapping);
    if (isset($relation[0]) && isset($relation[1])) {
      $idscats_wpterms[$relation[0]][] = $relation[1];
    }
  }
  return $idscats_wpterms;
}

// Relate a term with its related OKHUB categories. This currently only applies to countries (that are linked to regions).
function okhubimport_update_categories_term($term_id, $dataset, $taxonomy, $categories) {
  $okhubimport_new_categories = okhubapi_variable_get('okhubimport', 'import_new_categories', OKHUB_IMPORT_NEW_CATEGORIES);
  if ($okhubimport_new_categories) {
    $terms_ids = okhubimport_get_okhub_categories_terms_ids($dataset, $taxonomy, $categories);
    wp_set_post_terms($term_id, $terms_ids, $taxonomy, FALSE);
  }
}

// Given an array of categories objects, return an array of their corresponding term ids. New terms are created if they don't exist.
function okhubimport_get_okhub_categories_terms_ids($dataset, $taxonomy, $categories) {
  $terms_ids = array();
  foreach ($categories as $category) {
    $category_name = okhubimport_get_category_name($category);
    if ($category_name) {
      $category_name .= " ($category->object_id)";
      if ($term = term_exists($category_name, $taxonomy)) {
        if (is_array($term)) {
          $term_id = $term['term_id'];
        }
        else {
          $term_id = $term;
        }
      }
      else { // The term does not exist, insert it.
        $term_id = okhubimport_insert_category($dataset, $taxonomy, $category, array('name' => $category_name));
      }
      if ($term_id) {
        $terms_ids[] = $term_id;
      }
      else {
        okhubapi_register_error('okhubimport',  __('Category not inserted.'), 'okhubimport_get_okhub_categories_terms_ids', 'warning');
      }
    }
  }
  return $terms_ids;
}

//------------------------ Functions to manage periodic imports ------------------------

// Define new cron intervals
function okhubimport_cron_intervals($schedules) {
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

// Schedule periodic imports
function okhubimport_schedule_imports() {
  $okhubimport_new_categories = okhubapi_variable_get('okhubimport', 'import_new_categories', OKHUB_IMPORT_NEW_CATEGORIES);
  $import_assets['documents'] = okhubapi_variable_get('okhubimport', 'import_documents', 0);
  $import_assets['organisations'] = okhubapi_variable_get('okhubimport', 'import_organisations', 0);
  $import_categories['countries'] = okhubapi_variable_get('okhubimport', 'import_countries', 0);
  $import_categories['regions'] = okhubapi_variable_get('okhubimport', 'import_regions', 0);
  $import_categories['themes'] = okhubapi_variable_get('okhubimport', 'import_themes', 0);
  $frequency_assets = okhubapi_variable_get('okhubimport', 'import_period_assets', OKHUB_IMPORT_IMPORT_RECURRENCE_ASSETS);
  $frequency_categories = okhubapi_variable_get('okhubimport', 'import_period_categories', OKHUB_IMPORT_IMPORT_RECURRENCE_CATEGORIES);
  okhubimport_unschedule_imports();
  if ($frequency_assets) {
    foreach ($import_assets as $assets_type => $enabled) {
      if ($enabled) {
        $schedule_result = wp_schedule_event(time(), $frequency_assets, 'okhubimport_scheduled_events', array($assets_type));
      }
    }
  }
  if ($frequency_categories && $okhubimport_new_categories) {
    foreach ($import_categories as $categories_type => $enabled) {
      if ($enabled) {
        $schedule_result = wp_schedule_event(time(), $frequency_categories, 'okhubimport_scheduled_events', array($categories_type));
      }
    }
  }
}

// Call to periodic imports
function okhubimport_run_periodic_imports($type) {
  $date = date(DATE_RFC822);
  $okhubimport_import_dates = okhubapi_variable_get('okhubimport', 'import_dates', array('eldis' => array(), 'bridge' => array()));
  $okhubimport_import_results = okhubapi_variable_get('okhubimport', 'import_results', array('eldis' => array(), 'bridge' => array()));
  $okhubimport_new_categories = okhubapi_variable_get('okhubimport', 'import_new_categories', OKHUB_IMPORT_NEW_CATEGORIES);
  $datasets = okhubimport_get_datasets();
  $importer = new OKHUB_Importer();
  switch ($type) {
    case 'documents':
      foreach ($datasets as $dataset) {
        $importer->terms[$dataset][$type] = array('new' => 0, 'updated' => 0);
        $importer->import_assets($dataset, 'documents');
        $okhubimport_import_dates[$dataset]['documents'] = $date;
        $okhubimport_import_results[$dataset]['documents'] = $importer->terms[$dataset]['documents'];
      }
      break;
    case 'organisations':
      foreach ($datasets as $dataset) {
        $importer->terms[$dataset][$type] = array('new' => 0, 'updated' => 0);
        $importer->import_assets($dataset, 'organisations');
        $okhubimport_import_dates[$dataset]['organisations'] = $date;
        $okhubimport_import_results[$dataset]['organisations'] = $importer->terms[$dataset]['organisations'];
      }
      break;
    case 'countries':
      if ($okhubimport_new_categories) {
        foreach ($datasets as $dataset) {
          $importer->terms[$dataset][$type] = array('new' => 0, 'updated' => 0);
          $importer->import_categories($dataset, 'countries');
          $okhubimport_import_dates[$dataset]['countries'] = $date;
          $okhubimport_import_results[$dataset]['countries'] = $importer->terms[$dataset]['countries'];
        }
      }
      break;
    case 'regions': 
      if ($okhubimport_new_categories) {
        foreach ($datasets as $dataset) {
          $importer->terms[$dataset][$type] = array('new' => 0, 'updated' => 0);
          $importer->import_categories($dataset, 'regions');
          $okhubimport_import_dates[$dataset]['regions'] = $date;
          $okhubimport_import_results[$dataset]['regions'] = $importer->terms[$dataset]['regions'];
        }
      }
      break;
    case 'themes':
      if ($okhubimport_new_categories) {
        foreach ($datasets as $dataset) {
          $importer->terms[$dataset][$type] = array('new' => 0, 'updated' => 0);
          $importer->import_categories($dataset, 'themes');
          $okhubimport_import_dates[$dataset]['themes'] = $date;
          $okhubimport_import_results[$dataset]['themes'] = $importer->terms[$dataset]['themes'];
        }
      }
      break;
  }  
  okhubapi_variable_set('okhubimport', 'import_dates', $okhubimport_import_dates);
  okhubapi_variable_set('okhubimport', 'import_results', $okhubimport_import_results);
}

// Unschedule all the import events
function okhubimport_unschedule_imports() {
  wp_clear_scheduled_hook('okhubimport_scheduled_events', array('documents'));
  wp_clear_scheduled_hook('okhubimport_scheduled_events', array('organisations'));
  wp_clear_scheduled_hook('okhubimport_scheduled_events', array('countries'));
  wp_clear_scheduled_hook('okhubimport_scheduled_events', array('regions'));
  wp_clear_scheduled_hook('okhubimport_scheduled_events', array('themes'));
}





