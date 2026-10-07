#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#define DEBUG
#include "classes.hpp"

enum class Types{
    ADD,
    SUB,
    DIV,
    MUL,
    SET,
    GET,
    WRITE,
    READ
};

struct Command{
    Types type;
    std::vector<Command*> arg;
    std::string ar;
};

bool hasNoOp(const std::string& e){
    int bracketLevel=0;
    for (int i=0;i<e.size();i++){
        if (e[i]=='(') bracketLevel++;
        if (e[i]==')') bracketLevel--;
        if (bracketLevel==0 && (
                e[i]=='+' ||
                e[i]=='-' ||
                e.substr(i,2)=="==" ||
                e.substr(i,2)=="!=" ||
                e.substr(i,2)=="&&" ||
                e.substr(i,2)=="||" ||
                e[i]=='>' ||
                e[i]=='<'))
            return false;
    }
    return true;
}

void deleteAllSPaces(std::string& s){
    std::string newOne;
    bool quoted=false;
    for (auto &i:s){
        if (i=='"') quoted=!quoted;
        if ((i!=' ' && i!='\t' && i!='\n' && i!='\0') || quoted) newOne+=i;
    }
    s=newOne;
}

bool OnlyNum(const std::string& e){
    for (int i=0;i<e.size();i++){
        if (!isdigit(e[i])) return false;
    }
    return true;
}

bool OnlyName(const std::string& e){
    for (int i=0;i<e.size();i++){
        if (!isalpha(e[i])) return false;
    }
    return true;
}

bool isOnlyOneLayerOfBrackets(const std::string& e){
    if (e[0]!='(') return false;
    for (int j=1;j<e.size();j++){
        char i=e[j];
        if (i=='(') return false;
        if (i==')' && j!=e.size()-1) return false;
    }
    return true;
}

Pointer<BasicObj> basicParse(std::string expression, Namespace& context){
    deleteAllSPaces(expression);
    LOG("Expression is "+expression);
    if (expression.empty()) return MakePtr<BasicObj>(new IntObj(0));
    if (OnlyNum(expression)){
        return MakePtr<BasicObj>(new IntObj(std::stoi(expression)));
    }
    if (OnlyName(expression)){
        return context[expression];
    }
    if (expression[0]=='"'){
        return MakePtr<BasicObj>(new StringObject(expression.substr(1, 
            expression.size()-2)));
    }
}

class VirtualMachine{
    public:
    Pointer<BasicObj> s;
    Pointer<BasicObj> exec(std::vector<Command*> comms, Namespace& context){
        for (auto i:comms){
            if (i->type==Types::ADD){
                auto arg = exec(i->arg, context);
                s = s->add(arg, false);
            }
            if (i->type==Types::SUB){
                auto arg = exec(i->arg, context);
                s = s->sub(arg, false);
            }
            if (i->type==Types::DIV){
                auto arg = exec(i->arg, context);
                s = s->div(arg, false);
            }
            if (i->type==Types::MUL){
                auto arg = exec(i->arg, context);
                s = s->mul(arg, false);
            }
            if (i->type==Types::WRITE){
                s=basicParse(i->ar, context);
            }
            if (i->type==Types::SET){
                context[exec({i->arg}, context)->str()]=s->clone();
            }
            if (i->type==Types::GET){
                return context[exec({i->arg}, context)->str()];
            }
        }
        return s->clone();
    }
};

std::vector<Command*> parse(std::string expression, Namespace& n){
    deleteAllSPaces(expression);
    if (OnlyNum(expression)){
        return {new Command{Types::WRITE, {NULL}, expression}};
    }
    if (OnlyName(expression)){
        return {new Command{Types::WRITE, {NULL}, expression}};
    }
    if (expression[0]=='"'){
        return {new Command{Types::WRITE, {NULL}, expression}};
    }
    if (isOnlyOneLayerOfBrackets(expression)){
        return parse(expression.substr(1, expression.size()-2), n);
    }
    if (hasNoOp(expression)){
        std::vector<Command*> res;
        std::string curr;
        char op='u';
        for (int i=0;i<expression.size();i++){
            curr+=expression[i];
            if (expression[i]=='*' || expression[i]=='/' || i==expression.size()-1){
                if (i!=expression.size()-1) curr.pop_back();
                if (op=='u'){
                    res.push_back(new Command{Types::WRITE, parse(curr, n)});
                }
                if (op=='*'){
                    res.push_back(new Command{Types::MUL, parse(curr, n)});
                }
                if (op=='/'){
                    res.push_back(new Command{Types::DIV, parse(curr, n)});
                }
                op=expression[i];
                curr.clear();
            }
        }
        return res;
    }
    std::vector<Command*> res;
    std::string curr;
    char op='u';
    for (int i=0;i<expression.size();i++){
        curr+=expression[i];
        if (expression[i]=='+' || expression[i]=='-' || i==expression.size()-1){
            if (i!=expression.size()-1) curr.pop_back();
            if (op=='u'){
                res.push_back(new Command{Types::WRITE, parse(curr, n)});
            }
            if (op=='+'){
                res.push_back(new Command{Types::ADD, parse(curr, n)});
            }
            if (op=='-'){
                res.push_back(new Command{Types::SUB, parse(curr, n)});
            }
            op=expression[i];
            curr.clear();
        }
    }
    return res;
}

int main(){
    Namespace n;
    auto r=parse("5+(5*3)", n);
    VirtualMachine m;
    m.exec(r, n);
    for (auto i:r){
        std::cout<<(int)i->type<<std::endl;
    }
    std::cout<<"Res is "<<m.s->str()<<std::endl;
}