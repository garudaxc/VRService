#!/bin/sh

#AndroidPath=~/work2/android-4.0.4_r1
AndroidPath=$2
appPath=.

if [ -z $2 ]
then
	echo usage :
	echo -cf : copy from android project to this workspace
	echo -ct : copy from workspace to android project
	echo -push : 
	echo -pull :
	exit 0
fi


if [ $1=="-cf" ]
then
	cp $AndroidPath/development/samples/SimpleJNI/jni/native.cpp $appPath/jni/
	cp $AndroidPath/development/samples/SimpleJNI/jni/Android.mk $appPath/jni/
	cp $AndroidPath/frameworks/native/libs/vr/VRSurfaceManager.cpp $appPath/Service/
	cp $AndroidPath/frameworks/native/libs/vr/Android.mk $appPath/Service/
	cp $AndroidPath/frameworks/base/core/java/android/app/VRSurfaceManager.java $appPath/Service/
	cp $AndroidPath/frameworks/native/opengl/libs/EGL/eglApi.cpp $appPath/Service/
	#cp $AndroidPath/frameworks/base/services/surfaceflinger/SurfaceFlinger.h $appPath/jni/
	#cp $AndroidPath/frameworks/base/services/surfaceflinger/SurfaceFlinger.cpp $appPath/jni/
	#cp $AndroidPath/frameworks/base/include/surfaceflinger/ISurfaceComposer.h $appPath/jni/
	#cp $AndroidPath/frameworks/base/libs/gui/ISurfaceComposer.cpp $appPath/jni/
	echo copy from $AndroidPath to workspace
fi	

#if [ $2=="-build" ]
#then
	#m development/samples/SimpleJNI/jni
	#cp out/target/product/generic/system/libsimplejni.so $appPath/jniLib
#fi

