/*
 * 2023-05-23 
 * author：leeczxc
*/
#include <variant>
#include <cstdint>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>
#include <string_view>
#include <optional>
#include <regex>
#include <charconv>
#include "print.h"

struct JSONobject;

struct JSONobject{
    std::variant
    < std:: nullptr_t
    , bool
    , int
    , double 
    , std::string
    , std::vector<JSONobject>
    , std::unordered_map<std::string, JSONobject>
    > inner;
    void do_print() const{
        print(inner);
    }
};
    

char unesped_char(char c){
    switch(c){
        case 'n' : return '\n';
        case 'r' : return '\r';
        case '0' : return '\0';
        case 't' : return '\t';
        case 'v' : return '\v';
        case 'f' : return '\f';
        case 'b' : return '\b';
        case 'a' : return '\a';
        default: return c;
    }
}
template<class T>
std::optional<T> try_parse_num(std::string_view str){
    T value;
    auto res = std::from_chars(str.data(), str.data() +str.size(), value);
    if(res.ec == std::errc() && res.ptr == str.data() + str.size()){
        return value;
    }
    return std::nullopt;
}
 
std::pair<JSONobject, size_t> parse(std::string_view json){
    // 空
    if(json.empty()){
        return {JSONobject{std::nullptr_t{}}, 0};
        // int && double
    }else if ('0' < json[0] && json[0] <= '9' || json[0] == '+' || json[0] == '-'){
        //0-9, ., e, -,+
        std::regex num_re{"[+-]?[0-9]+(\\.[0-9]*)?([eE][+-]?[0-9]+)?"};
        std::cmatch match;
        if(std::regex_search(json.data(), json.data() + json.size(), match, num_re)){
            std::string str = match.str();
            if(auto num = try_parse_num<int>(str); num.has_value()){
                return {JSONobject{*num}, str.size()};
            }
            if(auto num = try_parse_num<double>(str); num.has_value()){
                return {JSONobject{*num}, str.size()};
            }
        }
    }else if('"' == json[0]){
        // str 
        std::string str;
        enum{
            Raw,
            Escaped,
        } phase = Raw;
        size_t i;
        for(i = 1; i < json.size(); i++){
            char ch = json[i];
            if(phase == Raw){
                if(ch == '\\')
                    phase = Escaped;
                else if(ch == '"'){
                    i += 1;
                    break;
                }else{
                    str += ch;
                }
            }else if(phase == Escaped){
                str += unesped_char(ch);
                phase = Raw;
            }
        }
        return {JSONobject{std::move(str)}, i};
    }else if('[' == json[0]){
        // list
        std::vector<JSONobject> res;
        size_t i;
        for(i = 1; i < json.size();){
            if(json[i] == ']'){            
                i += 1;
                break;
            }
            auto [obj, eaten] = parse(json.substr(i));
            if(eaten == 0){
                i = 0;
                break;
            }
            res.push_back(std::move(obj));
            i += eaten;
            if(json[i] == ','){
                i += 1;
            }
        }              
        return {JSONobject{std::move(res)}, i};
    };
    return {JSONobject{std::nullptr_t{}}, 0};
}



int main(){
    std::string_view str3 = R"JSON([42,1,2,3,4])JSON";
    auto [obj, eaten] = parse(str3);
    std::cout << eaten << std::endl;
    // print(obj);
    return 0;
}
