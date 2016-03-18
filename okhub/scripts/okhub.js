function blockAdminPage() {
    $jqorig.blockUI({
        message: 'Please wait...',
        bindEvents: true,
        timeout: 0,
        css: { 
            border: 'none', 
            padding: '15px', 
            backgroundColor: '#000', 
            '-webkit-border-radius': '10px', 
            '-moz-border-radius': '10px', 
            opacity: .5, 
            color: '#fff' 
        }
    });   
}

function hideTabs() {
	$jqorig(document).ready(function($jqorig) {
		$jqorig(".okhub-ui-tabs-panel").each(function(index) {
			if (index > 0)
				$jqorig(this).addClass("okhub-ui-tabs-hide");
		});
		$jqorig(".okhub-ui-tabs").tabs({ fx: { opacity: "toggle", duration: "fast" } });
	});
}

function changeImportSettings() {
  // TODO: Generalise.
  var assets_import_enabled = ($jqorig("#okhub_import_documents_checkbox").prop("checked") || $jqorig("#okhub_import_organisations_checkbox").prop("checked"));
  if (assets_import_enabled) {
    $jqorig(".okhub-assets-import-field").show();
  } else {
    $jqorig(".okhub-assets-import-field").hide();
  }
  // TODO: Generalise.
  var categories_import_enabled = ($jqorig("#okhub_new_categories").prop("checked") && ($jqorig("#okhub_import_countries_checkbox").prop("checked") || $jqorig("#okhub_import_regions_checkbox").prop("checked") || $jqorig("#okhub_import_themes_checkbox").prop("checked") || $jqorig("#okhub_import_documenttypes_checkbox").prop("checked")));
  if (categories_import_enabled) {
    $jqorig(".okhub-categories-import-field").show();
  } else {
    $jqorig(".okhub-categories-import-field").hide();
  }
}

function changeFields() {
  $jqorig.each($jqorig('.okhub_expose-content-types'), function(i, type) { 
    type_name = $jqorig(type).attr("id");
    if ($jqorig(type).prop("checked")) {
      $jqorig('.' + 'fields-' + type_name).show();
    }
    else {
      $jqorig('.' + 'fields-' + type_name).hide();
    }
  });
}

function changeFeedType() {
  var type = $jqorig('#okhub_expose_type_feed option:selected').val();
  if (type === 'posts') {
    $jqorig('#okhub_expose_select_posts').show();
    $jqorig('#okhub_expose_select_categories').hide();
  }
  else {
    $jqorig('#okhub_expose_select_posts').hide();
    $jqorig('#okhub_expose_select_categories').show();
  }
  changeFeedFilters();
}

function changeFeedFilters() {
  var feed_type = $jqorig('#okhub_expose_type_feed option:selected').val();
  var type = $jqorig('#okhub_expose_' + feed_type + ' option:selected').val();
  $jqorig('.okhub_expose_filters').hide();
  $jqorig('.okhub_expose_filters_' + type).show();
  $jqorig('#okhub_expose_' + feed_type + '_cats_op').val('OR');
  generateFeedUrl();
}

function generateFeedUrl() {
  var feed_type = $jqorig('#okhub_expose_type_feed option:selected').val();
  var query_string = $jqorig('#okhub_expose_original_' + feed_type + '_url').val();
  var type = $jqorig('#okhub_expose_' + feed_type + ' option:selected').val();
  var num_items = $jqorig('#okhub_expose_num_items').val();
  if (type) {
    if (feed_type === 'posts') {
      query_string += '&post_type=' + type;
    }
    else {
      query_string += '&taxonomy=' + type;
    }
  }
  if (num_items) {
    query_string += '&num_items=' + num_items;
  }
  var num_cats = 0;
  $jqorig('.okhub_expose_' + type +'_taxonomy_select').each(function(i, taxonomy){
    var tax_name = $jqorig(taxonomy).attr("id");
    var tax_values = $jqorig('#' + tax_name + ' option:selected').map(function(){ return this.value }).get();
    if (tax_values.length !== 0) {
      num_cats++;
      var string_values = tax_values.join('|');
      var tax_query = tax_name.split('-');
      query_string += '&cats[' + tax_query[1] + ']=' + string_values;
    }
  });
  if (num_cats > 1) {
    var cats_op = $jqorig('#okhub_expose_' + feed_type + '_cats_op').val();
    if (cats_op) {
      query_string += '&cats_op=' + cats_op;
    }
  }
  if (query_string) {
    $jqorig('#okhub_expose_feed_url').val(query_string);
  }
}

function gotoFeed() {
  var feed_url = $jqorig('#okhub_expose_feed_url').val();
  window.open(feed_url,'_blank');
}

function initDatasets() {
  if (selected_datasets.length == 0) {
    selected.datasets = [default_dataset];
  }
  $jqorig.each(selected_datasets, function(i, dataset) {
    $jqorig('#okhub_selected_datasets option[value="' + dataset +'"]').attr("selected","selected");
  });
  $jqorig('#okhub_selected_datasets').trigger("chosen:updated");
  changeDataset();
}

function changeDataset() {
  datasets_selected = $jqorig('#okhub_selected_datasets option:selected').length;
  $jqorig.each(datasets, function(i, dataset) {
    if ($jqorig('#okhub_selected_datasets option[value="' + dataset +'"]').prop("selected")) {
      $jqorig('.okhub-categories-' + dataset).show();
      $jqorig('.okhub-mappings-' + dataset).show();
    }
    else {
      $jqorig('.okhub-categories-' + dataset).hide();
      $jqorig('.okhub-mappings-' + dataset).hide();
    }
  });
}

function changeNewCategories() {
  if ($jqorig("#okhub_new_categories").prop("checked")) {
    $jqorig(".okhub-categories-new").show();
  }
  else {
    $jqorig(".okhub-categories-new").hide();
  }
  changeImportSettings();
}

function addMappings(id_source, id_target, id_mappings) {
  var target_val = $jqorig('#' + id_target + ' option:selected').val();
  var target_title = $jqorig('#' + id_target + ' option:selected').text();
  $jqorig('#' + id_source + ' option:selected').each(function(i, selected){
    value_mapping = $jqorig(selected).val() + ',' + target_val;
    existing_value = $jqorig('#' + id_mappings + ' option[value="' + value_mapping + '"]').val();
    if (existing_value == undefined) {
      title_mapping = $jqorig(selected).text() + ' --> ' + target_title;
      $jqorig('#' + id_mappings).prepend($jqorig("<option></option>").attr("value", value_mapping).text(title_mapping));
    }
  });
}

function changeMappingsTargets(dataset, category) {
  $jqorig('.okhub-mapping-target-'+ dataset + '-' + category).hide();
  var selected_tax = $jqorig('#' + dataset + '_' + category + '_select_target option:selected').val();
  if (selected_tax != '') {
    var div_id = dataset + '_' + category + '_' + selected_tax + '_div_target';
    $jqorig('#' + div_id).show();
  }
}

function populateSelectBoxes(dataset, category) {
  id_select_assets = dataset + '_' + category + '_assets';
  id_select_sources = dataset + '_' + category + '_source';
  id_select_mappings = dataset + '_' + category + '_mappings';
  select_category_assets = $jqorig('#' + id_select_assets);
  select_category_assets.empty();
  select_category_sources = $jqorig('#' + id_select_sources);
  select_category_sources.empty();
  select_category_mappings = $jqorig('#' + id_select_mappings);
  select_category_mappings.empty();

  dataset_category_array = okhub_array_categories[dataset][category];
  if (dataset_category_array != null) {
    /* Populate the filter select boxes */
    $jqorig.each(dataset_category_array, function(i, cat) {
      select_category_assets.append($jqorig("<option></option>").attr("value", cat[0]).text(cat[1]));
    });
    /* Mark previously selected categories */
    select_category_assets.val(selected_categories[dataset][category]);
    /* Populate mapping sources select boxes */
    $jqorig.each(okhub_array_categories[dataset][category], function(i, cat) {
      select_category_sources.append($jqorig("<option></option>").attr("value", cat[0]).text(cat[1]));
    });
  }

  dataset_selected_mappings_array = selected_categories_mappings[dataset][category];
  if (dataset_selected_mappings_array != null) {
    /* Populate previously selected mappings */
    $jqorig.each(dataset_selected_mappings_array, function(okhub_category, mapped_categories) {
      $jqorig.each(mapped_categories, function(i, wp_category) {
        tax = wp_category.split('-');
        tax_name = tax[0];
        //tax_value = tax[1];
        wp_category_select_id = dataset + '_' + category + '_' + tax_name + '_target'; //eldis_countries_category_target
        wp_category_name = $jqorig('#' + wp_category_select_id + ' option[value="' + wp_category + '"]').text();
        okhub_category_name = $jqorig('#' + id_select_sources + ' option[value="' + okhub_category + '"]').text();
        title = okhub_category_name + ' --> ' + wp_category_name;
        select_category_mappings.append($jqorig("<option></option>").attr("value", okhub_category + "," + wp_category).text(title));
      });
    });
  }
}

function selectAll(id_cat_field) {
  var id_select = "#" + id_cat_field;
  $jqorig(id_select + ' option').prop('selected', true);
  $jqorig(id_select).trigger("chosen:updated");
}

function deselectAll(id_cat_field) {
  var id_select = "#" + id_cat_field;
  $jqorig(id_select).val([]);
  $jqorig(id_select).trigger("chosen:updated");
}

function removeAll(id_cat_field) {
  var id_select = "#" + id_cat_field;
  $jqorig(id_select).empty();
  $jqorig(id_select).trigger("chosen:updated");
}

function selectAllMappings() {
  $jqorig.each(datasets, function(i, dataset) { 
    $jqorig.each(okhub_categories, function(j, category) {
      id_select = dataset + '_' + category + '_mappings';
      selectAll(id_select);
    });
  });
}

function removeMappings(id_cat_field) {
  $jqorig("#" + id_cat_field + " option:selected").remove();
}

function updateUsername() {
  var new_user = $jqorig("#okhub_user_select option:selected").val();
  if (new_user == -1) {
    new_username = default_user;
  }
  else {
    new_username = $jqorig("#okhub_user_select option:selected").html();
  }
  $jqorig("#okhub_user").val(new_username);
}

/* default_user is defined in function okhub_init_javascript */
function updateDefaultUser() {
  default_user = $jqorig("#okhub_user").val();
}

function loadTaxonomies(dataset, category, refresh) {
  updateSelectBoxes(dataset, category, refresh);
  select_assets = '#' + dataset + '_' + category + '_assets';
  $jqorig(select_assets).trigger('chosen:open'); 
}

function updateSelectBoxes(dataset, category, refresh) {
  $jqorig(".okhub-load-" + dataset + "-" + category).prop('disabled', true);
  $jqorig(".okhub-refresh-" + dataset + "-" + category).prop('disabled', true);
  $jqorig.ajax({
    url: home_url,
    dataType: 'json',
    data: {'okhub_javascript': 1, 'okhub_source': dataset, 'okhub_category': category, 'okhub_refresh': refresh}
  })
  .done(function(result) {
    okhub_array_categories[dataset][category] = result;
    populateSelectBoxes(dataset, category);
    // Hide/show load/refresh buttons
    $jqorig(".okhub-load-" + dataset + "-" + category).hide();
    $jqorig(".okhub-refresh-" + dataset + "-" + category).show();
    select_assets = '#' + dataset + '_' + category + '_assets';
    select_sources = '#' + dataset + '_' + category + '_source';
    select_mappings = '#' + dataset + '_' + category + '_mappings';
    //$jqorig(select_assets).data("chosen").default_text = "Select "  + category + '...';
    $jqorig(select_assets).chosen().placeholder_text_multiple  = "Select "  + category + '...';
    //$jqorig(select_sources).data("chosen").default_text = "Select "  + category + '...';
    $jqorig(select_sources).chosen().placeholder_text_single  = "Select "  + category + '...';
    $jqorig(select_assets).trigger("chosen:updated");
    $jqorig(select_sources).trigger("chosen:updated");
    $jqorig(select_mappings).trigger("chosen:updated");
  })
  .fail(function( jqXHR, textStatus ) {
    // alert("Request failed while retrieving " + dataset + " " + category + ". Error: " + textStatus + ". Output: " + JSON.stringify(jqXHR));
  })
  $jqorig(".okhub-load-" + dataset + "-" + category).prop('disabled', false);
  $jqorig(".okhub-refresh-" + dataset + "-" + category).prop('disabled', false);
}

function initSelectedBoxes() {
  $jqorig.each(selected_datasets, function(i, dataset) {
    $jqorig.each(okhub_categories, function(j, category) {
      updateSelectBoxes(dataset, category);
    });
  });
}


function loadAdminPage() {
  var config = {
    '.chosen-select'           : {search_contains:true, placeholder_text_multiple:"...", placeholder_text_single:"..."},
    '.chosen-select-deselect'  : {allow_single_deselect:true},
    '.chosen-select-no-single' : {disable_search_threshold:10},
    '.chosen-select-no-results': {no_results_text:'Oops, nothing found!'},
    '.chosen-select-width'     : {width:"95%"}
  }
  for (var selector in config) {
    $jqorig(selector).chosen(config[selector]);
  }
  hideTabs();
  if (okhub_plugin == 'okhub_expose') {
    changeFields();
    changeFeedType();
  }
  else {
    initDatasets();
    $jqorig('.okhub-mapping-target').hide();
    $jqorig('.okhub-refresh-buttons').hide();
    $jqorig('.okhub-mapping-target').trigger("chosen:updated");
    initCategoriesArrays();
    initSelectedBoxes();
    changeNewCategories();
    $jqorig("#okhub_user_select").change(updateUsername); 
    $jqorig("#okhub_user").change(updateDefaultUser);
  }
  $jqorig.unblockUI();
}

if (window.addEventListener) {
  window.addEventListener('load', loadAdminPage, false);
} else if (window.attachEvent) {
  window.attachEvent('onload', loadAdminPage);
}


