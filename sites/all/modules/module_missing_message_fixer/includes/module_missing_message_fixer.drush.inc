<?php
/**
 * @file
 * Provide Drush integration for release building and dependency building.
 */

/**
 * Helper function to check for modules to fix.
 *
 * @param bool $return
 *        If we are to return to rows or just print the list.
 *
 * @return array[]|null
 *   An array of table rows, or NULL if $return === FALSE.
 */
function module_missing_message_fixer_check_modules($return = FALSE) {

  if ($return) {
    return _module_missing_message_fixer_get_table_rows();
  }

  $rows = array();
  // Use a key for the head row that is not a valid module name.
  $rows['*HEAD*'] = _module_missing_message_fixer_get_table_header();
  $rows += _module_missing_message_fixer_get_table_rows();

  // Print Table here instead of in the hook_command.
  $output = count($rows) > 1 ? drush_format_table($rows, TRUE) : 'No Missing Modules Found!!!';
  drush_print($output);

  return NULL;
}

/**
 * Implements hook_drush_help().
 *
 * @param string $section
 *
 * @return null|string
 */
function module_missing_message_fixer_drush_help($section) {
  switch ($section) {
    case 'module-missing-message-fixer-list':
      return dt("Returns a list of modules that have missing messages.");
    case 'module-missing-message-fixer-fix':
      return dt("Fixes a specified module that has missing messages. (optional --all)");
    default:
      return NULL;
  }
}

/**
 * Implements hook_drush_command().
 */
function module_missing_message_fixer_drush_command() {
  $items = array();
  $items['module-missing-message-fixer-list'] = array(
    'description' => dt('Returns a list of modules that have missing messages.'),
    'aliases' => array(
      'mmmfl',
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL
  );
  $items['module-missing-message-fixer-fix'] = array(
    'description' => dt('Fixes modules that have missing messages.'),
    'aliases' => array(
      'mmmff',
    ),
    'arguments' => array(
      'filename' => 'The filename of the module to fix.',
    ),
    'options' => array(
      'all' => dt('Fixes all module missing messages'),
    ),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUPAL_FULL
  );
  return $items;
}

/**
 * Drush command.
 *
 * Displays a list of modules that have missing messages.
 */
function drush_module_missing_message_fixer_list() {
  module_missing_message_fixer_check_modules();
}

/**
 * Drush command.
 *
 * @param string $filename
 *        The filename of the module to fix messages for.
 */
function drush_module_missing_message_fixer_fix($filename = NULL) {
  $modules = array();
  $rows = module_missing_message_fixer_check_modules(TRUE);
  if (drush_get_option('all') !== NULL) {
    if (!empty($rows)) {
      foreach ($rows as $row) {
        $modules[] = $row['filename'];
      }
    }
  }
  elseif ($filename !== NULL) {
    foreach ($rows as $row) {
      if ($row['filename'] == $filename) {
        $modules[] = $row['filename'];
      }
    }
    if (empty($modules)) {
      drush_log(dt('Module ' . $filename . ' was not found.'), 'error');
    }
  }
  else {
    drush_log(dt('Missing input, provide module filename or run with --all'), 'error');
  }
  // Delete if there is no modules.
  if (count($modules) > 0) {
    db_delete('system')
      ->condition('filename', $modules, 'IN')
      ->execute();
    if (drush_get_option('all') !== NULL) {
      drush_log(dt('All missing references have been removed.'), 'success');
    }
    elseif ($filename !== NULL) {
      if (in_array($filename, $modules, TRUE)) {
        drush_log(dt('Reference to ' . $filename . ' (if found) has been removed.'), 'success');
      }
    }
  }
}
