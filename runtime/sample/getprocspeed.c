#include <stdio.h>

#include <cv.h>
#include <highgui.h>

#include <time.h>

#include "../picort.h"

/*
	portable time function
*/

#ifdef __GNUC__
#include <time.h>
float getticks()
{
	struct timespec ts;

	if(clock_gettime(CLOCK_MONOTONIC, &ts) < 0)
	{
		printf("clock_gettime error\n");

		return -1.0f;
	}

	return ts.tv_sec + 1e-9f*ts.tv_nsec;
}
#else
#include <windows.h>
float getticks()
{
	static double freq = -1.0;
	LARGE_INTEGER lint;

	if(freq < 0.0)
	{
		if(!QueryPerformanceFrequency(&lint))
			return -1.0f;

		freq = lint.QuadPart;
	}

	if(!QueryPerformanceCounter(&lint))
		return -1.0f;

	return (float)( lint.QuadPart/freq );
}
#endif

/*
	
*/

int minsize = 0;

float process_image(IplImage* frame, int niters)
{
	int i;
	float tstart, tend;

	uint8_t* pixels;
	int nrows, ncols, ldim;

	#define MAXNDETECTIONS 8192
	int ndetections;
	static float qs[MAXNDETECTIONS], rs[MAXNDETECTIONS], cs[MAXNDETECTIONS], ss[MAXNDETECTIONS];

	static IplImage* gray = 0;

	// a structure that encodes object appearance
	static char appfinder[] =
		{
			#include "../cascades/facefinder.ea"
		};

	// grayscale image
	if(!gray)
		gray = cvCreateImage(cvSize(frame->width, frame->height), frame->depth, 1);
	if(frame->nChannels == 3)
		cvCvtColor(frame, gray, CV_RGB2GRAY);
	else
		cvCopy(frame, gray, 0);

	// get relevant image data
	pixels = (uint8_t*)gray->imageData;
	nrows = gray->height;
	ncols = gray->width;
	ldim = gray->widthStep;

	//
	tstart = getticks();

	for(i=0; i<niters; ++i)
		find_objects(rs, cs, ss, qs, MAXNDETECTIONS, appfinder, pixels, nrows, ncols, ldim, 1.2f, 0.1f, minsize, MIN(nrows, ncols), 1);

	tend = getticks();

	//
	return 1000.0f*(tend-tstart)/niters;
}

int main(int argc, char* argv[])
{
	int n;
	IplImage* img = 0;

	if(argc!=0)
	{
		printf("Invalid number of arguments.");
		return 0;
	}

	//
	sscanf(argv[1], "%d", &minsize);
	img = cvLoadImage(argv[2], CV_LOAD_IMAGE_COLOR);
	sscanf(argv[3], "%d", &n);

	//
	printf("%f\n", process_image(img, n));

	//
	return 0;
}
