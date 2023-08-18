#ifndef __YHTTP_H_
#define __YHTTP_H_

#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <openssl/crypto.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <iostream>
#include <locale>
#include <mutex>
#include <regex>
#include <string_view>
#include <thread>
#include <unordered_map>
enum HEADER {
  Medthod,
  Url,
  Protocol,
  Host,
  Connection,
  Useragent,
  Langeage,
  Status,
  Result
};

enum WRONG {
  InvalidSocket,
  ConnectFail,
  ConnectOutTime,
  WrongUrl,
  SUCCESS,
  WriteFail,
  ReadFail,
  SSLFail,
};

using socket_t = std::int32_t;
class CORE {
 private:
  SSL_CTX *ctx_;
  bool IsVerify;
  SSL *ssl;
  socket_t socketfd = -1;

 public:
  CORE();
  virtual ~CORE();
  std::mutex ctx_mutex;
  bool Connect(struct addrinfo &ai, time_t sec, time_t usec) noexcept;
  bool Creat(std::string_view Host, std::uint16_t Port,
             std::uint32_t AddressFamily, std::uint32_t SocketFlags, time_t sec,
             time_t usec) noexcept;
  bool Init(std::string_view Host) noexcept;
  void SetVerify(bool isverify) noexcept { IsVerify = isverify; };
  bool Wait(time_t sec, time_t usec, bool WriteAble = true,
            bool ReadAble = true) noexcept;
  bool Write(std::string_view data) noexcept;
  bool Read(char *data, std::size_t &number) noexcept;

  virtual bool Send(std::string_view Data = "") noexcept = 0;
  virtual bool Get() noexcept = 0;
};

CORE::CORE() {
  /* Initialize the SSL libraries*/
  OPENSSL_init_ssl(
      OPENSSL_INIT_LOAD_SSL_STRINGS | OPENSSL_INIT_LOAD_CRYPTO_STRINGS, NULL);
  /*Create a new context block*/
  ctx_ = SSL_CTX_new(TLS_client_method());
}
CORE::~CORE() {
  SSL_shutdown(ssl);
  SSL_free(ssl);
  close(socketfd);
  SSL_CTX_free(ctx_);
}
bool CORE::Init(std::string_view Host) noexcept {
  ssl = nullptr;
  {
    std::lock_guard<std::mutex> guard(ctx_mutex);
    ssl = SSL_new(ctx_);
  }
  if (ssl) {
    if (1 != SSL_set_fd(ssl, socketfd) ||
        1 != SSL_set_tlsext_host_name(ssl, Host.data())) {
      return false;
    }
    int err = 0;
    // 等待TLS/SSL客户机发起TLS/SSL握手
    err = SSL_connect(ssl);
    if (1 != err) {
      err = SSL_get_error(ssl, err);
      SSL_shutdown(ssl);
      {
        std::lock_guard<std::mutex> guard(ctx_mutex);
        SSL_free(ssl);
      }
      return false;
    }
    return true;
  } else {
    return false;
  }
}
bool CORE::Write(std::string_view data) noexcept {
  auto length = data.size();
  std::uint16_t count = 1000;
  int err;
  while (count > 0 && (err = SSL_write(ssl, data.data(), length)) < 0) {
    err = SSL_get_error(ssl, err);
    if (SSL_ERROR_WANT_WRITE != err) {
      return false;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  };
  if (0 == count || err < 0) {
    return false;
  }

  return true;
}
bool CORE::Read(char *data, std::size_t &number) noexcept {
  std::int64_t err;
  try {
    err = SSL_read(ssl, data, number);
  } catch (const std::invalid_argument &e) {
    std::cout << e.what();
    return false;
  } catch (const std::logic_error &e) {
    std::cout << e.what();
    return false;
  } catch (...) {
    return false;
  }
  if (err < 1) {
    err = SSL_get_error(ssl, err);
    return false;
  }
  return true;
}
bool CORE::Creat(std::string_view Host, std::uint16_t Port,
                 std::uint32_t AddressFamily, std::uint32_t SocketFlags,
                 time_t sec, time_t usec) noexcept {
  if (socketfd > 0) {
    close(socketfd);
  }
  AI_ALL;
  struct addrinfo hints;
  struct addrinfo *result;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = 0;
  hints.ai_family = AddressFamily;
  hints.ai_flags = SocketFlags;
  if (getaddrinfo(Host.data(), std::to_string(Port).data(), &hints, &result)) {
    return false;
  }
  // 1、 创建套接字
  for (auto rp = result; rp; rp = rp->ai_next) {
    socketfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (socketfd < 0) {
      continue;
    }
    if (fcntl(socketfd, F_SETFD, FD_CLOEXEC) == -1) {
      close(socketfd);
      continue;
    }
    int yes = 1;
    if (setsockopt(socketfd, IPPROTO_TCP, TCP_NODELAY,
                   reinterpret_cast<char *>(&yes), sizeof(yes)) == -1) {
      close(socketfd);
      continue;
    };

    if (AF_INET6 == rp->ai_family) {
      int no = 0;
      if (setsockopt(socketfd, IPPROTO_IPV6, IPV6_V6ONLY,
                     reinterpret_cast<char *>(&no), sizeof(no)) == -1) {
        close(socketfd);
        continue;
      };
    }
    if (!Connect(*rp, sec, usec)) {
      close(socketfd);
      continue;
    }

    freeaddrinfo(result);
    return true;
  }
  freeaddrinfo(result);
  return false;
}

bool CORE::Connect(struct addrinfo &ai, time_t sec, time_t usec) noexcept {
  // 设置超时时间
  timeval tv;
  tv.tv_sec = static_cast<long>(sec);
  tv.tv_usec = static_cast<decltype(tv.tv_usec)>(usec);
  if (setsockopt(socketfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(tv)) <
      0) {
    return false;
  }

  if (setsockopt(socketfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv, sizeof(tv)) <
      0) {
    return false;
  }

  if (connect(socketfd, ai.ai_addr, static_cast<socklen_t>(ai.ai_addrlen)) <
      0) {
    if (errno != EINPROGRESS) {
      return false;
    }
  }
  if (!Wait(sec, usec, false, false)) {
    return false;
  }
  return true;
}

bool CORE::Wait(time_t sec, time_t usec, bool WriteAble,
                bool ReadAble) noexcept {
  if (socketfd >= FD_SETSIZE) {
    return false;
  }
  fd_set fdsr;
  FD_ZERO(&fdsr);
  FD_SET(socketfd, &fdsr);

  auto fdsw = fdsr;
  auto fdse = fdsr;

  timeval tv;
  tv.tv_sec = static_cast<long>(sec);
  tv.tv_usec = static_cast<decltype(tv.tv_usec)>(usec);

  if (select(static_cast<int>(socketfd + 1), &fdsr, &fdsw, nullptr, &tv) <= 0) {
    return false;
  }
  bool mReadAble = FD_ISSET(socketfd, &fdsr);
  bool mWriteAble = FD_ISSET(socketfd, &fdsw);

  if (mReadAble || mWriteAble) {
    int error = 0;
    socklen_t len = sizeof(error);
    auto res = getsockopt(socketfd, SOL_SOCKET, SO_ERROR,
                          reinterpret_cast<char *>(&error), &len);
    if (res < 0 || 0 != error) {
      return false;
    }
  } else {
    return false;
  }

  if (WriteAble && !mWriteAble) {
    return false;
  }
  if (ReadAble && !mReadAble) {
    return false;
  }
  return true;
}

class HTTP : public CORE {
 private:
  std::unordered_map<HEADER, std::string> Request;
  std::unordered_map<std::string, std::string> Response;
  std::uint64_t port;

 public:
  std::string RawResponse;
  HTTP(std::string url = "", std::string protocol = "HTTP/1.1",
       bool connection = true, std::uint64_t mport = 80,
       std::string useragent = "mhttplib");
  WRONG ParseUrl(std::string url) noexcept;
  bool Send(std::string_view Data = "") noexcept;
  bool Get() noexcept;
};

HTTP::HTTP(std::string url, std::string protocol, bool connection,
           std::uint64_t mport, std::string useragent) {
  Request[HEADER::Langeage] = std::locale("").name();
  Request[HEADER::Protocol] = protocol;
  Request[HEADER::Connection] = connection ? "keep-alive" : "close";
  Request[HEADER::Useragent] = useragent;
  port = mport;
  if ("" != url && WRONG::WrongUrl == ParseUrl(url)) {
    return;
  };
};
WRONG HTTP::ParseUrl(std::string url) noexcept {
  std::smatch base_match;
  std::regex UrlRregex(
      "(^https?|ftp|file)://[-A-Za-z0-9+&@#%?=~_|!:,.;]+",
      std::regex_constants::ECMAScript | std::regex_constants::icase);
  if (std::regex_search(url, base_match, UrlRregex)) {
    std::string temp = base_match[0].str();
    if ("https" == temp.substr(0, 5)) {
      Request[HEADER::Host] = temp.substr(8, temp.size() - 7);
      port = 443;
    } else {
      Request[HEADER::Host] = temp.substr(7, temp.size() - 6);
    }
    url.erase(0, temp.size());
    Request[HEADER::Url] = url;
  } else {
    return WRONG::WrongUrl;
  }
  return WRONG::SUCCESS;
}
bool HTTP::Get() noexcept {
  Response.clear();
  char readdata[1024 * 2];
  std::size_t readdatasize = 1024 * 2;
  if (!Read(readdata, readdatasize)) {
    return false;
  }
  std::smatch base_match;
  std::string ResponseHear(readdata);
  std::regex ResponseRregex(
      "(HTTPs?/)[\\.\\d\\s\\w]+\r\n",
      std::regex_constants::ECMAScript | std::regex_constants::icase);
  if (std::regex_search(ResponseHear, base_match, ResponseRregex)) {
    std::string temp = base_match[0].str();
    auto pos = temp.find_first_of(' ');
    Response["Protocol"] = temp.substr(0, pos);
    temp.erase(0, pos + 1);
    pos = temp.find_first_of(' ');
    Response["Status"] = temp.substr(0, pos);
    temp.erase(0, pos + 1);
    Response["Result"] = temp.substr(0, 2);
  } else {
    return false;
  }
  auto Headerpos = ResponseHear.find("\r\n\r\n");
  if (std::string::npos != Headerpos) {
    RawResponse = std::string(readdata);
    RawResponse.resize(Headerpos);
    std::uint64_t pos = ResponseHear.find("\r\n");
    while (pos < Headerpos) {
      auto firstpos = ResponseHear.find(": ", pos + 2);
      auto secondpos = ResponseHear.find("\r\n", firstpos);
      if (std::string::npos != firstpos && std::string::npos != firstpos) {
        Response[ResponseHear.substr(pos + 2, firstpos - pos - 2)] =
            ResponseHear.substr(firstpos + 2, secondpos - firstpos - 2);
      }
      pos = ResponseHear.find("\r\n", secondpos);
    };
  }
  return true;
}

bool HTTP::Send(std::string_view Data) noexcept {
  if ("" == Data) {
    Request[HEADER::Medthod] = "GET";
  } else {
    Request[HEADER::Medthod] = "POST";
  }
  // AF_UNSPEC值表示调用方将仅接受AF_INET和AF_INET6地址系列
  if (!Creat(Request[HEADER::Host], port, AF_UNSPEC, AI_CANONNAME, 10, 0)) {
    return false;
  }
  if (!Wait(10, 0, false, false)) {
    return false;
  }
  if (!Init(Request[HEADER::Host])) {
    return false;
  }
  std::string message;
  try {
    message += Request[HEADER::Medthod] + " " + Request[HEADER::Url] + " " +
               Request[HEADER::Protocol] + "\r\n";
    message += "Accept: " + Request[HEADER::Langeage] + "\r\n";
    message += "Connection: " + Request[HEADER::Connection] + "\r\n";
    message += "Host: " + Request[HEADER::Host] + "\r\n";
    message += "User-Agent: " + Request[HEADER::Useragent] + "\r\n\r\n";
    message += Data.data();
  } catch (const std::invalid_argument &e) {
    std::cout << e.what();
    return false;
  } catch (const std::logic_error &e) {
    std::cout << e.what();
    return false;
  } catch (...) {
    return false;
  }

  if (!Write(message)) {
    return false;
  }
  return true;
}
#endif