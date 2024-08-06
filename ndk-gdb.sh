#!/bin/bash
#
# Automate the process of attaching gdb to a process running on an android
# device or emulator.
# YMMV; only tested with a single android device - some commands herein may
# require tweaking.
#
# Heavily based on ndk-gdb.py from google.
#
# REF: https://android.googlesource.com/platform/development/+/refs/heads/main/python-packages/gdbrunner/gdbrunner/__init__.py
# REF: https://android.googlesource.com/platform/development/+/46e268e/python-packages/adb/device.py
# REF: https://source.android.com/docs/core/tests/debug/gdb
#
# DEPENDENCIES:
# - gdb-multiarch
#   TODO: Use the ndk gdb, and/or check if the system gdb supports the necessary ABI
#
# TODO:
# - Move part or all content into Makefile
#   Some of the user required envvars are duplicates; it makes sense to define
#   things in one place only.

# WARNING: The following MUST be set properly before running
APPNAME=cnfgtest
PACKAGENAME=org.yourorg.$APPNAME
NDK=
ACTIVITY=android.app.NativeActivity
APP_ABI=arm64-v8a
GDB_SCRIPT=./gdb-setup
SO_DIR=./makecapk/lib
declare -a SHARED_OBJECTS=(lib${APPNAME}.so)

main() {
    existence_checks

    local device_id=$(adb devices | sed -n '2 p' | cut -f1 | tr -d '\n')
    if [ -z "$device_id" ]; then
        error "Could not find a unique connected device/emulator"
    fi

    local adb_cmd="adb -s $device_id"
    local gdb_cmd="gdb-multiarch --tui"
    local make_cmd=$(find $NDK -iname 'make')
    # TODO: Handle busybox as well as toybox versions of ps
    # See get_processes() at the below reference.
    # REF: https://android.googlesource.com/platform/development/+/refs/heads/main/python-packages/gdbrunner/gdbrunner/__init__.py
    local ps_cmd="ps -A"
    local am_cmd="am start"
    # TODO: Allow user to set this
    local run_as_cmd="run-as"

    local adb_version=$($adb_cmd version | sed -z 's/\n/ | /g')
    local app_data_dir=$($adb_cmd shell $run_as_cmd $PACKAGENAME 'pwd')
    local android_tmpdir="/data/local/tmp"

    # TODO: Add support for old abi props
    # ro.product.cpu.abi and ro.product.cpu.abi2
    declare -a device_abis=($($adb_cmd shell getprop ro.product.cpu.abilist | sed 's/,/ /g'))

    local remote_socket=${app_data_dir}/debug_socket
    local tmp_remote_path=${android_tmpdir}/gdbserver

    # Variables with defaults
    local HOST_PORT="${HOST_PORT:=31337}"
    # Default waiting time after launching the requested process before
    # attempting to get its PID.
    local WAIT="${WAIT:=0.25}"
    local GDB_TIMEOUT="${GDB_TIMEOUT:=5}"

    # Set later
    local ZYGOTE_PATH="${ZYGOTE_PATH:-}"
    local SYSROOT="${SYSROOT:-}"
    local APP64="${APP64:-}"
    local TARGET_PID="${TARGET_PID:-}"

    yama_warning

    local device_abi=$(choose_abi)
    local arch=$(abi_to_arch "$device_abi")
    local app_is_64bit=0
    if [[ $device_abi == *64* ]]; then
        app_is_64bit=1
        APP64=64
    fi

    local gdbserver_local_path="$NDK/prebuilt/android-${arch}/gdbserver/gdbserver"
    local gdbserver_remote_path=${app_data_dir}/gdbserver
    if [ ! -f "$gdbserver_local_path" ]; then
        error "Cannot find gdbserver: $gdbserver_local_path"
    fi

    # I believe we could technically set SYSROOT arbitrarily. The main reason
    # I see this approach as being useful is for being able to debug multiple
    # devices with different ABIs
    # NOTE: If I understand correctly, build-local.mk will try to match the
    # requested ABI against one from a certain project file; the simplest
    # option appeared to be jni/Android.mk (I believe used as a last resort).
    # The easiest solution I've found is to just create that file and use it
    # for a single variable declaration that matches our APP_ABI
    mkdir -p jni
    echo "APP_ABI := $APP_ABI" > jni/Android.mk
    local SYSROOT=$($make_cmd --no-print-dir -f ${NDK}/build/core/build-local.mk \
        -C $PWD DUMP_TARGET_OUT APP_ABI=$device_abi)
    local abi_so_dir=${SO_DIR}/${device_abi}

    local ZYGOTE_PATH=
    if [ $app_is_64bit -eq 1 ]; then
        ZYGOTE_PATH=${SYSROOT}/system/bin/app_process64
    else
        ZYGOTE_PATH=${SYSROOT}/system/bin/app_process
    fi

    echo "Found device:     $device_id"
    echo "ADB version:      $adb_version"
    echo "Arch:             $arch"
    echo "Device ABI's:     ${device_abis[*]}"
    echo "Chosen ABI:       $device_abi"
    echo "Target package:   $PACKAGENAME"
    echo "Target activity:  $ACTIVITY"
    echo "SYSROOT:          $SYSROOT"
    echo "Zygote path:      $ZYGOTE_PATH"
    echo "Local SOs:        $abi_so_dir"
    echo "App data:         $app_data_dir"

    # DEBUG
    echo "make_cmd:         $make_cmd"
    echo "Host port:        $HOST_PORT"
    echo "Target socket:    $remote_socket"

    mkdir -p $SYSROOT

    # Copy local shared objects so they may be found easily by gdb
    for so in "${SHARED_OBJECTS[@]}"; do
        echo "==== Copying ${abi_so_dir}/$so in ${SYSROOT}/"
        cp ${abi_so_dir}/$so ${SYSROOT}/$so
    done

    # Pull linker, zygote, and notable system libraries
    # TODO: Also pull project-specific system libraries ?
    pull_binaries $SYSROOT $app_is_64bit

    # TODO: Only push gdbserver over if it's not present on the device
    echo "=== Pushing gdbserver binary to target..."
    $adb_cmd push $gdbserver_local_path $tmp_remote_path

    echo "=== Copying gdbserver binary to app data..."
    $adb_cmd shell "cat $tmp_remote_path | $run_as_cmd $PACKAGENAME sh -c 'cat > $gdbserver_remote_path'"
    # TODO: Error handling
    $adb_cmd shell $run_as_cmd $PACKAGENAME chmod 700 $gdbserver_remote_path
    # TODO: Error handling

    # TODO: Allow to force kill any running instance of PACKAGENAME and/or gdbserver
    # TODO: Allow attaching to an existing process
    # Note: We do not expect jdb to be present on the system
    # TODO: If jdb is present, use append -D to am_cmd and add the necessary
    # to our host-side gdb init script. See ${GDB_SCRIPT}.template

    # NOTE: We are not interested in the usual stdout content for launching
    # the process. However, we do want to make use of anything output to
    # stderr.
    echo "=== Launching activity ${PACKAGENAME}/${ACTIVITY}..."
    local target_err=$($adb_cmd shell $am_cmd ${PACKAGENAME}/${ACTIVITY} 2>&1 1>/dev/null | grep "Error type" | cut -d' ' -f3)
    # NOTE: Conversely if this has no value it means the process ran
    # successfully
    if [ ! -z "$target_err" ]; then
        error "Failed to start the activity: $target_err"
    fi

    # Allow the process to start before we try to grab its PID
    sleep $WAIT

    # TODO: Handle potential for multiple PIDs ?
    local TARGET_PID=$($adb_cmd shell "$ps_cmd | grep $PACKAGENAME | cut -F2")
    echo "Target PID:       $TARGET_PID"

    local gdbserver_log="${android_tmpdir}/gdbserver.log"
    echo "=== Starting gdbserver on target..."
    echo "    Redirecting output to $gdbserver_log"
    echo "    Use \`adb shell tail -f $gdbserver_log\` to monitor"
    # NOTE: Unlike other uses of adb shell <cmd> this one seems to require
    # being run and detached
    # We log stdout and stderr to preserve its output
    ($adb_cmd shell $run_as_cmd $PACKAGENAME \
        "$gdbserver_remote_path --once +$remote_socket --attach $TARGET_PID \
        1>$gdbserver_log 2>&1") &
    # TODO: Error handling

    echo "=== Forwarding host port to target socket..."
    $adb_cmd forward tcp:$HOST_PORT localfilesystem:$remote_socket

    echo "=== Generating gdb script..."
    PACKAGENAME=$PACKAGENAME \
        ACTIVITY=$ACTIVITY \
        ABI=$APP_ABI \
        NDK=$NDK \
        ZYGOTE_PATH=$ZYGOTE_PATH \
        SYSROOT=$SYSROOT \
        APP64=$APP64 \
        TARGET_PID=$TARGET_PID \
        HOST_PORT=$HOST_PORT \
        WAIT=$WAIT \
        GDB_TIMEOUT=$GDB_TIMEOUT \
        envsubst '$NDK $SYSROOT $APP64 $ZYGOTE_PATH $HOST_PORT $GDB_TIMEOUT TARGET_PID' \
            < ${GDB_SCRIPT}.template > $GDB_SCRIPT

    echo "=== Running gdb..."
    # NOTE: Not using the ndk-provided version; we currently expect a system-wide install
    $gdb_cmd -x $GDB_SCRIPT
}

# Exit early if these requirements are not met.
existence_checks() {
    # The google script automatically parses certain variables from the
    # manifest, but we require them be hardcoded instead.
    # TODO: Should we also parse the xml? Doesn't feel too worth it
    #local project_dir=$PWD
    #local manifest_filename="AndroidManifest.xml"
    #if [ ! -f "${project_dir}/${manifest_filename}" ]; then
    #    error "Could not find $manifest_filename in current directory"
    #fi

    if [ ! -f ${GDB_SCRIPT}.template ]; then
        error "${GDB_SCRIPT}.template is required"
    fi
    if [ -z ${PACKAGENAME+x} ]; then
        error "The PACKAGENAME environment variable MUST be set (e.g. org.foo.\$APPNAME)"
    fi
    if [ -z ${ACTIVITY+x} ]; then
        error "The ACTIVITY environment variable MUST be set (e.g. android.app.NativeActivity)"
    fi
    # TODO: Detect ABI(s)
    if [ -z ${APP_ABI+x} ]; then
        error "The ABI environment variable MUST be set (e.g. arm64-v8a)"
    fi
    if [ -z ${NDK+x} ]; then
        error "The NDK environment variable MUST be set"
    fi
}

# Expects the following to be properly set:
# - device_abis
# - ABI
choose_abi() {
    local result=""
    local match=0
    for a in "${device_abis[@]}"; do
        # TODO: Allow $APP_ABI to have multiple values
        if [ $a == $APP_ABI ]; then
            result=$a
            match=1
            break
        fi
    done
    if [ "$match" == 0 ]; then
        error "Application cannot run on the selected device"
    fi
    echo $result
}

abi_to_arch() {
    local abi=$1
    if [[ $abi == armeabi* ]]; then
        echo "arm"
    elif [[ $abi == arm64-v8a* ]]; then
        echo "arm64"
    else
        echo "$abi"
    fi
}

yama_warning() {
    # Warn on old Pixel C firmware (b/29381985). Newer devices may have Yama
    # enabled but still work with ndk-gdb (b/19277529).
    # NOTE: Not fully tested but I believe should work as intended.
    local yama_check=$(cat /proc/sys/kernel/yama/ptrace_scope 2>/dev/null)
    local build_product=$($adb_cmd shell getprop ro.build.product)
    local product_name=$($adb_cmd shell getprop ro.product.name)
    if [ ! -z "$yama_check" ] && [ "$yama_check" -ne 0 ] \
            && [ "$build_product" == "dragon" ] && [ "$product_name" == "ryu" ] \
            ; then
        echo "WARNING: The device uses Yama ptrace_scope to restrict debugging. ndk-gdb will"
        echo "    likely be unable to attach to a process. With root access, the restriction"
        echo "    can be lifted by writing 0 to /proc/sys/kernel/yama/ptrace_scope. Consider"
        echo "    upgrading your Pixel C to MXC89L or newer, where Yama is disabled."
    fi
}

pull_binaries() {
    # Technically we don't need these local copies but it helps readability,
    # at least for me
    local sysroot=$1
    local app_is_64bit=$2

    declare -a bins=()
    declare -a libs=(libc.so libm.so libdl.so)
    local lib_path=""

    # NOTE:
    # The following quote is directly from google's ndk-gdb.py.
    # > /system/bin/app_process is 32-bit on 32-bit devices, but a symlink to
    # > app_process64 on 64-bit. If we need the 32-bit version, try to pull
    # > app_process32, and if that fails, pull app_process.
    # While they separate the fetch, they save locally only as app_process,
    # and I don't fully understand why they don't just fetch app_process when
    # it's /known/ that the device is not 64-bit.
    # Unless there's a good reason not to, we shall opt for the simpler
    # choice.
    if [ $app_is_64bit == 1 ]; then
        bins+=(system/bin/app_process64 system/bin/linker64)
        lib_path=system/lib64
    else
        bins+=(system/bin/app_process system/bin/linker)
        lib_path=system/lib
    fi

    for lib in "${libs[@]}"; do
        bins+=(${lib_path}/$lib)
    done

    for bin in "${bins[@]}"; do
        local local_path=${sysroot}/$bin
        local local_dirpath=$(dirname $local_path)

        mkdir -p $local_dirpath

        # NOTE: Unlike the google script we store non-absolute paths then add
        # root (/) here. Just in case anything gets messed up with local_path
        # we'd rather litter files locally than to the root directory.
        echo "=== Pulling $bin to $local_path"
        $adb_cmd pull /$bin $local_path
    done
}

error() {
    local msg=$1
    echo "[ERROR] $msg"
    exit 1
}

main "$@"
