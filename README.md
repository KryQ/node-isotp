# node-isotp

Most rudimentary binding there is<br />
As this is my learning repo i woudn't recommend it for production use.

Library is a wrapper around isotp-can module.

to import it:<br />
const { IsotpWrapper } = require('./index');

Constructor<br />
const isotp = new IsotpWrapper(CAN_INTERFACE)

send<br />
isotp.send((string)data, tx_id, rx_id)

read<br />
let handle = isotp.read((data, rx, tx) => {}, tx_id, rx_id)<br />

stop reading<br />
isotp.stopReading(handle)<br />

<a rel="license" href="http://creativecommons.org/licenses/by-nc/4.0/"><img alt="Creative Commons License" style="border-width:0" src="https://i.creativecommons.org/l/by-nc/4.0/88x31.png" /></a><br />This work is licensed under a <a rel="license" href="http://creativecommons.org/licenses/by-nc/4.0/">Creative Commons Attribution-NonCommercial 4.0 International License</a>.
