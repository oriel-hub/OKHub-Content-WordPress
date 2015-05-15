<?php

// Plugin's page from where to access all its services
function okhubimport_general_page() {
  global $okhub_categories;
  global $okhub_assets;
  $datasets = okhubimport_display_datasets('admin');
  $okhubapi = new OkhubApiWrapper;
  ?>
  <div class="wrap">
  <div id="icon-edit-pages" class="icon32"><br /></div>
  <div id="ids-logo"><a href="http://api.ids.ac.uk" target="_blank"><img src="http://serp-p.pids.gov.ph/home/images/okhub-logo200.png"></a></div>
  <h2><?php _e('OKHUB Import Plugin'); ?></h2>
  <br /><br />
  <ul class="ids-category-list">
    <li class="cat-item"> <p><a href="<?php echo get_admin_url() . 'options-general.php?page=okhubimport'; ?>"><?php _e('Settings'); ?></a></p>
    <li class="cat-item"> <p><a href="<?php echo get_admin_url() . 'admin.php?page=okhubimport_help'; ?>"><?php _e('Help'); ?></a></p>
  </ul>
  <table>
    <tr valign="top">
    <td class="okhubimport-admin-col1">

    <b><?php _e('Import content'); ?></b>
    <ul class="ids-category-list">
    <li class="cat-item"> <p><a href="<?php echo get_admin_url() . 'admin.php?import=okhubimport_importer'; ?>"><?php _e('OKHUB Importer'); ?></a></p>
    </ul>

    <br/>
    <br/>

    <b><?php _e('View the imported items on your website'); ?></b>
    <ul class="ids-category-list">
    <?php if (in_array('hub', $datasets)) { ?>
    <li class="cat-item"> <p><a href="<?php echo get_post_type_archive_link('okhub_documents'); ?>"><?php _e('View all imported documents'); ?></a></p>
    <!--<li class="cat-item"> <p><a href="<?php echo get_post_type_archive_link('okhub_organisations'); ?>"><?php _e('View all imported organisations'); ?></a></p>-->
    <?php } ?>

    <?php
    foreach ($datasets as $dataset) {
      foreach ($okhub_assets as $asset) {
        if (!okhubapi_exclude($dataset, $asset)) {
          $name = ucfirst($dataset) . ' ' . ucfirst($asset); 
          $link = 'okhub_' . $asset;
          if (okhubapi_variable_get('okhubimport', 'default_dataset', OKHUB_IMPORT_DEFAULT_DATASET_ADMIN) == 'all') { // TODO: Generalize.
            $par = ((preg_match('[\?]', $link)) ? '&' : '?');
            $link .= $par . 'okhub_site=' . $dataset;
          }
          ?>
          <!--<li class="cat-item"> <p><a href="<?php echo site_url($link); ?>"><?php _e('View'); ?> <?php echo $name; ?></a></p>-->
      <?php }
      }
    } ?>
      </ul>
    </td>

    <td class="okhubimport-admin-col2">
    <b><?php _e('Edit imported assets'); ?></b>
    <ul class="ids-category-list">
    <?php
    foreach ($datasets as $dataset) {
      foreach ($okhub_assets as $asset) {
        if (!okhubapi_exclude($dataset, $asset)) {
          $name = ucfirst($dataset) . ' ' . ucfirst($asset); ?>
          <li class="cat-item"> <p><a href="<?php echo get_admin_url() . 'edit.php?post_type=okhub_' . $asset; ?>&okhub_site=<?php echo $dataset; ?>"><?php _e('Edit'); ?> <?php echo $name; ?></a></p>
      <?php }
      }
    } ?>
    </ul>

    <br/>
    <br/>

    <?php
    $okhubimport_new_categories = okhubapi_variable_get('okhubimport', 'import_new_categories', OKHUB_IMPORT_NEW_CATEGORIES);
    if ($okhubimport_new_categories) { ?>
      <b><?php _e('Manage imported categories'); ?></b>
      <ul class="ids-category-list">
      <?php
      foreach ($datasets as $dataset) {
        foreach ($okhub_categories as $category) {
          $cat_admin_page = 'admin.php?page=' . $dataset . '_' . $category;
          $cat_name = ucfirst($dataset) . ' ' . ucfirst($category); ?>
          <li class="cat-item"> <p><a href="<?php echo get_admin_url() . $cat_admin_page; ?>"><?php _e('View / Edit'); ?> <?php echo $cat_name; ?></a></p>
      <?php } ?>
      <?php } ?>
    <?php } ?>
      </ul>
    </td>
    
    <td class="okhubimport-admin-col3">
    <b><?php _e('Administrative tasks'); ?></b>
    <br/>
    <br/>
    <i><?php _e('Cache management'); ?></i>
    <ul class="ids-category-list">
    <?php
    $task = '';
    if(isset($_GET['admin_task'])) {
      $task = $_GET['admin_task'];
    }
    if ($task == 'clear_cache') {
      $okhubapi->cacheFlush();
      echo '<li class="cat-item">' . '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;' . __('All cache entries have been deleted.');
    }
    else { ?>
      <li class="cat-item"> <a href="<?php echo get_admin_url() . 'admin.php?page=okhubimport_menu&admin_task=clear_cache'; ?>"><?php _e('Clear all cache data'); ?></a>
    <?php } ?>
    </ul>

    <?php
    $site = '';
    $category_type = '';
    $asset_type = '';
    ?>
    <br/>
    <i><?php _e('Bulk processing'); ?></i>
    <br/>
    <br/>

    <?php _e('Assets'); ?>
    <ul class="ids-category-list">
    <?php if ($task == 'unpublish_assets') {
      if (isset($_GET['site']) && isset($_GET['asset_type'])) {
        $site = $_GET['site'];
        $asset_type = $_GET['asset_type'];
        if (okhubimport_unpublish_assets('okhub_' . $asset_type, $site)) {
          echo '<li class="cat-item">' . '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp' . __('All ') . ucfirst($site) . ' ' . $asset_type . __(' have been marked as "pending".');
        }
      }
    }
    foreach ($datasets as $dataset) { 
      foreach ($okhub_assets as $asset) {
        if (!okhubapi_exclude($dataset, $asset)) {
          if (($task != 'unpublish_assets') || ($dataset != $site) || ($asset != $asset_type)) { ?>
            <li class="cat-item"> <a href="<?php echo get_admin_url() . 'admin.php?page=okhubimport_menu&admin_task=unpublish_assets&site=' . $dataset . '&asset_type=' . $asset; ?>"><?php echo __('Unpublish ') . ucfirst($dataset) . ' ' . $asset; ?></a>  <i>(<?php _e('Change the status to "pending".'); ?>)</i>
          <?php }
        } 
      }
    } ?>
    </ul>

    <?php _e('Categories'); ?>
    <ul class="ids-category-list">
    <?php if ($task == 'delete_terms') {
      if (isset($_GET['site']) && isset($_GET['category_type'])) {
        $site = $_GET['site'];
        $category_type = $_GET['category_type'];
        if (okhubimport_delete_taxonomy_terms($site . '_' . $category_type)) {
          echo '<li class="cat-item">' . '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp' . __('All ') . ucfirst($site) . ' ' . $category_type . __(' have been deleted.');
        }
      }
    }
    foreach ($datasets as $dataset) { 
      foreach ($okhub_categories as $category) {
        if (($task != 'delete_terms') || ($dataset != $site) || ($category != $category_type)) { ?>
          <li class="cat-item"> <a href="<?php echo get_admin_url() . 'admin.php?page=okhubimport_menu&admin_task=delete_terms&site=' . $dataset . '&category_type=' . $category; ?>"><?php echo __('Delete ') . ucfirst($dataset) . ' ' . $category; ?></a> <i>(<?php _e('This action cannot be undone.'); ?>)</i>
        <?php } ?>
      <?php }
    } ?>
    </ul>
    </td>
    </tr>
  </table>
  </div>
<?php
}

// Importer main page
function okhubimport_importer_form() {
  $okhubapi = new OkhubApiWrapper;
  global $okhub_datasets;
  global $okhub_categories;
  $okhubimport_new_categories = okhubapi_variable_get('okhubimport', 'import_new_categories', OKHUB_IMPORT_NEW_CATEGORIES);
  $okhubimport_map_categories = okhubapi_variable_get('okhubimport', 'import_map_categories', OKHUB_IMPORT_MAP_CATEGORIES);
  $default_dataset = okhubapi_variable_get('okhubimport', 'default_dataset', OKHUB_IMPORT_DEFAULT_DATASET_ADMIN);
  $api_key = okhubapi_variable_get('okhubimport', 'api_key', '');

  /* Display latest imported content from each dataset */
  $latest_import_dates = okhubapi_variable_get('okhubimport', 'import_dates', array('hub' => array(), 'bridge' => array()));
  if (!empty($latest_import_dates)) {
    $message = __('Latest imports: ') . '<br />';
    $latest_import_results = okhubapi_variable_get('okhubimport', 'import_results', array('hub' => array(), 'bridge' => array()));
    foreach ($latest_import_dates as $set => $results_dataset) {
      foreach ($results_dataset as $type => $date) {
        if (!okhubapi_exclude($set, $type)) {
          $new = (isset($latest_import_results[$set][$type]) ? $latest_import_results[$set][$type]['new'] : 0);
          $updated = (isset($latest_import_results[$set][$type]) ? $latest_import_results[$set][$type]['updated'] : 0);
          $message .= ucfirst($set) . ' ' . ucfirst(__($type)) . ' imported on ' . $date . ': ' . $new . ' new, ' . $updated . ' updated' . '<br />';
        }
      }
    }
    okhub_show_message($message);
  }
  $message = __('Please note that it can take some time to import a large volume of content. Do not stop your browser.');
  okhub_show_message($message);
  ?>
  <form method="post" action="admin.php?import=okhubimport_importer&step=1">
    <table class="form-table ids-importer-types">
      <!-- Types of content to import -->
      <tr valign="top">
        <th scope="row"><?php _e('Select the types of content to import'); ?></th>
        <td>
          <label><strong><?php _e('Assets'); ?></strong></label><br/>
          <label><input name="okhubimport_import_documents" type="checkbox" /> <?php _e('Documents'); ?></label> &nbsp;&nbsp;&nbsp;
          <?php //TODO. Generalize
          if ($default_dataset !== 'bridge')  { ?>
            <!--<label><input name="okhubimport_import_organisations" type="checkbox" /> <?php _e('Organisations'); ?></label>-->
          <?php } ?>
          <br/>

          <?php if ($okhubimport_new_categories) { ?>
          <br/>
          <label><strong><?php _e('Categories'); ?></strong></label><br/>
          <label><input name="okhubimport_import_countries" type="checkbox" /> <?php _e('Countries'); ?></label> &nbsp;&nbsp;&nbsp;
          <label><input name="okhubimport_import_regions" type="checkbox" /> <?php _e('Regions'); ?></label> &nbsp;&nbsp;&nbsp;
          <label><input name="okhubimport_import_themes" type="checkbox" /> <?php _e('Themes'); ?></label>
          <?php } ?>
        </td>
      </tr>
    </table>
    <?php wp_nonce_field('okhubimport_import', 'okhubimport_import_check_referrer'); ?>
    <p class="submit">
      <input name="submit_import" type="submit" class="button-primary" value="<?php _e('Import now') ?>" />
    </p>
  </form>

  <table class="form-table">
    <tr valign="top">
      <td colspan="2"> 
        <strong><?php _e('Active settings and filters'); ?></strong><br />
        (<?php printf(__('Please go to the <a href="%s">OKHUB API administrative page</a> in order to view the details or change these settings.'), 'options-general.php?page=okhubimport'); ?>)
      </td>
    </tr>
    <tr valign="top">
      <th scope="row"><?php _e('General settings'); ?></th>
      <td>
        <ul>
          <li> <?php echo '<i>' . __('Import from: ') . '</i>' . '<strong>' . (($default_dataset == 'all') ? __('All sources') : ucfirst($default_dataset)) . '</strong>'; ?>
          <li> <?php echo '<i>' . __('Number of documents/organisations: ') . '</i>' . '<strong>' . (($num_items = okhubapi_variable_get('okhubimport', 'num_items', OKHUB_API_NUM_ITEMS)) ? $num_items : 'Not specified') . '</strong>'; ?>
          <li> <?php echo '<i>' . __('Default language: ') . '</i>' . '<strong>' . okhubapi_variable_get('okhubimport', 'language', OKHUB_IMPORT_DEFAULT_LANGUAGE) . '</strong>'; ?>
          <li> <?php echo '<i>' . __('Additional languages: ') . '</i>' . '<strong>' . (($additional_languages = okhubapi_variable_get('okhubimport', 'additional_languages', array())) ? implode(', ', $additional_languages) : 'None') . '</strong>'; ?>
          <li> <?php echo '<i>' . __('Updated resources: ') . '</i>' . '<strong>' . okhubapi_variable_get('okhubimport', 'updated_assets', OKHUB_IMPORT_UPDATED_ASSETS) . '</strong>'; ?>
          <li> <?php echo '<i>' . __('Excerpts length: ') . '</i>' . '<strong>' . (($excerpt_length = okhubapi_variable_get('okhubimport', 'excerpt_length', 0)) ? $excerpt_length : 'Not specified') . '</strong>'; ?>
          <li> <?php echo '<i>' . __('Username assigned to imported content: ') . '</i>' . '<strong>' . okhubapi_variable_get('okhubimport', 'import_user', OKHUB_IMPORT_IMPORT_USER) . '</strong>'; ?>
          <li> <?php echo '<i>' . __('Levels of categories to import: ') . '</i>' . '<strong>' . (($okhubimport_level_categories = okhubapi_variable_get('okhubimport', 'level_categories', OKHUB_IMPORT_CATEGORY_LEVELS)) ? $okhubimport_level_categories : 'All levels') . '</strong>' ?>
        </ul>
      </td>
    </tr>
    <tr valign="top">
      <th scope="row"><?php _e('Categories'); ?></th>
      <td>
        <ul>
        <?php
        if ($okhubimport_new_categories) {
          echo '<li><i>' . __('OKHUB categories will be imported and assigned to documents and/or organisations.') . '</i>';
        }
        else {
          echo '<li><i>' . __('Importing of OKHUB categories <u>is disabled</u>.') . '</i>';
        }
        if ($okhubimport_map_categories) {
          echo '<li><i>' . __('OKHUB categories will be mapped to existing Wordpress categories.') . '</i>';
        }
        else {
          echo '<li><i>' . __('Mapping of OKHUB categories to Wordpress categories <u>is disabled</u>.') . '</i>';
        }
        if (!($okhubimport_new_categories || $okhubimport_map_categories)) {
          echo '<li><i>' . __('<u>Imported documents and organisations will be filed as "Uncategorized"</u>.') . '</i>';
        }
        ?>
        </ul>
      </td>
    </tr>

    <tr valign="top">
      <th scope="row"><?php _e('Active filters for documents and/or organisations'); ?></th>
      <td>
        <?php
        foreach ($okhub_datasets as $dataset) {
          if (($default_dataset == $dataset) || ($default_dataset == 'all')) {
            $array_countries = $okhubapi->search('countries', $dataset, $api_key)->getArrayTitles();
            $array_regions = $okhubapi->search('regions', $dataset, $api_key)->getArrayTitles();
            $array_themes = $okhubapi->search('themes', $dataset, $api_key)->getArrayTitles();
            echo '<b><u>' . ucfirst($dataset) . ' categories</u></b>';
        ?>
        <ul>
          <li> <?php echo '<i>' . __('Countries: ') . '</i>' . '<strong>' . (($okhubimport_import_countries_assets = implode(', ', okhubimport_get_names(okhubapi_variable_get('okhubimport', $dataset . '_countries_assets', array()), $array_countries))) ? $okhubimport_import_countries_assets : '<i>none selected</i>') . '</strong>' ?>
          <li> <?php echo '<i>' . __('Regions: ') . '</i>' . '<strong>' . (($okhubimport_import_regions_assets = implode(', ', okhubimport_get_names(okhubapi_variable_get('okhubimport', $dataset . '_regions_assets', array()), $array_regions))) ? $okhubimport_import_regions_assets : '<i>none selected</i>') . '</strong>' ?>
          <li> <?php echo '<i>' . __('Themes: ') . '</i>' . '<strong>' . (($okhubimport_import_themes_assets = implode(', ', okhubimport_get_names(okhubapi_variable_get('okhubimport', $dataset . '_themes_assets', array()), $array_themes))) ? $okhubimport_import_themes_assets : '<i>none selected</i>') . '</strong>' ?>
        </ul>
          <?php } ?>
        <?php } ?>
        <b><u><?php _e('Additional filters'); ?></u></b>
        <ul>
          <li> <?php echo '<i>' . __('Age of documents and/or organisations: ') . '</i>' . '<strong>' . (($okhubimport_age_new_assets = okhubapi_variable_get('okhubimport', 'age_new_assets', OKHUB_API_AGE_NEW_ASSETS)) ? $okhubimport_age_new_assets . ' days' : '<i>Do not filter by age</i>') . '</strong>' ?>
          <li> <?php echo '<i>' . __('Resources\' languages: ') . '</i>' . '<strong>' . (($okhubimport_language_name_codes = okhubapi_variable_get('okhubimport', 'language_name_codes', array())) ? implode(', ', $okhubimport_language_name_codes) : '<i>Do not filter by language</i>') . '</strong>' ?>
          <li> <?php echo '<i>' . __('Exclude URIs: ') . '</i>' . '<strong>' . (($okhubimport_exclude_uris = okhubapi_variable_get('okhubimport', 'exclude_uris', '')) ? $okhubimport_exclude_uris : '<i>Do not exclude resources by their URIs</i>') . '</strong>' ?>
          <li> <?php echo '<i>' . __('Documents\' author(s): ') . '</i>' . '<strong>' . (($okhubimport_import_authors_assets = okhubapi_variable_get('okhubimport', 'import_authors_assets', '')) ? $okhubimport_import_authors_assets : '<i>none selected</i>') . '</strong>' ?>
          <li> <?php echo '<i>' . __('Documents\' publisher(s): ') . '</i>' . '<strong>' . (($okhubimport_import_publishers_assets = okhubapi_variable_get('okhubimport', 'import_publishers_assets', '')) ? $okhubimport_import_publishers_assets : '<i>none selected</i>') . '</strong>' ?>
          <li> <?php echo '<i>' . __('Additional query terms: ') . '</i>' . '<strong>' . (($okhubimport_import_query = okhubapi_variable_get('okhubimport', 'import_query', '')) ? $okhubimport_import_query : '<i>none selected</i>') . '</strong>' ?>
        </ul>
      </td>
    </tr>
  </table>
<?php
}

// Render the plugin's main admin form.
function okhubimport_render_form() {
  global $okhub_assets;
  global $okhub_datasets;
  global $okhub_categories;
  $okhubimport_api_key_validated = okhubapi_variable_get('okhubimport', 'api_key_validated', FALSE);
	?>
  <div class="wrap">
	<div class="ids-wrap">
		<!-- Display Plugin Icon, Header, and Description -->
		<div class="icon32" id="icon-options-general"></div>
		<h2><?php _e('OKHUB Import Plugin Settings'); ?></h2>
		<p>
      <?php printf(__('This plugin provides access to the Open Knowledge Hub  datasets via the OKHUB Knowledge Services API (<a href="%s" target="_new">%s</a>).'), OKHUB_API_HOME_URL, OKHUB_API_HOME_URL); ?>
      <br />
      <br />
      <br />
      <i><?php _e('Please look through all the tabs on this page to set your configuration options. Changes will only be effective after submitting them by clicking on the "Save changes" button.</i>'); ?>
    </p>

    <!-- Beginning of the Plugin Options Form -->
		<form method="post" id="okhub_import_form" action="options.php" onSubmit="selectAllMappings()">
    <!-- TODO: use selectAllClass() instead of selectAllMappings() -->
      <?php
          settings_fields('okhubimport');
      ?>

    <div class="ui-tabs">
      <ul class="ui-tabs-nav">
        <li><a href="#general"><?php _e('General settings'); ?></a></li>
        <li><a href="#import"><?php _e('Import settings'); ?></a></li>
        <li><a href="#filters"><?php _e('Import filters'); ?></a></li>
        <li><a href="#display"><?php _e('Display settings'); ?></a></li>
        <li><a href="#categories"><?php _e('Category settings'); ?></a></li>
        <li><a href="#periodic_import"><?php _e('Periodic imports'); ?></a></li>
      </ul>

      <!---------------------------------- GENERAL SETTINGS ------------------------------->

      <h3><?php _e('General settings'); ?></h3>

			<table class="form-table ui-tabs-panel" id="general">
				<!-- OKHUB API Key -->
				<tr>
					<th scope="row"><?php _e('OKHUB API Token-GUID (key)'); ?></th>
					<td>
						<input type="text" size="57" name="okhubimport_options[api_key]" value="<?php echo okhubapi_variable_get('okhubimport', 'api_key', ''); ?>" /><br />
						<p class="description">
              <?php printf(__('If you do not have an OKHUB API key, please request one at <a href="%s" target="_new">%s</a>'), OKHUB_API_KEY_URL, OKHUB_API_KEY_URL); ?>
            </p>
					</td>
				</tr>
      <?php if ($okhubimport_api_key_validated) { ?>
        <?php
          if (get_option('permalink_structure')) {
            $path_disabled = '';
            $default_path_documents = 'okhub_assets/documents';
            //$default_path_organisations = 'okhub_assets/organisations';
          }
          else {
            $path_disabled = ' disabled';
            $default_path_documents = '';
            $default_path_organisations = '';
          }
        ?>
        <tr>
          <th scope="row"><?php _e('OKHUB Documents path'); ?></th>
          <td>
            <code><?php echo home_url(); ?>/</code>
            <input type="text" size="30" name="okhubimport_options[okhub_documents_path]" value="<?php echo okhubapi_variable_get('okhubimport', 'okhub_documents_path', $default_path_documents); ?>" <?php echo $path_disabled; ?>/>
            <input type="hidden" size="30" name="okhubimport_options[okhub_documents_old_path]" value="<?php echo okhubapi_variable_get('okhubimport', 'okhub_documents_old_path', ''); ?>" />
          </td>
        </tr>
        <!--<tr>
          <th scope="row"><?php _e('OKHUB Organisations path'); ?></th>
          <td>
            <code><?php echo home_url(); ?>/</code>
            <input type="text" size="30" name="okhubimport_options[okhub_organisations_path]" value="<?php echo okhubapi_variable_get('okhubimport', 'okhub_organisations_path', $default_path_organisations); ?>" <?php echo $path_disabled; ?>/>
            <input type="hidden" size="30" name="okhubimport_options[okhub_organisations_old_path]" value="<?php echo okhubapi_variable_get('okhubimport', 'okhub_organisations_old_path', ''); ?>" />
	 <p class="description">
              <?php _e('These paths, relative to the site URL, can be used to retrieve and display imported elements.'); ?>
            </p>
          </td>
        </tr>-->
        <tr>
          <th scope="row"></th>
          <td>
            <p class="description">
            <small>
              <?php if ($path_disabled) { ?> 
                <?php printf(__('Paths rewriting is available only when custom <a href="%s">permalink structures</a> are active.'), 'options-permalink.php'); ?>
              <?php } else { ?>
                - <?php _e('If more than one source is selected in "Import settings", the source\'s slug (for instance: "hub" or "bridge") can appended to the paths in order to filter elements by their origin.'); ?>
                <?php printf(__('For instance, if the path is %s, the following URLs would be available: %s, which would display all OKHUB documents, and %s, which would only display Eldis\' documents.'), '<code>okhub_assets/documents</code>', '<code>'.home_url().'/okhub_assets/documents</code>', '<code>'.home_url().'/okhub_assets/documents/hub</code>'); ?><br>
                - <?php printf(__('If the paths are left empty, the default URLs: %s and %s can be used to retrieve OKHUB assets.'), '<code>'.home_url().'/okhub_documents/</code>', '<code>'.home_url().'/okhub_organisations/</code>'); ?><br>
              <?php } ?>
            </small>
            </p>
          </td>
        </tr>
      </table>

      <!---------------------------- GENERAL IMPORT SETTINGS ------------------------------>

      <h3><?php _e('General import settings'); ?></h3>
      <table class="form-table ui-tabs-panel" id="import">

        <!-- Dataset -->
        <tr valign="top">
          <th scope="row"><?php _e('Sources to import from'); ?></th>
          <td id="radio_dataset">
            <label><input name="okhubimport_options[default_dataset]" type="radio" value="hub" <?php checked('hub', okhubapi_variable_get('okhubimport', 'default_dataset', OKHUB_IMPORT_DEFAULT_DATASET_ADMIN)); ?> onchange="changeDataset();" /> OKHUB Dataset</label> <br />
           </td>
        </tr>

        <!-- Number of assets to import -->
        <tr>
          <th scope="row"><?php _e('Number of documents'); ?></th>
          <td>
            <input type="text" size="10" name="okhubimport_options[num_items]" value="<?php echo okhubapi_variable_get('okhubimport', 'num_items', OKHUB_API_NUM_ITEMS); ?>" />
            <p class="description">
            <?php _e('Number of documents and/or organisations to retrieve <u>from the hub dataset</u> in each call to the OKHUB API.'); ?>
            <br/>
            <?php _e('If not set (or if the value is 0) all available items will be imported.'); ?>
            </p>
          </td>
        </tr>

        <!-- Languages 
        <tr>
          <th scope="row"><?php _e('Default language'); ?></th>
          <td>
            <?php echo okhub_select_box('okhubimport_options[language]', 'okhubimport_language', okhub_languages(), array(okhubapi_variable_get('okhubimport', 'language', OKHUB_IMPORT_DEFAULT_LANGUAGE))); ?>
            <p class="description">
            <?php _e('Translatable metadata (title, description) will be set in this language, if available.'); ?>
            </p>
          </td>
        </tr>-->

        <!-- Additional languages 
        <tr>
          <th scope="row"><?php _e('Additional languages'); ?></th>
          <td>
            <?php echo okhub_select_box('okhubimport_options[additional_languages][]', 'okhubimport_additional_languages', okhub_languages(), okhubapi_variable_get('okhubimport', 'additional_languages', array()), array('multiple' => 'multiple')); ?>
            <a href="javascript:deselectAll('okhubimport_additional_languages');"><?php _e('Deselect all'); ?></a>
            <p class="description">
            <?php _e('If available, metadata in the selected languages (title, description) will be imported, in addition to the one in the default language.'); ?>
            </p>
          </td>
        </tr>-->

        <!-- Updated assets -->
        <tr>
          <th scope="row"><?php _e('Updated resources'); ?></th>
          <td>
            <?php echo okhub_select_box('okhubimport_options[updated_assets]', 'okhubimport_updated_assets', array('skip' => __('Always skip'), 'overwrite' => __('Always overwrite'), 'keep' => __('Keep both (pending of review)')), array(okhubapi_variable_get('okhubimport', 'updated_assets', OKHUB_IMPORT_UPDATED_ASSETS))); ?>
            <p class="description">
            <?php _e('Please indicate what to do with existing resources that have been updated in the source and have a more recent date_updated value than the one imported previously.'); ?>
            <br/>
            <?php _e('If "keep both" is selected, a new unpublished post will be created with the new values to be processed manually.'); ?>
            <br/>
            <?php _e('Please note that this will be applied only to documents and organisations. Categories will always be overwritten.'); ?>
            </p>
          </td>
        </tr>

        <!-- Document types -->
        <?php
        $existing_taxonomies = get_taxonomies(array('public' => TRUE, 'show_ui' => TRUE), 'objects');
        $select_existing_taxonomies = array();
        foreach ($existing_taxonomies as $tax_obj) {
          $select_existing_taxonomies[$tax_obj->name] = $tax_obj->labels->name;
        }
        foreach ($okhub_datasets as $dataset) { 
          $dataset_name = ucfirst($dataset); ?>
          <!--<tr valign="top" class="ids-categories-<?php echo $dataset; ?>">
            <th scope="row"><?php echo $dataset_name . __(' document types'); ?></th>
            <td id="radio_document_type_<?php echo $dataset; ?>">
              <label><input name="okhubimport_options[import_document_type_<?php echo $dataset; ?>]" type="radio" value="field" <?php checked('field', okhubapi_variable_get('okhubimport', 'import_document_type_' . $dataset, 'field')); ?> onchange="changeDocumentType();" /> Import as a field</label> <br />
              <label><input name="okhubimport_options[import_document_type_<?php echo $dataset; ?>]" type="radio" value="existing_taxonomy" <?php checked('existing_taxonomy', okhubapi_variable_get('okhubimport', 'import_document_type_' . $dataset, 'field')); ?> onchange="changeDocumentType();" /> Add to an existing taxonomy</label><br />
              <label><input name="okhubimport_options[import_document_type_<?php echo $dataset; ?>]" type="radio" value="new_taxonomy" <?php checked('new_taxonomy', okhubapi_variable_get('okhubimport', 'import_document_type_' . $dataset, 'field')); ?> onchange="changeDocumentType();" /> Add to a new taxonomy</label><br />
              <p class="description">
                <?php echo __('Please indicate how would you like to process "document types" information in ') . $dataset_name . __(' records.'); ?><br />
                <?php _e('If document types are imported as taxonomy terms, mappings can be created for them in the "Category settings" section.'); ?>
              </p>
            </td>
          </tr>-->
          <!-- Document types: existing taxonomy      
          <tr class="ids-categories-<?php echo $dataset; ?> document-type-taxonomy-<?php echo $dataset; ?> document-type-existing-taxonomy-<?php echo $dataset; ?>">
            <th scope="row"></th>
            <td>
              <?php echo okhub_select_box('okhubimport_options[import_document_type_taxonomy_'.$dataset.']', 'document_type_existing_taxonomy_'.$dataset, $select_existing_taxonomies, array(okhubapi_variable_get('okhubimport', 'import_document_type_taxonomy_'.$dataset, ''))); ?>
              <p class="description">
                <?php _e('Taxonomy to add "document type" terms to.'); ?>
              </p>
            </td>
          </tr> --> 
          <!-- Document types: new taxonomy
          <tr class="ids-categories-<?php echo $dataset; ?> document-type-taxonomy-<?php echo $dataset; ?> document-type-new-taxonomy-<?php echo $dataset; ?>">
            <th scope="row"></th>
            <td>
              <input id="document_type_taxonomy_label_singular_<?php echo $dataset; ?>" type="text" size="30" name="okhubimport_options[import_document_type_taxonomy_label_singular_<?php echo $dataset; ?>]" value="<?php echo okhubapi_variable_get('okhubimport', 'import_document_type_taxonomy_label_singular_'.$dataset, $dataset_name . ' document type'); ?>"/>
              <p class="description">
                <?php _e('New taxonomy label (singular).'); ?>
              </p>
              <br/>
              <input id="document_type_taxonomy_label_plural_<?php echo $dataset; ?>" type="text" size="30" name="okhubimport_options[import_document_type_taxonomy_label_plural_<?php echo $dataset; ?>]" value="<?php echo okhubapi_variable_get('okhubimport', 'import_document_type_taxonomy_label_plural_'.$dataset, $dataset_name . ' document types'); ?>"/>
              <p class="description">
                <?php _e('New taxonomy label (plural).'); ?>
              </p>
            </td>
          </tr> -->
        <?php } // foreach ?>

        <!-- Number of assets to retrieve -->
        <tr>
          <th scope="row"><?php _e('Excerpts length'); ?></th>
          <td>
            <input type="text" size="10" name="okhubimport_options[excerpt_length]" value="<?php echo okhubapi_variable_get('okhubimport', 'excerpt_length', 0); ?>" />
            <p class="description">
            <?php _e('Maximum length of excerpts to display.'); ?>
            <br/>
            <?php _e('If not set (or if the value is 0) the full description will be used.'); ?>
            </p>
          </td>
        </tr>

        <!-- Publication date -->
        <tr>
          <th scope="row"><?php _e('Publication date'); ?></th>
          <td>
            <?php echo okhub_select_box('okhubimport_options[pub_date]', 'okhubimport_pub_date', array('default' => __('Import date'), 'date_created' => __('Date created (OKHUB API)'), 'date_updated' => __('Date updated (OKHUB API)')), array(okhubapi_variable_get('okhubimport', 'pub_date', 'date_created'))); ?>
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
        if ($user = get_user_by('email', OKHUB_IMPORT_USER_EMAIL)) {
          $new_user_allowed = FALSE;
          $hide_field = 'style="display: none"';
        }
        else {
          $new_user_allowed = TRUE;
          $hide_field = '';
        }
        ?>
          <tr>
            <th scope="row"><?php _e('Username'); ?></th>
            <td>
              <input id="okhub_import_user" type="text" size="20" name="okhubimport_options[import_user]" value="<?php echo okhubapi_variable_get('okhubimport', 'import_user', OKHUB_IMPORT_IMPORT_USER); ?>" <?php echo $hide_field; ?>/>
              <?php if ($new_user_allowed) { ?>
                <p class="description">
                <?php _e('Enter a new username or select an existing one: '); ?>
              <?php } ?>
              <?php wp_dropdown_users(array('id' => 'okhub_import_user_select', 'name' => 'okhubimport_options[import_user_id]', 'selected' => okhubapi_variable_get('okhubimport', 'import_user_id', -1), 'show_option_none' => '-- Username --'));
              ?>
              <br/>
              <p class="description">
              <?php _e('Imported OKHUB content will be assigned to this user.'); ?>
              </p>
              </p>
            </td>
          </tr>

        <!-- Import tags (OKHUB keywords) -->
        <tr valign="top">
          <th scope="row"><?php _e('Import OKHUB keywords/themes'); ?></th>
          <td>
            <?php $import_tags = okhubapi_variable_get('okhubimport', 'import_tags', OKHUB_IMPORT_TAGS); ?>
            <label><input name="okhubimport_options[import_tags]" type="checkbox" <?php  echo ($import_tags ? 'checked="yes"' : ''); ?> /> <?php _e('Include keywords/themes as tags'); ?></label>
            <p class="description"><?php _e('Please indicate if you would like to include OKHUB keywords/themes as tags of the imported documents and/or organisations.'); ?></p>
          </td>
        </tr>
      </table>

      <!---------------------------------------- FILTERS ---------------------------------->

      <h3><?php _e('Import filters'); ?></h3>
      <table class="form-table ui-tabs-panel" id="filters">
        <tr>
          <td colspan="2">
            <p class="description"><?php _e('Documents and/or organisations to be imported can be limited to those matching the following filters.'); ?></p>
          </td>
        </tr>

        <!-- OKHUB Categories (countries, regions, themes) -->
        <?php foreach ($okhub_datasets as $dataset) { ?>
        <tr class="ids-categories-<?php echo $dataset; ?>">
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
            <select data-placeholder="Select <?php echo $category; ?>..." class="chzn-select" style="width:350px;" id='<?php echo $field_id; ?>' name='okhubimport_options[<?php echo $option_name; ?>][]' multiple='multiple' size='10'></select>
            <p class="description"><?php _e("Import only documents/organisations with the selected focus $category."); ?>
            <a href="javascript:selectAll('<?php echo $field_id; ?>');"><?php _e('Select all'); ?></a> / 
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
            <select data-placeholder="No themes selected" class="chzn-select" style="width:350px;" id='<?php echo $field_id; ?>' name='okhubimport_options[<?php echo $option_name; ?>][]' multiple='multiple' size='10'></select><br>
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
        <tr>
        <?php } // foreach dataset 
        ?>

          <!--<td colspan="2">
            <p class="ids-settings-separator">Additional filters</p>
          </td>-->
        </tr>

        <!-- Age of assets to import 
        <tr>
         <th scope="row"><?php _e('Age of documents and/or organisations'); ?></th>
          <td>
            <?php echo okhub_select_box('okhubimport_options[age_new_assets]', 'okhubimport_age_new_assets', array('0' => __('Do not filter by age'), '7' => __('One week'), '30' => __('One month'), '180' => __('Six months'), '365' => __('One year')), array(okhubapi_variable_get('okhubimport', 'age_new_assets', OKHUB_API_AGE_NEW_ASSETS))); ?>
            <p class="description"><?php _e('Only documents/organisations published after the indicated time will be imported.'); ?></p>
          </td>
        </tr>-->

        <!-- Resources' languages 
        <tr>
          <th scope="row"><?php _e('Resources\' languages'); ?></th>
          <td>
            <?php echo okhub_select_box('okhubimport_options[language_name_codes][]', 'okhubimport_language_name_codes', okhub_languages(), okhubapi_variable_get('okhubimport', 'language_name_codes', array()), array('multiple' => 'multiple')); ?>
            <a href="javascript:deselectAll('okhubimport_language_name_codes');"><?php _e('Deselect all'); ?></a>
            <p class="description">
            <?php _e('Only import resources that are available in the selected languages. If none is selected, resources in all languages will be imported.'); ?>
            </p>
          </td>
        </tr>-->

        <!-- Exclude URIs 
        <tr>
          <th scope="row"><?php _e('Exclude URIs'); ?></th>
          <td>
            <input type="text" size="57" name="okhubimport_options[exclude_uris]" value="<?php echo okhubapi_variable_get('okhubimport', 'exclude_uris', get_bloginfo('url')); ?>" /><br />
            <p class="description">
            <?php _e('Exclude resources that match certain URIs. Partial matching will be applied. Use commas to indicate several URIs.'); ?><br />
            <?php _e('This filter can be used, for instance, to avoid importing resources that are originated from this site.'); ?><br />
            <?php _e('Example: <i>http://www.worldbank.org/en/news, undp.org</i> would exclude WB\'s press releases and all resources from servers with undp.org as domain name.'); ?></p>
          </td>
        </tr>-->

        <!-- Authors
        <tr>
          <th scope="row"><?php _e('Documents\' author(s)'); ?></th>
          <td>
            <input type="text" size="57" name="okhubimport_options[import_authors_assets]" value="<?php echo okhubapi_variable_get('okhubimport', 'import_authors_assets', ''); ?>" /><br />
            <p class="description">
            <?php _e('A list of comma-separated authors\' names.'); ?><br />
            <?php _e('Example: <i>Chomsky, Foucault</i> will retrieve documents with either <i>Chomsky</i> or <i>Foucault</i> in the "author" field.'); ?></p>
          </td>
        </tr> -->

        <!-- Publishers 
        <tr>
          <th scope="row"><?php _e('Documents\' publisher(s)'); ?></th>
          <td>
            <input type="text" size="57" name="okhubimport_options[import_publishers_assets]" value="<?php echo okhubapi_variable_get('okhubimport', 'import_publishers_assets', ''); ?>" /><br />
            <p class="description">
            <?php _e('A list of comma-separated publishers\' names.'); ?><br />
            <?php _e('Example: <i>World Bank, UNDP</i> will retrieve documents with either <i>World Bank</i> or <i>UNDP</i> in the "publisher" field.'); ?></p>
          </td>
        </tr>-->

        <!-- Additional query terms
        <tr>
          <th scope="row"><?php _e('Query terms'); ?></th>
          <td>
            <input type="text" size="57" name="okhubimport_options[import_query]" value="<?php echo okhubapi_variable_get('okhubimport', 'import_query', ''); ?>" /><br />
            <p class="description">
            <?php _e('Query string to be appended to the API URL.'); ?><br />
            <?php _e('Use comma-separated pairs of unencoded query fields and values (example: <i>country=Argentina, keyword=climate change, language_name=Spanish</i>).'); ?><br />
            <?php printf(__('Please check the <a href="%s" target="_new">online documentation</a> for a list of available query fields.'), OKHUB_API_SEARCH_FIELDS_URL); ?>
           </p>
          </td>
        </tr> -->
      </table>

      <!---------------------------- DISPLAY SETTINGS ------------------------------>

      <h3><?php _e('General import settings'); ?></h3>
      <table class="form-table ui-tabs-panel" id="display">

        <!-- Include imported content in the loop -->
        <!--<tr valign="top">
          <th scope="row"><?php _e('Display imported content with regular posts'); ?></th>
          <td>
            <?php $include_imported_documents = okhubapi_variable_get('okhubimport', 'include_imported_documents', OKHUB_IMPORT_INCLUDE_IMPORTED_DOCUMENTS); ?>
            <label><input name="okhubimport_options[include_imported_documents]" type="checkbox" <?php  echo ($include_imported_documents ? 'checked="yes"' : ''); ?>" /> <?php _e('Include imported documents'); ?></label>
            <br/>
            <?php $include_imported_organisations = okhubapi_variable_get('okhubimport', 'include_imported_organisations', OKHUB_IMPORT_INCLUDE_IMPORTED_ORGANISATIONS); ?>
            <label><input name="okhubimport_options[include_imported_organisations]" type="checkbox" <?php  echo ($include_imported_organisations ? 'checked="yes"' : ''); ?>" /> <?php _e('Include imported organisations'); ?></label>

            <p class="description">
            <?php _e('Please indicate if you would like to include the imported OKHUB documents / organisations when displaying regular posts.'); ?><br>
            <?php _e('If these options are not checked, OKHUB documents / organisations will not be displayed on your site by default, but they will still be accesible through specific URLs, available on:'); ?><br>
            <a href="<?php echo get_admin_url(); ?>/admin.php?page=okhubimport_menu"><?php echo get_admin_url(); ?>/admin.php?page=okhubimport_menu</a>
            </p>
          </td>
        </tr>-->

        <!-- Dataset in public areas -->
        <tr valign="top">
          <th scope="row"><?php _e('Status of imported content'); ?></th>
          <td>
            <!--
            Commented - This setting should be reconsidered when the plugin is updated to see if we keep the option to set this at source level.
            <label><input name="okhubimport_options[display_dataset_public]" type="radio" value="hub" <?php checked('hub', okhubapi_variable_get('okhubimport', 'display_dataset_public', OKHUB_IMPORT_DEFAULT_DATASET_PUBLIC)); ?> /> Eldis only</label> <br />
            <label><input name="okhubimport_options[display_dataset_public]" type="radio" value="bridge" <?php checked('bridge', okhubapi_variable_get('okhubimport', 'display_dataset_public', OKHUB_IMPORT_DEFAULT_DATASET_PUBLIC)); ?> /> Bridge only</label><br />
            -->
            <label><input name="okhubimport_options[display_dataset_public]" type="radio" value="none" <?php checked('none', okhubapi_variable_get('okhubimport', 'display_dataset_public', OKHUB_IMPORT_DEFAULT_DATASET_PUBLIC)); ?> /> Pending review</label><br />
            <label><input name="okhubimport_options[display_dataset_public]" type="radio" value="all" <?php checked('all', okhubapi_variable_get('okhubimport', 'display_dataset_public', OKHUB_IMPORT_DEFAULT_DATASET_PUBLIC)); ?> /> Published</label><br />
            <p class="description">
            <!--?php _e('Content from the indicated datasets will be marked as "published" when imported. If a dataset is not selected, its content will be imported with "pending" status.'); ?><br-->
            <?php _e('Please note that this applies to new content. The status of content that has already been imported can be changed in the plugin\'s administrative area.'); ?>
            </p>
          </td>
        </tr>

        <!-- Datasets to display in admin areas - Displaying all the imported sources. Uncomment to enable this setting again -->
        <!--
        <tr valign="top">
          <th scope="row"><?php _e('Datasets in administrative areas'); ?></th>
          <td>

            <label><input name="okhubimport_options[display_dataset_admin]" type="radio" value="hub" <?php checked('hub', okhubapi_variable_get('okhubimport', 'display_dataset_admin', OKHUB_IMPORT_DEFAULT_DATASET_ADMIN)); ?> /> Eldis only</label> <br />
            <label><input name="okhubimport_options[display_dataset_admin]" type="radio" value="bridge" <?php checked('bridge', okhubapi_variable_get('okhubimport', 'display_dataset_admin', OKHUB_IMPORT_DEFAULT_DATASET_ADMIN)); ?> /> Bridge only</label><br />
            <label><input name="okhubimport_options[display_dataset_admin]" type="radio" value="all" <?php checked('all', okhubapi_variable_get('okhubimport', 'display_dataset_admin', OKHUB_IMPORT_DEFAULT_DATASET_ADMIN)); ?> /> All sources</label><br />
            <p class="description"><?php _e('Please indicate the datasets to display in the administrative areas of your site.'); ?></p>
          </td>
        </tr>
        -->
      </table>

      <!------------------------------- CATEGORIES SETTINGS ------------------------>

      <h3><?php _e('Categories settings'); ?></h3>

      <table class="form-table ui-tabs-panel" id="categories">
        <tr>
          <td colspan="4">
            <p class="description">
            <?php _e('Please indicate if you want to import and use OKHUB categories (countries, regions, themes) and if OKHUB categories associated to documents/organisations should be mapped to existing WordPress categories.'); ?><br>
            </p>
        <?php $okhubimport_new_categories = okhubapi_variable_get('okhubimport', 'import_new_categories', OKHUB_IMPORT_NEW_CATEGORIES); ?>
        <table class="form-table">
        <tr valign="top">
          <th scope="row"><?php _e('Categories settings'); ?></th>
          <td>
            <label><input id="okhub_new_categories" name="okhubimport_options[import_new_categories]" type="checkbox" <?php echo ($okhubimport_new_categories ? 'checked="yes"' : '');  ?> onchange="changeNewCategories();" /> Enable OKHUB categories </label>
            <p class="description">
            <?php _e('If checked, OKHUB countries, themes and regions will be imported and be made available on Wordpress as terms.'); ?><br>
            <?php _e('They will also be automatically assigned to imported documents and organisations.'); ?><br>
            <?php _e('If this option is unchecked, OKHUB countries, themes and regions will not be imported and <u>will not be available on Wordpress</u>.'); ?>
            </p>
          </td>
        </tr>

        <!-- Category levels to import -->
        <tr valign="top" class="ids-categories-new">
          <th scope="row"><?php _e('Level of categories'); ?></th>
          <td>
          <label><?php echo okhub_select_box('okhubimport_options[level_categories]', 'okhubimport_level_categories', array(1 => __('One level'), 2 => __('Two levels'), 0 => __('All levels')), array(okhubapi_variable_get('okhubimport', 'level_categories', OKHUB_IMPORT_CATEGORY_LEVELS))); ?></label>
          <p class="description">
          <?php _e('The level shows the position in the thematic hierarchy, starting at 1, the top level. Select the category levels to import.'); ?>
          </p>
          </td>
        </tr>

         <?php $okhubimport_map_categories = okhubapi_variable_get('okhubimport', 'import_map_categories', OKHUB_IMPORT_MAP_CATEGORIES); ?>
        <tr valign="top">
          <th scope="row"><?php _e('Category mappings'); ?></th>
          <td>
            <label><input id="okhub_map_categories" name="okhubimport_options[import_map_categories]" type="checkbox" <?php echo ($okhubimport_map_categories ? 'checked="yes"' : ''); ?> onchange="changeMappingsSettings();" /> Map to existing categories </label><br />
            <p class="description">
              <?php _e('Map OKHUB countries, regions and themes to categories to terms in Wordpress\' taxonomies when importing documents and/or organisations.'); ?><br/>
              <?php _e('Please note that this option is independent of the category importing settings: an OKHUB category can be imported as a new term and, in addition, mapped to an existing category.'); ?></p>
          </td>
        </tr>
        </table>

        <!-- Wordpress taxonomies -->
        <?php
          $wp_terms_select = array();
          $all_taxonomies = get_taxonomies(array('public' => TRUE, 'show_ui' => TRUE));
          foreach($all_taxonomies as $tax_name) { 
            $all_wp_terms = get_terms($tax_name, array('hide_empty' => FALSE));
            foreach ($all_wp_terms as $wp_term) {
              $wp_terms_select[$tax_name][$tax_name.'-'.$wp_term->term_id] = $wp_term->name;
            }
          }
        ?>

        <table class="form-table">
        <?php if (!empty($wp_terms_select)) { ?>
          <!-- OKHUB Categories mappings (countries, regions, themes) -->
          <?php foreach ($okhub_datasets as $dataset) { ?>
            <tr valign="top" class="ids-categories-mapping ids-mappings-<?php echo $dataset; ?>">
              <td colspan="4">
                <p id="title_categories" class="ids-settings-separator"><?php echo ucfirst($dataset) . ' mappings'; ?></p>
              </td>
            </tr>
            <?php foreach ($okhub_categories as $category) { 
              $field_id_source = $dataset . '_' . $category . '_source';
              $field_id_mappings = $dataset . '_' . $category . '_mappings';
              $option_name_mappings = $dataset . '_' . $category . '_mappings';
              $category_name = ucfirst($category);
              $reg_expr_datasets = '/(' . implode('|', $okhub_datasets) . ')/';
              ?>
              <tr valign="top" class="ids-categories-mapping ids-mappings-section ids-mappings-<?php echo $dataset; ?>">
              <th scope="row"><?php _e($category_name); ?></th>
                <td class="ids-categories-container" style="vertical-align: top;">
                  <select class="ids-mapping-select" id='<?php echo $field_id_source; ?>' multiple='multiple' size='5'></select>
                    <p class="description"><a href="javascript:selectAll('<?php echo $field_id_source; ?>');"><?php _e('Select all'); ?></a> / <a href="javascript:deselectAll('<?php echo $field_id_source; ?>');"><?php _e('Deselect all'); ?></a></p>
                </td>
                <td class="ids-categories-container" style="vertical-align: top;">
                <?php 
                foreach ($all_taxonomies as $tax_name) {
                  if (!preg_match($reg_expr_datasets, $tax_name) && isset($wp_terms_select[$tax_name])) {
                    $taxonomy = get_taxonomy($tax_name);
                    $field_id_target = $dataset . '_' . $category . '_' . $tax_name . '_target';
                    ?>
                    <p class="description"><?php _e('Map selected ' . $category . ' to ' . strtolower($taxonomy->labels->name)); ?></p>
                    <?php echo okhub_select_box($field_id_target, $field_id_target, $wp_terms_select[$tax_name], array(), array('class' => 'ids-mapping-select')); ?>
                    <br />
                    <p class="description"><a href="javascript:addMappings('<?php echo $field_id_source; ?>', '<?php echo $field_id_target; ?>', '<?php echo $field_id_mappings; ?>');"><?php _e('Add mapping'); ?> &rarr;</a></p>
                    <br />
                  <?php 
                  }
                } // foreach WP taxonomy ?> 
                </td>
                <td class="ids-categories-container" style="vertical-align: top;">
                  <select class="ids-mapping-select" name='okhubimport_options[<?php echo $option_name_mappings; ?>][]' id='<?php echo $field_id_mappings; ?>' multiple='multiple' size='5'></select>
                  <p class="description"> <a href="javascript:removeMappings('<?php echo $field_id_mappings; ?>');">&larr; <?php _e('Remove selected'); ?></a> / <a href="javascript:selectAll('<?php echo $field_id_mappings; ?>');"><?php _e('Select all'); ?></a> / <a href="javascript:deselectAll('<?php echo $field_id_mappings; ?>');"><?php _e('Deselect all'); ?></a></p>
                </td>
              </tr>
              <?php } // foreach OKHUB category ?>
          <?php } // foreach dataset ?>
        <?php } // there are no wordpress categories to map to
        else { ?>
            <tr valign="top" class="ids-categories-mapping ids-mappings-<?php echo $dataset; ?>">
              <td colspan="4">
                <p id="title_categories" class="ids-settings-separator"><?php _e('Please, create the Wordpress categories to map to first.'); ?></p>
              </td>
            </tr>          
        <?php } ?>
        </table>

          </td>
        </tr>
      </table>

      <!------------------------------------ PERIODIC IMPORTS ----------------------------->

      <h3><?php _e('Periodic import'); ?></h3>
      <table class="form-table ui-tabs-panel" id="periodic_import">
        <!-- Types of content to import -->
        <tr valign="top">
          <th scope="row"><?php _e('Select the types of content to import periodically'); ?></th>
          <td>
            <label><strong><?php _e('Assets'); ?></strong></label><br/>
            <label><input id="okhub_import_documents_checkbox" name="okhubimport_options[import_documents]" type="checkbox" <?php echo (okhubapi_variable_get('okhubimport', 'import_documents', '0') ? 'checked="yes"' : ''); ?> onchange="changeImportSettings();" /> <?php _e('Documents'); ?></label> &nbsp;&nbsp;&nbsp;
            <label><input id="okhub_import_organisations_checkbox" name="okhubimport_options[import_organisations]" type="checkbox" <?php  echo (okhubapi_variable_get('okhubimport', 'import_organisations', '0') ? 'checked="yes"' : ''); ?> onchange="changeImportSettings();" /> <?php _e('Organisations'); ?></label>
            <p class="description"><?php _e('Assets (documents and organisations) are updated frequently.'); ?></p>
          </td>
        </tr>
        <tr class = "ids-categories-new">
          <th scope="row"></th>
          <td>
            <label><strong><?php _e('Categories'); ?></strong></label><br/>
            <label><input id="okhub_import_countries_checkbox" name="okhubimport_options[import_countries]" type="checkbox" <?php echo (okhubapi_variable_get('okhubimport', 'import_countries', '0') ? 'checked="yes"' : ''); ?> onchange="changeImportSettings();" /> <?php _e('Countries'); ?></label> &nbsp;&nbsp;&nbsp;
            <label><input id="okhub_import_regions_checkbox" name="okhubimport_options[import_regions]" type="checkbox" <?php echo (okhubapi_variable_get('okhubimport', 'import_regions', '0') ? 'checked="yes"' : ''); ?> onchange="changeImportSettings();" /> <?php _e('Regions'); ?></label> &nbsp;&nbsp;&nbsp;
            <label><input id="okhub_import_themes_checkbox" name="okhubimport_options[import_themes]" type="checkbox" <?php echo (okhubapi_variable_get('okhubimport', 'import_themes', '0') ? 'checked="yes"' : ''); ?> onchange="changeImportSettings();" /> <?php _e('Themes'); ?></label>
            <p class="description"><?php _e('Categories are updated less frequently and it could be a good alternative to import them manually.'); ?></p>
          </td>
        </tr>

        <!-- Frequency to import -->
        <?php $okhubimport_schedules = okhubimport_get_schedules(); ?>
        <tr class="ids-assets-import-field">
          <th scope="row"><?php _e('Frequency to import/update documents/organisations'); ?></th>
          <td>
            <?php echo okhub_select_box('okhubimport_options[import_period_assets]', 'okhubimport_import_period_assets', $okhubimport_schedules, array(okhubapi_variable_get('okhubimport', 'import_period_assets', OKHUB_IMPORT_IMPORT_RECURRENCE_ASSETS))); ?>
            <p class="description"><?php _e('Please indicate the frequency with which you would like to import or update documents and/or organisations.'); ?></p>
          </td>
        </tr>

        <tr class="ids-categories-import-field ids-categories-new">
          <th scope="row"><?php _e('Frequency to import/update categories'); ?></th>
          <td>
            <?php echo okhub_select_box('okhubimport_options[import_period_categories]', 'okhubimport_import_period_categories', $okhubimport_schedules, array(okhubapi_variable_get('okhubimport', 'import_period_categories', OKHUB_IMPORT_IMPORT_RECURRENCE_CATEGORIES))); ?>
            <p class="description"><?php _e('Please indicate the frequency with which you would like to import or update categories (countries, regions and themes).'); ?></p>
          </td>
        </tr>
      </table>

    </div>	<!-- UI tabs -->	

      <p class="submit">
        <input name="okhubimport_options[submit_save]" type="submit" class="button-primary" value="<?php _e('Save changes') ?>"/>
      </p>

    <?php } else { // If API key is not validated ?>
      </table>
      <p class="submit ids-import-settings-message">
        <?php _e('Please validate your OKHUB API key in order to continue setting the plugin options.'); ?>
        <input name="okhubimport_options[submit_validate]" type="submit" class="button-primary" value="<?php _e('Validate') ?>" />
      </p>
    <?php } ?>
		</form>
    </div>	<!-- ids-wrap -->	
  </div><!-- wrap -->
	<?php	
}

// Add columns to the posts edit page
function okhubimport_posts_columns($defaults) {
  global $okhub_categories;
  global $okhub_datasets;
  $okhubimport_new_categories = okhubapi_variable_get('okhubimport', 'import_new_categories', OKHUB_IMPORT_NEW_CATEGORIES);
  $site = '';
  $datasets = $okhub_datasets;
  $post_type = get_query_var('post_type');
  if (!$post_type) {
    $post_type = 'post';
  }
  $site = get_query_var('okhub_site');
/*  if (!$site && (isset($_GET['hub_countries']) || isset($_GET['hub_regions']) || isset($_GET['hub_themes']))) {
    $site = 'hub';
  }
  elseif (!$site && (isset($_GET['bridge_countries']) || isset($_GET['bridge_regions']) || isset($_GET['bridge_themes']))) {
    $site = 'bridge';
  }
  if (($site == 'hub') || ($site == 'bridge')) {
    $datasets = array($site);
  }
 */
  $datasets = array($site);
  $supported_taxonomies = get_object_taxonomies($post_type);
  foreach ($datasets as $dataset) {
    foreach ($okhub_categories as $category) {
      $taxonomy_name = $dataset . '_' . $category;
      if (taxonomy_exists($taxonomy_name) && $okhubimport_new_categories && in_array($taxonomy_name, $supported_taxonomies)) {
        $category_name = ucfirst($dataset) . ' ' . ucfirst($category);
        $defaults[$taxonomy_name] = $category_name;
      }
    }
  }
  return $defaults;
}

// Add columns contents in the posts edit page.
// TODO: Generalize.
function okhubimport_populate_posts_columns($column){
  global $post;
  $is_okhub_category = preg_match('/[hub]/', $column);
  if ($is_okhub_category) {
    $str = explode('_', $column);
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
      $output = 'No ' . ucfirst($str[1]);
    }
    echo $output;
  }
}

// Removes the OKHUB category widgets if the categories are not enabled in settings.
function okhubimport_edit_post_form() {
  global $okhub_datasets;
  global $okhub_categories;
  $okhubimport_new_categories = okhubapi_variable_get('okhubimport', 'import_new_categories', OKHUB_IMPORT_NEW_CATEGORIES);
  if (!$okhubimport_new_categories) {
    $types = get_post_types(array('_builtin' => false));
    $types[] = 'post';
    foreach ($okhub_datasets as $dataset) {
      foreach ($okhub_categories as $category) {
        foreach ($types as $type) {
          $supported_taxonomies = get_object_taxonomies($type);
          $custom_taxonomy_name = $dataset . '_' . $category;
          if (in_array($custom_taxonomy_name, $supported_taxonomies)) {
            remove_meta_box($custom_taxonomy_name.'div', $type, 'side');
          }
        }
      }
    }
  }
}

// Functions to list OKHUB imported categories.

function okhubimport_hub_countries_page() {
  okhubimport_okhub_categories_page('hub', 'countries');
}

function okhubimport_hub_regions_page() {
  okhubimport_okhub_categories_page('hub', 'regions');
}

function okhubimport_hub_themes_page() {
  okhubimport_okhub_categories_page('hub', 'themes');
}

/*function okhubimport_bridge_countries_page() {
  okhubimport_okhub_categories_page('bridge', 'countries');
}

function okhubimport_bridge_regions_page() {
  okhubimport_okhub_categories_page('bridge', 'regions');
}

function okhubimport_bridge_themes_page() {
  okhubimport_okhub_categories_page('bridge', 'themes');
}
*/
function okhubimport_hub_document_type_page() {
  okhubimport_document_type_page('hub');
}
/*
function okhubimport_bridge_document_type_page() {
  okhubimport_document_type_page('bridge');
}
*/
function okhubimport_document_type_page($dataset) {
  $taxonomy_name = okhubapi_variable_get('okhubimport', 'import_document_type_taxonomy_'.$dataset, $dataset.'_document_type');
  $taxonomy_singular_label = okhubapi_variable_get('okhubimport', 'import_document_type_taxonomy_label_singular_'.$dataset, ucfirst($dataset). ' document type');
  okhubimport_taxonomy_page($taxonomy_name, $taxonomy_singular_label);  
}

function okhubimport_okhub_categories_page($dataset, $category) {
  $taxonomy_name = $dataset . '_' . $category;
  $taxonomy_singular_label = ucfirst($dataset) . ' ' . ucfirst($category);
  okhubimport_taxonomy_page($taxonomy_name, $taxonomy_singular_label);
}

function okhubimport_taxonomy_page($taxonomy_name, $taxonomy_singular_label) {
?>
  <div class="wrap">
  <div id="icon-edit-pages" class="icon32 icon32-posts-page"><br /></div>
  <h2><?php echo $taxonomy_singular_label; ?> <a href="edit-tags.php?taxonomy=<?php echo $taxonomy_name; ?>" class="add-new-h2">Add/Edit/Delete <?php echo $taxonomy_singular_label; ?></a> </h2>
  <br/><br/>
  <ul class="ids-category-list">
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
function okhubimport_help_page() {
?>
  <div class="wrap">
  <div id="icon-edit-comments" class="icon32 icon32-posts-page"><br /></div>
  <h2><?php _e('Help'); ?></h2>
  <p>
  <?php _e('The OKHUB KS API plugin allows access to OKHUB Knowledge Services content of thematically organised and hand selected academic research on poverty reduction in the developing world that is freely available to access online.'); ?>
  </p>
  <p>
  <?php _e('OKHUB Knowledge Services offer two collections that can be accessed via this plugin. They are:'); ?>
  </p>
  <p>
  <?php _e('BRIDGE is a research and information service supporting gender advocacy and mainstreaming efforts by bridging the gaps between theory, policy and practice. BRIDGE acts as a catalyst by facilitating the generation and exchange of relevant, accessible and diverse gender information in print, online and through other innovative forms of communication. BRIDGE hosts a global resources library on its website, which includes gender-focused information materials in a number of languages, including Arabic, Chinese, English, French, Portuguese and Spanish.'); ?>
  </p>
  <p>
  <?php _e('Eldis is an online information service covering development research on a wide change of thematic areas including agriculture, climate change, conflict, gender, governance, and health. Eldis includes over 32,000 summaries and links to free full-text research and policy documents from over 8,000 publishers.  Each document is editorially selected by members of our team.'); ?>
  </p>
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
  <ul class="ids-category-list">
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
  <b><?php _e('How do I use the plugin?'); ?></b>
  <p>
  <?php _e('To use the plugin, you must obtain a unique Token-GUID or key for the API. Please register for your API key <a href="http://api.ids.ac.uk/accounts/register/" target="_new">here</a>. Once obtained, enter this key into the OKHUB API Token-GUID (key) section of the OKHUB Plugin Settings.'); ?>
  </p>
  <p>
  <?php _e('The OKHUB API package actually has two plugins on offer, both allow the administrator to select content from'); ?> <a href="http://www.hub.org/" target="_new">Eldis</a>, <a href="http://www.bridge.ids.ac.uk/" target="_new">BRIDGE</a> <?php _e('or both collections, and bring relevant content easily into the site.'); ?>
  </p>
  <b><?php _e('OKHUB Import plugin'); ?></b>
  <p>
  <?php _e('This plugin enables the one time or periodic importing of content into the Wordpress database. OKHUB documents and organisations are imported as custom Wordpress content types, while OKHUB regions, countries and themes are imported as Wordpress custom taxonomies. Content and terms can also be imported manually.'); ?>
  </p>
  <p>
  <?php _e('You can select the number of items to import and assign to a specific Wordpress user. You can filter the content you import by country, region or theme. It is also possible the filter by publisher or author name.  You can integrate content seamlessly into your existing content structures by mapping Eldis or BRIDGE taxonomies to existing Wordpress taxonomies.'); ?>
  </p>
  <p>
  <?php _e('So, if you were the administrator of a website that highlighted recent research on ICTs and Gender in Kenya, you would be able to select specific criteria to suit your content. You would select the themes ICT and Gender, as well as the country Kenya.'); ?>
  </p>
  <b><?php _e('Tips for searching'); ?></b>
  <p>
  <?php _e('Although the Eldis and BRIDGE datasets are very wide ranging, very specific search terms may not return the number of results that you expect or want. Therefore, if you are not getting the results you expect, it might be worth trying more generic search terms or even simply searching at the Theme level.'); ?>
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
  <?php _e('For terms and condition for using API see'); ?> <a href="http://api.ids.ac.uk/about/terms/" target="_new"><?php _e('here'); ?></a>.
  </p>
  </div>
<?php
}

// Helper function to display the names of selected categories in the OKHUB Importer page from their object_ids.
function okhubimport_get_names($selected_object_ids, $names) {
  $ret = array();
  foreach ($selected_object_ids as $object_id) {
    if (isset($names[$object_id])) {
      $ret[] = $names[$object_id];
    }
  }
  return $ret;
}

// Format the array of available schedules to be displayed on the admin page.
function okhubimport_get_schedules() {
  $available_schedules = wp_get_schedules();
  uasort($available_schedules, 'okhubimport_sched_sort');
  foreach ($available_schedules as $recurrence => $sched) {
    $okhubimport_schedules[$recurrence] = $sched['display'];
  }
  return $okhubimport_schedules;
}

// Sorts the schedules according to the time period
function okhubimport_sched_sort($sched1, $sched2) {
  if ($sched1['interval'] == $sched2['interval']) {
    return 0;
  }
  return ($sched1['interval'] < $sched2['interval']) ? -1 : 1;
}

