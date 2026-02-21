'use strict';

const juice = require('juice');

/**
 * Creates a Juice plugin — a pre-configured function that inlines CSS into HTML.
 *
 * @param {object} [options] - Juice options to apply for every call to the plugin.
 *   See https://github.com/Automattic/juice#options for all available options.
 * @param {string} [options.extraCss=''] - Extra CSS to inject before inlining.
 * @param {boolean} [options.removeStyleTags=true] - Remove <style> tags after inlining.
 * @param {boolean} [options.preserveMediaQueries=true] - Keep @media rules in a <style> tag.
 * @returns {function} plugin - A function `plugin(html[, overrides])` that returns the
 *   processed HTML string with CSS inlined according to the configured options.
 */
function createPlugin(options) {
  const defaultOptions = Object.assign({}, options);

  /**
   * Inline CSS into the given HTML using the plugin's configured options.
   *
   * @param {string} html - HTML string to process.
   * @param {object} [overrides] - Per-call option overrides merged on top of the
   *   plugin's default options.
   * @returns {string} HTML with CSS inlined.
   */
  function plugin(html, overrides) {
    if (typeof html !== 'string') {
      throw new TypeError('html must be a string');
    }
    const callOptions = Object.assign({}, defaultOptions, overrides);
    return juice(html, callOptions);
  }

  return plugin;
}

module.exports = createPlugin;
