.PHONY: all test clean install

all:
	cmake -S . -B build -DBUILD_TESTING=OFF
	cmake --build build
	cp build/slamhaus$(if $(filter Windows_NT,$(OS)),.exe,) .

test:
	cmake -S . -B build -DBUILD_TESTING=ON
	cmake --build build
	ctest --test-dir build --output-on-failure

install:
	cmake --install build

clean:
	cmake -E remove_directory build
	cmake -E rm -f slamhaus slamhaus.exe
