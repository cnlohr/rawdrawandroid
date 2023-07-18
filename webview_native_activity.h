#ifndef _WEBVIEW_NATIVE_ACTIVITY
#define _WEBVIEW_NATIVE_ACTIVITY

#include <android/native_activity.h>

extern volatile jobject g_objRootView;

typedef struct
{
	jobject WebViewObject;
	jobjectArray MessageChannels;
} WebViewNativeActivityObject;

// Must be called from main thread
void WebViewCreate( WebViewNativeActivityObject * w, jobject useLooper );
void WebViewExecuteJavascript( WebViewNativeActivityObject * obj, const char * js );
void WebViewPostMessage( WebViewNativeActivityObject * obj, const char * mesg, int initial );
char * WebViewGetLastWindowTitle( WebViewNativeActivityObject * obj );

// Can be called from any thread.
void WebViewNativeGetPixels( WebViewNativeActivityObject * obj, void * pixel_data, int w, int h );

#ifdef WEBVIEW_NATIVE_ACTIVITY_IMPLEMENTATION

volatile jobject g_objRootView;

void WebViewCreate( WebViewNativeActivityObject * w, jobject useLooper )
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
	// Even more annoying - you can't pre-use loadUrl if you want to use message channels.
    jmethodID WebViewLoadBaseURLMethod = env->GetMethodID(envptr, WebViewClass, "loadDataWithBaseURL", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
	jstring strul = env->NewStringUTF( envptr, "http://example.com" );
	jstring strdata = env->NewStringUTF( envptr, "not-yet-loaded" );
	jstring strmime = env->NewStringUTF( envptr, "text/html" );
	jstring strencoding = env->NewStringUTF( envptr, "utf8" );
	jstring strhistoryurl = env->NewStringUTF( envptr, "" );
	env->CallVoidMethod(envptr, wvObj, WebViewLoadBaseURLMethod, strul, strdata, strmime, strencoding, strhistoryurl );
	env->DeleteLocalRef( envptr, strul );
	env->DeleteLocalRef( envptr, strdata );
	env->DeleteLocalRef( envptr, strmime );
	env->DeleteLocalRef( envptr, strencoding );
	env->DeleteLocalRef( envptr, strhistoryurl );

	// You have to switch to this to be able to run javascript code.
	jmethodID LoadURLMethod = env->GetMethodID(envptr, WebViewClass, "loadUrl", "(Ljava/lang/String;)V");
	jstring strjs = env->NewStringUTF( envptr, "about:blank" );
	env->CallVoidMethod(envptr, wvObj, LoadURLMethod, strjs );
	env->DeleteLocalRef( envptr, strjs );

	jmethodID WebViewGetSettingMethod = env->GetMethodID(envptr, WebViewClass, "getSettings", "()Landroid/webkit/WebSettings;");
	jobject websettings = env->CallObjectMethod(envptr, wvObj, WebViewGetSettingMethod );
	jclass WebSettingsClass = env->FindClass(envptr, "android/webkit/WebSettings");
	jmethodID setJavaScriptEnabledMethod = env->GetMethodID(envptr, WebSettingsClass, "setJavaScriptEnabled", "(Z)V");
	env->CallVoidMethod( envptr, websettings, setJavaScriptEnabledMethod, true );
	env->DeleteLocalRef( envptr, websettings );

/*	if( initialUrl )
	{
		jmethodID LoadURLMethod = env->GetMethodID(envptr, WebViewClass, "loadUrl", "(Ljava/lang/String;)V");
		jstring strjs = env->NewStringUTF( envptr, initialUrl );
		env->CallVoidMethod(envptr, wvObj, LoadURLMethod, strjs );
		env->DeleteLocalRef( envptr, strjs );
	}*/
	
    jmethodID setMeasuredDimensionMethodID = env->GetMethodID(envptr, WebViewClass, "setMeasuredDimension", "(II)V");
    env->CallVoidMethod(envptr, wvObj, setMeasuredDimensionMethodID, 500, 500 );

	jclass ViewClass = env->FindClass(envptr, "android/widget/LinearLayout");
	jmethodID addViewMethod = env->GetMethodID(envptr, ViewClass, "addView", "(Landroid/view/View;)V");
	env->CallVoidMethod( envptr, g_objRootView, addViewMethod, wvObj );


	jclass WebMessagePortClass = env->FindClass(envptr, "android/webkit/WebMessagePort" );
	jmethodID createWebMessageChannelMethod = env->GetMethodID(envptr, WebViewClass, "createWebMessageChannel", "()[Landroid/webkit/WebMessagePort;");
	jobjectArray messageChannels = env->CallObjectMethod( envptr, wvObj, createWebMessageChannelMethod );
	jobject mc0 = env->GetObjectArrayElement(envptr, messageChannels, 0);
	jobject mc1 = env->GetObjectArrayElement(envptr, messageChannels, 1);
	printf( "GEN PAIR: %p %p\n", mc0, mc1 );

//	jclass WebMessageCallbackClass = env->FindClass(envptr, "android/webkit/WebMessagePort/WebMessageCallback" );
//	jmethodID WebMessageCallbackObject = env->GetMethodID(envptr, WebMessageCallbackClass, "<init>", "()V");

/*
	jclass LooperClass = env->FindClass(envptr, "android/os/Looper");
	jmethodID myLooperMethod = env->GetStaticMethodID(envptr, LooperClass, "myLooper", "()Landroid/os/Looper;");
	jmethodID MainLooperMethod = env->GetStaticMethodID(envptr, LooperClass, "getMainLooper", "()Landroid/os/Looper;");
	jobject thisLooper = env->CallStaticObjectMethod( envptr, LooperClass, MainLooperMethod );
*/

	jclass HandlerClassType = env->FindClass(envptr, "android/os/Handler" );
	jmethodID HandlerObjectConstructor = env->GetMethodID(envptr, HandlerClassType, "<init>", "(Landroid/os/Looper;)V");
	jobject handlerObject = env->NewObject( envptr, HandlerClassType, HandlerObjectConstructor, useLooper );
	handlerObject = env->NewGlobalRef(envptr, handlerObject);
	printf(" USING LOOPER: %p <<<<<<<<<<<<<<<<<<<<<<<<<< >> Handler: %p\n", useLooper, handlerObject );

/*
	// Can't make one.  WebMessageCallbacks are abstract.
	jclass WMCallbackType = env->FindClass(envptr, "android/webkit/WebMessagePort$WebMessageCallback" );
	jmethodID MCObjectConstructor = env->GetMethodID(envptr, WMCallbackType, "<init>", "()V");
	jobject WMCObject = env->NewObject( envptr, WMCallbackType, MCObjectConstructor );
*/

	jmethodID setWebMessageCallbackMethod = env->GetMethodID( envptr, WebMessagePortClass, "setWebMessageCallback", "(Landroid/webkit/WebMessagePort$WebMessageCallback;Landroid/os/Handler;)V" );

	// Only can receive messages on MC0
	env->CallVoidMethod( envptr, mc0, setWebMessageCallbackMethod, 0, handlerObject );
	//XXX PICK UP HERE!!! CNL XXX

//	env->CallVoidMethod( envptr, mc1, setWebMessageCallbackMethod, WMCObject, handlerObject );
//	setWebMessageCallback
	//android.webkit.WebMessagePort

/*
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
*/


	w->WebViewObject = env->NewGlobalRef(envptr, wvObj);
	w->MessageChannels = env->NewGlobalRef(envptr, messageChannels);

	env->DeleteLocalRef( envptr, WebViewClass );
	env->DeleteLocalRef( envptr, activityClass );
	env->DeleteLocalRef( envptr, WebSettingsClass );
	env->DeleteLocalRef( envptr, ViewClass );
}

void WebViewPostMessage( WebViewNativeActivityObject * w, const char * mesg, int initial )
{
	const struct JNINativeInterface * env = 0;
	const struct JNINativeInterface ** envptr = &env;
	const struct JNIInvokeInterface ** jniiptr = gapp->activity->vm;
	const struct JNIInvokeInterface * jnii = *jniiptr;

	jnii->AttachCurrentThread( jniiptr, &envptr, NULL);
	env = (*envptr);

	jclass WebMessagePortClass = env->FindClass(envptr, "android/webkit/WebMessagePort" );
	jobject mc1 = env->GetObjectArrayElement(envptr, w->MessageChannels, 1);

/*
	printf( "MC: %p %p %p\n", w->MessageChannels, mc0, mc1 );
	jmethodID postMessageMethod = env->GetMethodID(envptr, WebMessagePortClass, "postMessage", "(Landroid/webkit/WebMessage;)V");

	jclass WebMessageClass = env->FindClass(envptr, "android/webkit/WebMessage" );
	jstring strjs = env->NewStringUTF( envptr, mesg );
    jmethodID WebMessageConstructor = env->GetMethodID(envptr, WebMessageClass, "<init>", "(Ljava/lang/String;)V");
	printf( "$$$$$$$$$$$$$$$$$$$$$$$$$ WMC: %p / %p\n", WebMessageConstructor, postMessageMethod );
	jobject newwm = env->NewObject(envptr, WebMessageClass, WebMessageConstructor, strjs );
*/

	if( initial )
	{
		//https://stackoverflow.com/questions/41753104/how-do-you-use-webmessageport-as-an-alternative-to-addjavascriptinterface
		jclass WebViewClass = env->FindClass(envptr, "android/webkit/WebView");
		jmethodID postMessageMethod = env->GetMethodID(envptr, WebViewClass, "postWebMessage", "(Landroid/webkit/WebMessage;Landroid/net/Uri;)V");

		jclass WebMessageClass = env->FindClass(envptr, "android/webkit/WebMessage" );
		jstring strjs = env->NewStringUTF( envptr, mesg );
		jmethodID WebMessageConstructor = env->GetMethodID(envptr, WebMessageClass, "<init>", "(Ljava/lang/String;[Landroid/webkit/WebMessagePort;)V");

		// Need to generate a new message channel array.
		printf( "ABOUT TO GEN ARRAY\n" );
		jobjectArray jsUseWebPorts = env->NewObjectArray( envptr, 1, WebMessagePortClass, mc1);
//		printf( "ADDING ARRAY\n" );
//		env->SetObjectArrayElement( envptr, jsUseWebPorts, 0, );
		printf( "ARRAY GENNED %p\n", mc1 );

		jobject newwm = env->NewObject(envptr, WebMessageClass, WebMessageConstructor, strjs, jsUseWebPorts );

		// Need Uri.EMPTY
		jclass UriClass = env->FindClass(envptr, "android/net/Uri" );
		jfieldID EmptyField = env->GetStaticFieldID( envptr, UriClass, "EMPTY", "Landroid/net/Uri;" );
		jobject EmptyURI = env->GetStaticObjectField( envptr, UriClass, EmptyField );

		env->CallVoidMethod( envptr, w->WebViewObject, postMessageMethod, newwm, EmptyURI );
		env->DeleteLocalRef( envptr, WebViewClass );
		env->DeleteLocalRef( envptr, WebMessageClass );
		env->DeleteLocalRef( envptr, jsUseWebPorts );

		env->DeleteLocalRef( envptr, newwm );
		env->DeleteLocalRef( envptr, strjs );
	}
	

	//XXX PICK UP HERE!!! CNL XXX
	//postMessage(WebMessage message)

	env->DeleteLocalRef( envptr, WebMessagePortClass );

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

	//WebView.evaluateJavascript(String script, ValueCallback<String> resultCallback) 
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
	char *nativeString = strdup( env->GetStringUTFChars(envptr, titleObject, 0) );
	env->DeleteLocalRef( envptr, titleObject );
	env->DeleteLocalRef( envptr, WebViewClass );

	return nativeString;
}


#endif
#endif

