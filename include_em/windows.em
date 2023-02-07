use inb
# will be injected in debug


class WINDOW_HWND_OPTIONS{
}
class WINDOW_HWND{
    int processID
    constructor(int procID){
        processID = procID
    }
    method EventOccured(string event){
        # return inb.windows[]
        return 0
    }
}

method createWindow(string title){
    int id = inb.window(title, 0)
    WINDOW_HWND hwnd = new WINDOW_HWND(id)
    return hwnd
}

