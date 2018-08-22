#include "stdafx.h"

using std::endl;
using std::cout;
using std::cerr;
using std::string;
using std::to_string;
using std::vector;
using std::regex;
using std::smatch;

//��utf-8�ַ���ת��Ϊstring���ο�https://www.cnblogs.com/shike8080/archive/2017/01/18/6297210.html
string UtfToGbk(const char* utf8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
	//��һ��char���鰴��ָ���ı��뷽ʽ���֮��ת��Ϊ���ַ�wchar_t����
	//��ָ����������ĺ����Ƿ���ת������Ŀ��ֽ�������
	wchar_t* wstr = new wchar_t[len + 1];
	memset(wstr, 0, len + 1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
	char* str = new char[len + 1];
	memset(str, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
	//��ת��ASCLL�룬��Ϊstring ��ASCLL����
	if (wstr) delete[] wstr;
	return str;
}


bool line_recognition(vector<string> &v, vector<unsigned> &v_c, bool isChin, string &path, unsigned numOfRows) {
	//����ʶ������isChinΪtrue��ʾ������ʶ�𣬷���Ϊ��Ӣ��ʶ��
	//api�����ʼ��������
	tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
	if (isChin) {
		if (api->Init(nullptr, "chi_sim")) {
			cerr << "ʶ������ʼ��ʧ��" << endl;
			return false;
		}
	}
	else {
		if (api->Init(nullptr, "eng")) {
			cerr << "ʶ������ʼ��ʧ��" << endl;
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
			cerr << "ͼƬ" << i << "��ȡʧ��" << endl;
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
		v_c.push_back(conf);//Ӧ����float����...
		pixDestroy(&image);
	}
	api->End();
	return true;
}

bool line_recognition_char(vector<vector<string>> &v, vector<vector<unsigned>> &v_c, bool isChin, string path, unsigned numOfRows) {
	//��ÿһ�е�ͼƬ��ʹ��API���в���ַ���ʶ��(�������ǲ���ģ����������Ը���������Ϣ����������ۺϱȽ��ж�)
	tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
	if (isChin) {
		if (api->Init(nullptr, "chi_sim")) {
			cerr << "ʶ������ʼ��ʧ��" << endl;
			return false;
		}
	}
	else {
		if (api->Init(nullptr, "eng")) {
			cerr << "ʶ������ʼ��ʧ��" << endl;
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
			cerr << "ͼƬ" << i << "��ȡʧ��" << endl;
			return false;
		}
		api->SetImage(image);
		api->SetVariable("save_blob_choices", "T");
		api->Recognize(0);
		tesseract::ResultIterator *ri = api->GetIterator();
		tesseract::PageIteratorLevel level = tesseract::RIL_SYMBOL;
		//�˲�������Ϊwordʱ����һ��ʶ��������Ϊ �� �л����񹲺͹����������Ժ ��
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
	//���ַ�ʶ��vector�е�ÿ��vector�洢һ��
	tesseract::TessBaseAPI *api = new tesseract::TessBaseAPI();
	if (isChin) {
		if (api->Init(nullptr, "chi_sim")) {
			cerr << "ʶ������ʼ��ʧ��" << endl;
			return false;
		}
	}
	else {
		if (api->Init(nullptr, "eng")) {
			cerr << "ʶ������ʼ��ʧ��" << endl;
			return false;
		}
	}
	char *result;
	string s;
	int count = 1;
	for (int i = 1; i <= numOfRows; i++) {
		vector<string> temp_vec;
		v.push_back(temp_vec);//temp_vec����ʱ�����������˳���ᱻ���������Ǹ��Ƶ�v�е����ݲ��ᣬ��Ϊv����main�ж����
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
	//����һ������һϵ��ʶ����Ƭ�ε��б�������б�ÿ��Ԫ�ض��ǵ����ַ����ҽ�һЩ
	//�ް��ջ�������ַ��滻Ϊ�ո�
	if (!isChin) {
		//�����Ӣ�Ľ����ֻ������ĸ�����֣�����ȫ���滻Ϊ�ո�
		auto iter_v = words.cbegin();
		auto iter_c = confs.cbegin();
		while (iter_v != words.cend()) {
			string temp = *iter_v;
			for (string::size_type i = 0; i < temp.size(); i++) {
				if (!((temp[i] >= 48 && temp[i] <= 57) || (temp[i] >= 65 && temp[i] <= 90) || (temp[i] >= 97 && temp[i] <= 122))){//Ӣ�ı������е����ֺ���ĸ�Ŀ�����
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
		//���ĵ�����
		auto iter_v = words.cbegin();
		auto iter_c = confs.cbegin();
		while (iter_v != words.cend()) {
			string temp = *iter_v;
			vector<string> temp_v;
			//���Ƚ��ַ���������Ӣ�Ĵ�ɢ
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
			//�ٶ�temp_v�е����ݽ��д���
			auto iter = temp_v.cbegin();
			while (iter != temp_v.cend()) {
				if((*iter).size() == 1)
					if ((*iter)[0] >= 48 && (*iter)[0] <= 57 && (*iter_c) >= 70) {//ֻ��������
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
						//Ҫ���ֱ���λ��GB2312������
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
	//Ѱ�������ַ����е�һ�γ��ֵĹ����Ӵ������û���ҵ����򷵻ؿ��ַ���
	//����������ַ�����������������ֽڽ��бȽ�
	//pos���ص��ǽ϶��ַ�����ƥ��λ��
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
			//�����жϵ�ǰ�ַ��ǲ���һ�������ַ�
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
			//�ҳ����ƥ���Ӵ�
			//���Ҫ���ƥ���Ӵ���������һ����ʱ������ÿһ��ѭ���в����滻��������һ�ҵ�������
			string::size_type i = isChin ? pos_short + 2 : pos_short;
			string::size_type j = isChin ? pos_long + 2 : pos_long;
			//�ӵ�ǰƥ���ַ���ʼ���ҵ�����ƥ���Ӵ�
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

		pos_long = 0;//��ǰ�ַ��ڳ����в����ڣ���һ���ַ����´ӳ�����ʼλ������
		if (isChin)
			pos_short += 2;
		else
			pos_short++;
	}
	return commonStr;
}

void assembleChars(const std::string &sentence1, const std::string &sentence2, std::vector<std::string> &v_sentences) {
	//�����������ʶ��ʽ��ÿһ���Ѿ�����Ԥ����֮�����Ч����
	string s1 = sentence1;
	string s2 = sentence2;
	vector<string> temp, result;
	result.push_back("");
	while (1) {
		string::size_type pos1 = string::npos, pos2 = string::npos;
		string commonStr = findFirstCommonStr(s1, s2, pos1, pos2);
		//�����������֧��һЩ��������Ĵ���
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

		//����ȥ����һ�����Ľ���������������ǲ������ȥ�أ���߿ռ临�ӶȽ����㷨ʵ�ֵ��Ѷ�
		//����Դ���ĵĽǶȽ�������Ҳ�ǱȽϺ��ʵ�

		if (commonStr == "") {
			//���ҽ���
			for (auto &s : result) {
				temp.push_back(s + s1 + s2);
				temp.push_back(s + s2 + s1);
				//���ȥ��
				temp.push_back(s + s1);
				temp.push_back(s + s2);
				temp.push_back(s);//����ȫ���Ǵ���ʶ�����������һ���Ŀ�����
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
				//ʵ���ϻ���һ��commonStrҲ�Ǵ���ʶ�����������ǿ����Ժܵ�
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
	//ɾ�����������е�ָ���ַ���
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
	//��ȡ���п��ܵ����֤����
	vector<vector<string>> v;
	vector<vector<unsigned>> v_c;
	if (!line_recognition_char(v, v_c, false, path, numOfRows))
		return false;
	for (int i = 1; i <= numOfRows; i++) {
		vector<string> temp_v;
		vector<unsigned> temp_c;
		split_sift(v[i - 1], v_c[i - 1], false, temp_v, temp_c);
		string line = "";
		//��ȡ���е�ʶ����
		//string���ַ���λ�ú�v���ַ���λ����һһ��Ӧ��
		for (auto &s : temp_v)
			line += s;
		regex id_regex("[0-9]{17}[0-9Xx]");
		smatch sm;
		while (1) {
			if (!regex_search(line, sm, id_regex))
				break;
			string::size_type pos = line.find(sm.str(0));
			//����ƽ�����Ÿ���
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