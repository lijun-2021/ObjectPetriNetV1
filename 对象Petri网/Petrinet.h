#pragma once
#include"Place.h"
#include"Transition.h"
#include"Token.h"
#include<set>
#include<queue>
/***************************************Node**********************************************************/
class Node {
public:
	multimap<string, shared_ptr<Token>>marking;//节点标识，unorderdmap(哈希)，map（红黑树）
	vector<pair<shared_ptr<Node>, string>>fathernode_action;//父节点及其激发变迁，类似数组
	vector<pair<shared_ptr<Node>, string>>son_action;//子节点及其激发变迁
	bool is_best;//最优路径标记
	int cost;//消耗时间
	int id;//节点id
	double h;//启发式函数预测值

	vector<int>get_waitingtime() {
		vector<int>waitingtime;
		for (auto it = marking.begin(); it != marking.end(); ++it) {
			if (it->second->token_attribute != "control") {
				waitingtime.emplace_back(it->second->waiting_time);
			}
			
		}
		return waitingtime;
	}
};
/***************************************Petrinet**********************************************************/
class Petrinet {
public:
	unordered_map<string, shared_ptr<Place>>places;//保存的是这个petri网里所有库所的对象，格式为{"库所id": 库所对象指针}
	unordered_map<string, shared_ptr<Transition>>transitions;//保存的是这个petri网里所有变迁的对象，格式为{ "变迁id": 变迁对象指针 }
	multimap<string, shared_ptr<Token>>m0;//初始状态{"inplace":托肯指针}
	vector<tuple<string, string, int>>target_m;//目标状态{"inplace","attritube"}
	multimap<string, shared_ptr<Token>>new_marking;
	shared_ptr<Node>best_node;//最优目标点
	shared_ptr<Node>expand_node = make_shared<Node>();//拓展节点的过渡容器
	string Fire_tran;//激发变迁存储
	int expand_num = 0;//拓展次数

	deque<tuple<multimap<string, shared_ptr<Token>>, string,int>>bestpath;//最优路径(状态和激发变迁、g值)，双端队列

	vector<string>search_firable_transition(multimap<string, shared_ptr<Token>>m);//寻找可激发变迁  F1
	set<string>Get_possible_firable_trans(multimap<string, shared_ptr<Token>> m);//存储有token的库所其后置变迁可能是可激发变迁，并用中间变量（possible_firable_trans）存储 F1-1
	bool judge_possible_firable_trans(multimap<string, shared_ptr<Token>> m, string trans_name);//对possible_firable_trans里的每个变迁进行筛选(1.前置库所都要有弧上对应类型的token 2.后置库所需要满足容量要求) F1-2

	multimap<string, shared_ptr<Node>>node_list;//节点表

	struct CompareByCost {
		bool operator()(shared_ptr<Node>& p1, shared_ptr<Node>& p2) const {
			return (p1->cost) > (p2->cost);  // 按照升序排列
		}
	};
	priority_queue<shared_ptr<Node>, vector<shared_ptr<Node>>, CompareByCost>open_list;//未拓展节点表

	void list_initialization();//列表初始化 F3_1
	string createKey(multimap<string, shared_ptr<Token>>m);//创建该状态的key值 F3_1_1
	void fire_trans_get_newnode(shared_ptr<Node> expand_node_temp, shared_ptr<Node> new_node, string trans_name);//激发可激发变迁,获得新节点 F3_2
	void newnode_deal(shared_ptr<Node> new_node);//对于新节点新旧判断后的处理 F3_3
	int judage_new_node(shared_ptr<Node> new_node);//新旧节点判断 F3_3_1 (0:无重复状态的新节点或者是时间轴判断无法确定的节点 1:时间轴判断完全重复节点（需要记住节点表内的“优秀节点”，后续需要存边）2:时间轴判断确定的全新节点（需要记住节点表内的“坏节点”，后续需要删除）)

	/***************************************dijkstra_search**********************************************************/
	void dijskstra_search();//dijkstra搜索算法 F4
	bool is_dijkstra_continue();//dijistra是否继续搜索 F4_1
	deque<tuple<multimap<string, shared_ptr<Token>>, string,int>>Createbestpath();
};
