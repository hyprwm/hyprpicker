<p align="center">
  <img src="https://i.imgur.com/qggapUb.png" width="600" alt="Hyprpicker logo"/>
</p>

<p align="center">
  <img alt="Repo Size" src="https://img.shields.io/github/repo-size/r3b00thx/hyprpicker?style=for-the-badge&color=0181a4&labelColor=1B1B1B">
  <img alt="Language" src="https://img.shields.io/badge/Language-C++17-00D2FF?style=for-the-badge&color=0181a4&labelColor=1B1B1B">
  <img alt="License" src="https://img.shields.io/github/license/hyprwm/hyprpicker?style=for-the-badge&color=0181a4&labelColor=1B1B1B">
  <img alt="Wayland" src="https://img.shields.io/badge/Wayland-wlroots-00D2FF?style=for-the-badge&color=0181a4&labelColor=1B1B1B">
</p>

<p align="center"><b>A sleek, wlroots-native color picker for Wayland built for Hyprland with a focus on speed, accuracy, and simplicity.</b></p>

<p align="center">
  <img src="https://i.imgur.com/tTLCcOA.gif" alt="hyprpicker demo" width="600"/>
</p>



## ✨ Description

**Hyprpicker** is a **blazing-fast, minimal, and accurate color picker** built for **Wayland compositors** like **Hyprland**.  
It lets you instantly grab any color from your screen with pixel-perfect precision.

Hyprpicker is designed to *just work* with clipboard copying, format selection, zoom lens, and desktop notifications.



## 🚀 Usage

**Run it, click anywhere, get the color.**  
By default, the color is printed to stdout. Combine it with options like clipboard copy or notification for a smoother experience.

```sh
hyprpicker -a -n -b -fmt rgb
```



## 🧰 Command-Line Options

```sh
Hyprpicker usage: hyprpicker [arg [...]].

Arguments:
 -a | --autocopy            | Automatically copies the output to the clipboard (requires wl-clipboard)
 -f | --format=fmt          | Specifies the output format (cmyk, hex, rgb, hsl, hsv)
 -n | --notify              | Sends a desktop notification when a color is picked (requires notify-send and a notification daemon like dunst)
 -b | --no-fancy            | Disables the "fancy" (aka. colored) outputting
 -h | --help                | Show this help message
 -r | --render-inactive     | Render (freeze) inactive displays
 -z | --no-zoom             | Disable the zoom lens
 -q | --quiet               | Disable most logs (leaves errors)
 -v | --verbose             | Enable more logs
 -t | --no-fractional       | Disable fractional scaling support
 -d | --disable-preview     | Disable live preview of color
 -l | --lowercase-hex       | Outputs the hexcode in lowercase
 -V | --version             | Print version info
```



## 📦 Installation

```sh
sudo pacman -S hyprpicker
```



## 🔧 Build From Source

### Dependencies

- cmake
- pkg-config
- pango
- cairo
- wayland
- wayland-protocols
- hyprutils
- xkbcommon



### Build and install

```sh
cmake --no-warn-unused-cli -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr -S . -B ./build
cmake --build ./build --config Release --target hyprpicker -j$(nproc)
sudo cmake --install ./build
```



## 📚 License

Hyprpicker is licensed under the BSD 3-Clause License — a permissive license that allows nearly unrestricted use, including commercial, as long as proper credit is given.
