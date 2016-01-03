#!/bin/sh

#AndroidPath=~/work2/android-4.0.4_r1
AndroidPath=$1
appPath=MyApplication/app/src/main

if [ -z $1 ]
then
	echo usage :
	echo -cf : copy from android project to this workspace
	echo -ct : copy from workspace to android project
	echo -push : 
	echo -pull :
	exit 0
fi


if [ $2=="-cf" ]
then
	cp $AndroidPath/development/samples/SimpleJNI/jni/native.cpp $appPath/jni/
	cp $AndroidPath/development/samples/SimpleJNI/jni/Android.mk $appPath/jni/
	cp $AndroidPath/frameworks/base/services/surfaceflinger/SurfaceFlinger.h $appPath/jni/
	cp $AndroidPath/frameworks/base/services/surfaceflinger/SurfaceFlinger.cpp $appPath/jni/
	cp $AndroidPath/frameworks/base/include/surfaceflinger/ISurfaceComposer.h $appPath/jni/
	cp $AndroidPath/frameworks/base/libs/gui/ISurfaceComposer.cpp $appPath/jni/
	echo copy from $AndroidPath to workspace
fi	

if [ $2=="-build" ]
then
	m development/samples/SimpleJNI/jni
	cp out/target/product/generic/system/libsimplejni.so $appPath/jniLib
fi

