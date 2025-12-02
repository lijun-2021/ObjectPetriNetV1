#include "Petrinet.h"

/*********************************************寻找可激发变迁********************************************/
vector<string> Petrinet::search_firable_transition(const multimap<string, shared_ptr<Token>>& m)
{
    vector<string> firable_trans;
    // 先获取候选变迁集合（去重）
    auto possible_firable_trans = Get_possible_firable_trans(m);

    // 对候选变迁筛选（前置库所、属性、后置容量约束）
    for (const auto& trans_name : possible_firable_trans) {
        if (judge_possible_firable_trans(m, trans_name)) {
            firable_trans.emplace_back(trans_name);
        }
    }
    return firable_trans;
}

/****function::Get_possible_firable_trans****/
set<string> Petrinet::Get_possible_firable_trans(const multimap<string, shared_ptr<Token>>& m) 
{
    set<string> possible_firable_trans;
    // 由于multimap是有序的，我们可以利用键的相邻性
    for (auto itr = m.begin(); itr != m.end(); ) {
        const auto& place_name = itr->first;

        // 安全检查：确保库所存在
        auto place_iter = places.find(place_name);
        if (place_iter != places.end()) {
            // 添加该库所的所有后继变迁
            for (const auto& post_trans : place_iter->second->post_arcs) {
                possible_firable_trans.emplace(post_trans);
            }
        }

        // 跳过相同库所的其他token
        itr = m.upper_bound(place_name);
    }
    return possible_firable_trans;
}

/****function::judge_possible_firable_trans****/
bool Petrinet::judge_possible_firable_trans(const multimap<string, shared_ptr<Token>>& m, string trans_name)
{
    // 安全检查
    auto t_it = transitions.find(trans_name);
    if (t_it == transitions.end()) return false;
    auto& trans = t_it->second;

    // 1. 前置库所都应该有token
    for (string pre_place_name : trans->pre_places) {
        if (m.find(pre_place_name) == m.end()) {
            return false;
        }
    }

    // 2. 修正的属性匹配逻辑
    for (string pre_place_name : trans->pre_places) {
        auto fr_it = trans->fire_rule.find(pre_place_name);
        if (fr_it == trans->fire_rule.end()) {
            return false; // 没有定义触发规则
        }

        // 检查该库所是否有token满足任意规则属性
        bool found_match = false;
        auto m_range = m.equal_range(pre_place_name);

        for (auto it = m_range.first; it != m_range.second && !found_match; ++it) {
            for (const auto& rule_attr : fr_it->second) {
                if (it->second->token_attribute == rule_attr) {
                    found_match = true;
                    break;
                }
            }
        }

        if (!found_match) {
            return false;
        }
    }

    // 3. 后置库所容量检查（添加安全检查）
    for (string post_place_name : trans->post_places) {
        auto p_it = places.find(post_place_name);
        if (p_it != places.end() && p_it->second->capacity == 1) {
            if (m.find(post_place_name) != m.end()) {
                return false;
            }
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
        auto m_range = expand_node_temp->marking.equal_range(pre_place_name);
        for (int i = 0; i < fire_rule[pre_place_name].size(); i++) {
            //
            if (m_range.first->second->token_attribute == fire_rule[pre_place_name][i]) {
                arc_num = i;
            }
            else if (arc_num >= 0) { break; }
        }
        auto cut_token_ptr = m_range.first;
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
    auto judge_new_node_result = judge_new_node(new_node);
    if (judge_new_node_result == Petrinet::new_node || judge_new_node_result == Petrinet::better_node) {
        expand_node->son_action.emplace_back(new_node, Fire_tran);
        node_list.emplace(createKey(new_node->marking), new_node);
        open_list.emplace(new_node);
    }
}
/****函数：新旧节点判断(0:无重复状态的新节点或者是时间轴判断无法确定的节点 1:时间轴判断为完全重复的节点（需要记住节点表内的“优秀节点”，后续需要存边）2:时间轴判断确定的全新节点（需要记住节点表内的“坏节点”，后续需要删除）3:无用节点)***/
 Petrinet::node_type_judge Petrinet::judge_new_node(shared_ptr<Node> new_node)
{
    string node_key = createKey(new_node->marking);
    // 若没有相同 key，则为全新状态
    auto range = node_list.equal_range(node_key);
    if (range.first == range.second) {
        return node_type_judge::new_node;
    }
    
    //相同marking的node // 只计算一次新节点的等待时间向量
    vector<int>new_node_v = new_node->get_waitingtime();
    for (auto it = range.first; it != range.second; ++it) {
        //按照map自带顺序处理，可能存在不严谨的地方
        vector<int>old_node_v = it->second->get_waitingtime();

        // delta = old.cost - new.cost，
        // 后续用 new_node_v[i] + delta 做比较，减少重复计算
        int delta = it->second->cost - new_node->cost;
        bool any_greater = false;   // 是否存在 adjusted > old
        bool all_greater = true;    // 是否对所有位置 adjusted > old
        int equal_count = 0;        // adjusted == old 的计数

        for (size_t i = 0; i < new_node_v.size(); ++i) {
            int adjusted = new_node_v[i] + delta;

            if (adjusted > old_node_v[i]) {
                any_greater = true;
            }
            else {
                all_greater = false;
            }
            if (adjusted == old_node_v[i]) {
                ++equal_count;
            }
        }

        // 等价且cost相同 -> 视为重复节点，记录父节点动作并返回 1
        if (equal_count == new_node_v.size() && new_node->cost == it->second->cost) {
            pair<shared_ptr<Node>, string> new_fathernode_action;
            new_fathernode_action.first = expand_node;
            new_fathernode_action.second = Fire_tran;
            it->second->fathernode_action.emplace_back(new_fathernode_action);
            return node_type_judge::duplicate_node;
        }
        // 如果对所有位置 adjusted > old -> 新节点在时间轴上更好，删除旧节点并返回 2
        if (all_greater) {
            node_list.erase(it);
            return node_type_judge::better_node;
        }

        // 如果没有任何 adjusted > old（即 big_time == 0） -> 新节点无用，返回 3
        if (!any_greater) {
            return node_type_judge::useless_node;
        }
    }
    return node_type_judge::new_node;
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
        // 取得当前节点的可激发变迁列表（返回的是 vector<string>）
        auto expand_node_firable_trans = search_firable_transition(expand_node->marking);
        while (!expand_node_firable_trans.empty()) {
            expand_num++;
            //指针相等只是将地址指向同一个地址
            shared_ptr<Node> expand_node_temp = make_shared<Node>();
            expand_node_temp->cost = expand_node->cost;
            for (auto itr = expand_node->marking.begin(); itr != expand_node->marking.end(); ++itr) {
                shared_ptr<Token> token_temp = make_shared<Token>(*(itr->second));
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
    // 若没有待扩展节点，搜索结束
    if (open_list.empty()) {
        return false;
    }

    auto top_node = open_list.top();

    // 检查top_node是否满足所有目标约束；只要有一条目标未满足就继续搜索
    for (auto target_node : target_m) {
        string place_name;
        string token_attr;
        int num = 0;
        tie(place_name, token_attr, num) = target_node;

        auto range = top_node->marking.equal_range(place_name);
        // 如果该库所不存在任何 token，则目标未满足
        if (range.first == range.second) {
            return true;
        }
        // 检查该库所中是否存在满足数量要求的token（state[token_attr] == num）
        bool matched = false;
        for (auto it = range.first; it != range.second; ++it) {
            auto& state = it->second->state;
            auto found = state.find(token_attr);
            if (found != state.end() && found->second == num) {
                matched = true;
                break;
            }
        }
        if (!matched) {
            return true; // 对于某个目标不满足，继续搜索
        }
    }
    // 所有目标都满足：记录并结束搜索
    best_node = top_node;
    best_node->id = expand_num + 1;
    best_node->is_best = true;
    return false;
}

void Petrinet::Createbestpath()
{
    deque<tuple<multimap<string, shared_ptr<Token>>, string, int>> path_temp;

    // 保护性检查
    if (!best_node) {
        return;
    }

    auto current = best_node;

    // 向前回溯直到到达初始标识（m0 且 cost == 0）或无法继续为止
    while (true) {
        // 没有父节点，结束回溯
        if (current->fathernode_action.empty() || (current->cost == 0 && current->marking == m0)) {
            break;
        }

        // 在 current 的父节点中选择 cost 最小的父节点
        shared_ptr<Node> chosen_parent;
        string chosen_tran;
        int chosen_cost = 10000; //std::numeric_limits<int>::max(); 
        for (const auto& p : current->fathernode_action) {
            const auto& parent = p.first;
            if (!parent) { continue; }
            if (parent->cost < chosen_cost) {
                chosen_cost = parent->cost;
                chosen_parent = parent;
                chosen_tran = p.second;
            }
        }

        // 若未找到有效父节点，结束回溯（防止无限循环）
        if (!chosen_parent) {
            break;
        }

        // 把选中的父节点状态与对应变迁、g 值放到路径前端
        path_temp.emplace_front(chosen_parent->marking, chosen_tran, chosen_cost);
        chosen_parent->is_best = true;

        // 继续向上回溯
        current = chosen_parent;
    }

    bestpath = path_temp;
    std::cout << "\n最小完工时间: g_min = " << best_node->cost << "s" << "\n";
    std::cout << "\n激发变迁序列: ";
    while (!path_temp.empty()) {
        string strTrans;
        tie(ignore, strTrans, ignore) = path_temp.front();
        std::cout << strTrans << (path_temp.size() != 1 ? " -> " : "\n");
        path_temp.pop_front();
    }
}