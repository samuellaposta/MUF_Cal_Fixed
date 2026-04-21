#include "mainwindow.h"
#include "watermarkwidget.h"
#include "dataentrywizard.h"
#include "resultspanel.h"
#include "aboutdialog.h"
#include "userguidedialog.h"
#include "virtualfacilitydialog.h"
#include "styles.h"
#include <QMenuBar>
#include <QStatusBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QFrame>
#include <QLabel>
#include <QPixmap>
#include <QDesktopServices>
#include <QStandardPaths>
#include <QFile>
#include <QUrl>
#include <QScrollArea>
#include <QFileDialog>
#include <QMessageBox>
#include <cmath>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(
        "MUFCalc v1.0  –  Nuclear Material Accountancy Management System  |  TPU");
    setMinimumSize(1100, 720);
    setStyleSheet(Styles::appStyle() + Styles::lineEditStyle());
    buildMenuBar();
    buildUI();
    setStatus(
        "System Ready  ·  IAEA Safeguards Compliant  ·  "
        "Tomsk Polytechnic University – School of Nuclear Science & Engineering",
        Styles::C_GREEN);
}

// ─────────────────────────────────────────────────────────────────
void MainWindow::buildMenuBar() {
    auto* mFile = menuBar()->addMenu("&File");
    QAction *actNew = mFile->addAction("&New Calculation", this, &MainWindow::onNewCalculation);
    actNew->setShortcut(QKeySequence::New);
    mFile->addSeparator();
    mFile->addAction("E&xport Report",                        this, &MainWindow::onExportReport);
    mFile->addSeparator();
    QAction *actExit = mFile->addAction("E&xit", this, &QWidget::close);
    actExit->setShortcut(QKeySequence::Quit);
    auto* mCalc = menuBar()->addMenu("&Calculate");
    mCalc->addAction("▶  New Calculation", this, &MainWindow::onNewCalculation);
    mCalc->addAction("Load Example Data",  this, &MainWindow::onLoadExample);
    mCalc->addSeparator();
    mCalc->addAction("View Process Flowchart", this, &MainWindow::onShowFlowchart);
    mCalc->addAction("View IAEA Equations",    this, &MainWindow::onShowEquations);
    mCalc->addSeparator();
    mCalc->addAction("Clear History",          this, &MainWindow::onClearHistory);

    auto* mTools = menuBar()->addMenu("&Tools");
    mTools->addAction("Virtual Facility Simulator", this, &MainWindow::onVirtualFacility);
    mTools->addSeparator();
    mTools->addAction("Student User Guide",     this, &MainWindow::onUserGuide);

    auto* mHelp = menuBar()->addMenu("&Help");
    mHelp->addAction("Student User Guide",        this, &MainWindow::onUserGuide);
    mHelp->addAction("IAEA Reference Equations",  this, &MainWindow::onIAEARef);
    mHelp->addAction("Diversion Detection Guide", this, &MainWindow::onDiversionGuide);
    mHelp->addSeparator();
    mHelp->addAction("&About MUFCalc",            this, &MainWindow::onAbout);
}

// ─────────────────────────────────────────────────────────────────
void MainWindow::buildUI() {
    auto* splitter = new QSplitter(Qt::Horizontal, this);
    splitter->setHandleWidth(1);

    // ════════════════════════════════════════════════════════════════
    // LEFT SIDEBAR – light blue-gray, institutional
    // ════════════════════════════════════════════════════════════════
    auto* sidebar = new QFrame();
    sidebar->setFixedWidth(240);
    sidebar->setStyleSheet(
        "QFrame{background:#f0f4f8;border-right:1px solid #d1d8e0;}");
    auto* sbl = new QVBoxLayout(sidebar);
    sbl->setContentsMargins(0,0,0,0); sbl->setSpacing(0);

    // Identity block – white with navy top border
    auto* idBlock = new QWidget;
    idBlock->setStyleSheet(
        "background:white;border-bottom:1px solid #d1d8e0;");
    auto* idL = new QVBoxLayout(idBlock);
    idL->setContentsMargins(16,16,16,14); idL->setSpacing(6);

    QPixmap logo(":/tpu_logo.png");
    if (!logo.isNull()) {
        auto* logoLbl = new QLabel();
        logoLbl->setPixmap(logo.scaledToWidth(178, Qt::SmoothTransformation));
        logoLbl->setAlignment(Qt::AlignCenter);
        idL->addWidget(logoLbl);
    }
    auto* sysNameLbl = new QLabel("MUFCalc");
    sysNameLbl->setAlignment(Qt::AlignCenter);
    sysNameLbl->setStyleSheet(
        "color:#1a3a5c;font-size:15px;font-weight:900;letter-spacing:1px;"
        "background:transparent;");
    auto* sysSubLbl = new QLabel("NUCLEAR MATERIAL ACCOUNTING");
    sysSubLbl->setAlignment(Qt::AlignCenter);
    sysSubLbl->setStyleSheet(
        "color:#4a6a8a;font-size:10px;font-weight:700;letter-spacing:2px;"
        "background:transparent;");
    auto* verLbl = new QLabel("IAEA SAFEGUARDS  v1.0");
    verLbl->setAlignment(Qt::AlignCenter);
    verLbl->setStyleSheet(
        "color:#7a9ab8;font-size:10px;letter-spacing:1.5px;"
        "font-family:'Courier New',monospace;background:transparent;");
    idL->addWidget(sysNameLbl);
    idL->addWidget(sysSubLbl);
    idL->addWidget(verLbl);
    sbl->addWidget(idBlock);

    // Action buttons
    auto* actionBlock = new QWidget;
    actionBlock->setStyleSheet("background:#f0f4f8;");
    auto* actionL = new QVBoxLayout(actionBlock);
    actionL->setContentsMargins(12,10,12,8); actionL->setSpacing(6);

    auto* btnNew = new QPushButton("▶   New Calculation");
    btnNew->setStyleSheet(Styles::primaryButtonStyle() +
        "QPushButton{text-align:left;padding:11px 14px;font-size:13px;}");
    connect(btnNew, &QPushButton::clicked, this, &MainWindow::onNewCalculation);

    auto* btnEx = new QPushButton("⚇   Load Example");
    btnEx->setStyleSheet(Styles::greenButtonStyle() +
        "QPushButton{text-align:left;padding:9px 14px;font-size:13px;}");
    connect(btnEx, &QPushButton::clicked, this, &MainWindow::onLoadExample);

    actionL->addWidget(btnNew);
    actionL->addWidget(btnEx);
    sbl->addWidget(actionBlock);

    // Tools section
    auto* toolSepLbl = new QLabel("  TOOLS & RESOURCES");
    toolSepLbl->setStyleSheet(
        "background:#e2eaf2;color:#4a6a8a;font-size:10px;font-weight:700;"
        "letter-spacing:2px;padding:8px 14px;"
        "border-top:1px solid #d1d8e0;border-bottom:1px solid #d1d8e0;");
    sbl->addWidget(toolSepLbl);

    auto mkSideBtn = [](const QString& icon, const QString& label) -> QPushButton* {
        auto* b = new QPushButton(icon + "  " + label);
        b->setStyleSheet(
            "QPushButton{background:transparent;color:#1a3a5c;border:none;"
            "text-align:left;padding:9px 18px;font-size:13px;border-radius:0;}"
            "QPushButton:hover{background:#dce8f0;color:#0f2438;}");
        return b;
    };

    auto* btnFlow  = mkSideBtn("[  ]", "Process Flowchart");
    auto* btnEq    = mkSideBtn("[=]",  "IAEA Equations");
    auto* btnGuide = mkSideBtn("[?]",  "Student Guide");
    auto* btnVF    = mkSideBtn("[S]",  "Virtual Facilities");

    connect(btnFlow,  &QPushButton::clicked, this, &MainWindow::onShowFlowchart);
    connect(btnEq,    &QPushButton::clicked, this, &MainWindow::onShowEquations);
    connect(btnGuide, &QPushButton::clicked, this, &MainWindow::onUserGuide);
    connect(btnVF,    &QPushButton::clicked, this, &MainWindow::onVirtualFacility);
    sbl->addWidget(btnFlow);
    sbl->addWidget(btnEq);
    sbl->addWidget(btnGuide);
    sbl->addWidget(btnVF);

    // History section
    auto* histSepLbl = new QLabel("  RECENT CALCULATIONS");
    histSepLbl->setStyleSheet(
        "background:#e2eaf2;color:#4a6a8a;font-size:10px;font-weight:700;"
        "letter-spacing:2px;padding:8px 14px;"
        "border-top:1px solid #d1d8e0;border-bottom:1px solid #d1d8e0;");
    sbl->addWidget(histSepLbl);

    m_historyList = new QListWidget();
    m_historyList->setStyleSheet(
        "QListWidget{background:#f7f9fb;border:none;color:#1a1a2e;}"
        "QListWidget::item{padding:9px 16px;border-bottom:1px solid #e8edf2;"
        "color:#1a3a5c;font-size:12px;font-family:'Courier New',monospace;}"
        "QListWidget::item:hover{background:#dce8f0;color:#0f2438;}"
        "QListWidget::item:selected{background:#1a3a5c;color:white;}");
    connect(m_historyList, &QListWidget::itemClicked,
            this, &MainWindow::onHistoryClicked);
    sbl->addWidget(m_historyList, 1);

    // Footer chip
    auto* sideFooter = new QWidget;
    sideFooter->setStyleSheet(
        "background:#e2eaf2;border-top:1px solid #d1d8e0;");
    auto* sfL = new QHBoxLayout(sideFooter);
    sfL->setContentsMargins(12,5,12,6); sfL->setSpacing(6);
    auto* dbChip = new QLabel("● DB Active");
    dbChip->setStyleSheet(
        "color:#15803d;font-size:10px;font-weight:700;background:transparent;");
    auto* tpuChip = new QLabel("TPU / NSE");
    tpuChip->setStyleSheet(
        "color:#4a6a8a;font-size:10px;background:transparent;");
    sfL->addWidget(dbChip); sfL->addStretch(); sfL->addWidget(tpuChip);
    sbl->addWidget(sideFooter);

    // Right stack
    m_stack = new QStackedWidget();
    m_stack->setStyleSheet("background:white;");
    m_stack->addWidget(buildWelcomePage());
    m_stack->addWidget(buildDashboard());

    splitter->addWidget(sidebar);
    splitter->addWidget(m_stack);
    splitter->setSizes({240, 860});
    setCentralWidget(splitter);

    m_statusLbl = new QLabel();
    m_statusLbl->setStyleSheet(
        "font-size:11px;padding:0 8px;font-family:'Courier New',monospace;");
    statusBar()->addWidget(m_statusLbl);
}

// ─────────────────────────────────────────────────────────────────
//  WELCOME PAGE – 10% IAEA watermark + aims & clickable ref links
// ─────────────────────────────────────────────────────────────────
QWidget* MainWindow::buildWelcomePage() {
    auto* page = new WatermarkWidget(0.10, Qt::white);
    auto* outerL = new QVBoxLayout(page);
    outerL->setContentsMargins(0,0,0,0); outerL->setSpacing(0);

    // Top banner – light blue gradient
    auto* banner = new QWidget;
    banner->setFixedHeight(68);
    banner->setStyleSheet(
        "background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #1a3a5c,stop:0.6 #2d5680,stop:1 #4a7fa8);"
        "border-bottom:3px solid #7fb3d3;");
    auto* bannerL = new QVBoxLayout(banner);
    bannerL->setContentsMargins(32,10,32,8); bannerL->setSpacing(2);
    auto* bannerTitle = new QLabel("Nuclear Material Accountancy Management System");
    bannerTitle->setStyleSheet(
        "color:white;font-size:17px;font-weight:900;background:transparent;");
    auto* bannerSub = new QLabel(
        "MUFCalc v1.0  ·  IAEA INFCIRC/153 Compliant  ·  "
        "Tomsk Polytechnic University  ·  School of Nuclear Science & Engineering");
    bannerSub->setStyleSheet(
        "color:#c8dff0;font-size:11px;background:transparent;");
    bannerL->addWidget(bannerTitle);
    bannerL->addWidget(bannerSub);
    outerL->addWidget(banner);

    // Scrollable content – transparent so watermark shows
    auto* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet(
        "QScrollArea{background:transparent;border:none;}"
        "QScrollArea>QWidget>QWidget{background:transparent;}");

    auto* inner = new QWidget();
    inner->setStyleSheet("background:transparent;");
    auto* layout = new QVBoxLayout(inner);
    layout->setContentsMargins(36,24,36,24); layout->setSpacing(20);

    // Two-column layout
    auto* twoCol = new QHBoxLayout(); twoCol->setSpacing(22);

    // ── LEFT: Aims & Objectives ─────────────────────────────────
    auto* aoPanel = new QFrame();
    aoPanel->setStyleSheet(
        "QFrame{background:rgba(255,255,255,0.93);"
        "border:1px solid #d1d8e0;border-top:3px solid #1a3a5c;}");
    auto* aoPanelL = new QVBoxLayout(aoPanel);
    aoPanelL->setContentsMargins(20,16,20,16); aoPanelL->setSpacing(8);

    auto* aoHdr = new QLabel("AIMS  &  OBJECTIVES");
    aoHdr->setStyleSheet(
        "color:#1a3a5c;font-size:11px;font-weight:700;letter-spacing:2px;"
        "background:transparent;");
    aoPanelL->addWidget(aoHdr);

    auto* aoDiv = new QFrame(); aoDiv->setFrameShape(QFrame::HLine);
    aoDiv->setStyleSheet("background:#d1d8e0;max-height:1px;border:none;");
    aoPanelL->addWidget(aoDiv);

    auto* tagline = new QLabel(
        "A training and operational platform for nuclear material balance "
        "calculations aligned with IAEA safeguards methodology.");
    tagline->setWordWrap(true);
    tagline->setStyleSheet(
        "color:#374151;font-size:12px;font-style:italic;"
        "padding:2px 0 6px 0;background:transparent;");
    aoPanelL->addWidget(tagline);

    struct Aim { const char* num; const char* text; };
    static const Aim AIMS[] = {
        {"01","Implement the IAEA material balance equation: "
              "MUF = (BI + ΣInputs − ΣOutputs) − EI  per INFCIRC/153."},
        {"02","Propagate measurement uncertainty: "
              "σ(MUF) = √[σ²(BI) + σ²(ΣIn) + σ²(ΣOut)] per IAEA quadrature."},
        {"03","Apply diversion detection diagnostics: sigma-ratio, SQ threshold, "
              "SITMUF, Page's CUSUM and CUMUF trend analysis."},
        {"04","Perform uncertainty contribution analysis (Cᵢ) to identify the "
              "dominant measurement source limiting safeguards conclusions."},
        {"05","Train facility operators and engineering students in end-to-end "
              "IAEA nuclear material accounting principles."},
        {"06","Simulate virtual nuclear facility scenarios (enrichment, "
              "reprocessing, research reactor) for hands-on NMA education."},
        {"07","Generate regulation-ready HTML audit reports for inspector review "
              "and academic documentation."},
    };

    for (const auto& a : AIMS) {
        auto* row = new QWidget; row->setStyleSheet("background:transparent;");
        auto* rowL = new QHBoxLayout(row);
        rowL->setContentsMargins(0,1,0,1); rowL->setSpacing(10);
        auto* numLbl = new QLabel(a.num);
        numLbl->setFixedWidth(26);
        numLbl->setAlignment(Qt::AlignCenter);
        numLbl->setStyleSheet(
            "background:#1a3a5c;color:white;font-size:10px;"
            "font-weight:700;border-radius:2px;padding:2px 0;");
        auto* txt = new QLabel(a.text);
        txt->setWordWrap(true);
        txt->setStyleSheet("color:#1a1a2e;font-size:12px;background:transparent;");
        rowL->addWidget(numLbl, 0, Qt::AlignTop);
        rowL->addWidget(txt, 1);
        aoPanelL->addWidget(row);
    }

    aoPanelL->addStretch(1);

    // ── Reference link-buttons (CLICKABLE) ──────────────────────
    auto* refHdr = new QLabel("REFERENCE STANDARDS:");
    refHdr->setStyleSheet(
        "color:#4a6a8a;font-size:10px;font-weight:700;"
        "letter-spacing:1px;background:transparent;");
    aoPanelL->addWidget(refHdr);

    auto mkRefBtn = [](const QString& label) -> QPushButton* {
        auto* b = new QPushButton(label);
        b->setCursor(Qt::PointingHandCursor);
        b->setStyleSheet(
            "QPushButton{background:#eef4f9;color:#1a3a5c;"
            "border:1px solid #b0c8dc;border-radius:3px;"
            "padding:3px 10px;font-size:11px;font-weight:700;"
            "letter-spacing:0.4px;text-align:left;}"
            "QPushButton:hover{background:#1a3a5c;color:white;"
            "border-color:#1a3a5c;}"
            "QPushButton:pressed{background:#0f2438;color:white;}");
        return b;
    };

    auto* badgeRow = new QHBoxLayout(); badgeRow->setSpacing(6);
    auto* btnINF  = mkRefBtn("INFCIRC/153");
    auto* btnNMA  = mkRefBtn("NMA Handbook 2008");
    auto* btnGlos = mkRefBtn("Safeguards Glossary 2022");
    badgeRow->addWidget(btnINF);
    badgeRow->addWidget(btnNMA);
    badgeRow->addWidget(btnGlos);
    badgeRow->addStretch();
    aoPanelL->addLayout(badgeRow);

    // Wire up the reference buttons to existing info dialogs
    connect(btnINF,  &QPushButton::clicked, this, &MainWindow::onIAEARef);
    connect(btnNMA,  &QPushButton::clicked, this, &MainWindow::onIAEARef);
    connect(btnGlos, &QPushButton::clicked, this, &MainWindow::onDiversionGuide);

    twoCol->addWidget(aoPanel, 5);

    // ── RIGHT: Navigation tiles ──────────────────────────────────
    auto* navCol = new QWidget; navCol->setStyleSheet("background:transparent;");
    auto* navL = new QVBoxLayout(navCol);
    navL->setContentsMargins(0,0,0,0); navL->setSpacing(10);

    auto* navHdr = new QLabel("MODULES  &  TOOLS");
    navHdr->setStyleSheet(
        "color:#1a3a5c;font-size:11px;font-weight:700;letter-spacing:2px;"
        "background:transparent;");
    navL->addWidget(navHdr);
    auto* navDiv = new QFrame(); navDiv->setFrameShape(QFrame::HLine);
    navDiv->setStyleSheet("background:#d1d8e0;max-height:1px;border:none;");
    navL->addWidget(navDiv);
    navL->addSpacing(2);

    auto mkNavTile = [](const QString& icon, const QString& title,
                        const QString& desc, const QString& hint) -> QPushButton*
    {
        auto* b = new QPushButton(
            QString("%1  %2\n\n%3\n\n%4").arg(icon, title, desc, hint));
        b->setCursor(Qt::PointingHandCursor);
        b->setStyleSheet(
            "QPushButton{background:rgba(255,255,255,0.95);color:#1a1a2e;"
            "border:1px solid #d1d8e0;border-left:4px solid #1a3a5c;"
            "border-radius:0;padding:14px 16px;font-size:12px;"
            "font-weight:normal;text-align:left;min-height:72px;}"
            "QPushButton:hover{background:#eef4f9;color:#1a3a5c;"
            "border-left-color:#2d5680;border-left-width:5px;}"
            "QPushButton:pressed{background:#dce8f0;}");
        return b;
    };

    auto* btnMUF    = mkNavTile("▶", "MUF CALCULATION",
        "Book inventory · MUF · σ(MUF)\nDiversion diagnostics",
        "File › New Calculation  |  Ctrl+N");
    auto* btnEx2    = mkNavTile("⚇", "LOAD EXAMPLE",
        "Pre-populated enrichment plant\nscenario for immediate use",
        "File › Load Example");
    auto* btnVF2    = mkNavTile("[S]", "VIRTUAL FACILITY",
        "Simulate enrichment, reprocessing\nand research reactor scenarios",
        "Tools › Virtual Facility Simulator");
    auto* btnGuide2 = mkNavTile("[?]", "STUDENT GUIDE",
        "Step-by-step lessons · theory\nGlossary and worked examples",
        "Help › Student User Guide");

    connect(btnMUF,    &QPushButton::clicked, this, &MainWindow::onNewCalculation);
    connect(btnEx2,    &QPushButton::clicked, this, &MainWindow::onLoadExample);
    connect(btnVF2,    &QPushButton::clicked, this, &MainWindow::onVirtualFacility);
    connect(btnGuide2, &QPushButton::clicked, this, &MainWindow::onUserGuide);

    navL->addWidget(btnMUF);
    navL->addWidget(btnEx2);
    navL->addWidget(btnVF2);
    navL->addWidget(btnGuide2);
    navL->addStretch(1);

    // Status chips
    auto* chipRow = new QHBoxLayout(); chipRow->setSpacing(6);
    auto mkChip = [](const QString& t, const QString& bg,
                     const QString& fg, const QString& bd) -> QLabel* {
        auto* c = new QLabel(t);
        c->setStyleSheet(QString("background:%1;color:%2;border:1px solid %3;"
            "border-radius:3px;padding:3px 8px;font-size:10px;font-weight:700;")
            .arg(bg,fg,bd));
        return c;
    };
    chipRow->addWidget(mkChip("● Engine Ready","#f0fdf4","#15803d","#86efac"));
    chipRow->addWidget(mkChip("SQLite Audit",  "#eef4f9","#1a3a5c","#b0c8dc"));
    chipRow->addStretch();
    navL->addLayout(chipRow);

    twoCol->addWidget(navCol, 4);
    layout->addLayout(twoCol);

    // IAEA equations strip
    auto* eqHdr = new QLabel("IAEA STANDARD EQUATIONS  (INFCIRC/153)");
    eqHdr->setStyleSheet(
        "color:#1a3a5c;font-size:11px;font-weight:700;letter-spacing:2px;"
        "background:transparent;");
    layout->addWidget(eqHdr);

    auto* eqRow = new QHBoxLayout(); eqRow->setSpacing(10);
    struct EQ { QString title, formula, color; };
    for (auto& eq : QVector<EQ>{
        {"Book Inventory",    "IB = BI + ΣInputs − ΣOutputs", "#1a3a5c"},
        {"MUF",               "MUF = IB − EI",                "#2a6020"},
        {"σ(MUF)",            "σ = √[σ²(BI)+σ²(ΣIn)+σ²(ΣOut)]","#c2410c"},
        {"Contribution Cᵢ",  "Cᵢ = σ²(i)/σ²(MUF) × 100%",   "#1d4ed8"},
    }) {
        auto* c = new QFrame();
        c->setStyleSheet(
            QString("QFrame{background:rgba(255,255,255,0.95);"
                    "border:1px solid #e5eaef;border-top:3px solid %1;}")
            .arg(eq.color));
        auto* cl = new QVBoxLayout(c);
        cl->setContentsMargins(12,9,12,11); cl->setSpacing(3);
        auto* t = new QLabel(eq.title);
        t->setStyleSheet(
            QString("color:%1;font-size:11px;font-weight:700;background:transparent;")
            .arg(eq.color));
        auto* f = new QLabel(eq.formula);
        f->setStyleSheet(
            "color:#1a3a5c;font-family:'Courier New',monospace;"
            "font-size:12px;font-weight:600;background:transparent;");
        f->setWordWrap(true);
        cl->addWidget(t); cl->addWidget(f);
        eqRow->addWidget(c, 1);
    }
    layout->addLayout(eqRow);

    scroll->setWidget(inner);
    scroll->viewport()->setStyleSheet("background:transparent;");
    outerL->addWidget(scroll, 1);
    return page;
}

// ─────────────────────────────────────────────────────────────────
//  DASHBOARD – 15% watermark, light theme
// ─────────────────────────────────────────────────────────────────
QWidget* MainWindow::buildDashboard() {
    auto* page = new WatermarkWidget(0.15, Qt::white);
    auto* outerL = new QVBoxLayout(page);
    outerL->setContentsMargins(0,0,0,0); outerL->setSpacing(0);

    // Light top bar
    auto* topBar = new QWidget; topBar->setFixedHeight(44);
    topBar->setStyleSheet(
        "background:#eef4f9;border-bottom:2px solid #d1d8e0;");
    auto* topBarL = new QHBoxLayout(topBar);
    topBarL->setContentsMargins(24,0,24,0);
    auto* topTitle = new QLabel(
        "CALCULATION RESULTS  ·  SAFEGUARDS DASHBOARD");
    topTitle->setStyleSheet(
        "color:#1a3a5c;font-size:11px;font-weight:700;letter-spacing:1.5px;"
        "background:transparent;");
    topBarL->addWidget(topTitle); topBarL->addStretch();
    outerL->addWidget(topBar);

    auto* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet(
        "QScrollArea{background:transparent;border:none;}"
        "QScrollArea>QWidget>QWidget{background:transparent;}");

    auto* w = new QWidget(); w->setStyleSheet("background:transparent;");
    auto* layout = new QVBoxLayout(w);
    layout->setContentsMargins(28,22,28,22); layout->setSpacing(14);

    // MUF banner card
    auto* banner = new QFrame();
    banner->setStyleSheet(
        "QFrame{background:rgba(255,255,255,0.96);"
        "border:1px solid #d1d8e0;border-top:3px solid #1a3a5c;}");
    auto* bl = new QHBoxLayout(banner);
    bl->setContentsMargins(26,20,26,20); bl->setSpacing(24);

    auto* leftBl = new QVBoxLayout();
    auto* mufTop = new QLabel(
        "MATERIAL UNACCOUNTED FOR   │   MUF = IB − EI");
    mufTop->setStyleSheet(
        "color:#4a6a8a;font-size:11px;letter-spacing:1.5px;background:transparent;");
    m_dashMUF = new QLabel("—");
    m_dashMUF->setStyleSheet(
        "color:#1a3a5c;font-size:28px;font-weight:900;"
        "font-family:'Courier New',monospace;background:transparent;");
    m_dashStatus = new QLabel("No calculation performed yet");
    m_dashStatus->setStyleSheet(
        "color:#4a5568;font-size:13px;background:transparent;");
    leftBl->addWidget(mufTop);
    leftBl->addWidget(m_dashMUF);
    leftBl->addWidget(m_dashStatus);
    bl->addLayout(leftBl, 1);

    auto* rightBl = new QVBoxLayout(); rightBl->setSpacing(8);
    auto mkMini = [](QLabel*& ref, const QString& lbl) {
        auto* f = new QFrame();
        f->setStyleSheet(
            "QFrame{background:#f7f9fb;border:1px solid #d1d8e0;}");
        auto* fl = new QVBoxLayout(f);
        fl->setContentsMargins(12,8,12,8); fl->setSpacing(2);
        ref = new QLabel("—");
        ref->setStyleSheet(
            "color:#1a3a5c;font-size:13px;font-weight:700;"
            "font-family:'Courier New',monospace;background:transparent;");
        auto* ll = new QLabel(lbl);
        ll->setStyleSheet(
            "color:#4a6a8a;font-size:10px;letter-spacing:1px;"
            "font-weight:700;background:transparent;");
        fl->addWidget(ref); fl->addWidget(ll);
        return f;
    };
    rightBl->addWidget(mkMini(m_dashSigma, "σ(MUF) UNCERTAINTY"));
    rightBl->addWidget(mkMini(m_dashIB,    "BOOK INVENTORY (IB)"));
    rightBl->addWidget(mkMini(m_dashSQ,    "SQ FRACTION"));
    bl->addLayout(rightBl);
    layout->addWidget(banner);

    m_dashFacility = new QLabel("No facility data loaded.");
    m_dashFacility->setStyleSheet(
        "color:#1a1a2e;font-size:13px;"
        "background:rgba(255,255,255,0.96);"
        "border:1px solid #d1d8e0;padding:10px 14px;");
    m_dashFacility->setWordWrap(true);
    layout->addWidget(m_dashFacility);

    auto* diagHdr = new QLabel("SAFEGUARDS DIAGNOSTIC SUMMARY");
    diagHdr->setStyleSheet(
        "color:#1a3a5c;font-size:11px;font-weight:700;"
        "letter-spacing:2px;background:transparent;");
    layout->addWidget(diagHdr);

    m_dashDiag = new QTextEdit();
    m_dashDiag->setReadOnly(true);
    m_dashDiag->setMaximumHeight(138);
    m_dashDiag->setStyleSheet(
        "background:rgba(255,255,255,0.96);"
        "border:none;border-left:3px solid #1a3a5c;"
        "font-family:'Courier New',monospace;font-size:12px;padding:8px;");
    m_dashDiag->setPlaceholderText(
        "Diagnostic flags appear here after calculation...");
    layout->addWidget(m_dashDiag);
    layout->addStretch();

    scroll->setWidget(w);
    scroll->viewport()->setStyleSheet("background:transparent;");
    outerL->addWidget(scroll, 1);
    return page;
}

// ─────────────────────────────────────────────────────────────────
void MainWindow::onNewCalculation() {
    auto* wiz = new DataEntryWizard(this);
    wiz->setAttribute(Qt::WA_DeleteOnClose);
    connect(wiz, &DataEntryWizard::calculateRequested, this,
            [this](const MBPInput& input) {
                MUFResult result = m_engine.calculate(input);
                onCalculationDone(input, result);
            });
    wiz->exec();
}
void MainWindow::onLoadExample() {
    auto* wiz = new DataEntryWizard(this);
    wiz->setAttribute(Qt::WA_DeleteOnClose);
    wiz->loadExample();
    connect(wiz, &DataEntryWizard::calculateRequested, this,
            [this](const MBPInput& input) {
                MUFResult result = m_engine.calculate(input);
                onCalculationDone(input, result);
            });
    wiz->exec();
}
void MainWindow::onCalculationDone(const MBPInput& input, const MUFResult& result) {
    if (!result.success) {
        QMessageBox::critical(this, "Validation Error", result.errorMessage);
        return;
    }
    m_lastInput  = input;
    m_lastResult = result;
    m_history.add(input, result);
    refreshDashboard(input, result);
    refreshHistory();
    m_stack->setCurrentIndex(1);
    ResultsPanel panel(input, result, this);
    panel.exec();
}
void MainWindow::refreshDashboard(const MBPInput& input, const MUFResult& result) {
    QString col = result.ratio <= 1 ? "#15803d" :
                  result.ratio <= 2 ? "#b45309" :
                  result.ratio <= 3 ? "#c2410c" : "#b91c1c";
    m_dashMUF->setStyleSheet(
        QString("color:%1;font-size:28px;font-weight:900;"
                "font-family:'Courier New',monospace;background:transparent;").arg(col));
    m_dashMUF->setText(
        QString("%1%2 kg")
        .arg(result.muf >= 0 ? "+" : "")
        .arg(result.muf, 0, 'f', 6));
    m_dashStatus->setStyleSheet(
        QString("color:%1;font-size:13px;font-weight:700;background:transparent;").arg(col));
    m_dashStatus->setText(
        result.ratio <= 1 ? "✔  NORMAL (≤1σ)   —  No diversion indicated" :
        result.ratio <= 2 ? "⚠  ELEVATED (1σ–2σ)  —  Monitor closely" :
        result.ratio <= 3 ? "⚠  WARNING (2σ–3σ)  —  Investigation required" :
                            "✖  CRITICAL (>3σ)  —  Possible diversion");
    m_dashSigma->setText(QString("±%1 kg").arg(result.sigmaMUF, 0, 'f', 4));
    m_dashSigma->setStyleSheet(
        "color:#1a3a5c;font-size:13px;font-weight:700;"
        "font-family:'Courier New',monospace;background:transparent;");
    m_dashIB->setText(QString("%1 kg").arg(result.bookInventory, 0, 'f', 4));
    m_dashIB->setStyleSheet(
        "color:#1a3a5c;font-size:13px;font-weight:700;"
        "font-family:'Courier New',monospace;background:transparent;");
    QString sqCol = result.sqFraction < 0.1  ? "#15803d" :
                    result.sqFraction < 0.25 ? "#b45309" :
                    result.sqFraction < 0.5  ? "#c2410c" : "#b91c1c";
    m_dashSQ->setText(
        QString("%1% of 1 SQ").arg(result.sqFraction * 100, 0, 'f', 2));
    m_dashSQ->setStyleSheet(
        QString("color:%1;font-size:13px;font-weight:700;"
                "font-family:'Courier New',monospace;background:transparent;").arg(sqCol));
    m_dashFacility->setText(
        QString("Facility: <b>%1</b>  [%2]  &nbsp;│&nbsp;  "
                "Material: <b>%3</b>  &nbsp;│&nbsp;  "
                "MBP: <b>%4</b>  &nbsp;│&nbsp;  Operator: %5")
        .arg(input.facilityName, input.facilityId,
             input.materialType, input.mbpId, input.operatorName));
    m_dashFacility->setTextFormat(Qt::RichText);
    m_dashDiag->clear();
    for (auto& f : result.flags) {
        QString pfx = f.severity==Severity::OK       ? "[OK]      " :
                      f.severity==Severity::INFO     ? "[INFO]    " :
                      f.severity==Severity::WARNING  ? "[WARNING] " :
                      f.severity==Severity::ALERT    ? "[ALERT]   " : "[CRITICAL]";
        m_dashDiag->append(pfx + f.code);
        m_dashDiag->append(
            "  " + f.message.left(100)+(f.message.length()>100?"...":"")+"\n");
    }
    setStatus(
        QString("Calculated: %1  |  MUF = %2 kg  |  σ(MUF) = %3 kg  |  "
                "Ratio = %4σ  |  SQ = %5%")
        .arg(input.mbpId)
        .arg(result.muf, 0, 'f', 4).arg(result.sigmaMUF, 0, 'f', 4)
        .arg(result.ratio, 0, 'f', 3).arg(result.sqFraction*100, 0, 'f', 2),
        result.ratio > 3 ? Styles::C_RED :
        result.ratio > 2 ? Styles::C_ORANGE : Styles::C_GREEN);
}
void MainWindow::refreshHistory() {
    m_historyList->clear();
    for (int i = 0; i < m_history.size(); ++i) {
        auto& [inp, res] = m_history.at(i);
        auto* item = new QListWidgetItem(
            QString("  ●  %1\n      MUF %2%3 kg  |  %4σ")
            .arg(inp.mbpId)
            .arg(res.muf >= 0 ? "+" : "")
            .arg(res.muf, 0, 'f', 2)
            .arg(res.ratio, 0, 'f', 2));
        m_historyList->addItem(item);
    }
}
void MainWindow::onHistoryClicked(QListWidgetItem* item) {
    int idx = m_historyList->row(item);
    if (idx < 0 || idx >= m_history.size()) return;
    auto& [inp, res] = m_history.at(idx);
    refreshDashboard(inp, res);
    m_stack->setCurrentIndex(1);
    ResultsPanel panel(inp, res, this);
    panel.exec();
}


void MainWindow::onShowFlowchart() {
    QString htmlPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/MUFCalc_Flowchart.html";
    QFile res(":/MUFCalc_Flowchart.html");
    if (res.open(QIODevice::ReadOnly)) {
        QFile out(htmlPath);
        if (out.open(QIODevice::WriteOnly)) { out.write(res.readAll()); out.close(); }
        res.close();
    }
    QDesktopServices::openUrl(QUrl::fromLocalFile(htmlPath));
}











void MainWindow::onShowEquations() {
    auto* dlg = new QDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowTitle("IAEA Standard Equations Reference");
    dlg->setMinimumSize(900, 620);
    dlg->setStyleSheet(Styles::appStyle());
    auto* lay = new QVBoxLayout(dlg); lay->setContentsMargins(20,20,20,20);
    auto* t = new QLabel("IAEA Standard Equations — MUF Calculation Reference");
    t->setStyleSheet("color:#1a3a5c;font-size:13px;font-weight:900;margin-bottom:8px;");
    lay->addWidget(t);
    QPixmap eq(":/equations_diagram.png");
    if (!eq.isNull()) {
        auto* l = new QLabel(); l->setPixmap(eq.scaledToWidth(860, Qt::SmoothTransformation));
        l->setAlignment(Qt::AlignCenter);
        auto* s = new QScrollArea(); s->setWidget(l);
        s->setWidgetResizable(true); s->setFrameShape(QFrame::NoFrame);
        lay->addWidget(s);
    }
    auto* cb = new QPushButton("Close"); cb->setStyleSheet(Styles::primaryButtonStyle());
    connect(cb, &QPushButton::clicked, dlg, &QDialog::accept);
    auto* br = new QHBoxLayout(); br->addStretch(); br->addWidget(cb);
    lay->addLayout(br); dlg->exec();
}
void MainWindow::onClearHistory() {
    m_history.clear(); m_historyList->clear();
    m_engine.diagnostics().resetSequentialState();
    m_stack->setCurrentIndex(0);
}
void MainWindow::onExportReport() {
    if (m_history.isEmpty()) {
        QMessageBox::information(this, "Export", "No calculations to export yet.");
        return;
    }
    int sel = m_historyList->currentRow();
    const MBPInput*  inp = (sel>=0 && sel<m_history.size()) ?
        &m_history.at(sel).first  : &m_lastInput;
    const MUFResult* res = (sel>=0 && sel<m_history.size()) ?
        &m_history.at(sel).second : &m_lastResult;
    QString path = QFileDialog::getSaveFileName(this, "Export HTML Report",
        "MUF_Report_" + inp->mbpId + ".html", "HTML (*.html)");
    if (!path.isEmpty() && m_reporter.saveHTML(path, *inp, *res))
        QMessageBox::information(this, "Exported", "Report saved:\n" + path);
}
void MainWindow::onIAEARef() {
    QMessageBox::information(this, "IAEA Reference Equations",
        "IAEA Standard Equations:\n\n"
        "1. Book Inventory:\n"
        "   IB = BI + ΣInputs − ΣOutputs\n\n"
        "2. Material Unaccounted For:\n"
        "   MUF = IB − EI\n\n"
        "3. Uncertainty Propagation:\n"
        "   σ(MUF) = √[ σ²(BI) + σ²(ΣInputs) + σ²(ΣOutputs) ]\n\n"
        "4. Uncertainty Contribution:\n"
        "   Cᵢ = [σ²(i) / σ²(MUF)] × 100%\n\n"
        "5. Significant Quantity (SQ):\n"
        "   Pu: 8 kg  |  HEU: 25 kg  |  LEU: 75 kg\n"
        "   Natural U: 10,000 kg  |  Depleted U: 20,000 kg\n\n"
        "References:\n"
        "  IAEA INFCIRC/153 (Corrected)\n"
        "  IAEA Safeguards Glossary 2022\n"
        "  IAEA Nuclear Material Accounting Handbook (2008)");
}
void MainWindow::onDiversionGuide() {
    QMessageBox::information(this, "Diversion Detection & Safeguards Glossary",
        "Key Terms (IAEA Safeguards Glossary 2022):\n\n"
        "  MUF – Material Unaccounted For\n"
        "  MBA – Material Balance Area\n"
        "  KMP – Key Measurement Point\n"
        "  SQ  – Significant Quantity\n"
        "  PIV – Physical Inventory Verification\n\n"
        "Diversion Detection Methods:\n\n"
        "1. SIGMA RATIO:  |MUF|/σ > 3 → CRITICAL\n"
        "2. SQ THRESHOLD: MUF vs IAEA Significant Quantity\n"
        "3. PAGE'S CUSUM: Protracted diversion detection\n"
        "4. SITMUF: |MUF/σ(MUF)| > 1.645 at α=0.05\n"
        "5. CUMUF: Cumulative MUF trend\n"
        "6. THROUGHPUT RATIO: |MUF|/throughput > 5%\n\n"
        "References:\n"
        "  IAEA Safeguards Glossary 2022\n"
        "  IAEA INFCIRC/153  |  NMA Handbook (2008)\n"
        "  Burr et al. (2013), Sci.&Tech.Nucl.Inst.");
}
void MainWindow::onAbout()          { AboutDialog dlg(this);      dlg.exec(); }
void MainWindow::onUserGuide()      { UserGuideDialog dlg(this);  dlg.exec(); }
void MainWindow::onVirtualFacility() {
    auto* dlg = new VirtualFacilityDialog(this);
    connect(dlg, &VirtualFacilityDialog::loadScenario, this,
            [this](const MBPInput& preloaded) {
        auto* wiz = new DataEntryWizard(this);
        wiz->loadScenario(preloaded);
        connect(wiz, &DataEntryWizard::calculateRequested, this,
                [this, wiz](const MBPInput& input) {
            auto result = m_engine.calculate(input);
            onCalculationDone(input, result);
            wiz->accept();
        });
        wiz->exec(); delete wiz;
    });
    dlg->exec(); delete dlg;
}
void MainWindow::setStatus(const QString& msg, const QString& color) {
    m_statusLbl->setText(msg);
    m_statusLbl->setStyleSheet(
        QString("color:%1;font-size:11px;"
                "font-family:'Courier New',monospace;").arg(color));
}



