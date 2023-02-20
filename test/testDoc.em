use windows

Window window = createWindow("Lambda test")

lambda mouseDown = lambda {
    log("Mouse Button Clicked!")
}
window.onMouseDown(0, mouseDown)

window.open()
