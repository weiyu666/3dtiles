/*
*extern.h 声明对外的三个接口，其他的定义函数只是跨文件调用不对外~
* 三个接口：
* make_gltf：支持 osg、osgb、obj、fbx、3ds 等单一通用模型数据转为 gltf、glb 格式。转出格式为 2.0 的gltf
* shp23dtile：目前仅支持 shapefile 的面数据，可用于建筑物轮廓批量生成 3dtile。shapefile 中需要有字段来表示高度信息。仅支持WGS84坐标系的矢量数据。
* osgb23dtile_path:
* 
*/
extern "C" bool make_gltf(const char* in_path, const char* out_path);
extern "C" bool shp23dtile(const char* filename, int layer_id, const char* dest, const char* height, int& flag);
extern "C" bool shp23dtile3(const char* filename, int layer_id, const char* dest, const char* height);
extern "C" void* osgb23dtile_path(const char* in_path, const char* out_path, double* box, int* len, double x, double y, int max_lvl);
extern "C" void transform_c(double center_x, double center_y, double height_min, double* ptr);
extern "C"  bool osgb_batch_convert(const char* dir, const char* dir_dest, int max_lvl, double center_x, double center_y, double region_offset);


#ifdef WIN32
#define LOG_E(fmt,...) \
			char buf[512];\
			sprintf_s(buf,fmt,__VA_ARGS__);\
			log_error(buf);
#else
#define LOG_E(fmt,...) \
			char buf[512];\
			sprintf_s(buf,fmt,##__VA_ARGS__);\
			log_error(buf);
#endif

//// -- others 
struct Transform
{
	double radian_x;
	double radian_y;
	double min_height;
};

struct Box
{
	double matrix[12];
};

struct Region
{
	double min_x;
	double min_y;
	double max_x;
	double max_y;
	double min_height;
	double max_height;
};

bool write_tileset_region(
	Transform* trans,
	Region& region,
	double geometricError,
	const char* b3dm_file,
	const char* json_file);

bool write_tileset_box(
	Transform* trans, Box& box,
	double geometricError,
	const char* b3dm_file,
	const char* json_file);

bool write_tileset(
	double longti, double lati,
	double tile_w, double tile_h,
	double height_min, double height_max,
	double geometricError,
	const char* filename, const char* full_path
);

#include<iostream>
bool write_tileset1(
	double radian_x, double radian_y,
	double tile_w, double tile_h,
	double height_min, double height_max,
	double geometricError,
	const char* filename, const char* full_path, std::string childstr
);

extern "C" bool mkdirs(const char* path);
extern "C" bool write_file(const char* filename, const char* buf, unsigned long buf_len);
extern "C" void log_error(const char* msg);

extern "C" {
	double degree2rad(double val);
	double lati_to_meter(double diff);
	double longti_to_meter(double diff, double lati);
	double meter_to_lati(double m);
	double meter_to_longti(double m, double lati);
};

////////////////////////