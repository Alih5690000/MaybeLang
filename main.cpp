#include <iostream>
#include <string>
#include "classes.hpp"
#define LOG(x) std::cout<<x<<std::endl

bool isOnlyOneLayerOfBrackets(const std::string& e){
    if (e[0]!='(') return false;
    for (int j=1;j<e.size();j++){
        char i=e[j];
        if (i=='(') return false;
        if (i==')' && j!=e.size()-1) return false;
    }
    return true;
}

bool hasNoOp(const std::string& e){
    int bracketLevel=0;
    for (auto i:e){
        if (i=='(') bracketLevel++;
        if (i==')') bracketLevel--;
        if ((i=='+' || i=='-') && bracketLevel==0) return false;
    }
    return true;
}

bool OnlyNum(const std::string& e){
    for (auto i:e){
        if (!isdigit(i)) return false;
    }
    return true;
}

Pointer<BasicObj> parseExpression(const std::string& expression) {
    if (OnlyNum(expression)){
        LOG("ONLYNUM");
        return MakePtr<BasicObj>(new IntObj(stoi(expression)));
    }
    bool noOp=hasNoOp(expression);
    LOG(std::string("NO OP IS ")+std::to_string(noOp));
    std::string curr;
    char op='u';
    int sum=0;
    int bracketLevel=0;
    if (isOnlyOneLayerOfBrackets(expression)){
        LOG("ONLY LAYER");
        std::string s=expression;
        s=s.substr(1);
        s.pop_back();
        LOG("s is "+s);
        return parseExpression(s);
    }
    for (int i=0;i<expression.size();i++){
        LOG(std::string("NUM IS ") + expression[i]);
        if (!isdigit(expression[i])){
            LOG("IS NOT ONLYNUM");
        }
        if (expression[i]=='(')
            bracketLevel++;
        if (expression[i]==')')
            bracketLevel--;
        curr+=expression[i];
        if (((bracketLevel==0 && (expression[i]=='+' || expression[i]=='-'))
         || (i==expression.size()-1)) && !noOp){
            LOG("OPERATOR DETECTED");
            if (expression[i]=='+' || expression[i]=='-')
                curr.pop_back();
            if (op=='u'){
                sum=parseExpression(curr)->asInt();
                LOG("FIRST NUM");
            }
            if (op=='+'){
                sum+=parseExpression(curr)->asInt();
                LOG("PLUS");
            }
            if (op=='-'){
                LOG("MINUS");
                sum-=parseExpression(curr)->asInt();
            }
            op=expression[i];
            LOG("Curr is "+curr);
            curr.clear();
            continue;
        }
        if (((bracketLevel==0 && (expression[i]=='/' || expression[i]=='*'))
        || (i==expression.size()-1)) && noOp){
            LOG("IDK DETECTED");
            if (expression[i]=='*' || expression[i]=='/')
                curr.pop_back();
            if (op=='u'){
                sum=parseExpression(curr)->asInt();
                LOG("FIRST NUM");
            }
            if (op=='*'){
                sum*=parseExpression(curr)->asInt();
                LOG("MULTIPLY");
            }
            if (op=='/'){
                LOG("DIVIDE");
                sum/=parseExpression(curr)->asInt();
            }
            op=expression[i];
            curr.clear();
            continue;
        }
    }
    if (!curr.empty()){
        if (op=='u'){
            sum=parseExpression(curr)->asInt();
            LOG("FIRST NUM");
        }
        if (op=='+'){
            sum+=parseExpression(curr)->asInt();
            LOG("PLUS");
        }
        if (op=='-'){
            LOG("MINUS");
            sum-=parseExpression(curr)->asInt();
        }
        if (op=='*'){
            sum*=parseExpression(curr)->asInt();
            LOG("MULTIPLY");
        }
        if (op=='/'){
            LOG("DIVIDE");
            sum/=parseExpression(curr)->asInt();
        }
    }
    return MakePtr<BasicObj>(new IntObj(sum));
}

int main(){
    std::cout<<parseExpression("5*(5+2)")->str()<<std::endl;
}