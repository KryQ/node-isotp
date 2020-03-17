const testAddon = require('./build/Release/canlistener.node');
const EventEmitter = require('events').EventEmitter
const emitter = new EventEmitter()

emitter.on('startedReading', (evt) => {
  console.log(evt);
})

emitter.on('created', () => {
  console.log("created");
})

emitter.on('data', (evt) => {
  console.log(evt);
})

var test = new testAddon.IsotpWrapper("can0", 0x123, 0x123, emitter.emit.bind(emitter));

test.read(function (data) {
  emitter.emit("data", data);
}, 5);

setInterval(() => test.send("lol"), 1000)