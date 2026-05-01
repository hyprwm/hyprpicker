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

  outputs =
    {
      self,
      nixpkgs,
      systems,
      ...
    }@inputs:
    let
      inherit (nixpkgs) lib;
      eachSystem = lib.genAttrs (import systems);
      pkgsFor = eachSystem (
        system:
        import nixpkgs {
          localSystem.system = system;
          overlays = with self.overlays; [ hyprpicker-with-deps ];
        }
      );
    in
    {
      overlays = import ./nix/overlays.nix { inherit inputs lib self; };

      packages = eachSystem (system: {
        default = self.packages.${system}.hyprpicker;
        inherit (pkgsFor.${system}) hyprpicker hyprpicker-debug;
      });

      formatter = eachSystem (system: pkgsFor.${system}.nixfmt-tree);
    };
}
