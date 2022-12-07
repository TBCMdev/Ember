use "windows"

declare inb true

exported module keyboard{
    enum KEY = WIND.keybinds()
    enum KEYSTATE = {
        DOWN,
        UP,
        PRESS
    }
    method onKeyDown(KEY k, lambda{} l){
        any x = WIND.Events.subscribe("__key_down", l);
        __err_assert(x instanceof WIND.Events.KeyEvent)

        WIND.Events._rsubs()
    }
    method onKeyUp(Key k, lambda{} l){
        any x = WIND.Events.subscribe("__key_up", l);
        __err_assert(x instanceof WIND.Events.KeyEvent)

        WIND.Events._rsubs()
    }
    method onKeyPress(Key k, lambda{} l){
        any x = WIND.Events.subscribe("__key_press", l);
        __err_assert(x instanceof WIND.Events.KeyEvent)
        WIND.Events._rsubs()
    }
    method onKeyEvent(Key k, lambda{KEYSTATE} l){
        any x = WIND.Events.subscribe("__key",k, l);
        __err_assert(x instanceof WIND.Events.KeyEvent)

        WIND.Events._rsubs()
    }
    method onKeyboardPress(lambda{KEY, KEYSTATE} l){
        WIND.Events.subscribe("__key_b_p",l)
        __err_assert(x instanceof WIND.Events.KeyEvent)

    }
   
}
