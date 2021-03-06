![logos](https://user-images.githubusercontent.com/49402826/176439822-57f3a7ad-e21b-4a8f-a25a-b8e7a2c084f5.png)


# PiWebVNC
A simple, highly efficient, web based VNC app for Raspberry pi (all models).
###### No external dependency required. Just `compile -> run -> go`.
## Installation process
To compile this app, some aditional packages will be installed in your pi. You can also use 
```sh
git clone https://github.com/jishan484/PiWebVNC.git
cd PiWebVNC
sudo sh compile.sh
```
### or
#### setup after downloading Release zip
```sh
git clone https://github.com/jishan484/PiWebVNC.git
cd PiWebVNC
sudo apt install -y libx11-dev libxdamage-dev libxfixes-dev libxtst-dev liblz4-dev g++
```
#### Compile and Run
```sh
g++ PIwebVNC.cpp -lX11 -lXdamage -lXfixes -pthread -lXtst -llz4 -o /bin/PiWebVNC
# for test
/bin/PiWebVNC
```
Using /bin (linux binary dir) for autostart setup (changed to bin)
open `http://localhost:8000` in browser<br>
or<br>
open `http://--ip-of-raspberry-pi--:8000` from another pc browser E.g. `http://192.168.0.15:8000`

#### Configure
Open libs/appConfigs.hpp to configure this app, like PORT , MAX clients etc.
HTTPS or WSS configuration will be available soon [#5_issue](/../../issues/5)

### Auto start
Use this auto-start method to run this app at startup
###### Note : You can use different methods also.
###### Open Nano or VIM
```sh
sudo nano /etc/systemd/system/PiWebVNC.service
```
###### Paste below section there
```sh
[Unit]
Description=Remote desktop service (PiWebVNC)

[Service]
User="$(who|awk '{print $1}')"
ExecStart=/bin/PiWebVNC
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```
```sh
sudo systemctl enable PiWebVNC.service
sudo systemctl daemon-reload
sudo systemctl start PiWebVNC.service
```

## Requirements
1. Linux os (Raspbian, TwisterOS, DietPI, Ubuntu with desktop etc)
2. A desktop environment (LXDE, MATE, XFCE,LXQT etc).

## Performance
max CPU usage PI4 : `3.7%` (10 FPS)<br>
max RAM usage PI4 2GB model : `0.7%`

## Features
* 30FPS at max CPU load of 10%, RAM 15MB only. CPU load will be 0% when screen is not changing. 
* It only sends Damaged area of the screen. Works just like VNC.
* mouse controll (click, double click, right click , scroll , drag , hover)
* keyboard controll
* Support multiple client simultaneously. (max client configurable)
* Auto start using systemd (script in section [Auto start](#auto-start))
* No external dependency. (no need to install VNC / NoVNC server)
* Clipboard support. [text only]
* Easily transfer Files / Folders from web client. [not implemented : TODO]
* password authentication. [not implemented : TODO]

![Image0101](https://user-images.githubusercontent.com/49402826/171990825-30321f79-c50e-4b03-aac5-eb9c029d7f3b.png)


## Pre compiled Binaries
####    [Not available for Any device for platform library dependency]
Please compile to ommit platform dependency. It will install all required dependencies before compiling the app.
It is always better to compile it in your PC. To compile this in your PC please check [Installation process](#installation-process) (this will install some dependencies in your Pi)
You can download pre-compiled app from links below
  NOT AVAILABLE. PLEASE COMPILE IT USING `sudo sh compile.sh` SCRIPT. [Must have to use sudo]

#### `please note that, this software does not provide any security features. Only use for low risk projects in your local network. A secure version is under development.`
