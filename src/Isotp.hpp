#include <cstdint>
#include <string>
#include <linux/can.h>

class Isotp {
  public:
    Isotp ( std::string can, uint32_t tx_id, uint32_t rx_id );
    int connect(std::string can, uint32_t tx_id, uint32_t rx_id);
    int send(const char* buf, uint32_t len);
    int read(char* buf);

  private:
    const static uint16_t BUFSIZE = 4096;

    int socket_id;
    struct sockaddr_can socket_addr;
    unsigned char tx_buf[BUFSIZE];
    unsigned char rx_buf[BUFSIZE];
};