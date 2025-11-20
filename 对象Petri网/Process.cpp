
#include<iostream>
#include <chrono>
#include"Process.h"
#include"read_json.h"
#include "Token.h"
using namespace std;

//vector<shared_ptr<ProcessPlace>> ProcessFlow::next_psble_marking(shared_ptr<ProcessPlace>CurrentMarking)
//{
//    vector<shared_ptr<ProcessTransition>> next_marking;
//    for (int i = 0; i < CurrentMarking->post_transitions.size(); ++i) {
//        next_marking.push_back(CurrentMarking->post_transitions[i]); // 对每个元素执行操作
//    }
//    return next_marking;
//}

//void ProcessFlow::update_marking(vector<pair<string, shared_ptr<Device>>> transition_device, vector<pair<string, shared_ptr<Human>>> transition_human) {
//    shared_ptr<ProcessPlace> current_marking;
//    vector<shared_ptr<ProcessPlace>>next_marking;
//    next_marking = next_psble_marking(current_marking);
//    vector<int> next_index;
//    for (int i = 0; i < next_marking.size(); ++i) {
//        if (next_marking[i]->devices(size) <= transition_device.size() & next_marking[i]->humans(size) <= transition_human.size()) {
//            next_index.push_back(i);
//        }
//    }
//    self.current_marking = next_marking[next_index[0]];
//
//}