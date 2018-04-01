[M8[license ("MIT", "your_name_here", "2018")]8M]

#include <string>
#include <iostream>

[M8[add int]8M]

[M8[add std::string]8M]

[M8[main (
  auto cat1 = add(5, 6);
  auto cat2 = add("hello", " world");

  std::string repeated {"[M8[repeat "-", 16]8M]"};

  std::string env_home {"[M8[env HOME]8M]"};

  std::string sh_example {R"STR([M8[sh (printf "Hello, M8!" DONE)]8M])STR"};

  std::string file_contents {R"STR([M8[file ~/.m8.json]8M])STR"};

  std::string my_ip {R"([[sh (printf "%s" $(curl -s http://ip.jsontest.com/) DONE)]])"};

  std::string external_macro {R"([M8[printf "hello %s" "world" EOF]8M])"};

  return 0;
)EOF]8M]
