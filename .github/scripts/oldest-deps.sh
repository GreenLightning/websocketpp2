#!/bin/bash
# Build and test against the oldest supported dependencies: Boost 1.66.0,
# standalone Asio 1.12.0 and CMake 3.18. Meant to run as root inside an
# ubuntu:18.04 container (GCC 7.5, OpenSSL 1.1.1), with the repository as the
# working directory:
#
#   docker run --rm -v "$PWD":/src -w /src ubuntu:18.04 \
#       .github/scripts/oldest-deps.sh <boost|standalone>
#
# Downloads and the Boost build are kept in $DEPS_DIR (default /deps), so
# mounting that directory as a volume makes later runs fast.
set -euo pipefail

backend="${1:?usage: $0 <boost|standalone>}"
deps="${DEPS_DIR:-/deps}"
jobs="${JOBS:-$(nproc)}"

boost_version=1.66.0
boost_sha256=5721818253e6a0989583192f96782c4a98eb6204965316df9f5ad75819225ca9
asio_version=1.12.0
asio_sha256=fa8c3a16dc2163f5b3451f2a14ce95277c971f46700497d4e94af6059c00dc06
cmake_version=3.18.6
cmake_sha256=87136646867ed65e935d6bacd44d52a740c448ad0806f6897d8c3d47ce438c8b

export DEBIAN_FRONTEND=noninteractive
apt-get update -q
apt-get install -y -q --no-install-recommends \
    g++ make ca-certificates curl bzip2 libssl-dev zlib1g-dev

fetch() { # url sha256 file
    if [ ! -f "$3" ]; then
        curl -fsSL --retry 3 -o "$3.part" "$1"
        mv "$3.part" "$3"
    fi
    echo "$2  $3" | sha256sum -c -
}

mkdir -p "$deps"

cmake_dir="$deps/cmake-$cmake_version-Linux-x86_64"
if [ ! -x "$cmake_dir/bin/cmake" ]; then
    fetch "https://github.com/Kitware/CMake/releases/download/v$cmake_version/cmake-$cmake_version-Linux-x86_64.tar.gz" \
        "$cmake_sha256" "$deps/cmake.tar.gz"
    tar -xzf "$deps/cmake.tar.gz" -C "$deps"
    rm "$deps/cmake.tar.gz"
fi
export PATH="$cmake_dir/bin:$PATH"

# Only Boost.Test and Boost.System are compiled, the rest is header-only.
boost_dir="$deps/boost-$boost_version"
if [ ! -f "$boost_dir/.complete" ]; then
    boost_name="boost_${boost_version//./_}"
    fetch "https://archives.boost.io/release/$boost_version/source/$boost_name.tar.bz2" \
        "$boost_sha256" "$deps/boost.tar.bz2"
    rm -rf "$deps/$boost_name" "$boost_dir"
    tar -xjf "$deps/boost.tar.bz2" -C "$deps"
    (
        cd "$deps/$boost_name"
        ./bootstrap.sh --with-libraries=test,system --prefix="$boost_dir"
        ./b2 -j"$jobs" -d0 variant=release link=shared threading=multi install
    )
    rm -rf "$deps/$boost_name" "$deps/boost.tar.bz2"
    touch "$boost_dir/.complete"
fi

cmake_args=()
if [ "$backend" = standalone ]; then
    asio_dir="$deps/asio-asio-${asio_version//./-}"
    if [ ! -f "$asio_dir/asio/include/asio.hpp" ]; then
        fetch "https://github.com/chriskohlhoff/asio/archive/refs/tags/asio-${asio_version//./-}.tar.gz" \
            "$asio_sha256" "$deps/asio.tar.gz"
        tar -xzf "$deps/asio.tar.gz" -C "$deps"
        rm "$deps/asio.tar.gz"
    fi
    cmake_args+=(-DASIO_STANDALONE=ON "-DASIO_ROOT=$asio_dir/asio")
elif [ "$backend" != boost ]; then
    echo "unknown backend: $backend" >&2
    exit 2
fi

build="${BUILD_DIR:-/tmp/build-$backend}"
cmake --version
g++ --version
cmake -S . -B "$build" -DCMAKE_BUILD_TYPE=Debug \
    -DBUILD_TESTS=ON -DBUILD_EXAMPLES=ON \
    -DBOOST_ROOT="$boost_dir" -DBoost_NO_SYSTEM_PATHS=ON \
    "${cmake_args[@]}"
cmake --build "$build" -- -j"$jobs"
cd "$build"
LD_LIBRARY_PATH="$boost_dir/lib" ctest --output-on-failure -j"$jobs"
