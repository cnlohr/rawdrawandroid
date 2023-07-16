#ifndef _WEBVIEW_NATIVE_ACTIVITY
#define _WEBVIEW_NATIVE_ACTIVITY

#include <android/native_activity.h>

extern volatile jobject g_objRootView;

typedef struct
{
	jobject WebViewObject;
} WebViewNativeActivityObject;

// Must be called from main thread
void WebViewCreate( WebViewNativeActivityObject * w, const char * initialUrl );
void WebViewExecuteJavascript( WebViewNativeActivityObject * obj, const char * js );
char * WebViewGetLastWindowTitle( WebViewNativeActivityObject * obj );

// Can be called from any thread.
void WebViewNativeGetPixels( WebViewNativeActivityObject * obj, void * pixel_data, int w, int h );


#ifdef WEBVIEW_NATIVE_ACTIVITY_IMPLEMENTATION

volatile jobject g_objRootView;

void WebViewCreate( WebViewNativeActivityObject * w, const char * initialUrl )
{
	const struct JNINativeInterface * env = 0;
	const struct JNINativeInterface ** envptr = &env;
	const struct JNIInvokeInterface ** jniiptr = gapp->activity->vm;
	jobject clazz = gapp->activity->clazz;
	const struct JNIInvokeInterface * jnii = *jniiptr;

	jnii->AttachCurrentThread( jniiptr, &envptr, NULL);
	env = (*envptr);

	if( g_objRootView == 0 )
	{
		jclass ViewClass = env->FindClass(envptr, "android/widget/LinearLayout");
		jmethodID ViewConstructor = env->GetMethodID(envptr, ViewClass, "<init>", "(Landroid/content/Context;)V");
		jclass activityClass = env->FindClass(envptr, "android/app/Activity");
		jmethodID activityGetContextMethod = env->GetMethodID(envptr, activityClass, "getApplicationContext", "()Landroid/content/Context;");
		jobject contextObject = env->CallObjectMethod(envptr, clazz, activityGetContextMethod);
		jobject jv = env->NewObject(envptr, ViewClass, ViewConstructor, contextObject );
		g_objRootView = env->NewGlobalRef(envptr, jv);

		jclass clszz = env->GetObjectClass(envptr,clazz);
		jmethodID setContentViewMethod = env->GetMethodID(envptr, clszz, "setContentView", "(Landroid/view/View;)V");
		env->CallVoidMethod(envptr,clazz, setContentViewMethod, g_objRootView );
	}

	jclass WebViewClass = env->FindClass(envptr, "android/webkit/WebView");
	jclass activityClass = env->FindClass(envptr, "android/app/Activity");
	jmethodID activityGetContextMethod = env->GetMethodID(envptr, activityClass, "getApplicationContext", "()Landroid/content/Context;");
	jobject contextObject = env->CallObjectMethod(envptr, clazz, activityGetContextMethod);

    jmethodID WebViewConstructor = env->GetMethodID(envptr, WebViewClass, "<init>", "(Landroid/content/Context;)V");
	jobject wvObj = env->NewObject(envptr, WebViewClass, WebViewConstructor, contextObject );

	// Unknown reason why - if you don't first load about:blank, it sometimes doesn't render right?
    jmethodID WebViewLoadURLMethod = env->GetMethodID(envptr, WebViewClass, "loadUrl", "(Ljava/lang/String;)V");
	jstring strul = env->NewStringUTF( envptr, "about:blank" );
	env->CallVoidMethod(envptr, wvObj, WebViewLoadURLMethod, strul );
	env->DeleteLocalRef( envptr, strul );

	jmethodID WebViewGetSettingMethod = env->GetMethodID(envptr, WebViewClass, "getSettings", "()Landroid/webkit/WebSettings;");
	jobject websettings = env->CallObjectMethod(envptr, wvObj, WebViewGetSettingMethod );
	jclass WebSettingsClass = env->FindClass(envptr, "android/webkit/WebSettings");
	jmethodID setJavaScriptEnabledMethod = env->GetMethodID(envptr, WebSettingsClass, "setJavaScriptEnabled", "(Z)V");
	env->CallVoidMethod( envptr, websettings, setJavaScriptEnabledMethod, true );
	env->DeleteLocalRef( envptr, websettings );

	if( initialUrl )
	{
		jmethodID LoadURLMethod = env->GetMethodID(envptr, WebViewClass, "loadUrl", "(Ljava/lang/String;)V");
		jstring strjs = env->NewStringUTF( envptr, initialUrl );
		env->CallVoidMethod(envptr, wvObj, LoadURLMethod, strjs );
		env->DeleteLocalRef( envptr, strjs );
	}
	
    jmethodID setMeasuredDimensionMethodID = env->GetMethodID(envptr, WebViewClass, "setMeasuredDimension", "(II)V");
    env->CallVoidMethod(envptr, wvObj, setMeasuredDimensionMethodID, 500, 500 );

	jclass ViewClass = env->FindClass(envptr, "android/widget/LinearLayout");
	jmethodID addViewMethod = env->GetMethodID(envptr, ViewClass, "addView", "(Landroid/view/View;)V");
	env->CallVoidMethod( envptr, g_objRootView, addViewMethod, wvObj );

	w->WebViewObject = env->NewGlobalRef(envptr, wvObj);

	env->DeleteLocalRef( envptr, WebViewClass );
	env->DeleteLocalRef( envptr, activityClass );
	env->DeleteLocalRef( envptr, WebSettingsClass );
	env->DeleteLocalRef( envptr, ViewClass );
}
 

void WebViewNativeGetPixels( WebViewNativeActivityObject * obj, void * pixel_data, int w, int h )
{
	const struct JNINativeInterface * env = 0;
	const struct JNINativeInterface ** envptr = &env;
	const struct JNIInvokeInterface ** jniiptr = gapp->activity->vm;
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

	jnii->DetachCurrentThread( jniiptr );
}

void WebViewExecuteJavascript( WebViewNativeActivityObject * obj, const char * js )
{
	const struct JNINativeInterface * env = 0;
	const struct JNINativeInterface ** envptr = &env;
	const struct JNIInvokeInterface ** jniiptr = gapp->activity->vm;
	const struct JNIInvokeInterface * jnii = *jniiptr;
	jnii->AttachCurrentThread( jniiptr, &envptr, NULL);
	env = (*envptr);


	jclass WebViewClass = env->FindClass(envptr, "android/webkit/WebView");
	jmethodID WebViewEvalJSMethod = env->GetMethodID(envptr, WebViewClass, "evaluateJavascript", "(Ljava/lang/String;Landroid/webkit/ValueCallback;)V");
	//evaluateJavascript(String script, ValueCallback<String> resultCallback) 
	jstring strjs = env->NewStringUTF( envptr, js );
	env->CallVoidMethod( envptr, obj->WebViewObject, WebViewEvalJSMethod, strjs, 0 );
	env->DeleteLocalRef( envptr, WebViewClass );
	env->DeleteLocalRef( envptr, strjs );
}

char * WebViewGetLastWindowTitle( WebViewNativeActivityObject * obj )
{
	const struct JNINativeInterface * env = 0;
	const struct JNINativeInterface ** envptr = &env;
	const struct JNIInvokeInterface ** jniiptr = gapp->activity->vm;
	const struct JNIInvokeInterface * jnii = *jniiptr;
	jnii->AttachCurrentThread( jniiptr, &envptr, NULL);
	env = (*envptr);

	jclass WebViewClass = env->FindClass(envptr, "android/webkit/WebView");
	jmethodID getTitle = env->GetMethodID(envptr, WebViewClass, "getTitle", "()Ljava/lang/String;");
	jobject titleObject = env->CallObjectMethod( envptr, obj->WebViewObject, getTitle );
	const char *nativeString = strdup( env->GetStringUTFChars(envptr, titleObject, 0) );
	env->DeleteLocalRef( envptr, titleObject );
	env->DeleteLocalRef( envptr, WebViewClass );


	return nativeString;
}


#endif
#endif

