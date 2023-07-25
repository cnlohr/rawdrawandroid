#ifndef _WEBVIEW_NATIVE_ACTIVITY
#define _WEBVIEW_NATIVE_ACTIVITY

#include <android/native_activity.h>

extern volatile jobject g_objRootView;

typedef struct
{
	jobject WebViewObject;
	jobjectArray MessageChannels;
	jobject BackingBitmap;
	jobject BackingCanvas;
	int updated_canvas;
	int w, h;
} WebViewNativeActivityObject;

// Must be called from main thread

// initial_url = "about:blank" for a java-script only page.  Can also be file:///android_asset/test.html.
// Loading from "about:blank" will make the page ready almost immediately, otherwise it's about 50ms to load.
// useLooperForWebMessages is required, and must be a global jobject of your preferred looper to handle webmessages.
void WebViewCreate( WebViewNativeActivityObject * w, const char * initial_url, jobject useLooperForWebMessages, int pw, int ph );
void WebViewExecuteJavascript( WebViewNativeActivityObject * obj, const char * js );

// Note: Do not initialize until page reports as 100% loaded, with WebViewGetProgress.
void WebViewPostMessage( WebViewNativeActivityObject * obj, const char * mesg, int initial );
void WebViewRequestRenderToCanvas( WebViewNativeActivityObject * obj );
int  WebViewGetProgress( WebViewNativeActivityObject * obj );
char * WebViewGetLastWindowTitle( WebViewNativeActivityObject * obj );

// Can be called from any thread.
void WebViewNativeGetPixels( WebViewNativeActivityObject * obj, uint32_t * pixel_data, int w, int h );

#ifdef WEBVIEW_NATIVE_ACTIVITY_IMPLEMENTATION

volatile jobject g_objRootView;

void WebViewCreate( WebViewNativeActivityObject * w, const char * initial_url, jobject useLooperForWebMessages, int pw, int ph )
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
	jstring strjs = env->NewStringUTF( envptr, initial_url );
	env->CallVoidMethod(envptr, wvObj, LoadURLMethod, strjs );
	env->DeleteLocalRef( envptr, strjs );

	jmethodID WebViewGetSettingMethod = env->GetMethodID(envptr, WebViewClass, "getSettings", "()Landroid/webkit/WebSettings;");
	jobject websettings = env->CallObjectMethod(envptr, wvObj, WebViewGetSettingMethod );
	jclass WebSettingsClass = env->FindClass(envptr, "android/webkit/WebSettings");
	jmethodID setJavaScriptEnabledMethod = env->GetMethodID(envptr, WebSettingsClass, "setJavaScriptEnabled", "(Z)V");
	env->CallVoidMethod( envptr, websettings, setJavaScriptEnabledMethod, true );
	env->DeleteLocalRef( envptr, websettings );

	jmethodID setMeasuredDimensionMethodID = env->GetMethodID(envptr, WebViewClass, "setMeasuredDimension", "(II)V");
	env->CallVoidMethod(envptr, wvObj, setMeasuredDimensionMethodID, pw, ph );

	jclass ViewClass = env->FindClass(envptr, "android/widget/LinearLayout");
	jmethodID addViewMethod = env->GetMethodID(envptr, ViewClass, "addView", "(Landroid/view/View;)V");
	env->CallVoidMethod( envptr, g_objRootView, addViewMethod, wvObj );

	jclass WebMessagePortClass = env->FindClass(envptr, "android/webkit/WebMessagePort" );
	jmethodID createWebMessageChannelMethod = env->GetMethodID(envptr, WebViewClass, "createWebMessageChannel", "()[Landroid/webkit/WebMessagePort;");
	jobjectArray messageChannels = env->CallObjectMethod( envptr, wvObj, createWebMessageChannelMethod );
	jobject mc0 = env->GetObjectArrayElement(envptr, messageChannels, 0); // MC1 is handed over to javascript.

	jclass HandlerClassType = env->FindClass(envptr, "android/os/Handler" );
	jmethodID HandlerObjectConstructor = env->GetMethodID(envptr, HandlerClassType, "<init>", "(Landroid/os/Looper;)V");
	jobject handlerObject = env->NewObject( envptr, HandlerClassType, HandlerObjectConstructor, useLooperForWebMessages );
	handlerObject = env->NewGlobalRef(envptr, handlerObject);
	jmethodID setWebMessageCallbackMethod = env->GetMethodID( envptr, WebMessagePortClass, "setWebMessageCallback", "(Landroid/webkit/WebMessagePort$WebMessageCallback;Landroid/os/Handler;)V" );

	// Only can receive messages on MC0
	env->CallVoidMethod( envptr, mc0, setWebMessageCallbackMethod, 0, handlerObject );
	
	// Generate backing bitmap and canvas.
	jclass CanvasClass = env->FindClass(envptr, "android/graphics/Canvas");
	jclass BitmapClass = env->FindClass(envptr, "android/graphics/Bitmap");
	jmethodID createBitmap = env->GetStaticMethodID(envptr, BitmapClass, "createBitmap", "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;");
	jclass bmpCfgCls = env->FindClass(envptr, "android/graphics/Bitmap$Config");
	jstring bitmap_mode = env->NewStringUTF(envptr, "ARGB_8888");
	jmethodID bmpClsValueOfMid = env->GetStaticMethodID(envptr, bmpCfgCls, "valueOf", "(Ljava/lang/String;)Landroid/graphics/Bitmap$Config;");
	jobject jBmpCfg = env->CallStaticObjectMethod(envptr, bmpCfgCls, bmpClsValueOfMid, bitmap_mode);
	jobject bitmap = env->CallStaticObjectMethod( envptr, BitmapClass, createBitmap, pw, ph, jBmpCfg );
	jmethodID canvasConstructor = env->GetMethodID(envptr, CanvasClass, "<init>", "(Landroid/graphics/Bitmap;)V");
	jobject canvas = env->NewObject(envptr, CanvasClass, canvasConstructor, bitmap );

	env->DeleteLocalRef( envptr, CanvasClass );
	env->DeleteLocalRef( envptr, BitmapClass );
	env->DeleteLocalRef( envptr, bmpCfgCls );
	env->DeleteLocalRef( envptr, bitmap_mode );

	w->BackingBitmap = env->NewGlobalRef(envptr, bitmap );
	w->BackingCanvas = env->NewGlobalRef(envptr, canvas );
	w->WebViewObject = env->NewGlobalRef(envptr, wvObj);
	w->MessageChannels = env->NewGlobalRef(envptr, messageChannels);
	w->w = pw;
	w->h = ph;	

	env->DeleteLocalRef( envptr, WebViewClass );
	env->DeleteLocalRef( envptr, activityClass );
	env->DeleteLocalRef( envptr, WebSettingsClass );
	env->DeleteLocalRef( envptr, ViewClass );
}

int  WebViewGetProgress( WebViewNativeActivityObject * obj )
{
	const struct JNINativeInterface * env = 0;
	const struct JNINativeInterface ** envptr = &env;
	const struct JNIInvokeInterface ** jniiptr = gapp->activity->vm;
	const struct JNIInvokeInterface * jnii = *jniiptr;

	jnii->AttachCurrentThread( jniiptr, &envptr, NULL);
	env = (*envptr);

	jclass WebViewClass = env->FindClass(envptr, "android/webkit/WebView");
	jmethodID WebViewProgress = env->GetMethodID(envptr, WebViewClass, "getProgress", "()I");
	int ret = env->CallIntMethod( envptr, obj->WebViewObject, WebViewProgress );
	env->DeleteLocalRef( envptr, WebViewClass );
	return ret;
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
	jclass WebViewClass = env->FindClass(envptr, "android/webkit/WebView");
	jclass WebMessageClass = env->FindClass(envptr, "android/webkit/WebMessage" );

	jstring strjs = env->NewStringUTF( envptr, mesg );

	if( initial )
	{
		jobject mc1 = env->GetObjectArrayElement(envptr, w->MessageChannels, 1);
		jmethodID WebMessageConstructor = env->GetMethodID(envptr, WebMessageClass, "<init>", "(Ljava/lang/String;[Landroid/webkit/WebMessagePort;)V");

		//https://stackoverflow.com/questions/41753104/how-do-you-use-webmessageport-as-an-alternative-to-addjavascriptinterface
		// Only on initial hop do we want to post the root webmessage, which hooks up out webmessage port.
		jmethodID postMessageMethod = env->GetMethodID(envptr, WebViewClass, "postWebMessage", "(Landroid/webkit/WebMessage;Landroid/net/Uri;)V");

		// Need to generate a new message channel array.
		jobjectArray jsUseWebPorts = env->NewObjectArray( envptr, 1, WebMessagePortClass, mc1);

		// Need Uri.EMPTY
		jclass UriClass = env->FindClass(envptr, "android/net/Uri" );
		jfieldID EmptyField = env->GetStaticFieldID( envptr, UriClass, "EMPTY", "Landroid/net/Uri;" );
		jobject EmptyURI = env->GetStaticObjectField( envptr, UriClass, EmptyField );


		jobject newwm = env->NewObject(envptr, WebMessageClass, WebMessageConstructor, strjs, jsUseWebPorts );
		env->CallVoidMethod( envptr, w->WebViewObject, postMessageMethod, newwm, EmptyURI );

		env->DeleteLocalRef( envptr, jsUseWebPorts );
		env->DeleteLocalRef( envptr, newwm );
		env->DeleteLocalRef( envptr, EmptyURI );
		env->DeleteLocalRef( envptr, UriClass );
	}
	else
	{
		jobject mc0 = env->GetObjectArrayElement(envptr, w->MessageChannels, 0);
		jmethodID postMessageMethod = env->GetMethodID(envptr, WebMessagePortClass, "postMessage", "(Landroid/webkit/WebMessage;)V");
		jmethodID WebMessageConstructor = env->GetMethodID(envptr, WebMessageClass, "<init>", "(Ljava/lang/String;)V");

		jobject newwm = env->NewObject(envptr, WebMessageClass, WebMessageConstructor, strjs );
		env->CallVoidMethod( envptr, mc0, postMessageMethod, newwm );

		env->DeleteLocalRef( envptr, newwm );
		env->DeleteLocalRef( envptr, mc0 );
	}

	env->DeleteLocalRef( envptr, strjs );
	env->DeleteLocalRef( envptr, WebViewClass );
	env->DeleteLocalRef( envptr, WebMessageClass );
	env->DeleteLocalRef( envptr, WebMessagePortClass );
}

void WebViewRequestRenderToCanvas( WebViewNativeActivityObject * obj )
{
	const struct JNINativeInterface * env = 0;
	const struct JNINativeInterface ** envptr = &env;
	const struct JNIInvokeInterface ** jniiptr = gapp->activity->vm;
	const struct JNIInvokeInterface * jnii = *jniiptr;

	jnii->AttachCurrentThread( jniiptr, &envptr, NULL);
	env = (*envptr);

	jclass WebViewClass = env->FindClass(envptr, "android/webkit/WebView");
	jmethodID drawMethod = env->GetMethodID(envptr, WebViewClass, "draw", "(Landroid/graphics/Canvas;)V");
	env->CallVoidMethod( envptr, obj->WebViewObject, drawMethod, obj->BackingCanvas );
	env->DeleteLocalRef( envptr, WebViewClass );
}

void WebViewNativeGetPixels( WebViewNativeActivityObject * obj, uint32_t * pixel_data, int w, int h )
{
	const struct JNINativeInterface * env = 0;
	const struct JNINativeInterface ** envptr = &env;
	const struct JNIInvokeInterface ** jniiptr = gapp->activity->vm;
	const struct JNIInvokeInterface * jnii = *jniiptr;

	jnii->AttachCurrentThread( jniiptr, &envptr, NULL);
	env = (*envptr);

	jclass BitmapClass = env->FindClass(envptr, "android/graphics/Bitmap");
	jobject buffer = env->NewDirectByteBuffer(envptr, pixel_data, obj->w*obj->h*4 );
	jmethodID copyPixelsBufferID = env->GetMethodID( envptr, BitmapClass, "copyPixelsToBuffer", "(Ljava/nio/Buffer;)V" );
	env->CallVoidMethod( envptr, obj->BackingBitmap, copyPixelsBufferID, buffer );

	int i;
	int num = obj->w * obj->h;
	for( i = 0; i < num; i++ ) pixel_data[i] = bswap_32( pixel_data[i] );

	env->DeleteLocalRef( envptr, BitmapClass );
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
	env->CallVoidMethod( envptr, obj->WebViewObject, WebViewEvalJSMethod, strjs, 0 );  // Tricky: resultCallback = 0, if you try running looper.loop() it will crash - only manually process messages.
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

