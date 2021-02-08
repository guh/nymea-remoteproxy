include(../nymea-remoteproxy.pri)

TEMPLATE = lib
TARGET = nymea-remoteproxy

HEADERS += \
    engine.h \
    loggingcategories.h \
    transportinterface.h \
    websocketserver.h \
    proxyclient.h \
    proxyserver.h \
    monitorserver.h \
    proxyconfiguration.h \
    tunnelconnection.h \
    jsonrpcserver.h \
    jsonrpc/jsonhandler.h \
    jsonrpc/jsonreply.h \
    jsonrpc/jsontypes.h \
    jsonrpc/authenticationhandler.h \
    authentication/authenticator.h \
    authentication/authenticationreply.h \
    authentication/dummy/dummyauthenticator.h \
    authentication/aws/awsauthenticator.h \
    authentication/aws/userinformation.h \
    authentication/aws/authenticationprocess.h \
    authentication/aws/sigv4utils.h \
    authentication/aws/awscredentialprovider.h \
    logengine.h

SOURCES += \
    engine.cpp \
    loggingcategories.cpp \
    transportinterface.cpp \
    websocketserver.cpp \
    proxyclient.cpp \
    proxyserver.cpp \
    monitorserver.cpp \
    proxyconfiguration.cpp \
    tunnelconnection.cpp \
    jsonrpcserver.cpp \
    jsonrpc/jsonhandler.cpp \
    jsonrpc/jsonreply.cpp \
    jsonrpc/jsontypes.cpp \
    jsonrpc/authenticationhandler.cpp \
    authentication/authenticator.cpp \
    authentication/authenticationreply.cpp \
    authentication/dummy/dummyauthenticator.cpp \
    authentication/aws/awsauthenticator.cpp \
    authentication/aws/userinformation.cpp \
    authentication/aws/authenticationprocess.cpp \
    authentication/aws/sigv4utils.cpp \
    authentication/aws/awscredentialprovider.cpp \
    logengine.cpp


# install header file with relative subdirectory
for(header, HEADERS) {
    path = $$[QT_INSTALL_PREFIX]/include/nymea-remoteproxy/$${dirname(header)}
    eval(headers_$${path}.files += $${header})
    eval(headers_$${path}.path = $${path})
    eval(INSTALLS *= headers_$${path})
}

target.path = $$[QT_INSTALL_LIBS]
INSTALLS += target
