#include "http.hh"

#include <curl/curl.h>

#include <cstdlib>
#include <string>
#include <vector>
#include <map>

// debug
// #include <cstdio>

Http::Http()
{
}

Http::~Http()
{
}

size_t Http::cb_header(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  Res *res = static_cast<Res*>(userp);

  std::string buf {static_cast<char*>(contents), realsize};
  res->headers.emplace_back(buf);

  return realsize;
}

size_t Http::cb_write(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  Res *res = static_cast<Res*>(userp);

  std::string buf {static_cast<char*>(contents), realsize};
  res->body += buf;

  return realsize;
}

int Http::run()
{
  // debug
  // std::printf("url: %s\n", req.url.c_str());
  // std::printf("method: %s\n", req.method.c_str());
  // std::printf("data: %s\n", req.data.c_str());
  // for (auto const &e : req.headers)
  // {
  //   std::printf("header: %s\n", e.c_str());
  // }

  CURL *curl_handle;
  CURLcode ec;

  curl_global_init(CURL_GLOBAL_ALL);
  curl_handle = curl_easy_init();

  if (req.method == "POST")
  {
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, req.data.c_str());
  }

  if (! req.headers.empty())
  {
    struct curl_slist *headers = NULL;
    for (auto const &e : req.headers)
    {
      headers = curl_slist_append(headers, e.c_str());
    }
    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
  }

  curl_easy_setopt(curl_handle, CURLOPT_URL, req.url.c_str());
  curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "OB v0.1.0");

  curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, 30L);
  curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, 30L);

  curl_easy_setopt(curl_handle, CURLOPT_HEADERFUNCTION, Http::cb_header);
  curl_easy_setopt(curl_handle, CURLOPT_HEADERDATA, static_cast<void*>(&res));

  curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, Http::cb_write);
  curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, static_cast<void*>(&res));

  ec = curl_easy_perform(curl_handle);

  int code = 0;
  if(ec != CURLE_OK)
  {
    code = -1;
  }

  curl_easy_cleanup(curl_handle);
  curl_global_cleanup();

  // set http status code
  if (res.headers.empty())
  {
    res.status = -1;
  }
  else
  {
    // debug
    // fprintf(stderr, "DEBUG: header[0] -> %s\n", res.headers.at(0).c_str());
    char status[4] = "\0";
    std::snprintf(status, 4, "%.3s", res.headers.at(0).c_str() + 9);
    res.status = std::atoi(status);
    // debug
    // fprintf(stderr, "DEBUG: status -> %d\n", res.status);
  }

  return code;
}
