#Copyright (c) 2019-2020 <>< Charles Lohr - Under the MIT/x11 or NewBSD License you choose.
# NO WARRANTY! NO GUARANTEE OF SUPPORT! USE AT YOUR OWN RISK

all : makecapk.apk 

.PHONY : push run

# WARNING WARNING WARNING!  YOU ABSOLUTELY MUST OVERRIDE THE PROJECT NAME
# you should also override these parameters, get your own signatre file and make your own manifest.
APPNAME?=cnfgtest
PACKAGENAME?=org.yourorg.$(APPNAME)
RAWDRAWANDROID?=.
RAWDRAWANDROIDSRCS=$(RAWDRAWANDROID)/rawdraw/CNFGFunctions.c $(RAWDRAWANDROID)/rawdraw/CNFGEGLDriver.c $(RAWDRAWANDROID)/rawdraw/CNFG3D.c $(RAWDRAWANDROID)/android_native_app_glue.c
SRC?=test.c
ANDROIDSRCS:= $(SRC) $(RAWDRAWANDROIDSRCS)
#We've tested it with android version 24.
ANDROIDVERSION?=24

#if you have a custom Android Home location you can add it to this list.  
#This makefile will select the first present folder.
SDK_LOCATIONS+=$(ANDROID_HOME) ~/Android/Sdk

#Just a little Makefile witchcraft to find the first SDK_LOCATION that exists
#Then find an ndk folder and build tools folder in there.
ANDROIDSDK?=$(firstword $(foreach dir, $(SDK_LOCATIONS), $(basename $(dir) ) ) )
NDK?=$(firstword $(wildcard $(ANDROIDSDK)/ndk/*) )
BUILD_TOOLS?=$(firstword $(wildcard $(ANDROIDSDK)/build-tools/*) )
ADB?=adb

testsdk :
	echo $(BUILD_TOOLS)

CFLAGS+=-Os -DCNFGGLES -DANDROID -DANDROID_FULLSCREEN -DAPPNAME=\"$(APPNAME)\"
CFLAGS+= -I$(RAWDRAWANDROID)/rawdraw -I$(NDK)/sysroot/usr/include -I$(NDK)/sysroot/usr/include/android -fPIC -I$(RAWDRAWANDROID)
LDFLAGS += -lm -L$(NDK)toolchains/llvm/prebuilt/linux-x86_64/sysroot/usr/lib/aarch64-linux-android/$(ANDROIDVERSION) -lGLESv3 -lEGL -landroid -llog
LDFLAGS += -shared -s -uANativeActivity_onCreate

CC_ARM32:=$(NDK)/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android$(ANDROIDVERSION)-clang
CC_ARM64:=$(NDK)/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi$(ANDROIDVERSION)-clang
CC_x86:=$(NDK)/toolchains/llvm/prebuilt/linux-x86_64/bin/x86_64-linux-android$(ANDROIDVERSION)-clang
CC_x86_64=$(NDK)/toolchains/llvm/prebuilt/linux-x86_64/bin/x86_64-linux-android$(ANDROIDVERSION)-clang
AAPT:=$(BUILD_TOOLS)/aapt

# Which binaries to build?
TARGETS:=makecapk/lib/arm64-v8a/lib$(APPNAME).so #makecapk/lib/armeabi-v7a/lib$(APPNAME).so makecapk/lib/x86/lib$(APPNAME).so makecapk/lib/x86_64/lib$(APPNAME).so

CFLAGS_ARM64:=-m64
CFLAGS_ARM32:=-mfloat-abi=softfp
CFLAGS_x86:=-march=i686 -mtune=intel -mssse3 -mfpmath=sse -m32
CFLAGS_x86_64:=-march=x86-64 -msse4.2 -mpopcnt -m64 -mtune=intel
/home/cnlohr/Android/Sdk/ndk-bundle/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android24-clang:
KEYPASSWORD:=password
DNAME:="CN=example.com, OU=ID, O=Example, L=Doe, S=John, C=GB"
KEYSTOREFILE:=my-release-key.keystore
ALIASNAME:=alias_name

keystore : $(KEYSTOREFILE)

$(KEYSTOREFILE) :
	keytool -genkey -v -keystore $(KEYSTOREFILE) -alias $(ALIASNAME) -keyalg RSA -keysize 2048 -validity 10000 -storepass password -keypass $(KEYPASSWORD) -dname $(DNAME)

folders:
	mkdir -p makecapk/lib/arm64-v8a
	mkdir -p makecapk/lib/armeabi-v7a
	mkdir -p makecapk/lib/x86
	mkdir -p makecapk/lib/x86_64

makecapk/lib/arm64-v8a/lib$(APPNAME).so : $(ANDROIDSRCS)
	mkdir -p makecapk/lib/arm64-v8a
	$(CC_ARM32) $(CFLAGS) $(CFLAGS_ARM64) -o $@ $^ $(LDFLAGS)

makecapk/lib/armeabi-v7a/lib$(APPNAME).so : $(ANDROIDSRCS)
	mkdir -p makecapk/lib/armeabi-v7a
	$(CC_ARM64) $(CFLAGS) $(CFLAGS_ARM64) -o $@ $^ $(LDFLAGS)

makecapk/lib/x86/lib$(APPNAME).so : $(ANDROIDSRCS)
	mkdir -p makecapk/lib/x86
	$(CC_x86) $(CFLAGS) $(CFLAGS_x86) -o $@ $^ $(LDFLAGS)

makecapk/lib/x86_64/lib$(APPNAME).so : $(ANDROIDSRCS)
	mkdir -p makecapk/lib/x86_64
	$(CC_x86) $(CFLAGS) $(CFLAGS_x86_64) -o $@ $^ $(LDFLAGS)

#We're really cutting corners.  You should probably use resource files.. Replace android:label="@string/app_name" and add a resource file.
#Then do this -S Sources/res on the aapt line.
#For icon support, add -S makecapk/res to the aapt line.  also,  android:icon="@mipmap/icon" to your application line in the manifest.
#If you want to strip out about 800 bytes of data you can remove the icon and strings.

#Notes for the past:  These lines used to work, but don't seem to anymore.  Switched to newer jarsigner.
#(zipalign -c -v 8 makecapk.apk)||true #This seems to not work well.
#jarsigner -verify -verbose -certs makecapk.apk



makecapk.apk : $(TARGETS) $(EXTRA_ASSETS_TRIGGER)
	mkdir -p makecapk/assets
	echo "Test asset file" > makecapk/assets/asset.txt
	rm -rf temp.apk
	$(AAPT) package -f -F temp.apk -I $(ANDROIDSDK)/platforms/android-$(ANDROIDVERSION)/android.jar -M AndroidManifest.xml -S Sources/res -A makecapk/assets -v --target-sdk-version $(ANDROIDVERSION)
	unzip -o temp.apk -d makecapk
	rm -rf makecapk.apk
	cd makecapk && zip -D9r ../makecapk.apk .
	ls -l makecapk.apk
	jarsigner -sigalg SHA1withRSA -digestalg SHA1 -verbose -keystore $(KEYSTOREFILE) -storepass $(KEYPASSWORD) makecapk.apk $(ALIASNAME)
	ls -l makecapk.apk


uninstall : 
	($(ADB) uninstall $(PACKAGENAME))||true

push : makecapk.apk
	echo "Installing" $(PACKAGENAME)
	$(ADB) install makecapk.apk

run : push
	$(eval ACTIVITYNAME:=$(shell $(AAPT) dump badging makecapk.apk | grep "launchable-activity" | cut -f 2 -d"'"))
	$(ADB) shell am start -n $(PACKAGENAME)/$(ACTIVITYNAME)

clean :
	rm -rf temp.apk makecapk.apk makecapk 


