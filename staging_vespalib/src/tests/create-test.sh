#!/bin/sh
# Copyright 2017 Yahoo Holdings. Licensed under the terms of the Apache 2.0 license. See LICENSE in the project root.

gen_project_file() {
    echo "generating '$1' ..."
    echo "APPLICATION ${test}_test"               > $1
    echo "OBJS ${test}_test"                     >> $1
    echo "LIBS vespalib/staging_vespalib"        >> $1
    echo "EXTERNALLIBS vespalib vespalog"        >> $1
    echo ""                                      >> $1
    echo "CUSTOMMAKE"                            >> $1
    echo "test: depend ${test}_test"             >> $1
    echo -e "\t@./${test}_test"                  >> $1
}

gen_source() {
    echo "generating '$1' ..."
    echo "#include <vespa/vespalib/testkit/testapp.h>" > $1
    echo "#include <vespa/log/log.h>"            >> $1
    echo "LOG_SETUP(\"${test}_test\");"          >> $1
    echo ""                                      >> $1
    echo "using namespace vespalib;"             >> $1
    echo ""                                      >> $1
    echo "TEST_SETUP(Test);"                     >> $1
    echo ""                                      >> $1
    echo "int"                                   >> $1
    echo "Test::Main()"                          >> $1
    echo "{"                                     >> $1
    echo "    TEST_INIT(\"${test}_test\");"      >> $1
    echo "    TEST_DONE();"                      >> $1
    echo "}"                                     >> $1
}

gen_desc() {
    echo "generating '$1' ..."
    echo "$test test. Take a look at ${test}_test.cpp for details." > $1
}

gen_file_list() {
    echo "generating '$1' ..."
    echo "${test}_test.cpp" > $1
}

if [ $# -ne 1 ]; then
	echo "usage: $0 <name>"
	echo "  name: name of the test to create"
	exit 1
fi

test=$1
if [ -e $test ]; then
    echo "$test already present, don't want to mess it up..."
    exit 1
fi

echo "creating directory '$test' ..."
mkdir -p $test || exit 1
cd $test || exit 1
test=`basename $test`

gen_project_file fastos.project
gen_source       ${test}_test.cpp
gen_desc         DESC
gen_file_list    FILES
