#include "scene.h"
#include <Windows.h>
#include <iostream>

int main( int argc, char *argv[] ) 
{
	Scene* scene = new Scene();

	float Rotation = 0.0f;
	
	scene->SetCameraLocation( Matrix4<>::AngleRotation(Vector3<>(0.0f,1.0f,0.0f),Rotation) * Matrix4<>::Translate(Vector3<>(0.0f,0.0f,3.0f)) );

	i64 freq;
	QueryPerformanceFrequency((LARGE_INTEGER*)&freq);
	
	i64 lastFrame,thisFrame;
	QueryPerformanceCounter((LARGE_INTEGER*)&thisFrame);

	while(scene->DrawFrame())
	{
		lastFrame = thisFrame;
		Rotation += 0.1f;
		scene->SetCameraLocation( Matrix4<>::AngleRotation(Vector3<>(0.0f,1.0f,0.0f),Rotation) * Matrix4<>::Translate(Vector3<>(0.0f,0.0f,3.0f)) );
		
		QueryPerformanceCounter((LARGE_INTEGER*)&thisFrame);

		i64 elapsed = thisFrame - lastFrame;
		f32 fElapsed = (float)elapsed/(float)freq;
		printf("Framerate: %f\n",1.0f/fElapsed);
	}

	delete scene;
	return 0;
}