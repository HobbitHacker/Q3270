# 3270 Emulator Backlog

_A living list of parked ideas, imminent work, and completed items for the Qt‑based 3270 emulator refactor._

---

## 🅿️ Parked / Deferred

### Architectural
- **SessionDialogBase: store `Session` objects in `UserRole`**
  - Add `Q_DECLARE_METATYPE(Session)` in shared header.
  - Populate list items with full `Session` objects, not just names.
  - Provide protected accessors (`selectedSessions()`, etc.) so subclasses never touch `ui` directly.
  - Remove `ui_SessionDialog.h` includes from derived classes.

- **UI encapsulation audit**
  - Sweep all dialogs for direct `ui` access.
  - Replace with protected getters or model accessors.
  - Standardise naming and signal/slot patterns.

- **Dual‑pane UI pattern adoption**
  - Apply AutoStart dual‑pane approach to other multi‑list dialogs.
  - Ensure tab order and resizing behaviour are consistent.

- **Directory re‑org follow‑through**
  - Complete moving all session management code into `Sessions/` directory.
  - Update project‑root‑relative includes.
  - Remove stale includes and redundant forward declarations.

- **Legacy dialog retirement**
  - Identify and delete unused/superseded dialogs after refactors.
  - Use history‑preserving deletes.

---

### Feature
- **AutoStart ordering**
  - Optional: allow user to reorder auto‑start sessions (Up/Down buttons).
  - Persist order in `SessionStore`.

- **Session deletion flow polish**
  - Add confirmation dialog with session details.
  - Optionally allow multi‑select delete.

- **AutoStart redesign integration**
  - Replace legacy auto‑start toggles with new dialog.
  - Remove redundant code paths.

- **Audit‑grade logging**
  - Add structured logging for all session add/remove/rename actions.
  - Ensure rollback‑safe and human‑readable.

---

## 🚧 In Progress / Imminent
- **ManageAutoStartDialog integration**
  - Hook “Manage AutoStart…” button in `ManageSessionsDialog` to launch dialog with live `SessionStore`.
  - Test add/remove flow end‑to‑end.
  - Confirm tab order and resizing behaviour in `.ui`.

- **AutoStart persistence**
  - `SessionStore` to list all auto‑start sessions and save per‑session auto‑start markers.
  - Dialog to maintain in‑memory working list; commit on OK.

---

## ✅ Done / Recently Completed
- `.ui`‑driven dual‑pane layout for AutoStart management.
- Explicit `connect()` calls in `ManageAutoStartDialog`.
- Explicit types (no `auto`) and header guards in new code.
- Clazy‑clean wiring for AutoStart dialog.

---

## 📝 Notes & Conventions
- **No `m_` prefix** for private members — use clear names or trailing underscore if needed.
- **Header guards** over `#pragma once`.
- **Explicit types** instead of `auto`.
- **Project‑root‑relative includes** for easier future moves.
- **Audit‑grade, rollback‑safe changes** with `git mv` for history preservation.
- **Type‑safe UI lists** — store full `Session` objects in `UserRole`.

---

## 📌 From Code Comments (TODO / FIXME / NOTE)

> _Harvested from in‑code comments. Keep in sync with above sections or clear when resolved._

- [TODO] Refactor ManageSessions to use `SessionDialogBase` accessor (ManageSessions.cpp:142)
- [FIXME] Handle null `Session` in AutoStart removal (SessionStore.cpp:88)
- [TODO] Remove legacy auto‑start toggle code after new dialog is integrated (AutoStartManager.cpp:55)
- [FIXME] Ensure `listAutoStartSessions()` handles duplicate names safely (SessionStore.cpp:101)
- [NOTE] Consider bulk setter for auto‑start list to avoid partial updates in Deferred mode (SessionStore.cpp:110)
- [TODO] Audit all derived dialogs for direct `ui` access (SessionDialogBase.cpp:75)
- [FIXME] Tab order incorrect in AutoStart dialog after adding new layout (ManageAutoStartDialog.ui:—)
- [NOTE] Parked: reorder auto‑start sessions feature (ManageAutoStartDialog.cpp:—)
