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
        LOG(std::string("NUM IS ") + expression[i]);
        if (!isdigit(expression[i])){
            onlyNum=false;
            LOG("IS NOT ONLYNUM");
        }
        if (expression[i]=='(')
            bracketLevel++;
        if (expression[i]==')')
            bracketLevel--;
        curr+=expression[i];
        if ((bracketLevel==0 && (expression[i]=='+' || expression[i]=='-'))
         || (i==expression.size()-1 && !onlyNum)){
            LOG("OPERATOR DETECTED");
            if (expression[i]=='+' || expression[i]=='-')
                curr.pop_back();
            if (op=='u'){
                sum=parseExpression(curr);
                LOG("FIRST NUM");
            }
            if (op=='+'){
                sum+=parseExpression(curr);
                LOG("PLUS");
            }
            if (op=='-'){
                LOG("MINUS");
                sum-=parseExpression(curr);
            }
            op=expression[i];
            curr.clear();
            continue;
        }
    }
    if (onlyNum){
        LOG("ONLYNUM");
        return stoi(expression);
    }
    return sum;
}

int main(){
    std::cout<<parseExpression("5+5")<<std::endl;
}