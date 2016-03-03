<?php
/**
 * This template is used to display an individual document retrieved by the OKHub Widget in a modal pop-up.
 * See the file content-okhub_documents.php for a list of fields that could be used.
 * The variable $okhub_source contains the source code of the dataset selected in the widget's administration form.
 * The variable $okhub_data contains the item to be displayed as an OkhubApiObject element.
 * The function okhub_get_api_value($document, $field_name, $source_code='', $language_code='', $return_single=TRUE) can be used to retrieved specific fields.
 * Depending on the field name and the passed parameters, this function can return a scalar or an array.
 * Please see the files okhubwrapper.default.inc and okhub.default.inc to see a list of available API fields.
 */

  $sources_codes = okhub_get_api_value($okhub_data, OKHUB_API_FN_SOURCES, '', '', FALSE); // Get the item's sources' codes.
?>
  <div id="okhub-widget-tabs">  
  <?php
  foreach ($sources_codes as $source_code) { // Display logos / names for each source.
    $source = okhub_get_source($source_code); // Retrieve array with source information. See okhub.default.inc for a list of all available sources' fields.
    $sources[$source_code] = $source;
    $selected = ($source_code == $okhub_source) ? 'okhub-widget-selected' : '';
    if (!empty($source[OKHUB_API_FN_SOURCE_LOGO])) {
      //echo '<a href="javascript:displayContentOKHubWidget(\''.$source_code.'\')">;
      echo '<img id="okhub-source-widget-'.$source_code.'" class="okhub-source-widget '.$selected.'" src="'.$source[OKHUB_API_FN_SOURCE_LOGO].'" onclick="displayContentOKHubWidget(\''.$source_code.'\')">';
      // echo '</a>';
    }
    elseif (!empty($source[OKHUB_API_FN_SOURCE_NAME])) {
      echo '<span id="okhub-source-widget-'.$source_code.'" class="okhub-source-widget '.$selected.'" onclick="javascript:displayContentOKHubWidget(\''.$source_code.'\')">'.$source[OKHUB_API_FN_SOURCE_NAME].'</span>';  
    }
    else {
      echo '<span id="okhub-source-widget-'.$source_code.'" class="okhub-source-widget '.$selected.'" onclick="javascript:displayContentOKHubWidget(\''.$source_code.'\')">'.$source_code.'</span>';
    }
  }
  ?>
  </div>

  <?php
  foreach ($sources_codes as $source_code) {  // Display content for each source.
    $title = okhub_get_api_value($okhub_data, OKHUB_API_FN_TITLE, $source_code, 'en');
    $description = okhub_get_api_value($okhub_data, OKHUB_API_FN_DESCRIPTION, $source_code, 'en');

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
            if ($website_url = okhub_get_api_value($okhub_data, OKHUB_API_FN_WEBSITE_URL, $source_code)) {
            ?>
                <b>URL:</b> <a href="<?php echo $website_url; ?>"><?php echo $website_url; ?></a>
            <?php
            }
            ?>
            <br/><br/>
            <?php
            if (!empty($sources[$source_code][OKHUB_API_FN_SOURCE_LICENSE_DESCRIPTION])) {
            ?>
                <b>License description:</b> <?php echo $sources[$source_code][OKHUB_API_FN_SOURCE_LICENSE_DESCRIPTION]; ?>
            <?php
            }
            ?>
            
            <!------ Source description ------>
            <p class="okhub-widget-source-description">
                <?php echo (!empty($sources[$source_code][OKHUB_API_FN_SOURCE_DESCRIPTION])) ? $sources[$source_code][OKHUB_API_FN_SOURCE_DESCRIPTION] : ''; ?>
            </p>
        </div>
        
    </div>
  <?php
  }