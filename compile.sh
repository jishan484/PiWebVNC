# check if running as sudo
if [ -z "$SUDO_USER" ]; then
  echo "[ERROR] Please run as sudo, even if you are root."
  echo "[INFO] Try: sudo sh compile.sh"
  echo "[EXIT] Exiting..."
  exit
fi

apt install -y libx11-dev libxdamage-dev libxfixes-dev libxtst-dev liblz4-dev g++

g++ PIwebVNC.cpp -lX11 -lXdamage -lXfixes -pthread -lXtst -llz4 -o /bin/PiWebVNC

echo "[INFO] Compile Successful."

echo "[INFO] Installing PiWebVNC...for autostart"

if [ -z "$SUDO_USER" ]; then
    PIwebVNC_USER=$USER
else
    PIwebVNC_USER=$SUDO_USER
fi

echo "[Unit]
Description=Remote desktop service (VNC)

[Service]
User="$PIwebVNC_USER"
ExecStart=/bin/PiWebVNC
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target" > /etc/systemd/system/PiWebVNC.service

sudo systemctl enable PiWebVNC.service
sudo systemctl daemon-reload
sudo systemctl start PiWebVNC.service