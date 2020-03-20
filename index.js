const testAddon = require('./build/Release/canlistener.node');
const EventEmitter = require('events').EventEmitter
const emitter = new EventEmitter()

emitter.on('created', () => {
  console.log("created");
})

emitter.on('data', (evt) => {
  console.log("Data:", evt);
})

var test = new testAddon.IsotpWrapper("can0", emitter.emit.bind(emitter));

console.log(test.read(function (data) {
  emitter.emit("data", data);
}, 0x001, 0x002));

console.log(test.read(function (data) {
  emitter.emit("data", data);
}, 0x001, 0x010));

setInterval(() => test.send("lol2", 0x321, 0x123), 1000)