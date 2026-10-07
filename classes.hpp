#include <string>
#include <map>
#include <vector>
#include <utility>
#include <functional>
//#define DEBUG
#include "memory.hpp"
#ifdef __WIN32
#include <windows.h>
#define IMPORT extern "C" __declspec(dllexport)
#else
#define IMPORT
#endif

class NotAvailable:public std::exception{
  public:
  std::string mes;
  NotAvailable(const char* mes) {
    this->mes=mes;
  }
  const char* what() const noexcept override{
    return mes.c_str();
  }
};

class ValueError:public std::exception{
  public:
  std::string mes;
  ValueError(const char* mes) {
    this->mes=mes;
  }
  const char* what() const noexcept override{
    return mes.c_str();
  }
};

class BasicObj;
typedef std::map<std::string,Pointer<BasicObj>> Namespace;

void doCode(const std::string& code, Namespace& context);

class ReturnSig:public std::exception{
  public:
  Pointer<BasicObj> sig;
  ReturnSig(Pointer<BasicObj> s):sig(s){}
  const char* what() const noexcept override{
    return "Uncaught return statement";
  }
};

class BreakSig:public std::exception{
  public:
  const char* what() const noexcept override{
    return "Uncaught break statement";
  }
};

class BasicObj{
    public:
    virtual Pointer<BasicObj> add(Pointer<BasicObj>,bool){THROW(NotAvailable, "That is Base class (add)");};
    virtual Pointer<BasicObj> sub(Pointer<BasicObj>,bool){THROW(NotAvailable, "That is Base class (sub)");};
    virtual Pointer<BasicObj> mul(Pointer<BasicObj>,bool){THROW(NotAvailable, "That is Base class (mul)");};
    virtual Pointer<BasicObj> div(Pointer<BasicObj>,bool){THROW(NotAvailable, "That is Base class (div)");};
    virtual std::string str(){return "Object at "+std::to_string((size_t)this);};
    virtual bool greater(Pointer<BasicObj>,bool){THROW(NotAvailable, "That is Base class (greater)");};
    virtual bool less(Pointer<BasicObj>,bool){THROW(NotAvailable, "That is Base class (less)");};
    virtual bool equal(Pointer<BasicObj>,bool){THROW(NotAvailable, "That is Base class (equal)");};
    virtual bool asbool(){THROW(NotAvailable, "That is Base class (asbool)");};
    virtual long long asInt(){THROW(NotAvailable, "That is Base class (asInt)");};
    virtual float asFloat(){THROW(NotAvailable, "That is Base class (asFloat)");};
    virtual Pointer<BasicObj> getattr(const std::string& s){
      auto it = attrs.find(s);
      if (it==attrs.end()) THROW(ValueError, ("Attribute "+s+" not found").c_str());
      return it->second;
    };
    virtual void setattr(const std::string& name,Pointer<BasicObj> o){
      Pointer<BasicObj> cloned = o->clone();
      attrs[name]=cloned;
    }
    virtual Pointer<BasicObj> getitem(Pointer<BasicObj>){THROW(NotAvailable, "That is Base class (getitem)");};
    virtual Pointer<BasicObj> setitem(std::vector<Pointer<BasicObj>>){THROW(NotAvailable, "That is Base class (setitem)");};
    virtual Pointer<BasicObj> call(std::vector<Pointer<BasicObj>>,Namespace&){THROW(NotAvailable, "That is Base class (call)");};
    virtual void setitem(Pointer<BasicObj>, Pointer<BasicObj>){THROW(NotAvailable, "That is Base class (setitem)");};
    virtual Pointer<BasicObj> clone()=0;
    virtual ~BasicObj()=default;
    std::map<std::string,Pointer<BasicObj>> attrs;
};

class IntObj:public BasicObj{
    public:
    long long value;
    IntObj(long long v):value(v){};

    Pointer<BasicObj> add(Pointer<BasicObj> other,bool swapped) override{
      if (auto integer=dynamic_cast<IntObj*>(other.get())){
        return MakePtr<BasicObj>(new IntObj(value+integer->value));
      }
      if (!swapped)
        return other->add(MakePtr<BasicObj>(new IntObj(value)),true);
      THROW(ValueError, "Cannot add non-integer object to integer");
    }

    Pointer<BasicObj> sub(Pointer<BasicObj> other,bool swapped) override{
      if (auto integer=dynamic_cast<IntObj*>(other.get())){
        int result=swapped ? integer->value-value : value-integer->value;
        return MakePtr<BasicObj>(new IntObj(result));
      }
      if (!swapped)
        return other->sub(MakePtr<BasicObj>(new IntObj(value)),true);
      THROW(ValueError, "Cannot subtract non-integer object from integer");
    }

    Pointer<BasicObj> mul(Pointer<BasicObj> other,bool swapped) override{
      if (auto integer=dynamic_cast<IntObj*>(other.get())){
        return MakePtr<BasicObj>(new IntObj(value*integer->value));
      }
      if (!swapped)
        return other->mul(MakePtr<BasicObj>(new IntObj(value)),true);
      THROW(ValueError, "Cannot multiply non-integer object by integer");
    }

    Pointer<BasicObj> div(Pointer<BasicObj> other,bool swapped) override{
      if (auto integer=dynamic_cast<IntObj*>(other.get())){
        int dividend=swapped ? integer->value : value;
        int divisor=swapped ? value : integer->value;
        if (divisor==0) THROW(ValueError, "Division by zero");
        return MakePtr<BasicObj>(new IntObj(dividend/divisor));
      }
      if (!swapped)
        return other->div(MakePtr<BasicObj>(new IntObj(value)),true);
      THROW(ValueError, "Cannot divide integer by non-integer object");
    }

    long long asInt() override{
      return value;
    }

    float asFloat() override{
      return value;
    }

    std::string str() override{
      return std::to_string(value);
    }

    bool greater(Pointer<BasicObj> other,bool) override{
      return value>asInt(other);
    }

    bool less(Pointer<BasicObj> other,bool) override{
      LOG("INT::LESS OTHER IS "+other->str());
      return value<asInt(other);
    }

    bool equal(Pointer<BasicObj> other,bool) override{
      return value==asInt(other);
    }

    bool asbool() override{
      return value!=0;
    }

    Pointer<BasicObj> clone() override{
      return MakePtr<BasicObj>(new IntObj(value));
    }

    private:
    int asInt(Pointer<BasicObj> other){
      IntObj* integer=dynamic_cast<IntObj*>(other.get());
      if (integer==nullptr) THROW(ValueError, "Expected an integer");
      return integer->value;
    }
};

class FloatObj:public BasicObj{
    public:
    float value;
    FloatObj(float v):value(v){};

    Pointer<BasicObj> add(Pointer<BasicObj> other,bool swapped) override{
      if (dynamic_cast<IntObj*>(other.get()) || dynamic_cast<FloatObj*>(other.get())){
        return MakePtr<BasicObj>(new FloatObj(asFloat(other)+value));
      }
      if (!swapped)
        return other->add(MakePtr<BasicObj>(new FloatObj(value)),true);
      THROW(ValueError, "Cannot add non-numeric object to float");
    }

    Pointer<BasicObj> sub(Pointer<BasicObj> other,bool swapped) override{
      if (dynamic_cast<IntObj*>(other.get()) || dynamic_cast<FloatObj*>(other.get())){
        float result=swapped ? asFloat(other)-value : value-asFloat(other);
        return MakePtr<BasicObj>(new FloatObj(result));
      }
      if (!swapped)
        return other->sub(MakePtr<BasicObj>(new FloatObj(value)),true);
      THROW(ValueError, "Cannot subtract non-numeric object from float");
    }

    Pointer<BasicObj> mul(Pointer<BasicObj> other,bool swapped) override{
      if (dynamic_cast<IntObj*>(other.get()) || dynamic_cast<FloatObj*>(other.get())){
        return MakePtr<BasicObj>(new FloatObj(asFloat(other)*value));
      }
      if (!swapped)
        return other->mul(MakePtr<BasicObj>(new FloatObj(value)),true);
      THROW(ValueError, "Cannot multiply non-numeric object by float");
    }

    Pointer<BasicObj> div(Pointer<BasicObj> other,bool swapped) override{
      if (dynamic_cast<IntObj*>(other.get()) || dynamic_cast<FloatObj*>(other.get())){
        float dividend=swapped ? asFloat(other) : value;
        float divisor=swapped ? value : asFloat(other);
        if (divisor==0.0f) THROW(ValueError, "Division by zero");
        return MakePtr<BasicObj>(new FloatObj(dividend/divisor));
      }
      if (!swapped)
        return other->div(MakePtr<BasicObj>(new FloatObj(value)),true);
      THROW(ValueError, "Cannot divide float by non-numeric object");
    }

    long long asInt() override{
      return value;
    }

    float asFloat() override{
      return value;
    }

    std::string str() override{
      return std::to_string(value);
    }

    bool greater(Pointer<BasicObj> other,bool) override{
      return value>asFloat(other);
    }

    bool less(Pointer<BasicObj> other,bool) override{
      return value<asFloat(other);
    }

    bool equal(Pointer<BasicObj> other,bool) override{
      return value==asFloat(other);
    }

    bool asbool() override{
      return value!=0.0f;
    }

    Pointer<BasicObj> clone() override{
      return MakePtr<BasicObj>(new FloatObj(value));
    }

    private:
    float asFloat(Pointer<BasicObj> other){
      if (auto floating=dynamic_cast<FloatObj*>(other.get()))
        return floating->value;
      if (auto integer=dynamic_cast<IntObj*>(other.get()))
        return static_cast<float>(integer->value);
      THROW(ValueError, "Expected a number");
    }
};

class StringObject:public BasicObj{
    public:
    std::string value;
    StringObject(const std::string& v):value(v){};

    Pointer<BasicObj> add(Pointer<BasicObj> other,bool swapped) override{
      if (auto string=dynamic_cast<StringObject*>(other.get())){
        return MakePtr<BasicObj>(new StringObject(value+string->value));
      }
      if (!swapped)
        return other->add(MakePtr<BasicObj>(new StringObject(value)),true);
      THROW(ValueError, "Cannot add non-string object to string");
    }

    Pointer<BasicObj> sub(Pointer<BasicObj> other,bool swapped) override{
      if (!swapped)
        return other->sub(MakePtr<BasicObj>(new StringObject(value)),true);
      THROW(ValueError, "Cannot subtract from string");
    }

    Pointer<BasicObj> mul(Pointer<BasicObj> other,bool swapped) override{
      if (auto integer=dynamic_cast<IntObj*>(other.get())){
        if (integer->value<0) THROW(ValueError, "Cannot multiply string by a negative integer");
        std::string result;
        for (int count=0;count<integer->value;count++)
          result+=value;
        return MakePtr<BasicObj>(new StringObject(result));
      }
      if (!swapped)
        return other->mul(MakePtr<BasicObj>(new StringObject(value)),true);
      THROW(ValueError, "Cannot multiply string by non-integer object");
    }

    Pointer<BasicObj> div(Pointer<BasicObj> other,bool swapped) override{
      if (!swapped)
        return other->div(MakePtr<BasicObj>(new StringObject(value)),true);
      THROW(ValueError, "Cannot divide string");
    }

    std::string str() override{
      return value;
    }

    bool greater(Pointer<BasicObj> other,bool) override{
      return value>asString(other);
    }

    bool less(Pointer<BasicObj> other,bool) override{
      return value<asString(other);
    }

    bool equal(Pointer<BasicObj> other,bool) override{
      return value==asString(other);
    }

    bool asbool() override{
      return !value.empty();
    }

    Pointer<BasicObj> clone() override{
      return MakePtr<BasicObj>(new StringObject(value));
    }

    private:
    std::string asString(Pointer<BasicObj> other){
      StringObject* string=dynamic_cast<StringObject*>(other.get());
      if (string==nullptr) THROW(ValueError, "Expected a string");
      return string->value;
    }
};

class BoolObject:public BasicObj{
    public:
    bool value;
    BoolObject(bool v):value(v){};

    long long asInt() override{
      return value ? 1 : 0;
    }

    bool asbool() override{
      return value;
    }

    std::string str() override{
      return value ? "true" : "false";
    }

    bool greater(Pointer<BasicObj> other,bool) override{
      return value>asBoolValue(other);
    }

    bool less(Pointer<BasicObj> other,bool) override{
      return value<asBoolValue(other);
    }

    bool equal(Pointer<BasicObj> other,bool) override{
      return value==asBoolValue(other);
    }

    Pointer<BasicObj> clone() override{
      return MakePtr<BasicObj>(new BoolObject(value));
    }

    private:
    bool asBoolValue(Pointer<BasicObj> other){
      BoolObject* boolean=dynamic_cast<BoolObject*>(other.get());
      if (boolean==nullptr) THROW(ValueError, "Expected a boolean");
      return boolean->value;
    }
};

Pointer<BasicObj> parseExpression(const std::string& expression, Namespace& context);

class FunctionObject:public BasicObj{
  public:
    std::vector<std::string> params;
    std::string body;
    FunctionObject(const std::vector<std::string>& p, const std::string& b):params(p),body(b){
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
      try{
        doCode(body, localContext);
      }
      catch(ReturnSig& s){
        return s.sig;
      }
      return MakePtr<BasicObj>(new IntObj(0));
    }

    Pointer<BasicObj> clone() override{
      return MakePtr<BasicObj>(new FunctionObject(params,body));
    }
};

struct EmptyObject:public BasicObj{
  Pointer<BasicObj> clone(){
    auto obj=MakePtr<BasicObj>(new EmptyObject);
    for (auto [k,v]:attrs){
      obj->setattr(k,v);
    }
    return obj;
  }
};

class NativeFunctionObject:public BasicObj{
  public:
    std::function<Pointer<BasicObj>(std::vector<Pointer<BasicObj>>,Namespace&)> func;
    NativeFunctionObject(std::function<Pointer<BasicObj>(std::vector<Pointer<BasicObj>>, Namespace&)> f):func(f){};

    Pointer<BasicObj> call(std::vector<Pointer<BasicObj>> args,Namespace& n) override{
      return func(args,n);
    }
    Pointer<BasicObj> clone() override{
      return MakePtr<BasicObj>(new NativeFunctionObject(func));
    }
};

class InstanceObject:public BasicObj{
  public:
    Pointer<BasicObj> Prototype;
    Namespace& context;

    InstanceObject(Namespace& context,Pointer<BasicObj> c=nullptr):Prototype(c),context(context){};

    Pointer<BasicObj> getattr(const std::string& s) override{
      if (attrs.find(s)!=attrs.end()) return attrs[s];
      if (Prototype.get()) return Prototype->attrs[s];
      THROW(ValueError,"Couldnt find variable named "+s);
    }

    Pointer<BasicObj> call(std::vector<Pointer<BasicObj>> args,Namespace& context) override{
      return operatorFunction("__call__", "call")->call(args,context);
    }

    Pointer<BasicObj> add(Pointer<BasicObj> other,bool swapped) override{
      return operatorFunction("__add__", "add")->call({receiver(),other},context);
    }

    Pointer<BasicObj> sub(Pointer<BasicObj> other,bool swapped) override{
      return operatorFunction("__sub__", "subtract")->call({receiver(),other},context);
    }

    Pointer<BasicObj> mul(Pointer<BasicObj> other,bool swapped) override{
      return operatorFunction("__mul__", "multiply")->call({receiver(),other},context);
    }

    Pointer<BasicObj> div(Pointer<BasicObj> other,bool swapped) override{
      return operatorFunction("__div__", "divide")->call({receiver(),other},context);
    }

    bool greater(Pointer<BasicObj> other,bool swapped) override{
      return operatorFunction("__greater__", "compare")->call({receiver(),other},context)->asbool();
    }

    bool less(Pointer<BasicObj> other,bool swapped) override{
      return operatorFunction("__less__", "compare")->call({receiver(),other},context)->asbool();
    }

    bool equal(Pointer<BasicObj> other,bool swapped) override{
      return operatorFunction("__equal__", "compare")->call({receiver(),other},context)->asbool();
    }

    Pointer<BasicObj> getitem(Pointer<BasicObj> index) override{
      return operatorFunction("__getitem__", "get item")->call({receiver(),index},context);
    }

    void setitem(Pointer<BasicObj> index, Pointer<BasicObj> value) override{
      operatorFunction("__setitem__", "set item")->call({receiver(),index,value},context);
    }

    Pointer<BasicObj> clone() override{
      auto obj=MakePtr<BasicObj>(new InstanceObject(context, Prototype));
      for (auto [k,v]:attrs){
        obj->setattr(k,v);
      }
      return obj;
    }

  private:
    Pointer<BasicObj> operatorFunction(const std::string& name,const std::string& operation){
      auto it=attrs.find(name);
      if (it!=attrs.end()) return it->second;
      if (Prototype.get()) {
        auto prototypeIt=Prototype->attrs.find(name);
        if (prototypeIt!=Prototype->attrs.end()) return prototypeIt->second;
      }
      THROW(ValueError, ("No instance or prototype attribute "+name+" to "+operation).c_str());
    }

    Pointer<BasicObj> receiver(){
      auto obj=MakePtr<BasicObj>(new InstanceObject(context, Prototype));
      obj->attrs=attrs;
      return obj;
    }
};

class ArrayObject:public BasicObj{
  public:
  std::vector<Pointer<BasicObj>> arr;

  ArrayObject(){
    auto p=[this](std::vector<Pointer<BasicObj>> a, Namespace&){
      arr.push_back(a[0]);
      return MakePtr<BasicObj>(new IntObj(0));
    };
    attrs["push_back"]=MakePtr<BasicObj>(new NativeFunctionObject(p));
    auto pp=[this](std::vector<Pointer<BasicObj>> a, Namespace&){
      arr.pop_back();
      return MakePtr<BasicObj>(new IntObj(0));
    };
    auto ppp=[this](std::vector<Pointer<BasicObj>> args, Namespace& context){
      LOG("[ArrayObject] arr size is "+std::to_string(arr.size()));
      return MakePtr<BasicObj>(new IntObj(arr.size()));
    };
    attrs["pop_back"]=MakePtr<BasicObj>(new NativeFunctionObject(pp));
    attrs["size"]=MakePtr<BasicObj>(new NativeFunctionObject(ppp));
  };
  ArrayObject(std::vector<Pointer<BasicObj>> a)
    {
      auto p=[this](std::vector<Pointer<BasicObj>> a, Namespace&){
      arr.push_back(a[0]);
      return MakePtr<BasicObj>(new IntObj(0));
    };
    attrs["push_back"]=MakePtr<BasicObj>(new NativeFunctionObject(p));
    auto pp=[this](std::vector<Pointer<BasicObj>> a, Namespace&){
      arr.pop_back();
      return MakePtr<BasicObj>(new IntObj(0));
    };
    auto ppp=[this](std::vector<Pointer<BasicObj>> args, Namespace& context){
      LOG("[ArrayObject] arr size is "+std::to_string(arr.size()));
      return MakePtr<BasicObj>(new IntObj(arr.size()));
    };
    attrs["pop_back"]=MakePtr<BasicObj>(new NativeFunctionObject(pp));
    attrs["size"]=MakePtr<BasicObj>(new NativeFunctionObject(ppp));
      arr = std::move(a);
    }

  void setitem(Pointer<BasicObj> s, Pointer<BasicObj> p) override{
    if (s->asInt()>=arr.size())
      THROW(ValueError,"Key "+s->str()+
        " is absent in array, size is "+std::to_string(arr.size()));
    arr[s->asInt()]=p->clone();
  }
  Pointer<BasicObj> getitem(Pointer<BasicObj> s) override{
    if (s->asInt()>=arr.size())
      THROW(ValueError,"Key "+s->str()+" is absent in array");
    return arr[s->asInt()];
  }
  Pointer<BasicObj> clone() override{
    Pointer<BasicObj> o=MakePtr<BasicObj>(new ArrayObject(arr));
    
    return o;
  }
};
