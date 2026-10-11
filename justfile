cxx := env("CXX", "c++")

default: build

build: c20

c11: (std "11")
c14: (std "14")
c17: (std "17")
c20: (std "20")

# Each example at its own minimum standard, plus tests/api.cpp at all four.
all:
    cmake -S . -B build/all -DCMAKE_CXX_COMPILER={{cxx}}
    cmake --build build/all -j
    ctest --test-dir build/all --output-on-failure

std version:
    cmake -S . -B build/c{{version}} -DCMAKE_CXX_COMPILER={{cxx}} -DRESULT_CXX_STANDARD={{version}}
    cmake --build build/c{{version}} -j
    ctest --test-dir build/c{{version}} --output-on-failure

docs:
    sh docs/build.sh

serve:
    sh docs/build.sh serve

clean:
    rm -rf build docs/site
