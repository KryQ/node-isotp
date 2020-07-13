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

Isotp::Isotp ( std::string can, uint16_t bs, uint16_t stmin ) {
  this->can_interface = can;

  this->bs = bs;
  this->stmin = stmin;
}

int Isotp::connect(uint32_t tx_id, uint32_t rx_id) {
  return this->connect(this->can_interface, tx_id, rx_id);
}

int Isotp::connect(std::string can, uint32_t tx_id, uint32_t rx_id) {
  //std::cout << "[DEBUG] Connecting to socket: " << can << std::endl;
  int sock_id = -1;
  this->socket_addr.can_addr.tp.tx_id = this->socket_addr.can_addr.tp.rx_id = NO_CAN_ID;
  struct ifreq ifr;

  this->socket_addr.can_addr.tp.tx_id = tx_id;
  this->socket_addr.can_addr.tp.rx_id = rx_id;

  if ((sock_id = socket(PF_CAN, SOCK_DGRAM, CAN_ISOTP)) < 0) {
    perror("clol :(");
    return -1;
  }

  can_isotp_fc_options fcopts;
  fcopts.bs = this->bs;
  fcopts.stmin = this->stmin;
  setsockopt(sock_id, SOL_CAN_ISOTP, CAN_ISOTP_RECV_FC, &fcopts, sizeof(fcopts));

  strcpy(ifr.ifr_name, can.c_str());
	ioctl(sock_id, SIOCGIFINDEX, &ifr);

  this->socket_addr.can_family = AF_CAN;
  this->socket_addr.can_ifindex = ifr.ifr_ifindex;

  //std::cout << can.c_str() << " at index " << ifr.ifr_ifindex << "\n";

  if (bind(sock_id, (struct sockaddr *)&this->socket_addr, sizeof(this->socket_addr)) < 0) {
    this->disconnect(sock_id);
    return -1;
  }

  return sock_id;
}

void Isotp::disconnect(int sock_id) {
  //std::cout << "Disconnecting socket" << std::endl;
  close(sock_id);
}

int Isotp::send(const char* buf, uint32_t len, uint32_t tx_id, uint32_t rx_id) {
  int sock_id = this->connect(this->can_interface, tx_id, rx_id);
  if(sock_id<0) {
    perror("cslol :(");
    return -1;
  }

  int retval = write(sock_id, buf, len);

  this->disconnect(sock_id);
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
  Isotp can = Isotp("vcan0", 0, 0);
  //int sock = can.connect(0x321, 0x123);
  char buff[16384];
  
  std::string resstring;

  can.send("lol\0", 3, 0x1, 0x2);

  /*while(1) {
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
}
*/