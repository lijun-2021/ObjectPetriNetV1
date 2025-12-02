#include<iostream>
#include <chrono>
#include <ctime>
#include"Petrinet.h"
#include"read_json.h"
#include"Process.h"
using namespace std;

int main() {
	// 初始化Petri网信息
	Petrinet petrinet;
	read_place_json(petrinet);
	read_trans_json(petrinet);
	read_tokens_json(petrinet);
	// 开始时间
	char buffer[32];
	std::time_t time = std::time(nullptr);
	ctime_s(buffer, sizeof(buffer), &time);
	std::cout << "Begin Time: " << buffer << '\n';
	auto begin = chrono::steady_clock::now();
	// 算法搜索
	petrinet.dijskstra_search();
	// 结束时间
	ctime_s(buffer, sizeof(buffer), &time);
	std::cout << "End Time: " << buffer << '\n';;
	auto end = chrono::steady_clock::now();
    cout << "算法搜索总时间: " << chrono::duration_cast<chrono::milliseconds>(end - begin).count() << "ms" << endl;
	// 生成最优路径变迁序列
	petrinet.Createbestpath();
	return 0;
}