all : makecapk.apk 

.PHONY : push run

APPNAME:=cnfgtest
PACKAGENAME:=org.yourorg.$(APPNAME)

ANDROIDVERSION:=24
SDK:=$$HOME/Android/Sdk
NDK:=$(SDK)/ndk/20.0.5594570
BUILD_TOOLS:=$(SDK)/build-tools/29.0.2
#for older SDKs
#NDK:=$(SDK)/ndk-bundle
#BUILD_TOOLS:=$(SDK)/build-tools/28.0.3

CFLAGS:=-Os -DCNFGGLES -DANDROID -DANDROID_FULLSCREEN -DAPPNAME=$(APPNAME)
CFLAGS+= -Irawdraw -I$(NDK)/sysroot/usr/include -I$(NDK)/sysroot/usr/include/android -fPIC -I.
LDFLAGS:= -lm -Ltoolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/aarch64-linux-android/$(ANDROIDVERSION) -lGLESv3 -lEGL -landroid -llog
LDFLAGS += -shared -s -uANativeActivity_onCreate

CC_ARM32:=$(NDK)/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android$(ANDROIDVERSION)-clang
CC_ARM64:=$(NDK)/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi$(ANDROIDVERSION)-clang
CC_x86:=$(NDK)/toolchains/llvm/prebuilt/linux-x86_64/bin/x86_64-linux-android$(ANDROIDVERSION)-clang
CC_x86_64=$(NDK)/toolchains/llvm/prebuilt/linux-x86_64/bin/x86_64-linux-android$(ANDROIDVERSION)-clang
AAPT:=$(BUILD_TOOLS)/aapt

CFLAGS_ARM64:=-m64
CFLAGS_ARM32:=-mfloat-abi=softfp
CFLAGS_x86:=-march=i686 -mtune=intel -mssse3 -mfpmath=sse -m32
CFLAGS_x86_64:=-march=x86-64 -msse4.2 -mpopcnt -m64 -mtune=intel
/home/cnlohr/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android24-clang:
KEYPASSWORD:=password
DNAME:="CN=example.com, OU=ID, O=Example, L=Doe, S=John, C=GB"
KEYSTOREFILE:=my-release-key.keystore
ALIASNAME:=alias_name
SRCS:= test.c rawdraw/CNFGFunctions.c rawdraw/CNFGEGLDriver.c rawdraw/CNFG3D.c android_native_app_glue.c

keystore : $(KEYSTOREFILE)

$(KEYSTOREFILE) :
	keytool -genkey -v -keystore $(KEYSTOREFILE) -alias $(ALIASNAME) -keyalg RSA -keysize 2048 -validity 10000 -storepass password -keypass $(KEYPASSWORD) -dname $(DNAME)

folders:
	mkdir -p makecapk/lib/arm64-v8a
	mkdir -p makecapk/lib/armeabi-v7a
	mkdir -p makecapk/lib/x86
	mkdir -p makecapk/lib/x86_64

makecapk/lib/arm64-v8a/lib$(APPNAME).so : $(SRCS)
	mkdir -p makecapk/lib/arm64-v8a
	$(CC_ARM32) $(CFLAGS) $(CFLAGS_ARM64) -o $@ $^ $(LDFLAGS)

makecapk/lib/armeabi-v7a/lib$(APPNAME).so : $(SRCS)
	mkdir -p makecapk/lib/armeabi-v7a
	$(CC_ARM64) $(CFLAGS) $(CFLAGS_ARM64) -o $@ $^ $(LDFLAGS)

makecapk/lib/x86/lib$(APPNAME).so : $(SRCS)
	mkdir -p makecapk/lib/x86
	$(CC_x86) $(CFLAGS) $(CFLAGS_x86) -o $@ $^ $(LDFLAGS)

makecapk/lib/x86_64/lib$(APPNAME).so : $(SRCS)
	mkdir -p makecapk/lib/x86_64
	$(CC_x86) $(CFLAGS) $(CFLAGS_x86_64) -o $@ $^ $(LDFLAGS)

#We're really cutting corners.  You should probably use resource files.. Replace android:label="@string/app_name" and add a resource file.
#Then do this -S Sources/res on the aapt line.

TARGETS:=makecapk/lib/arm64-v8a/lib$(APPNAME).so #makecapk/lib/armeabi-v7a/lib$(APPNAME).so makecapk/lib/x86/lib$(APPNAME).so makecapk/lib/x86_64/lib$(APPNAME).so

makecapk.apk : $(TARGETS)
	mkdir -p makecapk/assets
	echo "Test asset file" > makecapk/assets/asset.txt
	rm -rf temp.apk
	$(AAPT) package -f -F temp.apk -I $(SDK)/platforms/android-$(ANDROIDVERSION)/android.jar -M AndroidManifest.xml -A makecapk/assets -v --target-sdk-version $(ANDROIDVERSION)
	unzip -o temp.apk -d makecapk
	rm -rf makecapk.apk
	cd makecapk && zip -D9r ../makecapk.apk .
	ls -l makecapk.apk
	jarsigner -sigalg SHA1withRSA -digestalg SHA1 -verbose -keystore $(KEYSTOREFILE) -storepass $(KEYPASSWORD) makecapk.apk $(ALIASNAME)
	#(zipalign -c -v 8 makecapk.apk)||true #This seems to not work well.
	#jarsigner -verify -verbose -certs makecapk.apk
	ls -l makecapk.apk

uninstall : 
	(adb uninstall $(PACKAGENAME))||true

push : makecapk.apk
	echo "Installing" $(PACKAGENAME)
	adb install makecapk.apk

run : push
	$(eval ACTIVITYNAME:=$(shell $(AAPT) dump badging makecapk.apk | grep "launchable-activity" | cut -f 2 -d"'"))
	adb shell am start -n $(PACKAGENAME)/$(ACTIVITYNAME)

clean :
	rm -rf temp.apk makecapk.apk makecapk 


