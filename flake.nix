{
  description = "Dev Environment for Notification Daemon in Linux";

  inputs = {
    nixpkgs.url = "github:nixos/nixpkgs?ref=nixos-unstable";
  };

  outputs = {nixpkgs, ...}: let
    system = "x86_64-linux";

    pkgs = import nixpkgs {
      inherit system;
      config = {allowUnfree = true;};
    };
  in {
    packages.${system}.default = pkgs.stdenv.mkDerivation {
      name = "notificationdaemon";

      src = ./.;

      buildInputs = with pkgs; [
        glib
        libnotify
        gdb
        pkg-config
        gcc
      ];

      buildPhase = ''
        make build-dir
        make main
      '';

      installPhase = ''
        mkdir -p $out/bin
        cp build/watchdog.out $out/bin/watchdog
      '';
    };

    devShells.${system}.default = pkgs.mkShell {
      name = "notificationdaemon-dev-shell";

      packages = with pkgs; [
        gdb
        pkg-config
        gcc
        libnotify
        glib
        glib.dev
      ];

      env = {
        LD_LIBRARY_PATH = pkgs.lib.makeLibraryPath [
          pkgs.stdenv.cc.cc
          pkgs.libnotify
          pkgs.glib
        ];

        PKG_CONFIG_PATH = "${pkgs.libnotify}/lib/pkgconfig:${pkgs.glib}/lib/pkgconfig";

        CC = "gcc";
        CXX = "g++";
      };

      shellHook = ''
        echo "Entering devShell for ${system}";
        if command -v zsh; then
          exec zsh
        else
          exec bash
        fi
      '';
    };
  };
}
