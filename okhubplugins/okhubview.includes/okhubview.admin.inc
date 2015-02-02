<?php

// Checks if it has to display the admin settings or if it has to initialize the plugin's data, etc.
function okhubview_admin_main() {
  if (!current_user_can( 'manage_options')) {
    wp_die(__('You do not have sufficient permissions to manage the plugin settings.'));
  }
  okhubview_render_form();
}

// Display registered error messages
function okhubview_admin_notices() {
  settings_errors('okhubview_options', FALSE, TRUE);
  okhubapi_report_errors();
}

// Returns an array with the selected datasets.
function okhubview_get_datasets() {
  global $okhub_datasets;
  $default_dataset = okhubapi_variable_get('okhubview', 'default_dataset', OKHUB_API_DEFAULT_DATASET);
  if ($default_dataset == 'all') {
    $datasets = $okhub_datasets;
  }
  else {
    $datasets = array($default_dataset);
  }
  return $datasets;
}

// When a dataset is disabled, the status of the page that shows its content is changed to 'pending'.
function okhubview_update_pages_status() {
  global $okhub_datasets;
  global $okhub_assets;
  $current_datasets = okhubview_get_datasets();
  foreach ($okhub_datasets as $dataset) {
    foreach ($okhub_assets as $type) {
      $exclude = okhubapi_exclude($dataset, $type);
      if (!$exclude) {
        $page = get_page_by_path($dataset . 'view_' . $type);
        if (isset($page->ID)) {
          if (in_array($dataset, $current_datasets)) {
            $page->post_status = 'publish';
          }
          else {
            $page->post_status = 'pending';
          }
          wp_update_post((array) $page);
        }
      }
    }
  }
}

// Render the plugin's main admin form.
function okhubview_render_form() {
  global $okhub_datasets;
  global $okhub_categories;
  okhubview_templates_exist();
  okhubapi_report_errors();
  $okhubview_api_key_validated = okhubapi_variable_get('okhubview', 'api_key_validated', FALSE);
  ?>
  <div class = "wrap">
	<div class="ids-wrap">
		<!-- Display Plugin Icon, Header, and Description -->
		<div class="icon32" id="icon-options-general"></div>
		<h2><?php _e('OKhub View Plugin Settings'); ?></h2>
		<p>
      <?php printf(__('This plugin provides access to the OKHub datasets via the Open Knowledge Hub API (<a href="%s" target="_new">%s</a>).'), OKHUB_API_HOME_URL, OKHUB_API_HOME_URL); ?>
      <br />
      <br />
      <i>
      <?php _e('Please look through all the tabs on this page to set your configuration options. Changes will only be effective after submitting them by clicking on the "Save changes" button.'); ?></i>
    </p>

    <!-- Beginning of the Plugin Options Form -->
    <form method="post" id="ids_view_form" action="options.php">
      <?php
          settings_fields('okhubview');
      ?>

    <div class="ui-tabs">
      <ul class="ui-tabs-nav">
        <li><a href="#general"><?php _e('General settings'); ?></a></li>
        <li><a href="#filters"><?php _e('Filters'); ?></a></li>
      </ul>

      <!---------------------------------- GENERAL SETTINGS ------------------------------->

      <h3><?php _e('General settings'); ?></h3>
			<table class="form-table ui-tabs-panel" id="general">
				<!-- OKHUB API Key -->
				<tr>
					<th scope="row"><?php _e('OKHUB API Token-GUID (key)'); ?></th>
					<td>
						<input type="text" size="57" name="okhubview_options[api_key]" value="<?php echo okhubapi_variable_get('okhubview', 'api_key', ''); ?>" /><br />
						<p class="description">
						<?php printf(__('If you do not have an OKHUB API key, please request one at <a href="%s" target="_new">%s</a>'), OKHUB_API_KEY_URL, OKHUB_API_KEY_URL); ?>
						</p>
					</td>
				</tr>

      <?php if ($okhubview_api_key_validated) { ?>

        <?php okhubview_update_pages_status(); ?>

        <!-- Dataset -->
        <tr valign="top">
          <th scope="row"><?php _e('Display content from:'); ?></th>
          <td id="radio_dataset">
            <label><input name="okhubview_options[default_dataset]" type="radio" value="hub" <?php checked('hub', okhubapi_variable_get('okhubview', 'default_dataset', OKHUB_API_DEFAULT_DATASET)); ?> onchange="changeDataset();" /> OKHUB</label> <br />
            </td>
        </tr>

        <!-- Languages -->
        <!--<tr>
          <th scope="row"><?php _e('Default language'); ?></th>
          <td>
            <?php echo okhub_select_box('okhubview_options[language]', 'okhubview_language', okhub_languages(), array(okhubapi_variable_get('okhubview', 'language', IDS_VIEW_DEFAULT_LANGUAGE))); ?>
            <p class="description">
            <?php _e('Translatable metadata (title, description) will be displayed in this language, if available.'); ?>
            </p>
          </td>
        </tr>
        -->
        <!-- Number of assets to retrieve -->
        <tr>
          <th scope="row"><?php _e('Excerpts length'); ?></th>
          <td>
            <input type="text" size="10" name="okhubview_options[excerpt_length]" value="<?php echo okhubapi_variable_get('okhubview', 'excerpt_length', 0); ?>" />
            <p class="description">
            <?php _e('Maximum length of excerpts to display.'); ?>
            <br/>
            <?php _e('If not set (or if the value is 0) the full description will be used.'); ?>
            </p>
          </td>
        </tr>

      </table>

      <h3><?php _e('Filters'); ?></h3>
      <table class="form-table ui-tabs-panel" id="filters">
        <tr>
          <td colspan="2">
            <p class="description"><?php _e('Documents and/or organisations to be displayed can be limited to those matching the following filters.'); ?></p>
          </td>
        </tr>

        <!-- IDS Categories (countries, regions, themes) -->
        <?php foreach ($okhub_datasets as $dataset) { ?>
        <tr class="ids_categories_<?php echo $dataset; ?>">
          <td colspan="2">
            <p id="title_categories" class="ids-settings-separator ids-categories-<?php echo $dataset; ?>"><?php echo ucfirst($dataset); ?> categories</p>
          </td>
        </tr>
        <?php foreach (array('countries', 'regions') as $category) { 
        	$field_id = $dataset . '_' . $category . '_assets';
        	$option_name = $dataset . '_' . $category . '_assets';
        	$category_name = ucfirst($category);
        ?>
        <tr class="ids-categories-<?php echo $dataset; ?>">
         <th scope="row"><?php _e($category_name); ?></th>
          <td>
            <select data-placeholder="Select <?php echo $category; ?>..." class="chzn-select" style="width:350px;" id='<?php echo $field_id; ?>' name='okhubview_options[<?php echo $option_name; ?>][]' multiple='multiple' size='10'></select>
            <p class="description"><?php _e("Display only documents/organisations with the selected focus $category."); ?>
            <a href="javascript:deselectAll('<?php echo $field_id; ?>');"><?php _e('Deselect all'); ?></a>
            </p>
          </td>
        </tr>
        <?php } // countries, regions ?>

        <!-- Themes trees -->
        <?php $themes_dropdown_id = 'dropDownButton_' . $dataset; ?>
        <?php $themes_tree_id = 'jqxTree_' . $dataset; ?>
        <tr class="ids-categories-<?php echo $dataset; ?>">
         <th scope="row"><?php _e('Themes'); ?></th>
          <!-- Themes select boxes -->
          <td valign="top">
          <?php
          $field_id = $dataset . '_themes_assets';
          $option_name = $dataset . '_themes_assets';
          ?>
          <div class="ids-categories-themes">
            <select data-placeholder="No themes selected" class="chzn-select" style="width:350px;" id='<?php echo $field_id; ?>' name='okhubview_options[<?php echo $option_name; ?>][]' multiple='multiple' size='10'></select><br>
          </div>

          <div id='<?php echo $themes_dropdown_id; ?>' style="float:left; margin-right:40px;">
            <div id='<?php echo $themes_tree_id; ?>'>
            </div>
          </div>

          <div style="clear:both;">
          <p class="description">
            <br>
            Import only documents/organisations with the selected focus themes.
            <a href="javascript:removeAll('<?php echo $field_id; ?>');"><?php _e('Deselect all'); ?></a>
          </p>
          </div>

          </td>
        </tr>

        <?php } // foreach dataset ?>

        <tr>
          <td colspan="2">
            <p class="ids-settings-separator">Additional filters</p>
          </td>
        </tr>

        <!-- Age of assets to view -->
        <!--<tr>
         <th scope="row"><?php _e('Age of documents and/or organisations'); ?></th>
          <td>
            <?php echo okhub_select_box('okhubview_options[age_new_assets]', 'okhubview_age_new_assets', array('0' => __('Do not filter by age'), '7' => __('One week'), '30' => __('One month'), '180' => __('Six months'), '365' => __('One year')), array(okhubapi_variable_get('okhubview', 'age_new_assets', IDS_API_AGE_NEW_ASSETS))); ?>
            <p class="description"><?php _e('Only documents/organisations published after the indicated time will be shown.'); ?></p>
          </td>
        </tr>-->

        <!-- Resources' languages -->
        <!--<tr>
          <th scope="row"><?php _e('Resources\' languages'); ?></th>
          <td>
            <?php echo okhub_select_box('okhubview_options[language_name_codes][]', 'okhubview_language_name_codes', okhub_languages(), okhubapi_variable_get('okhubview', 'language_name_codes', array()), array('multiple' => 'multiple')); ?>
            <a href="javascript:deselectAll('okhubview_language_name_codes');"><?php _e('Deselect all'); ?></a>
            <p class="description">
            <?php _e('Only display resources that are available in the selected languages. If none is selected, resources in all languages will be displayed.'); ?>
            </p>
          </td>
        </tr>-->

        <!-- Exclude URIs -->
        <tr>
          <th scope="row"><?php _e('Exclude URIs'); ?></th>
          <td>
            <input type="text" size="57" name="okhubview_options[exclude_uris]" value="<?php echo okhubapi_variable_get('okhubview', 'exclude_uris', get_bloginfo('url')); ?>" /><br />
            <p class="description">
            <?php _e('Exclude resources that match certain URIs. Partial matching will be applied. Use commas to indicate several URIs.'); ?><br />
            <?php _e('This filter can be used, for instance, to avoid displaying resources that are originated from this site.'); ?><br />
            <?php _e('Example: <i>http://www.worldbank.org/en/news, undp.org</i> would exclude WB\'s press releases and all resources from servers with undp.org as domain name.'); ?></p>
          </td>
        </tr>

        <!-- Authors -->
        <tr>
          <th scope="row"><?php _e('Documents\' author(s)'); ?></th>
          <td>
            <input type="text" size="57" name="okhubview_options[view_authors_assets]" value="<?php echo okhubapi_variable_get('okhubview', 'view_authors_assets', ''); ?>" /><br />
            <p class="description">
            <?php _e('A list of comma-separated authors\' names.'); ?><br />
            <?php _e('Example: <i>Chomsky, Foucault</i> will retrieve documents with either <i>Chomsky</i> or <i>Foucault</i> in the "author" field.'); ?></p>
          </td>
        </tr>

        <!-- Publishers -->
        <tr>
          <th scope="row"><?php _e('Documents\' publisher(s)'); ?></th>
          <td>
            <input type="text" size="57" name="okhubview_options[view_publishers_assets]" value="<?php echo okhubapi_variable_get('okhubview', 'view_publishers_assets', ''); ?>" /><br />
            <p class="description">
            <?php _e('A list of comma-separated publishers\' names.'); ?><br />
            <?php _e('Example: <i>World Bank, UNDP</i> will retrieve documents with either <i>World Bank</i> or <i>UNDP</i> in the "publisher" field.'); ?></p>
          </td>
        </tr>

        <!-- Additional query terms -->
        <tr>
          <th scope="row"><?php _e('Query terms'); ?></th>
          <td>
            <input type="text" size="57" name="okhubview_options[view_query]" value="<?php echo okhubapi_variable_get('okhubview', 'view_query', ''); ?>" /><br />
            <p class="description">
            <?php _e('Query string to be appended to the API URL.'); ?><br />
            <?php _e('Use comma-separated pairs of unencoded query fields and values (example: <i>country=Argentina, keyword=climate change</i>).'); ?><br />
            <?php printf(__('Please check the <a href="%s" target="_new">online documentation</a> for a list of available query fields.'), IDS_API_SEARCH_FIELDS_URL); ?>
           </p>
          </td>
        </tr>
      </table>

    </div>	<!-- UI tabs -->	

      <p class="submit">
        <input name="okhubview_options[submit_save]" type="submit" class="button-primary" value="<?php _e('Save changes') ?>" />
      </p>

      <?php } else { // If API key is not validated ?>

      </table>
      <p class="submit ids-view-settings-message">
        <?php _e('Please validate your OKHUB API key in order to continue.'); ?>
        <input name="okhubview_options[submit_validate]" type="submit" class="button-primary" value="<?php _e('Validate') ?>" />
      </p>
      <?php } ?>
		</form>
    </div>
  </div>
  <?php
}

// Validate input and the API key.
function okhubview_validate_options($input) {
  $okhubapi = new OkhubApiWrapper;
  $error = FALSE;
  $error_message = '';
  $okhubview_api_key_validated = okhubapi_variable_get('okhubview', 'api_key_validated', FALSE);
  $okhubview_default_dataset = okhubapi_variable_get('okhubview', 'default_dataset', OKHUB_API_DEFAULT_DATASET);
  $reg_api_key = '/^[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}$/';
  $reg_api_url = '/^http:\/\/[\/0-9a-z\.]+$/';
  //$reg_api_menu_title = '/^[0-9a-zA-Z_\ ]+$/';
  $allowed_html = array();

  $current_permalinks = get_option('permalink_structure');
  $input['permalink_structure'] = okhubapi_variable_get('okhubview', 'permalink_structure', $current_permalinks);
  $input['hubview_documents'] = okhubapi_variable_get('okhubview', 'hubview_documents', 0);
  $input['hubview_organisations'] = okhubapi_variable_get('okhubview', 'hubview_organisations', 0);
  //$input['bridgeview_documents'] = okhubapi_variable_get('okhubview', 'bridgeview_documents', 0);

  if ((isset($input['api_key'])) && preg_match($reg_api_key, $input['api_key'])) {
    if ($okhubapi->validateKey($input['api_key'])) {
        $input['api_key_validated'] = TRUE;
    }
    else {
      $input['api_key'] = '';
      $input['api_key_validated'] = FALSE;
      $error = TRUE;
      $error_message = __(' The OKHUB API call did not retrieve results. Please check that you have entered a valid API key.');
    }
  }
  else {
    $input['api_key'] = '';
    $input['api_key_validated'] = FALSE;
    $error = TRUE;
    $error_message = __(' The OKHUB API key format is not valid. Please enter a valid API key.');
  }
  if ($okhubview_api_key_validated) { //If the API key was not validated, the other fields have not been displayed, so there's nothing else to check.
    if (isset($input['excerpt_length'])) {
      if (!is_numeric($input['excerpt_length'])) {
        $error = TRUE;
        $error_message .= ' ' . $input['excerpt_length'] . __(' is not a valid length. Please enter a numeric value.');
        $input['excerpt_length'] = 0;
      }
    }
    if (isset($input['exclude_uris'])) {
      $input['exclude_uris'] = wp_kses($input['exclude_uris'], $allowed_html);
    }
    if (isset($input['view_authors_assets'])) {
      $input['view_authors_assets'] = wp_kses($input['view_authors_assets'], $allowed_html);
    }
    if (isset($input['view_publishers_assets'])) {
      $input['view_publishers_assets'] = wp_kses($input['view_publishers_assets'], $allowed_html);
    }
    if (isset($input['view_query'])) {
      $input['view_query'] = wp_kses($input['view_query'], $allowed_html);
    }
  }
  if ($error) {
    $input['setup_done'] = FALSE;
    add_settings_error('okhubview_options', 'okhubview_errors', $error_message);
  }
  else {
    $input['setup_done'] = TRUE;
  }
  return $input;
}
