# Multi-extruder process settings

## Problem addressed

The process-page synchronization control assumes a pair of extruders. Its eligibility check rejects other tool counts, and its destination is calculated as `1 - source`. Identical toolhead variants can also leave the process selector disabled. This prevents a toolchanger from copying process settings to its other tools.

The copy operation also considers only settings that are dirty relative to the saved preset. A source tool's saved first-layer, first-layer infill, or outer-wall speed can therefore differ from the destination without appearing in the copy operation.

## Copy behavior

On a Process page, the copy control offers compatible destinations from the printer's configured extruder count. There is no fixed two-, four-, or twelve-tool limit. The source tool and inactive trailing profile slots are excluded. Existing drive-type and nozzle-volume compatibility rules apply to each destination, including hybrid nozzle variants. Two-tool printers retain their existing variant-selector policy; process tabs on printers with more than two tools also work with identical hotend variants.

For more than two tools, an Orca-styled selection dialog chooses the destination. The existing confirmation dialog identifies both tools. Only differing per-tool process values on the current page are copied, including saved and default values. Matching values are a no-op. Printer parameters, filament parameters, routing metadata and parameters from other pages are excluded. Printer and filament synchronization retain their existing eligibility and dirty-value behavior.

The UI labels tools starting at **T1**. These correspond to zero-based G-code tool indices: UI T1 is G-code T0.

## Process columns

`DynamicPrintConfig::ensure_process_variant_columns` creates process column metadata from the printer's variant layout when a process profile has only the default column. It extends shorter process vectors before the GUI edits them, retaining existing values and hybrid columns. `print_extruder_id` and `print_extruder_variant` remain routing metadata and are never copied as parameter values.

`ProcessSettingsSync.hpp` contains the page-value copy and destination eligibility calculations. These functions do not construct windows and are exercised by the native configuration tests.

## Shared process option

**Process (Global) → Others → Tool settings → Share print settings across tools** is available in Advanced and Expert mode. It is saved in the process preset as `share_process_settings` and defaults to false.

When enabled, the first process column supplies all tools' process parameters during slicing. Per-tool process switching and copying are disabled while sharing is enabled. Independent process values remain in the editable profile; turning sharing off restores their use. Object, part and layer process overrides use their first process column while sharing is enabled.

The sharing operation applies to a slicing copy of the configuration. Printer settings, filament temperatures, nozzle definitions, retraction, filament routing and physical tool mapping remain independent. Changing the option invalidates slicing so an existing result cannot retain the previous process-column policy.

## Regression coverage

Native tests cover:

- Every source/destination pair for 2, 6, 8 and 12 tools, including actual process-column lookup and first-layer, first-layer infill and outer-wall speeds.
- Compatible destinations, hybrid variants, differing drive types, inactive trailing slots, incomplete arrays and invalid source indices.
- Saved/default process values, unchanged source and other tools, exclusion of other pages and hardware/filament/routing settings, and repeat-copy no-ops.
- Sharing disabled by default, retained independent values, process-only sharing and four-tool filament routing when sharing is enabled and disabled.

Run the focused tests from a configured Release build:

```sh
cmake --build <build-dir> --config Release --target libslic3r_tests fff_print_tests
<build-dir>/tests/libslic3r/Release/libslic3r_tests "[ProcessSync],[SharedProcess]"
<build-dir>/tests/fff_print/Release/fff_print_tests "[SharedProcess]"
```

Executable locations vary with the generator and platform.

## Validation record

The change is based on revision `4a4128176110b73e1bbc87f66606d4834b50e160`.

- Windows x64, MSVC, Release, with precompiled headers disabled: the native test build passes and the changed GUI translation units compile.
- Configuration tests (`[Config]`): 111 cases and 11,170 assertions pass.
- Shared-process printing test (`[SharedProcess]`): one case and 19 assertions pass, including retained four-tool routing and restoration of independent speeds.
- Temporarily restoring the two-tool restriction and omission of saved values makes both targeted regression cases fail. The audited source is restored, rebuilt and passes again.
- The sanitized patch applies to a clean snapshot of the base revision.

The full GUI application was not linked or manually exercised for this exact branch. macOS/Linux validation is pending. The required clang-tidy 22.1.8 analysis was attempted but exits unexpectedly in the preparation environment; it must be completed in CI or another development environment before upstream review.

## Manual verification before upstream review

1. Use a disposable multi-extruder profile or a copy of an existing project. In the unmodified application, reproduce the unavailable copy control or an omitted saved speed value.
2. Build this branch. On the Speed page, give two tools different first-layer, first-layer infill and outer-wall speeds. Save the preset, select the source and copy to the destination. Confirm the saved values are listed and copied.
3. Repeat in the opposite direction. Cancel a copy and confirm no values change. Test a synthetic six-, eight- or twelve-tool profile to inspect the destination list and access the final tool tab.
4. Check that incompatible destinations are excluded and the printer/filament settings remain unchanged.
5. Enable sharing, slice a multi-tool project, then disable sharing and slice again. Confirm shared versus independent process speeds, with unchanged filament/tool routing.
6. Check light/dark mode and display scaling. Capture screenshots using generic test profiles, excluding account information, device addresses and local paths.

Automated configuration tests do not replace these interactive checks. Current-branch manual UI verification and macOS/Linux validation remain to be completed and must be recorded before claiming them in an upstream PR.
