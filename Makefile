all : makecapk.apk 

.PHONY : push run

APPNAME:=cnfgtest
PACKAGENAME:=org.yourorg.$(APPNAME)

ANDROIDVERSION:=24
SDK:=$$HOME/Android/Sdk
NDK:=$(SDK)/ndk-bundle
CFLAGS:=-Os -DCNFGGLES -DANDROID -DANDROID_FULLSCREEN -DAPPNAME=$(appname)
CFLAGS+= -Irawdraw -I$(NDK)/sources/android/native_app_glue -I$(NDK)/sysroot/usr/include -I$(NDK)/sysroot/usr/include/android -fPIC
LDFLAGS:= -lm -Ltoolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/aarch64-linux-android/$(ANDROIDVERSION) -lGLESv3 -lEGL -u ANativeActivity_onCreate -landroid -llog
LDFLAGS += -shared -s
CC_ARM32:=$(NDK)/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android$(ANDROIDVERSION)-clang
CC_ARM64:=$(NDK)/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi$(ANDROIDVERSION)-clang
CC_x86:=$(NDK)/toolchains/llvm/prebuilt/linux-x86_64/bin/x86_64-linux-android$(ANDROIDVERSION)-clang
CC_x86_64=$(NDK)/toolchains/llvm/prebuilt/linux-x86_64/bin/x86_64-linux-android$(ANDROIDVERSION)-clang

CFLAGS_ARM64:=-m64
CFLAGS_ARM32:=-mfloat-abi=softfp
CFLAGS_x86:=-march=i686 -mtune=intel -mssse3 -mfpmath=sse -m32
CFLAGS_x86_64:=-march=x86-64 -msse4.2 -mpopcnt -m64 -mtune=intel

KEYPASSWORD:=password
DNAME:="CN=example.com, OU=ID, O=Example, L=Doe, S=John, C=GB"
KEYSTOREFILE:=my-release-key.keystore
ALIASNAME:=alias_name
SRCS:= test.c rawdraw/CNFGFunctions.c rawdraw/CNFGEGLDriver.c rawdraw/CNFG3D.c ~/Android/Sdk/ndk-bundle/sources/android/native_app_glue/android_native_app_glue.c

keystore : $(KEYSTOREFILE)

$(KEYSTOREFILE) :
	keytool -genkey -v -keystore $(KEYSTOREFILE) -alias $(ALIASNAME) -keyalg RSA -keysize 2048 -validity 10000 -storepass password -keypass $(KEYPASSWORD) -dname $(DNAME)

folders:
	mkdir -p makecapk/lib/arm64-v8a
	mkdir -p makecapk/lib/armeabi-v7a
	mkdir -p makecapk/lib/x86
	mkdir -p makecapk/lib/x86_64

makecapk/lib/arm64-v8a/lib$(APPNAME).so : test.c rawdraw/CNFGFunctions.c rawdraw/CNFGEGLDriver.c rawdraw/CNFG3D.c ~/Android/Sdk/ndk-bundle/sources/android/native_app_glue/android_native_app_glue.c
	mkdir -p makecapk/lib/arm64-v8a
	$(CC_ARM32) $(CFLAGS) $(CFLAGS_ARM64) -o $@ $^ $(LDFLAGS)

makecapk/lib/armeabi-v7a/lib$(APPNAME).so : test.c rawdraw/CNFGFunctions.c rawdraw/CNFGEGLDriver.c rawdraw/CNFG3D.c ~/Android/Sdk/ndk-bundle/sources/android/native_app_glue/android_native_app_glue.c
	mkdir -p makecapk/lib/armeabi-v7a
	$(CC_ARM64) $(CFLAGS) $(CFLAGS_ARM64) -o $@ $^ $(LDFLAGS)

makecapk/lib/x86/lib$(APPNAME).so : test.c rawdraw/CNFGFunctions.c rawdraw/CNFGEGLDriver.c rawdraw/CNFG3D.c ~/Android/Sdk/ndk-bundle/sources/android/native_app_glue/android_native_app_glue.c
	mkdir -p makecapk/lib/x86
	$(CC_x86) $(CFLAGS) $(CFLAGS_x86) -o $@ $^ $(LDFLAGS)

makecapk/lib/x86_64/lib$(APPNAME).so : $(SRCS)
	mkdir -p makecapk/lib/x86_64
	$(CC_x86) $(CFLAGS) $(CFLAGS_x86_64) -o $@ $^ $(LDFLAGS)

#We're really cutting corners.  You should probably use resource files.. Replace android:label="@string/app_name" and add a resource file.
#Then do this -S Sources/res on the aapt line.

makecapk.apk : makecapk/lib/arm64-v8a/lib$(APPNAME).so makecapk/lib/armeabi-v7a/lib$(APPNAME).so makecapk/lib/x86/lib$(APPNAME).so makecapk/lib/x86_64/lib$(APPNAME).so
	mkdir -p makecapk/assets
	echo "Test asset file" > makecapk/assets/asset.txt
	rm -rf temp.apk
	aapt package -f -F temp.apk -I ~/Android/Sdk/platforms/android-$(ANDROIDVERSION)/android.jar -M AndroidManifest.xml -A makecapk/assets -v --target-sdk-version $(ANDROIDVERSION)
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

push : uninstall makecapk.apk
	echo "Installing" $(PACKAGENAME)
	adb install makecapk.apk

run : push
	$(eval ACTIVITYNAME:=$(shell aapt dump badging makecapk.apk | grep "launchable-activity" | cut -f 2 -d"'"))
	adb shell am start -n $(PACKAGENAME)/$(ACTIVITYNAME)

clean :
	rm -rf temp.apk makecapk.apk makecapk 


