declare NO_INCLUDE true
declare inb true

use 'input'
use keyboard.key

#region Events
#region keyBoard
    method onAnyKeyPress(key k, KEYSTATE state){
        log(stringify(k) + "key pressed.")
    }
    
    # templates are in development, if you have a more old version,
    # please cast the func lambda to an object:
    # lambda func = object_cast<lambda>(mem(onAnyKeyPress))


    lambda func = lambda<key, KEYSTATE>(onAnyKeyPress)
    keyboard.onKeyboardPress(func)
    # your function will run every time a key is pressed!
    # use keyboard.onKeyPress(Key, KEYSTATE) to specify a key. 
#endregion keyboard
#endregion Events