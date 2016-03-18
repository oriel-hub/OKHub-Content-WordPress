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
    add_action('wp_enqueue_scripts', array(&$this, 'js'));
		parent::__construct( 'okhub_widget', 'OKHub Content', $widget_ops );
	}

  function js(){
    if (is_active_widget(false, false, $this->id_base, true)) {
      wp_register_script('okhub_widget_javascript', OKHUB_SCRIPTS_PATH . '/okhub_widget.js');
      wp_enqueue_script('okhub_widget_javascript');
      wp_register_script('okhub_bpopup', OKHUB_SCRIPTS_PATH . '/bpopup/jquery.bpopup.min.js');
      wp_enqueue_script('okhub_bpopup');
      wp_register_script('okhub_jquery_easing', OKHUB_SCRIPTS_PATH . '/bpopup/jquery.easing.1.3.js');
      wp_enqueue_script('okhub_jquery_easing');
      wp_enqueue_script('jquery-ui-dialog');
      okhub_init_javascript_widget();
    }           
  }

  /**
  * Back-end widget form.
  * @see WP_Widget::form()
  * @param array $instance Previously saved values from database.
  */
  public function form($instance) {
    $array_datasets = okhub_datasets_names_with_content();
    $datasets_assets = array();
    foreach ($array_datasets as $code => $name) {
      if (okhub_has_content($code, 'documents') || okhub_has_content($code, 'organisations')) {
        $datasets_assets[$code] = $name;
      }
    }
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
      if ($num_items > OKHUB_WIDGET_MAX_ITEMS) {
        $num_items = OKHUB_WIDGET_MAX_ITEMS;
      }
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
    foreach (array('documents', 'organisations') as $asset_type) {
      if (isset($instance['okhub_widget_use_template_' . $asset_type])) {
        $use_template[$asset_type] = $instance['okhub_widget_use_template_' . $asset_type];
      }
      else {
        $use_template[$asset_type] = FALSE;
      }
    }
    if (isset($instance['okhub_widget_display_source'])) {
			$display_source = $instance['okhub_widget_display_source'];
		}
		else {
			$display_source = TRUE;
		}
    if (isset($instance['okhub_widget_open_pop_up'])) {
			$open_pop_up = $instance['okhub_widget_open_pop_up'];
		}
		else {
			$open_pop_up = TRUE;
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
    <hr>
    <b>Filters</b>
    <hr>

    <label for="<?php echo $this->get_field_id('okhub_widget_dataset'); ?>"><?php _e('Sources'); ?></label> 
    <!--?php echo okhub_select_box($this->get_field_name('okhub_widget_dataset').'[]', $this->get_field_id('okhub_widget_dataset'), $datasets_assets, $datasets, array('multiple' => 'multiple')); ?-->
    <!-- Allowing one dataset per widget by now -->
    <?php echo okhub_select_box($this->get_field_name('okhub_widget_dataset'), $this->get_field_id('okhub_widget_dataset'), $datasets_assets, array($dataset)); ?>

    <br /><br />

		<label for="<?php echo $this->get_field_id('okhub_widget_num_items'); ?>"><?php _e('Number of items to display'); ?></label> 
		<input id="<?php echo $this->get_field_id('okhub_widget_num_items'); ?>" name="<?php echo $this->get_field_name('okhub_widget_num_items'); ?>" type="text" value="<?php echo $num_items; ?>" size="10"/>
    <br /><br />

    <label for="<?php echo $this->get_field_id('okhub_widget_show_type'); ?>"><?php _e('Type of content'); ?></label><br>
    <?php echo okhub_select_box($this->get_field_name('okhub_widget_show_type'), $this->get_field_id('okhub_widget_show_type'), array('documents' => __('Documents'), 'organisations' => __('Organisations')), array($show_type), array('onchange'=>'changeTypeOKHubWidget(\''.$this->get_field_id('okhub_widget_show_type').'\')')); ?>
    <br /><br />

    <label for="<?php echo $this->get_field_id('okhub_widget_apply_filters'); ?>"><input id="<?php echo $this->get_field_id('okhub_widget_apply_filters'); ?>" name="<?php echo $this->get_field_name('okhub_widget_apply_filters'); ?>" type="checkbox" value="1" <?php  checked($apply_filters, TRUE); ?> /> <?php _e('Apply OKHub Content Plugin\'s filters (language, categories, etc.)'); ?></label>
    <br /><br />

		<label for="<?php echo $this->get_field_id('okhub_widget_query'); ?>"><?php _e('Additional search terms'); ?></label> 
		<input id="<?php echo $this->get_field_id('okhub_widget_query'); ?>" name="<?php echo $this->get_field_name('okhub_widget_query'); ?>" type="text" value="<?php echo $query; ?>" /></br>
    <small><i>
    <?php printf(__('Please refer to <a href="%s" target="_new">the API documentation</a>.'), OKHUB_API_SEARCH_DOCUMENTATION); ?>
    </i></small>
    <br /><br />

    <hr>
    <b>Display results</b>
    <hr>

    <?php
    $template_name['documents'] = OKHUB_WIDGET_TEMPLATE_DOCUMENTS;
    $template_name['organisations'] = OKHUB_WIDGET_TEMPLATE_ORGANISATIONS;
    foreach (array('documents', 'organisations') as $asset_type) {
      $template_file[$asset_type] = plugin_dir_path( __DIR__ ) . OKHUB_TEMPLATES_DIR .'/'. $template_name[$asset_type];
      $template_exists[$asset_type] = file_exists($template_file[$asset_type]);
      $field_id = 'okhub_widget_use_template_' . $asset_type;
    ?>
      <div class="okhub-widget-div-templates" id="okhub-widget-div-template-<?php echo "$asset_type"; ?>" <?php echo ($show_type != $asset_type) ? 'style="display:none"' : ''; ?>>
        <label for="<?php echo $this->get_field_id($field_id); ?>">
          <input class="okhub-widget-div-check-templates" id="<?php echo $this->get_field_id($field_id); ?>" name="<?php echo $this->get_field_name($field_id); ?>" type="checkbox" value="1" <?php  checked($use_template[$asset_type], TRUE); ?> <?php if (!$template_exists[$asset_type]) echo 'disabled'; ?> onchange="useTemplateOKHubWidget('<?php echo $this->get_field_id($field_id); ?>')"/> <?php echo __('Use ') . $asset_type . __(' template'); ?>
        </label>
        <br/>
        <small><?php echo __('If checked, the template "') . $template_name[$asset_type] . __('" will be used to display retrieved ') . $asset_type; ?></small>
        <br/>
        <?php
        if (!$template_exists[$asset_type]) {
          echo '<small><i>' . __('To use this option, the template file "') . $template_name[$asset_type] . __('" should be placed in the directory "') . OKHUB_TEMPLATES_DIR . '".</i></small>';
        }
        ?>
        <br /><br />
      </div>
    <?php
    } // foreach asset type
    ?>
    <div class="okhub-widget-div-no-templates" <?php echo ($use_template[$show_type]) ? 'style="display:none"' : ''; ?>>
      <label for="<?php echo $this->get_field_id('okhub_widget_title'); ?>"><?php _e('Title to display'); ?></label> 
      <input id="<?php echo $this->get_field_id('okhub_widget_title'); ?>" name="<?php echo $this->get_field_name('okhub_widget_title'); ?>" type="text" value="<?php echo esc_attr($title); ?>" />
      <br /><br />

      <label for="<?php echo $this->get_field_id('okhub_widget_open_pop_up'); ?>"><input id="<?php echo $this->get_field_id('okhub_widget_open_pop_up'); ?>" name="<?php echo $this->get_field_name('okhub_widget_open_pop_up'); ?>" type="checkbox" value="1" <?php  checked($open_pop_up, TRUE); ?> /> <?php _e('Open in modal pop-up'); ?></label>
      <br /><br />

      <label for="<?php echo $this->get_field_id('okhub_widget_display_source'); ?>"><input id="<?php echo $this->get_field_id('okhub_widget_display_source'); ?>" name="<?php echo $this->get_field_name('okhub_widget_display_source'); ?>" type="checkbox" value="1" <?php  checked($display_source, TRUE); ?> /> <?php _e('Display source in results'); ?></label>
      <br /><br />

      <label for="<?php echo $this->get_field_id('okhub_widget_display_description'); ?>"><input id="<?php echo $this->get_field_id('okhub_widget_display_description'); ?>" name="<?php echo $this->get_field_name('okhub_widget_display_description'); ?>" type="checkbox" value="1" <?php  checked($display_description, TRUE); ?> /> <?php _e('Display short description'); ?></label>
      <br /><br />
    </div>

    <?php if (OKHUB_WIDGET_ERROR_MESSAGES) { ?>
      <label for="<?php echo $this->get_field_id('okhub_widget_show_errors'); ?>"><input id="<?php echo $this->get_field_id('okhub_widget_show_errors'); ?>" name="<?php echo $this->get_field_name('okhub_widget_show_errors'); ?>" type="checkbox" value="1" <?php  checked($show_errors, TRUE); ?> /> <?php _e('Show error messages'); ?></label>
    <?php } ?>
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
    $instance['okhub_widget_use_template_documents'] = (isset($new_instance['okhub_widget_use_template_documents'])) ? $new_instance['okhub_widget_use_template_documents'] : FALSE;
    $instance['okhub_widget_use_template_organisations'] = (isset($new_instance['okhub_widget_use_template_organisations'])) ? $new_instance['okhub_widget_use_template_organisations'] : FALSE;
    $instance['okhub_widget_display_source'] = (isset($new_instance['okhub_widget_display_source'])) ? $new_instance['okhub_widget_display_source'] : FALSE;
    $instance['okhub_widget_open_pop_up'] = (isset($new_instance['okhub_widget_open_pop_up'])) ? $new_instance['okhub_widget_open_pop_up'] : FALSE;
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
    $data = array();
    $okhubapi = new OkhubApiWrapper;
    $api_key_validated = okhubapi_variable_get('okhub', 'api_key_validated', FALSE);
    $api_key = okhubapi_variable_get('okhub', 'api_key', FALSE);
    $filter_params = array();
    $assets = array();
    $default_language = okhubapi_variable_get('okhub', 'default_language', OKHUB_DEFAULT_LANGUAGE);
    $language_filters = okhubapi_variable_get('okhub', 'language_filters_codes', array()); 
    $extra_fields = array();
    if ($api_key_validated) {
      $num_items = $instance['okhub_widget_num_items'];
      $type = $instance['okhub_widget_show_type'];
      $use_template = $instance['okhub_widget_use_template_'.$type];
      $display_source = $instance['okhub_widget_display_source'];
      $display_description = $instance['okhub_widget_display_description'];
      $open_pop_up = $instance['okhub_widget_open_pop_up'];
      $dataset = $instance['okhub_widget_dataset'];
      $source = okhub_get_source($dataset);
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
      $filter_params[OKHUB_SORT_ORDER] = OKHUB_SORT_FIELD;      
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
        $exclude_uris = okhubapi_variable_get('okhub', 'exclude_uris', '');
        $preferred_languages = array_unique(array_merge(array($default_language), $language_filters));
      }
      else {
        $age_results = 0;
        $preferred_languages = array_unique(array_merge(array($default_language), array_keys(okhub_languages())));
        $exclude_uris = array();       
      }
      $num_responses = 0;
      $num_retrieved = 0;
      $total_results = 0;
      $offset = 0;
      $more_results = TRUE;
      while ($num_retrieved < $num_items && $num_responses < OKHUB_HARD_LIMIT_ASSETS && $more_results) {
        $response = $okhubapi->search($type, OKHUB_HUB_SOURCE_CODE, $api_key, 'full', $num_items, $age_results, $offset, $filter_params, $extra_fields);
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
              if (!okhub_exclude_asset($asset, $dataset, $exclude_uris) && okhub_has_language_content($asset, $dataset, $preferred_languages)) {
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
      $output = '<aside class="okhub-widget">';
      if (!empty($assets)) {
        if (!$use_template) {
          $title = apply_filters('widget_title', $instance['okhub_widget_title']);
          if (!empty($title)) {
            echo '<h3 class="widget-title">' . $title . '</h3>';
          }
        }
        echo '<ul class="okhub-widget-list">';
        foreach ($assets as $asset) {
          // Check that it has content in the preferred language. This can be removed after the hub's content is filtered and the right assets are returned for lang_only queries.
          $title_language = okhub_api_value_languages($asset, OKHUB_API_FN_TITLE, $dataset, $preferred_languages);
          if ($asset_title = current($title_language)) {
            $item_id = okhub_get_api_value($asset, OKHUB_API_FN_ITEM_ID);
            $widget_assets[$item_id] = $asset_title;
            if ($use_template) { // Display template
              $data[] = $asset;
            }
            else { // Displayed results when no template is used. Only link and source and description if checked.
              $link = '';
              if ($website_url = okhub_get_api_value($asset, OKHUB_API_FN_WEBSITE_URL, $dataset)) { 
                $link = $website_url;
              }
              elseif ($url = okhub_get_api_value($asset, OKHUB_API_FN_URLS, $dataset, '', TRUE)) {
                $link = $url;
              }
              else {
                $link = OKHUB_DATAHUB_URL . '/item/' . okhub_get_api_value($asset, OKHUB_API_FN_ITEM_ID);
              }

              echo '<li class="okhub-widget-item">';
              echo '<div class="okhub-widget-title">';
              if ($link) {
                $class_link = ($open_pop_up) ? 'class="okhub-widget-open-pop-up"' : '';
                echo '<a '.$class_link.' href="' . $link . '">'.$asset_title.'</a>';
              }
              else {
                echo $asset_title;
              }
              echo '</div>';
              if ($display_description) {
                $description_language = okhub_api_value_languages($asset, OKHUB_API_FN_DESCRIPTION, $dataset, $preferred_languages);
                if ($asset_description = current($description_language)) {
                  $first_sentence = preg_replace('/([^?!.]*.).*/', '\\1', $asset_description);
                  echo '<div class="okhub-widget-description">'.$first_sentence.'</div>';
                }
              }
              //$source[OKHUB_API_FN_SOURCE_DESCRIPTION]
              if ($display_source) {
                if (!empty($source[OKHUB_API_FN_SOURCE_NAME])) {
                  $dataset_name = $source[OKHUB_API_FN_SOURCE_NAME];
                }
                else {
                  $dataset_name = okhub_get_dataset_acronym($dataset);
                }
                if (!empty($source[OKHUB_API_FN_SOURCE_WEBSITE])) {
                  $dataset_name = '<a href="'.$source[OKHUB_API_FN_SOURCE_WEBSITE].'">'.$dataset_name.'</a>';
                }                              
                echo '<div class="okhub-widget-source">'.__('Source: ').$dataset_name.'</div>';
              }
              echo '</li>';
            }
          }
        }
        if ($use_template) {
          switch ($type) {
            case 'documents':
              $template_name = OKHUB_WIDGET_TEMPLATE_DOCUMENTS;
              break;
            case 'organisations':
              $template_name = OKHUB_WIDGET_TEMPLATE_ORGANISATIONS;
              break;
          }
          $template_file = plugin_dir_path( __DIR__ ) . OKHUB_TEMPLATES_DIR .'/'. $template_name;
          if (file_exists($template_file)) {
            $this::displayTemplate($dataset, $data, $template_file);
          }
          else {
            okhubapi_register_error('okhub', __('The template file was not found.'), 'OKHub_Widget::widget', 'error');
          }
        }
        else {
          echo '</ul>';
        }
        ?>
        <div id="okhub_widget_dialog" class="okhub-popup">
          <span class="button b-close"><span>X</span></span>
          <div class="okhub-popup-content">
          </div>
        </div>
        <script type="text/javascript">
        <?php
        foreach ($widget_assets as $item_id => $title) {
        ?>
          okhub_widget_assets.push({'item_id': '<?php echo $item_id; ?>', 'type': '<?php echo $type; ?>', 'source': '<?php echo $dataset; ?>','title': '<?php echo addslashes($title); ?>'});
        <?php
        }
        ?>
        </script>
        <?php
      }
      else {
        echo '<small>' . __('There are no matching assets to display.') . '</small><br/><br/>';
      }
      if ($show_errors) {
        okhubapi_report_errors();
      }
      echo '</aside>';
    }
    else {
      echo '<small>' . __('A valid OKHub API key has to be entered before using this widget.') . '</small><br/><br/>';
    }
  }

  public static function displayTemplate($okhub_source, $okhub_data, $template_file) {
    include($template_file);
  }

  public static function displayItem($item_id, $okhub_source, $type) {
    if ($item = okhub_get_item($type, $item_id)) {
      $template_name['documents'] = OKHUB_WIDGET_ITEM_TEMPLATE_DOCUMENTS;
      $template_name['organisations'] = OKHUB_WIDGET_ITEM_TEMPLATE_ORGANISATIONS;
      $template_file[$type] = plugin_dir_path( __DIR__ ) . OKHUB_TEMPLATES_DIR .'/'. $template_name[$type];
      if ($template_exists[$type] = file_exists($template_file[$type])) {
        self::displayTemplate($okhub_source, $item, $template_file[$type]);
      }
      else {
        $sources_codes = okhub_get_api_value($item, OKHUB_API_FN_SOURCES, '', '', FALSE); // Get the item's sources' codes.
        ?>
        <div id="okhub-widget-tabs">  
        <?php
        foreach ($sources_codes as $source_code) { // Display logos / names for each source.
            $source = okhub_get_source($source_code); // Retrieve array with source information. See okhub.default.inc for a list of all available sources' fields.
            $sources[$source_code] = $source;
            $selected = ($source_code == $okhub_source) ? 'okhub-widget-selected' : '';
            if (!empty($source[OKHUB_API_FN_SOURCE_LOGO])) {
            //echo '<a href="javascript:displayContentOKHubWidget(\''.$source_code.'\')">;
            echo '<img id="okhub-source-tab-widget-'.$source_code.'" class="okhub-source-logo-widget okhub-source-tab-widget '.$selected.'" src="'.$source[OKHUB_API_FN_SOURCE_LOGO].'" onclick="displayContentOKHubWidget(\''.$source_code.'\')">';
            // echo '</a>';
            }
            elseif (!empty($source[OKHUB_API_FN_SOURCE_NAME])) {
                echo '<span id="okhub-source-tab-widget-'.$source_code.'" class="okhub-source-link-widget okhub-source-tab-widget '.$selected.'" onclick="javascript:displayContentOKHubWidget(\''.$source_code.'\')">'.$source[OKHUB_API_FN_SOURCE_NAME].'</span>';  
            }
            else {
                echo '<span id="okhub-source-tab-widget-'.$source_code.'" class="okhub-source-link-widget okhub-source-tab-widget '.$selected.'" onclick="javascript:displayContentOKHubWidget(\''.$source_code.'\')">'.$source_code.'</span>';
            }
        }
        ?>
        </div>

        <?php
        foreach ($sources_codes as $source_code) {  // Display content for each source.
            $title = okhub_get_api_value($item, OKHUB_API_FN_TITLE, $source_code, 'en');
            $description = okhub_get_api_value($item, OKHUB_API_FN_DESCRIPTION, $source_code, 'en');
        ?>
        <div id="okhub-widget-content-<?php echo $source_code; ?>" class="okhub-widget-content" <?php echo ($source_code != $okhub_source) ? 'style="display:none"' : ''; ?>>
            <!-- Title -->
            <div class="okhub-widget-item-title">
                <h2><?php echo $title; ?></h2>
            </div>
            <!-- Description -->
            <div class="okhub-widget-item-description">
                <p><?php echo $description; ?></p>
            </div>
            <div>
                <?php
                if ($website_url = okhub_get_api_value($item, OKHUB_API_FN_WEBSITE_URL, $source_code)) {
                ?>
                    <b>URL:</b> <a href="<?php echo $website_url; ?>"><?php echo $website_url; ?></a>
                <?php
                }
                ?>
                <br/><br/>
                <!------ Source description ------>
                <p class="okhub-widget-source-description">
                    <?php echo (!empty($sources[$source_code][OKHUB_API_FN_SOURCE_DESCRIPTION])) ? $sources[$source_code][OKHUB_API_FN_SOURCE_DESCRIPTION] : ''; ?>
                </p>
            </div>
        </div>
        <?php
        }
      }
    }
  }

}


