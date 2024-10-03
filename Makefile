name=mandelbrot

# sanitize=-fsanitize=address,undefined,leak -fno-omit-frame-pointer
# sanitize=-fsanitize=thread

warnings=-Wall -Wextra -Wpedantic
# defines=-DDEBUG_OUTPUT
# defines=-DNDEBUG

CFLAGS=$(warnings) $(sanitize) $(defines) -I./src/ -MMD -g3 -O3
LDFLAGS=-L/usr/local/lib/ -Wl,-rpath,/usr/local/lib/ $(sanitize)
LDLIBS=-lGL -lglfw -lglad

sources=$(wildcard src/*.c) $(wildcard src/*/*.c)
headers=$(wildcard src/*.h) $(wildcard src/*/*.h)
objects=$(patsubst src/%,build/%,$(sources:.c=.o))
depends=$(objects:.o=.d)

build_dirs=out/ build/
dirs=$(filter-out build/,$(sort $(dir $(objects))))

###############################################################################

.PHONY:
all: $(build_dirs) $(dirs) out/$(name)

.PHONY:
clean:
	-rm -r $(filter-out build/ext/,$(dirs))
	-rm build/main.o build/main.d

.PHONY:
realclean:
	-rm -r build/

.PHONY:
install:
	mkdir -p ~/.local/bin/
	install -vm 0755 out/$(name) ~/.local/bin/$(name)
	mkdir -p ~/.local/share/$(name)/
	@for item in $$(find data -type f); \
		do install -Dvm 0755 $${item} ~/.local/share/$(name)/$${item#data/}; \
	done

.PHONY:
uninstall:
	-rm ~/.local/bin/$(name)
	-rm -r ~/.local/share/$(name)/

%/:
	mkdir -p $@

out/$(name): $(objects)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

-include $(depends)

build/%.o: src/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@  -c $<

###############################################################################

.PHONY:
memcheck:
	@echo ---
	valgrind --tool=memcheck --show-leak-kinds=all out/$(name)

.PHONY:
cachegrind:
	@echo ---
	valgrind --tool=cachegrind out/$(name)

.PHONY:
callgrind:
	@echo ---
	valgrind --tool=callgrind out/$(name)
