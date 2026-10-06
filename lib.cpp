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
    TextureObject(SimpleWindow& w, int wi, int h){
        txt=SDL_CreateTexture(w.renderer,
            SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
            wi, h);
    }
    TextureObject(SimpleWindow& w, const std::string& s){
        SDL_Surface* ss=IMG_Load(s.c_str());
        txt=SDL_CreateTextureFromSurface(w.renderer, ss);
        SDL_FreeSurface(ss);
    }
    void SetAttrs(){

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
    }
    WindowObject(std::string title, int w, int h){
        window=MakePtr(new SimpleWindow(title, w, h));
        this->w=w;
        this->h=h;
        SetAttrs();
    }
    Pointer<BasicObj> clone(){
        WindowObject* o=new WindowObject(*this);
        o->SetAttrs();
        return MakePtr<BasicObj>(o);
    }
};

IMPORT Namespace* Load(){
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
    return na;
}
