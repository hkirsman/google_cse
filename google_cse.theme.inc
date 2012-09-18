<?php

/**
 * @file
 * Themeable functions for Google Custom Search Engine.
 */

/**
 * The search results page can be themed/customized.
 */
function template_preprocess_google_cse_results(&$variables) {
  $query = google_cse_build_query(isset($_GET['query']) ? $_GET['query'] : '', NULL, FALSE);
  $variables['results_searchbox_form'] = $variables['form'] ? drupal_get_form('google_cse_results_searchbox_form') : '';
  $variables['noscript'] = t('!google, or enable JavaScript to view them here.', array('!google' => l(t('View the results at Google'), 'http://'. variable_get('google_cse_domain', 'www.google.com') .'/cse', array('query' => $query))));
  $variables['prefix'] = filter_xss_admin(variable_get('google_cse_results_prefix', ''));
  $variables['suffix'] = filter_xss_admin(variable_get('google_cse_results_suffix', ''));

  if (google_cse_validate_request()) {
    // drupal_add_js($variables['path'] . '/google_cse_results.js', array('scope' => 'footer'));
    // drupal_add_js('https://www.google.com/afsonline/show_afs_search.js', array('type' => 'external', 'scope' => 'footer'));
    drupal_add_js('//www.google.com/jsapi', array('type' => 'external', 'scope' => 'footer'));
    drupal_add_js('
    google.load(\'search\', \'1\', {language : \'en\'});
    google.setOnLoadCallback(function() {
      var customSearchOptions = {};
      customSearchOptions[\'adoptions\'] = {\'layout\': \'noTop\'};
      var customSearchControl = new google.search.CustomSearchControl(\'' . variable_get('google_cse_cx', '') . '\', customSearchOptions);
      customSearchControl.setResultSetSize(google.search.Search.FILTERED_CSE_RESULTSET);
      var options = new google.search.DrawOptions();
      options.enableSearchResultsOnly();
      customSearchControl.draw(\'cse\', options);
      function parseParamsFromUrl() {
        var params = {};
        var parts = window.location.search.substr(1).split(\'\x26\');
        for (var i = 0; i < parts.length; i++) {
          var keyValuePair = parts[i].split(\'=\');
          var key = decodeURIComponent(keyValuePair[0]);
          params[key] = keyValuePair[1] ?
              decodeURIComponent(keyValuePair[1].replace(/\+/g, \' \')) :
              keyValuePair[1];
        }
        return params;
      }

      var urlParams = parseParamsFromUrl();
      var queryParamName = "query";
      if (urlParams[queryParamName]) {
        customSearchControl.execute(urlParams[queryParamName]);
      }
    }, true);', array('type' => 'inline', 'scope' => 'footer'));
    drupal_add_css('//www.google.com/cse/style/look/default.css', 'external');
    // google.load(\'search\', \'1\', {language : \'en\', style : google.loader.themes.MINIMALIST});
  }
}

/**
 * Display an Add-to-Google button.
 */
function template_preprocess_google_cse_results_gadget(&$variables) {
  $cx = explode(':', variable_get('google_cse_cx', ''));
  $variables['creator'] = rawurlencode($cx[0]);
  $variables['id'] = isset($cx[1]) ? rawurlencode($cx[1]) : '';
}

/**
 * Validate GET parameters to avoid displaying inappropriate search results.
 */
function google_cse_validate_request() {
  return (
    (empty($_GET['cx']) || $_GET['cx'] == variable_get('google_cse_cx', '')) &&
    (empty($_GET['safe']) || $_GET['safe'] == variable_get('google_cse_safe', '')) &&
    (empty($_GET['sitesearch']) || (($options = google_cse_sitesearch_options()) && isset($options[$_GET['sitesearch']])))
  );
}

/**
 * Form builder for the searchbox forms.
 */
function google_cse_results_searchbox_form($form, &$form_state) {
  $form = array();
  if (variable_get('google_cse_results_display', 'here') == 'here') {
    $cof = variable_get('google_cse_cof_here', 'FORID:11');
  }
  else {
    $form['#action'] = 'http://'. variable_get('google_cse_domain', 'www.google.com') .'/cse';
    $cof = variable_get('google_cse_cof_google', 'FORID:0');
  }
  $form['#method'] = 'get';
  $form['cx'] = array(
    '#type' => 'hidden',
    '#value' => variable_get('google_cse_cx', ''),
  );
  $form['cof'] = array(
    '#type' => 'hidden',
    '#value' => $cof,
  );
  $form['query'] = array(
    '#type' => 'textfield',
    '#default_value' => isset($_GET['query']) ? $_GET['query'] : '',
  );
  $form['sa'] = array(
    '#type' => 'submit',
    '#value' => t('Search'),
  );
  foreach (google_cse_advanced_settings() as $parameter => $setting) {
    $form[$parameter] = array(
      '#type' => 'hidden',
      '#value' => $setting,
    );
  }
  $form['query']['#size'] = intval(variable_get('google_cse_results_searchbox_width', 40));
  $form['query']['#title'] = t('Enter your keywords');
  if (variable_get('google_cse_results_gadget', 1)) {
    $form['sa']['#suffix'] = theme('google_cse_results_gadget');
  }
  google_cse_sitesearch_form($form);
  return $form;
}
