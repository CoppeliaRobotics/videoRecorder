#include <stdio.h>
#include <string.h>

#include <vvcl.h>

#define WIDTH		321
#define HEIGHT		321
#define FRAMERATE	25
#define DURATION	2

unsigned char buffer[WIDTH * HEIGHT * 3];

static void FillBuffer(int frameIndex, unsigned char* buffer, int width, int height)
{
	int y, x;

	for(y = 0; y < height; y++){
		unsigned char* line = &buffer[y * width * 3];

		for(x = 0; x < width; x++){
			unsigned char* p = &line[x * 3];

			p[0] = x + y + frameIndex * 1;
			p[1] = x + y + frameIndex * 3;
			p[2] = x + y + frameIndex * 5;
		}
	}
}

static int TestEncoder(const char* fileAndPath, int encoderIndex)
{
	if(recorderInitialize(WIDTH, HEIGHT, fileAndPath, FRAMERATE, encoderIndex) == RECORDER_ERROR){
		return RECORDER_ERROR;
	}

	int i;

	for(i = 0; i < FRAMERATE * DURATION; i++){
		FillBuffer(i, buffer, WIDTH, HEIGHT);

		if(recorderAddFrame(buffer) != RECORDER_OK){
			recorderEnd();
			return RECORDER_ERROR;
		}
	}

	while(recorderAddFrame(NULL) == RECORDER_OK);

	if(recorderEnd() != RECORDER_OK){
		return RECORDER_ERROR;
	}

	return RECORDER_OK;
}

int main(int argc, char* argv[])
{
	if(argc != 2){
		const char* appName = strrchr(argv[0], L'\\');

		if(appName == NULL){
			appName = strrchr(argv[0], L'/');

			if(appName == NULL){
				appName = argv[0];
			}else{
				appName += 1;
			}
		}else{
			appName += 1;
		}

		printf("Usage: %s printf-like-path-template\n       For example: %s encoder.%%d\n", appName, appName);

		return -1;
	}

	char name[MAX_NAME];
	char path[FILENAME_MAX];

	int i;

	for(i = 0; getAvailableEncoderName(i, name) == RECORDER_OK; i++){
		const int pos = sprintf(path, argv[1], i);
		
		if(strncmp(name, "AVI/", 4) == 0){
			strcpy(&path[pos], ".avi");
		}else{
			if(strncmp(name, "MP4/", 4) == 0){
				strcpy(&path[pos], ".mp4");
			}else{
				strcpy(&path[pos], ".bin");
			}
		}

		printf("%d) %s - %s\n", i, name, TestEncoder(path, i) == RECORDER_OK ? "OK" : "Failed");
	}

	return 0;
}
