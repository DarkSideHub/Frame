
#include "../src/httplib/yhttp.hpp"
// #include "../src/httplib/httplib.hpp"
#include "gtest/gtest.h"
#define CA_CERT_FILE "./ca-bundle.crt"
class SocketTest : public ::testing::Test {
 public:
  std::string host = "www.bilibili.com";
  std::uint16_t Port = 443;
  std::uint32_t AddressFamily = AF_UNSPEC;
  std::uint32_t SocketFlags = 0;
  std::string url =
      "https://www.bilibili.com/v/popular/all/?spm_id_from=333.1007.0.0";
  std::string url1 = "https://httpbin.org/post";
  std::string data =
      "{\"args\": {},\"data\": \"\",\"files\": {},\"form\": {},\"headers\": "
      "{\"Accept\": \"application/json\",\"Accept-Encoding\": \"gzip, deflate, "
      "br\",\"Accept-Language\": \"zh-CN,zh;q=0.9\",\"Content-Length\": "
      "\"0\",\"Host\": \"httpbin.org\",\"Origin\": "
      "\"https://httpbin.org\",\"Referer\": "
      "\"https://httpbin.org/\",\"Sec-Ch-Ua\": \"\\\"Google "
      "Chrome\\\";v=\\\"117\\\", \\\"Not;A=Brand\\\";v=\\\"8\\\", "
      "\\\"Chromium\\\";v=\\\"117\\\"\",\"Sec-Ch-Ua-Mobile\": "
      "\"?0\",\"Sec-Ch-Ua-Platform\": \"\\\"Linux\\\"\",\"Sec-Fetch-Dest\": "
      "\"empty\",\"Sec-Fetch-Mode\": \"cors\",\"Sec-Fetch-Site\": "
      "\"same-origin\",\"User-Agent\": \"Mozilla/5.0 (X11; Linux x86_64) "
      "AppleWebKit/537.36 (KHTML, like Gecko) Chrome/117.0.0.0 "
      "Safari/537.36\",\"X-Amzn-Trace-Id\": "
      "\"Root=1-64c46896-5a8d980c564e0da8308b02c7\"},\"json\": "
      "null,\"origin\": \"39.185.113.166\",\"url\": "
      "\"https://httpbin.org/post\"}";
};

TEST_F(SocketTest, GetTest) {
  // HTTP
  time_t sec = 10;
  HTTP mhttp(url);
  mhttp.Send();
  mhttp.Get();
}

TEST_F(SocketTest, PostTest) {
  // HTTP
  time_t sec = 10;
  HTTP mhttp(url1);
  mhttp.Send(data);
  mhttp.Get();
}
