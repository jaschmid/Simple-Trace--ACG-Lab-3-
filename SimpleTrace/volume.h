#include "SDL/SDL.h"
#include <array>
#define HAGE_NO_MEMORY_ALLOC_REPLACEMENT
#include <HAGE.h>
#include <float.h>
#include <math.h>
#include "raybox.h"

using namespace HAGE;
typedef Vector3<> Color;

struct Ray
{
	Vector3<> loc;
	Vector3<> dir;
	Vector3<> inv_dir;
};

struct Color32
{
	unsigned char b,g,r,a;
};

struct Job
{
	//input
	Ray ray;

	//output
	int pixelOffset;
};




class Volume
{
public:

	Volume() : min(-1.0f,-1.0f,-1.0f), max(1.0f,1.0f,1.0f), data(nullptr), level(0.6f)
	{
	}

	~Volume()
	{
	}

	void AdjustLevel(float adj)
	{
		level += adj;
	}

	void LoadData(const char* string)
	{
		dataSize = Vector3<u32>(256,256,256);
		data = new float[dataSize.x*dataSize.y*dataSize.z];

		for(int i = 0; i < dataSize.x*dataSize.y*dataSize.z; i++)
			data[i] = 0.0f;

		#pragma omp parallel for
		for(int iz = 1; iz < dataSize.z-1; iz++)
			for(int iy = 1; iy < dataSize.y-1; iy++)
				for(int ix = 1; ix < dataSize.x-1; ix++)
				{
					Vector3<i32> offset = Vector3<i32>(iz - dataSize.z /2,iy - dataSize.y /2,ix - dataSize.x /2);
					Vector3<> fOffset = ((Vector3<>)offset)|((Vector3<>)dataSize/2);
					data[ (iz*dataSize.y + iy)*dataSize.x + ix ] = 1.0f-std::max( (cosf(0.5f+fOffset.x*5.0f) * cosf(0.5f+fOffset.y*5.0f) * cosf(0.5f+fOffset.z*5.0f))/2.0f+0.5f ,0.0f);
				}

		
		divergence = new Vector3<>[dataSize.x*dataSize.y*dataSize.z];

		
		#pragma omp parallel for
		for(int iz = 0; iz < dataSize.z; iz++)
			for(int iy = 0; iy < dataSize.y; iy++)
				for(int ix = 0; ix < dataSize.x; ix++)
				{
					divergence[ (iz*dataSize.y + iy)*dataSize.x + ix ].x = 
						(data[ (iz*dataSize.y + iy)*dataSize.x + (ix+1)%dataSize.x ] - data[ (iz*dataSize.y + iy)*dataSize.x + (ix-1)%dataSize.x ])/2.0f;
					divergence[ (iz*dataSize.y + iy)*dataSize.x + ix ].y = 
						(data[ (iz*dataSize.y + (iy+1)%dataSize.y)*dataSize.x + ix ] - data[ (iz*dataSize.y + (iy-1)%dataSize.y)*dataSize.x + ix ])/2.0f;
					divergence[ (iz*dataSize.y + iy)*dataSize.x + ix ].z = 
						(data[ ((iz+1)%dataSize.z*dataSize.y + iy)*dataSize.x + ix ] - data[ ((iz-1)%dataSize.z*dataSize.y + iy)*dataSize.x + ix ])/2.0f;
					
				}
	}

	float getData(const Vector3<>& loc) const
	{
		Vector3<> locInternal =  ((loc - min) | (max - min));
		return getData( Vector3<u32>( (u32)(locInternal.x * dataSize.x) % dataSize.x ,  
						(u32)(locInternal.y * dataSize.y) % dataSize.y ,
						(u32)(locInternal.z * dataSize.z) % dataSize.z ) );
	}
	float getData(const Vector3<u32>& loc) const
	{
		return data[ (loc.z*dataSize.y + loc.y)*dataSize.x + loc.x ];
	}
	Vector3<> getDivergence(const Vector3<>& loc) const
	{
		Vector3<> locInternal =  ((loc - min) | (max - min));
		return getDivergence( Vector3<u32>( (u32)(locInternal.x * dataSize.x) % dataSize.x ,  
						(u32)(locInternal.y * dataSize.y) % dataSize.y ,
						(u32)(locInternal.z * dataSize.z) % dataSize.z ) );
	}
	Vector3<> getDivergence(const Vector3<u32>& loc) const
	{
		return divergence[ (loc.z*dataSize.y + loc.y)*dataSize.x + loc.x ];
	}

	bool Intersect(const Ray& r, Color& c) const
	{

		aabb_t mybox;
		mybox.min.x = min.x;
		mybox.min.y = min.y;	
		mybox.min.z = min.z;
		mybox.max.x = max.x;
		mybox.max.y = max.y;	
		mybox.max.z = max.z;
		ray_t ray;
		ray.pos.x = r.loc.x;
		ray.pos.y = r.loc.y;
		ray.pos.z = r.loc.z;
		ray.inv_dir.x = 1.0f/r.dir.x;
		ray.inv_dir.y = 1.0f/r.dir.y;
		ray.inv_dir.z = 1.0f/r.dir.z;
		ray_segment_t segment;
		if(ray_box_intersect(mybox,ray,segment))
		{

			float tStep = 1.0f/256.0f;

			for(float t = segment.t_near; t< segment.t_far; t+= tStep)
				if(getData(r.dir *t + r.loc) > level)
				{
					Vector3<> normal = -getDivergence(r.dir *t + r.loc);

					normal = normal.normalize();
					float val = normal* (-r.dir);
					c = Color(val,val,val);
					return true;
				}
			return false;
		}
		else
			return false;
	}
private:
	
	Vector3<> min;
	Vector3<> max;
	
	float level;

	Vector3<u32> dataSize;
	float*	data;
	Vector3<>*	divergence;
};