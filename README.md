# M8
A find and replace string macro processor.  

### About
M8 is a framework for building macros that match a defined regex, run a c++ function or an external program, and replace the macro call with the resulting output string.  
It enables the ability to generate code from code. Macros can also take a macro as an argument, so the macros can generate code for a macro that will generate code. It's macros all the way down.  

### Real World Example
The following is a short illustration of a problem that M8 could help solve.  
An example of a common problem developers face is the handling of secrets within source code, such as api keys. The keys are usually kept out of the version controlled source. After checking out the repo, and obtaining the keys, one could manually copy them into the source code, or read them from a config file at runtime. M8 can help simplify this process.  
An M8 macro can be written to read a key from a shell environment variable, and insert it into the source code at the desired location. The process would be the following, checkout the repo, obtain the secrets as environment variables, and run M8 on the corresponding files. This workflow keeps the secrets seperate, but allows putting it altogether in a working environment quickly.  
There is a builtin macro in M8 that does exactly this. If the environment variable was named __SECRETKEY__, and the destination was a c++ std::string variable, the calling macro would look like the following:  
```cpp
// the environment variable
// SECRETKEY="123456"

// before running the macro, safe to commit to version control
std::string secret {"#[M8[env SECRETKEY]]"};

// after running the macro
std::string secret {"123456"};
```

## Build
Environment:  
* tested on linux
* c++ 14 compiler
* cmake

Libraries:  
* my [parg](https://github.com/octobanana/parg) library, for parsing cli args, included as `./src/parg.hh`
* nlohmann's [json](https://github.com/nlohmann/json) library, for parsing the config file, included as `./src/json.hh`

The following shell commands will build the project:  
```bash
git clone <repo_name>
cd <repo_name>
./build.sh -r
```
To build the debug version, run the build script without the -r flag.  

## Install
The following shell commands will install the project:  
```bash
./install.sh -r
```

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

An example macro with a regex of "([0-9]+) ([0-9]+)":  
```
macro       : "[[subtract 8 4]]"
delim_start : "[["
name        : "subtract"
args        : "8 4"
delim_end   : "]]"
```

## Usage
After building and installing M8, try running it on the `./examples/basic/src/main_m8.cc` file.  
```bash
cd ./examples/basic/src
m8 -f main_m8.cc -o main.cc
```

List out all available macros:  
```bash
m8 --list
```

## Examples
See `./src/macros.cc` for examples.  
A list of some of the builtin/example macros:  
* __env__ -> gets an environment variable
* __sh__ -> returns the output of a shell command
* __file__ -> reads in the contents of a file
* __license__ -> insert a templated source license header
* __repeat__ -> repeats a given string _n_ number of times

## Extending

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

### Internal C++ Macro
Let's create a new macro that will output a c++ comment block with the authors name, timestamp, version number and description.  
The file can be any file type, I'll use c++ as an example.  
Create a new file called test.cc:  
```cpp
// test.cc

// this is how we will call the macro
#[M8[comment_header 0.1.0, "octobanana", "this is an example of using m8"]]

int main()
{
  return 0;
}
```

Now let's define our macro.  
The `./src/macros.cc` file is where macros are defined.  
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
// res -> a std::string reference to the output string
// match -> a std::smatch to the matching regex search

// based off of the above regex:
// match[0] is the full matched regex string
// match[1] is the captured integer value for version
// match[2] is the captured string value for author
// match[3] is the captured string value for description

auto const fn_comment_header = [](auto& res, auto const& match) {
  std::stringstream ss; ss
  << "// timestamp   : " << std::time(nullptr) << "\n"
  << "// Version     : " << match[1] << "\n"
  << "// Author      : " << match[2] << "\n"
  << "// Description : " << match[3];
  res = ss.str();

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
