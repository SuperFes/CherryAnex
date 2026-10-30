# anex-host

Sends the time and CPU/RAM usage to the Anex keyboard's displays once a second
over VIA's raw HID interface. The keyboard falls back to its normal displays a
few seconds after the app stops. VIA can stay open while it runs.

Binaries for each OS are attached to the repo's `latest` release. To build
locally: `cargo build --release` (Linux needs `libudev` headers).

## Linux

The udev rule that gives VIA access (`/etc/udev/rules.d/60-via.rules`) also
covers this app.

```sh
install -Dm755 anex-host-linux-x86_64 ~/.local/bin/anex-host
install -Dm644 autostart/anex-host.service ~/.config/systemd/user/anex-host.service
systemctl --user enable --now anex-host
```

## macOS

The binary isn't signed, so clear the download quarantine first.

```sh
xattr -d com.apple.quarantine anex-host-macos-universal
sudo install -m755 anex-host-macos-universal /usr/local/bin/anex-host
cp autostart/com.cosmos.anex-host.plist ~/Library/LaunchAgents/
launchctl load ~/Library/LaunchAgents/com.cosmos.anex-host.plist
```

## Windows

The Windows build runs without a console window. Copy it somewhere permanent,
then start it at login:

```bat
reg add HKCU\Software\Microsoft\Windows\CurrentVersion\Run /v anex-host /d "%LOCALAPPDATA%\anex-host\anex-host-windows-x86_64.exe" /f
```
