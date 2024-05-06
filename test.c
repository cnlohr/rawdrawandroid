//Copyright (c) 2011-2020 <>< Charles Lohr - Under the MIT/x11 or NewBSD License you choose.
// NO WARRANTY! NO GUARANTEE OF SUPPORT! USE AT YOUR OWN RISK

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "os_generic.h"
#include <GLES3/gl3.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android_native_app_glue.h>
#include <android/sensor.h>
#include <byteswap.h>
#include <errno.h>
#include <fcntl.h>
#include "CNFGAndroid.h"

//#define CNFA_IMPLEMENTATION
#define CNFG_IMPLEMENTATION
#define CNFG3D

//#include "cnfa/CNFA.h"
#include "CNFG.h"

#define WEBVIEW_NATIVE_ACTIVITY_IMPLEMENTATION
#include "webview_native_activity.h"

float mountainangle;
float mountainoffsetx;
float mountainoffsety;

ASensorManager * sm;
const ASensor * as;
bool no_sensor_for_gyro = false;
ASensorEventQueue* aeq;
ALooper * l;

WebViewNativeActivityObject MyWebView;

const uint32_t SAMPLE_RATE = 44100;
const uint16_t SAMPLE_COUNT = 512;
uint32_t stream_offset = 0;
uint16_t audio_frequency;

void SetupIMU()
{
	sm = ASensorManager_getInstanceForPackage("gyroscope");
	as = ASensorManager_getDefaultSensor( sm, ASENSOR_TYPE_GYROSCOPE );
	no_sensor_for_gyro = as == NULL;
	l = ALooper_prepare( ALOOPER_PREPARE_ALLOW_NON_CALLBACKS );
	aeq = ASensorManager_createEventQueue( sm, (ALooper*)&l, 0, 0, 0 ); //XXX??!?! This looks wrong.
	if(!no_sensor_for_gyro) {
		ASensorEventQueue_enableSensor( aeq, as);
		printf( "setEvent Rate: %d\n", ASensorEventQueue_setEventRate( aeq, as, 10000 ) );
	}

}

float accx, accy, accz;
int accs;

void AccCheck()
{
	if(no_sensor_for_gyro) {
		return;
	}

	ASensorEvent evt;
	do
	{
		ssize_t s = ASensorEventQueue_getEvents( aeq, &evt, 1 );
		if( s <= 0 ) break;
		accx = evt.vector.v[0];
		accy = evt.vector.v[1];
		accz = evt.vector.v[2];
		mountainangle /*degrees*/ -= accz;// * 3.1415 / 360.0;// / 100.0;
		mountainoffsety += accy;
		mountainoffsetx += accx;
		accs++;
	} while( 1 );
}

unsigned frames = 0;
unsigned long iframeno = 0;

void AndroidDisplayKeyboard(int pShow);

int lastbuttonx = 0;
int lastbuttony = 0;
int lastmotionx = 0;
int lastmotiony = 0;
int lastbid = 0;
int lastmask = 0;
int lastkey, lastkeydown;

static int keyboard_up;
uint8_t buttonstate[8];

void HandleKey( int keycode, int bDown )
{
	lastkey = keycode;
	lastkeydown = bDown;
	if( keycode == 10 && !bDown ) { keyboard_up = 0; AndroidDisplayKeyboard( keyboard_up );  }

	if( keycode == 4 ) { AndroidSendToBack( 1 ); } //Handle Physical Back Button.
}

void HandleButton( int x, int y, int button, int bDown )
{
	buttonstate[button] = bDown;
	lastbid = button;
	lastbuttonx = x;
	lastbuttony = y;

	if( bDown ) { keyboard_up = !keyboard_up; AndroidDisplayKeyboard( keyboard_up ); }
}

void HandleMotion( int x, int y, int mask )
{
	lastmask = mask;
	lastmotionx = x;
	lastmotiony = y;
}

#define HMX 162
#define HMY 162
short screenx, screeny;
float Heightmap[HMX*HMY];

extern struct android_app * gapp;

void DrawHeightmap()
{
	int x, y;
	//float fdt = ((iframeno++)%(360*10))/10.0;

	mountainangle += .2;
	if( mountainangle < 0 ) mountainangle += 360;
	if( mountainangle > 360 ) mountainangle -= 360;

	mountainoffsety = mountainoffsety - ((mountainoffsety-100) * .1);

	float eye[3] = { (float)(sin(mountainangle*(3.14159/180.0))*30*sin(mountainoffsety/100.)), (float)(cos(mountainangle*(3.14159/180.0))*30*sin(mountainoffsety/100.)), (float)(30*cos(mountainoffsety/100.)) };
	float at[3] = { 0,0, 0 };
	float up[3] = { 0,0, 1 };

	tdSetViewport( -1, -1, 1, 1, screenx, screeny );

	tdMode( tdPROJECTION );
	tdIdentity( gSMatrix );
	tdPerspective( 30, ((float)screenx)/((float)screeny), .1, 200., gSMatrix );

	tdMode( tdMODELVIEW );
	tdIdentity( gSMatrix );
	tdTranslate( gSMatrix, 0, 0, -40 );
	tdLookAt( gSMatrix, eye, at, up );

	float scale = 60./HMX;

	for( x = 0; x < HMX-1; x++ )
	for( y = 0; y < HMY-1; y++ )
	{
		float tx = x-HMX/2;
		float ty = y-HMY/2;
		float pta[3];
		float ptb[3];
		float ptc[3];
		float ptd[3];

		float normal[3];
		float lightdir[3] = { .6, -.6, 1 };
		float tmp1[3];
		float tmp2[3];

		RDPoint pto[6];

		pta[0] = (tx+0)*scale; pta[1] = (ty+0)*scale; pta[2] = Heightmap[(x+0)+(y+0)*HMX]*scale;
		ptb[0] = (tx+1)*scale; ptb[1] = (ty+0)*scale; ptb[2] = Heightmap[(x+1)+(y+0)*HMX]*scale;
		ptc[0] = (tx+0)*scale; ptc[1] = (ty+1)*scale; ptc[2] = Heightmap[(x+0)+(y+1)*HMX]*scale;
		ptd[0] = (tx+1)*scale; ptd[1] = (ty+1)*scale; ptd[2] = Heightmap[(x+1)+(y+1)*HMX]*scale;

		tdPSub( pta, ptb, tmp2 );
		tdPSub( ptc, ptb, tmp1 );
		tdCross( tmp1, tmp2, normal );
		tdNormalizeSelf( normal );

		tdFinalPoint( pta, pta );
		tdFinalPoint( ptb, ptb );
		tdFinalPoint( ptc, ptc );
		tdFinalPoint( ptd, ptd );

		if( pta[2] >= 1.0 ) continue;
		if( ptb[2] >= 1.0 ) continue;
		if( ptc[2] >= 1.0 ) continue;
		if( ptd[2] >= 1.0 ) continue;

		if( pta[2] < 0 ) continue;
		if( ptb[2] < 0 ) continue;
		if( ptc[2] < 0 ) continue;
		if( ptd[2] < 0 ) continue;

		pto[0].x = pta[0]; pto[0].y = pta[1];
		pto[1].x = ptb[0]; pto[1].y = ptb[1];
		pto[2].x = ptd[0]; pto[2].y = ptd[1];

		pto[3].x = ptc[0]; pto[3].y = ptc[1];
		pto[4].x = ptd[0]; pto[4].y = ptd[1];
		pto[5].x = pta[0]; pto[5].y = pta[1];

//		CNFGColor(((x+y)&1)?0xFFFFFF:0x000000);

		float bright = tdDot( normal, lightdir );
		if( bright < 0 ) bright = 0;
		CNFGColor( 0xff | ( ( (int)( bright * 90 ) ) << 24 ) );

//		CNFGTackPoly( &pto[0], 3 );		CNFGTackPoly( &pto[3], 3 );
		CNFGTackSegment( pta[0], pta[1], ptb[0], ptb[1] );
		CNFGTackSegment( pta[0], pta[1], ptc[0], ptc[1] );
		CNFGTackSegment( ptb[0], ptb[1], ptc[0], ptc[1] );
	
	}
}


int HandleDestroy()
{
	printf( "Destroying\n" );
	return 0;
}

volatile int suspended;

void HandleSuspend()
{
	suspended = 1;
}

void HandleResume()
{
	suspended = 0;
}

/*
void AudioCallback( struct CNFADriver * sd, short * out, short * in, int framesp, int framesr )
{
	memset(out, 0, framesp*sizeof(uint16_t));
	if(suspended) return;
	if(!buttonstate[1]) return; // play audio only if ~touching with two fingers
	audio_frequency = 440;
	for(uint32_t i = 0; i < framesp; i++) {
		int16_t sample = INT16_MAX * sin(audio_frequency*(2*M_PI)*(stream_offset+i)/SAMPLE_RATE);
		out[i] = sample;
	}
	stream_offset += framesp;
}
*/

void MakeNotification( const char * channelID, const char * channelName, const char * title, const char * message )
{
	static int id;
	id++;

	const struct JNINativeInterface * env = 0;
	const struct JNINativeInterface ** envptr = &env;
	const struct JNIInvokeInterface ** jniiptr = gapp->activity->vm;
	const struct JNIInvokeInterface * jnii = *jniiptr;

	jnii->AttachCurrentThread( jniiptr, &envptr, NULL);
	env = (*envptr);

	jstring channelIDStr = env->NewStringUTF( ENVCALL channelID );
	jstring channelNameStr = env->NewStringUTF( ENVCALL channelName );

	// Runs getSystemService(Context.NOTIFICATION_SERVICE).
	jclass NotificationManagerClass = env->FindClass( ENVCALL "android/app/NotificationManager" );
	jclass activityClass = env->GetObjectClass( ENVCALL gapp->activity->clazz );
	jmethodID MethodGetSystemService = env->GetMethodID( ENVCALL activityClass, "getSystemService", "(Ljava/lang/String;)Ljava/lang/Object;");
	jstring notificationServiceName = env->NewStringUTF( ENVCALL "notification" );
	jobject notificationServiceObj = env->CallObjectMethod( ENVCALL gapp->activity->clazz, MethodGetSystemService, notificationServiceName);

	// create the Notification channel.
	jclass notificationChannelClass = env->FindClass( ENVCALL "android/app/NotificationChannel" );
	jmethodID notificationChannelConstructorID = env->GetMethodID( ENVCALL notificationChannelClass, "<init>", "(Ljava/lang/String;Ljava/lang/CharSequence;I)V" );
	jobject notificationChannelObj = env->NewObject( ENVCALL notificationChannelClass, notificationChannelConstructorID, channelIDStr, channelNameStr, 3 ); // IMPORTANCE_DEFAULT
	jmethodID createNotificationChannelID = env->GetMethodID( ENVCALL NotificationManagerClass, "createNotificationChannel", "(Landroid/app/NotificationChannel;)V" );
	env->CallVoidMethod( ENVCALL notificationServiceObj, createNotificationChannelID, notificationChannelObj );

	env->DeleteLocalRef( ENVCALL channelNameStr );
	env->DeleteLocalRef( ENVCALL notificationChannelObj );

	// Create the Notification builder.
	jclass classBuilder = env->FindClass( ENVCALL "android/app/Notification$Builder" );
	jstring titleStr = env->NewStringUTF( ENVCALL title );
	jstring messageStr = env->NewStringUTF( ENVCALL message );
	jmethodID eventConstructor = env->GetMethodID( ENVCALL classBuilder, "<init>", "(Landroid/content/Context;Ljava/lang/String;)V" );
	jobject eventObj = env->NewObject( ENVCALL classBuilder, eventConstructor, gapp->activity->clazz, channelIDStr );
	jmethodID setContentTitleID = env->GetMethodID( ENVCALL classBuilder, "setContentTitle", "(Ljava/lang/CharSequence;)Landroid/app/Notification$Builder;" );
	jmethodID setContentTextID = env->GetMethodID( ENVCALL classBuilder, "setContentText", "(Ljava/lang/CharSequence;)Landroid/app/Notification$Builder;" );
	jmethodID setSmallIconID = env->GetMethodID( ENVCALL classBuilder, "setSmallIcon", "(I)Landroid/app/Notification$Builder;" );

	// You could do things like setPriority, or setContentIntent if you want it to do something when you click it.

	env->CallObjectMethod( ENVCALL eventObj, setContentTitleID, titleStr );
	env->CallObjectMethod( ENVCALL eventObj, setContentTextID, messageStr );
	env->CallObjectMethod( ENVCALL eventObj, setSmallIconID, 17301504 ); // R.drawable.alert_dark_frame

	// eventObj.build()
	jmethodID buildID = env->GetMethodID( ENVCALL classBuilder, "build", "()Landroid/app/Notification;" );
	jobject notification = env->CallObjectMethod( ENVCALL eventObj, buildID );

	// NotificationManager.notify(...)
	jmethodID notifyID = env->GetMethodID( ENVCALL NotificationManagerClass, "notify", "(ILandroid/app/Notification;)V" );
	env->CallVoidMethod( ENVCALL notificationServiceObj, notifyID, id, notification );

	env->DeleteLocalRef( ENVCALL notification );
	env->DeleteLocalRef( ENVCALL titleStr );
	env->DeleteLocalRef( ENVCALL activityClass );
	env->DeleteLocalRef( ENVCALL messageStr );
	env->DeleteLocalRef( ENVCALL channelIDStr );
	env->DeleteLocalRef( ENVCALL NotificationManagerClass );
	env->DeleteLocalRef( ENVCALL notificationServiceObj );
	env->DeleteLocalRef( ENVCALL notificationServiceName );

}

void HandleThisWindowTermination()
{
	suspended = 1;
}


uint32_t randomtexturedata[256*256];
uint32_t webviewdata[500*500];
char fromJSBuffer[128];

void CheckWebView( void * v )
{
	static int runno = 0;
	WebViewNativeActivityObject * wvn = (WebViewNativeActivityObject*)v;
	if( WebViewGetProgress( wvn ) != 100 ) return;

	runno++;
	if( runno == 1 )
	{
		// The attach (initial) message payload has no meaning.
		WebViewPostMessage( wvn, "", 1 );
	}
	else
	{
		// Invoke JavaScript, which calls a function to send a webmessage
		// back into C land.
		WebViewExecuteJavascript( wvn, "SendMessageToC();" );
		
		// Send a WebMessage into the JavaScript code.
		char st[128];
		sprintf( st, "Into JavaScript %d\n", runno );
		WebViewPostMessage( wvn, st, 0 );
	}
}

jobject g_attachLooper;

void SetupWebView( void * v )
{
	WebViewNativeActivityObject * wvn = (WebViewNativeActivityObject*)v;


	const struct JNINativeInterface * env = 0;
	const struct JNINativeInterface ** envptr = &env;
	const struct JNIInvokeInterface ** jniiptr = gapp->activity->vm;
	const struct JNIInvokeInterface * jnii = *jniiptr;

	jnii->AttachCurrentThread( jniiptr, &envptr, NULL);
	env = (*envptr);

	while( g_attachLooper == 0 ) usleep(1);
	WebViewCreate( wvn, "file:///android_asset/test.html", g_attachLooper, 500, 500 );
	//WebViewCreate( wvn, "about:blank", g_attachLooper, 500, 500 );
}


pthread_t jsthread;

void * JavscriptThread( void * v )
{
	const struct JNINativeInterface * env = 0;
	const struct JNINativeInterface ** envptr = &env;
	const struct JNIInvokeInterface ** jniiptr = gapp->activity->vm;
	const struct JNIInvokeInterface * jnii = *jniiptr;

	jnii->AttachCurrentThread( jniiptr, &envptr, NULL);
	env = (*envptr);

	// Create a looper on this thread...
	jclass LooperClass = env->FindClass(envptr, "android/os/Looper");
	jmethodID myLooperMethod = env->GetStaticMethodID(envptr, LooperClass, "myLooper", "()Landroid/os/Looper;");
	jobject thisLooper = env->CallStaticObjectMethod( envptr, LooperClass, myLooperMethod );
	if( !thisLooper )
	{
		jmethodID prepareMethod = env->GetStaticMethodID(envptr, LooperClass, "prepare", "()V");
		env->CallStaticVoidMethod( envptr, LooperClass, prepareMethod );
		thisLooper = env->CallStaticObjectMethod( envptr, LooperClass, myLooperMethod );
		g_attachLooper = env->NewGlobalRef(envptr, thisLooper);
	}

	jmethodID getQueueMethod = env->GetMethodID( envptr, LooperClass, "getQueue", "()Landroid/os/MessageQueue;" );
	jobject   lque = env->CallObjectMethod( envptr, g_attachLooper, getQueueMethod );

	jclass MessageQueueClass = env->FindClass(envptr, "android/os/MessageQueue");
	jmethodID nextMethod = env->GetMethodID( envptr, MessageQueueClass, "next", "()Landroid/os/Message;" );
	
	jclass MessageClass = env->FindClass(envptr, "android/os/Message");
	jfieldID objid = env->GetFieldID( envptr, MessageClass, "obj", "Ljava/lang/Object;" );
	jclass PairClass = env->FindClass(envptr, "android/util/Pair");
	jfieldID pairfirst  = env->GetFieldID( envptr, PairClass, "first", "Ljava/lang/Object;" );

	while(1)
	{
		// Instead of using Looper::loop(), we just call next on the looper object.
		jobject msg = env->CallObjectMethod( envptr, lque, nextMethod );
		jobject innerObj = env->GetObjectField( envptr, msg, objid );
		const char * name;
		jstring strObj;
		jclass innerClass;
		
		// Check Object Type
		{
			innerClass = env->GetObjectClass( envptr, innerObj );
			jmethodID mid = env->GetMethodID( envptr, innerClass, "getClass", "()Ljava/lang/Class;");
			jobject clsObj = env->CallObjectMethod( envptr, innerObj, mid );
			jclass clazzz = env->GetObjectClass( envptr, clsObj );
			mid = env->GetMethodID(envptr, clazzz, "getName", "()Ljava/lang/String;");
			strObj = (jstring)env->CallObjectMethod( envptr, clsObj, mid);
			name = env->GetStringUTFChars( envptr, strObj, 0);
			env->DeleteLocalRef( envptr, clsObj );
			env->DeleteLocalRef( envptr, clazzz );
		}

		if( strcmp( name, "z5" ) == 0 )
		{
			// Special, Some Androids (notably Meta Quest) use a different private message type.
			jfieldID mstrf  = env->GetFieldID( envptr, innerClass, "a", "[B" );
			jbyteArray jba = (jstring)env->GetObjectField(envptr, innerObj, mstrf );
			int len = env->GetArrayLength( envptr, jba );
			jboolean isCopy = 0;
			jbyte * bufferPtr = env->GetByteArrayElements(envptr, jba, &isCopy);

			if( len >= 6 )
			{
				const char *descr = (const char*)bufferPtr + 6;
				char tcpy[len-5];
				memcpy( tcpy, descr, len-6 );
				tcpy[len-6] = 0;
				snprintf( fromJSBuffer, sizeof( fromJSBuffer)-1, "WebMessage: %s\n", tcpy );

				env->DeleteLocalRef( envptr, jba );
			}
		}
		else
		{
			jobject MessagePayload = env->GetObjectField( envptr, innerObj, pairfirst );
			// MessagePayload is a org.chromium.content_public.browser.MessagePayload

			jclass mpclass = env->GetObjectClass( envptr, MessagePayload );

			// Get field "b" which is the web message payload.
			// If you are using binary sockets, it will be in `c` and be a byte array.
			jfieldID mstrf  = env->GetFieldID( envptr, mpclass, "b", "Ljava/lang/String;" );
			jstring strObjDescr = (jstring)env->GetObjectField(envptr, MessagePayload, mstrf );

			const char *descr = env->GetStringUTFChars( envptr, strObjDescr, 0);
			snprintf( fromJSBuffer, sizeof( fromJSBuffer)-1, "WebMessage: %s\n", descr );

			env->ReleaseStringUTFChars(envptr, strObjDescr, descr);
			env->DeleteLocalRef( envptr, strObjDescr );
			env->DeleteLocalRef( envptr, MessagePayload );
			env->DeleteLocalRef( envptr, mpclass );
		}
		env->ReleaseStringUTFChars(envptr, strObj, name);
		env->DeleteLocalRef( envptr, strObj );
		env->DeleteLocalRef( envptr, msg );
		env->DeleteLocalRef( envptr, innerObj );
		env->DeleteLocalRef( envptr, innerClass );
	}
}

void SetupJSThread()
{
	pthread_create( &jsthread, 0, JavscriptThread, 0 );
}

int main( int argc, char ** argv )
{
	int x, y;
	double ThisTime;
	double LastFPSTime = OGGetAbsoluteTime();

	CNFGBGColor = 0x000040ff;
	CNFGSetupFullscreen( "Test Bench", 0 );
	
	HandleWindowTermination = HandleThisWindowTermination;

	for( x = 0; x < HMX; x++ )
	for( y = 0; y < HMY; y++ )
	{
		Heightmap[x+y*HMX] = tdPerlin2D( x, y )*8.;
	}

	const char * assettext = "Not Found";
	AAsset * file = AAssetManager_open( gapp->activity->assetManager, "asset.txt", AASSET_MODE_BUFFER );
	if( file )
	{
		size_t fileLength = AAsset_getLength(file);
		char * temp = (char*)malloc( fileLength + 1);
		memcpy( temp, AAsset_getBuffer( file ), fileLength );
		temp[fileLength] = 0;
		assettext = temp;
	}

	SetupIMU();
	
	// Disabled, for now.
	//InitCNFAAndroid( AudioCallback, "A Name", SAMPLE_RATE, 0, 1, 0, SAMPLE_COUNT, 0, 0, 0 );

	SetupJSThread();

	// Create webview and wait for its completion
	RunCallbackOnUIThread( SetupWebView, &MyWebView );
	while( !MyWebView.WebViewObject ) usleep(1);

	while(1)
	{
		int i;
		iframeno++;

		if( iframeno == 200 )
		{
			MakeNotification( "default", "rawdraw alerts", "rawdraw", "Hit frame two hundred\nNew Line" );
		}

		CNFGHandleInput();
		AccCheck();

		if( suspended ) { usleep(50000); continue; }

		RunCallbackOnUIThread( (void(*)(void*))WebViewRequestRenderToCanvas, &MyWebView );
		RunCallbackOnUIThread( CheckWebView, &MyWebView );

		CNFGClearFrame();
		CNFGColor( 0xFFFFFFFF );
		CNFGGetDimensions( &screenx, &screeny );

		// Mesh in background
		CNFGSetLineWidth( 9 );
		DrawHeightmap();
		CNFGPenX = 0; CNFGPenY = 400;
		CNFGColor( 0xffffffff );
		CNFGDrawText( assettext, 15 );
		CNFGFlushRender();

		CNFGPenX = 0; CNFGPenY = 480;
		char st[50];
		sprintf( st, "%dx%d %d %d %d %d %d %d\n%d %d\n%5.2f %5.2f %5.2f %d", screenx, screeny, lastbuttonx, lastbuttony, lastmotionx, lastmotiony, lastkey, lastkeydown, lastbid, lastmask, accx, accy, accz, accs );
		CNFGDrawText( st, 10 );
		CNFGSetLineWidth( 2 );

		// Square behind text
		CNFGColor( 0x303030ff );
		CNFGTackRectangle( 600, 0, 950, 350);

		CNFGPenX = 10; CNFGPenY = 10;

		// Text
		CNFGColor( 0xffffffff );
		for( i = 0; i < 1; i++ )
		{
			int c;
			char tw[2] = { 0, 0 };
			for( c = 0; c < 256; c++ )
			{
				tw[0] = c;

				CNFGPenX = ( c % 16 ) * 20+606;
				CNFGPenY = ( c / 16 ) * 20+5;
				CNFGDrawText( tw, 4 );
			}
		}

		// Green triangles
		CNFGPenX = 0;
		CNFGPenY = 0;
		CNFGColor( 0x00FF00FF );

		for( i = 0; i < 400; i++ )
		{
			RDPoint pp[3];
			pp[0].x = (short)(50*sin((float)(i+iframeno)*.01) + (i%20)*30);
			pp[0].y = (short)(50*cos((float)(i+iframeno)*.01) + (i/20)*20)+700;
			pp[1].x = (short)(20*sin((float)(i+iframeno)*.01) + (i%20)*30);
			pp[1].y = (short)(50*cos((float)(i+iframeno)*.01) + (i/20)*20)+700;
			pp[2].x = (short)(10*sin((float)(i+iframeno)*.01) + (i%20)*30);
			pp[2].y = (short)(30*cos((float)(i+iframeno)*.01) + (i/20)*20)+700;
			CNFGTackPoly( pp, 3 );
		}

		// Last WebMessage
		CNFGColor( 0xFFFFFFFF );
		CNFGPenX = 0; CNFGPenY = 100;
		CNFGDrawText( fromJSBuffer, 6 );

		int x, y;
		for( y = 0; y < 256; y++ )
		for( x = 0; x < 256; x++ )
			randomtexturedata[x+y*256] = x | ((x*394543L+y*355+iframeno*3)<<8);
		CNFGBlitImage( randomtexturedata, 100, 600, 256, 256 );

		WebViewNativeGetPixels( &MyWebView, webviewdata, 500, 500 );
		CNFGBlitImage( webviewdata, 500, 640, 500, 500 );

		frames++;
		//On Android, CNFGSwapBuffers must be called, and CNFGUpdateScreenWithBitmap does not have an implied framebuffer swap.
		CNFGSwapBuffers();

		ThisTime = OGGetAbsoluteTime();
		if( ThisTime > LastFPSTime + 1 )
		{
			printf( "FPS: %d\n", frames );
			frames = 0;
			LastFPSTime+=1;
		}

	}

	return(0);
}

