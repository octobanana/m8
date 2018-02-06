#include <string>
#include <iostream>

#[M8[add(int)]]

#[M8[add(std::string)]]

#[M8[oops(64)]]

int main(int argc, char *argv[])
{
  std::string msg {#[M8[repeat(hello,2)]]};

  auto cat1 = add(5, 6);
  auto cat2 = add(msg, "hi");

  std::cout << "cat1: " << cat1 << "\n";
  std::cout << "cat2: " << cat2 << "\n";

  return 0;
}
