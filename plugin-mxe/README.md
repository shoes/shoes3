MXE plugin for Ruby and deps that MXE doesn't build

Install MXE and set PATH. See https://mxe.cc/#tutorial

In mxe directory 
make cc
make zlib 
make bzip2
make libffi
make gtk3
make librsvg
make sqlite
make MXE_PLUGIN_DIRS=~/Projects/shoes3/plugin-mxe openssl1.0
make MXE_PLUGIN_DIRS=~/Projects/shoes3/plugin-mxe yaml
make MXE_PLUGIN_DIRS=~/Projects/shoes3/plugin-mxe gdbm
make MXE_PLUGIN_DIRS=~/Projects/shoes3/plugin-mxe ruby

Yes, you could put muliple package names on the command line:
make zlib bzip2 libffi gtk3 librsvg sqlite3
