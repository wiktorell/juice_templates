'use strict';

const createPlugin = require('../src/index');

describe('createPlugin', () => {
  it('returns a function', () => {
    const plugin = createPlugin();
    expect(typeof plugin).toBe('function');
  });

  it('inlines CSS from a <style> tag into element style attributes', () => {
    const plugin = createPlugin();
    const html = '<style>p { color: red; }</style><p>Hello</p>';
    const result = plugin(html);
    expect(result).toContain('style="color: red;"');
    expect(result).not.toContain('<style>');
  });

  it('applies extraCss option set at creation time', () => {
    const plugin = createPlugin({ extraCss: 'p { font-size: 14px; }' });
    const result = plugin('<p>Hello</p>');
    expect(result).toContain('font-size: 14px');
  });

  it('allows per-call overrides', () => {
    const plugin = createPlugin({ extraCss: 'p { color: blue; }' });
    const result = plugin('<p>Hello</p>', { extraCss: 'p { color: green; }' });
    expect(result).toContain('color: green');
    expect(result).not.toContain('color: blue');
  });

  it('does not mutate the options object passed to createPlugin', () => {
    const options = { extraCss: 'p { color: red; }' };
    const plugin = createPlugin(options);
    plugin('<p>Hello</p>', { extraCss: 'p { color: blue; }' });
    expect(options.extraCss).toBe('p { color: red; }');
  });

  it('preserves media queries by default', () => {
    const plugin = createPlugin();
    const html =
      '<style>@media (max-width:600px){p{color:blue;}} p{color:red;}</style><p>Hello</p>';
    const result = plugin(html);
    expect(result).toContain('@media');
  });

  it('throws TypeError when html is not a string', () => {
    const plugin = createPlugin();
    expect(() => plugin(null)).toThrow(TypeError);
    expect(() => plugin(123)).toThrow(TypeError);
  });

  it('isolated plugins do not share state', () => {
    const pluginA = createPlugin({ extraCss: 'p { color: red; }' });
    const pluginB = createPlugin({ extraCss: 'p { color: blue; }' });
    const resultA = pluginA('<p>Hello</p>');
    const resultB = pluginB('<p>Hello</p>');
    expect(resultA).toContain('color: red');
    expect(resultB).toContain('color: blue');
  });
});
