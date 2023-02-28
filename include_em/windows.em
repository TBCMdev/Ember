use inb


class Window{

[public]
    string title = ""
    constructor(string t) => title(t){
        __createCenWindowRequirements(0, t)
    }
    method open(){
        # opens the window on another thread.
        __initCenturionExternWindow(title)
        

        while(1 == 1){}
    }
    method onMouseDown(int button, lambda func){
        __centextMEventAdd(0, button, func)
    }
}

method createWindow(string windowTitle){
    Window hwnd = new Window(windowTitle)
    return hwnd
}

