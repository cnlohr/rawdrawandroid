//Copyright (c) 2011 <>< Charles Lohr - Under the MIT/x11 or NewBSD License you choose.

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "CNFGFunctions.h"
#include "os_generic.h"
#include "CNFG3D.h"
#include <GLES3/gl3.h>


unsigned frames = 0;
unsigned long iframeno = 0;

void HandleKey( int keycode, int bDown )
{
	if( keycode == 65307 ) exit( 0 );
	printf( "Key: %d -> %d\n", keycode, bDown );
}

void HandleButton( int x, int y, int button, int bDown )
{
	printf( "Button: %d,%d (%d) -> %d\n", x, y, button, bDown );
}

void HandleMotion( int x, int y, int mask )
{
//	printf( "Motion: %d,%d (%d)\n", x, y, mask );
}

#define HMX 132
#define HMY 132
short screenx, screeny;
float Heightmap[HMX*HMY];

void DrawHeightmap()
{
	int x, y;
	float fdt = ((iframeno++)%(360*10))/10.0;
	float eye[3] = { (float)sin(fdt*(3.14159/180.0))*30, (float)cos(fdt*(3.14159/180.0))*30, 30 };
	float at[3] = { 0,0, 0 };
	float up[3] = { 0,0, 1 };

	tdSetViewport( -1, -1, 1, 1, screenx, screeny );

	tdMode( tdPROJECTION );
	tdIdentity( gSMatrix );
	tdPerspective( 40, ((float)screenx)/((float)screeny), .1, 200., gSMatrix );

	tdMode( tdMODELVIEW );
	tdIdentity( gSMatrix );
	tdTranslate( gSMatrix, 0, 0, -40 );
	tdLookAt( gSMatrix, eye, at, up );

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
		float lightdir[3] = { 1, -1, 1 };
		float tmp1[3];
		float tmp2[3];

		RDPoint pto[6];

		pta[0] = tx+0; pta[1] = ty+0; pta[2] = Heightmap[(x+0)+(y+0)*HMX];
		ptb[0] = tx+1; ptb[1] = ty+0; ptb[2] = Heightmap[(x+1)+(y+0)*HMX];
		ptc[0] = tx+0; ptc[1] = ty+1; ptc[2] = Heightmap[(x+0)+(y+1)*HMX];
		ptd[0] = tx+1; ptd[1] = ty+1; ptd[2] = Heightmap[(x+1)+(y+1)*HMX];

		tdPSub( pta, ptb, tmp2 );
		tdPSub( ptc, ptb, tmp1 );
		tdCross( tmp1, tmp2, normal );

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

/*		if( pta[3] < -1 ) continue;
		if( ptb[3] < -1 ) continue;
		if( ptc[3] < -1 ) continue;
		if( ptd[3] < -1 ) continue;
*/

//		if( pta[2] < 0 ) continue;
//		if( ptb[2] < 0 ) continue;
//		if( ptc[2] < 0 ) continue;
//		if( ptd[2] < 0 ) continue;

		pto[0].x = pta[0]; pto[0].y = pta[1];
		pto[1].x = ptb[0]; pto[1].y = ptb[1];
		pto[2].x = ptd[0]; pto[2].y = ptd[1];

		pto[3].x = ptc[0]; pto[3].y = ptc[1];
		pto[4].x = ptd[0]; pto[4].y = ptd[1];
		pto[5].x = pta[0]; pto[5].y = pta[1];

//		CNFGColor(((x+y)&1)?0xFFFFFF:0x000000);

		float bright = tdDot( normal, lightdir );
		if( bright < 0 ) bright = 0;
		CNFGColor( (int)( bright * 50 ) );

//		CNFGTackPoly( &pto[0], 3 );		CNFGTackPoly( &pto[3], 3 );


		CNFGTackSegment( pta[0], pta[1], ptb[0], ptb[1] );
//		CNFGTackSegment( ptb[0], ptb[1], ptc[0], ptc[1] );
//		CNFGTackSegment( ptc[0], ptc[1], ptd[0], ptd[1] );
//		CNFGTackSegment( ptd[0], ptd[1], pta[0], pta[1] );
		CNFGTackSegment( pta[0], pta[1], ptc[0], ptc[1] );
//		CNFGTackSegment( ptd[0], ptd[1], ptb[0], ptb[1] );
		
	}
 
/*
	for( f = 0; f <= 6.28; f+=0.01 )
	{
		tdPSet( pta, cos( f ), sin(f), cos( f * 10. + ThisTime) );
		tdPSet( ptb, cos( f - 0.01 ), sin(f - 0.01), cos( (f-0.01) * 10. + ThisTime) );
	//			printf( "(%f, %f, %f) -> ", pta[0], pta[1], pta[2] );
		tdFinalPoint( pta, pta );
		tdFinalPoint( ptb, ptb );
	//			printf( "%f, %f, %f\n", pta[0], pta[1], pta[2] );
		CNFGTackSegment( pta[0], pta[1], ptb[0], ptb[1] );
	}

*/


}


void HandleDestroy()
{
	printf( "Destroying\n" );
	exit(10);
}


int main()
{
	int i, x, y;
	double ThisTime;
	double LastFPSTime = OGGetAbsoluteTime();
	double LastFrameTime = OGGetAbsoluteTime();
	double SecToWait;
	int linesegs = 0;

	CNFGBGColor = 0x800000;
	CNFGDialogColor = 0x444444;
	CNFGSetup( "Test Bench", 640, 480 );
	// CNFGSetupFullscreen( "Test Bench", 0 );

	for( x = 0; x < HMX; x++ )
	for( y = 0; y < HMY; y++ )
	{
		Heightmap[x+y*HMX] = tdPerlin2D( x, y )*8.;
	}

	while(1)
	{
		int i, pos;
		float f;
		iframeno++;
		RDPoint pto[3];

		CNFGHandleInput();

		CNFGClearFrame();
		CNFGColor( 0xFFFFFF );
		CNFGGetDimensions( &screenx, &screeny );

		// Mesh in background
		glLineWidth( 10.0 );
		DrawHeightmap();
		void FlushRender();
		FlushRender();
		glLineWidth( 2.0 );
/*

		pto[0].x = 100;
		pto[0].y = 100;
		pto[1].x = 200;
		pto[1].y = 100;
		pto[2].x = 100;
		pto[2].y = 200;
		CNFGTackPoly( &pto[0], 3 );

		CNFGColor( 0xFF00FF );
*/

/*		CNFGTackSegment( pto[0].x, pto[0].y, pto[1].x, pto[1].y );
		CNFGTackSegment( pto[1].x, pto[1].y, pto[2].x, pto[2].y );
		CNFGTackSegment( pto[2].x, pto[2].y, pto[0].x, pto[0].y );
*/

		// Square behind text
		CNFGDrawBox( 0, 0, 260, 260 );

		CNFGPenX = 10; CNFGPenY = 10;

		// Text
		pos = 0;
		CNFGColor( 0xffffff );
		for( i = 0; i < 1; i++ )
		{
			int c;
			char tw[2] = { 0, 0 };
			for( c = 0; c < 256; c++ )
			{
				tw[0] = c;

				CNFGPenX = ( c % 16 ) * 16+5;
				CNFGPenY = ( c / 16 ) * 16+5;
				CNFGDrawText( tw, 2 );
			}
		}

		// Green triangles
		CNFGPenX = 0;
		CNFGPenY = 0;

		for( i = 0; i < 400; i++ )
		{
			RDPoint pp[3];
			CNFGColor( 0x00FF00 );
			pp[0].x = (short)(50*sin((float)(i+iframeno)*.01) + (i%20)*30);
			pp[0].y = (short)(50*cos((float)(i+iframeno)*.01) + (i/20)*20);
			pp[1].x = (short)(20*sin((float)(i+iframeno)*.01) + (i%20)*30);
			pp[1].y = (short)(50*cos((float)(i+iframeno)*.01) + (i/20)*20);
			pp[2].x = (short)(10*sin((float)(i+iframeno)*.01) + (i%20)*30);
			pp[2].y = (short)(30*cos((float)(i+iframeno)*.01) + (i/20)*20);
			CNFGTackPoly( pp, 3 );
		}


		frames++;
		CNFGSwapBuffers();

		ThisTime = OGGetAbsoluteTime();
		if( ThisTime > LastFPSTime + 1 )
		{
			printf( "FPS: %d\n", frames );
			frames = 0;
			linesegs = 0;
			LastFPSTime+=1;
		}

		SecToWait = .016 - ( ThisTime - LastFrameTime );
		LastFrameTime += .016;
		if( SecToWait > 0 )
			OGUSleep( (int)( SecToWait * 1000000 ) );
	}

	return(0);
}

