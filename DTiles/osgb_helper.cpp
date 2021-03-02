#include "osgb_helper.h"

using namespace std;
using json = nlohmann::json;

struct TileResult {
	std::string path;
	std::string json;
	double box_v[6];
};

struct OsgbInfo {
	string in_dir;
	string out_dir;
	TileResult tileResult;
};


void getFiles(string path, vector<string>& files)
{
	//文件句柄  
	unsigned long long    hFile = 0;
	//文件信息  
	struct _finddata_t fileinfo;
	string p;
	if ((hFile = _findfirst(p.assign(path).append("//*").c_str(), &fileinfo)) != -1)
	{
		do
		{
			//如果是目录,迭代之  
			//如果不是,加入列表  
			if ((fileinfo.attrib & _A_SUBDIR))
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					files.push_back(p.assign(path).append("\\").append(fileinfo.name));
			}

		} while (_findnext(hFile, &fileinfo) == 0);
		_findclose(hFile);
	}
}


double* box_to_tileset_box(double box_v[])
{
	double* box_new = new double[12];
	box_new[0] = ((box_v[0] + box_v[3]) / 2.0);
	box_new[1] = ((box_v[1] + box_v[4]) / 2.0);
	box_new[2] = ((box_v[2] + box_v[5]) / 2.0);

	box_new[3] = ((box_v[3] - box_v[0]) / 2.0);
	box_new[4] = (0.0);
	box_new[5] = (0.0);

	box_new[6] = (0.0);
	box_new[7] = ((box_v[4] - box_v[1]) / 2.0);
	box_new[8] = (0.0);

	box_new[9] = (0.0);
	box_new[10] = (0.0);
	box_new[11] = ((box_v[5] - box_v[2]) / 2.0);
	return box_new;
}

string& replace_all(string& str, const   string& old_value, const   string& new_value)
{
	while (true) {
		string::size_type   pos(0);
		if ((pos = str.find(old_value)) != string::npos)
			str.replace(pos, old_value.length(), new_value);
		else   break;
	}
	return   str;
}


string& replace_all_distinct(string& str, const   string& old_value, const   string& new_value)
{
	for (string::size_type pos(0); pos != string::npos; pos += new_value.length()) {
		if ((pos = str.find(old_value, pos)) != string::npos)
			str.replace(pos, old_value.length(), new_value);
		else   break;
	}
	return   str;
}

extern "C"  bool osgb_batch_convert(const char* dir, const char* dir_dest, int max_lvl, double center_x, double center_y, double region_offset)
{
	char path[1024];
	strcpy(path, dir);
	/*strcat(path,"\\Data");*/

	/* Check for exist */
	if (!((_access(path, 0)) != -1))
	{
		log_error("目录不存在");
		return false;
	}

	std::vector<OsgbInfo> osgb_dir_pair;
	int task_count = 0;
	bool suc = mkdirs(dir_dest);


	std::vector<std::string> filedirs;
	getFiles(path, filedirs);//找到path下的下级所有目录

	for (auto path_tile : filedirs)
	{
		//vector<string> vec = split(path_tile, ‘, ‘);
		int pos = path_tile.find_last_of("\\");
		string stem = path_tile.substr(pos + 1, -1);
		string osgb = path_tile + "\\" + stem + ".osgb";
		fstream _file;
		_file.open(osgb, ios::in);
		if (_file)
		{
			task_count++;
			string out_dir = std::string(dir_dest) + "\\Data\\" + stem;
			mkdirs(out_dir.c_str());
			double box_v[6];
			TileResult tileResult = { "","", *box_v };
			OsgbInfo osgbInfo{ osgb,out_dir,tileResult };
			osgb_dir_pair.push_back(osgbInfo);
		}
		else
		{
			log_error("OSGB文件不存在");
		}
	}

	double rad_x = degree2rad(center_x);//当前盒子中心点x 的 弧度
	double rad_y = degree2rad(center_y);//当前盒子中心点y 的 宽度

	std::vector<TileResult> tile_array;

	int currentCount = 0;
	for (auto info : osgb_dir_pair)
	{
		double root_box[6];
		char json_buf[1024000];
		int json_len;
		string in_ptr = info.in_dir;
		string out_ptr = info.out_dir;
		char* out_ptr1 = (char*)osgb23dtile_path(in_ptr.c_str(), out_ptr.c_str(), root_box, &json_len, rad_x, rad_y, max_lvl);
		currentCount++;
		cout << currentCount << endl;

		if (!out_ptr1)
		{
			log_error("读取路径失败");
		}
		else
		{
			TileResult tileResult;
			tileResult.path = out_ptr;
			for (int i = 0; i < 6; i++)
			{
				tileResult.box_v[i] = root_box[i];
			}
			memcpy(json_buf, out_ptr1, json_len);
			tileResult.json = string(json_buf, json_len);
			tile_array.push_back(tileResult);
			free(out_ptr1);

			//memcpy(json_buf, out_ptr1, json_len);
			//free(out_ptr1);
		}

		//TileResult tileResult = { out_ptr,json_buf,*root_box };
		/*TileResult tileResult;
		tileResult.path = out_ptr;
		tileResult.json = json_buf;
		for (int i = 0; i < 6; i++)
		{
			tileResult.box_v[i] = root_box[i];
		}
		tile_array.push_back(tileResult);*/
	}

	double root_box[6] = { -1.0E+38, -1.0E+38, -1.0E+38, 1.0E+38, 1.0E+38, 1.0E+38 };//West South East North Min.height Max.height
	for (auto x : tile_array)
	{
		if (x.box_v[0] > root_box[0]) {
			root_box[0] = x.box_v[0];
		}
		if (x.box_v[1] > root_box[1]) {
			root_box[1] = x.box_v[1];
		}
		if (x.box_v[2] > root_box[2]) {
			root_box[2] = x.box_v[2];
		}
		if (x.box_v[3] < root_box[3]) {
			root_box[3] = x.box_v[3];
		}
		if (x.box_v[4] < root_box[4]) {
			root_box[4] = x.box_v[4];
		}
		if (x.box_v[5] < root_box[5]) {
			root_box[5] = x.box_v[5];
		}
	}
	double tras_height = region_offset - root_box[5];//??????????????????
	double trans_vec[16];

	transform_c(center_x, center_y, tras_height, trans_vec);//x\y\z变换

	std::string json_txt = "{\"asset\": {\"version\": \"1.0\",\"gltfUpAxis\": \"Z\"},\"geometricError\":2000,";

	json_txt += "\"root\":{\"transform\" :[";

	string trans_str = "";
	for (int i = 0; i < 15; i++) {
		double temp = trans_vec[i];
		trans_str += std::to_string(temp);
		trans_str += ",";
	}
	trans_str += std::to_string(trans_vec[15]);
	//-0.9162719395719664,-0.4005567784384965,0.0,0.0,0.15747527219549448,-0.3602240203041381,0.9194782182536347,0.0,-0.3679188975787587,0.841612924898813,0.39537750245518016,0.0,-2347865.962039195(-2347855.728848),5370733.475736132(5370710.067354),2506201.8765807077(2506190.879664),1.0(1.0)
	json_txt += trans_str;


	json_txt += "], \"boundingVolume\" : {\"box\":[";
	double* box_new;
	box_new = box_to_tileset_box(root_box);
	trans_str = "";
	for (int i = 0; i < 11; i++) {
		double temp = box_new[i];
		trans_str += std::to_string(temp);
		trans_str += ",";
	}
	trans_str += std::to_string(box_new[11]);
	//root_box [-631.2118835449219,-418.2552032470703,43.83972454071045,-39.03482666015623,0.0,0.0,0.0,-34.286773681640625,0.0,0.0,0.0,-16.026009750366214]

	json_txt += trans_str + "]},\
		\"geometricError\" : 2000,\
			\"children\" : []\
		}}";

	json root_json = json::parse(json_txt);
	//cout << root_json["asset"]["version"] << endl;

	//root_json["asset"]["version"] = "teset";

	string out_dir = dir_dest;
	try
	{
		for (auto item : tile_array)
		{
			string path = item.path;
			string itemstr = item.json;
			json json_val = json::parse(itemstr);
			double tilebox[12];
			json arrayObj = json_val["boundingVolume"]["box"];

			int size = arrayObj.size();
			for (int i = 0; i < size; ++i)
			{
				tilebox[i] = arrayObj[i];
			}

			string tile_json_str = "{\"boundingVolume\": { \"box\":[";
			string trans_str = "";
			for (int i = 0; i < 11; i++) {
				trans_str += std::to_string(tilebox[i]);
				trans_str += ",";
			}
			trans_str += std::to_string(tilebox[11]);
			tile_json_str += trans_str;
			//"box": [-631.211884,-418.255203,43.839725,32.529022,0.0,0.0,0.0,28.572311,0.0,0.0,0.0,13.355008]
			tile_json_str += "]},\"geometricError\": 1000,\"content\" : {\"uri\" :\"";

			string curpath = path;
			curpath = curpath.replace(curpath.begin(), curpath.begin() + out_dir.length(), ".");
			curpath += "/tileset.json";
			curpath = replace_all_distinct(curpath, "\\", "/");//  "./Data/Tile_+000_+000/tileset.json"

			tile_json_str += curpath + "\"}}";

			//json tile_object(tile_json_str);
			json tile_object = json::parse(tile_json_str);


			root_json["root"]["children"].push_back(tile_object);

			string sub_tile_str = "{\"asset\": {\"gltfUpAxis\" : \"Z\",\"version\":\"1.0\"},\"root\":" + item.json + "}";

			//json sub_tile(sub_tile_str);
			json sub_tile = json::parse(sub_tile_str);

			string out_file = path + "/tileset.json";

			write_file(out_file.c_str(), sub_tile_str.data(), sub_tile_str.size());//写入subtile

		}
	}
	catch (char* str)
	{
		cout << str << endl;
	}
	catch (int i)
	{
		cout << i << endl;
	}
	string root_json_dir = (std::string)dir_dest + "//tileset.json";
	string root_json_str = root_json.dump();

	write_file(root_json_dir.c_str(), root_json_str.data(), root_json_str.size());//写入subtile
}