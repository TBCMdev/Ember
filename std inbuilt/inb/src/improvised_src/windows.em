# will be injected in debug

class WINDOW_HWND_OPTIONS{
}
class WINDOW_HWND{
    any windowHwnd
    constructor(any hwnd){
        windowHwnd = hwnd
    }
    method EventOccured(string event){
        return 0
    }
}

method createWindow(string title){

    log("Creating process...")

    any process = CreateProcess(title, 0)
    WINDOW_HWND hwnd = new WINDOW_HWND(process)
    return hwnd
}

