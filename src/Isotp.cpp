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

#include <sstream>
#include <iomanip>

#define NO_CAN_ID 0xFFFFFFFFU

Isotp::Isotp ( std::string can ) {
  this->can_interface = can;
}

int Isotp::connect(uint32_t tx_id, uint32_t rx_id) {
  return this->connect(this->can_interface, tx_id, rx_id);
}

int Isotp::connect(std::string can, uint32_t tx_id, uint32_t rx_id) {
  this->socket_addr.can_addr.tp.tx_id = this->socket_addr.can_addr.tp.rx_id = NO_CAN_ID;

  this->socket_addr.can_addr.tp.tx_id = tx_id;
  this->socket_addr.can_addr.tp.rx_id = rx_id;

  if ((this->socket_id = socket(PF_CAN, SOCK_DGRAM, CAN_ISOTP)) < 0) {
    return -1;
  }

  static struct can_isotp_fc_options fcopts;
  fcopts.bs = 100;
  fcopts.stmin = 2;
  setsockopt(this->socket_id, SOL_CAN_ISOTP, CAN_ISOTP_RECV_FC, &fcopts, sizeof(fcopts));

  this->socket_addr.can_family = AF_CAN;
  this->socket_addr.can_ifindex = if_nametoindex(can.c_str());

  if (bind(this->socket_id, (struct sockaddr *)&this->socket_addr, sizeof(this->socket_addr)) < 0) {
    close(this->socket_id);
    return -1;
  }

  return this->socket_id;
}

void Isotp::disconnect(int sock_id) {
  close(socket_id);
}

int Isotp::send(const char* buf, uint32_t len, uint32_t tx_id, uint32_t rx_id) {
  int sock_id = this->connect(this->can_interface, tx_id, rx_id);
  if(sock_id<0) {
    return -1;
  }

  int retval = write(sock_id, buf, len);

  close(sock_id);
  return retval;
}

int Isotp::single_read(char* buf, uint32_t tx_id, uint32_t rx_id) {
  int sock_id = this->connect(this->can_interface, tx_id, rx_id);
  if(sock_id<0) {
    return -1;
  }

  int retval = this->read(buf, sock_id);
  
  close(sock_id);
  return retval;
}

int Isotp::read(char* buf, int sock_id) {
  return ::read(sock_id, buf, BUFSIZE);
}

/*void toHex(
    void *const data,           //!< Data to convert
    const size_t dataLength,    //!< Length of the data to convert
    std::string &dest           //!< Destination string
    )
{
    unsigned char     *byteData = reinterpret_cast<unsigned char*>(data);
    std::stringstream hexStringStream;
    
    hexStringStream << std::hex << std::setfill('0');
    for(size_t index = 0; index < dataLength; ++index)
        hexStringStream << std::setw(2) << static_cast<int>(byteData[index]);
    dest = hexStringStream.str();
}

int main () {
  Isotp can = Isotp("can0");
  int sock = can.connect(0x321, 0x123);
  char buff[16384];
  
  std::string resstring;

  while(1) {
    int res = can.read(buff, sock);
    if(res>=0) {
      
      toHex(buff, res, resstring);
      std::cout << resstring << " " << res << std::endl;
    }
    else {
      perror("Err: ");
    }
  }

  std::cout << "hello" << std::endl;
  return 2;
}*/