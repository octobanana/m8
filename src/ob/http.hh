#ifndef OB_HTTP_HH
#define OB_HTTP_HH

#include <string>
#include <vector>

class Http
{
public:

  Http();
  ~Http();

  int run();

  struct Req
  {
    std::string method {"GET"};
    std::string url;
    std::vector<std::string> headers;
    std::string data;
  } req;

  struct Res
  {
    int status {0};
    std::vector<std::string> headers;
    std::string body;
  } res;

private:

  static size_t cb_header(void *contents, size_t size, size_t nmemb, void *userp);
  static size_t cb_write(void *contents, size_t size, size_t nmemb, void *userp);
};

#endif // OB_HTTP_HH
