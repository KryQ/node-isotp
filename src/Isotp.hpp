#include <cstdint>
#include <string>
#include <linux/can.h>

class Isotp {
  public:
    Isotp ( std::string can );
    std::string getInterface() {return this->can_interface;};
    int connect(uint32_t tx_id, uint32_t rx_id);
    int connect(std::string can, uint32_t tx_id, uint32_t rx_id);
    int send(const char* buf, uint32_t len, uint32_t tx_id, uint32_t rx_id);
    int single_read(char* buf, uint32_t tx_id, uint32_t rx_id);
    int read(char* buf, int sock_id);

  private:
    const static uint16_t BUFSIZE = 8192;

    std::string can_interface;
    int socket_id;
    struct sockaddr_can socket_addr;
};