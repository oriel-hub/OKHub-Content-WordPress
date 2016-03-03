var $jqorig = jQuery.noConflict();

function initOKHubWidget() {
  $jqorig('#okhub_widget_dialog').hide();
  $jqorig('.okhub-widget-open-pop-up').click(function(e){
    var title = $jqorig(e.target).text();
    var item = getOKHubItemId(title);
    if (item) {
      e.preventDefault();
      displayItemOKHubWidget(item);
    }
  });
}

function getOKHubItemId(title) {
  var item = null;
  for (var i = 0; i < okhub_widget_assets.length; i++) {  
    if (okhub_widget_assets[i].title == title) {
      var item_id = okhub_widget_assets[i].item_id;
      var type = okhub_widget_assets[i].type;
      var source = okhub_widget_assets[i].source;
      item = {'item_id': item_id, 'type': type, 'source': source};
      break;
    }
  }
  return item;
}

function displayItemOKHubWidget(item) {
  // var url = home_url + '?okhub_get_item=' + item.item_id + '&okhub_source=' + item.source + '&okhub_type=' + item.type;
  $jqorig('#okhub_widget_dialog').bPopup({
    appending: false,
	scrollBar: true,
	positionStyle: 'fixed',
    opacity: 0.3,
    content: 'ajax',
    contentContainer:'.okhub-popup-content',
    loadUrl: home_url,
    loadData: {'okhub_get_item': item.item_id, 'okhub_source': item.source, 'okhub_type': item.type},
  });
}

function changeTypeOKHubWidget(select_id) {
  var type = $jqorig('#' + select_id + ' option:selected').val();
  $jqorig('.okhub-widget-div-templates').show();
  $jqorig('.okhub-widget-div-templates').not('#okhub-widget-div-template-' + type).hide();
  $jqorig('.okhub-widget-div-check-templates').attr('checked', false);
  $jqorig('.okhub-widget-div-no-templates').show();
}

function useTemplateOKHubWidget(checkbox_id) {
  var checked = $jqorig('#' + checkbox_id).prop('checked');
  if (checked) {
    $jqorig('.okhub-widget-div-no-templates').hide();
  }
  else {
    $jqorig('.okhub-widget-div-no-templates').show();
  }
}

function displayContentOKHubWidget(source, type) {
  $jqorig('.okhub-source-widget').removeClass('okhub-widget-selected');
  $jqorig('#okhub-source-widget-' + source).addClass('okhub-widget-selected');
  $jqorig('.okhub-widget-content').hide();
  $jqorig('#okhub-widget-content-' + source).show();
}


if (window.addEventListener) {
  window.addEventListener('load', initOKHubWidget, false);
} else if (window.attachEvent) {
  window.attachEvent('onload', initOKHubWidget);
}
