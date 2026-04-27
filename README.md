<div align="center">
# noctctl

*IPC client for the noctis Wayland compositor.*

<br/>

[![GitHub Stars](https://img.shields.io/github/stars/noxwm/noctctl?style=for-the-badge&logo=github&logoColor=D9E0EE&labelColor=252733&color=AB6C6A)](https://github.com/noxwm/noctctl/stargazers)
[![License](https://img.shields.io/badge/license-MIT-blue?style=for-the-badge&logoColor=D9E0EE&labelColor=252733&color=AB6C6A)](LICENSE)
[![Works With](https://img.shields.io/badge/works%20with-noctis-orange?style=for-the-badge&logoColor=D9E0EE&labelColor=252733&color=AB6C6A)](https://github.com/noxwm/noctis)

</div>


## 📦 Install (Arch Linux)

```bash
git clone https://github.com/noxwm/noctctl
cd noctctl
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
sudo cp build/noctctl /usr/local/bin/
```

## ⚡ Usage

```bash
noctctl <command> [args]
```


## 📋 Commands

|Command                        |Description                 |
|:------------------------------|:---------------------------|
|`noctctl reload`               |Reload config file live     |
|`noctctl kill`                 |Close focused window        |
|`noctctl exit`                 |Exit the compositor         |
|`noctctl focus next`           |Focus next window           |
|`noctctl focus prev`           |Focus previous window       |
|`noctctl exec <cmd>`           |Run any command             |
|`noctctl gap <px>`             |Set gap size on the fly     |
|`noctctl border active <hex>`  |Change active border color  |
|`noctctl border inactive <hex>`|Change inactive border color|
|`noctctl ratio <float>`        |Set master pane ratio       |


## 🔧 Examples

```bash
# Reload config after editing it
noctctl reload

# Launch apps
noctctl exec firefox
noctctl exec "kitty --class floating"

# Adjust layout on the fly
noctctl gap 10
noctctl ratio 0.6

# Change border colors without restarting
noctctl border active "#AB6C6A"
noctctl border inactive "#333333"

# Use in shell scripts or keybinds
noctctl exec "grim -g '$(slurp)'"
```

## 🔌 How It Works

noctctl communicates with noctis over a Unix socket at:

```
$XDG_RUNTIME_DIR/noctis.sock
```

noctis must be running for any command to work. Commands are sent as plain text and noctis responds with `ok` or an error message.


## 🔗 Related

- [noctis](https://github.com/noxwm/noctis) — the compositor itself
