use windows

class vec2{
    float x = 0
    float y = 0

    constructor(float cx, float cy) => x(cx), y(cy) {}

    method print(){
        log("(vec2) ", x, y)
    }
}
class GameConfig{
    string name
    vec2 window_scale
    constructor(string n, vec2 w_scale) => window_scale(w_scale), name(n){
    }
}
class Game{
    list objects = []
    Window _hwnd = nothing
    constructor(string name, vec2 scale)
    {
        config = new GameConfig(name, scale)
        _hwnd = new Window(name)
    }
    method mainloop(){
        # index things...
        _hwnd.open()
    }
}

