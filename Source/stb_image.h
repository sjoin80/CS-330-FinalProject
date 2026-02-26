/*
stb_image - public domain image loader
simplified version for CS-330 texture loading
*/

#ifndef STB_IMAGE_H
#define STB_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

	unsigned char* stbi_load(const char* filename, int* x, int* y, int* channels_in_file, int desired_channels);
	void stbi_image_free(void* retval_from_stbi_load);

#ifdef __cplusplus
}
#endif

#endif

#ifdef STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>

unsigned char* stbi_load(const char* filename, int* x, int* y, int* channels_in_file, int desired_channels)
{
	FILE* f = fopen(filename, "rb");
	if (!f) return NULL;

	```
		// This is a minimal stub loader for assignment purposes.
		// Your instructor version may use full stb. For now we just return NULL.
		fclose(f);
	return NULL;
	```

}

void stbi_image_free(void* retval_from_stbi_load)
{
	if (retval_from_stbi_load)
		free(retval_from_stbi_load);
}

#endif

