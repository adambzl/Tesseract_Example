#include "stdafx.h"

using std::endl;
using std::cout;
using std::cerr;
using std::string;
using std::to_string;
using std::vector;
using std::regex;
using std::smatch;

//将utf-8字符串转化为string，参考https://www.cnblogs.com/shike8080/archive/2017/01/18/6297210.html
string UtfToGbk(const char* utf8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	//将一个char数组按照指定的编码方式理解之后转化为宽字符wchar_t编码
	//不指定接收区域的含义是返回转换所需的宽字节数据量
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
	//再转回ASCLL码，因为string 用ASCLL编码
	if (wstr) delete[] wstr;
	return str;
}


bool line_recognition(vector<string> &v, vector<unsigned> &v_c, bool isChin, string &path, unsigned numOfRows) {
	//单行识别函数，isChin为true表示做中文识别，否则为做英文识别
	//api对象初始化及设置
	tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
	if (isChin) {
		if (api->Init(nullptr, "chi_sim")) {
			cerr << "识别句柄初始化失败" << endl;
			return false;
		}
	}
	else {
		if (api->Init(nullptr, "eng")) {
			cerr << "识别句柄初始化失败" << endl;
			return false;
		}
	}

	api->SetPageSegMode(tesseract::PSM_SINGLE_LINE);
	char *result;
	string s;
	for (int i = 1; i <= numOfRows; i++) {
		string imagePath = path + to_string(i) + ".jpg";
		Pix *image = pixRead(imagePath.c_str());
		if (image == nullptr) {
			cerr << "图片" << i << "读取失败" << endl;
			return false;
		}
		api->SetImage(image);
		result = api->GetUTF8Text();
		unsigned conf = api->MeanTextConf();
		if (isChin)
			s = UtfToGbk(result);
		else
			s = string(result);
		while (s.find('\n') != string::npos) {
			size_t pos = s.find('\n');
			s.replace(pos, 1, "");
		}
		v.push_back(s);
		v_c.push_back(conf);//应该是float类型...
		pixDestroy(&image);
	}
	api->End();
	return true;
}

bool line_recognition_char(vector<vector<string>> &v, vector<vector<unsigned>> &v_c, bool isChin, string path, unsigned numOfRows) {
	//对每一行的图片，使用API进行拆分字符的识别(整体结果是不变的，但这样可以给出坐标信息，最后用于综合比较判定)
	tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
	if (isChin) {
		if (api->Init(nullptr, "chi_sim")) {
			cerr << "识别句柄初始化失败" << endl;
			return false;
		}
	}
	else {
		if (api->Init(nullptr, "eng")) {
			cerr << "识别句柄初始化失败" << endl;
			return false;
		}
	}
	char *result;
	string s;
	for (int i = 1; i <= numOfRows; i++) {
		vector<string> temp_vec;
		v.push_back(temp_vec);
		vector<unsigned> temp_vec_c;
		v_c.push_back(temp_vec_c);
		string imagePath = path + to_string(i) + ".jpg";
		Pix *image = pixRead(imagePath.c_str());
		if (image == nullptr) {
			cerr << "图片" << i << "读取失败" << endl;
			return false;
		}
		api->SetImage(image);
		api->SetVariable("save_blob_choices", "T");
		api->Recognize(0);
		tesseract::ResultIterator *ri = api->GetIterator();
		tesseract::PageIteratorLevel level = tesseract::RIL_SYMBOL;
		//此参数设置为word时，第一行识别结果依次为 卧 中华人民共和国最高人民检察院 卧
		if (ri != nullptr) {
			do {
				result = ri->GetUTF8Text(level);
				if (result == nullptr)
					continue;
				unsigned conf = ri->Confidence(level);
				if (isChin)
					s = UtfToGbk(result);
				else
					s = string(result);
				while (s.find('\n') != string::npos) {
					size_t pos = s.find('\n');
					s.replace(pos, 1, "");
				}
				int x1, y1, x2, y2;
				ri->BoundingBox(level, &x1, &y1, &x2, &y2);
				v[i - 1].push_back(s);
				v_c[i - 1].push_back(conf);

			} while (ri->Next(level));
		}
		pixDestroy(&image);
	}
	api->End();
	return true;
}

bool char_recognition(vector<vector<string>> &v, vector<vector<unsigned>> &v_c, bool isChin, string path, unsigned numOfRows) {
	//单字符识别，vector中的每个vector存储一行
	tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
	if (isChin) {
		if (api->Init(nullptr, "chi_sim")) {
			cerr << "识别句柄初始化失败" << endl;
			return false;
		}
	}
	else {
		if (api->Init(nullptr, "eng")) {
			cerr << "识别句柄初始化失败" << endl;
			return false;
		}
	}
	char *result;
	string s;
	int count = 1;
	for (int i = 1; i <= numOfRows; i++) {
		vector<string> temp_vec;
		v.push_back(temp_vec);//temp_vec是临时变量，函数退出后会被析构，但是复制到v中的内容不会，因为v是在main中定义的
		vector<unsigned> temp_vec_c;
		v_c.push_back(temp_vec_c);
		while (1) {
			string imagePath = path + to_string(i) + "_" + to_string(count) + ".jpg";
			Pix *image = pixRead(imagePath.c_str());
			count = count + 1;
			if (count > 1200) {
				count = 1;
				break;
			}
			if (image == nullptr)
				continue;
			api->SetImage(image);
			api->SetVariable("save_blob_choices", "T");
			api->Recognize(0);
			tesseract::ResultIterator *ri = api->GetIterator();
			tesseract::PageIteratorLevel level = tesseract::RIL_SYMBOL;
			do {
				result = ri->GetUTF8Text(level);
				unsigned conf = api->MeanTextConf();
				if (isChin)
					s = UtfToGbk(result);
				else
					s = string(result);
				while (s.find('\n') != string::npos) {
					size_t pos = s.find('\n');
					s.replace(pos, 1, "");
				}
				v[i - 1].push_back(s);
				v_c[i - 1].push_back(conf);
			} while (ri->Next(level));
			pixDestroy(&image);
		}
	}
	api->End();
	return true;
}

void split_sift(vector<string> &words, vector<unsigned> &confs, bool isChin, vector<string> &v, vector<unsigned> &v_c) {
	//输入一个包含一系列识别结果片段的列表，输出的列表每个元素都是单个字符而且将一些
	//无把握或特殊的字符替换为空格
	if (!isChin) {
		//如果是英文结果，只保留字母和数字，其余全部替换为空格
		auto iter_v = words.cbegin();
		auto iter_c = confs.cbegin();
		while (iter_v != words.cend()) {
			string temp = *iter_v;
			for (string::size_type i = 0; i < temp.size(); i++) {
				if (!((temp[i] >= 48 && temp[i] <= 57) || (temp[i] >= 65 && temp[i] <= 90) || (temp[i] >= 97 && temp[i] <= 122))){//英文保留所有的数字和字母的可能性
					v.push_back("?");
					v_c.push_back(*iter_c);
				}
				else {
					v.push_back(string(1, temp[i]));
					v_c.push_back(*iter_c);
				}
			}
			iter_v++;
			iter_c++;
		}
	}
	else {
		//恶心的中文
		auto iter_v = words.cbegin();
		auto iter_c = confs.cbegin();
		while (iter_v != words.cend()) {
			string temp = *iter_v;
			vector<string> temp_v;
			//首先将字符串按照中英文打散
			string::size_type i = 0;
			while (i != temp.size()) {
				if (temp[i] >= 0 && temp[i] <= 127) {
					temp_v.push_back(string(1, temp[i]));
					i++;
				}
				else {
					temp_v.push_back(temp.substr(i, 2));
					i += 2;
				}
			}
			//再对temp_v中的内容进行处理
			auto iter = temp_v.cbegin();
			while (iter != temp_v.cend()) {
				if((*iter).size() == 1)
					if ((*iter)[0] >= 48 && (*iter)[0] <= 57 && (*iter_c) >= 70) {//只保留数字
						v.push_back(*iter);
						v_c.push_back(*iter_c);
					}
					else {
						v.push_back("?");
						v_c.push_back(*iter_c);
					}
				else {
					unsigned id1 = 256 + (*iter)[0];
					unsigned id2 = 256 + (*iter)[1];
					if (id1 >= 0xB0 && id1 <= 0xF7 && id2 >= 0xA1 && id2 <= 0xFE && (*iter_c) >= 70) {
						//要求汉字必须位于GB2312汉字区
						v.push_back(*iter);
						v_c.push_back(*iter_c);
					}
					else {
						v.push_back("?");
						v_c.push_back(*iter_c);
					}
				}
				iter++;
			}

			iter_v++;
			iter_c++;
		}
	}
	return;
}

string findFirstCommonStr(const string &s1, const string &s2, std::string::size_type &pos_s1, std::string::size_type &pos_s2) {
	//寻找两个字符串中第一次出现的公共子串，如果没有找到，则返回空字符串
	//如果是中文字符，则必须两个两个字节进行比较
	//pos返回的是较短字符串的匹配位置
	string commonStr = "";
	pos_s1 = pos_s2 = string::npos;
	const string &s_short = s1.size() <= s2.size() ? s1 : s2;
	const string &s_long = s1.size() > s2.size() ? s1 : s2;
	bool mark = false;
	bool isChin = true;
	string::size_type pos_short = 0, pos_long = 0;
	while (pos_short < s_short.size()) {
		if (s_short[pos_short] >= 0 && s_short[pos_short] <= 127)
			isChin = false;
		else
			isChin = true;
		while (pos_long < s_long.size()) {
			//首先判断当前字符是不是一个中文字符
			if (!(s_long[pos_long] >= 0 && s_long[pos_long] <= 127)) {
				if (isChin) {
					if (s_long[pos_long] == s_short[pos_short] && s_long[pos_long + 1] == s_short[pos_short + 1]) {
						mark = true;
						break;
					}
					else {
						pos_long += 2;
						continue;
					}
				}
				else {
					pos_long += 2;
					continue;
				}
			}
			else {
				if (isChin) {
					pos_long += 1;
					continue;
				}
				else {
					if (s_long[pos_long] == s_short[pos_short]) {
						mark = true;
						break;
					}
					else {
						pos_long += 1;
						continue;
					}
				}
			}
			
		}
		if (mark == true) {
			//找出这个匹配子串
			//如果要找最长匹配子串，则设置一个临时变量在每一次循环中不断替换，而不是一找到就跳出
			string::size_type i = isChin ? pos_short + 2 : pos_short;
			string::size_type j = isChin ? pos_long + 2 : pos_long;
			//从当前匹配字符开始，找到整个匹配子串
			while (1) {
				if (i >= s_short.size()) {
					commonStr = s_short.substr(pos_short);
					break;
				}
				else if (j >= s_long.size()) {
					commonStr = s_long.substr(pos_long);
					break;
				}
				if (s_short[i] >= 0 && s_short[i] <= 127) {
					if (s_short[i] == s_long[j]) {
						i++;
						j++;
					}
					else {
						commonStr = s_short.substr(pos_short, i - pos_short);
						break;
					}		
				}
				else {
					if (s_short[i] == s_long[j] && s_short[i + 1] == s_long[j + 1]) {
						i += 2;
						j += 2;
					}
					else {
						commonStr = s_short.substr(pos_short, i - pos_short);
						break;
					}		
				}
			}
			if (s1.size() <= s2.size()) {
				pos_s1 = pos_short;
				pos_s2 = pos_long;
			}
			else {
				pos_s2 = pos_short;
				pos_s1 = pos_long;
			}
			break;
		}

		pos_long = 0;//当前字符在长串中不存在，下一个字符重新从长串开始位置搜索
		if (isChin)
			pos_short += 2;
		else
			pos_short++;
	}
	return commonStr;
}

void assembleChars(const std::string &sentence1, const std::string &sentence2, std::vector<std::string> &v_sentences) {
	//传入的是两种识别方式下每一行已经经过预处理之后的有效句子
	string s1 = sentence1;
	string s2 = sentence2;
	vector<string> temp, result;
	result.push_back("");
	while (1) {
		string::size_type pos1 = string::npos, pos2 = string::npos;
		string commonStr = findFirstCommonStr(s1, s2, pos1, pos2);
		//下面的条件分支是一些特殊情况的处理
		if (s1 == "" && s2 == "")
			break;
		else if (s1 == "") {
			for (auto &s : result)
				temp.push_back(s + s2);
			result.clear();
			result = temp;
			temp.clear();
			break;
		}
		else if (s2 == "") {
			for (auto &s : result)
				temp.push_back(s + s1);
			result.clear();
			result = temp;
			temp.clear();
			break;
		}

		//几何去重是一种最后的解决方案，这里我们采用组合去重，提高空间复杂度降低算法实现的难度
		//从资源消耗的角度讲，这样也是比较合适的

		if (commonStr == "") {
			//查找结束
			for (auto &s : result) {
				temp.push_back(s + s1 + s2);
				temp.push_back(s + s2 + s1);
				//组合去重
				temp.push_back(s + s1);
				temp.push_back(s + s2);
				temp.push_back(s);//两个全部是错误识别，这种情况有一定的可能性
			}
			result.clear();
			result = temp;
			temp.clear();
			break;
		}
		else if (pos1 != 0 && pos2 != 0) {
			string temp1 = s1.substr(0, pos1);
			string temp2 = s2.substr(0, pos2);
			s1 = s1.substr(pos1 + commonStr.size());
			s2 = s2.substr(pos2 + commonStr.size());
			for (auto &s : result) {
				temp.push_back(s + temp1 + temp2 + commonStr);
				temp.push_back(s + temp2 + temp1 + commonStr);
				temp.push_back(s + temp1 + commonStr);
				temp.push_back(s + temp2 + commonStr);
				temp.push_back(s + commonStr);
				//实际上还有一种commonStr也是错误识别的情况，但是可能性很低
			}
			result.clear();
			result = temp;
			temp.clear();
		}
		else {
			if (pos1 == 0) {
				string temp2 = s2.substr(0, pos2);
				s1 = s1.substr(pos1 + commonStr.size());
				s2 = s2.substr(pos2 + commonStr.size());
				for (auto &s : result) {
					temp.push_back(s + temp2 + commonStr);
					temp.push_back(s + commonStr);
				}
					
				result.clear();
				result = temp;
				temp.clear();
			}
			else {
				string temp1 = s1.substr(0, pos1);
				s1 = s1.substr(pos1 + commonStr.size());
				s2 = s2.substr(pos2 + commonStr.size());
				for (auto &s : result) {
					temp.push_back(s + temp1 + commonStr);
					temp.push_back(s + commonStr);
				}
					
				result.clear();
				result = temp;
				temp.clear();
			}
		}
	}
	v_sentences = result;
	return;
}

void deleteStr(const std::vector<std::string> &v_in, const std::vector<unsigned> &c_in, std::vector<std::string> &v_out, std::vector<unsigned> &c_out, const std::string &str) {
	//删除给定容器中的指定字符串
	auto iter_s = v_in.cbegin();
	auto iter_c = c_in.cbegin();
	for (; iter_s != v_in.cend(); iter_s++, iter_c++) {
		if ((*iter_s) != str) {
			v_out.push_back(*iter_s);
			c_out.push_back(*iter_c);
		}
	}
	return;
}

bool getID(string path, int numOfRows, vector<string> &ids, vector<unsigned> &ids_conf) {
	//获取所有可能的身份证号码
	vector<vector<string>> v;
	vector<vector<unsigned>> v_c;
	if (!line_recognition_char(v, v_c, false, path, numOfRows))
		return false;
	for (int i = 1; i <= numOfRows; i++) {
		vector<string> temp_v;
		vector<unsigned> temp_c;
		split_sift(v[i - 1], v_c[i - 1], false, temp_v, temp_c);
		string line = "";
		//获取该行的识别结果
		//string中字符的位置和v中字符的位置是一一对应的
		for (auto &s : temp_v)
			line += s;
		regex id_regex("[0-9]{17}[0-9Xx]");
		smatch sm;
		while (1) {
			if (!regex_search(line, sm, id_regex))
				break;
			string::size_type pos = line.find(sm.str(0));
			//计算平均置信概率
			unsigned conf = 0;
			for (int j = 0; j < 18; j++)
				conf += temp_c[pos + j];
			conf = conf / 18.0;
			ids_conf.push_back(conf);
			ids.push_back(sm.str(0));
			line = line.substr(pos + 18);
		}
	}
	if (ids.size() == 0)
		return false;
	else
		return true;
}