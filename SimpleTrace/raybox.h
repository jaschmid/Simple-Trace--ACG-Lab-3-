
#include <xmmintrin.h>

// can you say "barebone"?
struct _MM_ALIGN16 vec_t { float x,y,z,pad; };
struct _MM_ALIGN16 aabb_t { 
	vec_t	min;
	vec_t	max;
};

struct _MM_ALIGN16 ray_t {
	vec_t	pos;
	vec_t	inv_dir;
};
struct _MM_ALIGN16 ray_segment_t {
	float	t_near,t_far;
};


bool ray_box_intersect(const aabb_t &box, const ray_t &ray, ray_segment_t &rs);
