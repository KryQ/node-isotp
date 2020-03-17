#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/types.h>

#include <sys/ioctl.h>

#include <linux/can/isotp.h>

#include "Isotp.hpp"

#define NO_CAN_ID 0xFFFFFFFFU

Isotp::Isotp ( std::string can, uint32_t tx_id, uint32_t rx_id ) {
  this->connect(can, tx_id, rx_id);
}

int Isotp::connect(std::string can, uint32_t tx_id, uint32_t rx_id) {
  this->socket_addr.can_addr.tp.tx_id = this->socket_addr.can_addr.tp.rx_id = NO_CAN_ID;

  this->socket_addr.can_addr.tp.tx_id = tx_id;
  this->socket_addr.can_addr.tp.rx_id = rx_id;

  if ((this->socket_id = socket(PF_CAN, SOCK_DGRAM, CAN_ISOTP)) < 0) {
    perror("socket");
    return -1;
  }

  this->socket_addr.can_family = AF_CAN;
  this->socket_addr.can_ifindex = if_nametoindex(can.c_str());

  if (bind(this->socket_id, (struct sockaddr *)&this->socket_addr, sizeof(this->socket_addr)) < 0) {
    
    perror("bind");
    close(this->socket_id);
    return -1;
  }

  return 0;
}

int Isotp::send(const char* buf, uint32_t len) {
  std::cout << buf << " len: " << len << std::endl;
  
  int retval = write(this->socket_id, buf, len);
  if (retval < 0) {
    perror("send");
  }
  return retval;
}

int Isotp::read(char* buf) {
  int ret = ::read(this->socket_id, buf, BUFSIZE);
  if (ret < 0) {
    perror("read");
  }
  
  return ret;
}

/*int main(void) {
  char tx_test[100] = "test test test test test test test test test test";
  char rx_test[100];
  Isotp can0 = Isotp("can0", 0x321, 0x123);

  std::cout << "Sended " << can0.send(tx_test, 49) << std::endl;

  int readed = can0.read2(rx_test);

  std::cout << "Readed " << readed << " bytes: " << rx_test << std::endl;
}

*/