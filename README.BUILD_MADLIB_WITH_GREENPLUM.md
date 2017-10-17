Steps to compile MADlib with greenplum

`MADLIB_SRC_CODE=<location of MADlib source code>`

`MADLIB_BUILD_DIR=$MADLIB_SRC_CODE/build-greenplum-debug` ( you can change this to any other writeable path)

Prerequisites
1. greenplum_path.sh and gpdemo-env.sh(if applicable) should be sourced.
1. greenplum should be up and running on the system.
1. gcc and g++ (For OSX, Clang will work for compiling the source, but not for documentation.)
1. MADlib works with Python 2.6 and 2.7.  Currently, Python 3.x is not supported.
1. cmake (the latest version of cmake might cause issues. Please try cmake 3.5.2 in case you get an error or a segmentation fault.)


Compilation Steps ( you can follow these steps for recompiling MADlib as well):
1. `cd $MADLIB_SRC_CODE`
1. `rm -rf $MADLIB_BUILD_DIR && mkdir $MADLIB_BUILD_DIR && cd $MADLIB_BUILD_DIR`
1. `cmake -DCMAKE_OSX_ARCHITECTURES=x86_64 -DCMAKE_BUILD_TYPE=debug -DCMAKE_CXX_COMPILER_LAUNCHER=ccache $MADLIB_SRC_CODE`
1. `make -j1`
1. `src/bin/madpack -p postgres -c [user[/password]@][host][:port][/database] install`
    1. A shorter version of this command : `src/bin/madpack -p postgres -c /database install`

Running tests
1. `cd $MADLIB_BUILD_DIR`
1. `src/bin/madpack -p postgres -c /database install-check`
1. Running tests for a specific module `src/bin/madpack -p postgres -c /database install-check -t <module_name>`. Module names can be found in $MADLIB_SRC_CODE/src/config/Modules.yml
