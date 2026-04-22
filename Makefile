##
## Project: Raytracer
## File name: Makefile
## Author: Cannelle Gourdet - lankley
## File description: Builds the raytracer binary and its shared plugin libraries.
##

CXX	:=	g++
INCLUDE_DIRS	:=	$(shell find include/ -type d)

CXXFLAGS	:=	-Wall -Wextra -Werror -std=c++20
CXXFLAGS	+=	$(addprefix -I,$(INCLUDE_DIRS))
CXXFLAGS	+=	-fPIC

LDFLAGS	:=	-lconfig++ -ldl

NAME	:=	raytracer
TEST_BIN	:=	unit_tests

SRC	:=	$(shell find src/ -name "*.cpp" ! -path "src/plugins/*" 2>/dev/null)
OBJ	:=	$(SRC:.cpp=.o)

PLUGIN_SRCS	:=	$(wildcard src/plugins/*.cpp)
PLUGINS	:=	$(patsubst src/plugins/%.cpp,plugins/%.so,$(PLUGIN_SRCS))

TEST_DIR	:=	tests/unit_tests
TEST_SRCS	:=	$(wildcard $(TEST_DIR)/*.cpp)
TEST_OBJ	:=	$(TEST_SRCS:.cpp=.o)

MAIN_OBJ	:=	src/main.o
LIB_OBJ	:=	$(filter-out $(MAIN_OBJ),$(OBJ))

SCENE	?=	scenes/demo_sphere.cfg

_COV_FLAGS	:=	--coverage -fprofile-arcs -ftest-coverage

VALGRIND_FLAGS	:=	--tool=memcheck \
		   --leak-check=full \
		   --show-leak-kinds=all \
		   --track-origins=yes \
		   --track-fds=yes \
		   --undef-value-errors=yes \
		   --num-callers=30 \
		   --error-exitcode=1 \
		   --verbose


all: $(NAME) $(PLUGINS)

$(NAME): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(TEST_DIR)/%.o: $(TEST_DIR)/%.cpp
	$(CXX) $(filter-out -Werror,$(CXXFLAGS)) -c $< -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@


plugins/%.so: src/plugins/%.cpp
	@mkdir -p plugins
	$(CXX) $(CXXFLAGS) -shared -o $@ $^ $(LDFLAGS)


unit_tests: $(LIB_OBJ) $(TEST_OBJ)
	$(CXX) $(filter-out -Werror,$(CXXFLAGS)) -o $(TEST_BIN) $^ $(LDFLAGS) -lcriterion
	./$(TEST_BIN)


func_tests: all
	@bash tests/functional_tests.sh


tests: unit_tests func_tests


coverage: fclean
	$(MAKE) _run_coverage

_run_coverage: CXXFLAGS := $(filter-out -Werror,$(CXXFLAGS)) $(_COV_FLAGS)
_run_coverage: LDFLAGS  := $(LDFLAGS) $(_COV_FLAGS)
_run_coverage: $(LIB_OBJ) $(TEST_OBJ)
	$(CXX) $(CXXFLAGS) -o $(TEST_BIN) $^ $(LDFLAGS) -lcriterion
	./$(TEST_BIN)
	lcov --capture --directory . --output-file coverage.info \
		--ignore-errors mismatch \
		--exclude '/usr/*' \
		--exclude '*/tests/*'
	genhtml coverage.info --output-directory coverage_html
	@echo ""
	@echo "Coverage report: coverage_html/index.html"


memcheck: all
	valgrind $(VALGRIND_FLAGS) ./$(NAME) $(SCENE)


clean:
	$(RM) $(OBJ) $(TEST_OBJ)
	$(RM) -f $(shell find . -name "*.gcno" -o -name "*.gcda" -o -name "*.gcov" 2>/dev/null)
	$(RM) -f coverage.info
	$(RM) -rf coverage_html/

fclean: clean
	$(RM) $(NAME) $(TEST_BIN)
	$(RM) -f $(PLUGINS)

re: fclean all


install:
	sudo apt-get update
	sudo apt-get install -y \
		libconfig++-dev \
		valgrind \
		lcov \
		libsfml-dev \
		cmake \
		ninja-build
	@echo ""
	@echo "Checking for Criterion..."
	@if pkg-config --exists criterion 2>/dev/null; then \
		echo "Criterion already installed."; \
	else \
		echo "Installing Criterion from source..."; \
		git clone --depth 1 https://github.com/Snaipe/Criterion /tmp/criterion_src; \
		cmake -S /tmp/criterion_src -B /tmp/criterion_src/build \
			-DCMAKE_BUILD_TYPE=Release \
			-GNinja; \
		ninja -C /tmp/criterion_src/build; \
		sudo ninja -C /tmp/criterion_src/build install; \
		sudo ldconfig; \
		echo "Criterion installed."; \
	fi

.PHONY: all re clean fclean \
	unit_tests func_tests tests coverage \
	memcheck install
