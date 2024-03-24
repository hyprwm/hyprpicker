{
  description = "Hyprpicker - a wlroots-compatible Wayland color picker that does not suck";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    systems.url = "github:nix-systems/default-linux";
  };

  outputs = {
    self,
    nixpkgs,
    systems,
    ...
  }: let
    inherit (nixpkgs) lib;
    eachSystem = lib.genAttrs (import systems);
    pkgsFor = eachSystem (system:
      import nixpkgs {
        localSystem.system = system;
        overlays = with self.overlays; [hyprpicker];
      });
    mkDate = longDate: (lib.concatStringsSep "-" [
      (builtins.substring 0 4 longDate)
      (builtins.substring 4 2 longDate)
      (builtins.substring 6 2 longDate)
    ]);
  in {
    overlays = {
      default = self.overlays.hyprpicker;
      hyprpicker = lib.composeManyExtensions [
        self.overlays.overrides
        (final: prev: rec {
          hyprpicker = final.callPackage ./nix/default.nix {
            stdenv = final.gcc13Stdenv;
            version = "0.pre" + "+date=" + (mkDate (self.lastModifiedDate or "19700101")) + "_" + (self.shortRev or "dirty");
            inherit (prev.xorg) libXdmcp;
          };
          hyprpicker-debug = hyprpicker.override {debug = true;};
        })
      ];
      overrides = final: prev: {
        wayland-protocols = prev.wayland-protocols.overrideAttrs (self: super: {
          version = "1.34";
          src = prev.fetchurl {
            url = "https://gitlab.freedesktop.org/wayland/${self.pname}/-/releases/${self.version}/downloads/${self.pname}-${self.version}.tar.xz";
            hash = "sha256-xZsnys2F9guvTuX4DfXA0Vdg6taiQysAq34uBXTcr+s=";
          };
        });
      };
    };

    packages = eachSystem (system: {
      default = self.packages.${system}.hyprpicker;
      inherit (pkgsFor.${system}) hyprpicker hyprpicker-debug;
    });

    formatter = eachSystem (system: pkgsFor.${system}.alejandra);
  };
}
