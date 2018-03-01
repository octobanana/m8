#[M8[license ("MIT", "your_name_here", "2018")]]

#include <string>
#include <iostream>

#[M8[add int]]

#[M8[add std::string]]

#[M8[main (

  auto cat1 = add(5, 6);
  auto cat2 = add("hello", " world");

  std::string repeated {"#[M8[repeat (- EOF, 16)]]"};

  std::string env_home {"#[M8[env HOME]]"};

  std::string sh_example {R"STR(#[M8[sh (printf "Hello, M8!" DONE)]])STR"};

  std::string file_contents {R"STR(#[M8[file ~/.m8.json]])STR"};

  std::string my_ip {R"(#[M8[sh (printf "%s" $(curl -s http://ip.jsontest.com/) DONE)]])"};

  std::string external_macro {R"(#[M8[printf "hello %s" "world" EOF]])"};

  return 0;
)EOF]]
