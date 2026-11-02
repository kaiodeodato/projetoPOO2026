CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11 -Iinclude

FONTES = src/main.cpp src/Dispositivo.cpp src/User.cpp src/Sensor.cpp src/Motor.cpp src/Fabrica.cpp

fabrica: $(FONTES)
	$(CXX) $(CXXFLAGS) -o fabrica $(FONTES)

clean:
	del /Q fabrica.exe
