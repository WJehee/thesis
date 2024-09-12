{
    description = "OBC Simulator";

    inputs = {
        nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
    };

    outputs = { self, nixpkgs }:
    let
        system = "x86_64-linux";
        pkgs = nixpkgs.legacyPackages.${system};
    in {
        devShells.${system}.default = with pkgs; mkShell {
            buildInputs = [
                clang-tools
                meson
                ninja
                pkg-config
                libpcap
                cmake
                glib
                zmqpp
                libyaml
                libbsd
                pcre2
                gdb
            ];
        };
    };
}
