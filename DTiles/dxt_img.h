#ifndef DXT_IMG_H
#define DXT_IMG_H

#include <vector>
#include <osg/Image>
using namespace std;

struct Color {
	int r;
	int g;
	int b;
};

void fill_4BitImage(std::vector<unsigned char>& jpeg_buf, osg::Image* img, int& width, int& height);



#endif