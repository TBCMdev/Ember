use inb


class Window{

[public]
    string title = ""
    constructor(string t) => title(t){
    }
    method open(){
        # opens the window on another thread.
        initCenturionExternWindow(title)
        

        while(1 == 1){}
    }
    method onMouseButtonDown(int button, lambda func){
        centextMEventAdd(0, button, func)
    }
}

method createWindow(string windowTitle){
    Window hwnd = new Window(windowTitle)
    return hwnd
}

