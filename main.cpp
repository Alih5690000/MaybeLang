#include <iostream>
#include <string>
#define LOG(x) std::cout<<x<<std::endl

bool isOnlyOneLayerOfBrackets(const std::string& e){
    if (e[0]!='(') return false;
    for (auto i:e){
        if (i=='(') return false;
    }
    if (e.back()!=')'){
        throw std::runtime_error("Unclosed bracket");
    }
    return true;
}

bool hasNoOp(const std::string& e){
    for (auto i:e){
        if (i=='+' || i=='-') return false;
    }
    return true;
}

int parseExpression(const std::string& expression) {
    bool onlyNum=true;
    bool noOp=hasNoOp(expression);
    LOG(std::string("NO OP IS ")+std::to_string(noOp));
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
        if (((bracketLevel==0 && (expression[i]=='+' || expression[i]=='-'))
         || (i==expression.size()-1 && !onlyNum)) && !noOp){
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
        if (((bracketLevel==0 && (expression[i]=='/' || expression[i]=='*'))
        || (i==expression.size()-1 && !onlyNum)) && noOp){
            LOG("IDK DETECTED");
            if (expression[i]=='*' || expression[i]=='/')
                curr.pop_back();
            if (op=='u'){
                sum=parseExpression(curr);
                LOG("FIRST NUM");
            }
            if (op=='*'){
                sum*=parseExpression(curr);
                LOG("MULTIPLY");
            }
            if (op=='/'){
                LOG("DIVIDE");
                sum/=parseExpression(curr);
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
    std::cout<<parseExpression("5*5+5")<<std::endl;
}