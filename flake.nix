{
  description = "Rust Result type in C++";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
      in
      {
        devShells.default = pkgs.mkShell {
          buildInputs = with pkgs; [
            cmake
            gcc
            ninja
          ];

          shellHook = ''
            echo "Development environment loaded."
            echo "CMake version: $(cmake --version | head -n1)"
            echo "GCC version: $(gcc --version | head -n1)"
            echo "ninja version: $(ninja --version)"
            echo ""
            echo "Build and test the examples:"
            echo "  cmake -S . -B build"
            echo "  cmake --build build"
            echo "  ctest --test-dir build --output-on-failure"
          '';
        };
      }
    );
}
