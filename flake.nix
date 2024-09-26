{
  description = "Hyprpicker - a wlroots-compatible Wayland color picker that does not suck";

  inputs = {
    nixpkgs.url = "github:NixOS/nixpkgs/nixos-unstable";
    systems.url = "github:nix-systems/default-linux";

    hyprutils = {
      url = "github:hyprwm/hyprutils";
      inputs.nixpkgs.follows = "nixpkgs";
      inputs.systems.follows = "systems";
    };

    hyprwayland-scanner = {
      url = "github:hyprwm/hyprwayland-scanner";
      inputs.nixpkgs.follows = "nixpkgs";
      inputs.systems.follows = "systems";
    };
  };

  outputs = {
    self,
    nixpkgs,
    systems,
    ...
  } @ inputs: let
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
    version = lib.removeSuffix "\n" (builtins.readFile ./VERSION);
  in {
    overlays = {
      default = self.overlays.hyprpicker;
      hyprpicker = lib.composeManyExtensions [
        inputs.hyprutils.overlays.default
        inputs.hyprwayland-scanner.overlays.default
        (final: prev: {
          hyprpicker = prev.callPackage ./nix/default.nix {
            stdenv = prev.gcc13Stdenv;
            version = version + "+date=" + (mkDate (self.lastModifiedDate or "19700101")) + "_" + (self.shortRev or "dirty");
          };
          hyprpicker-debug = final.hyprpicker.override {debug = true;};
        })
      ];
    };

    packages = eachSystem (system: {
      default = self.packages.${system}.hyprpicker;
      inherit (pkgsFor.${system}) hyprpicker hyprpicker-debug;
    });

    formatter = eachSystem (system: pkgsFor.${system}.alejandra);
  };
}
