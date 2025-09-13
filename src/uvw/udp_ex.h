#include <vector>

namespace uvw {

// buffer , length , offset
using one_buf = std::tuple<std::unique_ptr<char[]>, unsigned int, unsigned int>;
using vector_of_bufs = std::vector<one_buf>;

struct udp_data_raw_event {
  explicit udp_data_raw_event(const sockaddr* sndr, std::unique_ptr<char[]> buf, std::size_t len, bool part) noexcept;

  std::unique_ptr<char[]> data; /*!< A bunch of data read on the stream. */
  std::size_t length;           /*!< The amount of data read on the stream. */
  const sockaddr* sender;       /*!< A valid instance of sockaddr. */
  bool partial;                 /*!< True if the message was truncated, false otherwise. */
};

namespace details {

class send_ex_req final: public request<send_ex_req, uv_udp_send_t, send_event> {
  static void udp_send_ex_callback(uv_udp_send_t *req, int status);

public:
  using deleter = void (*)(char *);

  send_ex_req(loop::token token, std::shared_ptr<loop> parent, vector_of_bufs&& dt);

  int send_ex(uv_udp_t *hndl, const struct sockaddr *addr);

private:
  vector_of_bufs data;
  std::vector<uv_buf_t> bufs;
};

} // namespace details

} // namespace uvw
