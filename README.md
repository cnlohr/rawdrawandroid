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
