'use strict';

const fs = require('fs');
const path = require('path');
let WeakValueMap

if (fs.existsSync(path.join(__dirname, 'build/Release/cppmap.node'))) {
    module.exports = WeakValueMap = require('./build/Release/cppmap').WeakValueMap;
} else if (fs.existsSync(path.join(__dirname, 'build/Debug/cppmap.node'))) {
    console.log("weak-value-map loaded debug build");
    module.exports = WeakValueMap = require('./build/Debug/cppmap').WeakValueMap;
} else {
    throw new Error("weak-value-map not built!");
}
WeakValueMap.prototype.keys = function() {
	let keys = this._keysAsArray()
	let i = 0
	let l = keys.length
	return {
		next() {
			return i < l ? {
				value: keys[i++]
			} : {
				done: true
			}
		},
		[Symbol.iterator]() {
			return this
		}
	}
}
