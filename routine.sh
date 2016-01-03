#!/bin/sh

AndroidPath=~/work2/android-4.0.4_r1

if [ -z $1 ]
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
	cp $AndroidPath/development/samples/SimpleJNI/jni/native.cpp ./jni/
	cp $AndroidPath/development/samples/SimpleJNI/jni/Android.mk ./jni/
	cp $AndroidPath/frameworks/base/services/surfaceflinger/SurfaceFlinger.h ./jni/
	cp $AndroidPath/frameworks/base/services/surfaceflinger/SurfaceFlinger.cpp ./jni/
	cp $AndroidPath/frameworks/base/include/surfaceflinger/ISurfaceComposer.h ./jni/
	cp $AndroidPath/frameworks/base/libs/gui/ISurfaceComposer.cpp ./jni/
	echo copy from $AndroidPath to workspace
fi	


