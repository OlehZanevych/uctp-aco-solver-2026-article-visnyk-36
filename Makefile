CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra
TARGET = uctp_aco
GENERATOR = generate_data
SOLVER = solve_uctp
SRC = uctp_aco.cpp

all: $(TARGET) $(GENERATOR) $(SOLVER)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

$(GENERATOR): generate_data.cpp
	$(CXX) $(CXXFLAGS) -o $(GENERATOR) generate_data.cpp

$(SOLVER): solve_uctp.cpp
	$(CXX) $(CXXFLAGS) -o $(SOLVER) solve_uctp.cpp

clean:
	rm -f $(TARGET) $(GENERATOR) $(SOLVER) results.csv comparison_results.csv
	rm -f dataset_*

generate: $(GENERATOR)
	./$(GENERATOR)

solve_all: $(SOLVER) generate
	@echo "Solving all datasets..."
	./$(SOLVER) 1 20 50
	./$(SOLVER) 2 20 50
	./$(SOLVER) 3 20 50
	./$(SOLVER) 4 20 50
	./$(SOLVER) 5 20 50

run: $(TARGET)
	./$(TARGET)

experiment: $(TARGET)
	./$(TARGET) 30 20 50
	./$(TARGET) 50 20 100
	./$(TARGET) 70 30 150

.PHONY: all clean run experiment generate solve_all
