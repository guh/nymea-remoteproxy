include(../nymea-remoteproxy.pri)

TEMPLATE = lib
TARGET = nymea-remoteproxyclient
target.path = $$[QT_INSTALL_LIBS]

include(libnymea-remoteproxyclient.pri)

installheaders.files = remoteproxyconnection.h
installheaders.path = $$[QT_INSTALL_PREFIX]/include/nymea-remoteproxyclient/

INSTALLS += target installheaders
