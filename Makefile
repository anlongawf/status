CXX = g++
CXXFLAGS = -std=c++11 -O3 -Wall

SRCDIR = src
INCDIR = include
BUILDDIR = build
TARGET = csmon

# Gắn danh sách file *.cpp vào
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(patsubst $(SRCDIR)/%.cpp, $(BUILDDIR)/%.o, $(SOURCES))

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.cpp
	@mkdir -p $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

clean:
	rm -rf $(BUILDDIR) $(TARGET)

install: $(TARGET)
	@echo "Installing csmon as 'status' command..."
	sudo cp $(TARGET) /usr/local/bin/status
	sudo chmod +x /usr/local/bin/status
	@echo "Done! You can now type 'status' anywhere to run the monitor."

uninstall:
	@echo "Removing 'status' command..."
	sudo rm -f /usr/local/bin/status
	@echo "Done!"

.PHONY: all clean install uninstall
