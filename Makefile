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
