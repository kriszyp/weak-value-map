## weak-value-map

A collection of key/value pairs in which the values are weakly referenced.

## Install

```bash
$ npm install weakvaluemap
```

## Usage

Initialization

```js
'use strict';
var WeakValueMap = require('weakvaluemap');

var map = new WeakValueMap();
```

Insertion / Deletion

```js
map.set(1, "abcd")
   .set(2, "efg")
   .set(3, "hijk");

map.delete(2);
```

Retrieval

```js
map.get(1);
// => "abcd"

map.get(2);
// => undefined

map.get(3);
// => "hijk"
```

### Iterate Keys

You can iterate through the keys:
```
for (let key of map.keys()) {
	map.get(key);
}
```
##License

MIT
