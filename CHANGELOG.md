# Changelog

## [0.2.3] - 2026-09-28

### Changed
- -Wconversion and -Wsign-conversion are on for gcc and clang. sn_file_size,
  sn_file_stat and the copy loop in sn_file_copy left their widening from the
  stat fields and the read result to implicit conversions. The win32 side already
  cast these explicitly, so the nix side now matches it

## [0.2.2] - 2026-09-28

### Fixed
- sn_file_open() took an int in the header but an SnFileOpenFlag in the win32
  implementation, which do not match. A definition whose parameter type differs
  from its declaration is an error, so the Windows build could not have compiled
  this. The parameter is an SnFileOpenFlag everywhere now, which also matches
  how the rest of the API takes its enums

## [0.2.1] - 2026-09-28

### Changed
- Take sncore v0.3.1 rather than v0.2.0

## [0.2.0] - 2026-06-29

### Changed
- Updated the dependency versions

## [0.1.0] - 2026-06-11

- First release. See [0.0.0] section in CHANGELOG.md for full changelog.

## [0.0.0] - 2026-01-11

### Added
- Cross-platform file I/O (`sn_file_open`, `sn_file_close`, `sn_file_read`, `sn_file_write`)
- File positioning (`sn_file_seek`, `sn_file_tell`, `sn_file_size`)
- File system operations (`sn_file_copy`, `sn_file_move`, `sn_file_remove`, `sn_file_stat`)
- Directory operations (`sn_file_mkdir`, `sn_file_rmdir`)
- Path utilities (`sn_path_join`, `sn_path_normalize`)
- POSIX backend (POSIX I/O + `copy_file_range` / `sendfile`)
- Windows backend (`CreateFileW` / `GetFileInformationByHandle` / `CopyFileW`)
- SnCore dependency for platform detection and string utilities
- CI workflows (Linux, macOS, Windows, formatting)
