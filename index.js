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

var test = new testAddon.IsotpWrapper("can0", 0x321, 0x123, emitter.emit.bind(emitter));
console.log(test.startReading(2))

