all : rawdraw_android

#for X11 consider:             xorg-dev
#for X11, you will need:       libx-dev
#for full screen you'll need:  libxinerama-dev libxext-dev
#for OGL You'll need:          mesa-common-dev libglu1-mesa-dev

#-DRASTERIZER
#  and
#-CNFGOGL
#  are incompatible.

SDK:=$$HOME/Android/Sdk
NDK:=$(SDK)/ndk-bundle
CFLAGS:=-Os -DCNFGGLES -DANDROID -Irawdraw -I$(NDK)/sources/android/native_app_glue -I$(NDK)/sysroot/usr/include -I$(NDK)/sysroot/usr/include/android
LDFLAGS:= -lm -Ltoolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/aarch64-linux-android/24 -lGLESv3 -lEGL -u ANativeActivity_onCreate -landroid -llog -s
LDFLAGS += -shared
CC:=$(NDK)/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android28-clang

rawdraw_android : test.c rawdraw/CNFGFunctions.c rawdraw/CNFGEGLDriver.c rawdraw/CNFG3D.c ~/Android/Sdk/ndk-bundle/sources/android/native_app_glue/android_native_app_glue.c
	$(CC)  $(CFLAGS) -o $@ $^ $(LDFLAGS)

burn : rawdraw_android
	adb push rawdraw_android /data/
	adb shell -x "/data/rawdraw_android"
clean : 
	rm -rf *.o *~ rawdraw.exe ontop rawdraw_ogl rawdraw_mac rawdraw_mac_soft rawdraw_mac_cg rawdraw_mac_ogl ogltest ogltest.exe rawdraw_egl rawdraw_android

