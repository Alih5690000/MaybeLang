#include <string>
#include <map>
#include <vector>
#include "memory.hpp"
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

class BasicObj{
    public:
    virtual Pointer<BasicObj> add(Pointer<BasicObj>,bool){throw NotAvailable("That is Base class (add)");};
    virtual Pointer<BasicObj> sub(Pointer<BasicObj>,bool){throw NotAvailable("That is Base class (sub)");};
    virtual Pointer<BasicObj> mul(Pointer<BasicObj>,bool){throw NotAvailable("That is Base class (mul)");};
    virtual Pointer<BasicObj> div(Pointer<BasicObj>,bool){throw NotAvailable("That is Base class (div)");};
    virtual std::string str(){return "Object at "+std::to_string((size_t)this);};
    virtual bool greater(Pointer<BasicObj>,bool){throw NotAvailable("That is Base class (greater)");};
    virtual bool less(Pointer<BasicObj>,bool){throw NotAvailable("That is Base class (less)");};
    virtual bool equal(Pointer<BasicObj>,bool){throw NotAvailable("That is Base class (equal)");};
    virtual bool asbool(){throw NotAvailable("That is Base class (asbool)");};
    virtual void free(){throw NotAvailable("That is Base class (free)");};
    virtual int asInt(){throw NotAvailable("That is Base class (asInt)");};
    virtual Pointer<BasicObj> getattr(const std::string& s){
      auto it = attrs.find(s);
      if (it==attrs.end()) throw ValueError(("Attribute "+s+" not found").c_str());
      return it->second;
    };
    virtual void setattr(const std::string& name,Pointer<BasicObj> o){
      Pointer<BasicObj> cloned = o->clone();
      attrs[name]=cloned;
    }
    virtual Pointer<BasicObj> getitem(Pointer<BasicObj>){throw NotAvailable("That is Base class (getitem)");};
    virtual Pointer<BasicObj> setitem(std::vector<Pointer<BasicObj>>){throw NotAvailable("That is Base class (setitem)");};
    virtual Pointer<BasicObj> call(std::vector<Pointer<BasicObj>>,Namespace&){throw NotAvailable("That is Base class (call)");};
    virtual void setitem(Pointer<BasicObj>, Pointer<BasicObj>){throw NotAvailable("That is Base class (setitem)");};
    virtual Pointer<BasicObj> clone(){throw NotAvailable("That is Base class (clone)");};
    virtual ~BasicObj()=default;
    std::map<std::string,Pointer<BasicObj>> attrs;
};

class IntObj:public BasicObj{
    public:
    int value;
    IntObj(int v):value(v){};

    Pointer<BasicObj> add(Pointer<BasicObj> other,bool swapped) override{
      if (auto integer=dynamic_cast<IntObj*>(other.get())){
        return MakePtr<BasicObj>(new IntObj(value+integer->value));
      }
      if (!swapped)
        return other->add(MakePtr<BasicObj>(new IntObj(value)),true);
      throw ValueError("Cannot add non-integer object to integer");
    }

    Pointer<BasicObj> sub(Pointer<BasicObj> other,bool swapped) override{
      if (auto integer=dynamic_cast<IntObj*>(other.get())){
        int result=swapped ? integer->value-value : value-integer->value;
        return MakePtr<BasicObj>(new IntObj(result));
      }
      if (!swapped)
        return other->sub(MakePtr<BasicObj>(new IntObj(value)),true);
      throw ValueError("Cannot subtract non-integer object from integer");
    }

    Pointer<BasicObj> mul(Pointer<BasicObj> other,bool swapped) override{
      if (auto integer=dynamic_cast<IntObj*>(other.get())){
        return MakePtr<BasicObj>(new IntObj(value*integer->value));
      }
      if (!swapped)
        return other->mul(MakePtr<BasicObj>(new IntObj(value)),true);
      throw ValueError("Cannot multiply non-integer object by integer");
    }

    Pointer<BasicObj> div(Pointer<BasicObj> other,bool swapped) override{
      if (auto integer=dynamic_cast<IntObj*>(other.get())){
        int dividend=swapped ? integer->value : value;
        int divisor=swapped ? value : integer->value;
        if (divisor==0) throw ValueError("Division by zero");
        return MakePtr<BasicObj>(new IntObj(dividend/divisor));
      }
      if (!swapped)
        return other->div(MakePtr<BasicObj>(new IntObj(value)),true);
      throw ValueError("Cannot divide integer by non-integer object");
    }

    int asInt() override{
      return value;
    }

    std::string str() override{
      return std::to_string(value);
    }

    bool greater(Pointer<BasicObj> other,bool) override{
      return value>asInt(other);
    }

    bool less(Pointer<BasicObj> other,bool) override{
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
      if (integer==nullptr) throw ValueError("Expected an integer");
      return integer->value;
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
      throw ValueError("Cannot add non-string object to string");
    }

    Pointer<BasicObj> sub(Pointer<BasicObj> other,bool swapped) override{
      if (!swapped)
        return other->sub(MakePtr<BasicObj>(new StringObject(value)),true);
      throw ValueError("Cannot subtract from string");
    }

    Pointer<BasicObj> mul(Pointer<BasicObj> other,bool swapped) override{
      if (auto integer=dynamic_cast<IntObj*>(other.get())){
        if (integer->value<0) throw ValueError("Cannot multiply string by a negative integer");
        std::string result;
        for (int count=0;count<integer->value;count++)
          result+=value;
        return MakePtr<BasicObj>(new StringObject(result));
      }
      if (!swapped)
        return other->mul(MakePtr<BasicObj>(new StringObject(value)),true);
      throw ValueError("Cannot multiply string by non-integer object");
    }

    Pointer<BasicObj> div(Pointer<BasicObj> other,bool swapped) override{
      if (!swapped)
        return other->div(MakePtr<BasicObj>(new StringObject(value)),true);
      throw ValueError("Cannot divide string");
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
      if (string==nullptr) throw ValueError("Expected a string");
      return string->value;
    }
};