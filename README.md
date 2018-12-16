# M8
A general-purpose preprocessor for metaprogramming.

## About
M8 is a command line tool for preprocessing text files.
It executes macros that match a defined regex, running a C++ function or external program, and replaces the macro call with the response string.

## Real World Example
The following is a short illustration of a problem that M8 could help solve.  
An example of a common problem developers face is the handling of secrets within source code, such as api keys. The keys are usually kept out of the version controlled source. After checking out the repo, and obtaining the keys, one could manually copy them into the source code, or read them from a config file at runtime. M8 can help simplify this process.  
An M8 macro can be written to read a key from a shell environment variable, and insert it into the source code at the desired location. The process would be the following, checkout the repo, obtain the secrets as environment variables, and run M8 on the corresponding files. This workflow keeps the secrets seperate, but allows putting it altogether in a working environment quickly.  
There is a builtin macro in M8 that does exactly this. If the environment variable was named __SECRETKEY__, and the destination was a c++ std::string variable, the calling macro would look like the following:  
```cpp
// the environment variable
// SECRETKEY="123456"

// before processing with M8, safe to commit to version control
std::string secret {"[M8[env SECRETKEY]8M]"};

// after processing with M8
std::string secret {"123456"};
```

## Build
### Environment
* Linux (supported)
* BSD (untested)
* macOS (untested)

### Requirements
* C++17 compiler
* CMake >= 3.8
* curl >= 1.67
* OpenSSL >= 1.1.0

### Dependencies
* curl (libcurl)
* crypto (libcrypto)
* pthread (libpthread)
* stdc++fs (libstdc++fs)

### Libraries:
* [parg](https://github.com/octobanana/parg): for parsing CLI args, included as `./src/lib/parg.hh`
* [json](https://github.com/nlohmann/json): for working with JSON, included as `./src/lib/json.hh`
* [cpp-linenoise](https://github.com/yhirose/cpp-linenoise): for interactive line editing, included as `./src/lib/linenoise.hh`

The following shell command will build the project in release mode:
```sh
./build.sh
```
To build in debug mode, run the script with the `--debug` flag.

## Install
The following shell command will install the project in release mode:
```sh
./install.sh
```
To install in debug mode, run the script with the `--debug` flag.

## Config
The configuration file is a json file containing the definitions for external macros. An example config file is located in `./config/m8.json`.  
The default location for the config file is `~/.m8.json`.  

## Syntax
The grammer for a macro is as follows:  
```
macro       : [delim_start][name] [args][delim_end]
delim_start : a unique string of characters | integers | symbols
name        : string of characters naming the macro
args        : the arguments to be passed to the macro
delim_end   : a unique string of characters | integers | symbols
```

The delimiters can be changed with the __--start__ and __--end__ flags at runtime.  
Inbetween the delimiters is the macro name and arguments seperated by a single space character. How the arguments are passed depends on how the macros regex is defined. The regex uses capture groups to parse the arguments sent to the macro.  

An example macro with a regex of `([0-9]+) ([0-9]+)`:
```
macro       : "[[subtract 8 4]]"
delim_start : "[["
name        : "subtract"
args        : "8 4"
delim_end   : "]]"
```

## Usage
After building and installing M8, try running it on the `./examples/basic/src/main_m8.cc` file.
```
cd ./examples/basic/src
m8 main_m8.cc
```

List out built-in macros:
```
m8 --list
```

### Note
When the `-o|--output` option is used, M8 creates a temporary directory called `.m8` in the current working directory to store the output in a temporary file, before renaming to the final file. When done, the `.m8` directory is no longer needed and can be removed. 

## Examples
There are several examples located in the `./example` directory.

## Built-In Macros
A list of some of the builtin/example macros:
* __env__ -> gets an environment variable
* __sh__ -> returns the output of a shell command
* __file__ -> reads in the contents of a file
* __repeat__ -> repeats a given string __n__ number of times

## Extending
There are three types of macros, internal, external, and remote.  

__External__ macros can be written in any language, with their interface defined in m8's json config file.  

__Remote__ macros can be any http server, with their interface defined in m8's json config file as well.  

__Internal__ macros are defined within m8, written in c++.  

### External Program Macro
M8 can run external programs, so macros can be written in any language.  
Adding external macros do not require any recompiling.  
To add a new external macro, define a new macro object in the config file.  
```json
{
  "macros": [
    {
      "name": "program_name",
      "info": "describe what the macro does",
      "usage": "program_name <args>",
      "regex": "^(.*)$"
    }
  ]
}
```
Requirements of an external program:
* should exist and be in your shell path
* able to parse and read the captured regex arguments that are sent to it
* print the output string to stdout

### Remote Program Macro
Remote macros are defined similar to an external macro.
```json
{
  "macros": [
    {
      "name": "program_name",
      "info": "describe what the macro does",
      "usage": "program_name <args>",
      "regex": "^(.*)$",
      "url": "http://127.0.0.1:8080"
    }
  ]
}
```
The difference between defining an external and remote macro is the addition of the url parameter.  
The url should point to an http/https server that can handle the defined macro.  
When a remote macro runs, it sends a json object to the remote server.
The json request sent to the server will contain the following:  
```json
{
  "name": "program_name",
  "args": [
    "the full matched regex",
    "regex capture group 1",
    "regex capture group 2",
    "regex capture group 3",
    ...
  ]
}
```
The server should read the name and check if it exists, if it does, process the apropriate macro call, reading the arguments if necessary from the array of strings called args. Lastly, send back the response string in plain text in the body. Any other status code than 200 will be interpreted as an error.  

Requirements of a server:
* accept a json object
* parse and run corresponding macro call
* return response string

### Internal C++ Macro
Let's create a new macro that will output a c++ comment block with the authors name, timestamp, version number and description.  
The file can be any file type, I'll use c++ as an example.  

Create a new file called test.cc:
```cpp
// test.cc

// this is how we will call the macro
[M8[comment_header 0.1.0, "octobanana", "this is an example of using m8"]8M]

int main()
{
  return 0;
}
```

Now let's define our macro.  
The `./src/m8/macros_custom.cc` file is where custom macros can be defined.
```cpp
void macros(M8& m8)
{
  // define macros
  // M8::set_macro(name, info, usage, regex, func)

  m8.set_macro("comment_header",
    "outputs the authors name, timestamp, version, and description in a c++ comment block",
    "comment_header version, author, description",
    "^([.0-9]+?), \"(.+?)\", \"(.+?)\"$",
    fn_comment_header);
}
```

Now to write the lambda function.  
```cpp
// ctx -> a struct containing the following:
//   str -> the response string
//   args -> an array containing the captured arg values

// based off of the above regex:
// ctx.args.at(0) is the full matched regex string
// ctx.args.at(1) is the captured integer value for version
// ctx.args.at(2) is the captured string value for author
// ctx.args.at(3) is the captured string value for description

auto const fn_comment_header = [](auto& ctx) {
  std::stringstream ss; ss
  << "// timestamp   : " << std::time(nullptr) << "\n"
  << "// Version     : " << ctx.args.at(1) << "\n"
  << "// Author      : " << ctx.args.at(2) << "\n"
  << "// Description : " << ctx.args.at(3);

  // set the response string
  ctx.str = ss.str();

  //  0 = success
  // -1 = error
  return 0;
};
```

The response string will replace the calling macro.  
Building and running the example on `test.cc` will output:
```cpp
// test.cc

// this is how we will call the macro
// timestamp   : 1517960547
// Version     : 0.1.0
// Author      : octobanana
// Description : this is an example of using m8

int main()
{
  return 0;
}
```

## License
This project is licensed under the MIT License.

Copyright (c) 2018 [Brett Robinson](https://octobanana.com/)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
