declare inb true

use inb(__events)


exported module WIND{
    define: _WIND_FLAGS = {
        os: inb.__getWindowsData() -> catch = {
            return null
        }
    }
    class PROCINFO{
        bool handle_inheritance = false
        int creation_flags = 0
        bool use_dir = true
    [public]
        constructor(bool handle_i = false, int creation_f = 0, bool use_d = true)
        => handle_inheritance(handle_i), creation_flags(creation_f), use_dir(use_d) {}
        method unpack(){
            return [handle_inheritance, creation_flags, use_dir ? __dir__ : null]

        }
    }

    #[region] UNI VARS/FUNCS

    method keybinds(){
        # [TODO] 
    }

    #[endregion]


    


    class PROC{
        string path
    [public]
        constructor(string p) => path(p) {
            
        }
        method validate(){
            # validate that the process can be launched
        }
        method start(PROCINFO p){
            # start the process


            inb.CreateProcess(path, ... [ p.unpack() ])

        }
    }
    module Events{
        method subscribe(string sub, lambda{any...} l){
            
        }
        method _rsubs(){
            # [TODO]
            # reload subscriptions to all events
            for(Type t in inherited_from<Event>()){
                t._refresh_subs()
            }
        }

        class Event{
            static virtual list subscriptions = []
            method _refresh_subs() static, virtual {
                return false;
            }
        }

        class KeyEvent extends Event{
            static list subscriptions = []
            # [TODO]
            method _refresh_subs() static {
                for(lambda x of subscriptions){
                    # 'mem(Type any)' is a function that extracts the memory address of the variable.
                    # it can be used to create key identifiers.
                    if(__events["key"][mem(x)] == null) __events["key"].add(x)
                }
                return true;
            }

        }
        class MouseEvent extends Event{
            static list subscriptions = []
            # [TODO]
            method _refresh_subs() static {
                for(lambda x of subscriptions){
                    if(__events["mouse"][mem(x)] == null) __events["mouse"].add(x)
                }
                return true;
            }
        }
    }
}