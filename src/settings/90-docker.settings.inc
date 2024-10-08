<?php

/**
 * @file
 * Drupal 8 settings for docker.
 */

use Drupal\Component\Assertion\Handle;

if (SETTINGS_ENVIRONMENT == D_ENVIRONMENT_LOCAL && SETTINGS_PLATFORM == D_PLATFORM_DOCKER) {
  // @codingStandardsIgnoreFile
  if (getenv('LANDO') !== 'ON') {
    return;
  }

  /**
   * Assertions.
   *
   * The Drupal project primarily uses runtime assertions to enforce the
   * expectations of the API by failing when incorrect calls are made by code
   * under development.
   *
   * @see http://php.net/assert
   * @see https://www.drupal.org/node/2492225
   *
   * If you are using PHP 7.0 it is strongly recommended that you set
   * zend.assertions=1 in the PHP.ini file (It cannot be changed from .htaccess
   * or runtime) on development machines and to 0 in production.
   *
   * @see https://wiki.php.net/rfc/expectations
   */
  assert_options(ASSERT_ACTIVE, TRUE);
  Handle::register();

  $lando_info = json_decode(getenv('LANDO_INFO'), TRUE);

  $databases['default']['default'] = array (
    'database' => $lando_info['database']['creds']['database'],
    'username' => $lando_info['database']['creds']['user'],
    'password' => $lando_info['database']['creds']['password'],
    'prefix' => '',
    'host' => $lando_info['database']['internal_connection']['host'],
    'port' => $lando_info['database']['internal_connection']['port'],
    'driver' => 'mysql',
  );

  $config['search_api.server.solr_server_main'] = [
    'backend_config' => [
      'connector_config' => [
        'host' => $lando_info['solr']['internal_connection']['host'],
        'path' => '/',
        'core' => $lando_info['solr']['core'],
        'port' => $lando_info['solr']['internal_connection']['port'],
      ],
    ],
  ];

  $https = TRUE;

  if (!isset($_SERVER['HTTPS']) || $_SERVER['HTTPS'] != 'on') {
    $https = isset($_SERVER['HTTP_X_FORWARDED_PROTO']) && $_SERVER['HTTP_X_FORWARDED_PROTO'] === 'https';
    $_SERVER['HTTPS'] = $https ? 'on' : 'off';
  }

  $base_protocol = $https ? 'https' : 'http';
  $base_url = $base_protocol . '://' . SETTINGS_INSTANCE;

  // File paths...
  $settings['file_temp_path'] = '/tmp';
  $settings['file_public_path'] = 'sites/default/files';
  $settings['file_private_path'] = 'sites/default/files/private';

  // Configure Redis.
  $settings['redis.connection']['host'] = $lando_info['redis']['internal_connection']['host'];
  $settings['redis.connection']['port'] = $lando_info['redis']['internal_connection']['port'];
  $settings['redis.connection']['password'] = '';
  $settings['redis.connection']['base'] = 0;
  // $settings['cache']['default'] = 'cache.backend.redis';

  /**
   * Skip file system permissions hardening.
   *
   * The system module will periodically check the permissions of your site's
   * site directory to ensure that it is not writable by the website user. For
   * sites that are managed with a version control system, this can cause problems
   * when files in that directory such as settings.php are updated, because the
   * user pulling in the changes won't have permissions to modify files in the
   * directory.
   */
  $settings['skip_permissions_hardening'] = TRUE;

  // Use the development service config.
  $settings['container_yamls'][] = DRUPAL_ROOT . '/sites/development.services.yml';

  if (getenv('REDUCE_CACHING')) {
    // Disable some caches on docker development.
    $settings['cache']['bins']['render'] = 'cache.backend.null';
    $settings['cache']['bins']['dynamic_page_cache'] = 'cache.backend.null';
    $settings['cache']['bins']['page'] = 'cache.backend.null';
  }

  /**
   * Show all error messages, with backtrace information.
   *
   * In case the error level could not be fetched from the database, as for
   * example the database connection failed, we rely only on this value.
   */
  $config['system.logging']['error_level'] = 'verbose';

  /**
   * Disable CSS and JS aggregation.
   */
  $config['system.performance']['css']['preprocess'] = FALSE;
  $config['system.performance']['js']['preprocess'] = FALSE;
}
