#include "dataentrywizard.h"
#include "watermarkwidget.h"
#include "styles.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QFrame>
#include <QMessageBox>
#include <QScrollArea>
#include <QTimer>
#include <QPainter>
#include <QRegularExpression>
#include <cmath>

// ── Helper: light card frame ──────────────────────────────────────────
static QFrame* makeCard(const QString& bg = "white") {
    auto* f = new QFrame();
    f->setStyleSheet(QString("QFrame{background:%1;border:1px solid #d1d5db;border-radius:10px;}").arg(bg));
    return f;
}

// ── Validated field with visual feedback ──────────────────────────────
static QLineEdit* makeValidatedField(const QString& ph, int maxLen) {
    auto* le = new QLineEdit();
    le->setPlaceholderText(ph);
    le->setMaxLength(maxLen);
    le->setStyleSheet(
        "QLineEdit{background:white;border:1.5px solid #d1d5db;border-radius:6px;"
        "color:#111827;padding:8px 10px;font-size:12px;}"
        "QLineEdit:focus{border:2px solid #1a3a5c;}"
        "QLineEdit[valid='false']{border:2px solid #dc2626;background:#fef2f2;}"
        "QLineEdit[valid='true']{border:2px solid #16a34a;}");
    return le;
}

DataEntryWizard::DataEntryWizard(QWidget* parent) : QDialog(parent) {
    setWindowTitle("MUF Calculator — Data Entry");
    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
    setMinimumSize(700, 640);
    setStyleSheet(Styles::appStyle() + Styles::lineEditStyle());
    buildUI();
}

void DataEntryWizard::buildUI() {
    auto* root = new QVBoxLayout(this);
    root->setSpacing(0);
    root->setContentsMargins(0,0,0,0);

    // ── Header banner (light gray/navy) ───────────────────────────────
    auto* header = new QFrame();
    header->setStyleSheet(
        "background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #1a3a5c,stop:1 #2d5680);"
        "border-bottom:2px solid #d1d5db;");
    auto* hl = new QHBoxLayout(header);
    hl->setContentsMargins(24,14,24,14);

    // TPU logo in header
    QPixmap logo(":/tpu_logo.png");
    if (!logo.isNull()) {
        auto* logolbl = new QLabel();
        // Make logo white by painting on white-friendly surface
        QPixmap white_logo = logo.scaledToHeight(36, Qt::SmoothTransformation);
        logolbl->setPixmap(white_logo);
        logolbl->setStyleSheet("background:transparent;border:none;");
        hl->addWidget(logolbl);
        hl->addSpacing(12);
    }
    auto* titleBlock = new QVBoxLayout();
    auto* t1 = new QLabel("IAEA Nuclear Material Accounting");
    t1->setStyleSheet("color:white;font-size:13px;font-weight:800;");
    auto* t2 = new QLabel("Material Balance Period Data Entry");
    t2->setStyleSheet("color:#a5c8e8;font-size:12px;letter-spacing:1px;");
    titleBlock->addWidget(t1); titleBlock->addWidget(t2);
    hl->addLayout(titleBlock); hl->addStretch();

    // Security badge
    auto* secBadge = new QLabel("🔒  VALIDATED INPUT  |  IAEA INFCIRC/153");
    secBadge->setStyleSheet("color:#a5c8e8;font-size:12px;letter-spacing:1px;");
    hl->addWidget(secBadge);
    root->addWidget(header);

    // ── Page indicator ────────────────────────────────────────────────
    auto* indicatorBar = new QFrame();
    indicatorBar->setStyleSheet("background:#f0f2f4;border-bottom:1px solid #d1d5db;");
    auto* indLayout = new QHBoxLayout(indicatorBar);
    indLayout->setContentsMargins(20,8,20,8);
    m_pageIndicator = new QLabel();
    m_pageIndicator->setStyleSheet("color:#111827;font-size:12px;font-family:'Courier New',monospace;");

    // Step indicator dots
    auto* stepsRow = new QHBoxLayout(); stepsRow->setSpacing(6);
    for (int i = 0; i < 5; ++i) {
        auto* dot = new QLabel(QString::number(i+1));
        dot->setFixedSize(26,26);
        dot->setAlignment(Qt::AlignCenter);
        dot->setObjectName(QString("step_%1").arg(i));
        dot->setStyleSheet("background:#d1d5db;color:#111827;border-radius:13px;"
                           "font-size:12px;font-weight:700;");
        m_stepDots.append(dot);
        stepsRow->addWidget(dot);
        if (i < 4) {
            auto* line = new QFrame(); line->setFrameShape(QFrame::HLine);
            line->setStyleSheet("color:#d1d5db;"); line->setFixedWidth(20);
            stepsRow->addWidget(line);
        }
    }
    indLayout->addLayout(stepsRow);
    indLayout->addSpacing(20);
    indLayout->addWidget(m_pageIndicator);
    indLayout->addStretch();
    root->addWidget(indicatorBar);

    // ── Pages ─────────────────────────────────────────────────────────
    m_pageStack = new QStackedWidget();
    m_pageStack->setStyleSheet("background:#f2f4f6;");
    m_pageStack->addWidget(buildPage1());
    m_pageStack->addWidget(buildPage2());
    m_pageStack->addWidget(buildPage3());
    m_pageStack->addWidget(buildPage4());
    m_pageStack->addWidget(buildPage5());
    root->addWidget(m_pageStack, 1);

    // ── Validation summary bar ────────────────────────────────────────
    m_validationBar = new QLabel();
    m_validationBar->setWordWrap(true);
    m_validationBar->hide();
    m_validationBar->setStyleSheet("background:#fef2f2;color:#991b1b;border-top:2px solid #dc2626;"
                                   "padding:8px 20px;font-size:12px;");
    root->addWidget(m_validationBar);

    // ── Navigation bar ────────────────────────────────────────────────
    auto* navBar = new QFrame();
    navBar->setStyleSheet("background:white;border-top:1px solid #d1d5db;");
    auto* navL = new QHBoxLayout(navBar);
    navL->setContentsMargins(20,12,20,12); navL->setSpacing(10);

    m_btnPrev = new QPushButton("← Previous");
    m_btnPrev->setStyleSheet(Styles::secondaryButtonStyle());
    m_btnPrev->setEnabled(false);

    auto* btnLoad = new QPushButton("⚡ Load Example");
    btnLoad->setStyleSheet("QPushButton{background:#f0f7e8;color:#3d7520;border:1.5px solid #5da832;"
                           "border-radius:7px;padding:9px 16px;font-weight:600;font-size:12px;}"
                           "QPushButton:hover{background:#e0f0c8;}");

    m_btnNext = new QPushButton("Next →");
    m_btnNext->setStyleSheet(Styles::primaryButtonStyle());

    m_btnCalc = new QPushButton("▶  Calculate MUF");
    m_btnCalc->setStyleSheet(Styles::greenButtonStyle() + "QPushButton{padding:10px 28px;font-size:12px;}");
    m_btnCalc->hide();

    navL->addWidget(m_btnPrev);
    navL->addWidget(btnLoad);
    navL->addStretch();
    navL->addWidget(m_btnNext);
    navL->addWidget(m_btnCalc);

    connect(m_btnPrev, &QPushButton::clicked, this, &DataEntryWizard::prevPage);
    connect(m_btnNext, &QPushButton::clicked, this, &DataEntryWizard::nextPage);
    connect(btnLoad,   &QPushButton::clicked, this, &DataEntryWizard::loadExample);
    connect(m_btnCalc, &QPushButton::clicked, this, &DataEntryWizard::onCalculate);
    root->addWidget(navBar);

    updatePageIndicator();
}

// ═══════════════════════════════════════════════════════════════════════
//  PAGE 1 — Facility Information
// ═══════════════════════════════════════════════════════════════════════
QWidget* DataEntryWizard::buildPage1() {
    auto* scroll = new QScrollArea(); scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame); scroll->setStyleSheet("background:#f2f4f6;");
    auto* w = new QWidget(); w->setStyleSheet("background:#f2f4f6;");
    auto* lay = new QVBoxLayout(w); lay->setContentsMargins(32,28,32,28); lay->setSpacing(20);

    // Page title
    auto* titleRow = new QHBoxLayout();
    auto* pageIcon = new QLabel("🏭");
    pageIcon->setStyleSheet("font-size:30px;");
    auto* titleCol = new QVBoxLayout();
    auto* pageTitle = new QLabel("Step 1 of 5 — Facility Information");
    pageTitle->setStyleSheet("color:#1a3a5c;font-size:13px;font-weight:900;");
    auto* pageSub = new QLabel("Enter facility details, material type, and Material Balance Period identifier.");
    pageSub->setStyleSheet("color:#111827;font-size:12px;");
    titleCol->addWidget(pageTitle); titleCol->addWidget(pageSub);
    titleRow->addWidget(pageIcon); titleRow->addSpacing(10); titleRow->addLayout(titleCol); titleRow->addStretch();
    lay->addLayout(titleRow);

    auto* card = makeCard();
    auto* cardL = new QFormLayout(card);
    cardL->setContentsMargins(24,20,24,20); cardL->setSpacing(14);
    cardL->setLabelAlignment(Qt::AlignRight);

    // Helper for field labels with char count
    auto mkLbl = [](const QString& txt, const QString& hint = "") {
        auto* col = new QVBoxLayout();
        auto* l = new QLabel(txt);
        l->setStyleSheet("color:#374151;font-weight:700;font-size:12px;");
        col->addWidget(l);
        if (!hint.isEmpty()) {
            auto* h = new QLabel(hint);
            h->setStyleSheet("color:#111827;font-size:12px;");
            col->addWidget(h);
        }
        auto* w2 = new QWidget(); w2->setLayout(col); return w2;
    };

    m_facilityName = makeValidatedField("e.g. Uranium Enrichment Plant", 200);
    m_facilityId   = makeValidatedField("e.g. IAEA-FAC-2025-001", 50);
    m_operatorName = makeValidatedField("e.g. Samuel Nartey", 100);
    m_mbpId        = makeValidatedField("e.g. MBP-2025-Q1-001", 50);

    m_materialType = new QComboBox();
    m_materialType->addItems({
        "UF6 (Uranium Hexafluoride)",
        "UO2 (Uranium Dioxide)",
        "LEU (Low Enriched Uranium, <20%)",
        "HEU (Highly Enriched Uranium, ≥20%)",
        "Natural Uranium",
        "Depleted Uranium (DU)",
        "Plutonium (Pu)",
        "MOX (Mixed Oxide Fuel)",
        "U-233",
        "Thorium (Th)"
    });
    m_materialType->setStyleSheet(Styles::lineEditStyle());

    m_periodStart = new QDateEdit(QDate::currentDate().addMonths(-3));
    m_periodEnd   = new QDateEdit(QDate::currentDate());
    m_periodStart->setCalendarPopup(true);
    m_periodEnd->setCalendarPopup(true);
    m_periodStart->setDisplayFormat("yyyy-MM-dd");
    m_periodEnd->setDisplayFormat("yyyy-MM-dd");

    auto* dateRow = new QHBoxLayout();
    dateRow->addWidget(m_periodStart);
    dateRow->addWidget(new QLabel("  to  "));
    dateRow->addWidget(m_periodEnd);
    dateRow->addStretch();
    auto* dateW = new QWidget(); dateW->setLayout(dateRow);

    cardL->addRow(mkLbl("Facility Name*", "Max 200 chars, no special chars"), m_facilityName);
    cardL->addRow(mkLbl("Facility ID*",   "Max 50 chars, letters/digits/hyphens"), m_facilityId);
    cardL->addRow(mkLbl("Material Type*", "Nuclear material category (IAEA)"), m_materialType);
    cardL->addRow(mkLbl("Operator Name*", "Responsible officer"), m_operatorName);
    cardL->addRow(mkLbl("MBP Identifier*","Balance period ID"), m_mbpId);
    cardL->addRow(mkLbl("Period Dates*",  "Start — End of Material Balance Period"), dateW);
    lay->addWidget(card);

    // Wire material-type change → refresh all method dropdowns
    connect(m_materialType, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &DataEntryWizard::onMaterialTypeChanged);

    // Security note
    auto* secNote = new QFrame();
    secNote->setStyleSheet("background:#eff6ff;border:1px solid #bfdbfe;border-radius:8px;");
    auto* snL = new QHBoxLayout(secNote); snL->setContentsMargins(16,10,16,10);
    auto* snIcon = new QLabel("ℹ️"); snIcon->setStyleSheet("font-size:13px;");
    auto* snTxt = new QLabel("All fields are validated against injection attacks. Characters < > ; \" ' \\ | ` $ { } are blocked.");
    snTxt->setStyleSheet("color:#1e40af;font-size:12px;"); snTxt->setWordWrap(true);
    snL->addWidget(snIcon); snL->addWidget(snTxt,1);
    lay->addWidget(secNote);
    lay->addStretch();
    scroll->setWidget(w);
    return scroll;
}

// ═══════════════════════════════════════════════════════════════════════
//  PAGE 2 — Beginning Inventory (BI)
// ═══════════════════════════════════════════════════════════════════════
QWidget* DataEntryWizard::buildPage2() {
    auto* scroll = new QScrollArea(); scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame); scroll->setStyleSheet("background:#f2f4f6;");
    auto* w = new QWidget(); w->setStyleSheet("background:#f2f4f6;");
    auto* lay = new QVBoxLayout(w); lay->setContentsMargins(32,28,32,28); lay->setSpacing(20);

    auto* pageTitle = new QLabel("Step 2 of 5 — Beginning Inventory (BI)");
    pageTitle->setStyleSheet("color:#1a3a5c;font-size:13px;font-weight:900;");
    auto* pageSub = new QLabel("Physical inventory at the start of the balance period.");
    pageSub->setStyleSheet("color:#111827;font-size:12px;");
    lay->addWidget(pageTitle); lay->addWidget(pageSub);

    // Equation display card
    auto* eqCard = makeCard("#eff6ff");
    eqCard->setStyleSheet("QFrame{background:#eff6ff;border:1px solid #bfdbfe;border-radius:10px;}");
    auto* eqL = new QVBoxLayout(eqCard); eqL->setContentsMargins(16,12,16,12);
    auto* eqTitle = new QLabel("IAEA Equation Role:");
    eqTitle->setStyleSheet("color:#1e40af;font-size:12px;font-weight:700;letter-spacing:1px;");
    m_biEquation = new QLabel("σ²(BI) = (σ_BI)²  =  0.0000 kg²");
    m_biEquation->setStyleSheet("color:#1e40af;font-family:'Courier New',monospace;font-size:12px;font-weight:700;");
    eqL->addWidget(eqTitle); eqL->addWidget(m_biEquation);
    lay->addWidget(eqCard);

    auto* card = makeCard();
    auto* fl = new QFormLayout(card); fl->setContentsMargins(24,20,24,20); fl->setSpacing(14);
    fl->setLabelAlignment(Qt::AlignRight);

    // ── BI description: editable combo from NuclearTransferItemDB (Opening Inventory) ──
    m_biLabel = new QComboBox();
    m_biLabel->setEditable(true);
    m_biLabel->setInsertPolicy(QComboBox::NoInsert);
    m_biLabel->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
    m_biLabel->setMinimumContentsLength(20);
    m_biLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_biLabel->setMinimumWidth(340);
    m_biLabel->setStyleSheet(
        "QComboBox{background:white;border:1.5px solid #d1d5db;border-radius:6px;"
        "color:#111827;padding:6px 10px;font-size:12px;}"
        "QComboBox:focus{border:2px solid #1a3a5c;}"
        "QComboBox::drop-down{border-left:1px solid #d1d5db;width:22px;}"
        "QComboBox QAbstractItemView{font-size:11px;min-width:480px;}"
        "QComboBox QAbstractItemView::item{padding:4px 8px;min-height:22px;}");
    m_biLabel->setToolTip("Select the type of Physical Inventory at the start of this MBP,\n"
                          "or type a custom description. Items are filtered by material type.");
    NuclearTransferItemDB::populateInventoryCombo(
        m_biLabel, m_materialType->currentText(), true);
    // Inject-sanitise the editable field
    if (auto* le = m_biLabel->lineEdit()) {
        le->setMaxLength(200);
        le->setPlaceholderText("Select opening inventory type or type custom description...");
        le->setStyleSheet("background:transparent;border:none;color:#111827;"
                          "font-size:12px;padding:0px 2px;");
        connect(le, &QLineEdit::textChanged, this, [le](const QString& t) {
            static QRegularExpression blocked(R"([<>;"'\&|`$\{\}\[\] -])");
            if (t.contains(blocked)) {
                int pos = le->cursorPosition();
                QString cleaned = t; cleaned.remove(blocked);
                le->blockSignals(true); le->setText(cleaned);
                le->setCursorPosition(std::min(pos, (int)cleaned.length()));
                le->blockSignals(false);
            }
        });
    }
    // Custom sentinel clears field for typing
    connect(m_biLabel, QOverload<int>::of(&QComboBox::activated), this,
            [this](int idx) {
                if (m_biLabel->itemText(idx) ==
                        QString::fromUtf8(NuclearTransferItemDB::CUSTOM_TEXT)) {
                    m_biLabel->clearEditText();
                    if (auto* le = m_biLabel->lineEdit()) le->setFocus();
                }
            });
    m_biQty   = new QDoubleSpinBox(); m_biQty->setRange(0, 1e8); m_biQty->setDecimals(4);
    m_biQty->setSuffix(" kg"); m_biQty->setSingleStep(1.0);
    m_biErr   = new QDoubleSpinBox(); m_biErr->setRange(1e-6, 1e6); m_biErr->setDecimals(4);
    m_biErr->setSuffix(" kg (1σ)"); m_biErr->setValue(1.0); m_biErr->setSingleStep(0.1);
    m_biErr->setMinimumWidth(160);
    m_biErr->setStyleSheet(
        "QDoubleSpinBox{background:white;border:1.5px solid #d1d5db;border-radius:6px;"
        "color:#111827;padding:6px 8px;font-size:12px;}"
        "QDoubleSpinBox::up-button,QDoubleSpinBox::down-button{width:18px;}");

    // ── Type B method selector ────────────────────────────────────────
    m_biMethod = new QComboBox();
    m_biMethod->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
    m_biMethod->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_biMethod->setMinimumWidth(280);
    m_biMethod->setStyleSheet(
        "QComboBox{background:white;border:1.5px solid #d1d5db;border-radius:6px;"
        "color:#111827;padding:6px 10px;font-size:12px;}"
        "QComboBox:focus{border:2px solid #1a3a5c;}"
        "QComboBox::drop-down{border-left:1px solid #d1d5db;width:22px;}"
        "QComboBox QAbstractItemView{font-size:11px;min-width:380px;}"
        "QComboBox QAbstractItemView::item{padding:4px 8px;min-height:22px;}");
    m_biMethod->setToolTip("Select the measurement method used at this KMP. The system will compute\n"
                           "σ = ε × Q automatically from IAEA STR-368 reference values.\n"
                           "Select 'Manual Entry' to override with your own σ.");
    populateMethodCombo(m_biMethod, m_materialType->currentText());

    m_biEpsLbl = new QLabel("ε = —   (relative std uncertainty)");
    m_biEpsLbl->setStyleSheet("color:#1e40af;font-size:12px;font-family:'Courier New',monospace;");

    fl->addRow("Description:", m_biLabel);
    fl->addRow("Quantity (kg):", m_biQty);
    fl->addRow("Measurement Method:", m_biMethod);
    fl->addRow("", m_biEpsLbl);
    fl->addRow("Uncertainty 1σ (kg):", m_biErr);
    lay->addWidget(card);

    // IAEA STR-368 reference card
    auto* refCard = makeCard("#f0f9ff");
    refCard->setStyleSheet("QFrame{background:#f0f9ff;border:1px solid #bae6fd;border-radius:8px;}");
    auto* refL = new QVBoxLayout(refCard); refL->setContentsMargins(16,12,16,12); refL->setSpacing(4);
    auto* refT = new QLabel("📐  GUM / IAEA STR-368 Type B Uncertainty Evaluation");
    refT->setStyleSheet("color:#0369a1;font-weight:700;font-size:12px;");
    auto* refD = new QLabel(
        "The measurement method dropdown provides IAEA STR-368 reference relative standard\n"
        "uncertainties (ε). Selecting a method auto-computes σ = ε × Q and updates the\n"
        "uncertainty field. You may still override σ manually by choosing 'Manual Entry'.\n"
        "Ref: IAEA STR-368 Table 3.1 | GUM JCGM 100:2008 §4.3 (Type B evaluation)");
    refD->setStyleSheet("color:#0c4a6e;font-size:12px;"); refD->setWordWrap(true);
    refL->addWidget(refT); refL->addWidget(refD);
    lay->addWidget(refCard);

    // Physical inventory note
    auto* physNote = makeCard("#f0fdf4");
    physNote->setStyleSheet("QFrame{background:#f0fdf4;border:1px solid #bbf7d0;border-radius:8px;}");
    auto* pnL = new QVBoxLayout(physNote); pnL->setContentsMargins(16,12,16,12);
    auto* pnT = new QLabel("📋  Physical Inventory (PI) Guidance");
    pnT->setStyleSheet("color:#15803d;font-weight:700;font-size:12px;");
    auto* pnD = new QLabel(
        "The Beginning Inventory (BI) must be the physically verified quantity from the last "
        "Physical Inventory Verification (PIV) closing the previous MBP. Per IAEA INFCIRC/153 §3.1: "
        "inventory must be measured at Key Measurement Points (KMP) and verified by IAEA inspectors.");
    pnD->setStyleSheet("color:#166534;font-size:12px;"); pnD->setWordWrap(true);
    pnL->addWidget(pnT); pnL->addWidget(pnD);
    lay->addWidget(physNote);
    lay->addStretch();

    // Live update — equation display
    auto updateBIEq = [this](){
        double v = m_biErr->value();
        m_biEquation->setText(QString("σ²(BI) = (%1)² = %2 kg²")
            .arg(v,0,'f',4).arg(v*v,0,'f',6));
    };
    connect(m_biQty, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this, updateBIEq](double){
        applyMethodEpsilon(m_biMethod, m_biQty, m_biErr, m_biEpsLbl);
        updateBIEq();
    });
    connect(m_biErr, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, updateBIEq);
    connect(m_biMethod, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, updateBIEq](){
        applyMethodEpsilon(m_biMethod, m_biQty, m_biErr, m_biEpsLbl);
        updateBIEq();
    });
    // Initial apply
    applyMethodEpsilon(m_biMethod, m_biQty, m_biErr, m_biEpsLbl);

    scroll->setWidget(w); return scroll;
}

// ═══════════════════════════════════════════════════════════════════════
//  PAGE 3 — Inputs
// ═══════════════════════════════════════════════════════════════════════
QWidget* DataEntryWizard::buildPage3() {
    auto* w = new QWidget(); w->setStyleSheet("background:#f2f4f6;");
    auto* lay = new QVBoxLayout(w); lay->setContentsMargins(32,28,32,28); lay->setSpacing(16);

    auto* pageTitle = new QLabel("Step 3 of 5 — Material Inputs");
    pageTitle->setStyleSheet("color:#1a3a5c;font-size:13px;font-weight:900;");
    auto* pageSub = new QLabel("All nuclear material received during the balance period (transfers in).");
    pageSub->setStyleSheet("color:#111827;font-size:12px;");
    lay->addWidget(pageTitle); lay->addWidget(pageSub);

    // Equation display
    auto* eqCard = makeCard("#eff6ff");
    eqCard->setStyleSheet("QFrame{background:#eff6ff;border:1px solid #bfdbfe;border-radius:10px;}");
    auto* eqL = new QHBoxLayout(eqCard); eqL->setContentsMargins(16,10,16,10);
    m_inEquation = new QLabel("σ²(ΣInputs) = Σσ²ᵢ  =  0.0000 kg²   |   ΣInputs = 0.0000 kg");
    m_inEquation->setStyleSheet("color:#1e40af;font-family:'Courier New',monospace;font-size:12px;");
    eqL->addWidget(m_inEquation);
    lay->addWidget(eqCard);

    // Buttons row
    auto* btnRow = new QHBoxLayout(); btnRow->setSpacing(8);
    auto* addBtn = new QPushButton("＋ Add Input Row");
    addBtn->setStyleSheet(Styles::primaryButtonStyle() + "QPushButton{padding:8px 16px;}");
    auto* remBtn = new QPushButton("− Remove Selected");
    remBtn->setStyleSheet(Styles::dangerButtonStyle());
    btnRow->addWidget(addBtn); btnRow->addWidget(remBtn); btnRow->addStretch();

    // Validation notice
    auto* vlimit = new QLabel("⚠  Max 100 rows  |  Qty: 0–100,000,000 kg  |  Error: 0.000001–1,000,000 kg");
    vlimit->setStyleSheet("color:#111827;font-size:12px;");
    btnRow->addWidget(vlimit);
    lay->addLayout(btnRow);

    m_inputsTable = new QTableWidget(0, 4);
    m_inputsTable->setHorizontalHeaderLabels({"Description / Label", "Qty (kg)", "Meas. Method (STR-368)", "1σ Uncert. (kg)"});
    m_inputsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_inputsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    m_inputsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    m_inputsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    m_inputsTable->horizontalHeader()->setMinimumHeight(36);
    m_inputsTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    m_inputsTable->setColumnWidth(1, 130);
    m_inputsTable->setColumnWidth(2, 230);
    m_inputsTable->setColumnWidth(3, 190);
    m_inputsTable->horizontalHeader()->setMinimumSectionSize(80);
    m_inputsTable->verticalHeader()->setDefaultSectionSize(52);
    m_inputsTable->setWordWrap(false);
    m_inputsTable->setAlternatingRowColors(true);
    m_inputsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_inputsTable->setStyleSheet(Styles::tableStyle());
    m_inputsTable->verticalHeader()->setVisible(false);
    lay->addWidget(m_inputsTable, 1);

    // SRD note (from literature)
    auto* srdNote = makeCard("#fff7ed");
    srdNote->setStyleSheet("QFrame{background:#fff7ed;border:1px solid #fed7aa;border-radius:8px;}");
    auto* snL = new QHBoxLayout(srdNote); snL->setContentsMargins(14,10,14,10);
    auto* snT = new QLabel("📦  Shipper-Receiver Differences (SRD): Each input transfer should include the declared quantity "
                           "and your measured uncertainty. SRD = |declared − measured| is computed in the diagnostic report. "
                           "Per Burr et al. (2013), large SRDs are flagged as possible diversion indicators.");
    snT->setStyleSheet("color:#92400e;font-size:12px;"); snT->setWordWrap(true);
    snL->addWidget(snT);
    lay->addWidget(srdNote);

    connect(addBtn, &QPushButton::clicked, this, [this](){ addTableRow(m_inputsTable, "Input"); });
    connect(remBtn, &QPushButton::clicked, this, [this](){
        int row = m_inputsTable->currentRow();
        if (row >= 0) m_inputsTable->removeRow(row);
        updateEquationDisplays();
    });

    return w;
}

// ═══════════════════════════════════════════════════════════════════════
//  PAGE 4 — Outputs
// ═══════════════════════════════════════════════════════════════════════
QWidget* DataEntryWizard::buildPage4() {
    auto* w = new QWidget(); w->setStyleSheet("background:#f2f4f6;");
    auto* lay = new QVBoxLayout(w); lay->setContentsMargins(32,28,32,28); lay->setSpacing(16);

    auto* pageTitle = new QLabel("Step 4 of 5 — Material Outputs");
    pageTitle->setStyleSheet("color:#1a3a5c;font-size:13px;font-weight:900;");
    auto* pageSub = new QLabel("All nuclear material shipped during the balance period (transfers out).");
    pageSub->setStyleSheet("color:#111827;font-size:12px;");
    lay->addWidget(pageTitle); lay->addWidget(pageSub);

    auto* eqCard = makeCard("#eff6ff");
    eqCard->setStyleSheet("QFrame{background:#eff6ff;border:1px solid #bfdbfe;border-radius:10px;}");
    auto* eqL = new QHBoxLayout(eqCard); eqL->setContentsMargins(16,10,16,10);
    m_outEquation = new QLabel("σ²(ΣOutputs) = Σσ²ᵢ  =  0.0000 kg²   |   ΣOutputs = 0.0000 kg");
    m_outEquation->setStyleSheet("color:#1e40af;font-family:'Courier New',monospace;font-size:12px;");
    eqL->addWidget(m_outEquation);
    lay->addWidget(eqCard);

    auto* btnRow = new QHBoxLayout(); btnRow->setSpacing(8);
    auto* addBtn = new QPushButton("＋ Add Output Row");
    addBtn->setStyleSheet(Styles::primaryButtonStyle() + "QPushButton{padding:8px 16px;}");
    auto* remBtn = new QPushButton("− Remove Selected");
    remBtn->setStyleSheet(Styles::dangerButtonStyle());
    btnRow->addWidget(addBtn); btnRow->addWidget(remBtn); btnRow->addStretch();
    auto* vlimit = new QLabel("⚠  Include: product, waste, hold-up, scrap, transfers");
    vlimit->setStyleSheet("color:#111827;font-size:12px;");
    btnRow->addWidget(vlimit);
    lay->addLayout(btnRow);

    m_outputsTable = new QTableWidget(0, 4);
    m_outputsTable->setHorizontalHeaderLabels({"Description / Label", "Qty (kg)", "Meas. Method (STR-368)", "1σ Uncert. (kg)"});
    m_outputsTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_outputsTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    m_outputsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Fixed);
    m_outputsTable->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    m_outputsTable->horizontalHeader()->setMinimumHeight(36);
    m_outputsTable->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    m_outputsTable->setColumnWidth(1, 130);
    m_outputsTable->setColumnWidth(2, 230);
    m_outputsTable->setColumnWidth(3, 190);
    m_outputsTable->horizontalHeader()->setMinimumSectionSize(80);
    m_outputsTable->verticalHeader()->setDefaultSectionSize(52);
    m_outputsTable->setWordWrap(false);
    m_outputsTable->setAlternatingRowColors(true);
    m_outputsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_outputsTable->setStyleSheet(Styles::tableStyle());
    m_outputsTable->verticalHeader()->setVisible(false);
    lay->addWidget(m_outputsTable, 1);

    auto* outNote = makeCard("#f0fdf4");
    outNote->setStyleSheet("QFrame{background:#f0fdf4;border:1px solid #bbf7d0;border-radius:8px;}");
    auto* onL = new QHBoxLayout(outNote); onL->setContentsMargins(14,10,14,10);
    auto* onT = new QLabel("📤  Include ALL material transfers out: enriched product, tails, waste streams, process "
                           "hold-up, samples, and transfers to other facilities. Missing outputs cause artificially "
                           "negative MUF — a potential false diversion alarm per IAEA-TECDOC-1169.");
    onT->setStyleSheet("color:#166534;font-size:12px;"); onT->setWordWrap(true);
    onL->addWidget(onT);
    lay->addWidget(outNote);

    connect(addBtn, &QPushButton::clicked, this, [this](){ addTableRow(m_outputsTable, "Output"); });
    connect(remBtn, &QPushButton::clicked, this, [this](){
        int row = m_outputsTable->currentRow();
        if (row >= 0) m_outputsTable->removeRow(row);
        updateEquationDisplays();
    });

    return w;
}

// ═══════════════════════════════════════════════════════════════════════
//  PAGE 5 — Ending Inventory (EI)
// ═══════════════════════════════════════════════════════════════════════
QWidget* DataEntryWizard::buildPage5() {
    auto* scroll = new QScrollArea(); scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame); scroll->setStyleSheet("background:#f2f4f6;");
    auto* w = new QWidget(); w->setStyleSheet("background:#f2f4f6;");
    auto* lay = new QVBoxLayout(w); lay->setContentsMargins(32,28,32,28); lay->setSpacing(20);

    auto* pageTitle = new QLabel("Step 5 of 5 — Ending Inventory (EI)");
    pageTitle->setStyleSheet("color:#1a3a5c;font-size:13px;font-weight:900;");
    auto* pageSub = new QLabel("Physical inventory at the end of the balance period (measured).");
    pageSub->setStyleSheet("color:#111827;font-size:12px;");
    lay->addWidget(pageTitle); lay->addWidget(pageSub);

    // Full equation preview
    auto* eqCard = makeCard("#eff6ff");
    eqCard->setStyleSheet("QFrame{background:#eff6ff;border:1px solid #bfdbfe;border-radius:10px;}");
    auto* eqL = new QVBoxLayout(eqCard); eqL->setContentsMargins(16,12,16,12); eqL->setSpacing(4);
    auto* eqTitleLbl = new QLabel("LIVE EQUATION PREVIEW:");
    eqTitleLbl->setStyleSheet("color:#1e40af;font-size:12px;font-weight:700;letter-spacing:1px;");
    m_eiEquation = new QLabel("Awaiting EI entry...");
    m_eiEquation->setStyleSheet("color:#1e40af;font-family:'Courier New',monospace;font-size:12px;font-weight:700;");
    m_eiEquation->setWordWrap(true);
    eqL->addWidget(eqTitleLbl); eqL->addWidget(m_eiEquation);
    lay->addWidget(eqCard);

    auto* card = makeCard();
    auto* fl = new QFormLayout(card); fl->setContentsMargins(24,20,24,20); fl->setSpacing(14);
    fl->setLabelAlignment(Qt::AlignRight);

    // ── EI description: editable combo from NuclearTransferItemDB (Closing Inventory) ──
    m_eiLabel = new QComboBox();
    m_eiLabel->setEditable(true);
    m_eiLabel->setInsertPolicy(QComboBox::NoInsert);
    m_eiLabel->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
    m_eiLabel->setMinimumContentsLength(20);
    m_eiLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_eiLabel->setMinimumWidth(340);
    m_eiLabel->setStyleSheet(
        "QComboBox{background:white;border:1.5px solid #d1d5db;border-radius:6px;"
        "color:#111827;padding:6px 10px;font-size:12px;}"
        "QComboBox:focus{border:2px solid #1a3a5c;}"
        "QComboBox::drop-down{border-left:1px solid #d1d5db;width:22px;}"
        "QComboBox QAbstractItemView{font-size:11px;min-width:480px;}"
        "QComboBox QAbstractItemView::item{padding:4px 8px;min-height:22px;}");
    m_eiLabel->setToolTip("Select the type of Physical Inventory at the end of this MBP,\n"
                          "or type a custom description. Items are filtered by material type.");
    NuclearTransferItemDB::populateInventoryCombo(
        m_eiLabel, m_materialType->currentText(), false);
    if (auto* le = m_eiLabel->lineEdit()) {
        le->setMaxLength(200);
        le->setPlaceholderText("Select closing inventory type or type custom description...");
        le->setStyleSheet("background:transparent;border:none;color:#111827;"
                          "font-size:12px;padding:0px 2px;");
        connect(le, &QLineEdit::textChanged, this, [le](const QString& t) {
            static QRegularExpression blocked(R"([<>;"'\&|`$\{\}\[\] -])");
            if (t.contains(blocked)) {
                int pos = le->cursorPosition();
                QString cleaned = t; cleaned.remove(blocked);
                le->blockSignals(true); le->setText(cleaned);
                le->setCursorPosition(std::min(pos, (int)cleaned.length()));
                le->blockSignals(false);
            }
        });
    }
    connect(m_eiLabel, QOverload<int>::of(&QComboBox::activated), this,
            [this](int idx) {
                if (m_eiLabel->itemText(idx) ==
                        QString::fromUtf8(NuclearTransferItemDB::CUSTOM_TEXT)) {
                    m_eiLabel->clearEditText();
                    if (auto* le = m_eiLabel->lineEdit()) le->setFocus();
                }
            });
    m_eiQty   = new QDoubleSpinBox(); m_eiQty->setRange(0, 1e8); m_eiQty->setDecimals(4);
    m_eiQty->setSuffix(" kg"); m_eiQty->setSingleStep(1.0);
    m_eiErr   = new QDoubleSpinBox(); m_eiErr->setRange(1e-6, 1e6); m_eiErr->setDecimals(4);
    m_eiErr->setSuffix(" kg (1σ)"); m_eiErr->setValue(1.0); m_eiErr->setSingleStep(0.1);
    m_eiErr->setMinimumWidth(160);
    m_eiErr->setStyleSheet(
        "QDoubleSpinBox{background:white;border:1.5px solid #d1d5db;border-radius:6px;"
        "color:#111827;padding:6px 8px;font-size:12px;}"
        "QDoubleSpinBox::up-button,QDoubleSpinBox::down-button{width:18px;}");

    // ── Type B method selector (EI) ───────────────────────────────────
    m_eiMethod = new QComboBox();
    m_eiMethod->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
    m_eiMethod->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_eiMethod->setMinimumWidth(280);
    m_eiMethod->setStyleSheet(
        "QComboBox{background:white;border:1.5px solid #d1d5db;border-radius:6px;"
        "color:#111827;padding:6px 10px;font-size:12px;}"
        "QComboBox:focus{border:2px solid #1a3a5c;}"
        "QComboBox::drop-down{border-left:1px solid #d1d5db;width:22px;}"
        "QComboBox QAbstractItemView{font-size:11px;min-width:380px;}"
        "QComboBox QAbstractItemView::item{padding:4px 8px;min-height:22px;}");
    m_eiMethod->setToolTip("IAEA STR-368 Type B measurement method.\n"
                           "Selects reference ε and auto-computes σ = ε × Q.");
    populateMethodCombo(m_eiMethod, m_materialType->currentText());

    m_eiEpsLbl = new QLabel("ε = —   (relative std uncertainty)");
    m_eiEpsLbl->setStyleSheet("color:#1e40af;font-size:12px;font-family:'Courier New',monospace;");

    fl->addRow("Description:", m_eiLabel);
    fl->addRow("Quantity (kg):", m_eiQty);
    fl->addRow("Measurement Method:", m_eiMethod);
    fl->addRow("", m_eiEpsLbl);
    fl->addRow("Uncertainty 1σ (kg):", m_eiErr);
    lay->addWidget(card);

    // SQ reference card
    auto* sqCard = makeCard("#fff7ed");
    sqCard->setStyleSheet("QFrame{background:#fff7ed;border:1.5px solid #f59e0b;border-radius:10px;}");
    auto* sqL = new QVBoxLayout(sqCard); sqL->setContentsMargins(16,12,16,12); sqL->setSpacing(4);
    auto* sqTitle = new QLabel("⚛  IAEA Significant Quantities (SQ) — Diversion Detection Thresholds");
    sqTitle->setStyleSheet("color:#92400e;font-weight:700;font-size:12px;");
    auto* sqData = new QLabel(
        "Plutonium (Pu / MOX): 8 kg  |  U-233: 8 kg  |  HEU (≥20% U-235): 25 kg\n"
        "LEU / UF6 / UO2 (<20%): 75 kg  |  Natural U: 10,000 kg  |  Depleted U: 20,000 kg\n"
        "Source: IAEA Safeguards Glossary 2022, §3.19  —  Ref: INFCIRC/153 Art.25");
    sqData->setStyleSheet("color:#78350f;font-size:12px;font-family:'Courier New',monospace;");
    sqL->addWidget(sqTitle); sqL->addWidget(sqData);
    lay->addWidget(sqCard);
    lay->addStretch();

    // Live update for full preview
    auto updateFull = [this](){
        double bi    = m_biQty->value(), sbi = m_biErr->value();
        double ei    = m_eiQty->value();
        double sei   = m_eiErr->value(); Q_UNUSED(sei)
        double sumIn = 0, varIn = 0;
        for (int r = 0; r < m_inputsTable->rowCount(); ++r) {
            auto* qs = qobject_cast<QDoubleSpinBox*>(m_inputsTable->cellWidget(r,1));
            auto* es = qobject_cast<QDoubleSpinBox*>(m_inputsTable->cellWidget(r,3));
            if (qs) sumIn += qs->value();
            if (es) varIn += es->value()*es->value();
        }
        double sumOut = 0, varOut = 0;
        for (int r = 0; r < m_outputsTable->rowCount(); ++r) {
            auto* qs = qobject_cast<QDoubleSpinBox*>(m_outputsTable->cellWidget(r,1));
            auto* es = qobject_cast<QDoubleSpinBox*>(m_outputsTable->cellWidget(r,3));
            if (qs) sumOut += qs->value();
            if (es) varOut += es->value()*es->value();
        }
        double IB       = bi + sumIn - sumOut;
        double MUF      = IB - ei;
        double varMUF   = sbi*sbi + varIn + varOut; // IAEA: σ²(MUF)=σ²(BI)+σ²(ΣIn)+σ²(ΣOut) — EI uncertainty not included
        double sigmaMUF = std::sqrt(varMUF);
        m_eiEquation->setText(QString(
            "IB = BI + ΣIn − ΣOut  =  %1 + %2 − %3  =  %4 kg\n"
            "MUF = IB − EI  =  %4 − %5  =  %6 kg\n"
            "σ(MUF) = √[σ²(BI)+σ²(ΣIn)+σ²(ΣOut)]  =  %7 kg")
            .arg(bi,0,'f',3).arg(sumIn,0,'f',3).arg(sumOut,0,'f',3)
            .arg(IB,0,'f',4).arg(ei,0,'f',4).arg(MUF,0,'f',4).arg(sigmaMUF,0,'f',4));
    };
    connect(m_eiQty, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, [this, updateFull](double){
        applyMethodEpsilon(m_eiMethod, m_eiQty, m_eiErr, m_eiEpsLbl);
        updateFull();
    });
    connect(m_eiErr, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, updateFull);
    connect(m_eiMethod, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, updateFull](){
        applyMethodEpsilon(m_eiMethod, m_eiQty, m_eiErr, m_eiEpsLbl);
        updateFull();
    });
    // Initial apply
    applyMethodEpsilon(m_eiMethod, m_eiQty, m_eiErr, m_eiEpsLbl);

    scroll->setWidget(w); return scroll;
}

// ═══════════════════════════════════════════════════════════════════════
//  Table helpers
// ═══════════════════════════════════════════════════════════════════════
void DataEntryWizard::addTableRow(QTableWidget* tbl, const QString& prefix) {
    if (tbl->rowCount() >= 100) {
        QMessageBox::warning(this, "Row Limit", "Maximum 100 rows per table (security limit).");
        return;
    }
    const bool isInput = (prefix == "Input");
    int row = tbl->rowCount();
    tbl->insertRow(row);

    // ── Col 0: Transfer item description (editable combo + predefined list) ──
    auto* descCombo = new QComboBox();
    descCombo->setEditable(true);
    descCombo->setInsertPolicy(QComboBox::NoInsert);
    // AdjustToContentsOnFirstShow lets the cell control width without clipping text
    descCombo->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
    descCombo->setMinimumContentsLength(12);
    descCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    descCombo->setStyleSheet(
        "QComboBox{background:white;border:1px solid #d1d5db;border-radius:4px;"
        "padding:4px 6px 4px 8px;color:#111827;font-size:11px;}"
        "QComboBox:focus{border:1.5px solid #1a3a5c;}"
        "QComboBox::drop-down{border-left:1px solid #d1d5db;width:22px;}"
        "QComboBox QAbstractItemView{font-size:11px;min-width:460px;}"
        "QComboBox QAbstractItemView::item{padding:4px 8px;min-height:22px;}"
        "QComboBox QAbstractItemView::item[separator=\"true\"]{color:#6b7280;font-style:italic;}");

    // Populate from IAEA NMA item database
    NuclearTransferItemDB::populateCombo(
        descCombo, m_materialType->currentText(), isInput, QString());

    // Line-edit part of the combo: injection sanitiser
    if (auto* le = descCombo->lineEdit()) {
        le->setMaxLength(200);
        le->setPlaceholderText(isInput ? "Select receipt type or type custom description..."
                                       : "Select shipment type or type custom description...");
        le->setStyleSheet("background:transparent;border:none;color:#111827;"
                          "font-size:11px;padding:0px 2px;");
        connect(le, &QLineEdit::textChanged, this, [le](const QString& t) {
            static QRegularExpression blocked(R"([<>;"'\\&|`$\{\}\[\]\x00-\x1F])");
            if (t.contains(blocked)) {
                int pos = le->cursorPosition();
                QString cleaned = t; cleaned.remove(blocked);
                le->blockSignals(true); le->setText(cleaned);
                le->setCursorPosition(std::min(pos, (int)cleaned.length()));
                le->blockSignals(false);
            }
        });
    }

    // When user selects the "Custom…" sentinel, clear the line edit for typing
    connect(descCombo, QOverload<int>::of(&QComboBox::activated), this,
            [descCombo](int idx) {
                if (descCombo->itemText(idx) ==
                        QString::fromUtf8(NuclearTransferItemDB::CUSTOM_TEXT)) {
                    descCombo->clearEditText();
                    descCombo->lineEdit()->setFocus();
                }
            });

    // ── Col 1: Quantity ─────────────────────────────────────────────
    auto* qty = new QDoubleSpinBox();
    qty->setRange(0, 1e8); qty->setDecimals(4); qty->setSuffix(" kg");
    qty->setMinimumWidth(100);
    qty->setStyleSheet(
        "QDoubleSpinBox{background:white;border:1px solid #d1d5db;"
        "border-radius:4px;padding:3px 4px;color:#111827;font-size:11px;}"
        "QDoubleSpinBox::up-button,QDoubleSpinBox::down-button{width:16px;}");

    // ── Col 2: Measurement Method (IAEA STR-368 Type B) ─────────────
    auto* methodCombo = new QComboBox();
    methodCombo->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
    methodCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    methodCombo->setStyleSheet(
        "QComboBox{background:white;border:1px solid #d1d5db;border-radius:4px;"
        "padding:4px 6px;color:#111827;font-size:11px;}"
        "QComboBox:focus{border:1.5px solid #1a3a5c;}"
        "QComboBox::drop-down{border-left:1px solid #d1d5db;width:18px;}"
        "QComboBox QAbstractItemView{font-size:11px;min-width:280px;}"
        "QComboBox QAbstractItemView::item{padding:4px 6px;min-height:20px;}");
    populateMethodCombo(methodCombo, m_materialType->currentText());

    // ── Col 3: 1σ uncertainty (auto-filled when method ≠ manual) ────
    auto* err = new QDoubleSpinBox();
    err->setRange(1e-6, 1e6); err->setDecimals(4); err->setSuffix(" kg");
    err->setValue(1.0);
    err->setMinimumWidth(110);
    err->setStyleSheet(
        "QDoubleSpinBox{background:white;border:1px solid #d1d5db;"
        "border-radius:4px;padding:3px 4px;color:#111827;font-size:11px;}"
        "QDoubleSpinBox::up-button,QDoubleSpinBox::down-button{width:16px;}");

    // Wire: qty or method change → auto-compute σ
    auto autoErr = [this, qty, methodCombo, err]() {
        const QString key = methodCombo->currentData().toString();
        if (key == "manual") {
            err->setReadOnly(false);
            err->setStyleSheet("background:white;border:1px solid #d1d5db;border-radius:4px;");
            err->setToolTip("Manual entry — type your 1σ uncertainty directly");
            return;
        }
        MeasurementMethod m = MeasurementMethodDB::findByKey(key, m_materialType->currentText());
        double sigma = m.epsilonNominal * qty->value();
        if (sigma < 1e-6) sigma = 1e-6;
        err->blockSignals(true); err->setValue(sigma); err->blockSignals(false);
        err->setReadOnly(true);
        err->setStyleSheet("background:#eff6ff;border:1px solid #93c5fd;border-radius:4px;color:#1e40af;");
        err->setToolTip(QString("Auto (STR-368): ε=%1 (%2%) × %3 kg = %4 kg  |  range [%5%,%6%]  |  %7")
            .arg(m.epsilonNominal,0,'f',4).arg(m.epsilonNominal*100,0,'f',2)
            .arg(qty->value(),0,'f',3).arg(sigma,0,'f',4)
            .arg(m.epsilonMin*100,0,'f',2).arg(m.epsilonMax*100,0,'f',2).arg(m.iaeaRef));
        updateEquationDisplays();
    };
    connect(qty,         QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [autoErr, this](double) { autoErr(); updateEquationDisplays(); });
    connect(methodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [autoErr]() { autoErr(); });
    connect(err,         QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &DataEntryWizard::updateEquationDisplays);

    tbl->setCellWidget(row, 0, descCombo);
    tbl->setCellWidget(row, 1, qty);
    tbl->setCellWidget(row, 2, methodCombo);
    tbl->setCellWidget(row, 3, err);
    tbl->setRowHeight(row, 52);
    tbl->verticalHeader()->setMinimumSectionSize(52);

    autoErr();
    updateEquationDisplays();
}


void DataEntryWizard::updateEquationDisplays() {
    // Inputs
    double sumIn = 0, varIn = 0;
    for (int r = 0; r < m_inputsTable->rowCount(); ++r) {
        auto* qs = qobject_cast<QDoubleSpinBox*>(m_inputsTable->cellWidget(r,1));
        auto* es = qobject_cast<QDoubleSpinBox*>(m_inputsTable->cellWidget(r,3));
        if (qs) sumIn += qs->value();
        if (es) varIn += es->value()*es->value();
    }
    m_inEquation->setText(QString("σ²(ΣInputs) = Σσ²ᵢ = %1 kg²   |   ΣInputs = %2 kg")
        .arg(varIn,0,'f',4).arg(sumIn,0,'f',4));
    // Outputs
    double sumOut = 0, varOut = 0;
    for (int r = 0; r < m_outputsTable->rowCount(); ++r) {
        auto* qs = qobject_cast<QDoubleSpinBox*>(m_outputsTable->cellWidget(r,1));
        auto* es = qobject_cast<QDoubleSpinBox*>(m_outputsTable->cellWidget(r,3));
        if (qs) sumOut += qs->value();
        if (es) varOut += es->value()*es->value();
    }
    m_outEquation->setText(QString("σ²(ΣOutputs) = Σσ²ᵢ = %1 kg²   |   ΣOutputs = %2 kg")
        .arg(varOut,0,'f',4).arg(sumOut,0,'f',4));
}

// ═══════════════════════════════════════════════════════════════════════
//  Navigation
// ═══════════════════════════════════════════════════════════════════════
void DataEntryWizard::updatePageIndicator() {
    int p = m_pageStack->currentIndex();
    QStringList names{"Facility Info","Beginning Inv.","Inputs","Outputs","Ending Inv."};
    m_pageIndicator->setText(QString("Step %1 of 5  —  %2").arg(p+1).arg(names[p]));
    m_btnPrev->setEnabled(p > 0);
    m_btnNext->setVisible(p < 4);
    m_btnCalc->setVisible(p == 4);

    // Update step dots
    for (int i = 0; i < m_stepDots.size(); ++i) {
        if (i < p)
            m_stepDots[i]->setStyleSheet("background:#16a34a;color:white;border-radius:13px;"
                                         "font-size:12px;font-weight:700;");
        else if (i == p)
            m_stepDots[i]->setStyleSheet("background:#1a3a5c;color:white;border-radius:13px;"
                                         "font-size:12px;font-weight:700;");
        else
            m_stepDots[i]->setStyleSheet("background:#d1d5db;color:#111827;border-radius:13px;"
                                         "font-size:12px;font-weight:700;");
    }
    m_validationBar->hide();
}

void DataEntryWizard::nextPage() {
    if (!validateCurrentPage()) return;
    if (m_pageStack->currentIndex() < 4) {
        m_pageStack->setCurrentIndex(m_pageStack->currentIndex()+1);
        updatePageIndicator();
    }
}

void DataEntryWizard::prevPage() {
    if (m_pageStack->currentIndex() > 0) {
        m_pageStack->setCurrentIndex(m_pageStack->currentIndex()-1);
        updatePageIndicator();
    }
}

// ═══════════════════════════════════════════════════════════════════════
//  Per-page validation with visual feedback
// ═══════════════════════════════════════════════════════════════════════
static bool validateField(QLineEdit* le, const QString& name, QStringList& errs,
                           bool required = true, int maxLen = 200) {
    QString v = le->text().trimmed();
    static QRegularExpression blocked(R"([<>;"'\\&|`$\{\}\[\]\(\)\x00-\x1F])");
    bool ok = true;
    if (required && v.isEmpty()) { errs << name + ": required."; ok = false; }
    else if (v.length() > maxLen){ errs << name + QString(": max %1 chars.").arg(maxLen); ok = false; }
    else if (!v.isEmpty() && v.contains(blocked)) { errs << name + ": contains invalid characters."; ok = false; }
    le->setProperty("valid", ok ? "true" : "false");
    le->style()->unpolish(le); le->style()->polish(le);
    return ok;
}

bool DataEntryWizard::validateCurrentPage() {
    int p = m_pageStack->currentIndex();
    QStringList errs;
    bool ok = true;
    if (p == 0) {
        ok &= validateField(m_facilityName, "Facility Name", errs, true, 200);
        ok &= validateField(m_facilityId,   "Facility ID",   errs, true, 50);
        ok &= validateField(m_operatorName, "Operator Name", errs, true, 100);
        ok &= validateField(m_mbpId,        "MBP ID",        errs, true, 50);
        // Check alphanumeric for IDs
        static QRegularExpression idOk(R"(^[A-Za-z0-9\-_\./\s]+$)");
        if (!m_facilityId->text().trimmed().isEmpty() && !idOk.match(m_facilityId->text().trimmed()).hasMatch()) {
            errs << "Facility ID: only letters, digits, hyphens, underscores allowed.";
            m_facilityId->setProperty("valid","false"); m_facilityId->style()->unpolish(m_facilityId); m_facilityId->style()->polish(m_facilityId);
            ok = false;
        }
        if (!m_mbpId->text().trimmed().isEmpty() && !idOk.match(m_mbpId->text().trimmed()).hasMatch()) {
            errs << "MBP ID: only letters, digits, hyphens, underscores allowed.";
            m_mbpId->setProperty("valid","false"); m_mbpId->style()->unpolish(m_mbpId); m_mbpId->style()->polish(m_mbpId);
            ok = false;
        }
        // Date validation
        QDate s = m_periodStart->date(), e = m_periodEnd->date();
        if (!s.isValid() || !e.isValid())    { errs << "Invalid dates."; ok = false; }
        else if (s >= e)                     { errs << "Period: end date must be after start date."; ok = false; }
        else if (s.daysTo(e) > 5*365)        { errs << "Period: exceeds 5 years — verify dates."; ok = false; }
        else if (e > QDate::currentDate())   { errs << "Period end: cannot be in the future."; ok = false; }
    } else if (p == 1) {
        if (m_biErr->value() > m_biQty->value() * 0.5 && m_biQty->value() > 0)
            errs << "BI Uncertainty exceeds 50% of quantity — physically unrealistic.";
        if (!errs.isEmpty()) ok = false;
    } else if (p == 2) {
        if (m_inputsTable->rowCount() == 0 && m_outputsTable->rowCount() == 0)
            errs << "At least one input or output transfer is required.";
        // Validate each row
        for (int r = 0; r < m_inputsTable->rowCount(); ++r) {
            auto* qs = qobject_cast<QDoubleSpinBox*>(m_inputsTable->cellWidget(r,1));
            auto* es = qobject_cast<QDoubleSpinBox*>(m_inputsTable->cellWidget(r,3));
            if (qs && es && qs->value() > 0 && es->value() > qs->value()*0.5)
                errs << QString("Input row %1: uncertainty >50% of quantity.").arg(r+1);
        }
        if (!errs.isEmpty()) ok = false;
    } else if (p == 3) {
        for (int r = 0; r < m_outputsTable->rowCount(); ++r) {
            auto* qs = qobject_cast<QDoubleSpinBox*>(m_outputsTable->cellWidget(r,1));
            auto* es = qobject_cast<QDoubleSpinBox*>(m_outputsTable->cellWidget(r,3));
            if (qs && es && qs->value() > 0 && es->value() > qs->value()*0.5)
                errs << QString("Output row %1: uncertainty >50% of quantity.").arg(r+1);
        }
        if (!errs.isEmpty()) ok = false;
    } else if (p == 4) {
        if (m_eiErr->value() > m_eiQty->value() * 0.5 && m_eiQty->value() > 0)
            errs << "EI Uncertainty exceeds 50% of quantity — physically unrealistic.";
        if (!errs.isEmpty()) ok = false;
    }

    if (!errs.isEmpty()) {
        m_validationBar->setText("⚠  " + errs.join("   |   "));
        m_validationBar->show();
    }
    return ok;
}

// ═══════════════════════════════════════════════════════════════════════
//  Calculate
// ═══════════════════════════════════════════════════════════════════════
void DataEntryWizard::onCalculate() {
    if (!validateCurrentPage()) return;
    emit calculateRequested(collectInput());
    accept();
}

MBPInput DataEntryWizard::collectInput() const {
    MBPInput inp;
    inp.facilityName  = m_facilityName->text().trimmed();
    inp.facilityId    = m_facilityId->text().trimmed();
    inp.materialType  = m_materialType->currentText();
    inp.operatorName  = m_operatorName->text().trimmed();
    inp.mbpId         = m_mbpId->text().trimmed();
    inp.periodStart   = QDateTime(m_periodStart->date(), QTime(0,0,0));
    inp.periodEnd     = QDateTime(m_periodEnd->date(),   QTime(23,59,59));

    inp.beginningInventory = {m_biLabel->currentText().trimmed(), m_biQty->value(), m_biErr->value()};
    inp.endingInventory    = {m_eiLabel->currentText().trimmed(), m_eiQty->value(), m_eiErr->value()};

    for (int r = 0; r < m_inputsTable->rowCount(); ++r) {
        auto* cb = qobject_cast<QComboBox*>(m_inputsTable->cellWidget(r,0));
        auto* qs  = qobject_cast<QDoubleSpinBox*>(m_inputsTable->cellWidget(r,1));
        auto* es  = qobject_cast<QDoubleSpinBox*>(m_inputsTable->cellWidget(r,3));
        if (qs) inp.inputs.append({cb?cb->currentText().trimmed():QString(), qs->value(), es?es->value():1.0});
    }
    for (int r = 0; r < m_outputsTable->rowCount(); ++r) {
        auto* cb = qobject_cast<QComboBox*>(m_outputsTable->cellWidget(r,0));
        auto* qs  = qobject_cast<QDoubleSpinBox*>(m_outputsTable->cellWidget(r,1));
        auto* es  = qobject_cast<QDoubleSpinBox*>(m_outputsTable->cellWidget(r,3));
        if (qs) inp.outputs.append({cb?cb->currentText().trimmed():QString(), qs->value(), es?es->value():1.0});
    }
    return inp;
}

// ═══════════════════════════════════════════════════════════════════════
//  Load Example (UF6 enrichment plant)
// ═══════════════════════════════════════════════════════════════════════
void DataEntryWizard::loadExample() {
    m_facilityName->setText("Uranium Enrichment Plant — Lab Demo");
    m_facilityId->setText("IAEA-FAC-2025-DEMO");
    m_materialType->setCurrentText("UF6 (Uranium Hexafluoride)");
    m_operatorName->setText("Nartey Samuel Teye");
    m_mbpId->setText("MBP-2025-Q1-001");
    m_periodStart->setDate(QDate(2025,1,1));
    m_periodEnd->setDate(QDate(2025,3,31));

    m_biLabel->setCurrentText("Natural UF6 Feed Cylinders (KMP-A)");
    m_biQty->setValue(5000.0);
    // Set method to "Cylinder Weighing" — nominal ε for UF6 cylinder weighing
    { int idx = m_biMethod->findData("uf6_cw"); if (idx>=0) m_biMethod->setCurrentIndex(idx); }
    // applyMethodEpsilon will update m_biErr automatically via signal

    // Inputs
    m_inputsTable->setRowCount(0);
    struct Row { QString n; double q, e; };
    QVector<Row> ins = {
        {"Batch-1: Natural UF6 Feed",    2000.0, 5.0  },
        {"Batch-2: Natural UF6 Feed",    1500.0, 3.75 },
        {"Batch-3: Recycle UF6 Stream",   800.0, 2.0  },
    };
    for (auto& row : ins) {
        addTableRow(m_inputsTable, "Input");
        int r = m_inputsTable->rowCount()-1;
        if (auto* w = qobject_cast<QComboBox*>(m_inputsTable->cellWidget(r,0)))       w->setCurrentText(row.n);
        if (auto* w = qobject_cast<QDoubleSpinBox*>(m_inputsTable->cellWidget(r,1)))  w->setValue(row.q);
        if (auto* w = qobject_cast<QDoubleSpinBox*>(m_inputsTable->cellWidget(r,3)))  w->setValue(row.e);
    }

    // Outputs
    m_outputsTable->setRowCount(0);
    QVector<Row> outs = {
        {"Enriched UF6 Product (4.0% U-235)", 3200.0, 8.0   },
        {"Depleted UF6 Tails (KMP-D)",        1650.0, 4.125 },
        {"Process Hold-up / Maintenance",        25.0, 0.5   },
    };
    for (auto& row : outs) {
        addTableRow(m_outputsTable, "Output");
        int r = m_outputsTable->rowCount()-1;
        if (auto* w = qobject_cast<QComboBox*>(m_outputsTable->cellWidget(r,0)))      w->setCurrentText(row.n);
        if (auto* w = qobject_cast<QDoubleSpinBox*>(m_outputsTable->cellWidget(r,1))) w->setValue(row.q);
        if (auto* w = qobject_cast<QDoubleSpinBox*>(m_outputsTable->cellWidget(r,3))) w->setValue(row.e);
    }

    m_eiLabel->setCurrentText("Residual UF6 Inventory (Physical PIV)");
    m_eiQty->setValue(4234.75);
    { int idx = m_eiMethod->findData("uf6_cw"); if (idx>=0) m_eiMethod->setCurrentIndex(idx); }
    updateEquationDisplays();
}

void DataEntryWizard::loadScenario(const MBPInput& inp) {
    // ── Page 1: Facility ────────────────────────────────────────────
    m_facilityName->setText(inp.facilityName);
    m_facilityId->setText(inp.facilityId);
    m_materialType->setCurrentText(inp.materialType);
    m_operatorName->setText(inp.operatorName);
    m_mbpId->setText(inp.mbpId);
    m_periodStart->setDate(inp.periodStart.date());
    m_periodEnd->setDate(inp.periodEnd.date());

    // ── Page 2: Beginning Inventory ────────────────────────────────
    m_biLabel->setCurrentText(inp.beginningInventory.label);
    m_biQty->setValue(inp.beginningInventory.qty);
    m_biErr->setValue(inp.beginningInventory.error);

    // ── Page 3: Inputs table ────────────────────────────────────────
    m_inputsTable->setRowCount(0);
    for (auto& item : inp.inputs) {
        addTableRow(m_inputsTable, "Input");
        int r = m_inputsTable->rowCount() - 1;
        if (auto* w = qobject_cast<QComboBox*>(m_inputsTable->cellWidget(r, 0)))      w->setCurrentText(item.label);
        if (auto* w = qobject_cast<QDoubleSpinBox*>(m_inputsTable->cellWidget(r, 1))) w->setValue(item.qty);
        // Set manual mode so the loaded σ value is preserved exactly
        if (auto* cb = qobject_cast<QComboBox*>(m_inputsTable->cellWidget(r, 2))) {
            int mi = cb->findData("manual"); if (mi>=0) cb->setCurrentIndex(mi); }
        if (auto* w = qobject_cast<QDoubleSpinBox*>(m_inputsTable->cellWidget(r, 3))) {
            w->setReadOnly(false);
            w->setStyleSheet("background:white;border:1px solid #d1d5db;border-radius:4px;");
            w->setValue(item.error); }
    }

    // ── Page 4: Outputs table ───────────────────────────────────────
    m_outputsTable->setRowCount(0);
    for (auto& item : inp.outputs) {
        addTableRow(m_outputsTable, "Output");
        int r = m_outputsTable->rowCount() - 1;
        if (auto* w = qobject_cast<QComboBox*>(m_outputsTable->cellWidget(r, 0)))      w->setCurrentText(item.label);
        if (auto* w = qobject_cast<QDoubleSpinBox*>(m_outputsTable->cellWidget(r, 1))) w->setValue(item.qty);
        if (auto* cb = qobject_cast<QComboBox*>(m_outputsTable->cellWidget(r, 2))) {
            int mi = cb->findData("manual"); if (mi>=0) cb->setCurrentIndex(mi); }
        if (auto* w = qobject_cast<QDoubleSpinBox*>(m_outputsTable->cellWidget(r, 3))) {
            w->setReadOnly(false);
            w->setStyleSheet("background:white;border:1px solid #d1d5db;border-radius:4px;");
            w->setValue(item.error); }
    }

    // ── Page 5: Ending Inventory ────────────────────────────────────
    m_eiLabel->setCurrentText(inp.endingInventory.label);
    m_eiQty->setValue(inp.endingInventory.qty);
    { int idx = m_eiMethod->findData("manual"); if (idx>=0) m_eiMethod->setCurrentIndex(idx); }
    m_eiErr->setValue(inp.endingInventory.error);

    updateEquationDisplays();
    m_pageStack->setCurrentIndex(0);   // start on page 1 so user can review
    updatePageIndicator();
}

// ─────────────────────────────────────────────────────────────────
//  IAEA watermark – 15 % opacity backdrop
// ─────────────────────────────────────────────────────────────────
void DataEntryWizard::paintEvent(QPaintEvent* e)
{
    QDialog::paintEvent(e);
    QPainter p(this);
    IAEAWatermark::paint(p, rect(), 0.15);
}

// ══════════════════════════════════════════════════════════════════════
//  TYPE B UNCERTAINTY HELPERS  (IAEA STR-368 / GUM §4.3)
// ══════════════════════════════════════════════════════════════════════

/**
 * Populate a method QComboBox from the IAEA STR-368 database for the
 * given material type. If preserveKey is supplied and a matching entry
 * is found, that entry is selected; otherwise index 0 is selected.
 */
void DataEntryWizard::populateMethodCombo(QComboBox* combo,
                                          const QString& materialType,
                                          const QString& preserveKey)
{
    combo->blockSignals(true);
    combo->clear();

    const auto methods = MeasurementMethodDB::methodsForMaterial(materialType);
    for (const auto& m : methods) {
        combo->addItem(m.displayName, m.key);
        // Tooltip on item (works on most platforms)
        int idx = combo->count() - 1;
        combo->setItemData(idx, QString("%1\nε_nom = %2% (range %3%–%4%)\n%5\n%6")
            .arg(m.displayName)
            .arg(m.epsilonNominal * 100, 0, 'f', 2)
            .arg(m.epsilonMin     * 100, 0, 'f', 2)
            .arg(m.epsilonMax     * 100, 0, 'f', 2)
            .arg(m.iaeaRef)
            .arg(m.hint),
            Qt::ToolTipRole);
    }

    // Restore selection
    if (!preserveKey.isEmpty()) {
        int found = combo->findData(preserveKey);
        if (found >= 0) combo->setCurrentIndex(found);
    }
    combo->blockSignals(false);
}

/**
 * Read the current method key from methodCombo, look up ε from the
 * database, compute σ = ε × Q, and update errBox and epsilonLbl.
 * If the key is "manual", errBox is left editable and lbl shows "—".
 */
void DataEntryWizard::applyMethodEpsilon(QComboBox*      methodCombo,
                                         QDoubleSpinBox* qtyBox,
                                         QDoubleSpinBox* errBox,
                                         QLabel*         epsilonLbl)
{
    if (!methodCombo || !qtyBox || !errBox || !epsilonLbl) return;

    const QString key = methodCombo->currentData().toString();

    if (key == "manual" || key.isEmpty()) {
        errBox->setReadOnly(false);
        errBox->setStyleSheet(
            "QDoubleSpinBox{background:white;border:1px solid #d1d5db;"
            "border-radius:6px;color:#111827;padding:6px;}");
        epsilonLbl->setText("ε = —   (manual entry — type σ directly)");
        epsilonLbl->setStyleSheet("color:#6b7280;font-size:12px;font-family:'Courier New',monospace;");
        return;
    }

    MeasurementMethod m = MeasurementMethodDB::findByKey(key, m_materialType->currentText());
    const double Q     = qtyBox->value();
    double sigma       = m.epsilonNominal * Q;
    if (sigma < 1e-6) sigma = 1e-6;

    errBox->blockSignals(true);
    errBox->setValue(sigma);
    errBox->blockSignals(false);
    errBox->setReadOnly(true);
    errBox->setStyleSheet(
        "QDoubleSpinBox{background:#eff6ff;border:1.5px solid #93c5fd;"
        "border-radius:6px;color:#1e40af;padding:6px;font-weight:600;}");
    errBox->setToolTip(QString("Auto (STR-368): σ = ε × Q = %1 × %2 kg = %3 kg\n"
                               "ε range: [%4%, %5%]  |  %6\n%7")
        .arg(m.epsilonNominal, 0, 'f', 4)
        .arg(Q, 0, 'f', 3)
        .arg(sigma, 0, 'f', 4)
        .arg(m.epsilonMin * 100, 0, 'f', 2)
        .arg(m.epsilonMax * 100, 0, 'f', 2)
        .arg(m.iaeaRef)
        .arg(m.hint));

    epsilonLbl->setText(QString("ε = %1  (%2%)   σ = ε×Q = %3 kg   range: [%4%, %5%]   %6")
        .arg(m.epsilonNominal, 0, 'f', 4)
        .arg(m.epsilonNominal * 100, 0, 'f', 2)
        .arg(sigma, 0, 'f', 4)
        .arg(m.epsilonMin * 100, 0, 'f', 2)
        .arg(m.epsilonMax * 100, 0, 'f', 2)
        .arg(m.iaeaRef));
    epsilonLbl->setStyleSheet("color:#1e40af;font-size:12px;font-family:'Courier New',monospace;font-weight:600;");
}

/**
 * Refresh all method combos in a table when the global material type
 * changes. Preserves the existing key if still valid; otherwise falls
 * back to index 0 (which is always the recommended primary method).
 */
void DataEntryWizard::refreshTableMethods(QTableWidget* tbl)
{
    const QString mat = m_materialType->currentText();
    for (int r = 0; r < tbl->rowCount(); ++r) {
        auto* combo = qobject_cast<QComboBox*>(tbl->cellWidget(r, 2));
        auto* qty   = qobject_cast<QDoubleSpinBox*>(tbl->cellWidget(r, 1));
        auto* err   = qobject_cast<QDoubleSpinBox*>(tbl->cellWidget(r, 3));
        if (!combo) continue;
        const QString prevKey = combo->currentData().toString();
        populateMethodCombo(combo, mat, prevKey);
        // Re-apply epsilon with new material's ε values
        if (qty && err) {
            QString key = combo->currentData().toString();
            if (key != "manual") {
                MeasurementMethod m = MeasurementMethodDB::findByKey(key, mat);
                double sigma = m.epsilonNominal * qty->value();
                if (sigma < 1e-6) sigma = 1e-6;
                err->blockSignals(true); err->setValue(sigma); err->blockSignals(false);
                err->setReadOnly(true);
                err->setStyleSheet("background:#eff6ff;border:1px solid #93c5fd;"
                                   "border-radius:4px;color:#1e40af;");
            }
        }
    }
}

/**
 * Called when the user changes the Material Type on Page 1.
 * Rebuilds all method combos (BI, table rows, EI) so that only
 * IAEA STR-368 methods applicable to the new material are shown.
 */
void DataEntryWizard::onMaterialTypeChanged()
{
    const QString mat = m_materialType->currentText();

    // BI description combo — refresh to new material's opening inventory items
    if (m_biLabel) {
        const QString prev = m_biLabel->currentText();
        NuclearTransferItemDB::populateInventoryCombo(m_biLabel, mat, true, prev);
    }

    // BI method
    if (m_biMethod) {
        const QString prevKey = m_biMethod->currentData().toString();
        populateMethodCombo(m_biMethod, mat, prevKey);
        applyMethodEpsilon(m_biMethod, m_biQty, m_biErr, m_biEpsLbl);
    }

    // Inputs / Outputs table rows — refresh both methods AND item lists
    if (m_inputsTable) {
        refreshTableMethods(m_inputsTable);
        refreshTableItems(m_inputsTable, true);
    }
    if (m_outputsTable) {
        refreshTableMethods(m_outputsTable);
        refreshTableItems(m_outputsTable, false);
    }

    // EI description combo — refresh to new material's closing inventory items
    if (m_eiLabel) {
        const QString prev = m_eiLabel->currentText();
        NuclearTransferItemDB::populateInventoryCombo(m_eiLabel, mat, false, prev);
    }

    // EI method
    if (m_eiMethod) {
        const QString prevKey = m_eiMethod->currentData().toString();
        populateMethodCombo(m_eiMethod, mat, prevKey);
        applyMethodEpsilon(m_eiMethod, m_eiQty, m_eiErr, m_eiEpsLbl);
    }

    updateEquationDisplays();
}

// ══════════════════════════════════════════════════════════════════════
//  TRANSFER ITEM DESCRIPTION HELPERS  (NuclearTransferItemDB)
// ══════════════════════════════════════════════════════════════════════

/**
 * Refresh the col-0 description combo in every row of tbl when the
 * global material type changes. Preserves any custom text the user
 * already typed; updates predefined options to match the new material.
 *
 * @param isInput  true for the Inputs table, false for Outputs.
 */
void DataEntryWizard::refreshTableItems(QTableWidget* tbl, bool isInput)
{
    const QString mat = m_materialType->currentText();
    for (int r = 0; r < tbl->rowCount(); ++r) {
        auto* combo = qobject_cast<QComboBox*>(tbl->cellWidget(r, 0));
        if (!combo) continue;
        // Preserve whatever the user has in the editable field
        const QString prevText = combo->currentText();
        NuclearTransferItemDB::populateCombo(combo, mat, isInput, prevText);
    }
}

