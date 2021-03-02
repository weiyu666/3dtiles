// DTiles.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include"DTiles.h"



void readFileJson(string filePath)
{
	//从文件中读取，保证当前文件有demo.json文件  
	ifstream in(filePath, ios::binary);

	if (!in.is_open())
	{
		cout << "Error opening file\n";
		return;
	}
	//json中的每一行string
	string str;
	string sst;
	while (getline(in, str))
	{
		sst += str;
	}
	//循环递归读取完该json文件并且保存到内存中去，
	//cout << sst << endl;
	JsonData.push_back(sst);

	cout << "old: size = " << JsonData.size() << " ; capacity = " << JsonData.capacity();
	in.close();

}

int writeFileJson(const char* jsonPath)
{
	int ret = -1;
	Json::StyledWriter sw;
	//	Json::StreamWriterBuilder sw;
		//根节点  
	Json::Value val;
	//asset子节点  
	Json::Value assetVal;
	//root子节点  
	Json::Value rootVal;
	//children数组子节点
	Json::Value arrayVal;

	//子节点属性  
	assetVal["gltfUpAxis"] = Json::Value("Z");
	assetVal["version"] = Json::Value("0.0");
	//子节点挂到根节点上  
	val["asset"] = Json::Value(assetVal);

	val["geometricError"] = Json::Value(200);

	//root的region子节点  
	Json::Value root_boundingVolume_regionV;
	//预设的坐标
	double root_region[] = { 1.0E+38, 1.0E+38, -1.0E+38, -1.0E+38, 1.0E+38, -1.0E+38 };

	//children子节点属性  
	for (unsigned int i = 0; i < JsonData.size(); i++)
	{
		//取出每一个子json放到children中去
		string str = JsonData[i];
		Json::Reader reader;
		Json::Value childrenVal;
		Json::Value regionV;

		//从字符串中读取数据  
		if (reader.parse(str, childrenVal))
		{
			arrayVal["transform"].resize(0);
			//读取根节点信息 
			for (unsigned int i = 0; i < childrenVal["root"]["transform"].size(); i++)
			{
				double ach = childrenVal["root"]["transform"][i].asDouble();
				arrayVal["transform"].append(ach);
				//root_transformVector.push_back(ach);
			}

			//读取children，region数组信息  
			for (unsigned int i = 0; i < childrenVal["root"]["boundingVolume"]["region"].size(); i++)
			{
				double achRegion = childrenVal["root"]["boundingVolume"]["region"][i].asDouble();
				regionV["region"].append(achRegion);
				//整体坐标边缘适配 region六坐标分别对应西南东北低高
				switch (i)
				{
					//如果预设坐标小于现有坐标扩展
				case 0:
				case 1:
				case 4:
					if (root_region[i] > achRegion) {
						root_region[i] = achRegion;
					}
					break;
					//如果预设坐标大于现有坐标缩小
				case 2:
				case 3:
				case 5:
					if (achRegion > root_region[i]) {
						root_region[i] = achRegion;
					}
					break;
				}

			}
			arrayVal["boundingVolume"] = Json::Value(regionV);

			string root_refine = childrenVal["root"]["refine"].asString();
			int root_geometricError = childrenVal["root"]["geometricError"].asInt();
			string root_content_uri = childrenVal["root"]["content"]["uri"].asString();
			arrayVal["geometricError"] = Json::Value(root_geometricError);
			arrayVal["refine"] = Json::Value(root_refine);
			arrayVal["content"]["uri"] = Json::Value(root_content_uri);
		}
		//arrayVal.append(arrayVal);
		rootVal["children"].append(arrayVal);
	}

	for (unsigned int i = 0; i < (sizeof(root_region) / sizeof(root_region[0])); i++) {
		root_boundingVolume_regionV["region"].append(root_region[i]);
	}
	rootVal["boundingVolume"] = Json::Value(root_boundingVolume_regionV);


	rootVal["geometricError"] = Json::Value("200");
	rootVal["refine"] = Json::Value("REPLACE");
	//子节点挂到根节点上  
	val["root"] = Json::Value(rootVal);

	//const char* 常量字符串拼接
	const char* outJsonPath = NULL;
	std::string src1 = "\\tileset.json";
	std::string src2 = jsonPath;//char数组自动转为string
	std::string path = src2 + src1;
	outJsonPath = path.c_str();//string转const char*

							   //输出到文件  
	ofstream os;
	os.open(outJsonPath, std::ios::out | std::ios::app);
	if (!os.is_open())
		cout << "error：can not find or create the file which named \" tileset.json\"." << endl;
	os << sw.write(val);
	os.close();
	ret = 0;
	return ret;
}

int findAllFile(const char* path, const char* format) {
	int ret = -1;
	char newpath[200];
	strcpy_s(newpath, path);
	strcat_s(newpath, "\\*.*");    // 在目录后面加上"\\*.*"进行第一次搜索
	long long  handle;				//64位机器必须用longlong ，int是32位
	_finddata_t findData;
	handle = _findfirst(newpath, &findData);
	if (handle == -1)        // 检查是否成功
		return ret;
	while (_findnext(handle, &findData) == 0) {
		if (findData.attrib & _A_SUBDIR) {
			if (strcmp(findData.name, ".") == 0 || strcmp(findData.name, "..") == 0)
				continue;
			strcpy_s(newpath, path);
			strcat_s(newpath, "\\");
			strcat_s(newpath, findData.name);
			findAllFile(newpath, format);
		}
		else {
			if (strstr(findData.name, format)) {     //判断是不是json文件
				std::string str1 = findData.name;
				std::string str2 = path;
				//append(char* )将字符串str2拼接到字符串str1后面。
				str2.append("//");
				str2.append(str1);
				std::cout << str2 << std::endl;
				readFileJson(str2);
				cout << findData.name << "\t" << path << "\t" << findData.size << " bytes.\n";
			}
		}
	}
	_findclose(handle);    // 关闭搜索句柄
	ret = 0;
	return ret;
}

int WriteTilesset(const char* pathname, int& flags)
{
	flags = 90;
	int ret = -1;
	ret = findAllFile(pathname, ".json");
	if (ret == 0)
	{
		flags += 5;
		ret = -1;
	}
	else
	{
		return ret;
	}
	ret = writeFileJson(pathname);
	if (ret == 0)
	{
		flags += 5;
	}
	else
	{
		return ret;
	}
	return ret;
}

// 支持 osg、osgb、obj、fbx、3ds 等单一通用模型数据转为 gltf、glb 格式。其中是2.0的gltf
void convert_gltf(const char* srcPath, const char* destPath) {
	//获取扩展名
	string spath = srcPath;
	int sPos = spath.find_last_of('.');
	string srcSuffixName(spath.substr(sPos + 1));
	string dpath = destPath;
	int dPos = dpath.find_last_of('.');
	string destSuffixName(dpath.substr(dPos + 1));

	char* src;
	const int sLen = srcSuffixName.length();
	src = new char[sLen + 1];
	strcpy(src, srcSuffixName.c_str());
	char* dest;
	const int dLen = destSuffixName.length();
	dest = new char[dLen + 1];
	strcpy(dest, destSuffixName.c_str());

	if (0 != strcmp(dest, GLTF) && 0 != strcmp(dest, GLB)) {
		cout << "output format not support now: " << endl;
	}
	if (0 != strcmp(src, OSGB) && 0 != strcmp(src, OSG) && 0 != strcmp(src, OBJ) && 0 != strcmp(src, FBX) && 0 != strcmp(src, THREEDS)) {
		cout << "input format not support now" << endl;
	}

	bool ret = make_gltf(srcPath, destPath);
	if (!ret) {
		cout << "convert failed" << endl;
	}
	else {
		cout << "task over" << endl;
	}
}


//b3dm 数据转gltf: 支持将 b3dm 单个文件转成 glb 格式，便于调试程序和测试数据
void convert_b3dm(const char* srcPath, const char* destPath) {
	//获取扩展名
	string spath = srcPath;
	int sPos = spath.find_last_of('.');
	string srcSuffixName(spath.substr(sPos + 1));
	string dpath = destPath;
	int dPos = dpath.find_last_of('.');
	string destSuffixName(dpath.substr(dPos + 1));

	char* src;
	const int sLen = srcSuffixName.length();
	src = new char[sLen + 1];
	strcpy(src, srcSuffixName.c_str());
	char* dest;
	const int dLen = destSuffixName.length();
	dest = new char[dLen + 1];
	strcpy(dest, destSuffixName.c_str());

	if (0 != strcmp(src, B3DM)) {
		cout << "input format must be b3dm" << endl;
	}
	if (0 == strcmp(dest, GLTF) && 0 == strcmp(dest, GLB)) {
		cout << "output format not support now: " << endl;
	}

	if (access(srcPath, 0) == 0) {

		FILE *fid;
		fid = fopen(srcPath, "rb");
		fseek(fid, 0, SEEK_END);
		long lSize = ftell(fid);
		rewind(fid);
		//开辟存储空间
		int num = lSize / sizeof(unsigned long int);
		unsigned long int *pos = (unsigned long int*)malloc(sizeof(unsigned long int)*num);
		if (pos == NULL)
		{
			cout << "error: error opening up space" << endl;
			return;
		}

		fread(pos, sizeof(unsigned long int), num, fid);
		//intel本机已经是LittleEndian，所以直接获取
		unsigned long int offset;

		/*.
		偏移量从第12个字节开始读取featureTableJsonLenght，batchTableJsonLenght,
		featureTableBinaryByteLenght,batchTableBinaryByteLenght
		*/
		unsigned long int featureTableJsonLenght = pos[3];

		//读，跳过featureTableBinaryByteLenght去取batchTableJsonLenght
		unsigned long int batchTableJsonLenght = pos[5];
		//得到glb之外 b3dm瓦片文件的前三个部分的字节长包括header featuretable batchtable
		offset = featureTableJsonLenght + batchTableJsonLenght + 28;

		//回到文件头
		fseek(fid, 0, SEEK_END);
		//切片，排除3dtiles.b3dm文件结构头部28+featuretable+batchtable就剩下glb的部分储存到内存buffer中
		char* buffer = new char[num * sizeof(unsigned long int) - offset];

		int result;
		result = fseek(fid, offset, SEEK_SET);
		if (result) {
			printf("error:file fseek failed");
		}
		else {
			//fread一次性就能读完，在IDE中的值因为是二进制不能显示监控值
			fread(buffer, sizeof(char), (num* sizeof(unsigned long int) - offset), fid);//可能遇到“/n”			
		}
		
		//输出到文件  
		FILE *fout;
		fout = fopen(destPath, "wb");
		if (fout == NULL) {
			cout << "error: write file failed" << endl;
			return;
		}

		fwrite(buffer, sizeof(char), (num * sizeof(unsigned long int) - offset), fout);

		delete buffer;  //释放内存
		free(pos);  //释放内存	
		fclose(fid);//释放文件
		fclose(fout);//释放文件
	}
	else {
		cout << "file does not exist" << endl;
	}
	cout << "task over" << endl;
}


void main(int argc, char** argv)
{
	const char* source;
	//source = "C:\\Users\\13624\\Desktop\\2";
	//source = "E:\\shapeb\\2\\min_fangwumian_wgs84.shp";
	source = "F:/yunfu-longshantang/input/Data";//F:\yunfu-longshantang\input\Data
	//source = "E:\\gltfb\\src\\Tile_+000_+000.osgb";
	int layer_id = 0;
	const char* dest;
	//dest = "C:\\Users\\13624\\Desktop\\dest2";
	//dest = "E:\\shapeb\\dest2";
	dest = "F:/yunfu-longshantang/outpit/";//F:\yunfu-longshantang\outpit\
	//dest = "E:\\gltfb\\dest\\me\\Tile_+000_+000.glb";
	const char* height;
	height = "floor";
	int flag=0;//先给个假的

	cout << "Please be patient while transcoding..." << endl;

	if (0 == strcmp(OSGB, type)) {
		cout << "osgb format conversion" << endl;
		osgb_batch_convert(source,dest,100,0,0,0);
	}
	else if (0 == strcmp(SHAPE, type))
	{
		shp23dtile(source, layer_id, dest, height,flag);
		cout << "The transcoding work has ended, and the json file is being parsed" << endl;
		////递归遍历读取目录下所有的json文件
		findAllFile(dest, ".json");
		//findAllFile(source, ".json");
		cout << "The transcoding work has ended, and the json file is being parsed" << endl;
		////写json
		writeFileJson(dest);
	}
	else if (0 == strcmp(GLTF, type))
	{
		convert_gltf(source, dest);
		cout << "gltf format conversion" << endl;
	}
	else if (0 == strcmp(B3DM, type))
	{
		convert_b3dm(source, dest);
		cout << "b3dm format conversion" << endl;
	}
	return;
}

////DLL的主入口替代项目中的main
//void dTilesMain(const char* type, const char* dest, const char* source, int layer_id, const char* height)
//{
//	cout << "Please be patient while tra nscoding..." << endl;
//
//	if (0 == strcmp(OSGB, type)) {
//		cout << "osgb format conversion" << endl;
//	}
//	else if (0 == strcmp(SHAPE, type))
//	{
//		shp23dtile(source, layer_id, dest, height);
//	}
//	else if (0 == strcmp(GLTF, type))
//	{
//		cout << "gltf format conversion" << endl;
//	}
//	else if (0 == strcmp(B3DM, type))
//	{
//		cout << "b3dm format conversion" << endl;
//	}
//
//	cout << "The transcoding work has ended, and the json file is being parsed" << endl;
//	////递归遍历读取目录下所有的json文件
//	findAllFile(dest, ".json");
//	cout << "The transcoding work has ended, and the json file is being parsed" << endl;
//	////写json
//	writeFileJson(dest);
//
//}


bool ShpTo3Dtiles(const char* filename, int layer_id, const char* dest, const char* height, int flag)
{
	bool ret = false;
	flag = 0;
	ret = shp23dtile(filename, layer_id, dest, height, flag);
	if (ret == false)
	{
		return ret;
	}
	flag = 90;
	ret = WriteTilesset(dest, flag);
	if (ret == -1)
	{
		return ret;
	}
	return ret;
}

bool ShapeFileTransformation()
{
	return false;
}
