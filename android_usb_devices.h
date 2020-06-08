/* See LICENSE file for copyright and license details. */

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

