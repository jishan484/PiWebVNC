apt install -y libx11-dev libxdamage-dev libxfixes-dev libxtst-dev liblz4-dev g++

g++ PIwebVNC.cpp -lX11 -lXdamage -lXfixes -pthread -lXtst -llz4 -o PiWebVNC

sudo cp PiWebVNC /PiWebVNC

echo "Compile Successful"

echo "[Unit]
Description=Remote desktop service (VNC)
After=syslog.target network.target multi-user.target

[Service]
Type=simple
ExecStart=/PiWebVNC
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target" > /etc/systemd/system/PiWebVNC.service

systemctl enable PiWebVNC.service
systemctl daemon-reload
systemctl start PiWebVNC.service