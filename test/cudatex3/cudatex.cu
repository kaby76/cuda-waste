#include <iostream>
#include <stdio.h>

void CUDA_CALL(cudaError_t result)
{
	if (result != cudaSuccess)
	{
		std::cout << "Error " << result << "\n";
		exit(1);
	}
}

texture<float, cudaTextureType1D, cudaReadModeElementType> texRef;
texture<float, cudaTextureType1D, cudaReadModeElementType> texRef2;


// Simple transformation kernel
__global__ void transformKernel(float* output, int width, int height, int output_width, int output_height, float theta)
{
	// Calculate normalized texture coordinates
	unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
	unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;
     
	float u = x / (float)width;
	float v = y / (float)height;

	float tu = ((float)width * x) / output_width;
	float tv = ((float)height * y) / output_height;
	output[y * output_width + x] = tex1Dfetch(texRef, tu);
}

// Host code
int main()
{
	int width = 10;
	int height = 1;
	int size = width * height;

	float * h_data = (float*) malloc(width * height * sizeof(float));
	for (int i = 0 ; i < height; i++)
		for (int j = 0; j < width; j++)
				h_data[i * width + j] = i * width + j + 1;

	printf ("\n Original array \n");
	for (int i = 0; i < height; i++)
	{
		for (int j = 0 ; j < width; j++)
			printf ("%f ", h_data[i*width + j]);
		printf ("\n");
	}

          
	float * d_data;
	CUDA_CALL( cudaMalloc(&d_data, size * sizeof(float)) );
	// Copy to device memory some data located at address h_data
	// in host memory
	CUDA_CALL( cudaMemcpy(d_data, h_data, size * sizeof(float), cudaMemcpyHostToDevice) );
    
	// Set texture parameters
	//texRef.addressMode[0] = cudaAddressModeWrap;
	//texRef.addressMode[1] = cudaAddressModeWrap;
	//texRef.filterMode = cudaFilterModeLinear;
	//texRef.normalized = true;
     
	// Bind the array to the texture reference
	CUDA_CALL( cudaBindTexture(0, texRef, d_data, size * sizeof(float)) );
    cudaDeviceSynchronize();
	
	int output_width = 100;
	int output_height = 1;

	// Set up block dims.
	dim3 dimBlock(10, 1);
	dim3 dimGrid((output_width - 1)/dimBlock.x + 1, (output_height - 1)/dimBlock.y + 1);
     
	printf("blockDim = %d, %d\n", dimBlock.x, dimBlock.y);
	printf("gridDim = %d, %d\n", dimGrid.x, dimGrid.y);

	// Allocate result of transformation in device memory
	int output_size =  dimGrid.x * dimBlock.x * dimGrid.y * dimBlock.y;
	printf("output_size %d\n", output_size);
	float* output;
	CUDA_CALL( cudaMalloc(&output, output_size * sizeof(float)) );
	float * h_output = (float*)malloc(output_size * sizeof(float));
     
	transformKernel<<<dimGrid, dimBlock>>>(output, width, height, output_width, output_height, 90);
    cudaDeviceSynchronize();
	CUDA_CALL( cudaMemcpy(h_output, output, output_size * sizeof(float), cudaMemcpyDeviceToHost) );

	printf ("\nAfter operation\n");
	for (int i = 0; i < output_height; i++)
	{
		for (int j = 0; j < output_width; j++)
			printf ("%d %f\n", i*output_width + j, h_output[i*output_width + j]);
		printf ("\n");
	}

	system ("pause");
	// Free device memory
	cudaFree(d_data);
	cudaFree(output);
	return 0;
}
