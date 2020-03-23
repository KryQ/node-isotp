const testAddon = require('./build/Release/canlistener.node');

var isotp = new testAddon.IsotpWrapper("can0")

isotp.read(function (data) {
  console.log(data);
}, 0x321, 0x123);


//setInterval(() => isotp.send("lol2", 0x321, 0x123), 1000)