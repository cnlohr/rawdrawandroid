//Copyright (c) 2011-2020 <>< Charles Lohr - Under the MIT/x11 or NewBSD License you choose.
// NO WARRANTY! NO GUARANTEE OF SUPPORT! USE AT YOUR OWN RISK

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "os_generic.h"
#include <GLES3/gl3.h>
#include <asset_manager.h>
#include <asset_manager_jni.h>
#include <android_native_app_glue.h>
#include <android/sensor.h>
#include <byteswap.h>
#include <errno.h>
#include <fcntl.h>
#include "CNFGAndroid.h"

#define CNFG_IMPLEMENTATION
#define CNFG3D

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

void SetupIMU()
{
	sm = ASensorManager_getInstance();
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

void HandleKey( int keycode, int bDown )
{
	lastkey = keycode;
	lastkeydown = bDown;
	if( keycode == 10 && !bDown ) { keyboard_up = 0; AndroidDisplayKeyboard( keyboard_up );  }

	if( keycode == 4 ) { AndroidSendToBack( 1 ); } //Handle Physical Back Button.
}

void HandleButton( int x, int y, int button, int bDown )
{
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

	float eye[3] = { (float)sin(mountainangle*(3.14159/180.0))*30*sin(mountainoffsety/100.), (float)cos(mountainangle*(3.14159/180.0))*30*sin(mountainoffsety/100.), 30*cos(mountainoffsety/100.) };
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


void HandleDestroy()
{
	printf( "Destroying\n" );
	exit(10);
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

uint32_t randomtexturedata[256*256];
uint32_t webviewdata[500*500];


jobject GlobalWebViewObject = 0;
jobject SurfaceViewObject;

void CheckWebViewTitle( void * v )
{
	static int runno = 0;
	runno++;
	WebViewNativeActivityObject * wvn = (WebViewNativeActivityObject*)v;
	char * s = WebViewGetLastWindowTitle(wvn);

	if( runno == 1 )
	{
		WebViewExecuteJavascript( wvn, "\
		let i = 0;\n\
		document.body.innerHTML = '6';\n\
		var port;\n\
		function pull() {\n\
			port.postMessage(\"pingpingpingping1\");\n\
			port.postMessage(\"pingpingpingping2\");\n\
			document.body.innerHTML = i++;\n\
		}\n\
		onmessage = function (e) { \
			port = e.ports[0]; \
			document.body.innerHTML = port; \
			port.onmessage = function (f) { \
				parse(f.data); \
			} \
		} \
		" );
		usleep(4000);
		WebViewPostMessage( wvn, "YYYYXXXXZZZZWWWW", 1 );
		usleep(4000);
	}

//	WebViewPostMessage( wvn, "YYYYXXXXZZZZWWWW", 0 );

	WebViewExecuteJavascript( wvn, "\
		/*document.body.innerHTML = '<HTML><BODY>' + i + '</BODY></HTML>';*/ \
		/*document.title = 'Javascript:' + i++;*/ \
		/*port.postMessage( 'zzzz' );*/ \
		pull(); \
	" );
	puts( s );
	free( s );
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
	WebViewCreate( wvn, g_attachLooper, 500, 500 );
}


pthread_t jsthread;

int msgpipeaux[2];


static int process_aux( int dummy1, int dummy2, void * dummy3 ) {
	// Can't trust parameters in UI thread callback.
//	printf( "####################################################################3\n" );
	uint8_t c = 0;
    int r = read(msgpipeaux[0], &c, 1);
	if( r>0 )
		printf( "##<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< %02x\n", c );
	return 1;
}

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

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Handle calling events on the UI thread.  You can get callbacks with RunCallbackOnUIThread.
    if (pipe(msgpipeaux)) {
        fprintf(stderr,"could not create pipe: %s", strerror(errno));
        return 0;
    }
    ALooper* looper = ALooper_forThread();

    ALooper_addFd(looper, msgpipeaux[0], 7, ALOOPER_EVENT_INPUT, process_aux, 0);  //NOTE: Cannot use NULL callback
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
		int events;
		struct android_poll_source* source;
		if (ALooper_pollAll( 1, 0, &events, (void**)&source) >= 0)
		{
			printf( "PolllO(((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((\n" );
		}

		// Instead of using Looper::loop(), we just call next on the looper object.

		jobject msg = env->CallObjectMethod( envptr, lque, nextMethod );
		jobject innerObj = env->GetObjectField( envptr, msg, objid );
		jobject MessagePayload = env->GetObjectField( envptr, innerObj, pairfirst );
		// MessagePayload is a org.chromium.content_public.browser.MessagePayload

		jclass mpclass = env->GetObjectClass( envptr, MessagePayload );
		/*
		printf( "JC: %p\n", mpclass );
		printf( "OBJECTS:\n" );
		PrintClassOfObject(MessagePayload);
		PrintObjectString( MessagePayload );
		printf( "SECOND: %p\n", MessageSecond);
		PrintClassOfObject(MessageSecond);
		PrintObjectString( MessageSecond );
		printf( "DUMPARINO\n" );
		DumpObjectClassProperties( MessagePayload );
		*/

		// Get field "b" which is the web message payload.
		jfieldID mstrf  = env->GetFieldID( envptr, mpclass, "b", "Ljava/lang/String;" );
		jstring strObjDescr = (jstring)env->GetObjectField(envptr, MessagePayload, mstrf );

		const char *descr = env->GetStringUTFChars( envptr, strObjDescr, 0);
		printf( "String Out: %s\n", descr );

        env->ReleaseStringUTFChars(envptr, strObjDescr, descr);

		env->DeleteLocalRef( envptr, mpclass );
		env->DeleteLocalRef( envptr, msg );
		env->DeleteLocalRef( envptr, strObjDescr );
		env->DeleteLocalRef( envptr, innerObj );
		env->DeleteLocalRef( envptr, MessagePayload );
	}
}

void SetupJSThread()
{
	pthread_create( &jsthread, 0, JavscriptThread, 0 );
}

int main()
{
	int x, y;
	double ThisTime;
	double LastFPSTime = OGGetAbsoluteTime();
	int linesegs = 0;

	CNFGBGColor = 0x000040ff;
	CNFGSetupFullscreen( "Test Bench", 0 );
	//CNFGSetup( "Test Bench", 0, 0 );

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
		char * temp = malloc( fileLength + 1);
		memcpy( temp, AAsset_getBuffer( file ), fileLength );
		temp[fileLength] = 0;
		assettext = temp;
	}
	SetupIMU();

	SetupJSThread();

	// Create webview and wait for its completion
	RunCallbackOnUIThread( SetupWebView, &MyWebView );
	while( !MyWebView.WebViewObject ) usleep(1);

	while(1)
	{
		int i, pos;
		iframeno++;

		CNFGHandleInput();
		AccCheck();

		if( suspended ) { usleep(50000); continue; }

		RunCallbackOnUIThread( (void(*)(void*))WebViewRequestRenderToCanvas, &MyWebView );
		RunCallbackOnUIThread( CheckWebViewTitle, &MyWebView );

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
		pos = 0;
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

		for( i = 0; i < 400; i++ )
		{
			RDPoint pp[3];
			CNFGColor( 0x00FF00FF );
			pp[0].x = (short)(50*sin((float)(i+iframeno)*.01) + (i%20)*30);
			pp[0].y = (short)(50*cos((float)(i+iframeno)*.01) + (i/20)*20)+700;
			pp[1].x = (short)(20*sin((float)(i+iframeno)*.01) + (i%20)*30);
			pp[1].y = (short)(50*cos((float)(i+iframeno)*.01) + (i/20)*20)+700;
			pp[2].x = (short)(10*sin((float)(i+iframeno)*.01) + (i%20)*30);
			pp[2].y = (short)(30*cos((float)(i+iframeno)*.01) + (i/20)*20)+700;
			CNFGTackPoly( pp, 3 );
		}

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
			linesegs = 0;
			LastFPSTime+=1;
		}

	}

	return(0);
}

