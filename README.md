# M8
A general-purpose preprocessor for metaprogramming.

## About
M8 is a command line tool for preprocessing text files.
Its syntax is customizable, and easy to distinguish from its surrounding text.
Custom macros can be added, allowing it to be specialized for various uses.
M8 executes macros that match a defined regex, running either a built-in macro,
an external program, or a remote API, and replaces the call point with
the response string.

## A Brief Tour
Use the built-in 'def' macro to define a new macro called 'hello' that has an
argument regex that captures a single word.

The file `hello.m8`:
```
[M8[ def hello 'str' '(\w+)' Hello, {1}! ]8M]
The output: [M8[ hello octobanana ]8M]
```

Processing the file through M8 as `m8 hello.m8` will output:
```
The output: Hello, octobanana!
```

Define a new macro called 'name' that returns a static string and pass it to
the 'hello' macro.

The file `hello.m8`:
```
[M8[ def name octobanana ]8M]
[M8[ def hello 'str' '(\w+)' Hello, {1}! ]8M]
The output: [M8[ hello [M8[ name ]8M] ]8M]
```

Processing the file through M8 as `m8 hello.m8` will output:
```
The output: Hello, octobanana!
```

## Build
### Environment
* Linux (supported)
* BSD (untested)
* macOS (untested)

### Requirements
* C++17 compiler
* CMake >= 3.8
* curl >= 7.61.0
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

The delimiters can be changed with the `--start` and `--end` or `--mirror` flags at runtime.
The `--mirror` option takes a starting delimiter, creating an end delimiter by
mirroring the input. For example, `--mirror '[('` will use `[(` as the start delimiter, and
`)]` as the end delimiter.

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
Show program version.
```
m8 --version'
```

Show program help.
```
m8 --help'
```

Show all built-in macros.
```
m8 --list
```

Show information on a specific built-in macros.
```
m8 --info 'macro-name'
```

Process a file and print the output to stdout.
```
m8 'input-file'
```

Process a file and print the output to stdout, printing a summary out at the
end.
```
m8 'input-file' --summary
```

Process a file and print the output to stdout, printing out to stderr the
amount of time it took to run in seconds.
```
m8 'input-file' --timer
```

Process a file and print the output to stdout, printing debug information to
stderr.
```
m8 'input-file' --debug
```

Process a file and print the output to stdout, using custom delimiters.
```
m8 'input-file' --start '[[' --end ']]'
```

Process a file and print the output to stdout, using custom mirrored delimiter.
```
m8 'input-file' --mirror '[('
```

Process a file and save the output to a file.
```
m8 'input-file' --output 'output-file'
```

Process a file and print the output to a file, while ignoring lines starting
with '//' using the `--comment` option.
```
m8 'input-file' --output 'output-file' --comment '//'
```

### Note
When the `-o|--output` option is used, M8 creates a temporary directory called `.m8` in the current working directory to store the output in a temporary file, before renaming to the final file. When done, the `.m8` directory is no longer needed and can be removed. 

## Examples
There are several examples located in the `./example` directory.

## Built-In Macros
A list of some of the builtin/example macros:
* __def__ -> define a new macro
* __env__ -> get an environment variable
* __sh__ -> execute and return the output of a shell command
* __file__ -> read in the contents of a file
* __repeat__ -> repeat a given string a specific number of times

## Extending
There are three types of macros, internal, external, and remote.
* __External__ macros can be written in any language, with their interface defined in m8's json config file.
* __Remote__ macros can be any http server, with their interface defined in m8's json config file as well.
* __Internal__ macros are defined within m8, written in c++.

### External Program Macro
M8 can run external programs, so macros can be written in any language.  
Adding external macros do not require any recompiling.

To add a new external macro, define a new macro object in the config file:
```json
{
  "macros": [
    {
      "name": "program_name",
      "info": "describe what the macro does",
      "usage": "describe the parameters if any, ex 'name:string age:int'",
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
Remote macros are defined similar to an external macro:
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
The `./src/m8/macros_custom.cc` file is where custom macros can be defined:
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

Now to write the lambda function:
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
