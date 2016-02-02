<?php

//-------------------------------- OKHub Widget ------------------------------

/**
 * OKHub Widget class.
 *
 */

class OKHub_Widget extends WP_Widget {

	public function __construct() {
		$widget_ops = array( 
			//'class_name' => 'OKHub_Widget',
			'description' => 'Retrieves and displays content from the Open Knowledge Hub without importing it.',
		);
		parent::__construct( 'okhub_widget', 'OKHub Content', $widget_ops );
	}

  /**
  * Back-end widget form.
  * @see WP_Widget::form()
  * @param array $instance Previously saved values from database.
  */
  public function form($instance) {
    $array_datasets = okhub_datasets_names_with_content();
    if (isset($instance['okhub_widget_show_type'])) {
      $show_type = $instance['okhub_widget_show_type'];
    }
    else {
      $show_type = OKHUB_WIDGET_ASSETS_TYPE;
    }
    if (isset($instance['okhub_widget_title'])) {
			$title = $instance['okhub_widget_title'];
		}
		else {
			$title = __(OKHUB_WIDGET_TITLE);
		}
    if (isset($instance['okhub_widget_num_items'])) {
			$num_items = (is_numeric($instance['okhub_widget_num_items']) ? $instance['okhub_widget_num_items'] : OKHUB_WIDGET_NUM_ITEMS);
		}
		else {
			$num_items = OKHUB_WIDGET_NUM_ITEMS;
		}
    if (isset($instance['okhub_widget_dataset'])) {
			$dataset = $instance['okhub_widget_dataset'];
		}
		else {
			$dataset = okhubapi_variable_get('okhub', 'default_dataset', OKHUB_DEFAULT_DATASET);
		}
    if (isset($instance['okhub_widget_apply_filters'])) {
			$apply_filters = $instance['okhub_widget_apply_filters'];
		}
		else {
			$apply_filters = FALSE;
		}
    if (isset($instance['okhub_widget_query'])) {
			$query = $instance['okhub_widget_query'];
		}
		else {
			$query = '';
		}
    if (isset($instance['okhub_widget_display_source'])) {
			$display_source = $instance['okhub_widget_display_source'];
		}
		else {
			$display_source = FALSE;
		}
    if (isset($instance['okhub_widget_display_description'])) {
			$display_description = $instance['okhub_widget_display_description'];
		}
		else {
			$display_description = FALSE;
		}
    if (isset($instance['okhub_widget_show_errors'])) {
			$show_errors = $instance['okhub_widget_show_errors'];
		}
		else {
			$show_errors = FALSE;
		}
    ?>
		<p>
		<label for="<?php echo $this->get_field_id('okhub_widget_title'); ?>"><?php _e('Title to display'); ?></label> 
		<input id="<?php echo $this->get_field_id('okhub_widget_title'); ?>" name="<?php echo $this->get_field_name('okhub_widget_title'); ?>" type="text" value="<?php echo esc_attr($title); ?>" />
    <br /><br />

    <label for="<?php echo $this->get_field_id('okhub_widget_dataset'); ?>"><?php _e('Sources'); ?></label> 
    <!--?php echo okhub_select_box($this->get_field_name('okhub_widget_dataset').'[]', $this->get_field_id('okhub_widget_dataset'), $array_datasets, $datasets, array('multiple' => 'multiple')); ?-->
    <!-- Allowing one dataset per widget by now -->
    <?php echo okhub_select_box($this->get_field_name('okhub_widget_dataset'), $this->get_field_id('okhub_widget_dataset'), $array_datasets, array($dataset)); ?>

    <br /><br />

		<label for="<?php echo $this->get_field_id('okhub_widget_num_items'); ?>"><?php _e('Number of items to display'); ?></label> 
		<input id="<?php echo $this->get_field_id('okhub_widget_num_items'); ?>" name="<?php echo $this->get_field_name('okhub_widget_num_items'); ?>" type="text" value="<?php echo $num_items; ?>" size="10"/>
    <br /><br />

    <label for="<?php echo $this->get_field_id('okhub_widget_show_type'); ?>"><?php _e('Type of content'); ?></label><br>
    <?php echo okhub_select_box($this->get_field_name('okhub_widget_show_type'), $this->get_field_id('okhub_widget_show_type'), array('documents' => __('Documents'), 'organisations' => __('Organisations')), array($show_type)); ?>
    <br /><br />

    <label for="<?php echo $this->get_field_id('okhub_widget_apply_filters'); ?>"><input id="<?php echo $this->get_field_id('okhub_widget_apply_filters'); ?>" name="<?php echo $this->get_field_name('okhub_widget_apply_filters'); ?>" type="checkbox" value="1" <?php  checked($apply_filters, TRUE); ?> /> <?php _e('Apply OKHub Content Plugin\'s filters (language, categories, etc.)'); ?></label>
    <br /><br />

		<label for="<?php echo $this->get_field_id('okhub_widget_query'); ?>"><?php _e('Additional search terms'); ?></label> 
		<input id="<?php echo $this->get_field_id('okhub_widget_query'); ?>" name="<?php echo $this->get_field_name('okhub_widget_query'); ?>" type="text" value="<?php echo $query; ?>" /></br>
    <small><i>
    <?php printf(__('Please refer to <a href="%s" target="_new">the API documentation</a>.'), OKHUB_API_SEARCH_DOCUMENTATION); ?>
    </i></small>
    <br /><br />

    <label for="<?php echo $this->get_field_id('okhub_widget_display_source'); ?>"><input id="<?php echo $this->get_field_id('okhub_widget_display_source'); ?>" name="<?php echo $this->get_field_name('okhub_widget_display_source'); ?>" type="checkbox" value="1" <?php  checked($display_source, TRUE); ?> /> <?php _e('Display source in results'); ?></label>
    <br /><br />

    <label for="<?php echo $this->get_field_id('okhub_widget_display_description'); ?>"><input id="<?php echo $this->get_field_id('okhub_widget_display_description'); ?>" name="<?php echo $this->get_field_name('okhub_widget_display_description'); ?>" type="checkbox" value="1" <?php  checked($display_description, TRUE); ?> /> <?php _e('Display description in results'); ?></label>
    <br /><br />

    <label for="<?php echo $this->get_field_id('okhub_widget_show_errors'); ?>"><input id="<?php echo $this->get_field_id('okhub_widget_show_errors'); ?>" name="<?php echo $this->get_field_name('okhub_widget_show_errors'); ?>" type="checkbox" value="1" <?php  checked($show_errors, TRUE); ?> /> <?php _e('Show error messages'); ?></label>
		</p>
    <?php
  }

  /**
  * Sanitize widget form values as they are saved.
  * @see WP_Widget::update()
  * @param array $new_instance Values just sent to be saved.
  * @param array $old_instance Previously saved values from database.
  * @return array Updated safe values to be saved.
  */
  public function update($new_instance, $old_instance) {
    $instance = array();
		$instance['okhub_widget_title'] = (isset($new_instance['okhub_widget_title'])) ? strip_tags($new_instance['okhub_widget_title']) : __(OKHUB_WIDGET_TITLE);
    $instance['okhub_widget_show_type'] = (isset($new_instance['okhub_widget_show_type'])) ? $new_instance['okhub_widget_show_type'] : OKHUB_WIDGET_ASSETS_TYPE;
    $instance['okhub_widget_num_items'] = (isset($new_instance['okhub_widget_num_items'])) ? $new_instance['okhub_widget_num_items'] : OKHUB_WIDGET_NUM_ITEMS;
    $instance['okhub_widget_dataset'] = (isset($new_instance['okhub_widget_dataset'])) ? $new_instance['okhub_widget_dataset'] : okhubapi_variable_get('okhub', 'default_dataset', OKHUB_DEFAULT_DATASET);
    $instance['okhub_widget_apply_filters'] = (isset($new_instance['okhub_widget_apply_filters'])) ? $new_instance['okhub_widget_apply_filters'] : FALSE;
		$instance['okhub_widget_query'] = (isset($new_instance['okhub_widget_query'])) ? strip_tags($new_instance['okhub_widget_query']) : '';
    $instance['okhub_widget_display_source'] = (isset($new_instance['okhub_widget_display_source'])) ? $new_instance['okhub_widget_display_source'] : FALSE;
    $instance['okhub_widget_display_description'] = (isset($new_instance['okhub_widget_display_description'])) ? $new_instance['okhub_widget_display_description'] : FALSE;
    $instance['okhub_widget_show_errors'] = (isset($new_instance['okhub_widget_show_errors'])) ? $new_instance['okhub_widget_show_errors'] : FALSE;
    return $instance;
  }

  /**
  * Front-end display of widget.
  * @see WP_Widget::widget()
  * @param array $args     Widget arguments.
  * @param array $instance Saved values from database.
  */
  public function widget($args, $instance) {
    $okhubapi = new OkhubApiWrapper;
    $api_key_validated = okhubapi_variable_get('okhub', 'api_key_validated', FALSE);
    $api_key = okhubapi_variable_get('okhub', 'api_key', FALSE);
    $filter_params = array();
    $assets = array();
    $default_language = okhubapi_variable_get('okhub', 'default_language', OKHUB_DEFAULT_LANGUAGE);
    $language_filters = okhubapi_variable_get('okhub', 'language_filters_codes', array()); 
    $preferred_languages = array_unique(array_merge(array($default_language), $language_filters));
    $extra_fields = array(OKHUB_API_FN_WEBSITE_URL, OKHUB_API_FN_DESCRIPTION, OKHUB_API_FN_SOURCES);
    if ($api_key_validated) {
      $num_items = $instance['okhub_widget_num_items'];
      $type = $instance['okhub_widget_show_type'];
      $display_source = $instance['okhub_widget_display_source'];
      $display_description = $instance['okhub_widget_display_description'];
      $dataset = $instance['okhub_widget_dataset'];
      $filter_params[OKHUB_API_PARAMETER_SOURCES] = $dataset;
      $apply_filters = $instance['okhub_widget_apply_filters'];
      $show_errors = $instance['okhub_widget_show_errors'];
      if ($search_query = $instance['okhub_widget_query']) {
        $search_params = explode(',', $search_query);
        foreach ($search_params as $search_param) {
          $param = explode('=', trim($search_param));
          $key = $param[0];
          $value = $param[1];
          $filter_params[$key] = $value;
        }
      }
      if ($apply_filters) {
        $age_results = okhubapi_variable_get('okhub', 'age_new_assets', OKHUB_API_AGE_NEW_ASSETS);
        $import_query = okhubapi_variable_get('okhub', 'import_query', '');
        $countries_assets = okhubapi_variable_get('okhub', $dataset . '_countries_assets', array());
        $regions_assets = okhubapi_variable_get('okhub', $dataset . '_regions_assets', array());
        $themes_assets = okhubapi_variable_get('okhub', $dataset . '_themes_assets', array());
        $documenttypes_assets = okhubapi_variable_get('okhub', $dataset . '_documenttypes_assets', array());
        if ($import_query) {
          $search_params = explode(',', $import_query);
          foreach ($search_params as $search_param) {
            $param = explode('=', trim($search_param));
            $key = $param[0];
            $value = $param[1];
            $filter_params[$key] = $value;
          }
        }
        if (!empty($countries_assets)) {
          $filter_params[OKHUB_API_PARAMETER_COUNTRY_ID] = implode('|', $countries_assets);
        }
        if (!empty($regions_assets)) {
          $filter_params[OKHUB_API_PARAMETER_REGION_ID] = implode('|', $regions_assets);
        }
        if (!empty($themes_assets)) {
          $filter_params[OKHUB_API_PARAMETER_THEME_ID] = implode('|', $themes_assets);
        }
        if (!empty($documenttypes_assets)) {
          $filter_params[OKHUB_API_PARAMETER_DOCUMENT_TYPE_ID] = implode('|', $documenttypes_assets);
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
          if ($language_filters) {
            $language_codes = implode('|', $language_filters);
            $filter_params[OKHUB_API_PARAMETER_LANGUAGES] = $language_codes;
          }
        }
        $num_responses = 0;
        $num_retrieved = 0;
        $total_results = 0;
        $offset = 0;
        $more_results = TRUE;
        $exclude_uris = okhubapi_variable_get('okhub', 'exclude_uris', '');
        while ($num_retrieved < $num_items && $num_responses < OKHUB_HARD_LIMIT_ASSETS && $more_results) {
          $response = $okhubapi->search($type, OKHUB_HUB_SOURCE_CODE, $api_key, 'short', $num_items, $age_results, $offset, $filter_params, $extra_fields);
          if ($response->isEmpty()) {
            $error_message = __('No content retrieved by the API call. Please check your active filters.');
            if ($response->isError()) {
              $error_message .= $response->getErrorMessage();
            }
            okhubapi_register_error('okhub', $error_message, 'OKHub_Widget::widget', 'warning');
            $more_results = FALSE;
          }
          else {
            $assets_retrieved = $response->getResults();
            if (!empty($assets_retrieved)) {
              $num_responses += count($assets_retrieved);
              foreach ($assets_retrieved as $asset) {
                if (!okhub_exclude_asset($asset, $dataset, $exclude_uris) && okhub_has_language_content($asset, $dataset, $language_filters)) {
                  $assets[] = $asset;
                  $num_retrieved++;
                }
                $offset++; 
                if ($num_retrieved == $num_items) {
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
      }
      else {
        $response = $okhubapi->search($type, OKHUB_HUB_SOURCE_CODE, $api_key, 'short', $num_items, 0, 0, $filter_params, $extra_fields);
        if ($response->isError()) {
          $error_message = __('No content retrieved by the API call.') . $response->getErrorMessage();
          okhubapi_register_error('okhub', $error_message, 'OKHub_Widget::widget', 'warning');
        }
        else {
          $assets = $response->getResults();
        }
      }
      echo '<div class="okhub-widget">';
      if (!empty($assets)) {
        $title = apply_filters('widget_title', $instance['okhub_widget_title']);
        if (!empty($title)) {
          echo '<h3 class="widget-title">' . $title . '</h3>';
        }
        echo '<ul class="okhub-widget-list">';
        foreach ($assets as $asset) {         
          if ($apply_filters) {
            $title_language = okhub_api_value_languages($asset, OKHUB_API_FN_TITLE, $dataset, $preferred_languages);
            $description_language = okhub_api_value_languages($asset, OKHUB_API_FN_DESCRIPTION, $dataset, $preferred_languages);
          }
          else {
            $title_language = okhub_api_value_languages($asset, OKHUB_API_FN_TITLE, $dataset, array_keys(okhub_languages()));
            $description_language = okhub_api_value_languages($asset, OKHUB_API_FN_DESCRIPTION, $dataset, array_keys(okhub_languages()));
          }
          if ($asset_title = current($title_language)) {
            $asset_description = current($description_language);
            $link = '';
            if ($website_url = okhub_get_api_value($asset, OKHUB_API_FN_WEBSITE_URL, $dataset)) { 
              $link = $website_url;
            }
            else {
              $link = OKHUB_DATAHUB_URL . '/item/' . okhub_get_api_value($asset, OKHUB_API_FN_ITEM_ID);
            }
            echo '<li class="okhub-widget-item">';
            if ($link) {
              echo '<a href="' . $link . '">' . $asset_title . '</a>';
            }
            else {
              echo $asset_title;
            }
            if ($asset_description && $display_description) {
              echo '<div class="okhub-widget-description">'.$asset_description.'</div>';
            }
            if ($display_source) {
              echo '<div class="okhub-widget-source">'.__('Source: ').okhub_get_dataset_acronym($dataset).'</div>';
            }
            echo '</li>';
          }
        }
        echo '</ul>';
      }
      else {
        echo '<small>' . __('There are no matching assets to display.') . '</small><br/><br/>';
      }
      echo '</div>';
/*
      if ($show_errors) {
        okhubapi_report_errors();
      }
*/
    }
    else {
      echo '<small>' . __('A valid OKHub API key has to be entered before using this widget.') . '</small><br/><br/>';
    }
  }
}


