use inb
# will be injected in debug


class WINDOW_HWND_OPTIONS{
}
class WINDOW_HWND{

[public]
    int processID = -1

    constructor(int procID) => processID(procID){
    }
    method EventOccured(string event){
        # return inb.windows[]
        return 0
    }
    method pollFpsRate(int max){
        return max / 1
    }
}

method createWindow(string title){
    WINDOW_HWND hwnd = new WINDOW_HWND(0)
    return hwnd
}

