#!/bin/bash

# Export the library path for now
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$(pwd)/libnymea-remoteproxy:$(pwd)/libnymea-remoteproxyclient

# Build
qmake CONFIG+=coverage CONFIG+=ccache
make -j$(nproc)
#make test
make coverage-html

# Clean build
make clean

# Clean source directory
rm -v Makefile

rm -v libnymea-remoteproxy/libnymea-remoteproxy.so*
rm -v libnymea-remoteproxy/Makefile

rm -v libnymea-remoteproxyclient/libnymea-remoteproxyclient.so*
rm -v libnymea-remoteproxyclient/Makefile

rm -v server/nymea-remoteproxy
rm -v server/Makefile

rm -v tests/Makefile

rm -v tests/test-offline/nymea-remoteproxy-tests-offline
rm -v tests/test-offline/Makefile

#rm -v tests/test-online/nymea-remoteproxy-tests-online
#rm -v tests/test-online/Makefile

rm -v client/nymea-remoteproxy-client
rm -v client/Makefile
