# Define the compiler
CXX = g++

# Define the compiler flags
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic

# Define the object files
OBJECTS = client_logger_tests.o client_logger_builder.o client_logger.o logger_builder.o logger.o

# Define the executable
EXECUTABLE = client_logger_tests

# Define the directories
SRCDIR = ../src
LOGGERDIR = ../../logger/src

# Define the source files
SRCS = $(SRCDIR)/client_logger_builder.cpp $(SRCDIR)/client_logger.cpp $(LOGGERDIR)/logger_builder.cpp $(LOGGERDIR)/logger.cpp client_logger_tests.cpp

# Define the include directories
INCLUDES = -I$(SRCDIR) -I$(LOGGERDIR)

# Define the default target
all: $(EXECUTABLE)

# Define the executable target
$(EXECUTABLE): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# Define the object file targets
client_logger_tests.o: client_logger_tests.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

client_logger_builder.o: $(SRCDIR)/client_logger_builder.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

client_logger.o: $(SRCDIR)/client_logger.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

logger_builder.o: $(LOGGERDIR)/logger_builder.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

logger.o: $(LOGGERDIR)/logger.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Define the clean target
clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

# Include dependency files
-include $(OBJECTS:.o=.d)
