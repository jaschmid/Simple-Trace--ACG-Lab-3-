#include "SDL/SDL.h"
#include <array>
#include <omp.h>
#include "volume.h"

#define HAGE_NO_MEMORY_ALLOC_REPLACEMENT
#include <HAGE.h>

class Scene
{
public:
	Scene() :_nThreads(1)
	{
		
		SDL_Init(SDL_INIT_VIDEO);

		_screen = SDL_SetVideoMode(xScreen, yScreen, 32, SDL_SWSURFACE);
		SDL_WM_SetCaption("Simple Trace", "Simple Trace");

		
		SetCameraLocation(Matrix4<>::One());

		volume = new Volume();
		volume->LoadData("Blabla.vol");
	}
	~Scene()
	{
		delete volume;
		SDL_Quit();
	}

	void SetCameraLocation(const Matrix4<>& camera)
	{
		const float aspect = (float)xScreen / (float)yScreen;
		
		#pragma omp parallel for
		for(int iy = 0; iy < yScreen; iy++)
			for(int ix = 0; ix < xScreen; ix++)
			{
				float fx = (float)(ix) / (float)(xScreen) *2.0f - 1.0f;
				float fy = ((float)(iy) / (float)(yScreen) *2.0f - 1.0f)/aspect;

				Job& j = _raySource[iy * xScreen + ix];
				j.pixelOffset = iy * xScreen + ix;
				float len = sqrtf(fx*fx + fy*fy +1);
				j.ray.loc[0] = 0.0f;
				j.ray.loc[1] = 0.0f;
				j.ray.loc[2] = 0.0f;
				j.ray.dir[0] = fx /len;
				j.ray.dir[1] = fy / len;
				j.ray.dir[2] = -1.0f /len;

				Vector4<> p1 = Vector4<>(j.ray.loc,1.0f);
				Vector4<> p2 = Vector4<>(j.ray.loc+j.ray.dir,1.0f);

				p1 = camera*p1;
				p2 = camera*p2;

				j.ray.loc = p1.xyz();
				j.ray.dir = (p2-p1).xyz().normalize();
			}
	}

	bool DrawFrame()
	{
		const Job* j = nullptr;
		while(SDL_PollEvent(&_event))
		{
			switch(_event.type)
			{
				case SDL_KEYDOWN:
				{
					if(_event.key.keysym.sym == SDLK_ESCAPE)
						return false;
					if(_event.key.keysym.sym == SDLK_a)
						volume->AdjustLevel(-0.05f);
					else if(_event.key.keysym.sym == SDLK_d)
						volume->AdjustLevel(0.05f);
					else if(_event.key.keysym.sym == SDLK_q)
						_nThreads = (_nThreads-2)%8+1;
					else if(_event.key.keysym.sym == SDLK_e)
						_nThreads = (_nThreads)%8+1;
					printf("nThreads: %i\n",_nThreads);
				}
				break;
			}
		}

		if(SDL_MUSTLOCK(_screen)) 
		{
			if(SDL_LockSurface(_screen) < 0) 
				return false;
		}
		Color32* raw_pixels = (Color32 *)_screen->pixels;

		_nextRay=0;

		#pragma omp parallel private(j) num_threads (_nThreads)
		{
			while( (j = GetNextJob()) != nullptr)
			{
				Color c = IntersectRayWithScene(j->ray);
				raw_pixels[j->pixelOffset] = to32Bit(c);
			}
		}
	
		if(SDL_MUSTLOCK(_screen)) 
			SDL_UnlockSurface(_screen);

		SDL_Flip(_screen);
		return true;
	}

	const Job* GetNextJob()
	{
		const Job* result;

		#pragma omp critical
		{
			if(_nextRay >= _raySource.size())
				result = nullptr;
			else
				result = &_raySource[_nextRay++];
		}

		return result;
	}

	Color IntersectRayWithScene(Ray r) const
	{
		Color result;
		if(volume->Intersect(r,result))
			return result;
		else
			return Color(0.0f,0.0f,0.0f);
	}

private:

	Color32 to32Bit(Color c)
	{
		Color32 _32;
		_32.r = (unsigned char)(std::max(std::min(c[0]*255.0f,255.0f),0.0f));
		_32.g = (unsigned char)(std::max(std::min(c[1]*255.0f,255.0f),0.0f));
		_32.b = (unsigned char)(std::max(std::min(c[2]*255.0f,255.0f),0.0f));
		_32.a = 255;
		return _32;
	}

	const static int xScreen = 640;
	const static int yScreen = 480;
	
	int									_nextRay;
	u32									_nThreads;
	std::array<Color,xScreen*yScreen>	_screenBuffer;
	std::array<Job,xScreen*yScreen>		_raySource;

	SDL_Surface *_screen;
	SDL_Event _event;

	Volume* volume;
};