//#pragma OPENCL EXTENSION cl_khr_global_int32_base_atomics : enable

//#define M_PI (3.14159265359)
#define M_TWO_PI (2*M_PI)


typedef struct{
	float4 target;
	float4 targetColor;
	float speed;
	float dummy1;
	float dummy2;
	float dummy3;
} Particle;

__kernel void updateParticle(__global Particle* particles,
                             __global float4* posBuffer,
                             __global float4* colorBuffer){
	int id = get_global_id(0);  //index which is currently processed
	__global Particle *p = &particles[id];
    __global float4 *pos = &posBuffer[id];
    __global float4 *color = &colorBuffer[id];


    //float speed = p->speed;
    //let's speed of moving depends on the target color
    float speed = p->speed * (0.1 + 3*max(p->targetColor.b - p->targetColor.g, 0.0f));

    //linear fly - uncomment this, and comment the "radial fly" block
    //*pos += (p->target - *pos) * p->speed;
    
    //radial fly ------
    
    
    float a = atan2( pos->y, pos->x );
    float r = length( *pos );
    float aTarget = atan2( p->target.y, p->target.x );
    //angle unwrapping
    float deltaA = fabs(aTarget - a);
    float deltaA1 = fabs(aTarget + M_TWO_PI - a);
    float deltaA2 = fabs(aTarget - M_TWO_PI - a);
    
    if ( deltaA1 < min(deltaA, deltaA2 ) ) {
        aTarget += M_TWO_PI;
    }
    else {
        if ( deltaA2 < min(deltaA, deltaA1 ) ) {
            aTarget -= M_TWO_PI;
        }
    }
    
    float rTarget = length( p->target );
    
    a += (aTarget - a) * p->speed;
    r += (rTarget - r) * p->speed;
    
    pos->x = cos( a ) * r;
    pos->y = sin( a ) * r;
    pos->z +=  (p->target.z - pos->z) * speed;
    //--------
    
    
    *color += (p->targetColor - *color) * speed;
}

