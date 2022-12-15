# 1> run nix-shell
# 2> mkdir build
# 3> cd build && cmake ..
# 4> ninja
{ pkgs ? import <nixpkgs> {} }:
  pkgs.mkShell {
    # nativeBuildInputs is usually what you want -- tools you need to run
    nativeBuildInputs = [
      pkgs.gcc11   # Works with previous versions
      pkgs.cmake
      pkgs.ninja
      pkgs.qtcreator
      pkgs.qt6Packages.qtbase
      pkgs.qt6Packages.qttools
    ];
}
