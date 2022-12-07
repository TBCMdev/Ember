use "flags"

declare inb true

exported module os {
    method getOSName(){
        return _FLAG("os", "name")
    }
    method launchProcess(Process p){
        try{
            # should check for os first, now just using windows
            WIND.PROC proc(p)

            proc.validate()

            proc.start()

            return true
        }catch{
            return false
        }
    }

}