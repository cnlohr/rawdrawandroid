all : build 

.PHONY : build push run

KEYPASSWORD:=password
DNAME:="CN=example.com, OU=ID, O=Example, L=Doe, S=John, C=GB"
KEYSTOREFILE:=my-release-key.keystore
ALIASNAME:=alias_name
PACKAGENAME:=org.cntools.makecapk

keystore : $(KEYSTOREFILE)

$(KEYSTOREFILE) :
	keytool -genkey -v -keystore $(KEYSTOREFILE) -alias $(ALIASNAME) -keyalg RSA -keysize 2048 -validity 10000 -storepass password -keypass $(KEYPASSWORD) -dname $(DNAME)

build :
	mkdir -p makecapk/lib/arm64-v8a
	mkdir -p makecapk/assets
	cp ../rawdraw_android makecapk/lib/arm64-v8a/libmakecapk.so
	rm -rf temp.apk
	~/Android/Sdk/build-tools/28.0.3/aapt package -f -F temp.apk -I ~/Android/Sdk/platforms/android-24/android.jar -M AndroidManifest.xml -S Sources/res -v --target-sdk-version 24
	unzip -o temp.apk -d makecapk
	rm -rf makecapk.apk
	cd makecapk && zip -D9r ../makecapk.apk .
	zipalign -c -v 4 makecapk.apk
	jarsigner -sigalg SHA1withRSA -digestalg SHA1 -verbose -keystore $(KEYSTOREFILE) -storepass $(KEYPASSWORD) makecapk.apk $(ALIASNAME)
	#jarsigner -verify -verbose -certs makecapk.apk

uninstall : 
	(adb uninstall $(PACKAGENAME))||true

push : uninstall build
	echo "Installing" $(PACKAGENAME)
	adb install makecapk.apk

run : 
	$(eval ACTIVITYNAME:=$(shell aapt dump badging makecapk.apk | grep "launchable-activity" | cut -f 2 -d"'"))
	adb shell am start -n $(PACKAGENAME)/$(ACTIVITYNAME)

clean :
	rm -rf temp.apk makecapk.apk makecapk
