#include "crypto.hh"

#include <string>
#include <sstream>
#include <iomanip>

#include <openssl/sha.h>

namespace Crypto
{
  std::string sha256(std::string const str)
  {
    unsigned char digest[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(digest, &sha256);
    std::stringstream out;
    for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    {
      out << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(digest[i]);
    }
    return out.str();
  }
} // namespace Crypto
