<p align="center">
  <img src="assets/banner.png" alt="PSGFX" width="100%">
</p>

<p align="center">
  <a href="../../releases/latest"><b>Download PSGFX for Windows</b></a>
</p>

PSGFX gives your PS5 homebrew and games new home-screen art. Pick an app, drop in any image, and click **Apply**. PSGFX resizes and converts it to the exact format the console expects, uploads it to the right folder, and keeps a backup of the original so you can undo it with one click.

It works through [PS5 Upload](https://github.com/phantomptr/ps5upload)'s payload, which is already running on your console. There's nothing to install on the PS5 or on your PC: PSGFX is a single `.exe`.

![Previewing a new icon and background before applying](docs/screenshot-preview.png)

## Features

- **Custom icons.** Use any PNG or JPEG. PSGFX crops it to a square and saves it at the console's native size.
- **Custom backgrounds.** Any 16:9 image becomes the backdrop behind the app on the home screen. Homebrew gets full 4K backgrounds.
- **Homebrew and installed games.** Homebrew apps registered with PS5 Upload show up with their icons, and so do your installed PS4 and PS5 games, with their real names.
- **Live preview.** A replica of the PS5 home screen shows your art before anything touches the console. Drop an image on the tile to set the icon, or anywhere else to set the background.
- **One-click restore.** The first time you change something, PSGFX saves the original next to it. **Restore original art** puts it back exactly.
- **Clean list.** System apps and leftovers are tucked away under *Show other items*.
- **No setup.** A single portable Windows app that opens in your browser. It remembers your PS5's IP address.

## Requirements

- A jailbroken PS5 with **[PS5 Upload](https://github.com/phantomptr/ps5upload)**'s payload running (the same setup you use to upload homebrew).
- A Windows PC on the same network.

## How to use

1. Download `PSGFX.exe` from the [latest release](../../releases/latest) and double-click it. PSGFX opens in your browser.
2. Enter your PS5's IP address (the same one PS5 Upload uses) and click **Connect**.
3. Choose a homebrew app or an installed game from the list.
4. Drop an image onto the preview's tile (**icon**) or background (**background**), or use the **Choose image** buttons.
5. Close that app or game on the PS5, then click **Apply to PS5**.
6. Restart the PS5 to see the new art on the home screen.

To undo, select the app and click **Restore original art**.

![Homebrew app with its current background](docs/screenshot-homebrew.png)

## Tips

- **Image sizes:** icons look best at 512 × 512 or larger. Backgrounds look best at 3840 × 2160. PSGFX crops and scales anything else automatically.
- **Game updates:** the PS5 may put back a game's original art when that game updates. Apply yours again afterwards.
- **Windows SmartScreen:** the app isn't code-signed, so Windows may warn the first time. Click **More info → Run anyway**. If the firewall asks, allow PSGFX on private networks.
- **Quitting:** close the black console window.

![Apply result](docs/screenshot-applied.png)

## Build from source

PSGFX is plain C++17 with no external dependencies. Everything it needs is in `src/lib`.

```bash
# Linux / WSL: produces build/PSGFX.exe
sudo apt install g++-mingw-w64-x86-64
./build.sh

# Native Linux binary for development
./build.sh linux
```

On Windows, run `./build.sh` from an [MSYS2](https://www.msys2.org/) MinGW shell. Pushing a tag such as `v1.0.0` builds the exe on GitHub Actions and attaches it to a release.

### How it works

PSGFX runs a small local web server (127.0.0.1 only) and opens your browser to it. It talks to PS5 Upload's payload over its FTX2 protocol on ports 9113 and 9114. It uses the payload's existing commands to list apps, read files, upload them (BLAKE3-verified), copy, and back up. Homebrew art is written to the app's `sce_sys` folder and to the home-screen metadata folder. Game art is written to `/user/appmeta/<title id>` in the same size and format as the original files. Backgrounds are encoded as BC7 DDS where the console uses DDS.

`tools/mock_ps5.py` is a stand-in for the payload that serves a folder on your computer as if it were a PS5. Use it to develop without a console:

```bash
python3 tools/mock_ps5.py ./fake-ps5 39114 39113 &
PS5AS_HOST=127.0.0.1 PS5AS_MGMT_PORT=39114 PS5AS_XFER_PORT=39113 ./build/psgfx
```

## Credits

- [PS5 Upload](https://github.com/phantomptr/ps5upload) by phantomptr, whose payload PSGFX talks to
- [bc7enc](https://github.com/richgel999/bc7enc) by Rich Geldreich, [stb](https://github.com/nothings/stb) by Sean Barrett, [BLAKE3](https://github.com/BLAKE3-team/BLAKE3), [JSON for Modern C++](https://github.com/nlohmann/json) by Niels Lohmann, and the [Sora](https://github.com/sora-xor/sora-font) typeface. See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

## License

[MIT](LICENSE).

PSGFX is an independent fan project. It is not affiliated with, endorsed by, or sponsored by Sony Interactive Entertainment. "PlayStation" and "PS5" are trademarks of Sony Interactive Entertainment Inc. Only use PSGFX with games you own.
