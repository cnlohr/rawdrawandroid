# rawdrawandroid

Ever wanted to write C code and run it on Android?  Sick of multi-megabyte
packages just to do the most basic of things.  Well, this is a demo of how
to make your own APKs and build, install and automatically run them in about
2 seconds, and with an apk size of about 25kB.

With this framework you get:
 * To make a window
 * Accelerometer/gyro input
 * Multi-touch
 * An android keyboard for key input
 * OpenGL ES
 * Ability to store asset files in your APK and read them with `AAssetManager`

![Screen Shot](https://github.com/cnlohr/rawdrawandroid/raw/master/screenshot.png)


Note that this project, as it is will not build apps ready for the app store,
as it only targets arm64-v8a, and it is missing things like icon support,
etc.  But, it is a start!

This guide is written around a Linux x64 host.  Maybe Windows could be supported
at some later time, but 🤷.  Though the binaries this
makes are small, depending on what you're doing you may still need around
5 GB of HDD space to start the install, though you can delete everything
but your ~/Android folder.

Steps:
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
10) Go into developer options on your phone and enable "USB debugging" make sure to select always allow.
11) Plug your phone into the computer.
12) Run your program.
```
	make push run
```

## Example project

An example git submodule is available here:

https://github.com/cnlohr/rawdrawandroidexample



## If you are going to use this

 * You probably want to copy-and-paste this project, but, you could probably use it as a submodule.
 * You *MUST* override the app name.  See in Makefile `APPNAME` - you should be able to include this project's makefile and override that.

## General note:
 * Be sure to uninstall any previously installed apps which would look like this app.

## protips
 * You can see your log with:
```
adb logcat
```
 * If your app opens and closes instantly, try seeing if there are any missing symbols:
```
adb logcat | grep UnsatisfiedLinkError
```

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




