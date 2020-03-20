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

Isotp::Isotp ( std::string can ) {
  this->can_interface = can;
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

  return this->socket_id;
}

int Isotp::send(const char* buf, uint32_t len, uint32_t tx_id, uint32_t rx_id) {
  int sock_id = this->connect(this->can_interface, tx_id, rx_id);
  if(sock_id<0) {
    perror("connect");
    return -1;
  }

  int retval = write(sock_id, buf, len);
  if (retval < 0) {
    close(sock_id);
    perror("send");
  }

  close(sock_id);
  return retval;
}

int Isotp::single_read(char* buf, uint32_t tx_id, uint32_t rx_id) {
  int sock_id = this->connect(this->can_interface, tx_id, rx_id);
  if(sock_id<0) {
    perror("connect");
    return -1;
  }

  int retval = ::read(sock_id, buf, BUFSIZE);
  if (retval < 0) {
    close(sock_id);
    perror("read");
  }
  
  close(sock_id);
  return retval;
}

int Isotp::read(char* buf, int sock_id) {
  int retval = ::read(sock_id, buf, BUFSIZE);
  if (retval < 0) {
    perror("read");
  }
  
  return retval;
}