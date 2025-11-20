#pragma once
#include"Place.h"
#include"Transition.h"
#include<iostream>
#include<string>
#include<memory>
#include<map>
using namespace std;


class Token {
public:
	string inplace;//所在库所
	int waiting_time;//Token在该库所内的已等待时间
	string token_attribute;//托肯类型（agv\workpiece\robot\agv_workpiece\action_in(agv+workpiece+robot)\action_out(agv+workpiece+robot)）
	map<string, int>state;//库所状态{托肯类型：数量}

	map<string, int>createToken(string& token_attribute);//创造托肯状态,生成m0
	map<string, int>inheritToken(map<string, int>last_state, const string& token_attribute);//继承前托肯状态的属性,标识更新
};

