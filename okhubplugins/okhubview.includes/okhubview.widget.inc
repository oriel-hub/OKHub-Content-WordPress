<?php

//-------------------------------- OKHUB API Widget ------------------------------
/**
 * OkhubApiWidget class.
 *
 */
class OKHUB_View_Widget extends WP_Widget {

  public function __construct() {
    parent::__construct("text_widget", "OKHUB View Widget",
      array("description" => "Displays content from the OKHUB collection using the OKHUB API."));
  }

  /**
  * Back-end widget form.
  * @see WP_Widget::form()
  * @param array $instance Previously saved values from database.
  */
  public function form($instance) {
    if (isset($instance['okhubview_widget_show_type'])) {
      $show_type = $instance['okhubview_widget_show_type'];
    }
    else {
      $show_type = OKHUB_VIEW_WIDGET_ASSETS_TYPE;
    }
    if (isset($instance['okhubview_widget_title'])) {
			$title = $instance['okhubview_widget_title'];
		}
		else {
			$title = __(OKHUB_VIEW_WIDGET_TITLE);
		}
    if (isset($instance['okhubview_widget_num_items'])) {
			$num_items = (is_numeric($instance['okhubview_widget_num_items']) ? $instance['okhubview_widget_num_items'] : OKHUB_VIEW_WIDGET_NUM_ITEMS);
		}
		else {
			$num_items = OKHUB_VIEW_WIDGET_NUM_ITEMS;
		}
    if (isset($instance['okhubview_widget_dataset'])) {
			$dataset = $instance['okhubview_widget_dataset'];
		}
		else {
			$dataset = okhubapi_variable_get('okhubview', 'default_dataset', OKHUB_API_DEFAULT_DATASET);
		}
    if (isset($instance['okhubview_widget_apply_filters'])) {
			$apply_filters = $instance['okhubview_widget_apply_filters'];
		}
		else {
			$apply_filters = FALSE;
		}
    if (isset($instance['okhubview_widget_show_errors'])) {
			$show_errors = $instance['okhubview_widget_show_errors'];
		}
		else {
			$show_errors = FALSE;
		}
    ?>
		<p>
		<label for="<?php echo $this->get_field_id('okhubview_widget_title'); ?>"><?php _e('Title to display'); ?></label> 
		<input id="<?php echo $this->get_field_id('okhubview_widget_title'); ?>" name="<?php echo $this->get_field_name('okhubview_widget_title'); ?>" type="text" value="<?php echo esc_attr($title); ?>" />
    <br /><br />
    <label for="<?php echo $this->get_field_id('okhubview_widget_dataset'); ?>"><?php _e('Dataset'); ?></label> 
    <input id="<?php echo $this->get_field_id('okhubview_widget_dataset'); ?>" name="<?php echo $this->get_field_name('okhubview_widget_dataset'); ?>" type="radio" value="hub" <?php checked('hub', $dataset); ?> /> Hub</label> &nbsp;&nbsp;&nbsp;
  <br /><br />
		<label for="<?php echo $this->get_field_id('okhubview_widget_num_items'); ?>"><?php _e('Number of items to display'); ?></label> 
		<input id="<?php echo $this->get_field_id('okhubview_widget_num_items'); ?>" name="<?php echo $this->get_field_name('okhubview_widget_num_items'); ?>" type="text" value="<?php echo $num_items; ?>" size="10"/>
    <br /><br />
    <label for="<?php echo $this->get_field_id('okhubview_widget_show_type'); ?>"><?php _e('Type of content'); ?></label><br>
    <?php echo okhub_select_box($this->get_field_name('okhubview_widget_show_type'), $this->get_field_id('okhubview_widget_show_type'), array('documents' => __('Documents'), 'organisations' => __('Organisations')), array($show_type)); ?>
    <br /><br />
    <label for="<?php echo $this->get_field_id('okhubview_widget_apply_filters'); ?>"><input id="<?php echo $this->get_field_id('okhubview_widget_apply_filters'); ?>" name="<?php echo $this->get_field_name('okhubview_widget_apply_filters'); ?>" type="checkbox" value="1" <?php  checked($apply_filters, TRUE); ?> /> <?php _e('Apply OKHUB View Plugin\'s filters'); ?></label>
    <br /><br />
    <label for="<?php echo $this->get_field_id('okhubview_widget_show_errors'); ?>"><input id="<?php echo $this->get_field_id('okhubview_widget_show_errors'); ?>" name="<?php echo $this->get_field_name('okhubview_widget_show_errors'); ?>" type="checkbox" value="1" <?php  checked($show_errors, TRUE); ?> /> <?php _e('Show error messages'); ?></label>
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
    $instance['okhubview_widget_title'] = strip_tags($new_instance['okhubview_widget_title']);
    $instance['okhubview_widget_show_type'] = $new_instance['okhubview_widget_show_type'];
    $instance['okhubview_widget_num_items'] = $new_instance['okhubview_widget_num_items'];
    $instance['okhubview_widget_dataset'] = $new_instance['okhubview_widget_dataset'];
    $instance['okhubview_widget_apply_filters'] = $new_instance['okhubview_widget_apply_filters'];
    $instance['okhubview_widget_show_errors'] = $new_instance['okhubview_widget_show_errors'];
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
    $api_key_validated = okhubapi_variable_get('okhubview', 'api_key_validated', FALSE);
    $api_key = okhubapi_variable_get('okhubview', 'api_key', FALSE);
    $filter_params = array();
    $age_results = 0;
    $assets = array();
    if ($api_key_validated) {
      $num_items = $instance['okhubview_widget_num_items'];
      $type = $instance['okhubview_widget_show_type'];
      $dataset = $instance['okhubview_widget_dataset'];
      $page_name = $dataset . 'view_' . $type;
      $page_id = okhubapi_variable_get('okhubview', $page_name, 0);
      $apply_filters = $instance['okhubview_widget_apply_filters'];
      $show_errors = $instance['okhubview_widget_show_errors'];
      if ($apply_filters) {
        $age_results = okhubapi_variable_get('okhubview', 'age_new_assets', OKHUB_API_AGE_NEW_ASSETS);
        $view_query = okhubapi_variable_get('okhubview', 'view_query', '');
        $countries_assets = okhubapi_variable_get('okhubview', $dataset . '_countries_assets', array());
        $regions_assets = okhubapi_variable_get('okhubview', $dataset . '_regions_assets', array());
        $themes_assets = okhubapi_variable_get('okhubview', $dataset . '_themes_assets', array());
        if ($view_query) {
          $search_params = explode(',', $view_query);
          foreach ($search_params as $search_param) {
            $param = explode('=', trim($search_param));
            $key = $param[0];
            $value = $param[1];
            $filter_params[$key] = $value;
          }
        }
        if (!empty($countries_assets)) {
          $filter_params['country'] = implode('|', $countries_assets);
        }
        if (!empty($regions_assets)) {
          $filter_params['region'] = implode('|', $regions_assets);
        }
        if (!empty($themes_assets)) {
          $filter_params['theme'] = implode('|', $themes_assets);
        }
        if ($type == 'documents') { // Document-specific filters
          $authors_assets = okhubapi_variable_get('okhubview', 'view_authors_assets', '');
          $publishers_assets = okhubapi_variable_get('okhubview', 'view_publishers_assets', '');
          $language_name_codes = okhubapi_variable_get('okhubview', 'language_name_codes', array());
          if ($authors_assets) {
            $authors = explode(',', $authors_assets);
            $list_authors = implode('|', array_map('trim', $authors));
            $filter_params['author'] = $list_authors;
          }
          if ($publishers_assets) {
            $publishers = explode(',', $publishers_assets);
            $list_publishers = implode('|', array_map('trim', $publishers));
            $filter_params['publisher_name'] = $list_publishers;
          }
          /*if ($language_name_codes) {
            $ids_languages = okhub_languages();
            $list_language_names = array();
            foreach ($language_name_codes as $language_code) {
              if (isset($ids_languages[$language_code])) {
                $list_language_names[] = $ids_languages[$language_code]; 
              }
              if ($list_language_names) {
                $language_names = implode('|', $list_language_names);
                $filter_params['language_name'] = $language_names;
              }
            }
          }*/
        }
        $num_retrieved = 0;
        $total_results = 0;
        $offset = 0;
        $more_results = TRUE;
        while (($num_retrieved < $num_items) && $more_results) {
          /*if ($offset) {
            $filter_params['start_offset'] = $offset;
          }*/
          $response = $okhubapi->search('documents', 'hub', $api_key, 'full', 10, 0, $offset, $filter_params);
         // $response = $okhubapi->search($type, $dataset, $api_key, 'short', $num_items, $age_results, $filter_params, array('urls', 'language_array', 'website_url'));
          if ($response->isError()) {
            $error_message = __('No content retrieved by the API call. ') . $response->getErrorMessage();
            okhubapi_register_error('okhubview', $error_message, 'OKHUB_View_Widget::widget', 'warning');
            $more_results = FALSE;
          }
          else {
            $assets_retrieved = $response->getResults();
            if (!empty($assets_retrieved)) {
              foreach($assets_retrieved as $asset) {
                if (!okhub_exclude_asset('okhubview', $asset)) {
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
      	$response = $okhubapi->search($type,$dataset, $api_key, 'full', $num_items, 0,$offset,$filter_params);
        //$response = $okhubapi->search($type, $dataset, $api_key, 'short', $num_items, $age_results, $filter_params, array('website_url'));
        if ($response->isError()) {
          $error_message = __('No content retrieved by the API call. ') . $response->getErrorMessage();
          okhubapi_register_error('okhubview', $error_message, 'OKHUB_View_Widget::widget', 'warning');
        }
        else {
          $assets = $response->getResults();
          if (isset($response->next_page)){
				$parsed_url=parse_url($response->next_page);
				$p = explode("=",$parsed_url['query']);
				$start_offset_next=$p[count($p)-1];
          }
            if (isset($response->previous_page)){
				$parsed_url=parse_url($response->previous_page);
				$p = explode("=",$parsed_url['query']);
				$start_offset_previous=$p[count($p)-1];
          }        
        }
      }
      echo '<div class="ids-widget">';
      if (!empty($assets)) {
        $okhubview_default_language = okhubapi_variable_get('okhubview', 'language', OKHUB_VIEW_DEFAULT_LANGUAGE);
        $title = apply_filters('widget_title', $instance['okhubview_widget_title']);
        if (!empty($title)) {
          echo '<h3 class="widget-title">' . $title . '</h3>';
        }
        echo '<ul class="ids-widget-list">';
        foreach ($assets as $asset) {

          if ($apply_filters) {
            $asset_title = okhub_get_translation($asset, 'title', $okhubview_default_language);
          }
          else {
            $asset_title = $asset->title;
          }
          $link = '';
          echo '<li class="ids-widget-item">';
          if ($page_id) {
            $link = get_permalink($page_id);
            $par = ((preg_match('[\?]', $link)) ? '&' : '?');
            $link = $link . $par . 'object_id=' . $asset->object_id['hub'];
            echo '<a href="' . $link . '">' . $asset_title . '</a>';
          }
          elseif (isset($asset->url)) {
            $link = $asset->url;
            echo '<a href="' . $link . '" target=_new>' . $asset_title . '</a>';
          }else {
            echo $asset_title;
          }
        }
        echo '</ul>';
      }
      else {
        echo '<small>' . __('There are no matching assets to display.') . '</small><br/><br/>';
      }
      echo '</div>';
      if ($show_errors) {
        okhubapi_report_errors();
      }
    }
    else {
      echo '<small>' . __('A valid OKHUB API key has to be entered before using this widget.') . '</small><br/><br/>';
    }
  }
}


