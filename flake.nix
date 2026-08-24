{
  description = "Cremniy IDE - Qt C++ Application";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    flake-utils.url = "github:numtide/flake-utils";
  };

  outputs = { self, nixpkgs, flake-utils }:
    flake-utils.lib.eachDefaultSystem (system:
      let
        pkgs = nixpkgs.legacyPackages.${system};
        
        # Qt dependencies
        qt6Packages = with pkgs.qt6; [
          qtbase
          qtsvg
          qttools
        ];
        
        # libgit2 dependencies
        libgit2Deps = with pkgs; [
          libgit2
          openssl
          libssh2
          zlib
          curl
          http-parser
          pcre2
        ];

      in
      {
        devShells.default = pkgs.mkShell {
          buildInputs = qt6Packages ++ libgit2Deps;

          nativeBuildInputs = with pkgs; [
            cmake
            ninja
            gcc
            pkg-config
            gdb
          ];

          shellHook = ''
            echo "Cremniy IDE development shell"
            echo "Available dependencies: cmake, ninja, gcc, qt6, libgit2"
            export CMAKE_PREFIX_PATH="${pkgs.qt6.qtbase}"
            export QT6_DIR="${pkgs.qt6.qtbase}"
            export QT_QPA_PLATFORM=xcb
          '';
        };
      });
}