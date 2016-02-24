<?php

// Plugin's page from where to access all its services
function okhub_general_page() {
  $okhub_categories = okhub_categories();
  $okhub_assets = okhub_assets();
  $datasets = okhub_selected_datasets();
  $datasets_names = okhub_datasets_names();
  $okhubapi = new OkhubApiWrapper;
  ?>
  <div class="okhub-wrap">
  <div id="icon-edit-pages" class="icon32"><br /></div>
  <div id="okhub-logo"><a href="http://api.okhub.org" target="_blank"><img src="<?php echo OKHUB_IMAGES_PATH . '/logo.png';?>"></a></div>
  <h2><?php _e('OKHub Content - Control panel'); ?></h2>
  <br /><br />

  <table class="okhub-table">
    <tr valign="top">
    <td colspan="3">
    <ul class="okhub-category-list">
      <li class="cat-item"> <p><a href="<?php echo get_admin_url() . 'options-general.php?page=okhub'; ?>"><?php _e('Settings'); ?></a></p>
      <li class="cat-item"> <p><a href="<?php echo get_admin_url() . 'admin.php?page=okhub_help'; ?>"><?php _e('Help'); ?></a></p>
    </ul>
    </td>
    </tr>
 
    <tr valign="top">
    <td class="okhub-admin-col1">

    <b><?php _e('Import OKHub content'); ?></b>
    <ul class="okhub-category-list">
    <li class="cat-item"> <p><a href="<?php echo get_admin_url() . 'admin.php?import=okhub_importer'; ?>"><?php _e('Import'); ?></a></p>
    </ul>

    <br/>
    <br/>

    <b><?php _e('OKHub Widget'); ?></b>
    <ul class="okhub-category-list">
    <li class="cat-item"> <p><a href="widgets.php">Add a widget</a></p>
    <i>One or more widgets can be added to your site in order to retrieve and display OKHub content without importing it.</i>
    </ul>

    <br/>
    <br/>


    <b><?php _e('View the imported items on your website'); ?></b>
    <ul class="okhub-category-list">
    <li class="cat-item"> <p><a href="<?php echo get_post_type_archive_link('okhub_documents'); ?>"><?php _e('All OKHub documents'); ?></a></p>
    <li class="cat-item"> <p><a href="<?php echo get_post_type_archive_link('okhub_organisations'); ?>"><?php _e('All OKHub organisations'); ?></a></p>

    <?php
    foreach ($datasets as $dataset) {
      foreach ($okhub_assets as $asset) {
        if (okhub_has_content($dataset, $asset)) {
          $name = okhub_get_dataset_acronym($dataset) . ' ' . ucfirst($asset); 
          $link = okhub_custom_type_name($asset);
          $selected_datasets = okhub_selected_datasets();
          if (in_array($dataset, $selected_datasets)) { // TODO: Generalize.
            $par = ((preg_match('[\?]', $link)) ? '&' : '?');
            $link .= $par . 'okhub_source=' . $dataset;
          }
          ?>
          <li class="cat-item"> <p><a href="<?php echo site_url($link); ?>"><?php echo $name; ?></a></p>
      <?php }
      }
    } ?>
      </ul>
    </td>

    <td class="okhub-admin-col2">
    <b><?php _e('Manage imported assets'); ?></b>
    <ul class="okhub-category-list">
    <?php
    foreach ($datasets as $dataset) {
      foreach ($okhub_assets as $asset) {
        if (okhub_has_content($dataset, $asset)) {
          $name = okhub_get_dataset_acronym($dataset) . ' ' . ucfirst($asset); ?>
          <li class="cat-item"> <p><a href="<?php echo get_admin_url() . 'edit.php?post_type=okhub_' . $asset; ?>&okhub_source=<?php echo $dataset; ?>"><?php _e('Edit'); ?> <?php echo $name; ?></a></p>
      <?php }
      }
    } ?>
    </ul>

    <br/>
    <br/>

    <?php
    $okhub_new_categories = okhubapi_variable_get('okhub', 'import_new_categories', OKHUB_NEW_CATEGORIES);
    if ($okhub_new_categories) { ?>
      <b><?php _e('Manage imported categories'); ?></b>
      <ul class="okhub-category-list">
      <?php
      foreach ($datasets as $dataset) {
        foreach ($okhub_categories as $category) {
          $cat_admin_page = 'edit-tags.php?taxonomy=' . okhub_custom_taxonomy_name($dataset, $category);
          $cat_name = okhub_get_dataset_acronym($dataset) . ' ' . okhub_get_type_name($category, TRUE); ?>
          <li class="cat-item"> <p><a href="<?php echo get_admin_url() . $cat_admin_page; ?>"> <?php echo $cat_name; ?></a></p>
      <?php } ?>
      <?php } ?>
    <?php } ?>
      </ul>
    </td>
    
    <td class="okhub-admin-col3">
    <b><?php _e('Administrative tasks'); ?></b>
    <br/>
    <br/>
    <i><?php _e('Cache management'); ?></i>
    <ul class="okhub-category-list">
    <?php
    $task = '';
    if(isset($_GET['admin_task'])) {
      $task = $_GET['admin_task'];
    }
    if ($task == 'clear_cache') {
      okhub_clear_cache();
      echo '<li class="cat-item">' . '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;' . __('All cache entries have been deleted.');
    }
    else { ?>
      <li class="cat-item"> <a href="<?php echo get_admin_url() . 'admin.php?page=okhub_menu&admin_task=clear_cache'; ?>"><?php _e('Clear all cached data'); ?></a>
    <?php } ?>
    </ul>
    </td>
    </tr>
  </table>
  </div>
<?php
}

// Importer main page
function okhub_importer_form() {
  $okhubapi = new OkhubApiWrapper;
  $okhub_datasets_names = okhub_datasets_names();
  $okhub_datasets = okhub_datasets();
  $okhub_categories = okhub_categories();
  $okhub_new_categories = okhubapi_variable_get('okhub', 'import_new_categories', OKHUB_NEW_CATEGORIES);
  $selected_datasets = okhubapi_variable_get('okhub', 'selected_datasets', array());
  $api_key = okhubapi_variable_get('okhub', 'api_key', '');
  $languages = okhub_languages();

  /* Display latest imported content from each dataset */
  $latest_import_dates = okhubapi_variable_get('okhub', 'import_dates', array());
  if (!empty($latest_import_dates)) {
    $message = __('Latest imports: ') . '<br />';
    $latest_import_results = okhubapi_variable_get('okhub', 'import_results', array());
    foreach ($latest_import_dates as $set => $results_dataset) {
      foreach ($results_dataset as $type => $date) {
        if (okhub_has_content($set, $type)) {
          $new = (isset($latest_import_results[$set][$type]) ? $latest_import_results[$set][$type]['new'] : 0);
          $updated = (isset($latest_import_results[$set][$type]) ? $latest_import_results[$set][$type]['updated'] : 0);
          $message .= okhub_get_dataset_acronym($set) . ' ' . okhub_get_type_name($type, TRUE) . ' imported on ' . $date . ': ' . $new . ' new, ' . $updated . ' updated' . '<br />';
        }
      }
    }
    okhub_show_message($message);
  }
  $message = __('Please note that it can take some time to import a large volume of content. Do not stop your browser.');
  okhub_show_message($message);
  ?>
  <form method="post" action="admin.php?import=okhub_importer&step=1">
    <table class="okhub-table okhub-types">
      <!-- Types of content to import -->
      <tr valign="top">
        <!----------------------- TODO: Select by dataset / content type !!!!!!!!!!!!!!!!!! --------------------->
        <th scope="row"><?php _e('Select the types of content to import'); ?></th>
        <td>
          <label><strong><?php _e('Assets'); ?></strong></label><br/>
          <label><input name="okhub_import_documents" type="checkbox" /> <?php _e('Documents'); ?></label> &nbsp;&nbsp;&nbsp;
          <label><input name="okhub_import_organisations" type="checkbox" /> <?php _e('Organisations'); ?></label>
          <br/>

          <?php if ($okhub_new_categories) { ?>
          <br/>
          <label><strong><?php _e('Categories'); ?></strong></label><br/>
          <label><input name="okhub_import_countries" type="checkbox" /> <?php _e('Countries'); ?></label> &nbsp;&nbsp;&nbsp;
          <label><input name="okhub_import_regions" type="checkbox" /> <?php _e('Regions'); ?></label> &nbsp;&nbsp;&nbsp;
          <label><input name="okhub_import_themes" type="checkbox" /> <?php _e('Themes'); ?></label>
          <label><input name="okhub_import_documenttypes" type="checkbox" /> <?php _e('Document Types'); ?></label>
          <?php } ?>
        </td>
      </tr>
      <tr valign="top">
        <th scope="row"><?php _e('Force updates'); ?></th>
        <td>
          <label><input name="okhub_force_update" type="checkbox" /> <?php _e('Update existing assets independently of the date in which they were previously imported or updated'); ?></label>
        </td>
      </tr>
    </table>
    <?php wp_nonce_field('okhub_import', 'okhub_import_check_referrer'); ?>
    <p class="submit okhub-button">
      <input name="submit_import" type="submit" class="button-primary" value="<?php _e('Import now') ?>" />
    </p>
  </form>

  <table class="okhub-table">
    <tr valign="top">
      <td colspan="2"> 
        <strong><?php _e('Active settings and filters'); ?></strong><br />
        (<?php printf(__('Please go to the <a href="%s">OKHub API administrative page</a> in order to view the details or change these settings.'), 'options-general.php?page=okhub'); ?>)
      </td>
    </tr>
    <tr valign="top">
      <th scope="row"><?php _e('General settings'); ?></th>
      <td>
        <ul>
          <li> <?php echo '<i>' . __('Import from: ') . '</i>' . '<strong>' . implode(', ', $selected_datasets) . '</strong>'; ?>
          <li> <?php echo '<i>' . __('Number of documents/organisations: ') . '</i>' . '<strong>' . (($num_items = okhubapi_variable_get('okhub', 'num_items', OKHUB_API_NUM_ITEMS)) ? $num_items : 'Not specified') . '</strong>'; ?>
          <li> <?php echo '<i>' . __('Updated resources: ') . '</i>' . '<strong>' . okhubapi_variable_get('okhub', 'updated_assets', OKHUB_UPDATED_ASSETS) . '</strong>'; ?>
          <li> <?php echo '<i>' . __('Username assigned to imported content: ') . '</i>' . '<strong>' . okhubapi_variable_get('okhub', 'import_user', OKHUB_IMPORT_USER) . '</strong>'; ?>
          <li> <?php echo '<i>' . __('Import only top-level categories: ') . '</i>' . '<strong>' . (($okhub_top_level_categories = okhubapi_variable_get('okhub', 'top_level_categories', OKHUB_DEFAULT_TOP_LEVEL)) ? $okhub_top_level_categories : 'All levels') . '</strong>' ?>
        </ul>
      </td>
    </tr>
    <tr valign="top">
      <th scope="row"><?php _e('Categories'); ?></th>
      <td>
        <ul>
        <?php
        if ($okhub_new_categories) {
          echo '<li><i>' . __('OKHub categories will be imported and assigned to documents and/or organisations.') . '</i>';
        }
        else {
          echo '<li><i>' . __('Importing of OKHub categories <u>is disabled</u>.') . '</i>';
        }
        ?>
        </ul>
      </td>
    </tr>

    <tr valign="top">
      <th scope="row"><?php _e('Active filters and settings for documents and/or organisations'); ?></th>
      <td>
        <?php
        foreach ($selected_datasets as $dataset) {
          $array_countries = okhub_get_category_array($dataset, 'countries');
          $array_regions = okhub_get_category_array($dataset, 'regions');
          $array_themes = okhub_get_category_array($dataset, 'themes');
          echo '<b><u>' . okhub_get_dataset_acronym($dataset) . ' categories</u></b>';
        ?>
        <ul>
          <li> <?php echo '<i>' . __('Countries: ') . '</i>' . '<strong>' . (($okhub_import_countries_assets = implode(', ', okhub_get_names(okhubapi_variable_get('okhub', $dataset . '_countries_assets', array()), $array_countries))) ? $okhub_import_countries_assets : '<i>none selected</i>') . '</strong>' ?>
          <li> <?php echo '<i>' . __('Regions: ') . '</i>' . '<strong>' . (($okhub_import_regions_assets = implode(', ', okhub_get_names(okhubapi_variable_get('okhub', $dataset . '_regions_assets', array()), $array_regions))) ? $okhub_import_regions_assets : '<i>none selected</i>') . '</strong>' ?>
          <li> <?php echo '<i>' . __('Themes: ') . '</i>' . '<strong>' . (($okhub_import_themes_assets = implode(', ', okhub_get_names(okhubapi_variable_get('okhub', $dataset . '_themes_assets', array()), $array_themes))) ? $okhub_import_themes_assets : '<i>none selected</i>') . '</strong>' ?>
        </ul>
        <?php } ?>
        <b><u><?php _e('Additional filters'); ?></u></b>
        <ul>
          <li> <?php echo '<i>' . __('Age of documents and/or organisations: ') . '</i>' . '<strong>' . (($okhub_age_new_assets = okhubapi_variable_get('okhub', 'age_new_assets', OKHUB_API_AGE_NEW_ASSETS)) ? $okhub_age_new_assets . ' days' : '<i>Do not filter by age</i>') . '</strong>' ?>
          <li> <?php echo '<i>' . __('Resources\' languages: ') . '</i>' . '<strong>' . (($okhub_language_filters_codes = okhubapi_variable_get('okhub', 'language_filters_codes', array())) ? implode(', ', $okhub_language_filters_codes) : '<i>Do not filter by language</i>') . '</strong>' ?>
          <li> <?php echo '<i>' . __('Exclude URIs: ') . '</i>' . '<strong>' . (($okhub_exclude_uris = okhubapi_variable_get('okhub', 'exclude_uris', '')) ? $okhub_exclude_uris : '<i>Do not exclude resources by their URIs</i>') . '</strong>' ?>
          <li> <?php echo '<i>' . __('Documents\' author(s): ') . '</i>' . '<strong>' . (($okhub_import_authors_assets = okhubapi_variable_get('okhub', 'import_authors_assets', '')) ? $okhub_import_authors_assets : '<i>none selected</i>') . '</strong>' ?>
          <li> <?php echo '<i>' . __('Documents\' publisher(s): ') . '</i>' . '<strong>' . (($okhub_import_publishers_assets = okhubapi_variable_get('okhub', 'import_publishers_assets', '')) ? $okhub_import_publishers_assets : '<i>none selected</i>') . '</strong>' ?>
          <li> <?php echo '<i>' . __('Additional query terms: ') . '</i>' . '<strong>' . (($okhub_import_query = okhubapi_variable_get('okhub', 'import_query', '')) ? $okhub_import_query : '<i>none selected</i>') . '</strong>' ?>
        </ul>

        <?php 
          $default_language = okhubapi_variable_get('okhub', 'default_language', OKHUB_DEFAULT_LANGUAGE);
        ?>
        <b><u><?php _e('Import/display settings'); ?></u></b>
        <ul>
          <li> <?php echo '<i>' . __('Preferred language: ') . '</i>' . '<strong>' . $languages[$default_language] . '</strong>'; ?>
          <li> <?php echo '<i>' . __('Additional languages to import: ') . '</i>' . '<strong>' . (($additional_languages = okhubapi_variable_get('okhub', 'additional_languages', array())) ? implode(', ', $additional_languages) : 'None') . '</strong>'; ?>
          <li> <?php echo '<i>' . __('Excerpts length: ') . '</i>' . '<strong>' . (($excerpt_length = okhubapi_variable_get('okhub', 'excerpt_length', 0)) ? $excerpt_length : 'Not specified') . '</strong>'; ?>
        </ul>
      </td>
    </tr>

  </table>
<?php
}

// Render the plugin's main admin form.
function okhub_render_form() {
  $okhub_assets = okhub_assets();
  $okhub_categories = okhub_categories();
  $okhub_datasets_names = okhub_datasets_names();
  if (empty($okhub_datasets_names)) {
    $okhub_datasets_names = okhub_datasets_names(TRUE); // force update
  }
  $okhub_datasets = okhub_datasets_with_content();
  $selected_datasets = okhub_selected_datasets();
  $okhub_datasets_names = okhub_datasets_names_with_content();
  $okhub_api_key_validated = okhubapi_variable_get('okhub', 'api_key_validated', FALSE);
  $okhub_num_items_options = array_combine(range(10, OKHUB_MAX_REQUESTED_ASSETS, 10), range(10, OKHUB_MAX_REQUESTED_ASSETS, 10));
	?>
  <!--div class="okhub-wrap"-->
	<div class="okhub-wrap">

		<!-- Display Plugin Icon, Header, and Description -->
		<div class="icon32" id="icon-options-general"></div>
		<h2><?php _e('OKHub Content - Settings'); ?></h2>
		<div class="updated">
      <?php printf(__('This plugin provides access to content published and/or curated by OKHub partner organisations via the Open Knowledge Hub API (<a href="%s" target="_new">%s</a>).'), OKHUB_API_HOME_URL, OKHUB_API_HOME_URL); ?>
      <br />
      <br />
      <i><?php _e('Please look through all the tabs on this page to set your configuration options. Changes will only be effective after submitting them by clicking on the "Save changes" button.</i>'); ?>
      <br />
      <i><?php printf(__('Nothing will be imported until periodic imports are set or a manual import is done.')); ?></i>
    </div>

    <!-- Beginning of the Plugin Options Form -->
		<form method="post" id="okhub_import_form" action="options.php" onSubmit="selectAllMappings()">
      <?php
          settings_fields('okhub');
      ?>

    <div class="okhub-ui-tabs">
      <ul class="okhub-ui-tabs-nav">
        <li><a href="#general"><?php _e('General settings'); ?></a></li>
        <li><a href="#import"><?php _e('Import settings'); ?></a></li>
        <li><a href="#general-filters"><?php _e('General filters'); ?></a></li>
        <li><a href="#categories-filters"><?php _e('Category filters'); ?></a></li>
        <li><a href="#display"><?php _e('Display settings'); ?></a></li>
        <li><a href="#categories-mappings"><?php _e('Category mappings'); ?></a></li>
        <li><a href="#periodic_import"><?php _e('Periodic imports'); ?></a></li>
      </ul>

      <!---------------------------------- GENERAL SETTINGS ------------------------------->

      <h3><?php _e('General settings'); ?></h3>

			<table class="okhub-ui-tabs-panel" id="general">
				<!-- OKHub API Key -->
				<tr>
					<th scope="row"><?php _e('OKHub API Token-GUID (key)'); ?></th>
					<td>
						<input type="text" size="57" name="okhub_options[api_key]" value="<?php echo okhubapi_variable_get('okhub', 'api_key', ''); ?>" /><br />
						<p class="description">
              <?php printf(__('If you do not have an OKHub API key, please request one at <a href="%s" target="_new">%s</a>'), OKHUB_API_KEY_URL, OKHUB_API_KEY_URL); ?>
            </p>
					</td>
				</tr>
      <?php if ($okhub_api_key_validated) { ?>
        <?php
          if (get_option('permalink_structure')) {
            $path_disabled = '';
            $default_path_documents = 'okhub_assets/documents';
            $default_path_organisations = 'okhub_assets/organisations';
          }
          else {
            $path_disabled = ' disabled';
            $default_path_documents = '';
            $default_path_organisations = '';
          }
      ?>
      </table>

      <!---------------------------- GENERAL IMPORT SETTINGS ------------------------------>

      <h3><?php _e('General import settings'); ?></h3>
      <table class="okhub-ui-tabs-panel" id="import">

        <!-- Datasets -->
        <tr>
          <th scope="row"><?php _e('Sources to import from'); ?></th>
          <td>
            <?php echo okhub_select_box('okhub_options[selected_datasets][]', 'okhub_selected_datasets', $okhub_datasets_names, $selected_datasets, array('multiple' => 'multiple', 'class' => 'chosen-select', 'data-placeholder' => 'Select sources...', 'style' => 'width: 350px', 'onchange' => 'changeDataset()')); ?>
            <a href="javascript:deselectAll('okhub_selected_datasets');"><?php _e('Deselect all'); ?></a> 
          </td>
        </tr>

        <!-- Number of assets to import. -->
        <tr>
          <th scope="row"><?php _e('Number of documents/organisations'); ?></th>
          <td>
            <?php echo okhub_select_box('okhub_options[num_items]', 'okhub_num_items', $okhub_num_items_options, array(okhubapi_variable_get('okhub', 'num_items', OKHUB_API_NUM_ITEMS))); ?>
            <!--input type="text" size="10" name="okhub_options[num_items]" value="<?php echo okhubapi_variable_get('okhub', 'num_items', OKHUB_API_NUM_ITEMS); ?>" /-->
            <p class="description">
            <?php _e('Number of documents and/or organisations to retrieve <u>from each selected dataset</u> in each call to the OKHub API.'); ?>
            </p>
          </td>
        </tr>

        <!-- Updated assets -->
        <tr>
          <th scope="row"><?php _e('Updated resources'); ?></th>
          <td>
            <!--?php echo okhub_select_box('okhub_options[updated_assets]', 'okhub_updated_assets', array('skip' => __('Always skip'), 'overwrite' => __('Always overwrite'), 'keep' => __('Keep both (pending of review)')), array(okhubapi_variable_get('okhub', 'updated_assets', OKHUB_UPDATED_ASSETS))); ?-->
            <?php echo okhub_select_box('okhub_options[updated_assets]', 'okhub_updated_assets', array('overwrite' => __('Overwrite'), 'skip' => __('Skip')), array(okhubapi_variable_get('okhub', 'updated_assets', OKHUB_UPDATED_ASSETS))); ?>
            <p class="description">
            <?php _e('Please indicate what to do with existing resources that have been updated in the source and have a more recent date_updated value than the one imported previously.'); ?>
            <br/>
            <!--?php _e('If "keep both" is selected, a new unpublished post will be created with the new values to be processed manually.'); ?-->
            <!--br/-->
            <?php _e('Please note that this will only be applied to documents and organisations. Categories will always be overwritten.'); ?>
            </p>
          </td>
        </tr>

        <!-- Publication date -->
        <tr>
          <th scope="row"><?php _e('Publication date'); ?></th>
          <td>
            <?php echo okhub_select_box('okhub_options[pub_date]', 'okhub_pub_date', array('default' => __('Import date'), 'date_created' => __('Date created (OKHub API)'), 'date_updated' => __('Date updated (OKHub API)')), array(okhubapi_variable_get('okhub', 'pub_date', 'date_created'))); ?>
            <p class="description">
            <?php _e('Date of publication of imported documents and/or organisations.'); ?>
            <br />
            <?php _e('If selected, the date_created or date_updated fields retrieved by the API will be used, instead of the date in which they were imported into Wordpress.'); ?>
            </p>
          </td>
        </tr>

        <!-- Username (author of posts) -->
        <?php
        // We only allow to create a new user once. Then, it has to be selected from existing users.
        if ($user = get_user_by('email', OKHUB_USER_EMAIL)) {
          $new_user_allowed = FALSE;
          $hide_field = 'style="display: none"';
        }
        else {
          $new_user_allowed = TRUE;
          $hide_field = '';
        }
        ?>

        <?php $okhub_new_categories = okhubapi_variable_get('okhub', 'import_new_categories', OKHUB_NEW_CATEGORIES); ?>
        <tr valign="top">
          <th scope="row"><?php _e('Import categories as taxonomy terms'); ?></th>
          <td>
            <label><input id="okhub_new_categories" name="okhub_options[import_new_categories]" type="checkbox" <?php echo ($okhub_new_categories ? 'checked="yes"' : '');  ?> onchange="changeNewCategories();" /> Import OKHub categories </label>
            <p class="description">
            <?php _e('If checked, new Wordpress taxonomies will be created for OKHub categories (countries, themes, regions, etc.).'); ?><br>
            </p>
          </td>
        </tr>

        <!-- Category levels to import -->
        <tr valign="top" class="okhub-categories-new">
          <th scope="row"><?php _e('Import only top-level categories'); ?></th>
          <td>
          <label><?php echo okhub_select_box('okhub_options[top_level_categories]', 'okhub_top_level_categories', array('true' => __('Only top-level'), '' => __('All levels')), array(okhubapi_variable_get('okhub', 'top_level_categories', OKHUB_DEFAULT_TOP_LEVEL))); ?></label>
          <p class="description">
          <?php _e('The level shows the position in the thematic hierarchy, starting at 1, the top level. Select the category levels to import.'); ?>
          </p>
          </td>
        </tr>

        <!-- Import tags (OKHub keywords) -->
        <tr valign="top">
          <th scope="row"><?php _e('Import OKHub keywords'); ?></th>
          <td>
            <?php $import_tags = okhubapi_variable_get('okhub', 'import_tags', OKHUB_TAGS); ?>
            <label><input name="okhub_options[import_tags]" type="checkbox" <?php  echo ($import_tags ? 'checked="yes"' : ''); ?> /> <?php _e('Include keywords as tags'); ?></label>
            <p class="description"><?php _e('Please indicate if you would like to include OKHub keywords as tags of the imported documents and/or organisations.'); ?></p>
          </td>
        </tr>

        <tr>
          <th scope="row"><?php _e('Username'); ?></th>
          <td>
            <input id="okhub_import_user" type="text" size="20" name="okhub_options[import_user]" value="<?php echo okhubapi_variable_get('okhub', 'import_user', OKHUB_IMPORT_USER); ?>" <?php echo $hide_field; ?>/>
            <?php if ($new_user_allowed) { ?>
              <p class="description">
              <?php _e('Enter a new username or select an existing one: '); ?>
            <?php } ?>
            <?php wp_dropdown_users(array('id' => 'okhub_import_user_select', 'name' => 'okhub_options[import_user_id]', 'selected' => okhubapi_variable_get('okhub', 'import_user_id', -1), 'show_option_none' => '-- Username --'));
            ?>
            <br/>
            <p class="description">
            <?php _e('Imported OKHub content will be assigned to this user.'); ?>
            </p>
            </p>
          </td>
        </tr>

      </table>

      <!---------------------------------------- GENERAL FILTERS ---------------------------------->

      <h3><?php _e('General filters'); ?></h3>
      <table class="okhub-ui-tabs-panel" id="general-filters">
        <tr>
          <td colspan="2">
            <p class="description"><?php _e('Documents and/or organisations to be imported can be limited to those matching the following criteria.'); ?></p>
          </td>
        </tr>

        <!-- Age of assets to import -->
        <!-- NOTE: This is currently not supported for all the sources. TODO: Enable/disable this option depending on the selected sources. -->
        <tr>
         <th scope="row"><?php _e('Age of documents and/or organisations'); ?></th>
          <td>
            <?php echo okhub_select_box('okhub_options[age_new_assets]', 'okhub_age_new_assets', array('0' => __('Do not filter by age'), '7' => __('One week'), '30' => __('One month'), '180' => __('Six months'), '365' => __('One year')), array(okhubapi_variable_get('okhub', 'age_new_assets', OKHUB_API_AGE_NEW_ASSETS))); ?>
            <p class="description"><?php _e('Only documents/organisations published after the indicated time will be imported.'); ?></p>
          </td>
        </tr>

        <!-- Resources' languages -->
        <tr>
          <th scope="row"><?php _e('Resources\' languages'); ?></th>
          <td>
            <?php echo okhub_select_box('okhub_options[language_filters_codes][]', 'okhub_language_filters_codes', okhub_languages(), okhubapi_variable_get('okhub', 'language_filters_codes', array()), array('multiple' => 'multiple', 'data-placeholder' => 'Select languages...', 'style' => 'width: 350px', 'class' => 'chosen-select')); ?>
            <a href="javascript:deselectAll('okhub_language_filters_codes');"><?php _e('Deselect all'); ?></a>
            <p class="description">
            <?php _e('Only retrieve resources that have relevant content (at least title, description) available in the selected languages for at least one of the selected sources. If none is selected, resources with content in all available languages will be imported.'); ?>
            </p>
          </td>
        </tr>

        <!-- Exclude URIs -->
        <tr>
          <th scope="row"><?php _e('Exclude URIs'); ?></th>
          <td>
            <input type="text" size="57" name="okhub_options[exclude_uris]" value="<?php echo okhubapi_variable_get('okhub', 'exclude_uris', get_bloginfo('url')); ?>" /><br />
            <p class="description">
            <?php _e('Exclude resources that match certain URIs. Partial matching will be applied. Use commas to indicate several URIs.'); ?><br />
            <?php _e('This filter can be used, for instance, to avoid importing resources that are originated from this site.'); ?><br />
            <?php _e('Example: <i>http://www.worldbank.org/en/news, undp.org</i> would exclude WB\'s press releases and all resources from servers with undp.org as domain name.'); ?></p>
          </td>
        </tr>

        <!-- Authors -->
        <tr>
          <th scope="row"><?php _e('Documents\' author(s)'); ?></th>
          <td>
            <input type="text" size="57" name="okhub_options[import_authors_assets]" value="<?php echo okhubapi_variable_get('okhub', 'import_authors_assets', ''); ?>" /><br />
            <p class="description">
            <?php _e('A list of comma-separated authors\' names.'); ?><br />
            <?php _e('Example: <i>Chomsky, Foucault</i> will retrieve documents with either <i>Chomsky</i> or <i>Foucault</i> in the "author" field.'); ?></p>
          </td>
        </tr>

        <!-- Publishers -->
        <tr>
          <th scope="row"><?php _e('Documents\' publisher(s)'); ?></th>
          <td>
            <input type="text" size="57" name="okhub_options[import_publishers_assets]" value="<?php echo okhubapi_variable_get('okhub', 'import_publishers_assets', ''); ?>" /><br />
            <p class="description">
            <?php _e('A list of comma-separated publishers\' names.'); ?><br />
            <?php _e('Example: <i>World Bank, UNDP</i> will retrieve documents with either <i>World Bank</i> or <i>UNDP</i> in the "publisher" field.'); ?></p>
          </td>
        </tr>

        <tr>
          <th scope="row"><?php _e('Additional search terms'); ?></th>
          <td>
            <input type="text" size="57" name="okhub_options[import_query]" value="<?php echo okhubapi_variable_get('okhub', 'import_query', ''); ?>" /><br />
            <p class="description">
            <?php printf(__('Please refer to <a href="%s" target="_new">the API documentation</a>.'), OKHUB_API_SEARCH_DOCUMENTATION); ?>
           </p>
          </td>
        </tr>
      </table>

      <!---------------------------------------- CATEGORY FILTERS ---------------------------------->

      <h3><?php _e('Filter by sources\' categories'); ?></h3>
      <table class="okhub-ui-tabs-panel" id="categories-filters">
        <tr class="okhub-categories-help">
          <td colspan="2">
            <p class="description">
              <?php _e('Documents and/or organisations to be imported can be limited to those associated to the following categories.'); ?></br>
              <?php _e('Leave the option blank if you do not want to filter the imported content by a particular category.'); ?></br></br>
              <?php _e('Some options in the plugin\'s settings, including the sources\' categories, are cached for performance.'); ?>
              <?php printf(__('Please go to <a href="%s">the plugin\'s control panel</a> in order to clear the cache contents'), 'admin.php?page=okhub_menu' ); ?>
            </p>      
          </td>
        </tr>

        <?php 
        foreach ($okhub_datasets as $dataset) { ?>
          <tr class="okhub-categories-<?php echo $dataset; ?>">
            <td colspan="2">
              <p id="title_categories" class="okhub-settings-separator okhub-categories-<?php echo $dataset; ?>"><?php echo okhub_get_dataset_acronym($dataset); ?> categories</p>
            </td>
          </tr>
          <?php
          foreach ($okhub_categories as $category) {
            if (okhub_has_content($dataset, $category)) {
              $field_id = $dataset.'_'.$category.'_assets';
              $option_name = $dataset.'_'.$category.'_assets';
              $category_name = okhub_get_type_name($category, TRUE);
              ?>
              <tr class="okhub-categories-<?php echo $dataset; ?>">
              <th scope="row"><?php _e($category_name); ?></th>
                <td>
                  <select data-placeholder="Select..." class="chosen-select" style="width:350px;" tabindex="4" id='<?php echo $field_id; ?>' name='okhub_options[<?php echo $option_name; ?>][]' multiple></select>
                  <input class="button-primary okhub-load-<?php echo $dataset; ?>-<?php echo $category; ?>" type="button" value="Load" onClick="loadTaxonomies('<?php echo $dataset; ?>','<?php  echo $category; ?>', 0)">
                  <input class="button-primary okhub-refresh-buttons okhub-refresh-<?php echo $dataset; ?>-<?php echo $category; ?>" type="button" value="Refresh" onClick="loadTaxonomies('<?php echo $dataset; ?>','<?php  echo $category; ?>', 1)">
                  <p class="description"><?php _e("Import only documents/organisations with the selected focus $category."); ?>
                  <!--a href="javascript:selectAll('<?php echo $field_id; ?>');"><?php _e('Select all'); ?></a> /--> 
                  <a href="javascript:deselectAll('<?php echo $field_id; ?>');"><?php _e('Deselect all'); ?></a>
                  </p>
                </td>
              </tr>
            <?php
            } // if has content.
          }// countries, regions, themes ?>
        <?php
        } // foreach dataset ?>
      </table>

      <!---------------------------- DISPLAY SETTINGS ------------------------------>

      <h3><?php _e('General import settings'); ?></h3>
      <table class="okhub-ui-tabs-panel" id="display">

        <!-- Languages -->
        <tr>
          <th scope="row"><?php _e('Preferred language'); ?></th>
          <td>
            <?php echo okhub_select_box('okhub_options[default_language]', 'okhub_default_language', okhub_languages(), array(okhubapi_variable_get('okhub', 'default_language', OKHUB_DEFAULT_LANGUAGE))); ?>
            <p class="description">
            <?php _e('Translatable metadata (title, description) will be set in this language, if available. Please also see the filters settings for additional language options.'); ?>
            </p>
          </td>
        </tr>

        <!-- Additional languages -->
        <tr>
          <th scope="row"><?php _e('Additional languages'); ?></th>
          <td>
            <?php echo okhub_select_box('okhub_options[additional_languages][]', 'okhub_additional_languages', okhub_languages(), okhubapi_variable_get('okhub', 'additional_languages', array()), array('multiple' => 'multiple', 'data-placeholder' => 'Select languages...', 'style' => 'width: 350px', 'class' => 'chosen-select')); ?>
            <a href="javascript:deselectAll('okhub_additional_languages');"><?php _e('Deselect all'); ?></a>
            <p class="description">
            <?php _e('If available, metadata in the selected languages will be retrieved, in addition to the one in the default language.'); ?>
            <br>
            <?php _e('It can later be decided if / how to display the content in these other languages.'); ?>
            </p>
          </td>
        </tr>

        <!-- Excerpts length -->
        <tr>
          <th scope="row"><?php _e('Excerpts length'); ?></th>
          <td>
            <input type="text" size="10" name="okhub_options[excerpt_length]" value="<?php echo okhubapi_variable_get('okhub', 'excerpt_length', 0); ?>" />
            <p class="description">
            <?php _e('Maximum length of excerpts to display.'); ?>
            <br/>
            <?php _e('If not set (or if the value is 0) the full description will be used.'); ?>
            </p>
          </td>
        </tr>

        <!-- Include imported content in the loop -->
        <tr valign="top">
          <th scope="row"><?php _e('Display imported content with regular posts'); ?></th>
          <td>
            <?php $include_imported_documents = okhubapi_variable_get('okhub', 'include_imported_documents', OKHUB_INCLUDE_IMPORTED_DOCUMENTS); ?>
            <label><input name="okhub_options[include_imported_documents]" type="checkbox" <?php  echo ($include_imported_documents ? 'checked="yes"' : ''); ?>" /> <?php _e('Include imported documents'); ?></label>
            <br/>
            <?php $include_imported_organisations = okhubapi_variable_get('okhub', 'include_imported_organisations', OKHUB_INCLUDE_IMPORTED_ORGANISATIONS); ?>
            <label><input name="okhub_options[include_imported_organisations]" type="checkbox" <?php  echo ($include_imported_organisations ? 'checked="yes"' : ''); ?>" /> <?php _e('Include imported organisations'); ?></label>

            <p class="description">
            <?php _e('Please indicate if you would like to include the imported OKHub documents / organisations when displaying regular posts.'); ?><br>
            <?php _e('If these options are not checked, OKHub documents / organisations will not be displayed on your site by default, but they will still be accesible through specific URLs, available on:'); ?><br>
            <a href="<?php echo get_admin_url(); ?>/admin.php?page=okhub_menu"><?php echo get_admin_url(); ?>/admin.php?page=okhub_menu</a>
            </p>
          </td>
        </tr>

        <!-- Dataset in public areas -->
        <tr valign="top">
          <th scope="row"><?php _e('Status of imported content'); ?></th>
          <td>
            <label><input name="okhub_options[display_dataset_public]" type="radio" value="none" <?php checked('pending', okhubapi_variable_get('okhub', 'post_status', OKHUB_DEFAULT_POST_STATUS)); ?> /> Pending review</label><br />
            <label><input name="okhub_options[display_dataset_public]" type="radio" value="all" <?php checked('publish', okhubapi_variable_get('okhub', 'post_status', OKHUB_DEFAULT_POST_STATUS)); ?> /> Published</label><br />
            <p class="description">
            <?php _e('Please note that this applies to new content. The status of content that has already been imported can be changed in the plugin\'s administrative area.'); ?>
            </p>
          </td>
        </tr>
      </table>

      <!------------------------------- CATEGORIES MAPPINGS ------------------------>

      <h3><?php _e('Categories mappings'); ?></h3>

      <table class="okhub-ui-tabs-panel" id="categories-mappings">
        <tr class="okhub-mappings-help">
          <td colspan="4">
            <p class="description">
            <?php _e('Indicate if OKHub categories associated to documents/organisations should be mapped to existing WordPress categories.'); ?><br>
              <?php _e('Please note that this option is independent of the category importing settings: an OKHub category can be imported as a new term and, in addition, mapped to an existing category. It can also be mapped to an existing taxonomy without being imported as a new Wordpress taxonomy term.'); ?></p>
            </p>

        <?php
          /* --------------- Wordpress taxonomies ---------------- */
          $wp_terms_select = array();
          $all_taxonomies = get_taxonomies(array('public' => TRUE, 'show_ui' => TRUE));
          $select_taxonomies = array('' => 'Map to...');
          $reg_expr_datasets = '/(' . implode('|', $okhub_datasets) . ')/';
          foreach($all_taxonomies as $tax_name) {
            $all_wp_terms = get_terms($tax_name, array('hide_empty' => FALSE));
            if ($all_wp_terms && !is_wp_error($all_wp_terms)) {
              if (!preg_match($reg_expr_datasets, $tax_name)) {
                trim($tax_name);
                $labels = get_taxonomy_labels(get_taxonomy($tax_name));
                $taxonomy_name = ($labels->name) ? $labels->name : $tax_name;
                $target_taxonomies[$tax_name] = $taxonomy_name;
                $select_taxonomies[$tax_name] = $taxonomy_name;
              }
              foreach ($all_wp_terms as $wp_term) {
                $wp_terms_select[$tax_name][$tax_name.'-'.$wp_term->term_id] = $wp_term->name;
              }
            }
          }
        ?>

        <table>
        <?php 
        if (!empty($wp_terms_select)) { ?>
          <!-- OKHub Categories mappings (countries, regions, themes) -->
          <?php 
          foreach ($okhub_datasets as $dataset) { ?>
            <tr valign="top" class="okhub-categories-mapping okhub-mappings-<?php echo $dataset; ?>">
              <td colspan="4">
                <p id="title_categories" class="okhub-settings-separator"><?php echo okhub_get_dataset_acronym($dataset) . ' mappings'; ?></p>
              </td>
            </tr>
            <?php 
              foreach ($okhub_categories as $category) {
                if (okhub_has_content($dataset, $category)) {
                  $field_id_source = $dataset.'_'.$category.'_source';
                  $field_id_mappings = $dataset.'_'.$category.'_mappings';
                  $option_name_mappings = $dataset.'_'.$category.'_mappings';
                  $category_name = okhub_get_type_name($category, TRUE);
                  ?>
                  <tr valign="top" class="okhub-categories-mapping okhub-mappings-section okhub-mappings-<?php echo $dataset; ?>">

                    <!--------- Source / type ---------->
                    <td class="okhub-categories-container" style="vertical-align: top;">
                      <p style="line-height: 0.5;">
                      Map from <?php echo okhub_get_dataset_acronym($dataset) . ' ' . $category_name; ?>
                      </p>
                      <select multiple data-placeholder="Select <?php echo $category; ?>..." class="chosen-select okhub-mapping-select" id='<?php echo $field_id_source; ?>'></select>
                      <p class="description">
                        <a href="javascript:deselectAll('<?php echo $field_id_source; ?>');"><?php _e('Deselect all'); ?></a>
                      </p>
                    </td>
  
                    <!--------- Targets for source/type ---------->
                    <td class="okhub-categories-container" style="vertical-align: top;">
                    <?php $field_id_select_target = $dataset.'_'.$category.'_select_target'; ?>
                    <?php echo okhub_select_box($field_id_select_target, $field_id_select_target, $select_taxonomies, array(), array('onChange'=>'changeMappingsTargets(\''.$dataset.'\', \''.$category.'\');')); ?>
                      <?php               
                    $div_target_class_name = "okhub-mapping-target-$dataset-$category";
                    foreach ($target_taxonomies as $tax_name => $taxonomy_name) {
                      $field_id_target = $dataset . '_' . $category . '_' . $tax_name . '_target';
                      $field_id_div_target = $dataset.'_'.$category.'_'.$tax_name.'_div_target';
                      ?>                    
                      <div id="<?php echo $field_id_div_target;?>" class="okhub-mapping-target <?php echo $div_target_class_name;?>">
                        <?php echo okhub_select_box($field_id_target, $field_id_target, $wp_terms_select[$tax_name], array(), array('class' => "chosen-select okhub-mapping-select", 'data-placeholder' => "Select $taxonomy_name terms to map to...")); ?>
                        <br />
                        <p class="description"><a href="javascript:addMappings('<?php echo $field_id_source; ?>', '<?php echo $field_id_target; ?>', '<?php echo $field_id_mappings; ?>');"><?php _e('Add mappings'); ?> &rarr;</a></p>
                      </div>
                    <?php 
                    } // foreach WP taxonomy ?>
                    </td>

                    <!--------- Current mappings for source/type ---------->
                    <td class="okhub-categories-container" style="vertical-align: top;">
                      <select class="okhub-current-mappings okhub-mapping-select" name='okhub_options[<?php echo $option_name_mappings; ?>][]' id='<?php echo $field_id_mappings; ?>' multiple='multiple' size='3'></select>
                      <p class="description"> <a href="javascript:removeMappings('<?php echo $field_id_mappings; ?>');">&larr; <?php _e('Remove selected'); ?></a> / <a href="javascript:selectAll('<?php echo $field_id_mappings; ?>');"><?php _e('Select all'); ?></a> / <a href="javascript:deselectAll('<?php echo $field_id_mappings; ?>');"><?php _e('Deselect all'); ?></a></p>
                    </td>
  
                  </tr>
                <?php
                } // if has content
              } // foreach OKHub category ?> 
          <?php 
          } // foreach dataset ?>
        <?php 
        } // there are no wordpress categories to map to
        else { ?>
            <tr valign="top" class="okhub-categories-mapping okhub-mappings-<?php echo $dataset; ?>">
              <td colspan="4">
                <p id="title_categories" class="okhub-settings-separator"><?php _e('Please, create the Wordpress categories to map to first.'); ?></p>
              </td>
            </tr>          
        <?php 
        } ?>
        </table>

          </td>
        </tr>
      </table>

      <!------------------------------------ PERIODIC IMPORTS ----------------------------->

      <h3><?php _e('Periodic import'); ?></h3>
      <table class="okhub-ui-tabs-panel" id="periodic_import">
        <!-- Types of content to import -->
        <tr valign="top">
          <th scope="row"><?php _e('Select the types of content to import periodically'); ?></th>
          <td>
            <label><strong><?php _e('Assets'); ?></strong></label><br/>
            <?php foreach (okhub_assets() as $asset) { ?>
              <label><input id="okhub_import_<?php echo $asset; ?>_checkbox" name="okhub_options[import_<?php echo $asset; ?>]" type="checkbox" <?php echo (okhubapi_variable_get('okhub', 'import_' . $asset, '0') ? 'checked="yes"' : ''); ?> onchange="changeImportSettings();" /> <?php _e(okhub_get_type_name($asset, TRUE)); ?></label> &nbsp;&nbsp;&nbsp;
            <?php } ?>
            <p class="description"><?php _e('Assets (documents and organisations) are updated frequently.'); ?></p>
          </td>
        </tr>
        <tr class = "okhub-categories-new">
          <th scope="row"></th>
          <td>
            <label><strong><?php _e('Categories'); ?></strong></label><br/>
            <?php foreach (okhub_categories() as $category) { ?>
              <label><input id="okhub_import_<?php echo $category; ?>_checkbox" name="okhub_options[import_<?php echo $category; ?>]" type="checkbox" <?php echo (okhubapi_variable_get('okhub', 'import_' . $category, '0') ? 'checked="yes"' : ''); ?> onchange="changeImportSettings();" /> <?php _e(okhub_get_type_name($category, TRUE)); ?></label> &nbsp;&nbsp;&nbsp;
            <?php } ?>
            <p class="description"><?php _e('Categories are updated less frequently and it could be a good alternative to import them manually.'); ?></p>
          </td>
        </tr>

        <!-- Frequency to import -->
        <?php $okhub_schedules = okhub_get_schedules(); ?>
        <tr class="okhub-assets-import-field">
          <th scope="row"><?php _e('Frequency to import/update documents/organisations'); ?></th>
          <td>
            <?php echo okhub_select_box('okhub_options[import_period_assets]', 'okhub_import_period_assets', $okhub_schedules, array(okhubapi_variable_get('okhub', 'import_period_assets', OKHUB_IMPORT_RECURRENCE_ASSETS))); ?>
            <p class="description"><?php _e('Please indicate the frequency with which you would like to import or update documents and/or organisations.'); ?></p>
          </td>
        </tr>

        <tr class="okhub-categories-import-field okhub-categories-new">
          <th scope="row"><?php _e('Frequency to import/update categories'); ?></th>
          <td>
            <?php echo okhub_select_box('okhub_options[import_period_categories]', 'okhub_import_period_categories', $okhub_schedules, array(okhubapi_variable_get('okhub', 'import_period_categories', OKHUB_IMPORT_RECURRENCE_CATEGORIES))); ?>
            <p class="description"><?php _e('Please indicate the frequency with which you would like to import or update categories.'); ?></p>
          </td>
        </tr>
      </table>

    </div>	<!-- UI tabs -->	

      <p class="okhub-button submit">
        <input name="okhub_options[submit_save]" type="submit" class="button-primary" value="<?php _e('Save changes') ?>" onClick="hideImportMessage()"/>
      </p>

    <?php } else { // If API key is not validated ?>
      </table>
      <p class="okhub-button submit okhub-import-settings-message">
        <?php _e('Please validate your OKHub API key in order to continue setting the plugin options.'); ?>
        <input name="okhub_options[submit_validate]" type="submit" class="button-primary" value="<?php _e('Validate') ?>" />
      </p>
    <?php } ?>
		</form>
  </div>	<!-- okhub-wrap -->	
  <!--/div--><!-- wrap -->
	<?php	
}

// Add columns to the posts edit page
function okhub_posts_columns($defaults) {
  $okhub_categories = okhub_categories();
  $okhub_new_categories = okhubapi_variable_get('okhub', 'import_new_categories', OKHUB_NEW_CATEGORIES);
  $datasets = okhub_datasets();
  $post_type = get_query_var('post_type');
  $dataset = get_query_var('okhub_source');
  if (!$post_type) {
    $post_type = 'post';
  }
  //$datasets = okhub_selected_datasets();
  $supported_taxonomies = get_object_taxonomies($post_type);
  //foreach ($datasets as $dataset) {
    foreach ($okhub_categories as $category) {
      $taxonomy_name = okhub_custom_taxonomy_name($dataset, $category);
      if (taxonomy_exists($taxonomy_name) && $okhub_new_categories && in_array($taxonomy_name, $supported_taxonomies)) {
        $category_name = okhub_get_dataset_acronym($dataset) . ' ' . okhub_get_type_name($category, TRUE);
        $defaults[$taxonomy_name] = $category_name;
      }
    }
  //}
  return $defaults;
}

// Add columns contents in the posts edit page.
function okhub_populate_posts_columns($column){
  global $post;
  $is_okhub_category = preg_match('/'.OKHUB_CUSTOM_PREFIX.'/', $column);
  if ($is_okhub_category) {
	  //$terms = get_the_term_list($post->ID, $column, '', ', ', '');
    $terms = get_the_terms($post->ID, $column);
    if ($terms) {
      $terms_output = array();
      foreach ($terms as $term) {
        $term_link = '/?'.$column.'='.$term->slug;
        $post_type = get_query_var('post_type');
        if ($post_type) {
          $term_link .= '&post_type=' . $post_type;
        }
        $terms_output[] = '<a href="'.site_url($term_link).'">'.$term->name.'</a>';
      }
      $output = implode(', ', $terms_output);
    }
    else {
      $output = '&#8212;'; // long hyphen.
    }
    echo $output;
  }
}

// Removes the OKHub category widgets if the categories are not enabled in settings.
function okhub_edit_post_form() {
  $okhub_datasets = okhub_datasets();
  $okhub_categories = okhub_categories();
  $okhub_new_categories = okhubapi_variable_get('okhub', 'import_new_categories', OKHUB_NEW_CATEGORIES);
  if (!$okhub_new_categories) {
    $types = get_post_types(array('_builtin' => false));
    $types[] = 'post';
    foreach ($okhub_datasets as $dataset) {
      foreach ($okhub_categories as $category) {
        foreach ($types as $type) {
          $supported_taxonomies = get_object_taxonomies($type);
          $custom_taxonomy_name = okhub_custom_taxonomy_name($dataset, $category);
          if (in_array($custom_taxonomy_name, $supported_taxonomies)) {
            remove_meta_box($custom_taxonomy_name.'div', $type, 'side');
          }
        }
      }
    }
  }
}

function okhub_okhub_categories_page($dataset, $category) {
  $taxonomy_name = okhub_custom_taxonomy_name($dataset, $category);
  $taxonomy_singular_label = okhub_get_dataset_acronym($dataset) . ' ' . okhub_get_type_name($category);
  okhub_taxonomy_page($taxonomy_name, $taxonomy_singular_label);
}

function okhub_taxonomy_page($taxonomy_name, $taxonomy_singular_label) {
?>
  <div class="okhub-wrap">
  <div id="icon-edit-pages" class="icon32 icon32-posts-page"><br /></div>
  <h2><?php echo $taxonomy_singular_label; ?> <a href="edit-tags.php?taxonomy=<?php echo $taxonomy_name; ?>" class="add-new-h2">Add/Edit/Delete <?php echo $taxonomy_singular_label; ?></a> </h2>
  <br/><br/>
  <ul class="okhub-category-list">
  <?php
  $args = array(
    'hide_empty'         => FALSE,
    'orderby'            => 'name',
    'order'              => 'ASC',
    'hierarchical'       => true,
    'title_li'           => '',
    'show_option_none'   => __("No '$taxonomy_singular_label' found"),
    'taxonomy'           => $taxonomy_name,
  );
  wp_list_categories($args);
  ?>
  </ul>
  </div>
<?php
}

// Displays the help page.
function okhub_help_page() {
?>
  <div class="okhub-wrap">
  <div id="icon-edit-comments" class="icon32 icon32-posts-page"><br /></div>
  <h2><?php _e('OKHub Content - Help'); ?></h2>
  <table class="okhub-table">
  <tr>
  <td>
  <p>
  <?php _e('This plugin provides access to Open Knowledge Hub content of thematically organised and hand selected academic research on poverty reduction in the developing world that is freely available to access online.'); ?>
  </p>
  </td>
  </tr>

  <tr>
  <td>
  <b><?php _e('What content can I get?'); ?></b>
  <p>
  <b><?php _e('Documents'); ?></b> <?php _e('are a wide range of online resources, primarily academic research, on development issues that are freely available online.'); ?>
  <br>
  <?php _e('They are editorially selected, organised thematically and are summarised, in clear, non-technical language for easy consumption.'); ?>
  </p>
  <p>
  <b><?php _e('Organisations'); ?></b> <?php _e('are a wide range of organisations engaged in reducing poverty. Most of the organisations in our datasets have published documents available in the data.'); ?>
  <br>
  <?php _e('Please note: currently there are no organisations in the BRIDGE dataset. Organisational websites are recorded as "documents".'); ?>
  </p>  
  <p>
  <b><?php _e('Countries'); ?></b> <?php _e('are used to identify either:'); ?>
  <ul class="okhub-category-list">
  <li class="cat-item"> <?php _e('which geographic area the research document covers'); ?>
  <li class="cat-item"> <?php _e('where the research was produced, based on the publisher country'); ?>
  <li class="cat-item"> <?php _e('the countries in which an organisation works'); ?>
  </ul>
  </p>
  <p>
  <b><?php _e('Regions'); ?></b> <?php _e('are broad geographic groupings, which enable users to explore our documents and organisations by more than one country.'); ?>
  </p>
  <p>
  <b><?php _e('Themes'); ?></b> <?php _e('are development topics which reflect the key themes in development.'); ?>
  </p>
  </td>
  </tr>

  <tr>
  <td>
  <b><?php _e('How do I use the plugin?'); ?></b>
  <p>
  <?php _e('To use the plugin, you must obtain a unique Token-GUID or key for the API. Please register for your API key <a href="http://developer.okhub.org/hub-api-documentation/authentication/" target="_new">here</a>. Once obtained, enter this key into the OKHub API Token-GUID (key) section of the OKHub Plugin Settings.'); ?>
  </p>
  <b><?php _e('OKHub Content plugin'); ?></b>
  <p>
  <?php _e('This plugin enables the one time or periodic importing of content into the Wordpress database. OKHub documents and organisations are imported as custom Wordpress content types, while OKHub regions, countries and themes are imported as Wordpress custom taxonomies. Content and terms can also be imported manually.'); ?>
  </p>
  <p>
  <?php _e('You can select the number of items to import and assign to a specific Wordpress user. You can filter the content you import by country, region or theme. It is also possible the filter by publisher or author name.  You can integrate content seamlessly into your existing content structures by mapping OKHub taxonomies to existing Wordpress taxonomies.'); ?>
  </p>
  <p>
  <?php _e('So, if you were the administrator of a website that highlighted recent research on ICTs and Gender in Kenya, you would be able to select specific criteria to suit your content. You would select the themes ICT and Gender, as well as the country Kenya.'); ?>
  </p>
  </td>
  </tr>

  <tr>
  <td>
  <b><?php _e('Tips for searching'); ?></b>
  <p>
  <?php _e('Although the OKHub datasets are very wide ranging, very specific search terms may not return the number of results that you expect or want. Therefore, if you are not getting the results you expect, it might be worth trying more generic search terms or even simply searching at the Theme level.'); ?>
  </p>
  <p>
  <?php _e('Please note: You can add multiple selections for the Theme and Country fields. The search then looks for results that contain either Theme or Country.'); ?>
  </p>
  <p>
  <?php _e('e.g. A multiple selection of "India" and "Bangladesh" would show any results for:'); ?>
  </p>
  <p>
  <?php _e('either "India" OR "Bangladesh"'); ?>
  </p>
  <p>
  <?php _e('All other fields are combined in the search, so only results that contain ALL fields are returned.'); ?>
  </p>
  <p>
  <?php _e('e.g. A selection of "World Bank" as the publisher, "2010" as the year of publication, and a multiple selection of "India" and "Bangladesh" would show any results for:'); ?>
  </p>
  <p>
  <?php _e('World Bank AND 2010 AND (India OR Bangladesh)'); ?>
  </p>
  <p>
  <?php _e('For terms and condition for using API see'); ?> <a href="http://developer.okhub.org/hub-api-documentation/authentication/" target="_new"><?php _e('here'); ?></a>.
  </p>
  </td>
  </tr>
  </table>

  </div>
<?php
}

// Helper function to display the names of selected categories in the OKHub Content page from their object_okhub.
function okhub_get_names($selected_object_okhub, $names) {
  $ret = array();
  foreach ($selected_object_okhub as $object_id) {
    if (isset($names[$object_id])) {
      $ret[] = $names[$object_id];
    }
  }
  return $ret;
}

// Format the array of available schedules to be displayed on the admin page.
function okhub_get_schedules() {
  $available_schedules = wp_get_schedules();
  uasort($available_schedules, 'okhub_sched_sort');
  foreach ($available_schedules as $recurrence => $sched) {
    $okhub_schedules[$recurrence] = $sched['display'];
  }
  return $okhub_schedules;
}

// Sorts the schedules according to the time period
function okhub_sched_sort($sched1, $sched2) {
  if ($sched1['interval'] == $sched2['interval']) {
    return 0;
  }
  return ($sched1['interval'] < $sched2['interval']) ? -1 : 1;
}

