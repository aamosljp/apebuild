{ pkgs ? import <nixpkgs> { } }:
pkgs.mkShell
{
  nativeBuildInputs = with pkgs; [
    nixpkgs-fmt
    gcc
    clang-tools
  ];
  shellHook = ''
    					export PATH="${pkgs.clang-tools}/bin:$PATH"
    					'';
}
