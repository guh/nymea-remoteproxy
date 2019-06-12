#!/bin/sh
SCRIPT_DIR=$(dirname $0)
SRC_DIR="$SCRIPT_DIR/../build-nymea-remoteproxy-Desktop-Debug/"
COV_DIR="$SRC_DIR/coverage"
HTML_RESULTS="${COV_DIR}/html"

# Build code coverage html report
mkdir -p ${HTML_RESULTS}
lcov -d "${SRC_DIR}" -c -o "${COV_DIR}/coverage.info"
lcov -r "${COV_DIR}/coverage.info" "*.h" "*/tests/*" "*.moc" "*moc_*.cpp" "*/test/*" "/usr/include/*" "*/build*/*" "*libnymea-remoteproxy/authentication/aws*" -o "${COV_DIR}/coverage-filtered.info"
genhtml -o "${HTML_RESULTS}" "${COV_DIR}/coverage-filtered.info"
lcov -d "${COV_DIR}" -z
