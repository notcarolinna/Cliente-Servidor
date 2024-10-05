CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11
SRC_DIR = srcs

BIN_DIR = bin

CLIENT_TCP = $(BIN_DIR)/client_tcp
CLIENT_UDP = $(BIN_DIR)/client_udp
SERVER_TCP = $(BIN_DIR)/server_tcp
SERVER_UDP = $(BIN_DIR)/server_udp

SRC_CLIENT_TCP = $(SRC_DIR)/ClientTCP.cpp
SRC_CLIENT_UDP = $(SRC_DIR)/ClientUDP.cpp
SRC_SERVER_TCP = $(SRC_DIR)/ServerTCP.cpp
SRC_SERVER_UDP = $(SRC_DIR)/ServerUDP.cpp

all: $(CLIENT_TCP) $(CLIENT_UDP) $(SERVER_TCP) $(SERVER_UDP)

$(CLIENT_TCP): $(SRC_CLIENT_TCP)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(CLIENT_UDP): $(SRC_CLIENT_UDP)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(SERVER_TCP): $(SRC_SERVER_TCP)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(SERVER_UDP): $(SRC_SERVER_UDP)
	@mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -rf $(BIN_DIR)

.PHONY: all clean
