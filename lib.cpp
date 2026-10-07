#include "classes.hpp"
#include <SDL.h>
#include <SDL2_image/SDL_image.h>

class SimpleWindow{
    public:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SimpleWindow(std::string title, int w, int h){
        window=SDL_CreateWindow(title.c_str(),SDL_WINDOWPOS_CENTERED, 
            SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_SHOWN);
        renderer=SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    }
    ~SimpleWindow(){
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
    }
};

class TextureObject:public BasicObj{
    SDL_Texture* txt;
    SimpleWindow* win;
    std::string imagePath;
    void SetAttrs(){
        setattr("angle",MakePtr<BasicObj>(new FloatObj(0.f)));
        setattr("flip",MakePtr<BasicObj>(new IntObj(SDL_FLIP_NONE)));
        setattr("center",MakePtr<BasicObj>(new EmptyObject));
        setattr("defaulted", MakePtr<BasicObj>(new BoolObject(true)));
        setattr("Draw",MakePtr<BasicObj>(new NativeFunctionObject([this](auto args, auto& n){
            if (args.size()!=1 && args.size()!=2) THROW(ValueError, 
                "Invalid arg count");
            SDL_FRect dstrect={
                args[0]->getattr("x")->asFloat(),
                args[0]->getattr("y")->asFloat(),
                args[0]->getattr("w")->asFloat(),
                args[0]->getattr("h")->asFloat(),
            };
            SDL_FPoint p;
            bool nn=false;
            if (getattr("defaulted")->asbool()){
                nn=true;
            }
            else{
                p.x=getattr("center")->getattr("x")->asFloat();
                p.y=getattr("center")->getattr("y")->asFloat();
            }
            if (args.size()==2){
                SDL_Rect srcrect={
                    args[1]->getattr("x")->asInt(),
                    args[1]->getattr("y")->asInt(),
                    args[1]->getattr("w")->asInt(),
                    args[1]->getattr("h")->asInt(),
                };
                SDL_RenderCopyExF(win->renderer, txt, &srcrect, &dstrect, getattr("angle")->asFloat(), 
                    (nn ? NULL : &p), (SDL_RendererFlip)getattr("flip")->asInt());
            }
            else{
                SDL_RenderCopyExF(win->renderer, txt, NULL, &dstrect, getattr("angle")->asFloat(), 
                    (nn ? NULL : &p), (SDL_RendererFlip)getattr("flip")->asInt());
            }
            return MakePtr<BasicObj>(new IntObj(0));
        })));
        attrs["_target"]=MakePtr<BasicObj>(new NativeFunctionObject([this](auto args, auto& n){
            return MakePtr<BasicObj>(new IntObj((long long)txt));
        }));
    }
    public:
    TextureObject(SimpleWindow& w, int wi, int h){
        txt=SDL_CreateTexture(w.renderer,
            SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
            wi, h);
        SetAttrs();
        win=&w;
    }
    TextureObject(SimpleWindow& w, const std::string& s){
        imagePath=s;
        SDL_Surface* ss=IMG_Load(s.c_str());
        if (!ss) THROW(ValueError, ("IMG_Load failed: "+std::string(IMG_GetError())).c_str());
        txt=SDL_CreateTextureFromSurface(w.renderer, ss);
        SDL_FreeSurface(ss);
        if (!txt) THROW(ValueError, ("SDL_CreateTextureFromSurface failed: "+std::string(SDL_GetError())).c_str());
        SetAttrs();
        win=&w;
    }
    Pointer<BasicObj> clone() override{
        SDL_Texture* prev=SDL_GetRenderTarget(win->renderer);
        int w,h;
        SDL_QueryTexture(txt, NULL, NULL, &w, &h);
        TextureObject* n=new TextureObject(*win, w, h);
        SDL_SetRenderTarget(win->renderer, n->txt);
        SDL_RenderCopy(win->renderer, txt, NULL, NULL);
        SDL_SetRenderTarget(win->renderer, prev);
        n->SetAttrs();
        return MakePtr<BasicObj>(n);
    }
    ~TextureObject(){
        SDL_DestroyTexture(txt);
    }
};

class RectObject:public BasicObj{
    public:
    RectObject(SDL_FRect re){
        setattr("x",MakePtr<BasicObj>(new FloatObj(re.x)));
        setattr("y",MakePtr<BasicObj>(new FloatObj(re.y)));
        setattr("w",MakePtr<BasicObj>(new FloatObj(re.w)));
        setattr("h",MakePtr<BasicObj>(new FloatObj(re.h)));
    }
    Pointer<BasicObj> clone(){
        auto p=MakePtr<BasicObj>(new RectObject(*this));
        for (auto [k,v]:attrs){
            p->setattr(k,v);
        }
        return p;
    }
};

class WindowObject:public BasicObj{
    Pointer<SimpleWindow> window;
    std::string title;
    int w,h;
    public:
    void SetAttrs(){
        attrs["present"]=MakePtr<BasicObj>(new NativeFunctionObject([this](auto args, auto& n){
            SDL_RenderPresent(window->renderer);
            return MakePtr<BasicObj>(new IntObj(0));
        }));
        attrs["fill"]=MakePtr<BasicObj>(new NativeFunctionObject([this](auto args, auto& n){
            if (args.size()!=3 && args.size()!=4) THROW(ValueError, "Invalid arguments count");
            int red=args[0]->asInt();
            int green=args[1]->asInt();
            int blue=args[2]->asInt();
            int alpha=255;;
            if (args.size()==4){
                alpha=args[3]->asInt();
            }
            SDL_SetRenderDrawColor(window->renderer, red, green, blue, alpha);
            SDL_Rect rect={0,0,this->w,this->h};
            SDL_RenderFillRect(window->renderer, &rect);
            return MakePtr<BasicObj>(new IntObj(0));
        }));
        attrs["drawRect"]=MakePtr<BasicObj>(new NativeFunctionObject([this](auto args, auto& n){
            int red=args[1]->getitem(MakePtr<BasicObj>(new IntObj(0)))
                ->asInt();
            int green=args[1]->getitem(MakePtr<BasicObj>(new IntObj(1)))
                ->asInt();
            int blue=args[1]->getitem(MakePtr<BasicObj>(new IntObj(2)))
                ->asInt();
            int alpha=args[1]->getitem(MakePtr<BasicObj>(new IntObj(3)))
                ->asInt();
            SDL_FRect r={
                args[0]->getattr("x")->asFloat(),
                args[0]->getattr("y")->asFloat(),
                args[0]->getattr("w")->asFloat(),
                args[0]->getattr("h")->asFloat()
            };
            SDL_SetRenderDrawColor(window->renderer, 
                red, green, blue, alpha);
            SDL_RenderFillRectF(window->renderer, &r);
            return MakePtr<BasicObj>(new IntObj(0));
        }));
        attrs["SetTarget"]=MakePtr<BasicObj>(new NativeFunctionObject([this](auto args, auto& n){
            if (args.size()!=1) THROW(ValueError, "Invalid arg count");
            SDL_Texture* ptr=(SDL_Texture*)args[0]->asInt();
            return MakePtr<BasicObj>(new IntObj(SDL_SetRenderTarget(window->renderer, ptr)));
        }));
    }
    WindowObject(std::string title, int w, int h){
        window=MakePtr(new SimpleWindow(title, w, h));
        this->w=w;
        this->h=h;
        SetAttrs();
    }
    SimpleWindow& simpleWindow(){
        return *(window.get());
    }
    Pointer<BasicObj> clone(){
        WindowObject* o=new WindowObject(*this);
        o->SetAttrs();
        return MakePtr<BasicObj>(o);
    }
};

IMPORT Namespace* Load(){
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG | IMG_INIT_WEBP);
    Namespace* na=new Namespace;
    (*na)["Window"]=MakePtr<BasicObj>(new NativeFunctionObject([](auto args, auto& n){
        if (args.size()!=3) THROW(ValueError, "Invalid args count");
        return MakePtr<BasicObj>(new WindowObject(args[0]->str(), args[1]->asInt(), args[2]->asInt()));
    }));
    (*na)["GetEvents"]=MakePtr<BasicObj>(new NativeFunctionObject([](auto args, auto& n){
            Pointer<BasicObj> arr=MakePtr<BasicObj>(new ArrayObject);
            int i=0;
            SDL_Event e;
            while (SDL_PollEvent(&e)){
                if (e.type==SDL_QUIT){
                    Pointer<BasicObj> o=
                        MakePtr<BasicObj>(new InstanceObject(n));
                    o->setattr("type", MakePtr<BasicObj>(
                        new StringObject("QUIT")
                    ));
                    ((ArrayObject*)arr.get())->arr.push_back(o);
                }
            }
            LOG("[ArrayObject] size is "+std::to_string(
                ((ArrayObject*)arr.get())->arr.size()));
            return arr;
    }));
    (*na)["Rect"]=MakePtr<BasicObj>(new NativeFunctionObject([](auto args, auto& n){
        if (args.empty())
            return MakePtr<BasicObj>(new RectObject({}));
        SDL_FRect r={args[0]->asFloat(),args[1]->asFloat(),
            args[2]->asFloat(),args[3]->asFloat()};
        return MakePtr<BasicObj>(new RectObject(r));
    }));
    (*na)["isPressed"]=MakePtr<BasicObj>(new NativeFunctionObject([](auto args, auto& n){
        const Uint8* k=SDL_GetKeyboardState(NULL);
        SDL_Scancode sc=SDL_GetScancodeFromName(args[0]->str()
            .c_str());
        if (sc==SDL_SCANCODE_UNKNOWN){
            return MakePtr<BasicObj>(new BoolObject(false));
        }
        return MakePtr<BasicObj>(new BoolObject(k[sc]));
    }));
    (*na)["getDelta"]=MakePtr<BasicObj>(new NativeFunctionObject([](auto args, auto& n){
        static int start=SDL_GetTicks();
        static int end=SDL_GetTicks();

        start=SDL_GetTicks();
        int dt=start-end;
        end=start;
        return MakePtr<BasicObj>(new FloatObj(dt/1000.f));

    }));
    (*na)["CreateTexture"]=MakePtr<BasicObj>(new NativeFunctionObject([](auto args, auto& n){
        if (args.size()==3){
            WindowObject* w=dynamic_cast<WindowObject*>(args[0].get());
            if (!w) THROW(ValueError, "First arg is not SimpleWindow object");
            return MakePtr<BasicObj>(new TextureObject(w->simpleWindow(),
                args[1]->asInt(), args[2]->asInt()));
        }
        else if (args.size()==2){
            WindowObject* w=dynamic_cast<WindowObject*>(args[0].get());
            if (!w) THROW(ValueError, "First arg is not SimpleWindow object");
            return MakePtr<BasicObj>(new TextureObject(w->simpleWindow(),
                args[1]->str()));
        }
        else{
            THROW(ValueError, "Invalid args count");
        }
    }));
    (*na)["FLIP_NONE"]=MakePtr<BasicObj>(new IntObj(SDL_FLIP_NONE));
    (*na)["FLIP_HORIZONTAL"]=MakePtr<BasicObj>(new IntObj(SDL_FLIP_HORIZONTAL));
    (*na)["FLIP_VERTICAL"]=MakePtr<BasicObj>(new IntObj(SDL_FLIP_VERTICAL));
    (*na)["FLIP_BOTH"]=MakePtr<BasicObj>(new IntObj(SDL_FLIP_VERTICAL | SDL_FLIP_HORIZONTAL));
    return na;
}
