# PiWebVNC
A simple, highly efficient web baseed VNC app for Raspberry pi (all models).

max CPU usage PI4 : 3.7% (10 FPS)
max RAM usage PI4 2GB model : 0.7%

* 30FPS at CPU load of 10%, RAM 15MB only.
* It only sends Damaged area of the screen. Works just like VNC.
* mouse controll (click, double click, right click , scroll , drag , hover)
* keyboard controll
* Support multiple client simultaneously. (max client configurable)
* Auto start using systemd (script in section 2.2)
* No external dependency. (no need to install VNC / NoVNC server)
* password authentication [not implemented : TODO]

![image](https://user-images.githubusercontent.com/49402826/163715482-ae7e166f-7ac2-4baa-a946-9770576c0bf5.png)


#requirements
1. Linux os (Raspbian, DietPI, Ubuntu with desktop etc)
2. A desktop environment (LXDE, MATE, XFCE,LXQT etc).

a. To compile this in your PC run 'install.sh' (this will install some dependencies for compilation)
b. or you can download pre-compiled app from links below
  1. link1 for ARMv6 - pi zero , pi zero w
  2. link2 for ARMv7 - pi 3 , pi 2w
  3. link3 for ARMv8 - pi 4
  4. link4 for x86 - any other pc with x86 cpu

`please note that, this software does not provide any security features. Only use for low risk projects in your local network. A secure version is under development.`
