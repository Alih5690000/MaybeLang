#include <iostream>
#include <string>
#include "classes.hpp"

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

bool OnlyName(const std::string& e){
    for (auto i:e){
        if (!isalpha(i)) return false;
    }
    return true;
}

bool startsWithOnlyName(const std::string& e, std::string& remaining, std::string& beggining){
    for (int i=0;i<e.size();i++){
        if (e[i]=='(' || e[i]=='[' || e[i]=='.' || e[i]=='='){
            remaining = e.substr(i);
            beggining = e.substr(0, i);
            return true;
        }
        if (!isalpha(e[i])) return false;
    }
    return false;
}

bool isStringLiteral(const std::string& e){
    if (e[0]!='"') return false;
    for (int i=1;i<e.size();i++){
        if (e[i]=='"' && i!=e.size()-1) return false;
    }
    return true;
}

std::vector<std::string> splitBy(std::string s, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    int bracketLevel=0;
    int bracketLevel2=0;
    int bracketLevel3=0;
    for (char c : s) {
        if (c == '(') bracketLevel++;
        if (c == ')') bracketLevel--;
        if (c == '{') bracketLevel2++;
        if (c == '}') bracketLevel2--;
        if (c == '[') bracketLevel3++;
        if (c == ']') bracketLevel3--;
        if (c == delimiter) {
            if (!token.empty() && bracketLevel==0 && bracketLevel2==0 && bracketLevel3==0) {
                tokens.push_back(token);
                token.clear();
            }
        } else {
            token += c;
        }
    }
    if (!token.empty()) {
        tokens.push_back(token);
    }
    return tokens;
}

Pointer<BasicObj> parseExpression(const std::string& expression, Namespace& context) {
    if (OnlyNum(expression)){
        LOG("ONLYNUM");
        return MakePtr<BasicObj>(new IntObj(stoi(expression)));
    }
    if (OnlyName(expression)){
        return context[expression];
    }
    std::string remaining;
    std::string beggining;
    if (startsWithOnlyName(expression, remaining, beggining)){
        LOG("STARTS WITH NAME");
        std::string name = beggining;
        Pointer<BasicObj> obj = context[name];
        if (remaining[0] == '(') {
            std::string inbrackets;
            int bracketLevel=1;
            for (int i=1;i<remaining.size();i++){
                if (remaining[i]=='(') bracketLevel++;
                if (remaining[i]==')') bracketLevel--;
                if (bracketLevel==0){
                    inbrackets=remaining.substr(1,i-1);
                    remaining=remaining.substr(i+1);
                    break;
                }
            }
            std::vector<std::string> args = splitBy(inbrackets, ',');
            std::vector<Pointer<BasicObj>> parsedArgs;
            for (const auto& arg : args) {
                parsedArgs.push_back(parseExpression(arg, context));
            }
            return obj->call(parsedArgs, context);
        } else if (remaining[0] == '[') {
            // Handle indexing
            // Parse index and get the item
        } else if (remaining[0] == '.') {
            // Handle attribute access
            // Get the attribute from the object
        } else if (remaining[0] == '=') {
            context[name] = parseExpression(remaining.substr(1), context);
            return context[name];
        } else {
            throw ValueError("Unexpected character after variable name");
        }
    }
    bool noOp=hasNoOp(expression);
    LOG(std::string("NO OP IS ")+std::to_string(noOp));
    std::string curr;
    char op='u';
    int sum=0;
    int bracketLevel=0;
    int bracketLevel2=0;
    int bracketLevel3=0;
    if (isOnlyOneLayerOfBrackets(expression)){
        LOG("ONLY LAYER");
        std::string s=expression;
        s=s.substr(1);
        s.pop_back();
        LOG("s is "+s);
        return parseExpression(s, context);
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
        if (expression[i]=='{')
            bracketLevel2++;
        if (expression[i]=='}')
            bracketLevel2--;
        if (expression[i]=='[')
            bracketLevel3++;
        if (expression[i]==']')
            bracketLevel3--;
        if (bracketLevel==0 && bracketLevel2==0 && bracketLevel3==0 && expression.substr(i, 2)=="if"){
            i++;
            std::string condition;
            i++;
            if (expression[i]!='(') throw ValueError("Expected '(' after 'if'");
            i++;
            bracketLevel=1;
            while (bracketLevel>0 && i<expression.size()){
                if (expression[i]=='(') bracketLevel++;
                if (expression[i]==')') bracketLevel--;
                condition+=expression[i];
                i++;
            }
            if (bracketLevel!=0) throw ValueError("Mismatched parentheses in 'if' condition");
            std::string thenExpr;
            if (expression[i]!='{') throw ValueError("Expected '{' after 'if' condition");
            i++;
            bracketLevel2=1;
            while (bracketLevel2>0 && i<expression.size()){
                if (expression[i]=='{') bracketLevel2++;
                else if (expression[i]=='}') bracketLevel2--;
                else thenExpr+=expression[i];
                i++;
            }
            if (bracketLevel2!=0) throw ValueError("Mismatched braces in 'if' expression");
            std::string elseExpr;
            if (expression.substr(i, 4)=="else"){
                i+=4;
                if (expression[i]!='{') throw ValueError("Expected '{' after 'else'");
                i++;
                bracketLevel2=1;
                while (bracketLevel2>0 && i<expression.size()){
                    if (expression[i]=='{') bracketLevel2++;
                    else if (expression[i]=='}') bracketLevel2--;
                    else elseExpr+=expression[i];
                    i++;
                }
                if (bracketLevel2!=0) throw ValueError("Mismatched braces in 'else' expression");
            }
            Pointer<BasicObj> condResult = parseExpression(condition, context);
            if (condResult->asbool()) {
                return parseExpression(thenExpr, context);
            } else if (!elseExpr.empty()) {
                return parseExpression(elseExpr, context);
            } else {
                return MakePtr<BasicObj>(new IntObj(0)); // or some other default value
            }
        }
        
        curr+=expression[i];
        if (((bracketLevel==0 && bracketLevel2==0 && bracketLevel3==0 && (expression[i]=='+' || expression[i]=='-'))
         || (i==expression.size()-1)) && !noOp){
            LOG("OPERATOR DETECTED");
            if (expression[i]=='+' || expression[i]=='-')
                curr.pop_back();
            if (op=='u'){
                sum=parseExpression(curr, context)->asInt();
                LOG("FIRST NUM");
            }
            if (op=='+'){
                sum+=parseExpression(curr, context)->asInt();
                LOG("PLUS");
            }
            if (op=='-'){
                LOG("MINUS");
                sum-=parseExpression(curr, context)->asInt();
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
                sum=parseExpression(curr, context)->asInt();
                LOG("FIRST NUM");
            }
            if (op=='*'){
                sum*=parseExpression(curr, context)->asInt();
                LOG("MULTIPLY");
            }
            if (op=='/'){
                LOG("DIVIDE");
                sum/=parseExpression(curr, context)->asInt();
            }
            op=expression[i];
            curr.clear();
            continue;
        }
    }
    if (!curr.empty()){
        if (op=='u'){
            sum=parseExpression(curr, context)->asInt();
            LOG("FIRST NUM");
        }
        if (op=='+'){
            sum+=parseExpression(curr, context)->asInt();
            LOG("PLUS");
        }
        if (op=='-'){
            LOG("MINUS");
            sum-=parseExpression(curr, context)->asInt();
        }
        if (op=='*'){
            sum*=parseExpression(curr, context)->asInt();
            LOG("MULTIPLY");
        }
        if (op=='/'){
            LOG("DIVIDE");
            sum/=parseExpression(curr, context)->asInt();
        }
    }
    return MakePtr<BasicObj>(new IntObj(sum));
}

int main(){
    Namespace n;
    n["lol"]=MakePtr<BasicObj>(new IntObj(5));
    n["print"]=MakePtr<BasicObj>(new NativeFunctionObject([](std::vector<Pointer<BasicObj>> args){
        for (auto& arg : args) {
            std::cout << arg->str() << " ";
        }
        std::cout << std::endl;
        return MakePtr<BasicObj>(new IntObj(0));
    }));
    std::cout<<parseExpression("if(1==1){print(4)}", n)->str()<<std::endl;
}