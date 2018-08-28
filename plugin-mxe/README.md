MXE plugin for Ruby and other Shoes deps that MXE doesn't build 
by default. This is not part of the Official MXE project. 

Install MXE and set PATH. Edit settings.mk for i686-w64-mingw32.shared
See https://mxe.cc/#tutorial

In the mxe directory you can do 
```make MXE_PLUGIN_DIRS=~/path-to-git-clone/shoes3/plugin-mxe shoesdeps```
and it will download and build Gtk3, Ruby 2.3.7, sqlite3, and others
for a complete set of Windows dependecies for cross compiling. 

If you just want to build the Ruby for some reason
```make MXE_PLUGIN_DIRS=~/Projects/shoes3/plugin-mxe ruby```
Note it's not a Windows installer based ruby - it's missing some dlls and
structure for that. You could add them if you know what you are doing.

You can always do the individual steps:

make cc
make zlib 
make bzip2
make libffi
make gtk3
make librsvg
make sqlite
make giflib
make MXE_PLUGIN_DIRS=~/Projects/shoes3/plugin-mxe openssl1.0
make MXE_PLUGIN_DIRS=~/Projects/shoes3/plugin-mxe yaml
make MXE_PLUGIN_DIRS=~/Projects/shoes3/plugin-mxe gdbm
make MXE_PLUGIN_DIRS=~/Projects/shoes3/plugin-mxe ruby

Yes, you can put muliple package names on the command line:
make zlib bzip2 libffi gtk3 librsvg sqlite3
