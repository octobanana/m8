# M8
A find and replace string macro processor.  
M8 is a framework for building macros that match a defined regex, run a c++ function, and replace the macro call with the resulting output string.  

## Building
The following shell commands will build the project:
```bash
$ git clone --recurse-submodules <repo_name>
$ cd <repo_name>
$ ./build.sh -r
```
The release build will be located at './build/release/app'.  
To build the debug version, run the build script without the -r flag.  
To install, run the install script.  

## Usage
Let's create a new macro that will output a c++ comment block with the authors name, timestamp, version number and description.  
Create a new file called test.cc:
```cpp
// test.cc

// this is how we will call the macro
#[M8[comment_header(0.1.0, "octobanana", "this is an example of using m8")]]

int main()
{
  return 0;
}
```

Now let's define our macro.  
The `./src/m8.hh` file is where macros are defined.  
Add our macro to the macro map data structure:
```cpp
// macro map
// (name) -> (info, usage, regex, fn)
std::map<std::string, macro> M8 {
{"comment_header",
  {"outputs the authors name, timestamp, version, and description in a c++ comment block",
    "comment_header(version, author, description)",
    "^comment_header\\(([.0-9]+), \"(.+)\", \"().+)\"\\)$",
    fn_comment_header}},
};
```

Now to write the function.  
```cpp
// match[0] is the full matched regex string
// match[1] is the captured integer value for version
// match[2] is the captured string value for author
// match[3] is the captured string value for description
auto const fn_comment_header = [](auto& res, auto const& match) {
  std::stringstream ss;
  ss << "// timestamp:   " << std::time(nullptr) << "\n";
  ss << "// Version:     " << match[1] << "\n";
  ss << "// Author:      " << match[2] << "\n";
  ss << "// Description: " << match[3];
  res = ss.str();
  return 0;
};
```
The response string will replace the calling macro.  
Building and running the example on `test.cc` will output:
```cpp
// test.cc

// this is how we will call the macro
// timestamp:   1517960547
// Version:     0.1.0
// Author:      octobanana
// Description: this is an example of using m8

int main()
{
  return 0;
}
```

## Examples
After building and installing M8, try running it on the `./examples/basic/src/main_m8.cc` file.  
```bash
cd ./examples/basic/src
m8 -f main_m8.cc -o main.cc
```
See `./src/m8.hh` for examples of a macro that repeats a given string n number of times, and a macro that takes in a type and outputs a templated add function.  
