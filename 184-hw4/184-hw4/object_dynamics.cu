#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <stdio.h>
#include "object_dynamics.h"
#include <math.h>

__device__ const double G = 1.610e-3;

__device__ extern __shared__ volatile double sdata[];

/* Taken nearly word for word from http://developer.download.nvidia.com/compute/cuda/1.1-Beta/x86_website/projects/reduction/doc/reduction.pdf */
__device__ void reduce(const unsigned int blockSize, const unsigned int tid)
{
	if (blockSize >= 512) {
		if (tid < 256) { 
			sdata[tid] += sdata[tid + 256]; 
		}
		__syncthreads();
	}
	if (blockSize >= 256) {
		if (tid < 128) { 
			sdata[tid] += sdata[tid + 128]; 
		}
		__syncthreads();
	}
	if (blockSize >= 128) {
		if (tid < 64) {  
			sdata[tid] += sdata[tid + 64]; 
		}
		__syncthreads();
	}
	if (tid < 32) {
		if (blockSize >= 64) {
			sdata[tid] += sdata[tid + 32]; 
		}
		if (blockSize >= 32) {
			sdata[tid] += sdata[tid + 16]; 
		}
		if (blockSize >= 16) {
			sdata[tid] += sdata[tid + 8]; 
		}
		if (blockSize >= 8) {
			sdata[tid] += sdata[tid + 4]; 
		}
		if (blockSize >= 4) {
			sdata[tid] += sdata[tid + 2]; 
		}
		if (blockSize >= 2) {
			sdata[tid] += sdata[tid + 1]; 
		}
	}
}

__device__ void kfunction(const unsigned int blockSize, const unsigned int tid, const unsigned int obj, const double3 inpos, double3 &kvel, const double* g_idata, const unsigned int numdynamicobjects,  const unsigned int numdynamicobjects_align) 
{	
	// do object dynamics, each thread may have to do multiple items
	for (int i = tid; i < numdynamicobjects; i+=blockSize) {
		if (i != obj) {
			// read in other object's properties
			double3 o_pos = make_double3(g_idata[numdynamicobjects_align*0 + i],
										g_idata[numdynamicobjects_align*1 + i],
										g_idata[numdynamicobjects_align*2 + i]);
			double o_mass = g_idata[numdynamicobjects_align*6 + i];

			// gravity k > 1
			double3 rvec = make_double3(o_pos.x - inpos.x,
										o_pos.y - inpos.y,
										o_pos.z - inpos.z);

			double rinv = 1.0 / sqrt(rvec.x*rvec.x + rvec.y*rvec.y + rvec.z*rvec.z);

			kvel.x += o_mass * rinv*rinv*rinv * rvec.x;
			kvel.y += o_mass * rinv*rinv*rinv * rvec.y;
			kvel.z += o_mass * rinv*rinv*rinv * rvec.z;
		}
	}

	// reduction to get overall k
	sdata[tid] = kvel.x;
	__syncthreads();
	reduce(blockSize, tid);
	__syncthreads();
	kvel.x = sdata[0];

	__syncthreads();
	sdata[tid] = kvel.y;
	__syncthreads();
	reduce(blockSize, tid);
	kvel.y = sdata[0];

	__syncthreads();
	sdata[tid] = kvel.z;
	__syncthreads();
	reduce(blockSize, tid);
	__syncthreads();
	kvel.z = sdata[0];

	__syncthreads();
}

__device__ void cuda_collisions(const unsigned int blockSize, const unsigned int tid, const unsigned int obj, double3 &colvel, const double3 pos, const double3 vel, const double* g_idata, const unsigned int numdynamicobjects,  const unsigned int numdynamicobjects_align)
{
	double mass = g_idata[numdynamicobjects_align*6 + obj],
		   radius = g_idata[numdynamicobjects_align*7 + obj];

	// do object collisions, each thread may have to do multiple items
	for (int i = tid; i < numdynamicobjects; i+=blockSize) {
		if (i != obj) {
			// read in other object's properties
			double3 o_pos = make_double3(g_idata[numdynamicobjects_align*0 + i],
											g_idata[numdynamicobjects_align*1 + i],
											g_idata[numdynamicobjects_align*2 + i]);
			double3 o_vel = make_double3(g_idata[numdynamicobjects_align*3 + i],
											g_idata[numdynamicobjects_align*4 + i],
											g_idata[numdynamicobjects_align*5 + i]);
			double o_mass = g_idata[numdynamicobjects_align*6 + i],
					o_radius = g_idata[numdynamicobjects_align*7 + i];

			double3 Dp = make_double3(o_pos.x - pos.x,
									  o_pos.y - pos.y,
									  o_pos.z - pos.z);
			double d = sqrt(Dp.x*Dp.x + Dp.y*Dp.y + Dp.z*Dp.z);

			if (d <= radius + o_radius){

				// from the other project/game

				double3 u = make_double3(Dp.x/d, Dp.y/d, Dp.z/d);

				// find their speed in the collision direction
				double cs = u.x * vel.x + u.y * vel.y + u.z * vel.z,
					   o_cs = u.x * o_vel.x + u.y * o_vel.y + u.z * o_vel.z;

				// momentum and energy of the objects in the collision direction
				double P = mass*cs + o_mass*o_cs;
				double E = 0.5*(mass*cs*cs + o_mass*o_cs*o_cs);

				// conserve momentum and energy
				double a = mass * ( 1 + mass/o_mass),
						b = -2.0*P*mass/o_mass,
						c = P*P/o_mass - 2.0*E;

				double dcs = (-b - sqrt(b*b - 4.0*a*c)) / (2.0*a);
			
				colvel.x += (dcs - cs)*u.x;
				colvel.y += (dcs - cs)*u.y;
				colvel.z += (dcs - cs)*u.z;
			}
		}
	}
	// reduce
	sdata[tid] = colvel.x;
	__syncthreads();
	reduce(blockSize, tid);
	__syncthreads();
	colvel.x = sdata[0];

	__syncthreads();
	sdata[tid] = colvel.y;
	__syncthreads();
	reduce(blockSize, tid);
	__syncthreads();
	colvel.y = sdata[0];

	__syncthreads();
	sdata[tid] = colvel.z;
	__syncthreads();
	reduce(blockSize, tid);
	__syncthreads();
	colvel.z = sdata[0];

	__syncthreads();
}

__global__ void cuda_dynamics(double *g_idata, const unsigned int numdynamicobjects_align, const double dt)
{
	//extern __shared__ volatile double sdata[];

	const unsigned int tid = threadIdx.x;
	const unsigned int obj = blockIdx.x;
	const unsigned int numdynamicobjects = gridDim.x;
	const unsigned int blockSize = blockDim.x;

	sdata[tid] = 0.0; 
	if(tid >= numdynamicobjects) return;

	// read in object properties

	double3 pos = make_double3(g_idata[numdynamicobjects_align*0 + obj],
								g_idata[numdynamicobjects_align*1 + obj],
								g_idata[numdynamicobjects_align*2 + obj]);
	double3 vel = make_double3(g_idata[numdynamicobjects_align*3 + obj],
								g_idata[numdynamicobjects_align*4 + obj],
								g_idata[numdynamicobjects_align*5 + obj]);

	double3 kvel = make_double3(0.0, 0.0, 0.0);

	cuda_collisions(blockSize, tid, obj, kvel, pos, vel, g_idata, numdynamicobjects, numdynamicobjects_align);

	vel.x += kvel.x;
	vel.y += kvel.y;
	vel.z += kvel.z;

	// calculate new positions

	double3 inpos = make_double3(pos.x, pos.y, pos.z);
	
	double3 kpos = make_double3(dt*vel.x,
								dt*vel.y,
								dt*vel.z);
	//double3 kvel = make_double3(0.0, 0.0, 0.0);
	kvel.x = 0.0;
	kvel.y = 0.0;
	kvel.z = 0.0;

	kfunction(blockSize, tid, obj, inpos, kvel, g_idata, numdynamicobjects, numdynamicobjects_align);

	kvel.x *= dt*G;
	kvel.y *= dt*G;
	kvel.z *= dt*G;

	double3 newpos = make_double3(pos.x + kpos.x/6.0, 
								  pos.y + kpos.y/6.0, 
								  pos.z + kpos.z/6.0);
	double3 newvel = make_double3(vel.x + kvel.x/6.0, 
								  vel.y + kvel.y/6.0, 
								  vel.z + kvel.z/6.0);

	inpos.x = pos.x + 0.5*kpos.x;
	inpos.y = pos.y + 0.5*kpos.y;
	inpos.z = pos.z + 0.5*kpos.z;

	kpos.x = dt*(vel.x + 0.5*kvel.x);
	kpos.y = dt*(vel.y + 0.5*kvel.y);
	kpos.z = dt*(vel.z + 0.5*kvel.z);
	kvel.x = 0.0;
	kvel.y = 0.0;
	kvel.z = 0.0;

	kfunction(blockSize, tid, obj, inpos, kvel, g_idata, numdynamicobjects, numdynamicobjects_align);

	kvel.x *= dt*G;
	kvel.y *= dt*G;
	kvel.z *= dt*G;

	newpos.x += kpos.x/3.0;
	newpos.y += kpos.y/3.0;
	newpos.z += kpos.z/3.0;
	newvel.x += kvel.x/3.0;
	newvel.y += kvel.y/3.0;
	newvel.z += kvel.z/3.0;

	inpos.x = pos.x + 0.5*kpos.x;
	inpos.y = pos.y + 0.5*kpos.y;
	inpos.z = pos.z + 0.5*kpos.z;

	kpos.x = dt*(vel.x + 0.5*kvel.x);
	kpos.y = dt*(vel.y + 0.5*kvel.y);
	kpos.z = dt*(vel.z + 0.5*kvel.z);
	kvel.x = 0.0;
	kvel.y = 0.0;
	kvel.z = 0.0;

	kfunction(blockSize, tid, obj, inpos, kvel, g_idata, numdynamicobjects, numdynamicobjects_align);

	kvel.x *= dt*G;
	kvel.y *= dt*G;
	kvel.z *= dt*G;

	newpos.x += kpos.x/3.0;
	newpos.y += kpos.y/3.0;
	newpos.z += kpos.z/3.0;
	newvel.x += kvel.x/3.0;
	newvel.y += kvel.y/3.0;
	newvel.z += kvel.z/3.0;

	inpos.x = pos.x + kpos.x;
	inpos.y = pos.y + kpos.y;
	inpos.z = pos.z + kpos.z;

	kpos.x = dt*(vel.x + kvel.x);
	kpos.y = dt*(vel.y + kvel.y);
	kpos.z = dt*(vel.z + kvel.z);
	kvel.x = 0.0;
	kvel.y = 0.0;
	kvel.z = 0.0;

	kfunction(blockSize, tid, obj, inpos, kvel, g_idata, numdynamicobjects, numdynamicobjects_align);

	kvel.x *= dt*G;
	kvel.y *= dt*G;
	kvel.z *= dt*G;

	newpos.x += kpos.x/6.0;
	newpos.y += kpos.y/6.0;
	newpos.z += kpos.z/6.0;
	newvel.x += kvel.x/6.0;
	newvel.y += kvel.y/6.0;
	newvel.z += kvel.z/6.0;

	// collide based on new object position

	//kvel.x = 0.0;
	//kvel.y = 0.0;
	//kvel.z = 0.0;

	//cuda_collisions(blockSize, tid, obj, kvel, newpos, newvel, g_idata, numdynamicobjects, numdynamicobjects_align);

	// calculate final result and write result for this block to global memory
	if (tid == 0) {
		g_idata[obj + 0*numdynamicobjects_align] = newpos.x;
		g_idata[obj + 1*numdynamicobjects_align] = newpos.y;
		g_idata[obj + 2*numdynamicobjects_align] = newpos.z;
		g_idata[obj + 3*numdynamicobjects_align] = newvel.x;
		g_idata[obj + 4*numdynamicobjects_align] = newvel.y;
		g_idata[obj + 5*numdynamicobjects_align] = newvel.z;
	}
}

double* prev_d_in_out = 0;
//double* prev_d_out = 0;
unsigned int prev_numdynamicobjects_in_out = 0;
//unsigned int prev_numdynamicobjects_out = 0;


double* get_d_in_out(const unsigned int numdynamicobjects){
	if (prev_numdynamicobjects_in_out < numdynamicobjects) {
		cudaError_t cudaStatus;
		cudaStatus = cudaMalloc(&prev_d_in_out, numdynamicobjects*8*sizeof(double));
		if (cudaStatus != cudaSuccess) {
			fprintf(stderr, "d_in cudaMalloc failed!\n");
		}
		prev_numdynamicobjects_in_out = numdynamicobjects;
		on_device = false;
	}
	return prev_d_in_out;
}
//double* get_d_out(const unsigned int numdynamicobjects){
//	if (prev_numdynamicobjects_out < numdynamicobjects){
//		cudaError_t cudaStatus;
//		cudaStatus = cudaMalloc(&prev_d_out, numdynamicobjects*6*sizeof(double));
//		if (cudaStatus != cudaSuccess) {
//			fprintf(stderr, "d_out cudaMalloc failed!\n");
//		}
//		prev_numdynamicobjects_out = numdynamicobjects;
//	}
//	return prev_d_out;
//}
void freeallcuda(void){
	cudaFree(prev_d_in_out);
    //cudaFree(prev_d_out);
	prev_numdynamicobjects_in_out = 0;
	//prev_numdynamicobjects_out = 0;
	on_device = false;
}

void do_dynamics(double* dynamic_in_out, const unsigned int numdynamicobjects, const unsigned int numdynamicobjects_align, const double dt)
{
	if (numdynamicobjects > 0){
		cudaError_t cudaStatus;

		double* d_in_out = get_d_in_out(numdynamicobjects_align);
		//double* d_out = get_d_out(numdynamicobjects_align);

		if (! on_device){
			// Copy input vectors from host memory to GPU buffers.
			cudaStatus = cudaMemcpy(d_in_out, dynamic_in_out, numdynamicobjects_align*8*sizeof(double), cudaMemcpyHostToDevice);
			if (cudaStatus != cudaSuccess) {
				fprintf(stderr, "host to device cudaMemcpy failed!\n");
				goto Error;
			}
			on_device = true;
		}

		int dimBlock = 1;
		
		if (numdynamicobjects >= 512)
			dimBlock = 512;
		else 
			while (dimBlock < numdynamicobjects) 
				dimBlock*=2 ;

		int smemSize = dimBlock*sizeof(double);

		cuda_dynamics<<< numdynamicobjects, dimBlock, smemSize >>>(d_in_out, numdynamicobjects_align, dt);

		//scanf("%d");

		// cudaDeviceSynchronize waits for the kernel to finish, and returns
		// any errors encountered during the launch.
		cudaStatus = cudaDeviceSynchronize();
		if (cudaStatus != cudaSuccess) {
			fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching addKernel!\n", cudaStatus);
			goto Error;
		}

		cudaStatus = cudaMemcpy(dynamic_in_out, d_in_out, numdynamicobjects_align*3*sizeof(double), cudaMemcpyDeviceToHost);
		if (cudaStatus != cudaSuccess) {
			fprintf(stderr, "device to host cudaMemcpy failed!\n");
			goto Error;
		}
	}

	return;
Error:
	printf("Dynamics Error\n");
	freeallcuda();
	scanf("%d");
	return;
}