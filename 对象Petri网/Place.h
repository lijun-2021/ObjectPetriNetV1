#pragma once
#include<iostream>
#include<vector>
#include<string>
#include<map>
#include<utility>
using namespace std;
class Place {
public:
	string place_name;//库所的名字
	int delay;//库所的时延
	int capacity;//库所容量
	vector<string>pre_arcs;//保存库所和它的前置变迁之间的对应关系
	vector<string>post_arcs;//保存库所和它的后置变迁之间的对应关系
};