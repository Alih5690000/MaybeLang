#include "classes.hpp"
#include <SDL2/SDL.h>

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

class WindowObject:public BasicObj{
    Pointer<SimpleWindow> window;
    std::string title;
    int w,h;
    public:
    WindowObject(std::string title, int w, int h){
        window=MakePtr(new SimpleWindow(title, w, h));
        attrs["present"]=MakePtr<BasicObj>(new NativeFunctionObject([this](auto args, auto& n){
            SDL_RenderPresent(window->renderer);
            return MakePtr<BasicObj>(new IntObj(0));
        }));
        this->w=w;
        this->h=h;
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
    }
    Pointer<BasicObj> clone(){
        WindowObject* o=new WindowObject(*this);
        return MakePtr<BasicObj>(o);
    }
};

IMPORT Namespace* Load(){
    Namespace* na=new Namespace;
    (*na)["Window"]=MakePtr<BasicObj>(new NativeFunctionObject([](auto args, auto& n){
        if (args.size()!=3) THROW(ValueError, "Invalid args count");
        return MakePtr<BasicObj>(new WindowObject(args[0]->str(), args[1]->asInt(), args[2]->asInt()));
    }));
    return na;
}