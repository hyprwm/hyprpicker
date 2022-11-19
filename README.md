# hyprpicker

A wlroots-compatible Wayland color picker that does not suck.

![hyprpickerShort](https://user-images.githubusercontent.com/43317083/188224867-7d77a3b3-0a66-488c-8019-39b00060ab42.gif)

# Usage

Launch it. Click. That's it.

## Options

`-f | --format=[fmt]` specifies the output format (`cmyk`, `hex`, `rgb`, `hsl`, `hsv`)

`-n | --no-fancy` disables the "fancy" (aka. colored) outputting

`-h | --help` prints a help message

`--autocopy` automatically copies the output to the clipboard (requires [wl-clipboard](https://github.com/bugaevc/wl-clipboard))

# Building

## Arch
`yay -S hyprpicker-git`

## Manual
`make all`

the output binary is in `./build/hyprpicker`

# Caveats

"Freezes" your displays when picking the color.
