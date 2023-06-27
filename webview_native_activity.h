#ifndef _WEBVIEW_NATIVE_ACTIVITY
#define _WEBVIEW_NATIVE_ACTIVITY

#include <android/native_activity.h>

extern struct android_app * gapp;

typedef struct
{
	jobject WebViewObject;
} WebViewNativeActivityObject;

void CreateWebViewTrigger( void * v );
void WebViewNativeGetPixels( WebViewNativeActivityObject * obj, void * pixel_data, int w, int h );


#ifdef WEBVIEW_NATIVE_ACTIVITY_IMPLEMENTATION


void CreateWebViewTrigger( void * v )
{
	while( !gapp ) usleep(1000);
	printf( "HANDLE!!! %p\n", v  );
	WebViewNativeActivityObject * w = (WebViewNativeActivityObject*)v;

	const struct JNINativeInterface * env = 0;
	const struct JNINativeInterface ** envptr = &env;
	printf( "GAPP: %p\n", gapp );
	const struct JNIInvokeInterface ** jniiptr = gapp->activity->vm;
	jobject clazz = gapp->activity->clazz;
	printf( "---> clazz: %p\n", clazz );
	const struct JNIInvokeInterface * jnii = *jniiptr;

	jnii->AttachCurrentThread( jniiptr, &envptr, NULL);
	env = (*envptr);

	ALooper_prepare(ALOOPER_PREPARE_ALLOW_NON_CALLBACKS);

	printf( "This Looper: %p\n", ALooper_forThread() );

	int oFD, oEvents;
	void * odat;
	ALooper_pollOnce( 100, &oFD, &oEvents, &odat );

	jclass clszz = env->GetObjectClass(envptr,clazz);

	jclass WebViewClass = env->FindClass(envptr, "android/webkit/WebView");
	//jclass niClass = env->FindClass(envptr, "android/app/NativeActivity");

	jclass activityClass = env->FindClass(envptr, "android/app/Activity");
	jmethodID activityGetContextMethod = env->GetMethodID(envptr, activityClass, "getApplicationContext", "()Landroid/content/Context;");
	jobject contextObject = env->CallObjectMethod(envptr, clazz, activityGetContextMethod);

	printf( "%p %p %p\n", activityClass, activityGetContextMethod, contextObject );

	// WebView(Context context)
    jmethodID WebViewConstructor = env->GetMethodID(envptr, WebViewClass, "<init>", "(Landroid/content/Context;)V");
	printf( "Getting object %p\n", w );
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

    jmethodID WebViewEvalJSMethod = env->GetMethodID(envptr, WebViewClass, "evaluateJavascript", "(Ljava/lang/String;Landroid/webkit/ValueCallback;)V");
	//evaluateJavascript(String script, ValueCallback<String> resultCallback) 
	jstring strjs = env->NewStringUTF( envptr, "document.write('<HTML><BODY>Test</BODY></HTML>');" );
	env->CallVoidMethod(envptr, w->WebViewObject, WebViewEvalJSMethod, strjs, 0 );

	jstring strjs2 = env->NewStringUTF( envptr, "document.title = 'x';" );
	env->CallVoidMethod(envptr, w->WebViewObject, WebViewEvalJSMethod, strjs2, 0 );


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

	jmethodID setContentViewMethod = env->GetMethodID(envptr, clszz, "setContentView", "(Landroid/view/View;)V");
	printf( "CVM %p\n", setContentViewMethod );
	env->CallVoidMethod(envptr,clazz, setContentViewMethod, w->WebViewObject );

}
 

void WebViewNativeGetPixels( WebViewNativeActivityObject * obj, void * pixel_data, int w, int h )
{
	const struct JNINativeInterface * env = 0;
	const struct JNINativeInterface ** envptr = &env;
	const struct JNIInvokeInterface ** jniiptr = gapp->activity->vm;
	jobject clazz = gapp->activity->clazz;
	//printf( "---> clszz: %p\n", clszz );
	const struct JNIInvokeInterface * jnii = *jniiptr;

	jnii->AttachCurrentThread( jniiptr, &envptr, NULL);
	env = (*envptr);

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
}

#endif
#endif

