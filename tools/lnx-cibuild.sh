#!/bin/bash

# get some infos from git to embed it in the build name
export SOURCE_DIRECTORY=`pwd`
mkdir _build

# define the build name
if [ "$COVERITY_SCAN_BRANCH" == "1" ]; then
  exit 0
elif [ "$TRAVIS_PULL_REQUEST" != "false" ]; then
  export BUILD_NAME=$TRAVIS_PULL_REQUEST
elif [[ -n "$TRAVIS_COMMIT_RANGE" ]]; then
  export BUILD_NAME=$TRAVIS_COMMIT_RANGE
else
  export BUILD_NAME=$TRAVIS_COMMIT
fi

ctest -VV -S tools/lnx-cibuild.cmake

# we indicate build failures if ctest experienced any errors
if [ -f ${SOURCE_DIRECTORY}/failed ]; then
	echo "Configure/Build failed"
  exit -1
fi

FAILED_TEST=$(find _build -name "Test.xml" -type f | xargs grep "<Test Status=\"failed\">" -c)

if [ "$FAILED_TEST" -gt "0" ]; then
	echo "$FAILED_TEST tests failed"
	exit -1
fi

# it seems like everything worked
exit 0
