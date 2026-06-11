# Changelog

## [0.0.0] — 2026-01-11

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
