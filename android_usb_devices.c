//Copyright 2020 <>< Charles Lohr, You may use this file and library freely under the MIT/x11, NewBSD or ColorChord Licenses.

#include "android_usb_devices.h"
#include "CNFG.h"
#include "os_generic.h"

double dTimeOfUSBFail;
double dTimeOfLastAsk;
jobject deviceConnection = 0;
int deviceConnectionFD = 0;
extern struct android_app * gapp;

void DisconnectUSB()
{
	deviceConnectionFD = 0;
	dTimeOfUSBFail = OGGetAbsoluteTime();
}

int RequestPermissionOrGetConnectionFD( char * ats, uint16_t vid, uint16_t pid )
{
	//Don't permit 
	if( OGGetAbsoluteTime() - dTimeOfUSBFail < 1 ) 
	{
		ats+=sprintf(ats, "Comms failed.  Waiting to reconnect." );
		return -1;
	}

	struct android_app* app = gapp;
	const struct JNINativeInterface * env = 0;
	const struct JNINativeInterface ** envptr = &env;
	const struct JNIInvokeInterface ** jniiptr = app->activity->vm;
	const struct JNIInvokeInterface * jnii = *jniiptr;
	jnii->AttachCurrentThread( jniiptr, &envptr, NULL);
	env = (*envptr);

	// Retrieves NativeActivity.
	jobject lNativeActivity = gapp->activity->clazz;

	//https://stackoverflow.com/questions/13280581/using-android-to-communicate-with-a-usb-hid-device

	//UsbManager manager = (UsbManager)getSystemService(Context.USB_SERVICE);
	jclass ClassContext = env->FindClass( envptr, "android/content/Context" );
	jfieldID lid_USB_SERVICE = env->GetStaticFieldID( envptr, ClassContext, "USB_SERVICE", "Ljava/lang/String;" );
	jobject USB_SERVICE = env->GetStaticObjectField( envptr, ClassContext, lid_USB_SERVICE );

	jmethodID MethodgetSystemService = env->GetMethodID( envptr, ClassContext, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;" );
	jobject manager = env->CallObjectMethod( envptr, lNativeActivity, MethodgetSystemService, USB_SERVICE);
			//Actually returns an android/hardware/usb/UsbManager
	jclass ClassUsbManager = env->FindClass( envptr, "android/hardware/usb/UsbManager" );

	//HashMap<String, UsbDevice> deviceList = mManager.getDeviceList();
	jmethodID MethodgetDeviceList = env->GetMethodID( envptr, ClassUsbManager, "getDeviceList", "()Ljava/util/HashMap;" );
	jobject deviceList = env->CallObjectMethod( envptr, manager, MethodgetDeviceList );

	//Iterator<UsbDevice> deviceIterator = deviceList.values().iterator();
	jclass ClassHashMap = env->FindClass( envptr, "java/util/HashMap" );
	jmethodID Methodvalues = env->GetMethodID( envptr, ClassHashMap, "values", "()Ljava/util/Collection;" );
	jobject deviceListCollection = env->CallObjectMethod( envptr, deviceList, Methodvalues );
	jclass ClassCollection = env->FindClass( envptr, "java/util/Collection" );
	jmethodID Methoditerator = env->GetMethodID( envptr, ClassCollection, "iterator", "()Ljava/util/Iterator;" );
	jobject deviceListIterator = env->CallObjectMethod( envptr, deviceListCollection, Methoditerator );
	jclass ClassIterator = env->FindClass( envptr, "java/util/Iterator" );

	//while (deviceIterator.hasNext())
	jmethodID MethodhasNext = env->GetMethodID( envptr, ClassIterator, "hasNext", "()Z" );
	jboolean bHasNext = env->CallBooleanMethod( envptr, deviceListIterator, MethodhasNext );

	ats+=sprintf(ats, "Has Devices: %d\n", bHasNext );

	jmethodID Methodnext = env->GetMethodID( envptr, ClassIterator, "next", "()Ljava/lang/Object;" );

	jclass ClassUsbDevice = env->FindClass( envptr, "android/hardware/usb/UsbDevice" );
	jclass ClassUsbInterface = env->FindClass( envptr, "android/hardware/usb/UsbInterface" );
	jclass ClassUsbEndpoint = env->FindClass( envptr, "android/hardware/usb/UsbEndpoint" );
	jclass ClassUsbDeviceConnection = env->FindClass( envptr, "android/hardware/usb/UsbDeviceConnection" );
	jmethodID MethodgetDeviceName = env->GetMethodID( envptr, ClassUsbDevice, "getDeviceName", "()Ljava/lang/String;" );
	jmethodID MethodgetVendorId = env->GetMethodID( envptr, ClassUsbDevice, "getVendorId", "()I" );
	jmethodID MethodgetProductId = env->GetMethodID( envptr, ClassUsbDevice, "getProductId", "()I" );
	jmethodID MethodgetInterfaceCount = env->GetMethodID( envptr, ClassUsbDevice, "getInterfaceCount", "()I" );
	jmethodID MethodgetInterface = env->GetMethodID( envptr, ClassUsbDevice, "getInterface", "(I)Landroid/hardware/usb/UsbInterface;" );

	jmethodID MethodgetEndpointCount = env->GetMethodID( envptr, ClassUsbInterface, "getEndpointCount", "()I" );
	jmethodID MethodgetEndpoint = env->GetMethodID( envptr, ClassUsbInterface, "getEndpoint", "(I)Landroid/hardware/usb/UsbEndpoint;" );

	jmethodID MethodgetAddress = env->GetMethodID( envptr, ClassUsbEndpoint, "getAddress", "()I" );
	jmethodID MethodgetMaxPacketSize = env->GetMethodID( envptr, ClassUsbEndpoint, "getMaxPacketSize", "()I" );

	jobject matchingDevice = 0;
	jobject matchingInterface = 0;

	while( bHasNext )
	{
		//  UsbDevice device = deviceIterator.next();
    	//	Log.i(TAG,"Model: " + device.getDeviceName());
		jobject device = env->CallObjectMethod( envptr, deviceListIterator, Methodnext );
		uint16_t vendorId = env->CallIntMethod( envptr, device, MethodgetVendorId );
		uint16_t productId = env->CallIntMethod( envptr, device, MethodgetProductId );
		int ifaceCount = env->CallIntMethod( envptr, device, MethodgetInterfaceCount );
		const char *strdevname = env->GetStringUTFChars(envptr, env->CallObjectMethod( envptr, device, MethodgetDeviceName ), 0);
		ats+=sprintf(ats, "%s,%04x:%04x(%d)\n", strdevname,
			vendorId,
			productId, ifaceCount );

		if( vendorId == vid && productId == pid )
		{
			if( ifaceCount )
			{
				matchingDevice = device;
				matchingInterface = env->CallObjectMethod( envptr, device, MethodgetInterface, 0 );
			}
		}

		bHasNext = env->CallBooleanMethod( envptr, deviceListIterator, MethodhasNext );
	}
	
	jobject matchingEp = 0;

	if( matchingInterface )
	{
		//matchingInterface is of type android/hardware/usb/UsbInterface
		int epCount = env->CallIntMethod( envptr, matchingInterface, MethodgetEndpointCount );
		ats+=sprintf(ats, "Found device %d eps\n", epCount );
		int i;
		for( i = 0; i < epCount; i++ )
		{
			jobject endpoint = env->CallObjectMethod( envptr, matchingInterface, MethodgetEndpoint, i );
			jint epnum = env->CallIntMethod( envptr, endpoint, MethodgetAddress );
			jint mps = env->CallIntMethod( envptr, endpoint, MethodgetMaxPacketSize );
			if( epnum == 0x02 ) matchingEp = endpoint;
			ats+=sprintf(ats, "%p: %02x: MPS: %d (%c)\n", endpoint, epnum, mps, (matchingEp == endpoint)?'*':' ' );
		}			
	}

	jmethodID MethodopenDevice = env->GetMethodID( envptr, ClassUsbManager, "openDevice", "(Landroid/hardware/usb/UsbDevice;)Landroid/hardware/usb/UsbDeviceConnection;" );
	jmethodID MethodrequestPermission = env->GetMethodID( envptr, ClassUsbManager, "requestPermission", "(Landroid/hardware/usb/UsbDevice;Landroid/app/PendingIntent;)V" );
	jmethodID MethodhasPermission = env->GetMethodID( envptr, ClassUsbManager, "hasPermission", "(Landroid/hardware/usb/UsbDevice;)Z" );
	jmethodID MethodclaimInterface = env->GetMethodID( envptr, ClassUsbDeviceConnection, "claimInterface", "(Landroid/hardware/usb/UsbInterface;Z)Z" );
	jmethodID MethodsetInterface = env->GetMethodID( envptr, ClassUsbDeviceConnection, "setInterface", "(Landroid/hardware/usb/UsbInterface;)Z" );
	jmethodID MethodgetFileDescriptor = env->GetMethodID( envptr, ClassUsbDeviceConnection, "getFileDescriptor", "()I" );
	//jmethodID MethodbulkTransfer = env->GetMethodID( envptr, ClassUsbDeviceConnection, "bulkTransfer", "(Landroid/hardware/usb/UsbEndpoint;[BII)I" );  

	//see https://android.googlesource.com/platform/frameworks/base/+/master/core/java/android/hardware/usb/UsbDeviceConnection.java
	//Calls: native_bulk_request -> android_hardware_UsbDeviceConnection_bulk_request -> usb_device_bulk_transfer
	//				UsbEndpoint endpoint, byte[] buffer, int length, int timeout
	//bulkTransfer(UsbEndpoint endpoint, byte[] buffer, int length, int timeout) 

	//UsbDeviceConnection bulkTransfer

	if( matchingEp && matchingDevice )
	{
		//UsbDeviceConnection deviceConnection = manager.openDevice( device )
		deviceConnection = env->CallObjectMethod( envptr, manager, MethodopenDevice, matchingDevice );
		jint epnum = env->CallIntMethod( envptr, matchingEp, MethodgetAddress );

		if( !deviceConnection )
		{
			// 	hasPermission(UsbDevice device) 

			if( OGGetAbsoluteTime() - dTimeOfLastAsk < 5 )
			{
				ats+=sprintf(ats, "Asked for permission.  Waiting to ask again." );
			}
			else if( env->CallBooleanMethod( envptr, manager, MethodhasPermission, matchingDevice ) )
			{
				ats+=sprintf(ats, "Has permission - disconnected?" );
			}
			else
			{
				//android.app.PendingIntent currently setting to 0 (null) seems not to cause crashes, but does force lock screen to happen.
				//Because the screen locks we need to do a much more complicated operation, generating a PendingIntent.  See Below.
				//  			env->CallVoidMethod( envptr, manager, MethodrequestPermission, matchingDevice, 0 );

				//This part mimiced off of:
				//https://www.programcreek.com/java-api-examples/?class=android.hardware.usb.UsbManager&method=requestPermission
				// manager.requestPermission(device, PendingIntent.getBroadcast(context, 0, new Intent(MainActivity.ACTION_USB_PERMISSION), 0));
				jclass ClassPendingIntent = env->FindClass( envptr, "android/app/PendingIntent" );
				jclass ClassIntent = env->FindClass(envptr, "android/content/Intent");
				jmethodID newIntent = env->GetMethodID(envptr, ClassIntent, "<init>", "(Ljava/lang/String;)V");
				jstring ACTION_USB_PERMISSION = env->NewStringUTF( envptr, "com.android.recipes.USB_PERMISSION" );
				jobject intentObject = env->NewObject(envptr, ClassIntent, newIntent, ACTION_USB_PERMISSION);

				jmethodID MethodgetBroadcast = env->GetStaticMethodID( envptr, ClassPendingIntent, "getBroadcast", 
					"(Landroid/content/Context;ILandroid/content/Intent;I)Landroid/app/PendingIntent;" );
				jobject pi = env->CallStaticObjectMethod( envptr, ClassPendingIntent, MethodgetBroadcast, lNativeActivity, 0, intentObject, 0 );

				//This actually requests permission.
				env->CallVoidMethod( envptr, manager, MethodrequestPermission, matchingDevice, pi );
				dTimeOfLastAsk = OGGetAbsoluteTime();
			}
		}
		else
		{
			//Because we want to read and write to an interrupt endpoint, we need to claim the interface - it seems setting interfaces is insufficient here.
			jboolean claimOk = env->CallBooleanMethod( envptr, deviceConnection, MethodclaimInterface, matchingInterface, 1 );
			//jboolean claimOk = env->CallBooleanMethod( envptr, deviceConnection, MethodsetInterface, matchingInterface );
			//jboolean claimOk = 1;
			if( claimOk )
			{
				deviceConnectionFD = env->CallIntMethod( envptr, deviceConnection, MethodgetFileDescriptor );
			}

			ats+=sprintf(ats, "DC: %p; Claim: %d; FD: %d\n", deviceConnection, claimOk, deviceConnectionFD );
		}

	}

	jnii->DetachCurrentThread( jniiptr );
	return (!deviceConnectionFD)?-5:0;
}

