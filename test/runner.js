const { JSDOM } = require("jsdom");
const path = require("path");

if (process.argv.length < 3) {
    console.error("Usage: node runner.js <compiled-module.js>");
    process.exit(1);
}

const modulePath = path.resolve(process.argv[2]);
const catchArgs = process.argv.slice(3);

const dom = new JSDOM('<!DOCTYPE html><body></body>');
global.window = dom.window;
global.document = dom.window.document;
global.navigator = dom.window.navigator;

const Module = require(modulePath);
global.Module = Module;

Module({
    arguments: catchArgs
}).then(() => {
    // tests executed through main()
}).catch(err => {
    console.error("Error running test module:", err);
    process.exit(1);
});
