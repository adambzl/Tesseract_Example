// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>



// TODO: 在此处引用程序需要的其他头文件
#include <baseapi.h>
#include <allheaders.h>
#include <iostream>
#include <string>
#include <regex>
#include <vector>

std::string UtfToGbk(const char* utf8);
bool line_recognition(std::vector<std::string> &v, std::vector<unsigned> &v_c, bool isChin, std::string &path, unsigned numOfRows);
bool char_recognition(std::vector<std::vector<std::string>> &v, std::vector<std::vector<unsigned>> &v_c, bool isChin, std::string path, unsigned numOfRows);
bool line_recognition_char(std::vector<std::vector<std::string>> &v, std::vector<std::vector<unsigned>> &v_c, bool isChin, std::string path, unsigned numOfRows);
void split_sift(std::vector<std::string> &words, std::vector<unsigned> &confs, bool isChin, std::vector<std::string> &v, std::vector<unsigned> &v_c);
void assembleChars(const std::string &sentence1, const std::string &sentence2, std::vector<std::string> &v_sentences);
std::string findFirstCommonStr(const std::string &s1, const std::string &s2, std::string::size_type &pos_s1, std::string::size_type &pos_s2);
void deleteStr(const std::vector<std::string> &v_in, const std::vector<unsigned> &c_in, std::vector<std::string> &v_out, std::vector<unsigned> &c_out, const std::string &str);
bool getID(std::string path, int numOfRows, std::vector<std::string> &ids, std::vector<unsigned> &ids_conf);