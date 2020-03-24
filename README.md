# node-isotp

Most rudimentary binding there is
As this is my learning repo i woudn't recommend it for production use.

Library is a wrapper around isotp-can module.

Constructor
isotp.IsotpWrapper(CAN_INTERFACE)

send
isotp.send((string)data, tx_id, rx_id)

read
isotp.read((data, rx, tx) => {}, tx_id, rx_id)
