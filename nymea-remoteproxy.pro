include(nymea-remoteproxy.pri)

TEMPLATE=subdirs
SUBDIRS += server client libnymea-remoteproxy libnymea-remoteproxyclient tests

!disablemonitor {
    SUBDIRS+=monitor
}

server.depends = libnymea-remoteproxy
client.depends = libnymea-remoteproxyclient
tests.depends = libnymea-remoteproxy libnymea-remoteproxyclient

test.commands = LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:$$top_builddir/libnymea-remoteproxy:$$top_builddir/libnymea-remoteproxyclient \
                LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:$$top_srcdir/libnymea-remoteproxy:$$top_srcdir/libnymea-remoteproxyclient \
                make check
QMAKE_EXTRA_TARGETS += test

message("----------------------------------------------------------")
message("Building nymea-remoteproxy $${SERVER_VERSION}")
message("----------------------------------------------------------")
message("JSON-RPC API version $${API_VERSION_MAJOR}.$${API_VERSION_MINOR}")
message("Qt version:" $$[QT_VERSION])

coverage { message("Building with coverage report") }
ccache { message("Building with ccache support") }
disablemonitor {
    message("Building without the monitor")
}
