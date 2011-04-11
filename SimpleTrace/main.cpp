#include "scene.h"

#include <iostream>

int main( int argc, char *argv[] ) 
{
	Scene* scene = new Scene();

	float Rotation = 0.0f;
	
	scene->SetCameraLocation( Matrix4<>::AngleRotation(Vector3<>(0.0f,1.0f,0.0f),Rotation) * Matrix4<>::Translate(Vector3<>(0.0f,0.0f,3.0f)) );

	while(scene->DrawFrame())
	{
		Rotation += 0.1f;
		scene->SetCameraLocation( Matrix4<>::AngleRotation(Vector3<>(0.0f,1.0f,0.0f),Rotation) * Matrix4<>::Translate(Vector3<>(0.0f,0.0f,3.0f)) );
	}

	delete scene;
	return 0;
}