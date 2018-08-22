#include "stdafx.h"

using std::endl;
using std::cout;
using std::cerr;
using std::string;
using std::to_string;
using std::regex;
using std::smatch;
using std::regex_search;
using std::vector;



int main(int argc, char *argv[]) {
	string path(argv[1]);
	unsigned numOfRows = atoi(argv[2]);
	vector<vector<string>> temp_results, results_lines, results_chars;
	vector<vector<unsigned>> temp_confs, confs_lines, confs_chars;
	///*char_recognition(temp_results, temp_confs, true, path, numOfRows);
	//for (int i = 0; i < numOfRows; i++) {
	//	vector<string> v;
	//	vector<unsigned> v_c;
	//	split_sift(temp_results[i], temp_confs[i], true, v, v_c);
	//	results_chars.push_back(v);
	//	confs_chars.push_back(v_c);
	//}*/

	temp_results.clear();
	temp_confs.clear();
	line_recognition_char(temp_results, temp_confs, true, path, numOfRows);
	for (int i = 0; i < numOfRows; i++) {
		vector<string> v;
		vector<unsigned> v_c;
		split_sift(temp_results[i], temp_confs[i], true, v, v_c);
		results_lines.push_back(v);
		confs_lines.push_back(v_c);
	}
	vector<string> ids;
	vector<unsigned> ids_conf;
	if (getID(path, numOfRows, ids, ids_conf)) {
		cout << "所有可能的身份证号码信息如下：" << endl;
		for (auto &id : ids)
			cout << id << endl;
	}
	else
		cout << "没能识别到任何身份证号码信息" << endl;

	cout << endl;


	//输出切分识别结果
	/*cout << "将字符拆分后的识别结果如下" << endl;
	for (auto &v : results_chars) {
		for (auto &s : v)
			cout << s;
		cout << endl;
	}*/

	/*cout << "按行识别的结果如下" << endl;*/
	for (auto &v : results_lines) {
		for (auto &s : v)
			cout << s;
		cout << endl;
	}
	
	//如果按行识别首尾出现 卧 字是常见的情况，那么应该考虑删除这个字，需要后续验证
	return 0;
}

//vector<vector<string>> finalResults;
////合并并输出所有可能的结果
//for (int i = 0; i < numOfRows; i++) {
//	auto &result_lines = results_lines[i];
//	auto &result_chars = results_chars[i];
//	auto &conf_lines = confs_lines[i];
//	auto &conf_chars = confs_chars[i];
//	//删除中英文问号
//	vector<string> v_lines, temp_lines, v_chars, temp_chars, v_sentences;
//	vector<unsigned> c_lines, temp_c_lines, c_chars, temp_c_chars;
//	string sentence1, sentence2;
//	deleteStr(result_lines, conf_lines, temp_lines, temp_c_lines, "?");
//	deleteStr(result_chars, conf_chars, temp_chars, temp_c_chars, "?");
//	deleteStr(temp_lines, temp_c_lines, v_lines, c_lines, "？");
//	deleteStr(temp_chars, temp_c_chars, v_chars, c_chars, "？");//此函数可以优化，删除一个字符串序列
//	//拼接成句子
//	for (auto &s : v_lines)
//		sentence1 += s;
//	for (auto &s : v_chars)
//		sentence2 += s;
//	//合并所有可能结果
//	assembleChars(sentence1, sentence2, v_sentences);
//	finalResults.push_back(v_sentences);
//	//cout << "第" << i << "行的所有可能结果为:" << endl;
//	//for (auto s : v_sentences)
//	//	cout << s << endl;
//	cout << endl;
//}