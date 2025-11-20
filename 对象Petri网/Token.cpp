#include "Token.h"
using namespace std;

inline int extractNumberFromString(const string token_type, const string token_name) {  //token_name 表示托肯的名称，例如 AGV1；token_type表示token类型，例如 AGV。
    size_t typeStartPos = token_name.find(token_type); //size_t 整型数据类型，64位无符号整型,token_name.find(token_type)返回token_type在token_name中第一次出现的位置索引
    size_t NumStartPos = typeStartPos + token_type.length();//计算数字部分的起始位置
    string numberStr; //用于存储提取的数字字符串
    for (size_t i = NumStartPos; i < token_name.size(); ++i) {
        if (isdigit(token_name[i])) {      //isdigit()函数用于判断字符是否为数字字符
            numberStr += token_name[i]; //将数字字符添加到数字字符串中
        }
        else {
            break;
        }
    }
    int number = stoi(numberStr); // 将提取的数字字符串转换为整数
    return number;
}

map<string, int> Token::createToken(string& token_name) //创造托肯,生成m0,例如{（"agv",1）,（"agv",2）...}
{
    if (token_name.find("agv") != string::npos && token_name.find("workpiece") == string::npos && token_name.find("robot") == string::npos) {
        map<string, int>data_temp;
        data_temp.emplace("agv", extractNumberFromString("agv", token_name));
        token_attribute = "agv";
        return data_temp;
    }
    else if (token_name.find("control") != string::npos) { //"control"表示控制托肯，或黑点
        map<string, int>data_temp;
        data_temp.emplace("control", 1);
        return data_temp;
    }
    else if (token_name.find("agv") == string::npos && token_name.find("workpiece") != string::npos && token_name.find("robot") == string::npos) {
        map<string, int>data_temp;
        data_temp.emplace("workpiece", extractNumberFromString("workpiece", token_name));
        token_name = "workpiece";
        return data_temp;
    }
    else if (token_name.find("agv") == string::npos && token_name.find("workpiece") == string::npos && token_name.find("robot") != string::npos) {
        map<string, int>data_temp;
        data_temp.emplace("robot", extractNumberFromString("robot", token_name));
        token_name = "robot";
        return data_temp;
    }
    else if ((token_name.find("agv") != string::npos && token_name.find("workpiece") != string::npos && token_name.find("robot") == string::npos)) {
        map<string, int>data_temp;
        data_temp.emplace("agv", extractNumberFromString("agv", token_name));
        data_temp.emplace("workpiece", extractNumberFromString("workpiece", token_name));
        token_name = "agv_workpiece";
        return data_temp;
    }
    
}

map<string, int> Token::inheritToken(map<string, int> last_state, const string& token_name)//继承前托肯状态的属性,标识更新
{
    map<string, int>state;
    if (token_name.find("agv") != string::npos && token_name.find("workpiece") == string::npos) {
        state.emplace("agv", last_state["agv"]);//
    }
    else if (token_name.find("agv") == string::npos && token_name.find("workpiece") != string::npos) {
        state.emplace("workpiece", last_state["workpiece"]);

    }
    else if (token_name.find("agv") != string::npos && token_name.find("workpiece") != string::npos) {
        state.emplace("agv", last_state["agv"]);
        state.emplace("workpiece", last_state["workpiece"]);

    }
    else if (token_name.find("robot") != string::npos) {
        state.emplace("robot", last_state["robot"]);
    }
    else if (token_name.find("control") != string::npos) {
        state.emplace("control", 1);
    }
    else {
        state.emplace("agv", last_state["agv"]);
        state.emplace("workpiece", last_state["workpiece"]);
        state.emplace("robot", last_state["robot"]);
    }

    return state;
}

