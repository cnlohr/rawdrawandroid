# rawdrawandroid

Ever wanted to write C code and run it on Android?  Sick of multi-megabyte
packages just to do the most basic of things.  Well, this is a demo of how
to make your own APKs and build, install and automatically run them in about
2 seconds, and with an apk size of about 25kB.

With this framework you get a demo which has:
 * To make a window with OpenGL ES support
 * Accelerometer/gyro input, multi-touch
 * An android keyboard for key input
 * Ability to store asset files in your APK and read them with `AAssetManager`
 * Permissions support for using things like sound. Example in https://github.com/cnlohr/cnfa
 * Directly access USB devices.  Example in https://github.com/cnlohr/androidusbtest

[![Youtube Video](http://img.youtube.com/vi/Cz_LvaN36Ag/0.jpg)](http://www.youtube.com/watch?v=Cz_LvaN36Ag "")

DISCLAIMER: I take no warranty or responsibility for this code.  Use at your own risk.  I've never released an app on the app store, so there may be some fundamental issue with using this toolset to make commercial apps!

For support, you can try chatting with folks on my discord: https://discord.com/invite/YgBuPUZ

# Why?

Because sometimes you want to do things that don't fit into the normal way of doing it and all the discussions online revolve around doing it with all the normal processes.  And those processes change, making it difficult to keep up and do specific things.  By using `Makefile`s it's easy to see what exact commands are executed and add custom rules and tweak your build.  C is a universal language.  Rawdraw operates on everything from an ESP8266, to RaspberryPi, Windows Linux and now, even Android.  Write code once, use it everywhere.

When you don't fill your build process with hills of beans, you end up being left with the parts that are important, and not the frivilous parts. This makes it easier to develop, deploy, etc, because everything takes much less time.

A little bit of this also has to do to stick it to all those Luddites on the internet who post "that's impossible" or "you're doing it wrong" to Stack Overflow questions... Requesting permissions in the JNI "oh you **have** to do that in Java" or other dumb stuff like that.  I am completely uninterested in your opinions of what is or is not possible.  This is computer science.  There aren't restrictions.  I can do anything I want.  It's just bits.  You don't own me.

P.S. If you want a bunch of examples of how to do a ton of things in C on Android that you "need" java for, scroll to the bottom of this file: https://github.com/cntools/rawdraw/blob/master/CNFGEGLDriver.c - it shows how to use the JNI to marshall a ton of stuff to/from the Android API without needing to jump back into Java/Kotlin land.

# Development Environment

Most of the testing was done on Linux, however @AEFeinstein has done at least cursory testing in Windows.  You still need some components of Android studio set up to use this, so it's generally easier to just install Android studio completely, but there are instructions on sort of how to do it piecemeal for Windows.

## Linux install Android Studio with NDK.

This set of steps describes how to install Android Studio with NDK support in Linux.  It uses the graphical installer and installs a lot more stuff than the instructions below.  You may be able to mix-and-match these two sets of instructions.  For instance if you are on Linux but don't want to sacrifice 6 GB of disk to the Googs.

1) Install prerequisites:
```
	# sudo apt install openjdk-11-jdk-headless adb
```
2) Download Android Studio: https://developer.android.com/studio
3) Start 'studio.sh' in android-studio/bin
4) Let it install the SDK.
5) Go to sdkmanager ("Configure" button in bottom right)
6) Probably want to use Android 24, so select that from the list.
7) Select "SDK Tools" -> "NDK (Side-by-side)"
8) Download this repo
```
	# git clone https://github.com/cnlohr/rawdrawandroid --recurse-submodules
```
9) Turn on developer mode on your phone (will vary depending on android version)
10) Make your own key
```
	make keystore
```
11) Go into developer options on your phone and enable "USB debugging" make sure to select always allow.
12) Plug your phone into the computer.
13) Run your program.
```
	make push run
```

## Steps for GUI-less install (Windows, WSL)

If you're developing in Windows Subsystem for Linux (WSL), follow the "Steps for GUI-less install" to install the Android components from the command line, without any GUI components.

### Extra note for actually deploying to device in Windows

In order to push the APK to your phone, you need `adb` installed in Windows as well.  You can do that by getting the full Android Studio from https://developer.android.com/studio#downloads or directly https://dl.google.com/android/repository/platform-tools_r24.0.4-windows.zip. Installing the full Android Studio is easier, but you can also get the "Command line tools only" and install `adb` from there.  The steps below outline how to do this with the direct link.

### Rest of steps

1. Install Windows Subsystem for Linux (WSL).  You can find instructions here: https://docs.microsoft.com/en-us/windows/wsl/install-win10 - we use "Ubuntu" for this.

2. Install prerequisites:
```
# sudo apt install openjdk-11-jdk-headless adb unzip zip
```
2. Download "Command line tools only": https://developer.android.com/studio#downloads - you can get a URL and use `wget` in WSL to download the tools by clicking on the "Linux" toolset, then right-clicking on the accept link and saying copy link to location.  Then you can say `wget <link>` in WSL.
3. Create a folder for the Android SDK and export it. You may want to add that export to your `~/.bashrc`:
```
# mkdir ~/android-sdk
# export ANDROID_HOME=~/android-sdk
# printf "\nexport ANDROID_HOME=~/android-sdk\n" >> ~/.bashrc
```
4. Unzip the "Command line tools only" file so that `tools` is in your brand new `android-sdk` folder.
5. Install the SDK and NDK components:
```
# yes | $ANDROID_HOME/tools/bin/sdkmanager --sdk_root=${ANDROID_HOME} --licenses
# $ANDROID_HOME/tools/bin/sdkmanager --sdk_root=${ANDROID_HOME} "build-tools;29.0.3" "cmake;3.10.2.4988404" "ndk;21.1.6352462" "patcher;v4" "platform-tools" "platforms;android-24" "tools"
```
6. Install the Windows ADB toolset.
```
mkdir -p $ANDROID_HOME/windows
cd $ANDROID_HOME/windows
wget https://dl.google.com/android/repository/platform-tools_r24.0.4-windows.zip
unzip platform-tools_r24.0.4-windows.zip
export ADB=$ANDROID_HOME/windows/platform-tools/adb.exe
printf "\nexport ADB=$ANDROID_HOME/windows/platform-tools/adb.exe\n" >> ~/.bashrc
```
6. NOTE: because of updates to environment variables, you may want to close and re-open your WSL terminal.
7. Download this repo
```
# git clone https://github.com/cnlohr/rawdrawandroid --recurse-submodules
```
8. Turn on developer mode on your phone (will vary depending on android version)
9. Go into developer options on your phone and enable "USB debugging" make sure to select always allow.
10. Plug your phone into the computer.
11. Make your keystore.
```
make keystore
```
12. Compile and run your program.
```
make run
```

# If you are going to use this
* Check out the example here: https://github.com/cnlohr/rawdrawandroidexample
* You may want to copy-and-paste this project, but, you could probably use it as a submodule.  You may also want to copy-and-paste the submodule.
* You *MUST* override the app name.  
  - See in Makefile `APPNAME` and `PACKAGENAME` you should be able to include this project's makefile and override that.
  - You must also update `AndroidManifest.xml` with whatever name and org you plan to use.
  - You will need to update: `package` in `<manifest>` to be your `PACKAGENAME` variable in Makefile.
  - Both `android:label` labels need to reflect your new app name.  They are in your `<application>` and `<activity>` sections.
  - Update the `android:value` field in `android.app.lib_name`
 
* If you are using permission you have to prompt for, you must both add it to your `AndroidManifest.xml` as well as check if you have it, and if not, prompt the user.  See helper functions below.  You can see an example of this with `sound_android.c` from ColorChord.  https://github.com/cnlohr/colorchord/blob/master/colorchord2/sound_android.c
* Be sure to uninstall any previously installed apps which would look like this app, if you have a different build by the same name signed with another key, bad things will happen.
* You can see your log with:
```
adb logcat
```
 * If your app opens and closes instantly, try seeing if there are any missing symbols:
```
adb logcat | grep UnsatisfiedLinkError
```


# Helper functions

Because we are doing this entirelly in the NDK, with the JNI, we won't have the luxury of writing any Java/Kotlin code and calling it.  That means all of the examples online have to be heavily marshalled.  In rawdraw's EGL driver, we have many examples of how to do that.  That said, you can use the following functions which get you most of the way there. 

`struct android_app * gapp;`

`int AndroidHasPermissions(const char* perm_name);`

`void AndroidRequestAppPermissions(const char * perm);`

`void AndroidDisplayKeyboard(int pShow);`

`int AndroidGetUnicodeChar( int keyCode, int metaState );`

`int android_width, android_height;`

`extern int android_sdk_version; //Derived at start from property ro.build.version.sdk`


# Departures from regular rawdraw.

Also, above and beyond rawdraw, you *must* implement the following two functions to handle when your apps is suspended or resumed.

`void HandleResume();`
`void HandleSuspend();`

In addition to that, the syntax of `HandleMotion(...)` is different, in that instead of the `mask` variable being a mask, it is simply updating that specific pointer.

# Google Play

As it turns out, Google somehow lets apps built with this onto the store.  Like ColorChord https://github.com/cnlohr/colorchord.

## Part 0: Changes to your app.

1. Make sure you are using the newest SDK.
2. You will need to add a versionCode to your `AndroidManifest.xml`.  In your `AndroidManifest.xml`, add `android:versionCode="integer"` to the tag where "integer" is a version number.
3. In your `AndroidManifest.xml`, change `android:debuggable` to false.
4. You may want to support multiple platforms natively.  Add the following to your `Makefile`: `TARGETS:=makecapk/lib/arm64-v8a/lib$(APPNAME).so makecapk/lib/armeabi-v7a/lib$(APPNAME).so makecapk/lib/x86/lib$(APPNAME).so makecapk/lib/x86_64/lib$(APPNAME).so`
5. You will need to specify target and Min SDK in your `AndroidManifest.xml`  See: `<uses-sdk android:minSdkVersion="22" android:targetSdkVersion="28" />`
6. Those target / min versions must match your Makefile.  Note that without a `minSdkVerson` google will wrongfully assume 1.  This is dangerous.  Be sure to test your app on a device with whichever minSdkVersion you've specified.
7.  You will need to disable the debuggable flag in your app.  See `<application android:debuggable="false" ...>`


Get a google play account.  Details surrounding app creation are outside the scope of this readme.  When getting ready to upload your APK.

## Keys:  You will want a key for yourself that's a real key.  Not the fake one.

First you will need to make a real key.  This can be accomplished by deleting our fake key `my-release-key.keystore` and executing the following command (being careful to fill `####` in with real info): 

```make keystore STOREPASS=#### DNAME="\"CN=####, OU=ID, O=####, L=####, S=####, C=####\"" ALIASNAME=####```

The alias name will be `standkey`. You will want to verify you can build your app with this key.  Be sure to fill in STOREPASS the same.

```make clean run  STOREPASS=####```


## Let Google create and manage my app signing key (recommended) 


## Export and upload a key and certificate from a Java keystore 

If you want to use the play store key with "Export and upload a key and certificate from a Java keystore" Instead of `Let Google create and manage my app signing key (recommended)` and follow PEKP instructions.

## Prepping your app for upload.

You MUST have aligned ZIPs for the Play store.  You must run the following command:

```zipalign -c -v 8 makecapk.apk```

Upload your APK `makecapk.apk` made with your key.



# TODO

Try a bunch of these cool priveleges, see what they all do.
* permission.ACCESS
* permission.INTERNET
* permission.HIDE_NON_SYSTEM_OVERLAY_WINDOWS
* permission.ACCESS_NETWORK_STATE
* permission.WRITE_EXTERNAL_STORAGE
* permission.READ_PHONE_STATE
* permission.GET_TASKS
* permission.REORDER_TASKS
* permission.WRITE_APN_SETTINGS
* permission.READ_SECURE_SETTINGS
* permission.READ_SETTINGS
* permission.REAL_GET_TASKS
* permission.INTERACT_ACROSS_USERS
* permission.MANAGE_USERS
* permission.INSTALL_PACKAGES
* permission.DELETE_PACKAGES
* permission.INTERACT_ACROSS_USERS_FULL
* permission.READ_MEDIA_STORAGE
* permission.WRITE_MEDIA_STORAGE
* android.permission.VR
* android.permission.INSTALL_PACKAGES




