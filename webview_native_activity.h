#ifndef _WEBVIEW_NATIVE_ACTIVITY
#define _WEBVIEW_NATIVE_ACTIVITY

#include <android/native_activity.h>

extern struct android_app * tapp;
extern volatile jobject g_objRootView;

typedef struct
{
	jobject WebViewObject;
} WebViewNativeActivityObject;

void CreateWebViewTrigger( void * v );
void WebViewNativeGetPixels( WebViewNativeActivityObject * obj, void * pixel_data, int w, int h );
void GenWebViewAttachPoint( struct android_app * papp );
void LooperCheck( struct android_app * app, const char * name );

#ifdef WEBVIEW_NATIVE_ACTIVITY_IMPLEMENTATION

volatile jobject g_objRootView;
struct android_app * tapp;

void LooperCheck( struct android_app * app, const char * name )
{
	printf( "PRELOOPERCHECK %s %p\n", name, gettid() );
	const struct JNINativeInterface * env = 0;
	const struct JNINativeInterface ** envptr = &env;
	const struct JNIInvokeInterface ** jniiptr = app->activity->vm;
	jobject clazz = app->activity->clazz;
	const struct JNIInvokeInterface * jnii = *jniiptr;
	jnii->AttachCurrentThread( jniiptr, &envptr, NULL);
	env = (*envptr);

	jclass LooperClass = env->FindClass(envptr, "android/os/Looper");
	jmethodID myLooperMethod = env->GetStaticMethodID(envptr, LooperClass, "myLooper", "()Landroid/os/Looper;");
	jmethodID PrepareMethod = env->GetStaticMethodID(envptr, LooperClass, "prepare", "()V");

	jmethodID MainLooperMethod = env->GetStaticMethodID(envptr, LooperClass, "getMainLooper", "()Landroid/os/Looper;");

	jobject myLooper = env->CallStaticObjectMethod( envptr, LooperClass, myLooperMethod );
	jobject mainLooper = env->CallStaticObjectMethod( envptr, LooperClass, MainLooperMethod );
	printf( ":::::::::::::::::::::::::::::::::::::::LOOPER CHECK %s My:%p Main:%p\n", name, myLooper, mainLooper );

//	jnii->DetachCurrentThread( jniiptr );

}

void NextTestFunctionFromMain()
{
/*
printf( "SYMBOL\n" );
	const struct JNINativeInterface * env = 0;
	const struct JNINativeInterface ** envptr = &env;
	const struct JNIInvokeInterface ** jniiptr = gapp->activity->vm;
	jobject clazz = gapp->activity->clazz;
	const struct JNIInvokeInterface * jnii = *jniiptr;
printf( "PREATTACH\n" );
	jnii->AttachCurrentThread( jniiptr, &envptr, NULL);
	env = (*envptr);
printf( "GEN STARTTT*******************************************1\n" ); 
	jclass ViewClass = env->FindClass(envptr, "android/widget/LinearLayout");
    jmethodID ViewConstructor = env->GetMethodID(envptr, ViewClass, "<init>", "(Landroid/content/Context;)V");
printf( "GEN STARTTT*******************************************2\n" );
	jclass activityClass = env->FindClass(envptr, "android/app/Activity");
	jmethodID activityGetContextMethod = env->GetMethodID(envptr, activityClass, "getApplicationContext", "()Landroid/content/Context;");
	jobject contextObject = env->CallObjectMethod(envptr, clazz, activityGetContextMethod);
printf( "CTO: %p %p %p\n", activityClass, activityGetContextMethod, contextObject );
	jobject jv = env->NewObject(envptr, ViewClass, ViewConstructor, contextObject );
	g_objRootView = env->NewGlobalRef(envptr, jv);
printf( "JRV: %p\n", g_objRootView ); 
printf( "GEN STARTTT*******************************************3\n" );
	jnii->DetachCurrentThread( jniiptr );
printf( "DEEEEEEEEEEEEEEEEEEEEEEEETACH\n" );
*/
}

void GenWebViewAttachPoint( struct android_app * papp )
{
	tapp = papp;
printf( "PENDING\n" );
//	while(!g_objRootView);
printf( "SYMBOL\n" );
	const struct JNINativeInterface * env = 0;
	const struct JNINativeInterface ** envptr = &env;
	const struct JNIInvokeInterface ** jniiptr = papp->activity->vm;
	jobject clazz = papp->activity->clazz;
	const struct JNIInvokeInterface * jnii = *jniiptr;
printf( "REEEEEEEEEEEEEEEEEEATTACH\n" );
	jnii->AttachCurrentThread( jniiptr, &envptr, NULL);
	env = (*envptr);
printf( "OOOOGEN STARTTT*******************************************1\n" ); 




	jclass ViewClass = env->FindClass(envptr, "android/widget/LinearLayout");
    jmethodID ViewConstructor = env->GetMethodID(envptr, ViewClass, "<init>", "(Landroid/content/Context;)V");
printf( "OOOGEN STARTTT*******************************************2\n" );
	jclass activityClass = env->FindClass(envptr, "android/app/Activity");
	jmethodID activityGetContextMethod = env->GetMethodID(envptr, activityClass, "getApplicationContext", "()Landroid/content/Context;");
	jobject contextObject = env->CallObjectMethod(envptr, clazz, activityGetContextMethod);
printf( "OOOCTO: %p %p %p\n", activityClass, activityGetContextMethod, contextObject );
	jobject jv = env->NewObject(envptr, ViewClass, ViewConstructor, contextObject );
printf( "OOOJRV: %p\n", jv ); 
	g_objRootView = env->NewGlobalRef(envptr, jv);
printf( "OOOGEN STARTTT*******************************************3\n" );

	jclass clszz = env->GetObjectClass(envptr,clazz);
	jmethodID setContentViewMethod = env->GetMethodID(envptr, clszz, "setContentView", "(Landroid/view/View;)V");
	printf( "ROOT VIEW OBJECT %p\n", g_objRootView );
	env->CallVoidMethod(envptr,clazz, setContentViewMethod, g_objRootView );
}

void CreateWebViewTrigger( void * v )
{
	static int jj;
	tapp = gapp;
	//while( !tapp ) usleep(1000);
	printf( "HANDLE!!! %p\n", v  );
	WebViewNativeActivityObject * w = (WebViewNativeActivityObject*)v;

	const struct JNINativeInterface * env = 0;
	const struct JNINativeInterface ** envptr = &env;
	printf( "GAPP: %p\n", tapp );
	const struct JNIInvokeInterface ** jniiptr = tapp->activity->vm;
	jobject clazz = tapp->activity->clazz;
	printf( "---> clazz: %p\n", clazz );
	const struct JNIInvokeInterface * jnii = *jniiptr;

	jnii->AttachCurrentThread( jniiptr, &envptr, NULL);
	env = (*envptr);

//	ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);

	printf( "This Looper: %p\n", ALooper_forThread() );

	{
		jclass LooperClass = env->FindClass(envptr, "android/os/Looper");
		jmethodID myLooperMethod = env->GetStaticMethodID(envptr, LooperClass, "myLooper", "()Landroid/os/Looper;");
		jmethodID PrepareMethod = env->GetStaticMethodID(envptr, LooperClass, "prepare", "()V");

		jmethodID MainLooperMethod = env->GetStaticMethodID(envptr, LooperClass, "getMainLooper", "()Landroid/os/Looper;");

		jobject myLooper = env->CallStaticObjectMethod( envptr, LooperClass, myLooperMethod );
		jobject mainLooper = env->CallStaticObjectMethod( envptr, LooperClass, MainLooperMethod );
		if( !myLooper )
		{
			printf( "LOOPER OBJECT NOT PREPARED ON CURRENT THREAD. PREPARING\n" );
			env->CallStaticVoidMethod( envptr, LooperClass, PrepareMethod );
			myLooper = env->CallStaticObjectMethod( envptr, LooperClass, myLooperMethod );
		}				
		printf( "WEBVIEW GEN LOOPER OBJECT:::::::::::::::: %p %p / MAIN: %p\n", myLooperMethod, myLooper, mainLooper );
	}

	if( jj == 0 )
	{
		printf( "GGGGGGGGGGG1\n" );GenWebViewAttachPoint(gapp); printf( "GGGGGGGGGGG2\n" );
		jj = 1;
	}
	// Using looper


	int oFD, oEvents;
	void * odat;
	//ALooper_pollOnce( 100, &oFD, &oEvents, &odat );

	jclass clszz = env->GetObjectClass(envptr,clazz);

	jclass WebViewClass = env->FindClass(envptr, "android/webkit/WebView");
	//jclass niClass = env->FindClass(envptr, "android/app/NativeActivity");

	jclass activityClass = env->FindClass(envptr, "android/app/Activity");
	jmethodID activityGetContextMethod = env->GetMethodID(envptr, activityClass, "getApplicationContext", "()Landroid/content/Context;");
	jobject contextObject = env->CallObjectMethod(envptr, clazz, activityGetContextMethod);

	printf( "%p %p %p\n", activityClass, activityGetContextMethod, contextObject );

	// WebView(Context context)
    jmethodID WebViewConstructor = env->GetMethodID(envptr, WebViewClass, "<init>", "(Landroid/content/Context;)V");
	printf( "Getting object %p // Constructor: %p CONTEXT: %p\n", w, WebViewConstructor, contextObject );
	w->WebViewObject = env->NewObject(envptr, WebViewClass, WebViewConstructor, contextObject );
	printf( "WVVV: %p\n", w->WebViewObject );
    jmethodID WebViewLoadURLMethod = env->GetMethodID(envptr, WebViewClass, "loadUrl", "(Ljava/lang/String;)V");
	printf( "******** URL: %p %p\n", w->WebViewObject, WebViewLoadURLMethod );

	jstring strul = env->NewStringUTF( envptr, "about:blank" );
	env->CallVoidMethod(envptr, w->WebViewObject, WebViewLoadURLMethod, strul );

//	jclass jc = env->FindClass( envptr, "android/webkit/ValueCallback");
//    jmethodID jc_constructor = env->GetMethodID(envptr, jc, "<init>", "()Landroid/webkit/ValueCallback;");
//	printf( "=============================== %p %p\n", jc, jc_constructor );

///////////////// XXX XXX XXX TODO PICK UP HERE!

	if( 1 )
	{
		jmethodID WebViewEvalJSMethod = env->GetMethodID(envptr, WebViewClass, "evaluateJavascript", "(Ljava/lang/String;Landroid/webkit/ValueCallback;)V");
		//evaluateJavascript(String script, ValueCallback<String> resultCallback) 
		jstring strjs = env->NewStringUTF( envptr, "document.write('<HTML><BODY>Test</BODY></HTML>');" );
		env->CallVoidMethod(envptr, w->WebViewObject, WebViewEvalJSMethod, strjs, 0 );
	}
	else
	{
		jmethodID LoadURLMethod = env->GetMethodID(envptr, WebViewClass, "loadUrl", "(Ljava/lang/String;)V");
		jstring strjs = env->NewStringUTF( envptr, "https://en.wikipedia.com" );
		env->CallVoidMethod(envptr, w->WebViewObject, LoadURLMethod, strjs );
	}

	//jstring strjs2 = env->NewStringUTF( envptr, "document.title = 'x';" );
	//env->CallVoidMethod(envptr, w->WebViewObject, WebViewEvalJSMethod, strjs2, 0 );


	printf( "WEBBBBBB %p %p %p\n", WebViewClass, WebViewConstructor, w->WebViewObject );

	jmethodID getTitle = env->GetMethodID(envptr, WebViewClass, "getTitle", "()Ljava/lang/String;");
	jobject titleObject = env->CallObjectMethod( envptr, w->WebViewObject, getTitle );
	const char *nativeString = env->GetStringUTFChars(envptr, titleObject, 0);
	printf( "TITLE: %s\n",nativeString );
	env->DeleteLocalRef( envptr, titleObject );
	
	
    jmethodID setMeasuredDimensionMethodID = env->GetMethodID(envptr, WebViewClass, "setMeasuredDimension", "(II)V");
    env->CallVoidMethod(envptr, w->WebViewObject, setMeasuredDimensionMethodID, 500, 500 );
    
	w->WebViewObject = env->NewGlobalRef(envptr, w->WebViewObject);


//			jnii->DetachCurrentThread( jniiptr );

/*
	jmethodID setContentViewMethod = env->GetMethodID(envptr, activityClass, "setContentView", "(Landroid/view/View;)V");
	printf( "CVM %p\n", setContentViewMethod );
	env->CallVoidMethod(envptr, clazz, setContentViewMethod, WebViewObject );

	// Let's try to make a new view. --> NOPE!

	jclass SurfaceViewClass = env->FindClass(envptr, "android/opengl/GLSurfaceView");
    jmethodID SurfaceViewConstructor = env->GetMethodID(envptr, SurfaceViewClass, "<init>", "(Landroid/content/Context;)V");
	SurfaceViewObject = env->NewObject(envptr, SurfaceViewClass, SurfaceViewConstructor, contextObject );

	jmethodID addViewMethod = env->GetMethodID(envptr, SurfaceViewClass, "addView", "(Landroid/view/View;)V");
	env->CallVoidMethod(envptr, SurfaceViewObject, addViewMethod, WebViewObject );
	//jmethodID setContentViewMethod = env->GetMethodID(envptr, activityClass, "setContentView", "(Landroid/view/View;)V");
	printf( "CVM %p %p\n", setContentViewMethod, SurfaceViewObject );
	//env->CallVoidMethod(envptr, clazz, setContentViewMethod, WebViewObject );

	printf( "SurfaceView: %p\n", SurfaceViewObject);
*/


/*
	jmethodID setContentViewMethod = env->GetMethodID(envptr, clszz, "setContentView", "(Landroid/view/View;)V");
	printf( "CVM %p\n", setContentViewMethod );
	env->CallVoidMethod(envptr,clazz, setContentViewMethod, w->WebViewObject );
*/



	jclass ViewClass = env->FindClass(envptr, "android/widget/LinearLayout");
	jmethodID addViewMethod = env->GetMethodID(envptr, ViewClass, "addView", "(Landroid/view/View;)V");
	printf( "ADDVIEW: %p\n", addViewMethod );
	env->CallVoidMethod( envptr, g_objRootView, addViewMethod, w->WebViewObject );
	printf( "VIEWWWWW: %p %p %p\n", g_objRootView, addViewMethod, w->WebViewObject );

//	printf("NODETACH\n" );jnii->DetachCurrentThread( jniiptr ); printf( "DEATCHDONE\n" );

printf( "VIEW ADD COMPLETE!!!!!!!!!!!!!!\n" );
	/*
g_objRootView

    jmethodID ViewConstructor = env->GetMethodID(envptr, ViewClass, "<init>", "(Landroid/content/Context;)V");
printf( "GEN STARTTT*******************************************2\n" );
	jclass activityClass = env->FindClass(envptr, "android/app/Activity");
	jobject contextO	bject = env->CallObjectMethod(envptr, clazz, activityGetContextMethod);
printf( "CTO: %p %p %p\n", activityClass, activityGetContextMethod, contextObject );
	 = env->NewObject(envptr, ViewClass, ViewConstructor, contextObject );
printf( "JRV: %p\n", g_objRootView ); 
      parentLayout.addView(textView);
*/
printf( "DETATCCCHCEDDD\n" );
}
 

void WebViewNativeGetPixels( WebViewNativeActivityObject * obj, void * pixel_data, int w, int h )
{
	const struct JNINativeInterface * env = 0;
	const struct JNINativeInterface ** envptr = &env;
	const struct JNIInvokeInterface ** jniiptr = tapp->activity->vm;
	jobject clazz = tapp->activity->clazz;
	//printf( "---> clszz: %p\n", clszz );
	const struct JNIInvokeInterface * jnii = *jniiptr;

	jnii->AttachCurrentThread( jniiptr, &envptr, NULL);
	env = (*envptr);

			jclass LooperClass = env->FindClass(envptr, "android/os/Looper");
			jmethodID myLooperMethod = env->GetStaticMethodID(envptr, LooperClass, "myLooper", "()Landroid/os/Looper;");
			jobject myLooper = env->CallStaticObjectMethod( envptr, LooperClass, myLooperMethod );
			//printf( "MAIN LOOPER OBJECT:::::::::::::::: %p %p\n", myLooperMethod, myLooper );


	jclass SurfaceViewClass = env->FindClass(envptr, "android/view/SurfaceView");
	jclass PictureClass = env->FindClass(envptr, "android/graphics/Picture");
	jclass CanvasClass = env->FindClass(envptr, "android/graphics/Canvas");
	jclass BitmapClass = env->FindClass(envptr, "android/graphics/Bitmap");
	jclass WebViewClass = env->FindClass(envptr, "android/webkit/WebView");
	jmethodID createBitmap = env->GetStaticMethodID(envptr, BitmapClass, "createBitmap", "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
	jmethodID drawMethod = env->GetMethodID(envptr, WebViewClass, "draw", "(Landroid/graphics/Canvas;)V");
	jclass bmpCfgCls = env->FindClass(envptr, "android/graphics/Bitmap$Config");
	jstring bitmap_mode = env->NewStringUTF(envptr, "ARGB_8888");
	jmethodID bmpClsValueOfMid = env->GetStaticMethodID(envptr, bmpCfgCls, "valueOf", "(Ljava/lang/String;)Landroid/graphics/Bitmap$Config;");
	jobject jBmpCfg = env->CallStaticObjectMethod(envptr, bmpCfgCls, bmpClsValueOfMid, bitmap_mode);
	jobject bitmap = env->CallStaticObjectMethod( envptr, BitmapClass, createBitmap, w, h, jBmpCfg );
	jmethodID canvasConstructor = env->GetMethodID(envptr, CanvasClass, "<init>", "(Landroid/graphics/Bitmap;)V");
	jobject canvas = env->NewObject(envptr, CanvasClass, canvasConstructor, bitmap );
	env->CallVoidMethod( envptr, obj->WebViewObject, drawMethod, canvas );

	jobject buffer = env->NewDirectByteBuffer(envptr, pixel_data, w*h*4 );

	jmethodID copyPixelsBufferID = env->GetMethodID( envptr, BitmapClass, "copyPixelsToBuffer", "(Ljava/nio/Buffer;)V" );
	env->CallVoidMethod( envptr, bitmap, copyPixelsBufferID, buffer );




	env->DeleteLocalRef( envptr, SurfaceViewClass );
	env->DeleteLocalRef( envptr, PictureClass );
	env->DeleteLocalRef( envptr, CanvasClass );
	env->DeleteLocalRef( envptr, BitmapClass );
	env->DeleteLocalRef( envptr, WebViewClass );
	env->DeleteLocalRef( envptr, bmpCfgCls );
	env->DeleteLocalRef( envptr, bitmap_mode );
	env->DeleteLocalRef( envptr, bitmap );
	env->DeleteLocalRef( envptr, canvas );
	env->DeleteLocalRef( envptr, jBmpCfg );
	env->DeleteLocalRef( envptr, buffer );

	jnii->DetachCurrentThread( jniiptr );
}

#endif
#endif

