#include <iostream>
#include <string>
#include <algorithm>
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

void deleteAllSPaces(std::string& s){
    std::string newOne;
    bool quoted=false;
    for (auto &i:s){
        if (i=='"') quoted=!quoted;
        if (i!=' ' || quoted) newOne+=i;
    }
    s=newOne;
}

bool hasNoOp(const std::string& e){
    int bracketLevel=0;
    int bracketLevel2=0;
    int bracketLevel3=0;
    for (int i=0;i<e.size();i++){
        if (e[i]=='(') bracketLevel++;
        if (e[i]==')') bracketLevel--;
        if (e[i]=='{') bracketLevel2++;
        if (e[i]=='}') bracketLevel2--;
        if (e[i]=='[') bracketLevel3++;
        if (e[i]==']') bracketLevel3--;
        if (bracketLevel==0 && bracketLevel2==0 && bracketLevel3==0 && (
                e[i]=='+' ||
                e[i]=='-' ||
                e.substr(i,2)=="==" ||
                e.substr(i,2)=="!=" ||
                e.substr(i,2)==">=" ||
                e.substr(i,2)=="<=" ||
                e[i]=='>' ||
                e[i]=='<'))
            return false;
    }
    return true;
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

bool isStringLiteral(const std::string& e) {
    if (e.size() < 2 || e.front() != '"' || e.back() != '"')
        return false;

    for (size_t i = 1; i < e.size() - 1; i++) {
        if (e[i] == '"' && (i == 0 || e[i - 1] != '\\'))
            return false;
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

Pointer<BasicObj> parseExpression(const std::string& e, Namespace& context) {
    LOG("Parsing expression: " + e + "\n");
    std::string expression=e;
    if (expression.empty()) return MakePtr<BasicObj>(new IntObj(0));
    deleteAllSPaces(expression);
    if (expression[0]=='{'){
        //create dict
        //syntax: {key1:value1,key2:value2,...}
        if (expression.back()!='}') throw ValueError("Expected '}' at the end of dict expression");
        std::string inner=expression.substr(1,expression.size()-2);
        auto res=splitBy(inner,',');
        Pointer<BasicObj> dict=MakePtr<BasicObj>(new InstanceObject(context));
        for (auto &r:res){
            auto keyValue=splitBy(r,':');
            if (keyValue.size()!=2) throw ValueError("Expected key:value pair in dict expression");
            std::string key=keyValue[0];
            Pointer<BasicObj> value=parseExpression(keyValue[1],context);
            dict->setattr(key,value);
        }
        return dict;
    }
    if (expression.starts_with("func(")){
        std::string name;
        int i=5;
        while (i<expression.size() && expression[i]!=')') {
            name+=expression[i];
            i++;
        }
        if (i>=expression.size() || expression[i]!=')') throw ValueError("Expected ')' after function name");
        std::string insideBrackets;
        i+=2;
        int bracketLevel=1;
        while (bracketLevel>0 && i<expression.size()){
            if (expression[i]=='(') bracketLevel++;
            else if (expression[i]==')') {
                bracketLevel--;
                if (bracketLevel==0) break;
            }
            else insideBrackets+=expression[i];
            i++;
        }
        auto res=splitBy(insideBrackets,',');
        std::vector<std::string> params;
        for (auto &r:res){
            params.push_back(r);
        }
        i++;
        if (i>=expression.size() || expression[i]!='{') throw ValueError("Expected '{' after function parameters");
        std::string body;
        i++;
        int bracketLevel2=1;
        while (bracketLevel2>0 && i<expression.size()){
            if (expression[i]=='{') bracketLevel2++;
            else if (expression[i]=='}') {
                bracketLevel2--;
                if (bracketLevel2==0) break;
            }
            else body+=expression[i];
            i++;
        }
        Pointer<BasicObj> funcObj=MakePtr<BasicObj>(new FunctionObject(params,body));
        context[name]=funcObj;
        return funcObj;
    }
    if (expression.starts_with("if(")){
            int i=0;
            LOG("IF DETECTED");
            i++;
            std::string condition;
            i++;
            if (expression[i]!='(') throw ValueError("Expected '(' after 'if'");
            i++;
            int bracketLevel=1;
            while (bracketLevel>0 && i<expression.size()){
                if (expression[i]=='(') bracketLevel++;
                if (expression[i]==')'){ 
                    bracketLevel--;
                    if (bracketLevel==0) break;
                }
                condition+=expression[i];
                i++;
            }
            LOG("Condition is "+condition);
            if (bracketLevel!=0) throw ValueError("Mismatched parentheses in 'if' condition");
            std::string thenExpr;
            i++;
            if (expression[i]!='{') throw ValueError("Expected '{' after 'if' condition");
            i++;
            int bracketLevel2=1;
            while (bracketLevel2>0 && i<expression.size()){
                if (expression[i]=='{') bracketLevel2++;
                else if (expression[i]=='}') bracketLevel2--;
                else thenExpr+=expression[i];
                i++;
            }
            LOG("Then expression is "+thenExpr);
            if (bracketLevel2!=0) throw ValueError("Mismatched braces in 'if' expression");
            std::string elseExpr;
            if (expression.substr(i, 4)=="else"){
                i+=4;
                if (expression[i]!='{') throw ValueError("Expected '{' after 'else'");
                i++;
                int bracketLevel2=1;
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
    if (expression.starts_with("for(")){
        int i=3;
        if (expression[i]!='(') throw ValueError("Expected '(' after 'for'");
        i++;
        std::string initExpr;
        while (i<expression.size() && expression[i]!=';'){
            initExpr+=expression[i];
            i++;
        }
        if (i>=expression.size() || expression[i]!=';') throw ValueError("Expected ';' after 'for' initialization");
        i++;
        std::string conditionExpr;
        while (i<expression.size() && expression[i]!=';'){
            conditionExpr+=expression[i];
            i++;
        }
        if (i>=expression.size() || expression[i]!=';') throw ValueError("Expected ';' after 'for' condition");
        i++;
        std::string stepExpr;
        while (i<expression.size() && expression[i]!=')'){
            stepExpr+=expression[i];
            i++;
        }
        if (i>=expression.size() || expression[i]!=')') throw ValueError("Expected ')' after 'for' step");
        i++;
        if (expression[i]!='{') throw ValueError("Expected '{' after 'for' loop header");
        i++;
        std::string bodyExpr;
        int bracketLevel=1;
        while (bracketLevel>0 && i<expression.size()){
            if (expression[i]=='{') bracketLevel++;
            else if (expression[i]=='}') {
                bracketLevel--;
                if (bracketLevel==0) break;
            }
            else bodyExpr+=expression[i];
            i++;
        }
        if (bracketLevel!=0) throw ValueError("Mismatched braces in 'for' loop body");
        auto initResult = parseExpression(initExpr, context);
        auto condResult = parseExpression(conditionExpr, context);
        auto stepResult = parseExpression(stepExpr, context);
        while (condResult->asbool()) {
            parseExpression(bodyExpr, context);
            stepResult = parseExpression(stepExpr, context);
            condResult = parseExpression(conditionExpr, context);
        }
        return MakePtr<BasicObj>(new IntObj(0)); // or some other default value
    }
    if (OnlyNum(expression)){
        LOG("ONLYNUM");
        return MakePtr<BasicObj>(new IntObj(stoi(expression)));
    }
    if (OnlyName(expression)){
        return context[expression];
    }
    if (isStringLiteral(expression)) {
        std::string strValue = expression.substr(1, expression.size() - 2);
        return MakePtr<BasicObj>(new StringObject(strValue));
    }
    std::string remaining;
    std::string beggining;
    if (startsWithOnlyName(expression, remaining, beggining)){
        LOG("STARTS WITH NAME");
        std::string name = beggining;
        Pointer<BasicObj> obj;
        if (context.find(name) != context.end()) {
            obj = context[name];
            if (!remaining.empty() && remaining[0] == '=') {
                remaining = remaining.substr(1);
                Pointer<BasicObj> value = parseExpression(remaining, context);
                context[name] = value->clone();
                return value;
            }
        } else {
            if (!remaining.starts_with('='))
                throw ValueError(("Variable " + name + " not found in context").c_str());
            else{
                remaining=remaining.substr(1);
                Pointer<BasicObj> value = parseExpression(remaining, context);
                context[name] = value->clone();
                return value;
            }
        }
        char Parsing='u';
        for (int i=0;i<remaining.size();i++){
            if (remaining[i]=='.'){
                std::string attrName;
                i++;
                while (i<remaining.size() && isalpha(remaining[i])){
                    attrName+=remaining[i];
                    i++;
                }
                i--;
                obj=obj->getattr(attrName);
                Parsing='.';
            }
            else if (remaining[i]=='['){
                std::string indexExpr;
                i++;
                int bracketLevel=1;
                while (bracketLevel>0 && i<remaining.size()){
                    if (remaining[i]=='[') bracketLevel++;
                    else if (remaining[i]==']') {
                        bracketLevel--;
                        if (bracketLevel==0) break;
                    }
                    else indexExpr+=remaining[i];
                    i++;
                    Parsing='[';
                }
                Pointer<BasicObj> indexObj = parseExpression(indexExpr, context);
                obj=obj->getitem(indexObj);
            }
            else if (remaining[i]=='('){
                std::string argsExpr;
                i++;
                int bracketLevel=1;
                while (bracketLevel>0 && i<remaining.size()){
                    if (remaining[i]=='(') bracketLevel++;
                    else if (remaining[i]==')') {
                        bracketLevel--;
                        if (bracketLevel==0) break;
                    }
                    else argsExpr+=remaining[i];
                    i++;
                }
                auto argStrings=splitBy(argsExpr,',');
                std::vector<Pointer<BasicObj>> args;
                for (auto &argStr:argStrings){
                    args.push_back(parseExpression(argStr, context));
                }
                obj=obj->call(args, context);
                Parsing='(';
            }
            else if (remaining[i]=='='){
                if (Parsing=='.'){
                    std::string attrName;
                    int j=i-1;
                    while (j>=0 && isalpha(remaining[j])){
                        attrName=remaining[j]+attrName;
                        j--;
                    }
                    std::string valueExpr;
                    i++;
                    while (i<remaining.size()){
                        valueExpr+=remaining[i];
                        i++;
                    }
                    Pointer<BasicObj> valueObj = parseExpression(valueExpr, context)->clone();
                    obj->setattr(attrName, valueObj);
                }
                else if (Parsing=='['){
                    std::string indexExpr;
                    int j=i-1;
                    while (j>=0 && remaining[j]!=']'){
                        indexExpr=remaining[j]+indexExpr;
                        j--;
                    }
                    Pointer<BasicObj> indexObj = parseExpression(indexExpr, context);
                    std::string valueExpr;
                    i++;
                    while (i<remaining.size()){
                        valueExpr+=remaining[i];
                        i++;
                    }
                    Pointer<BasicObj> valueObj = parseExpression(valueExpr, context);
                    obj->setitem(indexObj, valueObj);
                }
                std::string valueExpr;
                i++;
                while (i<remaining.size()){
                    valueExpr+=remaining[i];
                    i++;
                }
                Pointer<BasicObj> valueObj = parseExpression(valueExpr, context);
                obj->setitem(valueObj, valueObj);
            }
            else{
                throw ValueError(("Unexpected character in expression: " + std::string(1, remaining[i])).c_str());
            }
        }
        return obj;
    }
    bool noOp=hasNoOp(expression);
    LOG(std::string("NO OP IS ")+std::to_string(noOp));
    std::string curr;
    std::string op="u";
    Pointer<BasicObj> sum=MakePtr<BasicObj>(new BasicObj);
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
        
        curr+=expression[i];
        if (((bracketLevel==0 && bracketLevel2==0 && bracketLevel3==0 && (expression[i]=='+' || expression[i]=='-'
        || expression.substr(i, 2)=="==" || expression.substr(i, 2)=="!=" || expression[i]=='>'
        || expression[i]=='<')
         || (i==expression.size()-1)) && !noOp)){
            LOG("OPERATOR DETECTED");
            if (expression[i]=='+' || expression[i]=='-' || expression.substr(i, 2)=="==" 
            || expression.substr(i, 2)=="!=" || expression[i]=='>' || expression[i]=='<')
                curr.pop_back();
            if (op=="u"){
                sum=parseExpression(curr, context);
                LOG("FIRST NUM");
            }
            if (op=="+"){
                sum=MakePtr<BasicObj>(new IntObj(sum->asInt() + parseExpression(curr, context)->asInt()));
                LOG("PLUS");
            }
            if (op=="-"){
                LOG("MINUS");
                sum=MakePtr<BasicObj>(new IntObj(sum->asInt() - parseExpression(curr, context)->asInt()));
            }
            if (op=="=="){
                sum=MakePtr<BasicObj>(new BoolObject(sum->equal(parseExpression(curr, context), false)));
            }
            if (op=="!="){
                sum=MakePtr<BasicObj>(new BoolObject(!sum->equal(parseExpression(curr, context), false)));
            }
            if (op==">"){
                sum=MakePtr<BasicObj>(new BoolObject(sum->greater(parseExpression(curr, context), false)));
            }
            if (op=="<"){
                sum=MakePtr<BasicObj>(new BoolObject(sum->less(parseExpression(curr, context), false)));
            }
            if (expression.substr(i, 2)=="==" || expression.substr(i, 2)=="!="){
                op=expression.substr(i, 2);
                i++;
            }
            else{
                op=expression[i];
            }
            LOG("Curr is "+curr);
            curr.clear();
            continue;
        }
        if (((bracketLevel==0 && (expression[i]=='/' || expression[i]=='*'))
        || (i==expression.size()-1)) && noOp){
            LOG("IDK DETECTED");
            if (expression[i]=='*' || expression[i]=='/')
                curr.pop_back();
            if (op=="u"){
                sum=parseExpression(curr, context);
                LOG("FIRST NUM");
            }
            if (op=="*" ){
                sum=MakePtr<BasicObj>(new IntObj(sum->asInt() * parseExpression(curr, context)->asInt()));
                LOG("MULTIPLY");
            }
            if (op=="/"){
                LOG("DIVIDE");
                sum=MakePtr<BasicObj>(new IntObj(sum->asInt() / parseExpression(curr, context)->asInt()));
            }
            op=expression[i];
            curr.clear();
            continue;
        }
    }
    if (!curr.empty()){
        if (op=="u"){
            sum=parseExpression(curr, context);
            LOG("FIRST NUM");
        }
        if (op=="+"){
            sum=MakePtr<BasicObj>(new IntObj(sum->asInt() + parseExpression(curr, context)->asInt()));
            LOG("PLUS");
        }
        if (op=="-"){
            LOG("MINUS");
            sum=MakePtr<BasicObj>(new IntObj(sum->asInt() - parseExpression(curr, context)->asInt()));
        }
        if (op=="*"){
            sum=MakePtr<BasicObj>(new IntObj(sum->asInt() * parseExpression(curr, context)->asInt()));
            LOG("MULTIPLY");
        }
        if (op=="/"){
            LOG("DIVIDE");
            sum=MakePtr<BasicObj>(new IntObj(sum->asInt() / parseExpression(curr, context)->asInt()));
        }
    }
    return sum;
}

void doCode(const std::string& code, Namespace& context) {
    auto res=splitBy(code,';');
    for (auto &r:res){
        parseExpression(r,context);
    }
}

int main() {
    std::cout << "A\n";

    Namespace n;

    std::cout << "B\n";

    n["lol"] = MakePtr<BasicObj>(new IntObj(5));

    std::cout << "C\n";

    n["print"] = MakePtr<BasicObj>(
        new NativeFunctionObject([](std::vector<Pointer<BasicObj>> args) -> Pointer<BasicObj> {
            std::cout << "INSIDE PRINT\n";

            for (auto& arg : args)
                std::cout << arg->str() << " ";

            std::cout << std::endl;

            return MakePtr<BasicObj>(new IntObj(0));
        })
    );
    n["input"] = MakePtr<BasicObj>(
        new NativeFunctionObject([](std::vector<Pointer<BasicObj>> args){
            std::string input;
            std::getline(std::cin, input);
            return MakePtr<BasicObj>(new StringObject(input));
        })
    );

    std::cout << "D\n";

    std::cout << "D1\n";
    auto code=R"a(
    proto={
        init:func()(x,y){a=dummy();a.x=x;a.y=y;return(a);},
    };
    a=proto.init(5,10);
    print(a.x);s
    print(a.y);
    )a";
    doCode("a={a:func()(x,y){a=0;a.x=x;a.y=y}}", n); //if(1==1){print(\"lol\")}
    std::cout << "D2\n";

    std::cout << "E\n";

    return 0;
}