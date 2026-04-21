#include "aboutdialog.h"
#include "watermarkwidget.h"
#include <QPainter>
#include "styles.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>

AboutDialog::AboutDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("About MUF Calculator");
    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
    setFixedSize(520, 480);
    setStyleSheet(Styles::appStyle() + Styles::lineEditStyle());

    auto* root = new QVBoxLayout(this);
    root->setSpacing(0); root->setContentsMargins(0,0,0,0);

    // Header
    auto* hdr = new QFrame();
    hdr->setStyleSheet("background:qlineargradient(x1:0,y1:0,x2:1,y2:1,"
                       "stop:0 #1a3a5c,stop:1 #2d5680);border-bottom:2px solid #d1d5db;");
    auto* hl = new QVBoxLayout(hdr);
    hl->setContentsMargins(24,24,24,24); hl->setSpacing(6);
    auto* atom = new QLabel("⚛");
    atom->setAlignment(Qt::AlignCenter);
    atom->setStyleSheet("font-size:60px;");
    auto* name = new QLabel("MUF Calculator");
    name->setAlignment(Qt::AlignCenter);
    name->setStyleSheet("color:#111827;font-size:26px;font-weight:900;");
    auto* ver = new QLabel("Version 1.0  —  IAEA Safeguards Compliant");
    ver->setAlignment(Qt::AlignCenter);
    ver->setStyleSheet("color:#111827;font-size:19px;font-family:'Courier New',monospace;");
    hl->addWidget(atom); hl->addWidget(name); hl->addWidget(ver);
    root->addWidget(hdr);

    // Body
    auto* body = new QWidget();
    body->setStyleSheet("background:white;");
    auto* bl = new QVBoxLayout(body);
    bl->setContentsMargins(28,24,28,24); bl->setSpacing(12);

    auto* desc = new QLabel(R"(
<p><b style='color:#93c5fd'>Nuclear Material Accounting System</b></p>
<p style='color:#111827'>Implements IAEA standard equations for:</p>
<ul style='color:#111827;line-height:2'>
  <li>Book Inventory: IB = BI + &Sigma;Inputs &minus; &Sigma;Outputs</li>
  <li>Material Unaccounted For: MUF = IB &minus; EI</li>
  <li>Uncertainty Propagation: &sigma;(MUF) = &radic;[&sigma;&sup2;(BI) + &sigma;&sup2;(&Sigma;In) + &sigma;&sup2;(&Sigma;Out)]</li>
  <li>Explainability Layer: C&#7522; = [&sigma;&sup2;(i) / &sigma;&sup2;(MUF)] &times; 100%</li>
</ul>
<table style='width:100%;margin-top:8px;'>
  <tr><td style='color:#111827;'>Developer:</td><td style='color:#111827;font-weight:600;'>Samuel Nartey Teye</td></tr>
  <tr><td style='color:#111827;'>Student Group:</td><td style='color:#111827;'>0AM4&Phi;  &mdash;  Tomsk, 2025</td></tr>
  <tr><td style='color:#111827;'>Supervisor:</td><td style='color:#111827;'>Roman Pushkarskiy, Senior Lecturer</td></tr>
  <tr><td style='color:#111827;'>Framework:</td><td style='color:#111827;'>Qt Creator / C++17</td></tr>
  <tr><td style='color:#111827;'>Reference:</td><td style='color:#111827;'>IAEA Safeguards Glossary, 2001</td></tr>
  <tr><td style='color:#111827;'>License:</td><td style='color:#111827;'>MIT &mdash; Open Source</td></tr>
</table>
<p style='color:#374151;font-size:18px;margin-top:12px;'>
For educational and training use only. Not for operational safeguards reporting.
</p>
)");
    desc->setTextFormat(Qt::RichText);
    desc->setWordWrap(true);
    desc->setStyleSheet("font-size:20px;background:transparent;");
    bl->addWidget(desc);

    auto* closeBtn = new QPushButton("Close");
    closeBtn->setStyleSheet(Styles::primaryButtonStyle() + "QPushButton{padding:9px 28px;}");
    bl->addStretch();
    bl->addWidget(closeBtn, 0, Qt::AlignCenter);
    root->addWidget(body);

    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

// ─────────────────────────────────────────────────────────────────
//  IAEA watermark – 15 % opacity backdrop
// ─────────────────────────────────────────────────────────────────
void AboutDialog::paintEvent(QPaintEvent* e)
{
    QDialog::paintEvent(e);
    QPainter p(this);
    IAEAWatermark::paint(p, rect(), 0.15);
}

