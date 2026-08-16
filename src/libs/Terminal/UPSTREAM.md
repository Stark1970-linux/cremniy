# Terminal core provenance

The terminal emulator is derived from Qt Creator's standalone `TerminalLib`
and its vendored libvterm 0.3.3 at commit
`b815fca1aa9ab2332de8968609799290e9cd73cf`.

- Qt Creator terminal sources: GPL-3.0 with the Qt GPL exception, as stated in
  each source file.
- libvterm: MIT; see `third_party/libvterm/LICENSE`.

The code is kept as a small in-tree library so Cremniy builds reproducibly on
Windows, Linux, and macOS without a network connection during configuration.
