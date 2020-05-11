//Copyright 2020 <>< Charles Lohr, You may use this file and library freely under the MIT/x11, NewBSD or ColorChord Licenses.

#ifndef _ANDROID_USB_DEVICES_H
#define _ANDROID_USB_DEVICES_H

#include <asset_manager.h>
#include <asset_manager_jni.h>
#include <android_native_app_glue.h>

int RequestPermissionOrGetConnectionFD( char * debug_status, uint16_t vid, uint16_t pid );
void DisconnectUSB(); //Disconnect from USB

extern jobject deviceConnection;
extern int deviceConnectionFD;

#endif

