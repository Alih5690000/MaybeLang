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
    READ,
    WRITE2,
    READ2,
    GETATTR,
    SETATTR,
    GETITEM,
    SETITEM,
    CALL,
    FUNC_START,
    FUNC_END,
    PUSH_STACK,
    POP_STACK,
    EXCHANGE,
    JMP_IF,
    POINT
};

struct Command{
    Types type;
    std::vector<Command*> arg;
    std::string ar;
    std::vector<std::vector<Command*>> args;
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
        if (c == delimiter && bracketLevel==0
            && bracketLevel2==0 && bracketLevel3==0) {
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

Pointer<BasicObj> CreateFunctionObjectt
    (const std::vector<std::string>& p, std::vector<Command*> b);

class VirtualMachine{
    public:
    std::vector<Pointer<BasicObj>> stack;
    Pointer<BasicObj> s;
    Pointer<BasicObj> s2;
    std::vector<Command*> funcBody;
    std::vector<std::string> params;
    bool isFuncBody;
    int layer;
    std::string jmp;
    Pointer<BasicObj> exec(std::vector<Command*> comms, Namespace& context){
        s=MakePtr<BasicObj>(new EmptyObject);
        for (auto i:comms){
            if (i->type==Types::JMP_IF){
                if (exec(i->arg, context)->asbool())
                    jmp=i->ar;
            }
            if (!jmp.empty()){
                if (i->type==Types::POINT && i->ar==jmp){
                    jmp.clear();
                }
                continue;
            }
            if (i->type==Types::PUSH_STACK){
                stack.push_back(s);
            }
            if (i->type==Types::POP_STACK){
                s=stack.back();
                stack.pop_back();
            }
            if (i->type==Types::EXCHANGE){
                std::swap(s, stack.back());
            }
            if (i->type==Types::FUNC_START){
                layer++;
                isFuncBody=true;
                auto a=splitBy(i->ar, ' ');
                params=a;
            }
            if (i->type==Types::FUNC_END){
                layer--;
                if (layer==0){
                    isFuncBody=false;
                    s=MakePtr<BasicObj>(new FunctionObjectt(
                        params, funcBody));
                    params.clear();
                    funcBody.clear();
                }
            }
            if (isFuncBody){
                funcBody.push_back(new Command(*i));
            }
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
                if (i->arg[0]==NULL)
                    s=basicParse(i->ar, context);
                else
                    s=exec(i->arg, context);
            }
            if (i->type==Types::WRITE2){
                if (i->arg[0]==NULL)
                    s2=basicParse(i->ar, context);
                else
                    s2=exec(i->arg, context);
            }
            if (i->type==Types::GETATTR){
                auto e=s->getattr(i->ar);
                s=e;
            }
            if (i->type==Types::SETITEM){
                s->setattr(i->ar, exec(i->arg, context));
            }
            if (i->type==Types::GETITEM){
                auto e=s->getitem(exec(i->arg, context));
                s=e;
            }
            if (i->type==Types::CALL){
                std::vector<Pointer<BasicObj>> args;
                for (auto j:i->args){
                    args.push_back(exec(j, context));
                }
                auto r=s->call(args, context);
                s=r;
            }
            if (i->type==Types::SETATTR){
                s->setattr(i->ar, exec(i->arg, context));
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

class FunctionObjectt:public BasicObj{
  public:
    std::vector<std::string> params;
    std::vector<Command*> body;
    FunctionObjectt(const std::vector<std::string>& p, std::vector<Command*> b):params(p),body(b){
      for (auto i:params){
        LOG("Param "+i);
      }
    };

    Pointer<BasicObj> call(std::vector<Pointer<BasicObj>> args,Namespace& context) override{
      
      if (args.size()!=params.size()) THROW(ValueError, "Incorrect number of arguments");
      Namespace localContext=context;
      for (size_t i=0;i<params.size();i++){
        localContext[params[i]]=args[i];
        LOG("Setting "+params[i]+" to "+args[i]->str());
      }
      VirtualMachine vm;
      try{
        vm.exec(body, localContext);
      }
      catch(ReturnSig& s){
        return s.sig;
      }
      return MakePtr<BasicObj>(new IntObj(0));
    }

    Pointer<BasicObj> clone() override{
      return MakePtr<BasicObj>(new FunctionObjectt(params,body));
    }
};

Pointer<BasicObj> CreateFunctionObjectt
    (const std::vector<std::string>& p, std::vector<Command*> b){
    Pointer<BasicObj> f=MakePtr<BasicObj>(new FunctionObjectt(p, b));
    return f;
}

std::vector<Command*> parse(std::string expression, Namespace& n){
    deleteAllSPaces(expression);
    if (OnlyNum(expression)){
        return {new Command{Types::WRITE, {NULL}, expression}};
    }
    if (OnlyName(expression)){
        return {new Command{Types::WRITE, {NULL}, expression}};
    }
    if (expression[0]=='{'){
        auto arr=splitBy(expression.substr(1, expression.size()-2), ',');
        std::vector<Command*> res;
        for (auto i:arr){
            auto a=splitBy(i, ':');
            res.push_back(new Command{Types::SETATTR, {parse(a[0], n)}, a[0]});
        }
        return res;
    }
    std::string rem, beg;
    if (startsWithOnlyName(expression, rem, beg)){
        std::vector<Command*> res;
        res.push_back(new Command{Types::WRITE, parse(beg, n)});
        char op=rem[0];
        rem=rem.substr(1);
        std::string curr;
        for (int i=0;i<rem.size();i++){
            curr+=rem[i];
            if (rem[i]=='.' || rem[i]=='[' || rem[i]=='(' || 
                i==rem.size()-1){
                if (i!=rem.size()-1) rem.pop_back();
                if (rem[i]=='.'){
                    i++;
                    while (i<rem.size() 
                        && isalpha(rem[i]) || rem[i]=='_'){
                        curr+=rem[i];
                        i++;
                    }
                    i++;
                    if (i!=rem.size()-1 && rem[i]=='='){
                        i++;
                        std::string rvalue;
                        while (rem[i]!=';' && i<rem.size()){
                            rvalue+=rem[i];
                            i++;
                        }
                        res.push_back(new Command{Types::SETATTR, parse(rvalue, n)});
                    }
                    else
                        res.push_back(new Command{Types::GETATTR, {NULL}, curr});
                    curr.clear();
                }
                if (rem[i]=='('){
                    i++;
                    std::string inside;
                    int bracketLevel=1;
                    while (true){
                        if (rem[i]=='(') bracketLevel++;
                        if (rem[i]==')') {
                            bracketLevel--;
                            if (bracketLevel==0) break;
                        }
                        inside+=rem[i];
                    }
                    std::vector<std::vector<Command*>> aa;
                    auto f=splitBy(inside, ',');
                    for (auto k:f){
                        aa.push_back(parse(k, n));
                    }
                    res.push_back(new Command{Types::CALL, {NULL}, "", aa});
                }
                if (rem[i]=='['){
                    i++;
                    std::string inside;
                    int bracketLevel=1;
                    while (true){
                        if (rem[i]=='[') bracketLevel++;
                        if (rem[i]==']') {
                            bracketLevel--;
                            if (bracketLevel==0) break;
                        }
                        inside+=rem[i];
                    }
                    i++;
                    if (i!=rem.size()-1 && rem[i]=='='){
                        i++;
                        std::string rvalue;
                        while (rem[i]!=';' && i<rem.size()){
                            rvalue+=rem[i];
                            i++;
                        }
                        res.push_back(new Command{Types::SETITEM, parse(rvalue, n)});
                    }
                    else
                        res.push_back(new Command{Types::GETITEM, parse(inside, n)});
                }
            }
        }
        return res;
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
    n["lol"]=MakePtr<BasicObj>(new InstanceObject(n));
    n["lol"]->setattr("a", MakePtr<BasicObj>(new IntObj(67)));
    auto r=parse("lol.a", n);
    VirtualMachine m;
    m.exec(r, n);
    for (auto i:r){
        std::cout<<(int)i->type<<std::endl;
    }
    std::cout<<"Res is "<<m.s->getattr("a")->str()<<std::endl;
}