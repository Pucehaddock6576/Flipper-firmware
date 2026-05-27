# Changelog

---

### Added
- Protocol name allowlist filter: in Receiver Config, a new "Proto Filter"
  field accepts a comma-separated list of protocol names (e.g. "Ford V2,VAG").
  When set, the receiver ignores all decoded signals that are not in the list,
  reducing RAM usage and increasing the chance of capturing the target protocol.
  Leave empty to disable (default behavior, all protocols accepted).
  Setting is persisted in last_subghz.settings under the ProtocolFilter key.

### Changed
- Protocol Filter: replaced free-text input with a dedicated protocol list
  scene (Proto Filter in Receiver Config). All registered protocols are shown
  as toggleable items (--- / ONLY). Selecting one or more protocols restricts
  the receiver to only show those; leaving all as --- disables the filter.
  The active count is shown inline in Receiver Config ("N set" or "All").
  Filter is persisted across sessions and cleared by Reset to default.
