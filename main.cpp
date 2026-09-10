#include <iostream>
#include <string>
#define LOG(x) std::cout<<x<<std::endl

int parseExpression(const std::string& expression) {
    bool onlyNum=true;
    std::string curr;
    char op='u';
    int sum=0;
    int bracketLevel=0;
    for (int i=0;i<expression.size();i++){
        if (!isdigit(expression[i])){
            onlyNum=false;
            LOG("IS NOT ONLYNUM");
        }
        if (expression[i]=='(')
            bracketLevel++;
        if (expression[i]==')')
            bracketLevel--;
        if (expression[i]=='+' || expression[i]=='-' || i==expression.size()){
            if (op=='u'){
                sum=parseExpression(curr);
                LOG("FIRST NUM");
            }
            if (op=='+'){
                sum+=parseExpression(curr);
            }
            if (op=='-'){
                sum-=parseExpression(curr);
            }
            op=expression[i];
        }
        curr+=expression[i];
    }
    if (onlyNum){
        return stoi(expression);
    }
    return sum;
}

int main(){
    std::cout<<parseExpression("5+5")<<std::endl;
}