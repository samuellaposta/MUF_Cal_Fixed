#include "resultspanel.h"
#include "watermarkwidget.h"
#include <QPainter>
#include "reportgenerator.h"
#include "styles.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QProgressBar>
#include <QScrollArea>
#include <QFrame>
#include <QFileDialog>
#include <QMessageBox>
#include <cmath>

// ── helpers ───────────────────────────────────────────────────────────
static QWidget* statCard(const QString& val, const QString& lbl,
                         const QString& col = "#38bdf8") {
    auto* card = new QFrame();
    card->setStyleSheet("background:white;border:1px solid #e5e7eb;border-radius:10px;padding:4px;");
    auto* cl = new QVBoxLayout(card);
    cl->setContentsMargins(14,12,14,12); cl->setSpacing(4);
    auto* vl = new QLabel(val);
    vl->setStyleSheet(QString("color:%1;font-size:13px;font-weight:900;font-family:'Courier New',monospace;").arg(col));
    auto* ll = new QLabel(lbl);
    ll->setStyleSheet("color:#111827;font-size:12.5px;letter-spacing:1px;font-weight:600;");
    cl->addWidget(vl); cl->addWidget(ll);
    return card;
}

ResultsPanel::ResultsPanel(const MBPInput& input,
                            const MUFResult& result,
                            QWidget* parent)
    : QDialog(parent), m_input(input), m_result(result)
{
    setWindowTitle("MUF Results — Explainable Safeguards Output");
    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
    setMinimumSize(820, 660);
    setStyleSheet(Styles::appStyle() + Styles::lineEditStyle());
    buildUI();
}

QString ResultsPanel::ratioColorCSS() const {
    double r = m_result.ratio;
    if (r <= 1) return "#22c55e";
    if (r <= 2) return "#f59e0b";
    if (r <= 3) return "#f97316";
    return "#ef4444";
}

QString ResultsPanel::statusLabel() const {
    double r = m_result.ratio;
    if (r <= 1) return "✔  NORMAL  (≤ 1σ)";
    if (r <= 2) return "⚠  ELEVATED  (1σ – 2σ)";
    if (r <= 3) return "⚠  WARNING  (2σ – 3σ)";
    return "✖  CRITICAL  (> 3σ)";
}

void ResultsPanel::buildUI() {
    auto* root = new QVBoxLayout(this);
    root->setSpacing(0);
    root->setContentsMargins(0,0,0,0);

    // Header
    auto* hdr = new QFrame();
    hdr->setStyleSheet("background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #1a3a5c,stop:1 #2d5680);border-bottom:2px solid #d1d5db;");
    auto* hl = new QHBoxLayout(hdr);
    hl->setContentsMargins(20,14,20,14);
    auto* htitle = new QLabel("⚛  Explainable MUF Results");
    htitle->setStyleSheet("color:#111827;font-size:13.5px;font-weight:900;");
    auto* hsub = new QLabel(m_input.facilityName + "  |  " + m_input.mbpId + "  |  " + m_result.calculatedAt.toString("yyyy-MM-dd HH:mm"));
    hsub->setStyleSheet("color:#111827;font-size:12.5px;");
    hl->addWidget(htitle); hl->addStretch(); hl->addWidget(hsub);
    root->addWidget(hdr);

    // Tabs
    m_tabs = new QTabWidget();
    m_tabs->setStyleSheet(Styles::tabStyle());
    m_tabs->addTab(buildSummaryTab(),     "📊  Summary");
    m_tabs->addTab(buildEquationsTab(),   "🔢  Equations");
    m_tabs->addTab(buildUncertaintyTab(), "📉  Uncertainty");
    m_tabs->addTab(buildDiagnosticsTab(), "🔍  Diagnostics");
    root->addWidget(m_tabs, 1);

    // Footer buttons
    auto* footer = new QFrame();
    footer->setStyleSheet("background:white;border-top:1px solid #e5e7eb;");
    auto* fl = new QHBoxLayout(footer);
    fl->setContentsMargins(16,10,16,10);

    auto* btnHTML = new QPushButton("💾  Export HTML Report");
    btnHTML->setStyleSheet(Styles::secondaryButtonStyle());
    auto* btnTxt = new QPushButton("📄  Export Text Report");
    btnTxt->setStyleSheet(Styles::secondaryButtonStyle() + "QPushButton{color:#166534;border-color:#16a34a;}");
    auto* btnClose = new QPushButton("Close");
    btnClose->setStyleSheet("QPushButton{background:#f4f5f7;color:#374151;border:1px solid #d1d5db;border-radius:7px;padding:8px 18px;}"
                            "QPushButton:hover{background:#e5e7eb;}");

    fl->addWidget(btnHTML); fl->addWidget(btnTxt); fl->addStretch(); fl->addWidget(btnClose);
    root->addWidget(footer);

    connect(btnHTML,  &QPushButton::clicked, this, &ResultsPanel::onExportHTML);
    connect(btnTxt,   &QPushButton::clicked, this, &ResultsPanel::onExportText);
    connect(btnClose, &QPushButton::clicked, this, &QDialog::accept);
}

// ── Tab 0: Summary ────────────────────────────────────────────────────
QWidget* ResultsPanel::buildSummaryTab() {
    auto* scroll = new QScrollArea();
    scroll->setWidgetResizable(true); scroll->setFrameShape(QFrame::NoFrame);
    auto* w = new QWidget();
    auto* layout = new QVBoxLayout(w);
    layout->setContentsMargins(20,20,20,20); layout->setSpacing(16);

    // MUF Banner
    QString col = ratioColorCSS();
    auto* banner = new QFrame();
    banner->setStyleSheet(QString("background:#e5e7eb;border:2px solid %1;border-radius:12px;")
                          .arg(col));
    auto* bl = new QVBoxLayout(banner);
    bl->setContentsMargins(24,22,24,22); bl->setSpacing(6);
    auto* bsub = new QLabel("MATERIAL UNACCOUNTED FOR  ·  MUF = IB − EI");
    bsub->setAlignment(Qt::AlignCenter);
    bsub->setStyleSheet("color:#1a1a2e;font-size:13px;letter-spacing:1px;font-weight:700;");
    auto* bval = new QLabel(QString("%1%2 kg")
                            .arg(m_result.muf>=0?"+":"")
                            .arg(m_result.muf,0,'f',6));
    bval->setAlignment(Qt::AlignCenter);
    bval->setStyleSheet(QString("color:%1;font-size:32px;font-weight:900;font-family:'Courier New',monospace;").arg(col));
    auto* badge = new QLabel(statusLabel());
    badge->setAlignment(Qt::AlignCenter);
    badge->setStyleSheet(QString("background:%1;color:#000;font-weight:900;font-size:13.5px;"
                                 "padding:8px 24px;border-radius:20px;letter-spacing:1px;")
                         .arg(col));
    auto* btime = new QLabel(m_result.calculatedAt.toString("yyyy-MM-dd HH:mm:ss") +
                             "  ·  " + m_input.facilityName + "  ·  " + m_input.mbpId);
    btime->setAlignment(Qt::AlignCenter);
    btime->setStyleSheet("color:#374151;font-size:12.5px;font-weight:600;");
    bl->addWidget(bsub); bl->addWidget(bval); bl->addWidget(badge, 0, Qt::AlignCenter); bl->addWidget(btime);
    layout->addWidget(banner);

    // Stat grid
    auto* grid = new QGridLayout();
    grid->setSpacing(10);
    auto f4 = [](double v){ return QString::number(v,'f',4)+" kg"; };
    auto f6 = [](double v){ return QString::number(v,'f',6); };
    grid->addWidget(statCard("±"+f6(m_result.sigmaMUF)+" kg", "σ(MUF)", "#c084fc"), 0, 0);
    grid->addWidget(statCard(QString::number(m_result.ratio,'f',3)+"σ", "|MUF|/σ RATIO", col),0,1);
    grid->addWidget(statCard(f4(m_result.bookInventory), "BOOK INVENTORY (IB)", "#38bdf8"),0,2);
    grid->addWidget(statCard(f4(m_result.sumInputs),  "ΣInputs",            "#34d399"), 1,0);
    grid->addWidget(statCard(f4(m_result.sumOutputs), "ΣOutputs",           "#f87171"), 1,1);
    grid->addWidget(statCard(f4(m_input.endingInventory.qty), "ENDING INVENTORY (EI)", "#fbbf24"),1,2);
    layout->addLayout(grid);

    // Dominant source
    if (!m_result.contributions.isEmpty()) {
        auto& dom = m_result.contributions[0];
        auto* dc = new QFrame();
        dc->setStyleSheet("background:white;border:1px solid #e5e7eb;border-radius:8px;padding:14px 18px;");
        auto* dl = new QVBoxLayout(dc);
        auto* dt = new QLabel("DOMINANT UNCERTAINTY SOURCE");
        dt->setStyleSheet("color:#111827;font-size:12.5px;font-weight:700;letter-spacing:2px;");
        auto* dn = new QLabel(dom.label);
        dn->setStyleSheet("color:#c084fc;font-weight:700;font-size:13.5px;");
        auto* dp = new QLabel(QString("Contributes  %1%  to total σ²(MUF)")
                              .arg(dom.percentContrib,0,'f',1));
        dp->setStyleSheet("color:#111827;font-size:13.5px;");
        dl->addWidget(dt); dl->addWidget(dn); dl->addWidget(dp);
        layout->addWidget(dc);
    }
    layout->addStretch();
    scroll->setWidget(w);
    return scroll;
}

// ── Tab 1: Equations ─────────────────────────────────────────────────
QWidget* ResultsPanel::buildEquationsTab() {
    auto* scroll = new QScrollArea();
    scroll->setWidgetResizable(true); scroll->setFrameShape(QFrame::NoFrame);
    auto* browser = new QTextBrowser();
    browser->setStyleSheet("background:white;border:none;font-family:'Courier New',monospace;font-size:13px;padding:12px;");
    browser->setOpenExternalLinks(false);

    auto f4 = [this](double v){ return QString::number(v,'f',4); };
    auto f6 = [this](double v){ return QString::number(v,'f',6); };

    auto eq = [](const QString& title, const QString& formula,
                 const QString& sub, const QString& res) {
        return QString("<p style='color:#a78bfa;font-weight:700;font-size:12px;letter-spacing:2px;margin:16px 0 4px'>%1</p>"
                       "<div style='background:#f4f5f7;border-left:3px solid #1a3a5c;padding:12px 16px;"
                       "border-radius:0 6px 6px 0;font-family:Courier New,monospace;color:#93c5fd;line-height:2'>"
                       "%2<br/>%3<br/><span style='color:#4ade80;font-weight:700'>= %4</span></div>")
                   .arg(title,formula,sub,res);
    };

    QString html = "<div style='padding:16px;font-family:Arial,sans-serif;'>"
                   "<h3 style='color:#93c5fd;margin-top:0;font-family:Georgia,serif;'>IAEA Standard Equations — Step-by-Step</h3>";

    html += eq("STEP 1 — BOOK INVENTORY",
               "IB = BI + &Sigma;Inputs &minus; &Sigma;Outputs",
               QString("IB = %1 + %2 &minus; %3").arg(f4(m_input.beginningInventory.qty),f4(m_result.sumInputs),f4(m_result.sumOutputs)),
               f6(m_result.bookInventory) + " kg");

    html += eq("STEP 2 — MATERIAL UNACCOUNTED FOR (MUF)",
               "MUF = IB &minus; EI",
               QString("MUF = %1 &minus; %2").arg(f6(m_result.bookInventory),f6(m_input.endingInventory.qty)),
               f6(m_result.muf) + " kg");

    html += eq("STEP 3 — UNCERTAINTY PROPAGATION",
               "&sigma;(MUF) = &radic;[ &sigma;&sup2;(BI) + &sigma;&sup2;(&Sigma;In) + &sigma;&sup2;(&Sigma;Out) ]",
               QString("&sigma;(MUF) = &radic;[ %1 + %2 + %3 ]")
                   .arg(f6(m_result.sigmaBI*m_result.sigmaBI),
                        f6(m_result.sigmaInputs*m_result.sigmaInputs),
                        f6(m_result.sigmaOutputs*m_result.sigmaOutputs)),
               f6(m_result.sigmaMUF) + " kg");

    html += "<p style='color:#a78bfa;font-weight:700;font-size:12px;letter-spacing:2px;margin:16px 0 4px'>"
            "STEP 4 — EXPLAINABILITY LAYER &nbsp; C&#7522; = [&sigma;&sup2;(i) / &sigma;&sup2;(MUF)] &times; 100%</p>";

    html += "<table style='width:100%;border-collapse:collapse;font-size:13px;font-family:Courier New,monospace;'>"
            "<tr style='border-bottom:1px solid #e5e7eb;'>"
            "<th style='color:#111827;padding:8px 12px;text-align:left;'>Term</th>"
            "<th style='color:#111827;padding:8px 12px;'>σ(i) kg</th>"
            "<th style='color:#111827;padding:8px 12px;'>σ²(i) kg²</th>"
            "<th style='color:#111827;padding:8px 12px;'>C&#7522; %</th></tr>";
    bool alt = false;
    for (auto& c : m_result.contributions) {
        QString col = c.percentContrib>50?"#ef4444":c.percentContrib>25?"#f59e0b":"#4ade80";
        html += QString("<tr style='background:%5;border-bottom:1px solid #f3f4f6;'>"
                        "<td style='color:#111827;padding:7px 12px;'>%1</td>"
                        "<td style='color:#1a3a5c;padding:7px 12px;text-align:right;'>%2</td>"
                        "<td style='color:#1a3a5c;padding:7px 12px;text-align:right;'>%3</td>"
                        "<td style='color:%4;padding:7px 12px;text-align:right;font-weight:700;'>%6%</td></tr>")
                    .arg(c.label)
                    .arg(f6(std::sqrt(c.varianceContrib)))
                    .arg(f6(c.varianceContrib))
                    .arg(col)
                    .arg(alt?"#f9fafb":"#ffffff")
                    .arg(c.percentContrib,0,'f',2);
        alt = !alt;
    }
    html += "</table></div>";
    browser->setHtml(html);
    return browser;
}

// ── Tab 2: Uncertainty ────────────────────────────────────────────────
QWidget* ResultsPanel::buildUncertaintyTab() {
    auto* scroll = new QScrollArea();
    scroll->setWidgetResizable(true); scroll->setFrameShape(QFrame::NoFrame);
    auto* w = new QWidget();
    auto* layout = new QVBoxLayout(w);
    layout->setContentsMargins(20,20,20,20); layout->setSpacing(6);

    auto* hdr = new QLabel("Uncertainty Contribution Analysis");
    hdr->setStyleSheet("color:#93c5fd;font-size:13.5px;font-weight:900;margin-bottom:4px;");
    layout->addWidget(hdr);

    auto* sumInfo = new QLabel(
        QString("σ²(MUF) = <b style='color:#c084fc'>%1 kg²</b>"
                "  &nbsp;|&nbsp;  σ(MUF) = <b style='color:#c084fc'>±%2 kg</b>"
                "  &nbsp;|&nbsp;  Formula: C&#7522; = [σ²(i) / σ²(MUF)] × 100%")
            .arg(m_result.varianceMUF,0,'f',6)
            .arg(m_result.sigmaMUF,0,'f',6));
    sumInfo->setTextFormat(Qt::RichText);
    sumInfo->setStyleSheet("color:#111827;font-size:13.5px;background:#f9fafb;border:1px solid #e5e7eb;border-radius:7px;padding:10px 14px;margin-bottom:16px;");
    sumInfo->setWordWrap(true);
    layout->addWidget(sumInfo);

    for (auto& c : m_result.contributions) {
        QString col = c.percentContrib>50?"#ef4444":c.percentContrib>25?"#f59e0b":"#3b82f6";

        auto* row = new QWidget();
        auto* rl = new QVBoxLayout(row);
        rl->setContentsMargins(0,0,0,12); rl->setSpacing(5);

        // Label + pct
        auto* topRow = new QHBoxLayout();
        auto* nameLbl = new QLabel(c.label);
        nameLbl->setStyleSheet("color:#374151;font-size:13.5px;font-weight:600;");
        auto* pctLbl = new QLabel(QString("%1%").arg(c.percentContrib,0,'f',2));
        pctLbl->setStyleSheet(QString("color:%1;font-weight:900;font-family:'Courier New',monospace;font-size:13.5px;").arg(col));
        topRow->addWidget(nameLbl); topRow->addStretch(); topRow->addWidget(pctLbl);
        rl->addLayout(topRow);

        // Progress bar
        auto* bar = new QProgressBar();
        bar->setRange(0, 1000);
        bar->setValue(static_cast<int>(std::min(100.0, c.percentContrib) * 10));
        bar->setTextVisible(false);
        bar->setFixedHeight(12);
        bar->setStyleSheet(QString(
            "QProgressBar{background:#e5e7eb;border-radius:6px;border:none;}"
            "QProgressBar::chunk{background:%1;border-radius:6px;}").arg(col));
        rl->addWidget(bar);

        auto* details = new QLabel(QString("σ²(i) = %1 kg²   |   σ(i) = %2 kg")
                                   .arg(c.varianceContrib,0,'f',6)
                                   .arg(std::sqrt(c.varianceContrib),0,'f',6));
        details->setStyleSheet("color:#374151;font-size:12.5px;font-family:'Courier New',monospace;");
        rl->addWidget(details);
        layout->addWidget(row);
    }
    layout->addStretch();
    scroll->setWidget(w);
    return scroll;
}

// ── Tab 3: Diagnostics ────────────────────────────────────────────────
QWidget* ResultsPanel::buildDiagnosticsTab() {
    auto* scroll = new QScrollArea();
    scroll->setWidgetResizable(true); scroll->setFrameShape(QFrame::NoFrame);
    auto* w = new QWidget();
    auto* layout = new QVBoxLayout(w);
    layout->setContentsMargins(20,20,20,20); layout->setSpacing(10);

    auto* hdr = new QLabel(QString("Safeguards Diagnostic Results — %1 flag(s) generated")
                           .arg(m_result.flags.size()));
    hdr->setStyleSheet("color:#93c5fd;font-size:13.5px;font-weight:900;margin-bottom:8px;");
    layout->addWidget(hdr);

    for (auto& f : m_result.flags) {
        QString bg, border, textColor;
        switch(f.severity){
        case Severity::OK:       bg="#f0fdf4"; border="#16a34a"; textColor="#15803d"; break;
        case Severity::INFO:     bg="#eff6ff"; border="#2563eb"; textColor="#1e40af"; break;
        case Severity::WARNING:  bg="#fffbeb"; border="#d97706"; textColor="#b45309"; break;
        case Severity::ALERT:    bg="#fff7ed"; border="#ea580c"; textColor="#c2410c"; break;
        case Severity::CRITICAL: bg="#fef2f2"; border="#dc2626"; textColor="#b91c1c"; break;
        }
        auto* card = new QFrame();
        card->setStyleSheet(QString("background:%1;border:2px solid %2;border-left:4px solid %2;border-radius:0 8px 8px 0;").arg(bg,border));
        auto* cl = new QVBoxLayout(card);
        cl->setContentsMargins(16,12,16,12); cl->setSpacing(5);

        // Badge + code
        auto* topRow = new QHBoxLayout();
        auto* badge = new QLabel(
            f.severity==Severity::OK?"OK":f.severity==Severity::INFO?"INFO":
            f.severity==Severity::WARNING?"WARNING":f.severity==Severity::ALERT?"ALERT":"CRITICAL");
        badge->setStyleSheet(QString("background:%1;color:#000;font-size:12.5px;font-weight:900;padding:3px 9px;border-radius:4px;letter-spacing:1px;").arg(border));
        auto* codeLbl = new QLabel(f.code);
        codeLbl->setStyleSheet(QString("color:%1;font-weight:700;font-size:13.5px;font-family:'Courier New',monospace;").arg(textColor));
        topRow->addWidget(badge); topRow->addWidget(codeLbl); topRow->addStretch();
        cl->addLayout(topRow);

        auto* msgLbl = new QLabel(f.message);
        msgLbl->setStyleSheet(QString("color:#1a1a2e;font-size:13.5px;font-weight:600;"));
        msgLbl->setWordWrap(true);
        cl->addWidget(msgLbl);

        auto* actLbl = new QLabel("▶  " + f.action);
        actLbl->setStyleSheet("color:#374151;font-size:12.5px;font-style:italic;");
        actLbl->setWordWrap(true);
        cl->addWidget(actLbl);

        layout->addWidget(card);
    }
    layout->addStretch();
    scroll->setWidget(w);
    return scroll;
}

void ResultsPanel::onExportHTML() {
    QString path = QFileDialog::getSaveFileName(this, "Save HTML Report",
        "MUF_Report_" + m_input.mbpId + ".html", "HTML Files (*.html)");
    if (path.isEmpty()) return;
    ReportGenerator rg;
    if (rg.saveHTML(path, m_input, m_result))
        QMessageBox::information(this, "Saved", "HTML report saved to:\n" + path);
    else
        QMessageBox::warning(this, "Error", "Could not save report to:\n" + path);
}

void ResultsPanel::onExportText() {
    QString path = QFileDialog::getSaveFileName(this, "Save Text Report",
        "MUF_Report_" + m_input.mbpId + ".txt", "Text Files (*.txt)");
    if (path.isEmpty()) return;
    ReportGenerator rg;
    if (rg.savePlainText(path, m_input, m_result))
        QMessageBox::information(this, "Saved", "Text report saved to:\n" + path);
    else
        QMessageBox::warning(this, "Error", "Could not save report.");
}

// ─────────────────────────────────────────────────────────────────
//  IAEA watermark – 15 % opacity backdrop
// ─────────────────────────────────────────────────────────────────
void ResultsPanel::paintEvent(QPaintEvent* e)
{
    QDialog::paintEvent(e);
    QPainter p(this);
    IAEAWatermark::paint(p, rect(), 0.15);
}

