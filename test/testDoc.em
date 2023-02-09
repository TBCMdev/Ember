use windows


WINDOW_HWND mywindow = createWindow("MyWindow!")

while (1 == 1){
int x = mywindow.pollFpsRate(60)
log(x)
}