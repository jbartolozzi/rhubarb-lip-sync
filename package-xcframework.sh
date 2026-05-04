#!/usr/bin/env bash
#
# Builds an XCFramework (`Rhubarb.xcframework`) for macOS arm64 + x86_64,
# bundling rhubarb-c plus all its transitive static dependencies into a
# single static archive per slice. Whisper support is disabled.
#
# Output:
#   build-xcframework/Rhubarb.xcframework
#   build-xcframework/RhubarbResources/   (sphinx model + acoustic model)
#
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_ROOT="${REPO_ROOT}/build-xcframework"
ARCHS=(arm64 x86_64)
DEPLOYMENT_TARGET="${MACOSX_DEPLOYMENT_TARGET:-11.0}"

rm -rf "${BUILD_ROOT}"
mkdir -p "${BUILD_ROOT}"

build_arch() {
    local arch="$1"
    local build_dir="${BUILD_ROOT}/${arch}"
    local install_dir="${BUILD_ROOT}/${arch}-install"

    echo "==> Configuring (${arch})"
    cmake -S "${REPO_ROOT}" -B "${build_dir}" \
        -G "Unix Makefiles" \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_OSX_ARCHITECTURES="${arch}" \
        -DCMAKE_OSX_DEPLOYMENT_TARGET="${DEPLOYMENT_TARGET}" \
        -DCMAKE_INSTALL_PREFIX="${install_dir}" \
        -DRHUBARB_BUILD_C_API=ON \
        -DRHUBARB_BUILD_WHISPER=OFF \
        -DRHUBARB_BUILD_PYTHON=OFF

    echo "==> Building rhubarb-c (${arch})"
    cmake --build "${build_dir}" --target rhubarb-c --config Release --parallel

    echo "==> Merging static archives (${arch})"
    local merged="${BUILD_ROOT}/${arch}-librhubarb.a"
    # Collect every .a produced by our build, excluding fetched test deps.
    local archives=()
    while IFS= read -r -d '' a; do
        archives+=("$a")
    done < <(find "${build_dir}" -type f -name '*.a' \
        -not -path '*/_deps/googletest-build/*' \
        -not -name 'libgtest*.a' \
        -not -name 'libgmock*.a' \
        -print0)

    if [ "${#archives[@]}" -eq 0 ]; then
        echo "ERROR: no static archives found in ${build_dir}" >&2
        exit 1
    fi

    libtool -static -o "${merged}" "${archives[@]}" 2>&1 \
        | grep -v "has no symbols" || true
    if [ ! -s "${merged}" ]; then
        echo "ERROR: libtool produced no output for ${arch}" >&2
        exit 1
    fi
    echo "    merged ${#archives[@]} archives -> ${merged}"
}

for arch in "${ARCHS[@]}"; do
    build_arch "${arch}"
done

echo "==> Staging headers"
HEADERS_DIR="${BUILD_ROOT}/Headers"
mkdir -p "${HEADERS_DIR}"
cp "${REPO_ROOT}/rhubarb/src/capi/Rhubarb.h" "${HEADERS_DIR}/"
cp "${REPO_ROOT}/rhubarb/src/capi/module.modulemap" "${HEADERS_DIR}/"

echo "==> Creating macOS universal archive"
UNIVERSAL_DIR="${BUILD_ROOT}/macos-universal"
mkdir -p "${UNIVERSAL_DIR}"
UNIVERSAL_LIB="${UNIVERSAL_DIR}/librhubarb.a"
lipo -create \
    "${BUILD_ROOT}/arm64-librhubarb.a" \
    "${BUILD_ROOT}/x86_64-librhubarb.a" \
    -output "${UNIVERSAL_LIB}"
echo "    archs: $(lipo -archs "${UNIVERSAL_LIB}")"

echo "==> Creating Rhubarb.xcframework"
XCFRAMEWORK="${BUILD_ROOT}/Rhubarb.xcframework"
rm -rf "${XCFRAMEWORK}"
xcodebuild -create-xcframework \
    -library "${UNIVERSAL_LIB}" -headers "${HEADERS_DIR}" \
    -output "${XCFRAMEWORK}"

stage_resources() {
    local target_res_dir="$1"
    rm -rf "${target_res_dir}"
    mkdir -p "${target_res_dir}/sphinx"
    cp -R "${REPO_ROOT}/rhubarb/lib/pocketsphinx-rev13216/model/en-us/" \
        "${target_res_dir}/sphinx/"
    mkdir -p "${target_res_dir}/sphinx/acoustic-model"
    cp -R "${REPO_ROOT}/rhubarb/lib/cmusphinx-en-us-5.2/" \
        "${target_res_dir}/sphinx/acoustic-model/"
}

echo "==> Staging resources (standalone)"
stage_resources "${BUILD_ROOT}/RhubarbResources/res"

echo "==> Populating Swift Package"
SWIFT_PKG="${REPO_ROOT}/swift"
rm -rf "${SWIFT_PKG}/Rhubarb.xcframework"
cp -R "${XCFRAMEWORK}" "${SWIFT_PKG}/Rhubarb.xcframework"
stage_resources "${SWIFT_PKG}/Sources/Rhubarb/Resources/res"

echo
echo "Done."
echo "  XCFramework:    ${XCFRAMEWORK}"
echo "  Resources:      ${BUILD_ROOT}/RhubarbResources/"
echo "  Swift package:  ${SWIFT_PKG}/  (Rhubarb.xcframework + Resources copied in place)"
