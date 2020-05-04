# rawdrawandroid

Ever wanted to write C code and run it on Android?  Sick of multi-megabyte
packages just to do the most basic of things.  Well, this is a demo of how
to make your own APKs and build, install and automatically run them in about
2 seconds, and with an apk size of about 25kB.

With this framework you get:
 * To make a window with OpenGL ES support
 * Accelerometer/gyro input
 * Multi-touch
 * An android keyboard for key input
 * Ability to store asset files in your APK and read them with `AAssetManager`

![Screen Shot](https://github.com/cnlohr/rawdrawandroid/raw/master/screenshot.png)

DISCLAIMER: I take no warranty or responsibility for this code.  Use at your own risk.  I've never released an app on the app store, so there may be some fundamental
issue with using this toolset to make commercial apps!

## Development Environment

Most of the testing was done on Linux, however @AEFeinstein has done at least cursory testing in Windows.

You still need some components of Android studio set up to use this, so it's generally easier to just install
Android studio completely, but there are instructions on sort of how to do it piecemeal for Windows.

### Windows

If you're developing in Windows Subsystem for Linux (WSL), follow the "Steps for GUI-less install" to install the Android components from the command line, without any GUI components.
In order to push the APK to your phone, you need `adb` installed in Windows as well, so get that from https://developer.android.com/studio#downloads. Installing the full Android Studio is easier, but you can also get the "Command line tools only" and install `adb` from there.
Once you have `adb` for Windows, modify this project's `Makefile` to invoke `adb.exe` instead of `adb` in all three places. The `.exe` will invoke the Windows host `adb` instead of the Linux version. That will allow you to upload the APK.

### Linux install Android Studio with NDK.

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

### Steps for GUI-less install (Windows)
1. Install prerequisites:
```
# sudo apt install openjdk-11-jdk-headless adb
```
2. Download "Command line tools only": https://developer.android.com/studio#downloads
3. Create a folder for the Android SDK and export it. You may want to add that export to your `~/.bashrc`:
```
# mkdir ~/android-sdk
# export ANDROID_HOME=~/android-sdk
# printf "\nANDROID_HOME=~/android-sdk\n" >> ~/.bashrc
```
4. Unzip the "Command line tools only" file so that `tools` is in your brand new `android-sdk` folder.
5. Install the SDK and NDK components:
```
# yes | $ANDROID_HOME/tools/bin/sdkmanager --sdk_root=${ANDROID_HOME} --licenses
# $ANDROID_HOME/tools/bin/sdkmanager --sdk_root=${ANDROID_HOME} "build-tools;29.0.3" "cmake;3.10.2.4988404" "ndk;21.1.6352462" "patcher;v4" "platform-tools" "platforms;android-24" "tools"
```
6. Download this repo
```
# git clone https://github.com/cnlohr/rawdrawandroid --recurse-submodules
```
7. Turn on developer mode on your phone (will vary depending on android version)
8. Go into developer options on your phone and enable "USB debugging" make sure to select always allow.
9. Plug your phone into the computer.
10. Run your program.
```
make push run
```

## If you are going to use this
 * Check out the example here: https://github.com/cnlohr/rawdrawandroidexample
 * You may want to copy-and-paste this project, but, you could probably use it as a submodule.  You may also want to copy-and-paste the submodule.
 * You *MUST* override the app name.  See in Makefile `APPNAME` - you should be able to include this project's makefile and override that.  You must also update `AndroidManifest.xml` with whatever name and org you plan to use.  That means updating all three fields. Both `android:name` fields and the `package` field in the header.
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


## Helper functions

Because we are doing this entirelly in the NDK, with the JNI, we won't have the luxury of writing any Java/Kotlin code and calling it.  That means all of the examples online have to be heavily marshalled.  In rawdraw's EGL driver, we have many examples of how to do that.  That said, you can use the following functions which get you most of the way there.

`struct android_app * gapp;`

`int AndroidHasPermissions(const char* perm_name);`

`void AndroidRequestAppPermissions(const char * perm);`

`void AndroidDisplayKeyboard(int pShow);`

`int AndroidGetUnicodeChar( int keyCode, int metaState );`

`int android_width, android_height;`

## Departures from regular rawdraw.

Also, above and beyond rawdraw, you *must* implement the following two functions to handle when your apps is suspended or resumed.

`void HandleResume();`
`void HandleSuspend();`

In addition to that, the syntax of `HandleMotion(...)` is different, in that instead of the `mask` variable being a mask, it is simply updating that specific pointer.

## TODO

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




