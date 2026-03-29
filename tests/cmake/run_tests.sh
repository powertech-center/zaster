#!/bin/sh
set -e

WORKSPACE="/workspace"
TEST_DIR="$WORKSPACE/tests/cmake"
PASS=0
FAIL=0
TOTAL=0

report() {
    TOTAL=$((TOTAL + 1))
    if [ "$1" = "0" ]; then
        PASS=$((PASS + 1))
        printf "\033[32m  PASS\033[0m: %s\n" "$2"
    else
        FAIL=$((FAIL + 1))
        printf "\033[31m  FAIL\033[0m: %s\n" "$2"
    fi
}

run_cmake_test() {
    test_name="$1"
    shift
    build_dir="$TEST_DIR/.build/$test_name"

    printf "\n\033[36m=== %s ===\033[0m\n" "$test_name"
    rm -rf "$build_dir"
    mkdir -p "$build_dir"

    cmake_args="-DZASTER_ROOT=$WORKSPACE"
    for arg in "$@"; do
        cmake_args="$cmake_args $arg"
    done

    echo "  cmake $cmake_args -S $TEST_DIR -B $build_dir"
    if cmake $cmake_args -S "$TEST_DIR" -B "$build_dir" 2>&1; then
        report 0 "$test_name: configure"
    else
        report 1 "$test_name: configure"
        return
    fi

    if cmake --build "$build_dir" 2>&1; then
        report 0 "$test_name: build"
    else
        report 1 "$test_name: build"
        return
    fi

    for exe in "$build_dir"/test_*; do
        [ -x "$exe" ] || continue
        exe_name=$(basename "$exe")
        echo "  Running: $exe_name"
        if "$exe" 2>&1; then
            report 0 "$test_name: run $exe_name"
        else
            report 1 "$test_name: run $exe_name"
        fi
    done
}

echo "======================================"
echo "  Zaster CMakeLists.txt Test Suite"
echo "======================================"

# Test 1: Prebuilt + auto-detect target (native = x86_64-linux-musl in Alpine)
run_cmake_test "prebuilt_autodetect"

# Test 2: Prebuilt + explicit ZASTER_TARGET
run_cmake_test "prebuilt_explicit_target" "-DZASTER_TARGET=x86_64-linux-musl"

# Test 3: ZASTER_LINK_ZSTD=OFF
run_cmake_test "no_zstd" "-DZASTER_LINK_ZSTD=OFF"

# Test 4: Source build (rename prebuilt temporarily)
echo ""
echo "--- Preparing source build test (hiding prebuilt) ---"
mv "$WORKSPACE/prebuilt" "$WORKSPACE/prebuilt_bak"

run_cmake_test "source_build" "-DZASTER_TARGET=x86_64-linux-musl"

mv "$WORKSPACE/prebuilt_bak" "$WORKSPACE/prebuilt"
echo "--- Restored prebuilt ---"

# Test 5: Source build without zstd (no prebuilt)
echo ""
echo "--- Preparing source build no-zstd test (hiding prebuilt) ---"
mv "$WORKSPACE/prebuilt" "$WORKSPACE/prebuilt_bak"

run_cmake_test "source_build_no_zstd" "-DZASTER_TARGET=x86_64-linux-musl" "-DZASTER_LINK_ZSTD=OFF"

mv "$WORKSPACE/prebuilt_bak" "$WORKSPACE/prebuilt"
echo "--- Restored prebuilt ---"

# Test 6: Cross-compile configure test (aarch64-linux-musl prebuilt, configure only)
printf "\n\033[36m=== cross_aarch64_configure ===\033[0m\n"
build_dir="$TEST_DIR/.build/cross_aarch64_configure"
rm -rf "$build_dir"
mkdir -p "$build_dir"
if cmake -DZASTER_ROOT="$WORKSPACE" \
         -DZASTER_TARGET=aarch64-linux-musl \
         -DCMAKE_C_COMPILER=clang-aarch64-linux-musl \
         -DCMAKE_CXX_COMPILER=clang-aarch64-linux-musl \
         -S "$TEST_DIR" -B "$build_dir" 2>&1; then
    report 0 "cross_aarch64_configure: configure"
else
    report 1 "cross_aarch64_configure: configure"
fi

# Test 7: Verify target auto-detection with cross-compiler triple
printf "\n\033[36m=== auto_detect_cross_triple ===\033[0m\n"
build_dir="$TEST_DIR/.build/auto_detect_cross_triple"
rm -rf "$build_dir"
mkdir -p "$build_dir"
cmake_output=$(cmake -DZASTER_ROOT="$WORKSPACE" \
                     -DCMAKE_C_COMPILER=clang-x86_64-linux-gnu \
                     -S "$TEST_DIR" -B "$build_dir" 2>&1) || true
echo "$cmake_output"
if echo "$cmake_output" | grep -q "ZASTER_TARGET = x86_64-linux-gnu"; then
    report 0 "auto_detect_cross_triple: detected x86_64-linux-gnu"
else
    report 1 "auto_detect_cross_triple: expected x86_64-linux-gnu"
fi

echo ""
echo "======================================"
printf "  Results: %d passed, %d failed, %d total\n" "$PASS" "$FAIL" "$TOTAL"
echo "======================================"

if [ "$FAIL" -gt 0 ]; then
    exit 1
fi
