<?php
/**
 * @file
 * Picture - map breakpoints and image styles.
 */

/**
 * Select breakpoint group form.
 */
function picture_select_breakpoint_group_form($form, &$form_state) {
  $groups = breakpoints_breakpoint_group_load_all();
  $options = array();
  foreach ($groups as $group) {
    $options[$group->machine_name] = $group->name;
  }
  if ($options) {
    $form['breakpoint_group'] = array(
      '#title' => t('Breakpoint group'),
      '#type' => 'select',
      '#required' => TRUE,
      '#description' => t('Select a breakpoint group.'),
      '#options' => $options,
      '#default_value' => $form_state['item']->getBreakpointGroup() ? $form_state['item']->getBreakpointGroup()->machine_name : NULL,
    );
  }
  else {
    $form['breakpoint_group'] = array(
      '#title' => t('Breakpoint group'),
      '#type' => 'item',
      '#markup' => t(
        'There are no breakpoint groups. !create_link.',
        array('!create_link' => l(t('Create a breakpoint group'), 'admin/config/media/breakpoints/groups/add'))
      ),
    );
  }

  return $form;
}

/**
 * Validate breakpoint group.
 */
function picture_select_breakpoint_group_form_validate($form, &$form_state) {
  if (!isset($form_state['values']['breakpoint_group']) || empty($form_state['values']['breakpoint_group'])) {
    form_error($form['breakpoint_group'], t('No breakpoint group was selected.'));
  }
}

/**
 * Submit callback.
 */
function picture_select_breakpoint_group_form_submit($form, &$form_state) {
  $form_state['item']->setBreakpointGroup($form_state['values']['breakpoint_group']);
}

/**
 * Build picture mapping form.
 */
function picture_mapping_form(&$form, &$form_state) {
  $picture_mapping = $form_state['item'];
  $form['info']['label']['#required'] = TRUE;
  $form['info']['machine_name']['#title'] = t('Machine name');
  $groups = breakpoints_breakpoint_group_load_all();
  $options = array();
  foreach ($groups as $group) {
    $options[$group->machine_name] = $group->name;
  }
  $form['breakpoint_group'] = array(
    '#title' => t('Breakpoint group'),
    '#type' => 'select',
    '#required' => TRUE,
    '#description' => t('Select a breakpoint group.'),
    '#options' => $options,
    '#default_value' => $picture_mapping->getBreakpointGroup()->machine_name,
    '#ajax' => array(
      'callback' => 'picture_mapping_form_change_breakpoint_group',
      'wrapper' => 'picture-mapping-container',
      'method' => 'replace',
      'effect' => 'fade',
    ),
  );

  $form['mapping'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'id' => 'picture-mapping-container',
    ),
    '#tree' => TRUE,
  );

  $image_styles = image_style_options(TRUE) + array(
    PICTURE_EMPTY_IMAGE => t('Empty image'),
    PICTURE_ORIGINAL_IMAGE => t('Original image'),
  );

  $minimum_speed_options = array();
  foreach (preg_split ('/$\R?^/m', variable_get('picture_network_speeds', '')) as $network_type) {
    list ($speed, $label) = explode('|', $network_type, 2);
    $minimum_speed_options[$speed] = $label;
  }
  foreach ($picture_mapping->getMappings() as $breakpoint_id => $mapping) {
    foreach ($mapping as $multiplier => $mapping_definition) {
      $breakpoint = breakpoints_breakpoint_load_by_fullkey($breakpoint_id);
      $label = $multiplier . ' ' . $breakpoint->name . ' [' . $breakpoint->breakpoint . ']';
      $form['mapping'][$breakpoint_id][$multiplier] = array(
        '#type' => 'fieldset',
        '#title' => check_plain($label),
      );
      $form['mapping'][$breakpoint_id][$multiplier]['mapping_type'] = array(
        '#title' => t('Type'),
        '#type' => 'radios',
        '#options' => array(
          '_none' => t('Do not use this breakpoint'),
          'speed' => t('Use image style to load dynamically by network speed'),
          'image_style' => t('Use image styles'),
          'sizes' => t('Use the sizes attribute'),
        ),
        '#default_value' => isset($mapping_definition['mapping_type']) ? $mapping_definition['mapping_type'] : '_none',
      );
      $form['mapping'][$breakpoint_id][$multiplier]['minimum_speed'] = array(
        '#type' => 'select',
        '#title' => t('Minimum connection speed type'),
        '#options' => $minimum_speed_options,
        '#default_value' => isset($mapping_definition['minimum_speed']) ? $mapping_definition['minimum_speed'] : '',
        '#description' => t('Enter minimum connection speed required to use this image style. The image will be loaded only if connection speed match this condition.'),
        '#states' => array(
          'visible' => array(
            ':input[name="mapping[' . $breakpoint_id . '][' . $multiplier . '][mapping_type]"]' => array('value' => 'speed'),
          ),
        ),
      );

      $form['mapping'][$breakpoint_id][$multiplier]['image_style'] = array(
        '#type' => 'select',
        '#title' => t('Image style'),
        '#options' => $image_styles,
        '#maxlength' => 1024,
        '#default_value' => isset($mapping_definition['image_style']) ? $mapping_definition['image_style'] : '',
        '#description' => t('Select an image style for this breakpoint.'),
        '#states' => array(
          'visible' => array(
            array(
              array(':input[name="mapping[' . $breakpoint_id . '][' . $multiplier . '][mapping_type]"]' => array('value' => 'image_style')),
              'or',
              array(':input[name="mapping[' . $breakpoint_id . '][' . $multiplier . '][mapping_type]"]' => array('value' => 'speed')),
            ),
          ),
        ),
      );
      $form['mapping'][$breakpoint_id][$multiplier]['sizes'] = array(
        '#type' => 'textfield',
        '#title' => t('Sizes'),
        '#default_value' => isset($mapping_definition['sizes']) ? $mapping_definition['sizes'] : '',
        '#description' => t('Enter the value for the sizes attribute (e.g. "(min-width:700px) 700px, 100vw").'),
        '#states' => array(
          'visible' => array(
            ':input[name="mapping[' . $breakpoint_id . '][' . $multiplier . '][mapping_type]"]' => array('value' => 'sizes'),
          ),
        ),
      );
      $form['mapping'][$breakpoint_id][$multiplier]['sizes_image_styles'] = array(
        '#title' => t('Image styles'),
        '#type' => 'checkboxes',
        '#options' => array_diff_key($image_styles, array('' => '')),
        '#default_value' => isset($mapping_definition['sizes_image_styles']) ? $mapping_definition['sizes_image_styles'] : array(),
        '#states' => array(
          'visible' => array(
            ':input[name="mapping[' . $breakpoint_id . '][' . $multiplier . '][mapping_type]"]' => array('value' => 'sizes'),
          ),
        ),
      );

    }
  }
  $form['#tree'] = FALSE;
  return $form;
}

/**
 * Helper function for the form.
 */
function picture_mapping_form_change_breakpoint_group($form, &$form_state) {
  return $form['mapping'];
}

/**
 * Validate callback.
 */
function picture_mapping_form_validate($form, &$form_state) {
  $picture_mapping = $form_state['item'];
  if ($form_state['triggering_element']['#type'] != 'submit') {
    $picture_mapping->setBreakpointGroup($form_state['values']['breakpoint_group']);
    $form_state['rebuild'] = TRUE;
    return;
  }
  $picture_mapping->setLabel($form_state['values']['label']);
  $picture_mapping->setMachineName($form_state['values']['machine_name']);
  $picture_mapping->setMappings($form_state['values']['mapping']);
  // Make sure at least one mapping is defined.
  if (!$picture_mapping->hasMappings()) {
    form_error($form['mapping'], t('Please select at least one mapping.'));
  }
  foreach ($form_state['values']['mapping'] as $breakpoint => $mapping) {
    foreach ($mapping as $multiplier => $data) {
      if ($data['mapping_type'] == 'speed') {
        // User has to enter valid speed start and end values if 'speed' option was chosen.
        if (!preg_match('/^\d+(\.\d*)?$/', $data['minimum_speed'])) {
          $error_elements = array('mapping' , $breakpoint, $multiplier, 'minimum_speed');
          form_set_error(implode('][', $error_elements), t('Please enter a valid number'));
        }
      }
    }
  }
}

/**
 * Chooses which picture groups are available in the CKEditor image dialog.
 */
function picture_admin_settings() {
  $form = array();
  $picture_mappings = picture_get_mapping_options();

  // Check if picture mappings have been configured before proceeding.
  if ($picture_mappings) {
    // Create a settings form.
    $form['description'] = array(
      '#type' => 'item',
      '#title' => t('Choose which picture mappings will be available in the CKEditor image dialog.'),
      '#description' => t('See picture_wysiwyg.css for an example of how to style these images in your theme using the selectors suggested below.'),
    );

    // Retrieve pre-existing settings.
    $ckeditor_mappings = variable_get('picture_ckeditor_mappings', array());

    // Loop through each picture mapping and place a checkbox and weight.
    foreach ($picture_mappings as $machine_name => $display_name) {
      $form[$machine_name] = array(
        '#type' => 'fieldset',
        '#title' => t('@name picture mapping', array('@name' => $display_name)),
      );
      $form[$machine_name]['enabled'] = array(
        '#type' => 'checkbox',
        '#default_value' => isset($ckeditor_mappings[$machine_name]) ? $ckeditor_mappings[$machine_name]['enabled'] : 0,
        '#title' => t('Include @name picture mapping in the CKEditor image dialog', array('@name' => $display_name)),
      );
      $form[$machine_name]['css'] = array(
        '#type' => 'item',
        '#markup' => t('Consider using the selector <code>span[data-picture-mapping="@machine_name"]</code> in your theme CSS.', array(
          '@machine_name' => $machine_name,
        )),
      );
      $form[$machine_name]['weight'] = array(
        '#type' => 'select',
        '#title' => t('Weight'),
        '#options' => drupal_map_assoc(array(1, 2, 3, 4, 5, 6, 7, 8, 9, 10)),
        '#default_value' => isset($ckeditor_mappings[$machine_name]) ? $ckeditor_mappings[$machine_name]['weight'] : 1,
        '#description' => t('Control the sort order of picture mappings in the CKEditor "size" drop-down. Higher weights sink to the bottom of the list.'),
      );
      $form[$machine_name]['fallback'] = array(
        '#type' => 'select',
        '#title' => t('Fallback image style'),
        '#options' => drupal_map_assoc(array_keys(image_styles())) + array(
          PICTURE_EMPTY_IMAGE => t('Empty image'),
        ),
        '#default_value' => isset($ckeditor_mappings[$machine_name]) ? $ckeditor_mappings[$machine_name]['fallback'] : NULL,
      );
      $form[$machine_name]['lazyload'] = array(
        '#title' => t('Picture lazyload'),
        '#description' => t('Image will be rendered when it appears in viewport, helps to optimize page load speed.'),
        '#type' => 'checkbox',
        '#default_value' => isset($ckeditor_mappings[$machine_name]['lazyload']) ? $ckeditor_mappings[$machine_name]['lazyload'] : 0,
      );

      $form[$machine_name]['lazyload_aspect_ratio'] = array(
        '#title' => t('Keep aspect ratio'),
        '#type' => 'checkbox',
        '#description' => t('Preserve the space for the image being lazyloaded to avoid layout reflows. <br /> Image ratio is defined per breakpoint, make sure all images from srcset have the same ratio. <br />Output example: !example',
          array('!example' => htmlentities('<source media="(...)" data-srcset="image_400x200.jpg x1, iamge_800x400.jpg x2, image_1200x600.jpg x3" data-aspectratio="2" />'))
        ),
        '#default_value' => isset($ckeditor_mappings[$machine_name]['lazyload_aspect_ratio']) ? $ckeditor_mappings[$machine_name]['lazyload_aspect_ratio'] : 0,
        '#states' => array(
          'visible' => array(
            ':input[name="article_responsive_image[lazyload]"]' => array('checked' => TRUE),
            ':input[name="article_responsive_image[fallback]"]' => array('value' => PICTURE_EMPTY_IMAGE),
          ),
        ),
      );
    }
    $form['#tree'] = TRUE;
    $form['ckeditor_label'] = array(
      '#type' => 'textfield',
      '#title' => t('Label in the CKEditor image dialog'),
      '#description' => t('This sets the label for the drop-down select box containing these picture mappings in the CKEditor image dialog'),
      '#default_value' => variable_get('picture_ckeditor_label', 'Image size (required)'),
    );
  }
  else {
    $form['info'] = array(
      '#markup' => t('No picture mappings found. !create', array('!create' => l(t('Create one now.'), 'admin/config/media/picture/add'))),
    );
  }
  if (module_exists('file_entity') && function_exists('file_type_load_all') && !variable_get('picture_updated_to_file_entity_2', FALSE)) {
    $form['picture_updated_to_file_entity_2'] = array(
      '#type' => 'checkbox',
      '#title' => t('Update the file formatter to version 2 of the File Entity module'),
      '#default_value' => variable_get('picture_updated_to_file_entity_2', FALSE),
    );
  }

  $form['picture_js_scope'] = array(
    '#title' => t('Picture JavaScript scope'),
    '#description' => t("For performance reasons, JS should be loaded in the footer. However, you can move the Picture module's JS to the header so that it loads before JS from other modules like Views Slideshow."),
    '#type' => 'radios',
    '#options' => array(
      'header' => t('Header'),
      'footer' => t('Footer'),
    ),
    '#default_value' => variable_get('picture_js_scope', 'footer'),
  );

  $form['picture_polyfill_version'] = array(
    '#title' => t('Picture polyfill to use'),
    '#description' => t('For performance reasons, use the minified version.'),
    '#type' => 'radios',
    '#options' => array(
      'min' => t('Minified polyfill'),
      'dev' => t('Development polyfill'),
      'own' => t('Use custom polyfill'),
    ),
    '#default_value' => variable_get('picture_polyfill_version', 'min'),
  );

  $form['picture_fallback_method'] = array(
    '#title' => t('Picture fallback method'),
    '#description' => t('Use the fallback method, the defualt is to use srcset since it avoids double downloads, but src is the right method and will also work if the browser does not support javascript.'),
    '#type' => 'radios',
    '#options' => array(
      'src' => t('src fallback'),
      'srcset' => t('srcset fallback'),
    ),
    '#default_value' => variable_get('picture_fallback_method', 'srcset'),
  );

  $form['picture_img_sizes_output_method'] = array(
    '#title' => t('"Image with sizes" output method'),
    '#description' => t('"srcset on Image tag" creates the correct markup, however "picture element" will prevent duplicate downloads.'),
    '#type' => 'radios',
    '#options' => array(
      'img_srcset' => t('srcset on Image tag'),
      'picture_element' => t('Picture element'),
    ),
    '#default_value' => variable_get('picture_img_sizes_output_method', 'picture_element'),
  );

  $form['picture_network_speed_test_image'] = array(
    '#title' => t('Image for Network Speed Test'),
    '#type' => 'managed_file',
    '#required' =>  TRUE,
    '#description' => t('Upload an image to be used to calcualate network speed on user systems.'),
    '#default_value' => variable_get('picture_network_speed_test_image', ''),
    '#upload_location' => 'public://picture/',
    '#upload_validators' => array(
      'file_validate_extensions' => array('gif png jpg jpeg'),
      // Pass the maximum file size in bytes
      'file_validate_size' => array(2*1024*1024),
    ),
  );

  $form['picture_network_speeds'] = array(
    '#title' => t('Network speeds'),
    '#type' => 'textarea',
    '#required' =>  TRUE,
    '#description' => t('Specify minimum network speeds with labels. Enter one pair of network speed and label in each line. Network speed and label has to be sperated with |. For example, "1|One Mpbs connection". Network speed value are assumed in Mbps.'),
    '#default_value' => variable_get('picture_network_speeds', ''),
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Save',
  );
  return $form;
}

/**
 * Validate handler for the picture_ckeditor_settings form.
 *
 * It checks that a fallback image style is selected for every
 * picture mapping that has been enabled for the CKEditor image dialog.
 */
function picture_admin_settings_validate($form, &$form_state) {
  $picture_mappings = picture_mapping_load_all();
  if ($picture_mappings) {
    foreach ($picture_mappings as $picture_mapping) {
      $machine_name = $picture_mapping->getMachineName();
      if ($form_state['values'][$machine_name]['enabled'] == 1) {
        if (empty($form_state['values'][$machine_name]['fallback'])) {
          form_set_error($machine_name . '][fallback', t('Please choose a fallback image style for this picture mapping'));
        }
      }
    }
  }
  $network_types = preg_split ('/$\R?^/m', $form_state['values']['picture_network_speeds']);
  foreach ($network_types as $index => $network_type) {
    if (!empty($network_type) && !preg_match('/^\d+(\.\d*)?\|.+$/', $network_type)) {
      form_set_error('picture_network_speeds', t('Invalid item network speed type item on line @line', array('@line' => $index + 1)));
    }
  }
}

/**
 * Submit handler, places chosen picture groups into the variables table.
 */
function picture_admin_settings_submit($form, &$form_state) {
  $picture_mappings = picture_mapping_load_all();
  $ckeditor_mappings = array();
  if ($picture_mappings) {
    // Loop each picture mapping and record the settings.
    foreach ($picture_mappings as $picture_mapping) {
      $machine_name = $picture_mapping->getMachineName();
      $ckeditor_mappings[$machine_name]['enabled'] = $form_state['values'][$machine_name]['enabled'];
      $ckeditor_mappings[$machine_name]['weight'] = $form_state['values'][$machine_name]['weight'];
      $ckeditor_mappings[$machine_name]['fallback'] = $form_state['values'][$machine_name]['fallback'];
      $ckeditor_mappings[$machine_name]['lazyload'] = $form_state['values'][$machine_name]['lazyload'];
      $ckeditor_mappings[$machine_name]['lazyload_aspect_ratio'] = $form_state['values'][$machine_name]['lazyload_aspect_ratio'];
    }

    uasort($ckeditor_mappings, 'picture_compare_weights');
    variable_set('picture_ckeditor_mappings', $ckeditor_mappings);
    variable_set('picture_ckeditor_label', $form_state['values']['ckeditor_label']);
  }

  if (isset($form_state['values']['picture_updated_to_file_entity_2']) && $form_state['values']['picture_updated_to_file_entity_2']) {
    variable_set('picture_updated_to_file_entity_2', _picture_update_to_file_entity_2());
  }

  variable_set('picture_js_scope', $form_state['values']['picture_js_scope']);
  variable_set('picture_polyfill_version', $form_state['values']['picture_polyfill_version']);
  variable_set('picture_fallback_method', $form_state['values']['picture_fallback_method']);
  variable_set('picture_img_sizes_output_method', $form_state['values']['picture_img_sizes_output_method']);

  $old_file_id = variable_get('picture_network_speed_test_image');
  $new_file_id = $form_state['values']['picture_network_speed_test_image'];
  if ($old_file_id != $new_file_id) {
    if (!empty($old_file_id)) {
      // User uploaded new file. Delte old file.
      $old_file = file_load($old_file_id);
      file_usage_delete($old_file, 'picture', 'picture', 0);
      file_delete($old_file);
    }

    variable_set('picture_network_speed_test_image', $form_state['values']['picture_network_speed_test_image']);
    // Load the file via file.fid.
    $file = file_load($form_state['values']['picture_network_speed_test_image']);
    // Change status to permanent.
    $file->status = FILE_STATUS_PERMANENT;
    // Save.
    file_save($file);
    // Record that the module (in this example, user module) is using the file.
    file_usage_add($file, 'picture', 'picture', 0);
  }
  variable_set('picture_network_speeds', $form_state['values']['picture_network_speeds']);

  drupal_set_message(t('Your settings have been saved'));
}

/**
 * Update picture module to version 2 of File Entity module.
 */
function _picture_update_to_file_entity_2() {
  // File Entity is not installed or the installed version is not version 2.
  if (!module_exists('file_entity') || !function_exists('file_type_load_all')) {
    return FALSE;
  }
  // Picture was already updated.
  if (variable_get('picture_updated_to_file_entity_2', FALSE)) {
    return TRUE;
  }
  $entity_info = entity_get_info('file');
  $types = file_type_load_all();
  foreach ($types as $file_type) {
    $view_modes = array('default' => array('label' => t('Default'))) + $entity_info['view modes'];
    foreach (array_keys($view_modes) as $view_mode) {
      $current_displays = file_displays_load($file_type->type, $view_mode, TRUE);
      if (isset($current_displays['file_picture']) && $current_displays['file_picture']->status) {
        if (!isset($current_displays['file_field_picture']) || !$current_displays['file_field_picture']->status) {
          $display = file_display_new($file_type->type, $view_mode, 'file_field_picture');
          $display->settings = $current_displays['file_picture']->settings;
          unset($display->settings['alt']);
          unset($display->settings['title']);
          $display->settings['image_link'] = '';
          $display->settings['colorbox'] = isset($display->settings['picture_mapping']) ? $display->settings['picture_mapping'] : $display->settings['picture_group'];
          $display->settings['picture_mapping'] = isset($display->settings['picture_mapping']) ? $display->settings['picture_mapping'] : $display->settings['picture_group'];
          unset($display->settings['picture_group']);
          $display->status = 1;
          file_display_save($display);
        }
      }
      elseif (isset($current_displays['file_field_picture'])) {
        $display = $current_displays['file_field_picture'];
        $display->settings['picture_mapping'] = isset($display->settings['picture_mapping']) ? $display->settings['picture_mapping'] : $display->settings['picture_group'];
        unset($display->settings['picture_group']);
        file_display_save($display);
      }
    }
  }
  return TRUE;
}

/**
 * Sort picture groups for the CKEditor image dialog.
 */
function picture_compare_weights($a, $b) {
  if ($a['weight'] == $b['weight']) {
    return 0;
  }
  return ($a['weight'] < $b['weight']) ? -1 : 1;
}
