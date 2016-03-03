<?php
/**
 * This template is used to display a list of documents retrieved by the OKHub Widget.
 * See the file content-okhub_documents.php for a list of fields that could be used.
 * The variable $okhub_source contains the source code of the dataset selected in the widget's administration form.
 * The variable $okhub_data contains an array of OkhubApiObject elements. Each one with a document's data, as returned by the API.
 * The function okhub_get_api_value($document, $field_name, $okhub_source='', $language_code='', $return_single=TRUE) can be used to retrieved specific fields.
 * Depending on the field name and the passed parameters, this function can return a scalar or an array.
 * Please see the files okhubwrapper.default.inc and okhub.default.inc to see a list of available API fields.
 */
?>

<h3 class="widget-title">OKHub content</h3>
<ul class="okhub-widget-list">
  <?php 
  $source = okhub_get_source($okhub_source); // Retrieve array with source information. See okhub.default.inc for a list of all available source's fields.
  foreach ($okhub_data as $document) {
    $website_url = okhub_get_api_value($document, OKHUB_API_FN_WEBSITE_URL, $okhub_source);
    $url = okhub_get_api_value($document, OKHUB_API_FN_URLS, $okhub_source, '', TRUE);
    $title = okhub_get_api_value($document, OKHUB_API_FN_TITLE, $okhub_source, 'en');
    $description = okhub_get_api_value($document, OKHUB_API_FN_DESCRIPTION, $okhub_source, 'en');
    if (!empty($source[OKHUB_API_FN_SOURCE_NAME])) {
      $source_name = $source[OKHUB_API_FN_SOURCE_NAME];
    }
    else {
      $source_name = okhub_get_dataset_acronym($okhub_source);
    }
    if (!empty($source[OKHUB_API_FN_SOURCE_WEBSITE])) {
      $source_name = '<a href="'.$source[OKHUB_API_FN_SOURCE_WEBSITE].'">'.$source_name.'</a>';
    }    
    
    ?>
    <li class="okhub-widget-item">
      <div class="okhub-widget-title">
        <!-- Use class="okhub-widget-open-pop-up" in order to open the document in a modal pop-up box. In this case, the link will be ignored. -->
        <a class="okhub-widget-open-pop-up" href="<?php echo ($website_url) ? $website_url : $url; ?>"><?php echo $title; ?></a>
      </div>
      <div class="okhub-widget-description">
        <?php $first_sentence = preg_replace('/([^?!.]*.).*/', '\\1', $description); ?>
        <?php echo $first_sentence; ?>
      </div>
      <div class="okhub-widget-source">
        Source: <?php echo $source_name; ?>
      </div>
    </li>
  <?php
  }
?>
</ul>












