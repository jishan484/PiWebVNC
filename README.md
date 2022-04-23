# PiWebVNC
A simple, highly efficient web baseed VNC app for Raspberry pi (all models).

## Installation process
To compile this app some adition package will be installed in your pi. You can use pre-compiled binary, please check [Downloads](#Downloads) section.
#### setup
```sh
git clone https://github.com/jishan484/PiWebVNC.git
cd PiWebVNC
sudo apt install -y libx11-dev libxdamage-dev libxfixes-dev libxtst-dev liblz4-dev g++
```
#### Compile and Run
```sh
g++ PIwebVNC.cpp -lX11 -lXdamage -lXfixes -pthread -lXtst -llz4 -o PiWebVNC
# for test
./PiWebVNC
```

open http://localhost:8000 in browser<br>
or<br>
open http://--ip-of-raspberry-pi--:8000 from another pc browser



## Requirements
1. Linux os (Raspbian, DietPI, Ubuntu with desktop etc)
2. A desktop environment (LXDE, MATE, XFCE,LXQT etc).

## Performance
max CPU usage PI4 : 3.7% (10 FPS)
max RAM usage PI4 2GB model : 0.7%

## Features
* 30FPS at CPU load of 10%, RAM 15MB only.
* It only sends Damaged area of the screen. Works just like VNC.
* mouse controll (click, double click, right click , scroll , drag , hover)
* keyboard controll
* Support multiple client simultaneously. (max client configurable)
* Auto start using systemd (script in section 2.2)
* No external dependency. (no need to install VNC / NoVNC server)
* password authentication [not implemented : TODO]

![image](https://user-images.githubusercontent.com/49402826/163715482-ae7e166f-7ac2-4baa-a946-9770576c0bf5.png)

## Downloads
####    [not available for now]
It is always better to compile it in your PC. To compile this in your PC please check installation process (this will install some dependencies for compilation)
You can download pre-compiled app from links below
  1. link1 for ARMv6 - pi zero , pi zero w
  2. link2 for ARMv7 - pi 3 , pi 2w
  3. link3 for ARMv8 - pi 4
  4. link4 for x86 - any other pc with x86 cpu

#### `please note that, this software does not provide any security features. Only use for low risk projects in your local network. A secure version is under development.`
