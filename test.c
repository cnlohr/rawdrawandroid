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
#include "CNFGAndroid.h"

#define CNFG_IMPLEMENTATION
#define CNFG3D

#include "CNFG.h"

float mountainangle;
float mountainoffsetx;
float mountainoffsety;

ASensorManager * sm;
const ASensor * as;
bool no_sensor_for_gyro = false;
ASensorEventQueue* aeq;
ALooper * l;


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

	while(1)
	{
		int i, pos;
		iframeno++;

		CNFGHandleInput();
		AccCheck();

		if( suspended ) { usleep(50000); continue; }

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


/*		CNFGTackSegment( pto[0].x, pto[0].y, pto[1].x, pto[1].y );
		CNFGTackSegment( pto[1].x, pto[1].y, pto[2].x, pto[2].y );
		CNFGTackSegment( pto[2].x, pto[2].y, pto[0].x, pto[0].y );
*/

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
			randomtexturedata[x+y*256] = x | ((x*394543L+y*355+iframeno)<<8);
		CNFGBlitImage( randomtexturedata, 100, 600, 256, 256 );

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

