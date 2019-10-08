# rawdrawandroid

This guide is written around a Linux x64 host.  Though the binaries this
makes are small, you're still going to need around 5 GB of HDD space to
start the install, though you can delete everything but your ~/Android
folder, which should be about 

Step 1: Install prerequisites:
```
	# sudo apt install openjdk-11-jdk-headless adb
```
Step 2: Download Android Studio: https://developer.android.com/studio
Step 3: Start 'studio.sh' in android-studio/bin
Step 4: Let it install the SDK.
Step 5: Go to sdkmanager ("Configure" button in bottom right)
Step 6: Probably want to use Android 24, so select that from the list.
Step 7: Select "SDK Tools" -> "NDK (Side-by-side)"

Step 8: Download this repo
```
	# git clone https://github.com/cnlohr/rawdrawandroid --recurse-submodules
```


TODO:

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







