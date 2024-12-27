{
	inputs = {
		nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
		flake-compat = {
			url = "github:edolstra/flake-compat";
			flake = false;
		};
	};

	outputs = { self, nixpkgs, flake-compat }:

		let
			pkgs = nixpkgs.legacyPackages.x86_64-linux;
		in {
			devShell.x86_64-linux = pkgs.mkShell {
				buildInputs = with pkgs; [
					nixpkgs-fmt
					gcc
					clang-tools
				];
				shellHook = ''
					export PATH="${pkgs.clang-tools}/bin:$PATH"
					'';
			};
		};
}
