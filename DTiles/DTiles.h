#ifndef _D_TILES_
#define _D_TILES_

#include <io.h>		//_finddata_t的头文件
#include <iostream>
#include <fstream>
#include <cstring>
#include <gdal_priv.h>
#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <json/json.h>


#include "extern.h"
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

const char* type = "osgb";
const char* OBJ = "obj";
const char* OSG = "osg";
const char* OSGB = "osgb";
const char* SHAPE = "shape";
const char* GLTF = "gltf";
const char* GLB = "glb";
const char* B3DM = "b3dm";
const char* FBX = "fbx";
const char* THREEDS = "3ds";

//读取到的json数据保存在内存在
vector<string> JsonData;

void readFileJson(string filePath);
int findAllFile(const char* path, const char* format);
int writeFileJson(const char* jsonPath);
int WriteTilesset(const char* pathname, int& flags);


void convert_b3dm(const char* srcPath, const char* destPath);
void convert_gltf(const char* srcPath, const char* destPath);


//DLL的主入口替代项目中的main
extern "C" __declspec(dllexport) void  dTilesMain(const char* type, const char* dest, const char* source, int layer_id, const char* height);

extern "C" __declspec(dllexport) bool ShpTo3Dtiles(const char* filename, int layer_id, const char* dest, const char* height, int flag);

extern "C" __declspec(dllexport) bool ShapeFileTransformation();
#endif // !_3D_TILES_

