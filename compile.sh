apt install -y libx11-dev libxdamage-dev libxfixes-dev libxtst-dev liblz4-dev g++

g++ PIwebVNC.cpp -lX11 -lXdamage -lXfixes -pthread -lXtst -lXext -llz4 -o PiWebVNC

echo "Compile Successful."

echo "Running this app. using ./PiWebVNC"

./PiWebVNC
