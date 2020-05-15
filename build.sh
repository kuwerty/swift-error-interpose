#!/bin/sh -e

xcrun clang -g -O0 -c -o interpose.o interpose.c

# use swiftc to invoke linker so we get correct paths to Swift libraries etc
xcrun swiftc -module-name swiftinterpose -emit-library -o libswiftinterpose-macosx.dylib interpose.o -lswiftCore
