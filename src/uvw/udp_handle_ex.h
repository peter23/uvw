
//namespace uvw - class udp_handle

private:

  static void recv_raw_callback(uv_udp_t *hndl, ssize_t nread, const uv_buf_t *buf, const sockaddr *addr, unsigned flags);


public:

  //- allows to set null addr (it is required when socket is connected)
  //- allows to send several buffers
  int send_ex(const sockaddr* addr, vector_of_bufs&& data);


  //returns raw sockaddr of sender
  int recv_raw();
