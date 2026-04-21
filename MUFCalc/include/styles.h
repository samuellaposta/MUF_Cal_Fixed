#pragma once
#include <QString>

// ══════════════════════════════════════════════════════════════════
//  MUFCalc  –  Nuclear Facility Management System
//  LIGHT institutional theme throughout – no dark panels
//  Input controls: white bg, black text, clean underline style
// ══════════════════════════════════════════════════════════════════
namespace Styles {

// ── Palette ───────────────────────────────────────────────────────
static const QString NAVY       = "#1a3a5c";
static const QString NAVY_DK    = "#0f2438";
static const QString NAVY_LT    = "#2d5680";
static const QString BG_SIDEBAR = "#f0f4f8";   // light blue-gray sidebar
static const QString BG_BASE    = "#f7f9fb";   // page background
static const QString BORDER     = "#d1d8e0";
static const QString C_GREEN    = "#15803d";
static const QString C_YELLOW   = "#b45309";
static const QString C_ORANGE   = "#c2410c";
static const QString C_RED      = "#b91c1c";
// Severity (kept for compatibility)
static const QString SEV_OK_BG  = "#f0fdf4"; static const QString SEV_OK_BD = "#16a34a";
static const QString SEV_IN_BG  = "#eff6ff"; static const QString SEV_IN_BD = "#1d4ed8";
static const QString SEV_WA_BG  = "#fffbeb"; static const QString SEV_WA_BD = "#d97706";
static const QString SEV_AL_BG  = "#fff7ed"; static const QString SEV_AL_BD = "#ea580c";
static const QString SEV_CR_BG  = "#fef2f2"; static const QString SEV_CR_BD = "#dc2626";

// ── Global app stylesheet ──────────────────────────────────────────
inline QString appStyle() {
    return QStringLiteral(
// Base – transparent so watermark shows, white on top-level windows
"QWidget{background-color:transparent;color:#1a1a2e;"
  "font-family:'Segoe UI','Ubuntu','Liberation Sans',Arial,sans-serif;font-size:16px;}"
"QMainWindow{background-color:#ffffff;}"
"QDialog{background-color:#ffffff;}"
// Menu bar – navy, which is standard for management systems
"QMenuBar{background:#1a3a5c;color:white;"
  "border-bottom:2px solid #2d5680;padding:2px 6px;"
  "font-size:16px;font-weight:600;}"
"QMenuBar::item{padding:6px 16px;border-radius:3px;color:white;}"
"QMenuBar::item:selected{background:#2d5680;}"
"QMenu{background:white;border:1px solid #d1d8e0;color:#1a1a2e;"
  "border-radius:4px;padding:4px 0;}"
"QMenu::item{padding:7px 24px;font-size:16px;}"
"QMenu::item:selected{background:#eef4f9;color:#1a3a5c;}"
"QMenu::separator{height:1px;background:#e5eaef;margin:4px 10px;}"
// Status bar – light
"QStatusBar{background:#eef4f9;border-top:1px solid #d1d8e0;color:#1a3a5c;"
  "font-size:14px;padding:3px 12px;font-family:'Courier New',monospace;}"
// Scrollbars – subtle
"QScrollBar:vertical{background:#f0f4f8;width:8px;border-radius:4px;margin:2px;}"
"QScrollBar::handle:vertical{background:#b0c8dc;border-radius:4px;min-height:32px;}"
"QScrollBar::handle:vertical:hover{background:#1a3a5c;}"
"QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical{height:0;}"
"QScrollBar:horizontal{background:#f0f4f8;height:8px;border-radius:4px;margin:2px;}"
"QScrollBar::handle:horizontal{background:#b0c8dc;border-radius:4px;min-width:32px;}"
"QScrollBar::add-line:horizontal,QScrollBar::sub-line:horizontal{width:0;}"
"QSplitter::handle{background:#d1d8e0;}"
"QToolTip{background:#1a3a5c;color:white;border:none;padding:5px 10px;"
  "border-radius:4px;font-size:14.5px;}"
// Group boxes – clean, top accent only
"QGroupBox{color:#1a3a5c;border:none;border-top:2px solid #1a3a5c;"
  "margin-top:18px;padding:14px 0 0 0;"
  "font-weight:700;font-size:13.5px;letter-spacing:1.2px;background:transparent;}"
"QGroupBox::title{subcontrol-origin:margin;left:0;padding:2px 10px;"
  "background:#1a3a5c;color:white;font-size:12.5px;letter-spacing:1px;}"
// Tabs
"QTabWidget::pane{border:1px solid #d1d8e0;"
  "background:rgba(255,255,255,0.97);margin-top:-1px;}"
"QTabBar::tab{background:#e8eff5;color:#4a5568;padding:7px 18px;"
  "border:1px solid #d1d8e0;border-bottom:none;"
  "border-radius:4px 4px 0 0;font-size:15px;font-weight:600;margin-right:2px;}"
"QTabBar::tab:selected{background:white;color:#1a3a5c;"
  "border-top:3px solid #1a3a5c;font-weight:700;}"
"QTabBar::tab:hover:!selected{background:#dce8f0;}"
// List widgets
"QListWidget{background:white;border:none;color:#1a1a2e;}"
"QListWidget::item{padding:9px 14px;border-bottom:1px solid #eef1f5;font-size:16px;}"
"QListWidget::item:hover{background:#eef4f9;color:#1a3a5c;}"
"QListWidget::item:selected{background:#1a3a5c;color:white;}"
// Text areas – plain, left accent
"QTextEdit,QTextBrowser{background:white;border:none;border-left:3px solid #1a3a5c;"
  "color:#1a1a2e;padding:10px;font-size:16px;}"
"QFrame{border:none;}"
    );
}

// ── Input fields: white bg, black text, underline only ─────────────
inline QString lineEditStyle() {
    return QStringLiteral(
"QLineEdit,QDoubleSpinBox,QSpinBox,QDateEdit,QComboBox{"
  "background:#FFFFFF;border:none;border-bottom:1.5px solid #b0c8dc;"
  "border-radius:0;color:#000000;padding:6px 4px;font-size:16px;"
  "selection-background-color:#1a3a5c;selection-color:#ffffff;}"
"QLineEdit:focus,QDoubleSpinBox:focus,QSpinBox:focus,"
"QDateEdit:focus,QComboBox:focus{"
  "border-bottom:2px solid #1a3a5c;background:#FFFFFF;}"
"QLineEdit:read-only{background:#f7f9fb;color:#4a5568;}"
"QLineEdit[valid='false']{border-bottom:2px solid #dc2626;}"
"QLineEdit[valid='true']{border-bottom:2px solid #16a34a;}"
"QComboBox::drop-down{border:none;width:24px;background:transparent;}"
"QComboBox QAbstractItemView{background:white;border:1px solid #d1d8e0;"
  "color:#000000;selection-background-color:#1a3a5c;selection-color:white;}"
"QDoubleSpinBox::up-button,QDoubleSpinBox::down-button,"
"QSpinBox::up-button,QSpinBox::down-button,"
"QDateEdit::up-button,QDateEdit::down-button{"
  "background:transparent;border:none;width:20px;}"
    );
}

// ── Buttons ───────────────────────────────────────────────────────
inline QString primaryButtonStyle() {
    return QStringLiteral(
"QPushButton{background:#1a3a5c;color:white;border:none;"
  "border-radius:3px;padding:8px 22px;font-weight:700;"
  "font-size:16px;letter-spacing:0.3px;}"
"QPushButton:hover{background:#2d5680;}"
"QPushButton:pressed{background:#0f2438;}"
"QPushButton:disabled{background:#d1d8e0;color:#9ca3af;}"
    );
}
inline QString greenButtonStyle() {
    return QStringLiteral(
"QPushButton{background:#2a6020;color:white;border:none;"
  "border-radius:3px;padding:8px 22px;font-weight:700;font-size:16px;}"
"QPushButton:hover{background:#3d8a2e;}"
"QPushButton:pressed{background:#1a4010;}"
"QPushButton:disabled{background:#d1d8e0;color:#9ca3af;}"
    );
}
inline QString secondaryButtonStyle() {
    return QStringLiteral(
"QPushButton{background:white;color:#1a3a5c;border:1.5px solid #1a3a5c;"
  "border-radius:3px;padding:7px 18px;font-size:16px;font-weight:600;}"
"QPushButton:hover{background:#eef4f9;}"
"QPushButton:pressed{background:#dce8f0;}"
    );
}
inline QString dangerButtonStyle() {
    return QStringLiteral(
"QPushButton{background:#fef2f2;color:#b91c1c;border:1px solid #fca5a5;"
  "border-radius:3px;padding:6px 14px;font-size:15px;font-weight:600;}"
"QPushButton:hover{background:#fee2e2;}"
    );
}
inline QString tableStyle() {
    return QStringLiteral(
"QTableWidget{background:white;alternate-background-color:#f4f8fb;"
  "border:none;gridline-color:#e8edf2;"
  "color:#000000;selection-background-color:#c8dff0;selection-color:#0f2438;}"
"QTableWidget::item{padding:7px 12px;border:none;color:#000000;}"
"QHeaderView::section{background:#1a3a5c;color:white;font-weight:700;"
  "font-size:13.5px;letter-spacing:0.6px;padding:8px 12px;border:none;"
  "border-right:1px solid #2d5680;}"
"QHeaderView{background:#1a3a5c;}"
    );
}
inline QString tabStyle() { return QString(); }

} // namespace Styles


