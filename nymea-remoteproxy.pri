QT *= network websockets
QT -= gui

# Define versions
SERVER_NAME=nymea-remoteproxy
API_VERSION_MAJOR=0
API_VERSION_MINOR=3
SERVER_VERSION=0.1.8

DEFINES += SERVER_NAME_STRING=\\\"$${SERVER_NAME}\\\" \
           SERVER_VERSION_STRING=\\\"$${SERVER_VERSION}\\\" \
           API_VERSION_STRING=\\\"$${API_VERSION_MAJOR}.$${API_VERSION_MINOR}\\\"

CONFIG += c++11 console

QMAKE_CXXFLAGS *= -Werror -std=c++11 -g
QMAKE_LFLAGS *= -std=c++11

top_srcdir=$$PWD
top_builddir=$$shadowed($$PWD)

ccache {
    QMAKE_CXX = ccache g++
}

coverage {<
    # Note: this works only if you build in the source dir
    OBJECTS_DIR =
    MOC_DIR =

    LIBS += -lgcov
    QMAKE_CXXFLAGS += --coverage
    QMAKE_LDFLAGS += --coverage

    QMAKE_EXTRA_TARGETS += coverage cov
    QMAKE_EXTRA_TARGETS += clean-gcno clean-gcda coverage-html \
        generate-coverage-html clean-coverage-html coverage-gcovr \
        generate-gcovr generate-coverage-gcovr clean-coverage-gcovr

    clean-gcno.commands = \
        "@echo Removing old coverage instrumentation"; \
        "find -name '*.gcno' -print | xargs -r rm"

    clean-gcda.commands = \
        "@echo Removing old coverage results"; \
        "find -name '*.gcda' -print | xargs -r rm"

    coverage-html.depends = clean-gcda check generate-coverage-html

    generate-coverage-html.commands = \
        "echo 'Collecting coverage data'"; \
        "lcov --directory $${top_srcdir} --capture --output-file coverage.info --no-checksum --compat-libtool"; \
        "lcov --extract coverage.info \"*/server/*.cpp\" --extract coverage.info \"*/libnymea-remoteproxy/*.cpp\" --extract coverage.info \"*/libnymea-remoteproxyclient/*.cpp\" -o coverage.info"; \
        "lcov --remove coverage.info \"moc_*.cpp\" --remove coverage.info \"*/test/*\" -o coverage.info"; \
        "LANG=C genhtml --prefix $${top_srcdir} --output-directory coverage-html --title \"nymea-remoteproxy coverage\" --legend --show-details coverage.info"

    clean-coverage-html.depends = clean-gcda
    clean-coverage-html.commands = \
        "echo 'Clean html coverage report'"; \
        "lcov --directory $${top_srcdir} -z"; \
        "rm -rf coverage.info coverage-html"

    coverage-gcovr.depends = clean-gcda check generate-coverage-gcovr

    generate-coverage-gcovr.commands = \
        "echo 'Generating coverage GCOVR report'"; \
        "gcovr -x -r $${top_srcdir} -o $${top_srcdir}/coverage.xml -e \".*/moc_.*\" -e \"tests/.*\" -e \".*\\.h\""

    clean-coverage-gcovr.depends = clean-gcda
    clean-coverage-gcovr.commands = \
        "echo 'Clean coverage report'"; \
        "rm -rf $${top_srcdir}/coverage.xml"

    QMAKE_CLEAN += *.gcda *.gcno coverage.info coverage.xml
}
