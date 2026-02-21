# juice-plugin-creator

A factory for creating pre-configured [Juice](https://github.com/Automattic/juice) CSS-inlining plugins.

## Install

```bash
npm install juice-plugin-creator
```

## Usage

```js
const createPlugin = require('juice-plugin-creator');

// Create a plugin with default Juice options
const plugin = createPlugin();
const html = plugin('<style>p { color: red; }</style><p>Hello</p>');
// => '<p style="color: red;">Hello</p>'

// Create a plugin with custom default options
const emailPlugin = createPlugin({
  extraCss: 'body { font-family: sans-serif; }',
  preserveMediaQueries: true,
});

const result = emailPlugin('<p>Hello</p>');

// Per-call overrides are supported
const resultWithOverride = emailPlugin('<p>Hello</p>', { extraCss: 'p { color: blue; }' });
```

## API

### `createPlugin([options])`

Returns a **plugin** function pre-configured with the supplied Juice options.

| Parameter | Type | Description |
|-----------|------|-------------|
| `options` | `object` | Any [Juice option](https://github.com/Automattic/juice#options). Applied as defaults for every call to the plugin. |

### `plugin(html[, overrides])`

Inlines CSS into `html` and returns the resulting HTML string.

| Parameter | Type | Description |
|-----------|------|-------------|
| `html` | `string` | HTML to process. |
| `overrides` | `object` | Per-call Juice options that override the plugin's defaults. |

Throws a `TypeError` if `html` is not a string.

## Testing

```bash
npm test
```

## License

MIT