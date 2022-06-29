apt install -y libx11-dev libxdamage-dev libxfixes-dev libxtst-dev liblz4-dev g++

g++ PIwebVNC.cpp -lX11 -lXdamage -lXfixes -pthread -lXtst -llz4 -o /bin/PiWebVNC

echo "Compile Successful."

echo "Installing PiWebVNC...for autostart"
echo "[Unit]
Description=Remote desktop service (VNC)

[Service]
User="$(who|awk '{print $1}')"
ExecStart=/bin/PiWebVNC
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target" > /etc/systemd/system/PiWebVNC.service

sudo systemctl enable PiWebVNC.service
sudo systemctl daemon-reload
sudo systemctl start PiWebVNC.service