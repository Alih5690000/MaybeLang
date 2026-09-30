#include "classes.hpp"

IMPORT Namespace* Load(){
    Namespace* n=new Namespace;
    (*n)["lol"]=MakePtr<BasicObj>(new IntObj(67));
    return n;
}