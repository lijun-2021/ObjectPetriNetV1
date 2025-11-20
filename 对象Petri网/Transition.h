#pragma once
#include<iostream>
#include<vector>
#include<string>
#include<map>
#include<unordered_map>
#include<utility>
using namespace std;
class Transition {
public:
    string trans_name;//变迁名称
    vector<string>pre_places;//前置库所集合
    vector<string>post_places;//后置库所集合
    unordered_map<string, vector<string>>fire_rule;//激发规则（后置库所名：对应弧的集合）
};