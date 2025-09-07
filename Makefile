# Makefile for PIwebVNC

CXX = g++
CXXFLAGS = -Wall -Wextra -O2
LIBS = -lX11 -lXdamage -lXfixes -pthread -lXtst -llz4
SRC = PIwebVNC.cpp
OUT = PiWebVNC

# Default target
all: $(OUT)

$(OUT): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) $(LIBS) -o $(OUT)

check:
	@echo "Running compile check..."
	$(CXX) -fsyntax-only $(SRC) $(LIBS)
	@echo "✅ Compile check passed"

distcheck: all
	@echo "Distribution check: binary exists?"
	@if [ -f "$(OUT)" ]; then echo "✅ Binary built successfully"; else echo "❌ Build failed"; exit 1; fi

clean:
	rm -f $(OUT)

vfb:
	g++ PIwebVNC.cpp -Wl,--allow-multiple-definition lib_Xvfb_x86.a -L/usr/lib/x86_64-linux-gnu -Wl,-Bstatic -lX11 -lXdamage -lXfixes -lXtst -lXext -lxcb -lXau -lXdmcp -lXrender -llz4 -pthread -lXau -lcrypto -lpixman-1 -lpng16 -lfreetype -Wl,-Bdynamic -lXfont2 -o PiWebVNCx86

install:
	sudo apt install -y libx11-dev libxdamage-dev libxfixes-dev libxtst-dev liblz4-dev g++

