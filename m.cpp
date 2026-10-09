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
    LESS,
    GREATER,
    EQUAL,
    NOT,
    AND,
    OR,
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
    JMP_IF_NOT,
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

bool hasAssignment(const std::string& e){
    int bracketLevel=0;
    int bracketLevel2=0;
    int bracketLevel3=0;
    bool quoted=false;
    for (int i=0;i<e.size();i++){
        if (e[i]=='"' && (i==0 || e[i-1]!='\\'))
            quoted=!quoted;
        if (quoted) continue;
        if (e[i]=='(') bracketLevel++;
        if (e[i]==')') bracketLevel--;
        if (e[i]=='{') bracketLevel2++;
        if (e[i]=='}') bracketLevel2--;
        if (e[i]=='[') bracketLevel3++;
        if (e[i]==']') bracketLevel3--;
        if (bracketLevel==0 && bracketLevel2==0 && bracketLevel3==0
            && e[i]=='='
            && (i==0 || e[i-1]!='=')
            && (i+1>=e.size() || e[i+1]!='='))
            return true;
    }
    return false;
}

Pointer<BasicObj> basicParse(std::string expression, Namespace& context){
    deleteAllSPaces(expression);
    LOG("Expression is "+expression);
    if (expression.empty()) return MakePtr<BasicObj>(new IntObj(0));
    if (expression=="{}") return MakePtr<BasicObj>(new EmptyObject);
    if (expression=="[]") return MakePtr<BasicObj>(new ArrayObject());
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
    Pointer<BasicObj> exec(std::vector<Command*> comms, Namespace& context){
        std::vector<Pointer<BasicObj>> stack;
        Pointer<BasicObj> s;
        Pointer<BasicObj> s2;
        std::vector<Command*> funcBody;
        std::vector<std::string> params;
        bool isFuncBody=false;
        int layer=0;
        std::string jmp;
        bool back=false;
        s=MakePtr<BasicObj>(new EmptyObject);
        for (int j=0;j<comms.size();j++){
            auto i=comms[j];
            if (isFuncBody){
                if (i->type==Types::FUNC_START){
                    layer++;
                    funcBody.push_back(new Command(*i));
                } else if (i->type==Types::FUNC_END){
                    layer--;
                    if (layer==0){
                        isFuncBody=false;
                        s=CreateFunctionObjectt(params, funcBody);
                        params.clear();
                        funcBody.clear();
                    } else {
                        funcBody.push_back(new Command(*i));
                    }
                } else {
                    funcBody.push_back(new Command(*i));
                }
                continue;
            }
            if (i->type==Types::FUNC_START){
                layer=1;
                isFuncBody=true;
                params=splitBy(i->ar, ' ');
                funcBody.clear();
                continue;
            }
            if (i->type==Types::FUNC_END){
                THROW(ValueError, "Unexpected function end");
            }
            if (i->type==Types::JMP_IF){
                LOG("JMP_IF DETECTED");
                auto ll=exec(i->arg, context);
                LOG("JMP_IF COND IS "+ll->str());
                if (!ll->asbool()){
                    auto it=std::find_if(comms.begin(), comms.end(),
                        [&](const auto& x) {
                            if (x->type==Types::POINT && x->ar==i->ar){
                                return true;
                            }
                            return false;
                        }
                    );
                    j+=it-comms.begin();
                }
            }
            if (i->type==Types::JMP_IF_NOT){
                LOG("JMP_IF_NOT DETECTED");
                auto ll=exec(i->arg, context);
                LOG("JMP_IF_NOT COND IS "+ll->str());
                LOG("BEFORE J IS "+std::to_string(j));
                if (ll->asbool()){
                    auto it=std::find_if(comms.begin(), comms.end(),
                        [&](const auto& x) {
                            if (x->type==Types::POINT && x->ar==i->ar){
                                return true;
                            }
                            return false;
                        }
                    );
                    j=it-comms.begin()-1;
                    LOG("J IS "+std::to_string(j));
                }
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
            if (i->type==Types::LESS){
                auto arg = exec(i->arg, context);
                s = MakePtr<BasicObj>(new BoolObject(s->less(arg, false)));
            }
            if (i->type==Types::GREATER){
                auto arg = exec(i->arg, context);
                s = MakePtr<BasicObj>(new BoolObject(s->greater(arg, false)));
            }
            if (i->type==Types::EQUAL){
                auto arg = exec(i->arg, context);
                s = MakePtr<BasicObj>(new BoolObject(s->equal(arg, false)));
            }
            if (i->type==Types::NOT){
                s = MakePtr<BasicObj>(new BoolObject(!s->asbool()));
            }
            if (i->type==Types::AND){
                auto arg = exec(i->arg, context);
                s = MakePtr<BasicObj>(new BoolObject(s->asbool() && arg->asbool()));
            }
            if (i->type==Types::OR){
                auto arg = exec(i->arg, context);
                s = MakePtr<BasicObj>(new BoolObject(s->asbool() || arg->asbool()));
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
                s->setitem(MakePtr<BasicObj>(new IntObj(std::stoi(i->ar))), exec(i->arg, context));
            }
            if (i->type==Types::GETITEM){
                auto e=s->getitem(exec(i->arg, context));
                s=e;
            }
            if (i->type==Types::CALL){
                LOG("GOT CALL");
                auto callable=s;
                std::vector<Pointer<BasicObj>> args;
                VirtualMachine ma;
                for (auto j:i->args){
                    args.push_back(ma.exec(j, context));
                }
                auto r=callable->call(args, context);
                s=r;
            }
            if (i->type==Types::SETATTR){
                s->setattr(i->ar, exec(i->arg, context));
            }
            if (i->type==Types::SET){
                context[i->ar]=s->clone();
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

static long long jmps=0;

std::vector<Command*> parse(std::string expression, Namespace& n){
    deleteAllSPaces(expression);
    LOG("PARSING "+expression);
    if (expression.back()==';') expression.pop_back();
    if (expression.starts_with("func(")){
        LOG("GOT FUNC");
        std::string name;
        int i=5;
        while (i<expression.size() && expression[i]!=')') {
            name+=expression[i];
            i++;
        }
        if (i>=expression.size() || expression[i]!=')') THROW(ValueError, "Expected ')' after function name");
        std::string insideBrackets;
        i+=2;
        int bracketLevel=1;
        while (bracketLevel>0 && i<expression.size()){
            if (expression[i]=='(') bracketLevel++;
            else if (expression[i]==')') {
                bracketLevel--;
                if (bracketLevel==0) break;
            }
            if (expression[i]!=',')
                insideBrackets+=expression[i];
            else
                insideBrackets+=' ';
            i++;
        }
        auto res=splitBy(insideBrackets,',');
        std::vector<std::string> params;

        for (auto &r:res){
            params.push_back(r);
        }
        i++;
        if (i>=expression.size() || expression[i]!='{') THROW(ValueError, "Expected '{' after function parameters");
        std::string body;
        i++;
        int bracketLevel2=1;
        while (bracketLevel2>0 && i<expression.size()){
            if (expression[i]=='{') bracketLevel2++;
            else if (expression[i]=='}') {
                bracketLevel2--;
                if (bracketLevel2==0) break;
            }
            body+=expression[i];
            i++;
        }
        std::vector<Command*> re;
        re.push_back(new Command{Types::FUNC_START, {NULL}, insideBrackets});
        VirtualMachine mm;
        auto f=splitBy(body, ';');
        for (auto i:f){
            auto k=parse(i, n);
            for (auto j:k){
                re.push_back(j);
            }
        }
        re.push_back(new Command{Types::FUNC_END});
        re.push_back(new Command{Types::SET, {NULL}, name});
        return re;
    }
    if (expression.starts_with("for(")){
        int i=3;
        if (expression[i]!='(') THROW(ValueError, "Expected '(' after 'for'");
        i++;
        std::string initExpr;
        while (i<expression.size() && expression[i]!=';'){
            initExpr+=expression[i];
            i++;
        }
        if (i>=expression.size() || expression[i]!=';') THROW(ValueError, "Expected ';' after 'for' initialization");
        i++;
        std::string conditionExpr;
        while (i<expression.size() && expression[i]!=';'){
            conditionExpr+=expression[i];
            i++;
        }
        if (i>=expression.size() || expression[i]!=';') THROW(ValueError, "Expected ';' after 'for' condition");
        i++;
        std::string stepExpr;
        while (i<expression.size() && expression[i]!=')'){
            stepExpr+=expression[i];
            i++;
        }
        if (i>=expression.size() || expression[i]!=')') THROW(ValueError, "Expected ')' after 'for' step");
        i++;
        if (expression[i]!='{') THROW(ValueError, "Expected '{' after 'for' loop header");
        i++;
        std::string bodyExpr;
        int bracketLevel=1;
        while (bracketLevel>0 && i<expression.size()){
            if (expression[i]=='{') bracketLevel++;
            else if (expression[i]=='}') {
                bracketLevel--;
                if (bracketLevel==0) break;
            }
            bodyExpr+=expression[i];
            i++;
        }
        if (bracketLevel!=0) THROW(ValueError, "Mismatched braces in 'for' loop body");
        LOG("For is "+initExpr+' '+conditionExpr+' '+stepExpr+' '+bodyExpr);
        auto initResult = parse(initExpr, n);
        auto condResult = parse(conditionExpr, n);
        auto bodyResult = splitBy(bodyExpr, ';');
        auto stepResult = parse(stepExpr, n);
        std::vector<Command*> res;
        int j=++jmps;
        for (auto i:initResult){
            res.push_back(i);
        }
        res.push_back(new Command{Types::POINT, {NULL}, std::to_string(j)});
        for (auto i:bodyResult){
            auto r=parse(i, n);
            for (auto j:r){
                res.push_back(j);
            }
        }
        for (auto i:stepResult){
            res.push_back(i);
        }
        res.push_back(new Command{Types::JMP_IF_NOT, condResult, std::to_string(j)});
        return res;
    }
    if (expression.starts_with("if(")){
        int i=0;
        LOG("IF DETECTED");
        i++;
        std::string condition;
        i++;
        if (expression[i]!='(') THROW(ValueError, "Expected '(' after 'if'");
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
        if (bracketLevel!=0) THROW(ValueError, "Mismatched parentheses in 'if' condition");
        std::string thenExpr;
        i++;
        if (expression[i]!='{') THROW(ValueError, "Expected '{' after 'if' condition");
        i++;
        int bracketLevel2=1;
        while (bracketLevel2>0 && i<expression.size()){
            if (expression[i]=='{') bracketLevel2++;
            else if (expression[i]=='}'){ 
                bracketLevel2--;
                if (bracketLevel2==0) break;
            }
            thenExpr+=expression[i];
            i++;
        }
        LOG("Then expression is "+thenExpr);
        if (bracketLevel2!=0) THROW(ValueError, "Mismatched braces in 'if' expression");
        std::string elseExpr;
        if (expression.substr(i, 4)=="else"){
            i+=4;
            if (expression[i]!='{') THROW(ValueError, "Expected '{' after 'else'");
            i++;
            int bracketLevel2=1;
            while (bracketLevel2>0 && i<expression.size()){
                if (expression[i]=='{') bracketLevel2++;
                else if (expression[i]=='}') bracketLevel2--;
                elseExpr+=expression[i];
                i++;
            }
            if (bracketLevel2!=0) THROW(ValueError, "Mismatched braces in 'else' expression");
        }
        std::vector<std::vector<Command*>> idks;
        auto body=splitBy(thenExpr, ';');
        std::vector<Command*> res;
        int j=jmps+1;
        res.push_back(new Command(Types::JMP_IF, parse(condition, n), std::to_string(++jmps)));
        for (auto i:body){
            auto nn=parse(i, n);
            for (auto j:nn){
                res.push_back(j);
            }
        }
        res.push_back(new Command{Types::POINT, {NULL}, std::to_string(j)});
        return res;
    }
    if (OnlyNum(expression)){
        return {new Command{Types::WRITE, {NULL}, expression}};
    }
    if (OnlyName(expression)){
        return {new Command{Types::WRITE, {NULL}, expression}};
    }
    if (expression[0]=='{'){
        LOG("GOT DICT");
        auto arr=splitBy(expression.substr(1, expression.size()-2), ',');
        std::vector<Command*> res={new Command(Types::WRITE, {NULL}, "{}")};
        for (auto i:arr){
            auto a=splitBy(i, ':');
            LOG("MEMBER "+a[0]+" "+a[1]);
            res.push_back(new Command{Types::SETATTR, {parse(a[1], n)}, a[0]});
        }
        return res;
    }
    if (expression[0]=='['){
        LOG("GOT ARR");
        auto arr=splitBy(expression.substr(1, expression.size()-2), ',');
        std::vector<Command*> res={new Command(Types::WRITE, {NULL}, "[]"),
            new Command(Types::PUSH_STACK),
            new Command{Types::GETATTR, {NULL}, "resize"},
            new Command{Types::CALL, {NULL}, "", 
                {{new Command{Types::WRITE, {NULL}, std::to_string(arr.size())}}}},
            new Command{Types::POP_STACK}
        };
        int j=0;
        for (auto i:arr){
            res.push_back(new Command{Types::SETITEM, parse(i, n)
                , std::to_string(j)});
            j++;
        }
        return res;
    }
    /*auto assignment=expression.find('=');
    if (assignment!=std::string::npos
        && (assignment+1==expression.size() || expression[assignment+1]!='=')){
        std::string remaining, objectName;
        std::string target=expression.substr(0, assignment);
        if (startsWithOnlyName(target, remaining, objectName)
            && remaining.size()>1 && remaining[0]=='.'
            && OnlyName(remaining.substr(1))){
            return {
                new Command{Types::WRITE, parse(objectName, n)},
                new Command{Types::SETATTR, parse(expression.substr(assignment+1), n),
                    remaining.substr(1)}
            };
        }
    }*/
    std::string rem, beg;
    bool has=hasAssignment(expression);
    if (startsWithOnlyName(expression, rem, beg) && (hasNoOp(expression) || has)){
        LOG("STARTSWITHONLYNAME "+beg+" "+rem);
        std::vector<Command*> res;
        if (!has)
            res.push_back(new Command{Types::WRITE, parse(beg, n)});
        char op;
        std::string curr;
        for (int i=0;i<rem.size();i++){
            LOG("CURR IS "+curr+" REM IS "+rem[i]);
            curr+=rem[i];
            if (rem[i]=='.' || rem[i]=='[' || rem[i]=='(' || rem[i]=='=' ||
                i==rem.size()-1){
                if (rem[i]=='='){
                    i++;
                    std::string rvalue;
                    while (rem[i]!=';' && i<rem.size()){
                        rvalue+=rem[i];
                        i++;
                    }
                    LOG("RVALUE IS "+rvalue);
                    res.push_back(new Command{Types::WRITE, parse(rvalue, n)});
                    res.push_back(new Command{Types::SET, {NULL},
                       beg});
                }
                if (rem[i]=='.'){
                    i++;
                    while (i<rem.size() 
                        && isalpha(rem[i]) || rem[i]=='_'){
                        curr+=rem[i];
                        i++;
                    }
                    LOG("GOT .");
                    if (i!=rem.size()-1 && rem[i]=='='){
                        LOG("GOT =");
                        i++;
                        std::string rvalue;
                        while (rem[i]!=';' && i<rem.size()){
                            rvalue+=rem[i];
                            i++;
                        }
                        LOG("RVALUE IS "+rvalue);
                        res.push_back(new Command{Types::SETATTR, parse(rvalue, n)});
                    }
                    else
                        res.push_back(new Command{Types::GETATTR, {NULL}, curr});
                    curr.clear();
                }
                if (rem[i]=='('){
                    LOG("GOT OBJECT CALL");
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
                        i++;
                    }
                    LOG("INSIDE IS "+inside);
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
        LOG("NO OP");
        std::vector<Command*> res;
        std::string curr;
        std::string op="u";
        int bracketLevel=0;
        int bracketLevel2=0;
        int bracketLevel3=0;
        for (int i=0;i<expression.size();i++){
            curr+=expression[i];
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
        
            if ((expression[i]=='*' || expression[i]=='/' || 
                i==expression.size()-1) && (bracketLevel==0 && bracketLevel2==0 &&
                bracketLevel3==0)){
                if (i!=expression.size()-1) curr.pop_back();
                if (op=="u"){
                    res.push_back(new Command{Types::WRITE, parse(curr, n)});
                }
                if (op=="*"){
                    res.push_back(new Command{Types::MUL, parse(curr, n)});
                }
                if (op=="/"){
                    res.push_back(new Command{Types::DIV, parse(curr, n)});
                }
                if (expression[i+1]=='='){
                    LOG("GOT ASSIGNMENT IN PLACE");
                    op=expression[i];
                    i+=2;
                    std::string ss;
                    while (i<expression.size() || expression[i]==';'){
                        ss+=expression[i];
                        i++;
                    }
                    std::string itog=curr+"="+curr+op+ss;
                    LOG("ITOG IS "+itog);
                    return parse(
                        itog, n
                    );
                }
                op=expression[i];
                curr.clear();
            }
        }
        return res;
    }
    std::vector<Command*> res;
    std::string curr;
    std::string op="u";
    int bracketLevel=0;
    int bracketLevel2=0;
    int bracketLevel3=0;
    for (int i=0;i<expression.size();i++){
        curr+=expression[i];
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
        if (expression[i]=='+' || expression[i]=='-' || expression[i]=='<' || expression[i]=='>' || 
            expression.substr(i,2)=="==" || i==expression.size()-1 ||
            expression.substr(i,2)=="&&" || expression.substr(i,2)=="||" && 
            (bracketLevel==0 && bracketLevel2==0 &&
            bracketLevel3==0)){
            if (i!=expression.size()-1) curr.pop_back();
            if (op=="u"){
                res.push_back(new Command{Types::WRITE, parse(curr, n)});
            }
            if (op=="+"){
                res.push_back(new Command{Types::ADD, parse(curr, n)});
            }
            if (op=="-"){
                res.push_back(new Command{Types::SUB, parse(curr, n)});
            }
            if (op==">"){
                res.push_back(new Command{Types::GREATER, parse(curr, n)});
            }
            if (op=="<"){
                res.push_back(new Command{Types::LESS, parse(curr, n)});
            }
            if (op=="&&"){
                res.push_back(new Command{Types::AND, parse(curr, n)});
            }
            if (op=="||"){
                res.push_back(new Command{Types::OR, parse(curr, n)});
            }
            if (op=="=="){
                res.push_back(new Command{Types::EQUAL, parse(curr, n)});
            }
            if (expression[i+1]=='=' && expression.substr(i,2)!="=="){
                    LOG("GOT ASSIGNMENT IN PLACE");
                    op=expression[i];
                    i+=2;
                    std::string ss;
                    while (i<expression.size() || expression[i]==';'){
                        ss+=expression[i];
                        i++;
                    }
                    std::string itog=curr+"="+curr+op+ss;
                    LOG("ITOG IS "+itog);
                    return parse(
                        itog, n
                    );
                }
            if (expression.substr(i,2)=="==" || expression.substr(i,2)=="&&" 
                || expression.substr(i,2)=="||"){
                LOG("GOT double shi");
                op=expression.substr(i,2);
                i++;
            }
            else
                op=expression[i];
            curr.clear();
        }
    }
    return res;
}

std::vector<Command*> doCodee(std::string s, Namespace& n){
    std::vector<Command*> res;
    auto a=splitBy(s, ';');
    for (auto i:a){
        auto k=parse(i, n);
        for (auto j:k){
            res.push_back(j);
        }
    }
    return res;
}

Namespace CreateContext(){
    Namespace n;
    n["print"] = MakePtr<BasicObj>(
        new NativeFunctionObject([](std::vector<Pointer<BasicObj>> args, Namespace&) -> Pointer<BasicObj> {
            LOG("INSIDE PRINT");

            LOG("TYPE IS "+std::string(
                dynamic_cast<IntObj*>(args[0].get()) ? 
                "IntObj" : "idk other "));

            for (auto& arg : args)
                std::cout << arg->str() << " ";

            std::cout << std::endl;

            return MakePtr<BasicObj>(new IntObj(0));
        })
    );
    n["input"] = MakePtr<BasicObj>(
        new NativeFunctionObject([](std::vector<Pointer<BasicObj>> args, Namespace&){
            std::string input;
            std::getline(std::cin, input);
            return MakePtr<BasicObj>(new StringObject(input));
        })
    );
    n["newObject"] = MakePtr<BasicObj>(
        new NativeFunctionObject([](std::vector<Pointer<BasicObj>> args, Namespace& context){
            return MakePtr<BasicObj>(new InstanceObject(context, (args.size()==2 ? args[1]:nullptr)));
        })
    );
    n["import"] = MakePtr<BasicObj>(
        new NativeFunctionObject([](std::vector<Pointer<BasicObj>> args, Namespace& context){
            HINSTANCE m=LoadLibraryA((args[0]->str()+".dll").c_str());
            if (!m){
                THROW(ValueError, "Couldnt locate .dll file named "+args[0]->str());
            }
            Namespace* (*func)() = 
                (Namespace* (*)())GetProcAddress(m, "Load");
            if (!func){
                THROW(ValueError, "Couldnt find Load method in file "+args[0]->str());
            }
            Pointer<BasicObj> o=MakePtr<BasicObj>(
                new InstanceObject(context, nullptr));
            o->attrs=*func();
            if (args.size()!=2)
                context[args[0]->str()]=o;
            else
                context[args[1]->str()]=o;
            return MakePtr<BasicObj>(new IntObj(0));
        })
    );
    n["wait"]=MakePtr<BasicObj>(
        new NativeFunctionObject([](std::vector<Pointer<BasicObj>> args, Namespace& context){
            if (args.size()!=1) THROW(ValueError, "Invalid arguments count");
            Sleep(args[0]->asInt());
            return MakePtr<BasicObj>(new IntObj(0));
        })
    );
    return n;
}

int main(){
    Namespace n=CreateContext();
    n["lol"]=MakePtr<BasicObj>(new InstanceObject(n));
    n["lol"]->setattr("a", MakePtr<BasicObj>(new IntObj(67)));
    auto r=doCodee(R"(a=1;a+=5;print(a))", n);
    VirtualMachine m;
    m.exec(r, n);
}