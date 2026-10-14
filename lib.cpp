#include "classes.hpp"
#include <SDL.h>
#include <SDL2_ttf/SDL_ttf.h>
#include <SDL2_image/SDL_image.h>

class SimpleWindow{
    public:
    SDL_Window* window;
    SDL_Renderer* renderer;
    SimpleWindow(std::string title, int w, int h){
        window=SDL_CreateWindow(title.c_str(),SDL_WINDOWPOS_CENTERED, 
            SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_SHOWN);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
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
        setattr("SetSize",MakePtr<BasicObj>(new NativeFunctionObject([this](auto args, auto& n){
            setattr("w", args[0]->getitem(MakePtr<BasicObj>(new IntObj(0))));
            setattr("h", args[0]->getitem(MakePtr<BasicObj>(new IntObj(1))));
            return MakePtr<BasicObj>(new IntObj(1));
        })));
    }
    public:
    TextureObject(SimpleWindow& w, int wi, int h){
        txt=SDL_CreateTexture(w.renderer,
            SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET,
            wi, h);
        SDL_SetTextureBlendMode(txt, SDL_BLENDMODE_BLEND);
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
    TextureObject(SimpleWindow& w, SDL_Texture* t){
        txt=t;
        win=&w;
        SetAttrs();
    }
    Pointer<BasicObj> clone() override{
        SDL_Texture* prev=SDL_GetRenderTarget(win->renderer);
        int w,h;
        SDL_QueryTexture(txt, NULL, NULL, &w, &h);
        TextureObject* n=new TextureObject(*win, w, h);
        SDL_SetRenderTarget(win->renderer, n->txt);
        SDL_RenderCopy(win->renderer, txt, NULL, NULL);
        SDL_SetRenderTarget(win->renderer, prev);
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
    public:
    Pointer<SimpleWindow> window;
    std::string title;
    int w,h;
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
        attrs["IsFocused"]=MakePtr<BasicObj>(new NativeFunctionObject([this](auto args, auto& n){
            bool focused = SDL_GetWindowFlags(window->window) & SDL_WINDOW_INPUT_FOCUS;
            return MakePtr<BasicObj>(new BoolObject(focused));
        }));
        attrs["SetFullscreen"] = MakePtr<BasicObj>(new NativeFunctionObject([this](auto args, auto& n){
            if (args.size()!=1) THROW(ValueError, "Invalid arg count");
            SDL_SetWindowFullscreen(window->window, (args[0]->asbool() ? SDL_WINDOW_FULLSCREEN_DESKTOP:0));
            return MakePtr<BasicObj>(new IntObj(0));
        }));
        attrs["RaiseAbove"] = MakePtr<BasicObj>(new NativeFunctionObject([this](auto args, auto& n){
            if (args.size()!=0) THROW(ValueError, "Invalid arg count");
            SDL_RaiseWindow(window->window);
            return MakePtr<BasicObj>(new IntObj(0));
        }));
        attrs["GrabMouse"] = MakePtr<BasicObj>(new NativeFunctionObject([this](auto args, auto& n){
            if (args.size()!=1) THROW(ValueError, "Invalid arg count");
            SDL_SetWindowGrab(window->window, (SDL_bool)args[0]->asbool());
            return MakePtr<BasicObj>(new IntObj(0));
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

class FontObject:public BasicObj{
    public:
    TTF_Font* font;
    std::string path;
    int size;
    void SetAttrs(){
        attrs["CreateTxt"] = MakePtr<BasicObj>(new NativeFunctionObject([this](auto args, auto& n){
            if (args.size()!=3) THROW(ValueError, "Invalid arg count");
            SDL_Surface* s=TTF_RenderText_Solid(font, args[1]->str().c_str(), {
                (int)args[2]->getattr("r")->asInt(),
                (int)args[2]->getattr("g")->asInt(),
                (int)args[2]->getattr("b")->asInt(),
                (int)args[2]->getattr("a")->asInt()
            });
            if (!s) THROW(ValueError, ("TTF_RenderText_Solid failed: "+std::string(TTF_GetError())).c_str());
            WindowObject* w=dynamic_cast<WindowObject*>(args[0].get());
            if (!w) {
                SDL_FreeSurface(s);
                THROW(ValueError, "First argument is not WindowObject");
            }
            SDL_Texture* t=SDL_CreateTextureFromSurface(w->window->renderer, s);
            SDL_FreeSurface(s);
            if (!t) THROW(ValueError, ("SDL_CreateTextureFromSurface failed: "+std::string(SDL_GetError())).c_str());
            return MakePtr<BasicObj>(new TextureObject(*w->window.get(), t));
        }));
    }
    FontObject(std::string path, int size){
        font=TTF_OpenFont(path.c_str(), size);
        if (!font) THROW(ValueError, ("TTF_OpenFont failed: "+std::string(TTF_GetError())).c_str());
        this->path=path;
        this->size=size;
        SetAttrs();
    }
    Pointer<BasicObj> clone(){
        auto pp=MakePtr<BasicObj>(new FontObject(path, size));
        return pp;
    }
};

Pointer<BasicObj> MakeEvent(SDL_Event e, Namespace& n){
    Pointer<BasicObj> o=
        MakePtr<BasicObj>(new InstanceObject(n));
    if (e.type==SDL_QUIT){
        o->setattr("type", MakePtr<BasicObj>(
            new StringObject("QUIT")
        ));
    }
    else if (e.type==SDL_KEYDOWN){
        o->setattr("type", MakePtr<BasicObj>(
            new StringObject("KEYDOWN")
        ));
        o->setattr("key", MakePtr<BasicObj>(
            new StringObject(SDL_GetKeyName(e.key.keysym.sym))
        ));
        o->setattr("alt", MakePtr<BasicObj>(
            new BoolObject((e.key.keysym.mod & KMOD_ALT) != 0)
        ));
        o->setattr("shift", MakePtr<BasicObj>(
            new BoolObject((e.key.keysym.mod & KMOD_SHIFT) != 0)
        ));
    }
    else{
        o->setattr("type", MakePtr<BasicObj>(
            new StringObject("IDK")
        ));
    }
    return o;
}

struct Data{
    std::vector<Pointer<BasicObj>> a;
    Namespace* n;
};

auto Bound(void* d,SDL_Event* e){
    Data dd=*(Data*)d;
    return (int)dd.a[0]->call({MakeEvent(*e, *dd.n)}, *dd.n)->asInt();
};

Pointer<BasicObj> currHandler;
Namespace* currNamespace;

BOOL WINAPI Bound2(DWORD type) {
    if (!currHandler.get())
        return FALSE;
    switch (type) {
        case CTRL_C_EVENT:
            currHandler->call({MakePtr<BasicObj>(new IntObj(type))}, *currNamespace);
            return TRUE;

        case CTRL_CLOSE_EVENT:
            currHandler->call({MakePtr<BasicObj>(new IntObj(type))}, *currNamespace);
            return TRUE;

        case CTRL_BREAK_EVENT:
            currHandler->call({MakePtr<BasicObj>(new IntObj(type))}, *currNamespace);
            return TRUE;
    }
    return FALSE;
}

std::vector<Data*> datas;

IMPORT Namespace* Load(){
    SDL_Init(SDL_INIT_EVERYTHING);
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG | IMG_INIT_WEBP);
    if (TTF_Init()!=0) THROW(ValueError, ("TTF_Init failed: "+std::string(TTF_GetError())).c_str());
    Namespace* na=new Namespace;
    (*na)["Window"]=MakePtr<BasicObj>(new NativeFunctionObject([](auto args, auto& n){
        if (args.size()<3) THROW(ValueError, "Invalid args count");
        return MakePtr<BasicObj>(new WindowObject(args[0]->str(), args[1]->asInt(), args[2]->asInt()));
    }));
    (*na)["GetEvents"]=MakePtr<BasicObj>(new NativeFunctionObject([](auto args, auto& n){
            Pointer<BasicObj> arr=MakePtr<BasicObj>(new ArrayObject);
            int i=0;
            SDL_Event e;
            while (SDL_PollEvent(&e)){
                auto o=MakeEvent(e ,n);
                ((ArrayObject*)arr.get())->arr.push_back(o);
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
    (*na)["CreateFont"]=MakePtr<BasicObj>(new NativeFunctionObject([](auto args, auto& n){
        if (args.size()!=2) THROW(ValueError, "Invalid args count");
        return MakePtr<BasicObj>(new FontObject(args[0]->str(), args[1]->asInt()));
    }));
    (*na)["SetEventHook"]=MakePtr<BasicObj>(new NativeFunctionObject([](auto args, auto& n){
        Data* d = new Data{args, &n};
        SDL_AddEventWatch(Bound, d);
        datas.push_back(d);
        return MakePtr<BasicObj>(new IntObj(0));
    }));
    (*na)["SetConsoleHook"]=MakePtr<BasicObj>(new NativeFunctionObject([](auto args, auto& n){
        currHandler=args[0];
        currNamespace=&n;
        SetConsoleCtrlHandler(Bound2, TRUE);
        return MakePtr<BasicObj>(new IntObj(0));
    }));
    (*na)["FLIP_NONE"]=MakePtr<BasicObj>(new IntObj(SDL_FLIP_NONE));
    (*na)["FLIP_HORIZONTAL"]=MakePtr<BasicObj>(new IntObj(SDL_FLIP_HORIZONTAL));
    (*na)["FLIP_VERTICAL"]=MakePtr<BasicObj>(new IntObj(SDL_FLIP_VERTICAL));
    (*na)["FLIP_BOTH"]=MakePtr<BasicObj>(new IntObj(SDL_FLIP_VERTICAL | SDL_FLIP_HORIZONTAL));
    (*na)["CONSOLE_CTRL_C"]=MakePtr<BasicObj>(new IntObj(CTRL_C_EVENT));
    return na;
}

IMPORT void Quit(Namespace* n){
    for (auto i:datas){
        SDL_DelEventWatch(Bound, i);
        delete i;
    }
}
