
namespace uvw {

UVW_INLINE udp_data_raw_event::udp_data_raw_event(const sockaddr* sndr, std::unique_ptr<char[]> buf, std::size_t len, bool part) noexcept
  : data{std::move(buf)},
    length{len},
    sender{sndr},
    partial{part} {}

UVW_INLINE void details::send_ex_req::udp_send_ex_callback(uv_udp_send_t *req, int status) {
  if(auto ptr = reserve(req); status) {
    ptr->publish(error_event{status});
  } else {
    ptr->publish(send_event{});
  }
}

UVW_INLINE details::send_ex_req::send_ex_req(loop::token token, std::shared_ptr<loop> parent, vector_of_bufs&& dt)
  : request{token, std::move(parent)},
    data{std::move(dt)}
{
  bufs.reserve(data.size());
  for(auto& data1 : data) {
    const auto offs = std::get<2>(data1);
    bufs.emplace_back(uv_buf_init(std::get<0>(data1).get() + offs, std::get<1>(data1) - offs));
  }
}

UVW_INLINE int details::send_ex_req::send_ex(uv_udp_t *hndl, const struct sockaddr *addr) {
  return this->leak_if(uv_udp_send(raw(), hndl, bufs.data(), bufs.size(), addr, &udp_send_ex_callback));
}

UVW_INLINE void udp_handle::recv_raw_callback(uv_udp_t *hndl, ssize_t nread, const uv_buf_t *buf, const sockaddr *addr, unsigned flags) {
  udp_handle &udp = *(static_cast<udp_handle *>(hndl->data));
  // data will be destroyed no matter of what the value of nread is
  std::unique_ptr<char[]> data{buf->base};

  if(nread > 0) {
    // data available (can be truncated)
    udp.publish(udp_data_raw_event{addr, std::move(data), static_cast<std::size_t>(nread), !(0 == (flags & UV_UDP_PARTIAL))});
  } else if(nread == 0 && addr == nullptr) {
    // no more data to be read, doing nothing is fine
  } else if(nread == 0 && addr != nullptr) {
    // empty udp packet
    udp.publish(udp_data_raw_event{addr, std::move(data), static_cast<std::size_t>(nread), false});
  } else {
    // transmission error
    udp.publish(error_event(nread));
  }
}

UVW_INLINE int udp_handle::send_ex(const sockaddr* addr, vector_of_bufs&& data) {
  auto req = parent().resource<details::send_ex_req>(std::move(data));

  auto listener = [ptr = shared_from_this()](const auto &event, const auto &) {
    ptr->publish(event);
  };

  req->on<error_event>(listener);
  req->on<send_event>(listener);

  return req->send_ex(raw(), addr);
}

UVW_INLINE int udp_handle::recv_raw() {
  return uv_udp_recv_start(raw(), &details::common_alloc_callback, &recv_raw_callback);
}

} // namespace uvw
