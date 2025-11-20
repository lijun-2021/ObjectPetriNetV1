#include "Petrinet.h"

/*********************************************寻找可激发变迁********************************************/
vector<string> Petrinet::search_firable_transition(multimap<string, shared_ptr<Token>> m)
{
    vector<string>firable_trans;
    set<string>possible_firable_trans;
    //存储有token的库所其后置变迁可能是可激发变迁，并用中间变量（possible_firable_trans）存储
    possible_firable_trans = Get_possible_firable_trans(m);
    //对possible_firable_trans里的每个变迁进行筛选(1.前置库所都要有弧上对应类型的token 2.后置库所需要满足容量要求)
    for (auto it = possible_firable_trans.begin(); it != possible_firable_trans.end(); ++it) {
        if (judge_possible_firable_trans(m, it->c_str())) {
            firable_trans.emplace_back(it->c_str());
        }
    }
    return firable_trans;
}
/****function::Get_possible_firable_trans****/
set<string> Petrinet::Get_possible_firable_trans(multimap<string, shared_ptr<Token>> m) {
    set<string>possible_firable_trans;
    for (auto itr = m.begin(); itr != m.end(); ++itr) {
        auto& place_name = itr->first;
        for (auto post_trans : places[place_name]->post_arcs) {
            possible_firable_trans.emplace(post_trans);
        }
    }
    return possible_firable_trans;
}
/****function::judge_possible_firable_trans****/
bool Petrinet::judge_possible_firable_trans(multimap<string, shared_ptr<Token>> m, string trans_name)
{
    //判断改变迁的前置库所是否满足条件
    //1.前置库所中都应该有托肯
    int a = 0;
    for (string pre_place_name : transitions[trans_name]->pre_places) {
        if (m.find(pre_place_name) == m.end()) {
            return false;
        }
    }
    //2.前置库所中的托肯应该满足弧上定义的规则
    for (string pre_place_name : transitions[trans_name]->pre_places) {
        bool place_lock = false;
        for (int i = 0; i < transitions[trans_name]->fire_rule[pre_place_name].size(); ++i) {
            for (auto it = m.equal_range(pre_place_name).first; it != m.equal_range(pre_place_name).second; ++it) {
                if (it->second->token_attribute == transitions[trans_name]->fire_rule[pre_place_name][i]) {
                    place_lock = true;
                    break;
                }
                else if (it->second->token_attribute != transitions[trans_name]->fire_rule[pre_place_name][i] && i == transitions[trans_name]->fire_rule[pre_place_name].size() - 1) {
                    return false;
                }
            }
            if (place_lock == true) { break; }
        }
    }
    //3.后置库所满足容量要求
    for (string post_place_name : transitions[trans_name]->post_places) {
        if (places[post_place_name]->capacity == 1 && m.find(post_place_name) != m.end()) {
            return false;
        }
    }
    return true;
}

/****函数：列表初始化****/
void Petrinet::list_initialization()
{
    shared_ptr<Node>node_initialization = make_shared<Node>();
    node_initialization->marking = m0;
    node_initialization->cost = 0;
    node_list.emplace(createKey(m0), node_initialization);
    open_list.emplace(node_initialization);
}
/****函数：创建状态key值***/
string Petrinet::createKey(multimap<string, shared_ptr<Token>> m)
{
    string key = "";
    for (auto itr1 = m.begin(); itr1 != m.end(); ++itr1) {
        if (itr1->second->token_attribute != "control") {
            key = key + itr1->first;
            for (auto itr2 = itr1->second->state.begin(); itr2 != itr1->second->state.end(); ++itr2) {
                key = key + itr2->first + to_string(itr2->second);
            }
        }
    }
    return key;
}
/****函数：激发可激发变迁,获得新节点***/
void Petrinet::fire_trans_get_newnode(shared_ptr<Node>expand_node_temp, shared_ptr<Node> new_node, string trans_name)
{
    //消去前置库所中的托肯时需要存储，激发后的托肯需要继承的信息
    map<string, int>state_temp;//库所状态{托肯类型：数量}
    int arc_num = -1;//记住激发类型的编号，前置后置需一致
    int lamda = 0;//激发该变迁后得到未变化的托肯发生等待时间的变化
    //消去该变迁前置库所中托肯
    for (string pre_place_name : transitions[trans_name]->pre_places) {
        auto& fire_rule = transitions[trans_name]->fire_rule;
        for (int i = 0; i < fire_rule[pre_place_name].size(); i++) {
            //
            if (expand_node_temp->marking.equal_range(pre_place_name).first->second->token_attribute == fire_rule[pre_place_name][i]) {
                arc_num = i;
            }
            else if (arc_num >= 0) { break; }
        }
        auto cut_token_ptr = expand_node_temp->marking.equal_range(pre_place_name).first;
        if (lamda < places[pre_place_name]->delay - cut_token_ptr->second->waiting_time) {
            lamda = places[pre_place_name]->delay - cut_token_ptr->second->waiting_time;
        }
        state_temp.insert(cut_token_ptr->second->state.begin(), cut_token_ptr->second->state.end());
        expand_node_temp->marking.erase(cut_token_ptr);
    }
    //m中未变化托肯的waiting_time变化 lamda计算
    for (auto itr = expand_node_temp->marking.begin(); itr != expand_node_temp->marking.end(); itr++) {
        itr->second->waiting_time = itr->second->waiting_time + lamda;
        if (itr->second->waiting_time > places[itr->second->inplace]->delay) {
            itr->second->waiting_time = places[itr->second->inplace]->delay;
        }
    }
    //后置库所添加托肯
    for (string post_place_name : transitions[trans_name]->post_places) {
        auto new_token = make_shared<Token>();
        new_token->inplace = post_place_name;
        new_token->waiting_time = 0;
        new_token->token_attribute = transitions[trans_name]->fire_rule[post_place_name][arc_num];
        //继承
        new_token->state = new_token->inheritToken(state_temp, new_token->token_attribute);
        expand_node_temp->marking.emplace(post_place_name, new_token);
        new_node->marking = expand_node_temp->marking;
    }
    new_node->cost = expand_node_temp->cost + lamda;
    pair<shared_ptr<Node>, string>father_node_action_temp;
    father_node_action_temp.first = expand_node;
    father_node_action_temp.second = trans_name;
    new_node->fathernode_action.emplace_back(father_node_action_temp);
}
/****函数：对于新节点新旧判断后的处理***/
void Petrinet::newnode_deal(shared_ptr<Node> new_node)
{
    bool judge_new_node_result = judage_new_node(new_node);
    if (judge_new_node_result == 0) {
        expand_node->son_action.emplace_back(new_node, Fire_tran);
        node_list.emplace(createKey(new_node->marking), new_node);
        open_list.emplace(new_node);
    }
    else if (judge_new_node_result == 2) {
        expand_node->son_action.emplace_back(new_node, Fire_tran);
        node_list.emplace(createKey(new_node->marking), new_node);
        open_list.emplace(new_node);
    }
}
/****函数：新旧节点判断(0:无重复状态的新节点或者是时间轴判断无法确定的节点 1:时间轴判断为完全重复的节点（需要记住节点表内的“优秀节点”，后续需要存边）2:时间轴判断确定的全新节点（需要记住节点表内的“坏节点”，后续需要删除）3:无用节点)***/
int Petrinet::judage_new_node(shared_ptr<Node> new_node)
{
    string node_key = createKey(new_node->marking);
    if (node_list.count(node_key) == 0) {
        return 0;
    }
    else {
        //相同marking的node
        vector<int>new_node_v = new_node->get_waitingtime();
        auto range = node_list.equal_range(node_key);
        for (auto it = range.first; it != range.second; ++it) {
            //按照map自带顺序处理，可能存在不严谨的地方
            vector<int>old_node_v = it->second->get_waitingtime();
            int big_time = 0;
            int equal_time = 0;
            for (int i = 0; i < new_node_v.size(); ++i) {
                if (new_node_v[i] - new_node->cost + it->second->cost > old_node_v[i]) {
                    big_time++;
                }
                else if (new_node_v[i] - new_node->cost + it->second->cost == old_node_v[i]) {
                    equal_time++;
                }
            }
            if (equal_time == new_node_v.size() && new_node->cost == it->second->cost) {
                pair<shared_ptr<Node>, string> new_fathernode_action;
                new_fathernode_action.first = expand_node;
                new_fathernode_action.second = Fire_tran;
                it->second->fathernode_action.emplace_back(new_fathernode_action);
                return 1;
            }
            else {
                if (big_time == new_node_v.size()) {
                    node_list.erase(it);
                    return 2;
                }
                else if (big_time == 0) {
                    return 3;
                }
            }
        }
    }
    return 0;
}
/***************************************dijkstra_search**********************************************************/
void Petrinet::dijskstra_search()
{
    list_initialization();
    while (is_dijkstra_continue()) {
        expand_node = open_list.top();
        open_list.pop();
        expand_node->id = expand_num;
        //cout << expand_node->cost << endl;
        auto expand_node_firable_trans = search_firable_transition(expand_node->marking);
        while (!expand_node_firable_trans.empty()) {
            expand_num++;
            //指针相等只是将地址指向同一个地址
            shared_ptr<Node> expand_node_temp = make_shared<Node>();
            expand_node_temp->cost = expand_node->cost;
            for (auto itr = expand_node->marking.begin(); itr != expand_node->marking.end(); ++itr) {
                shared_ptr<Token>token_temp = make_shared<Token>();
                token_temp->waiting_time = itr->second->waiting_time;
                token_temp->inplace = itr->second->inplace;
                token_temp->token_attribute = itr->second->token_attribute;
                token_temp->state = itr->second->state;
                expand_node_temp->marking.emplace(token_temp->inplace, token_temp);
            }
            expand_node_temp->fathernode_action = expand_node->fathernode_action;
            shared_ptr<Node>new_node = make_shared<Node>();//新拓展出来的节点的过渡容器
            Fire_tran = expand_node_firable_trans[0];
            auto fire_tran = expand_node_firable_trans[0];
            expand_node_firable_trans.erase(expand_node_firable_trans.begin());
            fire_trans_get_newnode(expand_node_temp, new_node, fire_tran);
            newnode_deal(new_node);
        }
    }
}
/****函数：dijistra是否继续搜索***/
bool Petrinet::is_dijkstra_continue()
{
    auto end_node = open_list.top();
    for (auto target_node : target_m) {
        string place_name;
        string token_attr;
        int num = 0;
        tie(place_name, token_attr, num) = target_node;
        if (end_node->marking.count(place_name) == 0) {
            return true;
        }
        else {
            auto range = end_node->marking.equal_range(place_name);
            int eauqal_time = 0;
            for (auto itr1 = range.first; itr1 != range.second; ++itr1) {
                if (itr1->second->state[token_attr] == num) {
                    eauqal_time++;
                }
            }
            if (eauqal_time == 0) {
                return true;
            }
        }
    }
    best_node = end_node;
    best_node->id = expand_num + 1;
    best_node->is_best = true;
    return false;
}

deque<tuple<multimap<string, shared_ptr<Token>>, string,int>> Petrinet::Createbestpath()
{
    deque<tuple<multimap<string, shared_ptr<Token>>, string,int>>path_temp;
    bool continue_flag_back = true;
    auto best_nextnode = make_shared<Node>();
    string best_tran;
    auto best_node_temp = best_node;
    while (continue_flag_back) {
        tuple<multimap<string, shared_ptr<Token>>, string,int>state_temp1;//状态、变迁、g值
        for (auto& father_node : best_node_temp->fathernode_action) {
            if (father_node.first->marking == m0 && father_node.first->cost == 0) {
                continue_flag_back = false;
                break;
            }
        }
        int best_cost = 10000;
        for (auto& father_node : best_node_temp->fathernode_action) {
            if (father_node.first->cost < best_cost) {
                best_cost = father_node.first->cost;
                best_nextnode = father_node.first;
                best_tran = father_node.second;
            }
        }

        best_node_temp = best_nextnode;
        state_temp1 = make_tuple(best_node_temp->marking, best_tran,best_cost);
        path_temp.emplace_front(state_temp1);
        best_nextnode->is_best = true;
    }
    return path_temp;
}
