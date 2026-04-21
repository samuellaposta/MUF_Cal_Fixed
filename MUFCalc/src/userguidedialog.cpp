#include "userguidedialog.h"
#include "watermarkwidget.h"
#include <QPainter>
#include "styles.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QSplitter>
#include <QFrame>
#include <QPixmap>
#include <QProgressBar>

// ── Shared HTML CSS ───────────────────────────────────────────────────
static const QString CSS = R"(
<style>
  body  { font-family:'Segoe UI',Ubuntu,Arial,sans-serif;
          font-size:13.5px; color:#111827; background:white;
          margin:0; padding:0; line-height:1.75; }
  h1    { color:#1a3a5c; font-size:12.5px; font-weight:900;
          border-bottom:3px solid #1a3a5c; padding-bottom:12px; margin-bottom:20px; }
  h2    { color:#1a3a5c; font-size:13.5px; font-weight:800;
          margin-top:30px; margin-bottom:12px; }
  h3    { color:#2d5680; font-size:13.5px; font-weight:700;
          margin-top:22px; margin-bottom:10px; }
  p     { margin:12px 0; }
  code  { background:#f0f2f5; border:1px solid #e5e7eb; border-radius:4px;
          padding:3px 8px; font-family:'Courier New',monospace; font-size:12.5px;
          color:#1a3a5c; }
  pre   { background:#f0f2f5; border:1px solid #e5e7eb; border-radius:8px;
          padding:16px 20px; font-family:'Courier New',monospace; font-size:12.5px;
          color:#1a3a5c; margin:14px 0; white-space:pre-wrap; }
  .step { background:white; border:1px solid #e5e7eb;
          border-left:5px solid #1a3a5c; border-radius:8px;
          padding:18px 22px; margin:16px 0; }
  .step-num { color:#1a3a5c; font-size:12.5px; font-weight:900;
              float:left; margin-right:16px; line-height:1; }
  .tip  { background:#eff6ff; border:1px solid #bfdbfe;
          border-radius:8px; padding:16px 20px; margin:16px 0; }
  .warn { background:#fffbeb; border:1px solid #fde68a;
          border-radius:8px; padding:16px 20px; margin:16px 0; }
  .crit { background:#fef2f2; border:1px solid #fca5a5;
          border-radius:8px; padding:16px 20px; margin:16px 0; }
  .ok   { background:#f0fdf4; border:1px solid #bbf7d0;
          border-radius:8px; padding:16px 20px; margin:16px 0; }
  .quiz { background:#f5f3ff; border:1px solid #c4b5fd;
          border-radius:8px; padding:16px 20px; margin:16px 0; }
  .obj  { background:#fff7ed; border:1px solid #fed7aa;
          border-radius:8px; padding:14px 20px; margin:14px 0;
          font-size:12.5px; }
  table { width:100%; border-collapse:collapse; margin:16px 0; font-size:12.5px; }
  th    { background:#1a3a5c; color:white; padding:11px 15px;
          text-align:left; font-weight:700; }
  td    { padding:10px 15px; border-bottom:1px solid #f3f4f6; }
  tr:nth-child(even) td { background:#f9fafb; }
  .badge { display:inline-block; padding:3px 11px; border-radius:12px;
           font-size:12.5px; font-weight:700; margin-right:6px; }
  .b-ok   { background:#f0fdf4; color:#15803d; border:1px solid #16a34a; }
  .b-info { background:#eff6ff; color:#1d4ed8; border:1px solid #1d4ed8; }
  .b-warn { background:#fffbeb; color:#b45309; border:1px solid #d97706; }
  .b-alert{ background:#fff7ed; color:#c2410c; border:1px solid #ea580c; }
  .b-crit { background:#fef2f2; color:#b91c1c; border:1px solid #dc2626; }
  dt { font-weight:700; color:#1a3a5c; margin-top:16px; font-size:13.5px; }
  dd { margin-left:22px; color:#374151; margin-bottom:10px; }
  ul,ol { padding-left:22px; }
  li { margin:6px 0; }
</style>
)";

QTextBrowser* UserGuideDialog::makeBrowser(const QString& html) {
    auto* b = new QTextBrowser();
    b->setHtml(CSS + html);
    b->setOpenExternalLinks(true);
    b->setStyleSheet("QTextBrowser{background:white;border:none;padding:22px;font-size:13.5px;}");
    return b;
}

// ══════════════════════════════════════════════════════════════════════
//  CONSTRUCTOR
// ══════════════════════════════════════════════════════════════════════
UserGuideDialog::UserGuideDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("MUF Calculator — Student User Guide");
    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
    setMinimumSize(1050, 740);
    setStyleSheet(Styles::appStyle() + Styles::lineEditStyle());
    buildUI();
}

void UserGuideDialog::buildUI() {
    auto* root = new QVBoxLayout(this);
    root->setSpacing(0); root->setContentsMargins(0,0,0,0);

    // Header
    auto* hdr = new QFrame();
    hdr->setStyleSheet("background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
                       "stop:0 #1a3a5c,stop:1 #2d5680);"
                       "border-bottom:2px solid #d1d5db;");
    auto* hl = new QHBoxLayout(hdr); hl->setContentsMargins(24,16,24,16);

    QPixmap logo(":/tpu_logo.png");
    if (!logo.isNull()) {
        auto* lg = new QLabel(); lg->setPixmap(logo.scaledToHeight(42, Qt::SmoothTransformation));
        lg->setStyleSheet("background:transparent;border:none;");
        hl->addWidget(lg); hl->addSpacing(16);
    }
    auto* tc = new QVBoxLayout();
    auto* t1 = new QLabel("Student User Guide");
    t1->setStyleSheet("color:white;font-size:14px;font-weight:900;");
    auto* t2 = new QLabel("MUF Calculator — IAEA Nuclear Material Accounting System");
    t2->setStyleSheet("color:#a5c8e8;font-size:12px;");
    tc->addWidget(t1); tc->addWidget(t2);
    hl->addLayout(tc); hl->addStretch();
    auto* badge = new QLabel("Tomsk Polytechnic University  |  IAEA Safeguards");
    badge->setStyleSheet("color:#a5c8e8;font-size:12px;");
    hl->addWidget(badge);
    root->addWidget(hdr);

    // Tabs
    auto* tabs = new QTabWidget();
    tabs->setStyleSheet(Styles::appStyle());
    tabs->addTab(buildHowToUse(),            "  How to Use  ");
    tabs->addTab(buildInteractiveTutorial(), "  Lessons  ");
    tabs->addTab(buildQuickStart(),          "  Quick Start  ");
    tabs->addTab(buildTheory(),              "  IAEA Theory  ");
    tabs->addTab(buildDiagnosticsGuide(),    "  Diagnostics Guide  ");
    tabs->addTab(buildGlossary(),            "  Glossary  ");
    tabs->addTab(buildFAQ(),                 "  FAQ  ");
    root->addWidget(tabs, 1);

    // Footer
    auto* footer = new QFrame();
    footer->setStyleSheet("background:white;border-top:1px solid #e5e7eb;");
    auto* fl = new QHBoxLayout(footer); fl->setContentsMargins(20,10,20,10);
    auto* note = new QLabel("Reference: IAEA INFCIRC/153  |  IAEA Safeguards Glossary 2022  |  Burr et al. (2013) doi:10.1155/2013/961360");
    note->setStyleSheet("color:#111827;font-size:12px;");
    auto* closeBtn = new QPushButton("Close Guide");
    closeBtn->setStyleSheet(Styles::primaryButtonStyle());
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    fl->addWidget(note); fl->addStretch(); fl->addWidget(closeBtn);
    root->addWidget(footer);
}

// ══════════════════════════════════════════════════════════════════════
//  INTERACTIVE TUTORIAL TAB
// ══════════════════════════════════════════════════════════════════════
void UserGuideDialog::buildLessons() {
    m_lessons.clear();

    // ── LESSON 1 ──────────────────────────────────────────────────────
    m_lessons.append({"Introduction to NMA", "[1]", R"(
<h1>Lesson 1: What is Nuclear Material Accounting?</h1>
<div class="obj">
<strong>Learning Objectives:</strong><br>
After this lesson you will be able to: (1) define MUF and NMA, (2) explain why nuclear material accounting matters for non-proliferation, (3) identify the four main types of quantities tracked in an NMA system.
</div>

<h2>Why Does Nuclear Material Accounting Exist?</h2>
<p>The International Atomic Energy Agency (IAEA) was established in 1957 with a mandate to ensure that nuclear material is not diverted from peaceful uses to nuclear weapons. The primary tool for achieving this is <strong>Nuclear Material Accounting (NMA)</strong>.</p>
<p>The underlying principle is simple: <em>matter is conserved</em>. If a nuclear facility starts with 1,000 kg of uranium, receives 500 kg, ships out 400 kg, and ends with 1,100 kg — all 1,000 kg are accounted for. If only 1,050 kg are found at the end, 50 kg are <em>material unaccounted for</em>.</p>

<h2>The Four Key Quantities</h2>
<table>
<tr><th>Symbol</th><th>Full Name</th><th>Definition</th></tr>
<tr><td><code>BI</code></td><td>Beginning Inventory</td><td>Physically measured nuclear material at the start of the accounting period</td></tr>
<tr><td><code>IB</code></td><td>Book Inventory</td><td>Calculated: BI + all inputs − all outputs (what the records say should be there)</td></tr>
<tr><td><code>EI</code></td><td>Ending Inventory</td><td>Physically measured at end of period (what is actually there)</td></tr>
<tr><td><code>MUF</code></td><td>Material Unaccounted For</td><td>IB − EI: the gap between what books say and what is found</td></tr>
</table>

<h2>The Basic MUF Formula</h2>
<pre>MUF = IB − EI  =  (BI + ΣInputs − ΣOutputs) − EI</pre>
<div class="ok">
<strong>Key insight:</strong> MUF = 0 in a perfect world. In reality, every measurement has error, so MUF is always slightly non-zero. The question is: <em>is MUF statistically significant, or just measurement noise?</em>
</div>

<h2>Real-World Significance</h2>
<p>The IAEA's safeguards system has been applied at over 1,300 facilities in 180+ countries. The system detected the Iraqi nuclear weapons programme in 1991 and the North Korean programme in 1992. At the heart of every detection was a discrepancy in the material balance.</p>

<div class="quiz">
<strong>Self-Check Questions:</strong>
<ol>
<li>A facility has BI = 500 kg, adds 300 kg inputs, removes 200 kg outputs, and measures EI = 612 kg. What is IB? What is MUF?</li>
<li>If MUF = +50 kg, does this mean material was stolen? Explain your reasoning.</li>
<li>What is the difference between IB and EI? Why are both needed?</li>
</ol>
<details><summary style="color:#1a3a5c;font-weight:700;cursor:pointer;">Show Answers</summary>
<p><strong>1.</strong> IB = 500 + 300 − 200 = 600 kg; MUF = 600 − 612 = −12 kg</p>
<p><strong>2.</strong> Not necessarily. A positive MUF of any size could be due to measurement error. Only if MUF is statistically large (many sigmas above zero) should you be concerned about diversion.</p>
<p><strong>3.</strong> IB is a paper calculation; EI is a physical measurement. Both are needed to find the discrepancy MUF = IB − EI.</p>
</details>
</div>
)"});

    // ── LESSON 2 ──────────────────────────────────────────────────────
    m_lessons.append({"Measurement Uncertainty", "[2]", R"(
<h1>Lesson 2: Measurement Uncertainty and σ(MUF)</h1>
<div class="obj">
<strong>Learning Objectives:</strong><br>
Understand why MUF is never exactly zero, calculate σ(MUF) from individual uncertainties, interpret the significance ratio |MUF|/σ(MUF).
</div>

<h2>Why Is MUF Never Zero?</h2>
<p>Every measurement instrument has limited precision. A scale that weighs a uranium cylinder shows a slightly different reading each time. A density meter has calibration drift. A sample analysed by mass spectrometry has counting statistics. These errors <em>propagate</em> into MUF through the basic equation:</p>
<pre>MUF = BI + ΣInputs − ΣOutputs − EI</pre>
<p>Because each term has error, so does MUF. The key formula for <strong>uncertainty propagation</strong> (assuming independent random errors) is:</p>

<h2>The σ(MUF) Formula</h2>
<pre>σ²(MUF) = σ²(BI) + σ²(ΣInputs) + σ²(ΣOutputs) + σ²(EI)

σ(MUF)  = √[ σ²(BI) + σ²(ΣIn) + σ²(ΣOut) + σ²(EI) ]</pre>
<div class="tip">
<strong>Note:</strong> The uncertainties add in quadrature (squares), not linearly. This means one large uncertain term dominates — fixing the biggest uncertainty source gives you the most improvement.
</div>

<h2>The Significance Ratio</h2>
<p>Once you have MUF and σ(MUF), the key test statistic is:</p>
<pre>Significance Ratio = |MUF| / σ(MUF)</pre>
<table>
<tr><th>Ratio Value</th><th>Statistical Meaning</th><th>System Flag</th></tr>
<tr><td>≤ 1σ</td><td>68% likely to occur by chance alone — normal</td><td><span class="badge b-ok">OK</span></td></tr>
<tr><td>1σ – 2σ</td><td>Expected ~27% of the time — slightly elevated</td><td><span class="badge b-info">INFO</span></td></tr>
<tr><td>2σ – 3σ</td><td>Only ~4.5% likely by chance — statistically significant</td><td><span class="badge b-warn">WARNING</span></td></tr>
<tr><td>&gt; 3σ</td><td>Only ~0.3% likely by chance — highly alarming</td><td><span class="badge b-crit">CRITICAL</span></td></tr>
</table>

<h2>Worked Example</h2>
<pre>BI = 5,000 kg,  σ(BI) = 12.5 kg
ΣIn = 6,500 kg,  σ(ΣIn) = √(6.25² + 5.5² + 4.5²) = 9.17 kg
ΣOut = 6,821.5 kg, σ(ΣOut) = √(7² + 4.75² + 3² + 0.05²) = 8.93 kg
EI = 4,488.25 kg,  σ(EI) = 11.22 kg

IB  = 5000 + 6500 − 6821.5 = 4,678.5 kg
MUF = 4678.5 − 4488.25 = +190.25 kg

σ²(MUF) = 12.5² + 9.17² + 8.93² + 11.22²
         = 156.25 + 84.09 + 79.74 + 125.89 = 445.97 kg²
σ(MUF)  = √445.97 ≈ 21.1 kg

Ratio = 190.25 / 21.1 ≈ 9.0σ  → CRITICAL</pre>
<div class="crit"><strong>Interpretation:</strong> A ratio of 9σ would be extremely alarming in practice. This example is exaggerated for clarity — click "Load Example" in the calculator to see a more typical scenario.</div>

<div class="quiz">
<strong>Self-Check:</strong><br>
A facility reports: MUF = 25 kg, σ(BI) = 8 kg, σ(ΣIn) = 6 kg, σ(ΣOut) = 5 kg, σ(EI) = 9 kg.
<ol>
<li>Calculate σ(MUF).</li>
<li>Calculate the significance ratio.</li>
<li>What flag would the system show?</li>
</ol>
<details><summary style="color:#1a3a5c;font-weight:700;cursor:pointer;">Show Answers</summary>
<p><strong>1.</strong> σ²(MUF) = 64 + 36 + 25 + 81 = 206; σ(MUF) = √206 ≈ 14.35 kg</p>
<p><strong>2.</strong> Ratio = 25 / 14.35 ≈ 1.74σ</p>
<p><strong>3.</strong> 1σ–2σ → <span class="badge b-info">INFO</span></p>
</details>
</div>
)"});

    // ── LESSON 3 ──────────────────────────────────────────────────────
    m_lessons.append({"Using the Data Entry Wizard", "[3]", R"(
<h1>Lesson 3: Using the Data Entry Wizard</h1>
<div class="obj">
<strong>Learning Objectives:</strong><br>
Navigate all 5 pages of the wizard, enter valid data for each quantity, understand what each field means and what validation rules apply.
</div>

<h2>Opening the Wizard</h2>
<p>Click <code>▶ New Calculation</code> in the sidebar or from the Calculate menu. The wizard opens with 5 pages, indicated by numbered dots at the top.</p>
<div class="tip">
<strong>First time?</strong> Click <code>⚡ Load Example</code> on any page to populate a complete realistic UF6 enrichment plant scenario so you can see what properly filled data looks like.
</div>

<h2>Page 1: Facility Information</h2>
<div class="step">
<span class="step-num">1</span>
<strong>Facility Name</strong> — Full official name (e.g., "Siberian Chemical Combine"). Max 200 characters.<br>
<strong>Facility ID</strong> — IAEA-assigned code: letters, numbers, hyphens, dots only (e.g., "RUS-FCF-001").<br>
<strong>Material Type</strong> — Choose from dropdown (UF6, HEU, Pu, LEU, etc.) — affects which Significant Quantity threshold applies.<br>
<strong>Operator Name</strong> — The responsible nuclear material accountancy officer.<br>
<strong>MBP ID</strong> — A unique identifier for this balance period (e.g., "MBP-2025-Q1-001").<br>
<strong>Period Start / End</strong> — Calendar dates. End must be after start, not in the future, within 5 years of start.
</div>

<h2>Pages 2 &amp; 5: Inventory Measurements</h2>
<div class="step">
<span class="step-num">2/5</span>
<strong>Description</strong> — Identify what material and where (e.g., "UF6 feed cylinders at KMP-A").<br>
<strong>Quantity (kg)</strong> — The measured mass. Must be between 0 and 100,000,000 kg. Cannot be negative for physical inventory.<br>
<strong>1σ Uncertainty (kg)</strong> — Must be greater than 0 (zero is physically impossible). Typical values: 0.05–3% of the quantity. Cannot exceed 50% of quantity.
</div>
<div class="warn">
<strong>Common Mistake:</strong> Leaving uncertainty at 0 will trigger a CRITICAL ZERO-SIGMA flag. Always enter a realistic uncertainty — even if you don't know the exact value, 0.5% of the quantity is a reasonable starting estimate.
</div>

<h2>Pages 3 &amp; 4: Transfers (Inputs and Outputs)</h2>
<div class="step">
<span class="step-num">3/4</span>
Click <code>+ Add Row</code> to add each transfer batch. Include <em>every</em> shipment, receipt, process sample, and waste stream.<br><br>
<strong>Common outputs to never forget:</strong>
<ul>
<li>Process waste and effluent</li>
<li>Samples shipped to IAEA or other labs</li>
<li>Process hold-up (material stuck in pipes, vessels)</li>
<li>Tails at enrichment plants</li>
</ul>
</div>
<div class="crit">
<strong>Critical:</strong> A missing output causes a large NEGATIVE MUF — the facility appears to have <em>gained</em> material. This triggers the NEGATIVE-MUF ALERT flag. Always account for ALL material streams.
</div>

<h2>Page 5: Live Equation Preview</h2>
<p>As you enter the Ending Inventory, the page automatically calculates and shows IB, MUF, and σ(MUF) in real time. This lets you check your numbers before running the full analysis. When ready, click <code>▶ Calculate MUF</code>.</p>

<h2>Validation: Green and Red Fields</h2>
<p>Fields turn <strong style="color:#16a34a">green</strong> when they contain valid data and <strong style="color:#dc2626">red</strong> when invalid. A validation summary bar appears at the bottom of each page listing all errors. You cannot proceed to the next page until all errors are fixed.</p>
)"});

    // ── LESSON 4 ──────────────────────────────────────────────────────
    m_lessons.append({"Reading the Results Panel", "[4]", R"(
<h1>Lesson 4: Interpreting the Results Panel</h1>
<div class="obj">
<strong>Learning Objectives:</strong><br>
Read and interpret all four tabs of the results panel, understand what each diagnostic flag means, know what action each severity level requires.
</div>

<h2>The Four Results Tabs</h2>
<table>
<tr><th>Tab</th><th>Contains</th><th>Most Important For</th></tr>
<tr><td><strong>Summary</strong></td><td>MUF value, σ(MUF), ratio, SQ fraction, status badge</td><td>Quick status check — is there a problem?</td></tr>
<tr><td><strong>Equations</strong></td><td>Step-by-step calculation with your actual numbers substituted</td><td>Verifying the arithmetic is correct</td></tr>
<tr><td><strong>Uncertainty</strong></td><td>Contribution (Cᵢ%) of each measurement to σ²(MUF)</td><td>Finding which instrument to improve first</td></tr>
<tr><td><strong>Diagnostics</strong></td><td>All 12 safeguards checks with colour-coded severity</td><td>Deciding what regulatory action is required</td></tr>
</table>

<h2>Understanding the Dashboard</h2>
<p>The main window dashboard shows three key numbers at all times:</p>
<ul>
<li><strong>MUF value</strong> — large coloured number: green = normal, yellow = elevated, orange = warning, red = critical</li>
<li><strong>σ(MUF)</strong> — the measurement uncertainty of MUF</li>
<li><strong>SQ Fraction</strong> — |MUF| as percentage of one IAEA Significant Quantity</li>
</ul>

<h2>Severity Levels — What To Do</h2>
<table>
<tr><th>Badge</th><th>Sigma Range</th><th>Required Action</th></tr>
<tr><td><span class="badge b-ok">OK</span></td><td>≤ 1σ</td><td>File routine report. No further action needed.</td></tr>
<tr><td><span class="badge b-info">INFO</span></td><td>1σ–2σ</td><td>Note in records. Monitor next MBP for trend.</td></tr>
<tr><td><span class="badge b-warn">WARNING</span></td><td>2σ–3σ</td><td>Review all records. Schedule verification. Inform supervisor.</td></tr>
<tr><td><span class="badge b-alert">ALERT</span></td><td>Various</td><td>Notify State authority. Perform early physical inventory.</td></tr>
<tr><td><span class="badge b-crit">CRITICAL</span></td><td>&gt; 3σ</td><td>Report immediately to IAEA (INFCIRC/153 Art.25). Preserve all records.</td></tr>
</table>

<h2>The Uncertainty Contribution Analysis</h2>
<p>The formula for each term's contribution is:</p>
<pre>Cᵢ = σ²(i) / σ²(MUF) × 100%</pre>
<p>For example, if σ²(BI) = 156 and σ²(MUF) = 446, then C(BI) = 156/446 × 100% = 35%. The system flags any single contributor over 50% as a DOMINANT-UNCERTAINTY-SOURCE.</p>
<div class="ok">
<strong>Why this matters:</strong> If your EI measurement contributes 70% to σ²(MUF), investing in a better EI weighing system reduces your MDF (Minimum Detectable False Alarm) by far more than improving any other instrument.
</div>

<h2>Exporting Reports</h2>
<p>After every calculation, go to <code>File → Export Report</code> to save a professional HTML report. This includes all equations with actual values substituted, all uncertainty contributions, and all diagnostic flags with IAEA article references. The report is suitable for submission to state authorities or academic review.</p>

<div class="quiz">
<strong>Scenario Exercise:</strong><br>
You calculate: MUF = +85 kg, σ(MUF) = 18 kg. The facility handles LEU (SQ = 75 kg U-235).<br>
<ol>
<li>What is the significance ratio? What flag fires?</li>
<li>MUF is 113% of 1 SQ — what additional flag fires?</li>
<li>What regulatory actions are required?</li>
</ol>
<details><summary style="color:#1a3a5c;font-weight:700;cursor:pointer;">Show Answers</summary>
<p><strong>1.</strong> Ratio = 85/18 = 4.7σ → CRITICAL MUF-EXCEED-3σ</p>
<p><strong>2.</strong> SQ-EXCEEDED (CRITICAL) — more than 1 full SQ is unaccounted for</p>
<p><strong>3.</strong> Report immediately to State authority and IAEA under INFCIRC/153 Art.25. Conduct emergency PIV. Preserve all records. Do not move material until investigated.</p>
</details>
</div>
)"});

    // ── LESSON 5 ──────────────────────────────────────────────────────
    m_lessons.append({"Significant Quantities & SQ Thresholds", "[5]", R"(
<h1>Lesson 5: Significant Quantities and Material Categories</h1>
<div class="obj">
<strong>Learning Objectives:</strong><br>
Know the IAEA Significant Quantity for each material type, explain why different materials have different SQs, understand timeliness goals and why shorter MBPs matter.
</div>

<h2>What is a Significant Quantity (SQ)?</h2>
<p>The IAEA defines the <strong>Significant Quantity</strong> as the approximate amount of nuclear material from which a nuclear explosive device cannot be excluded, taking into account unavoidable losses in conversion and fabrication of material into a weapon component.</p>
<p>SQ is <em>not</em> the same as the critical mass for a chain reaction — it accounts for implosion design efficiency and fabrication losses and is therefore larger than the bare critical mass.</p>

<h2>IAEA Significant Quantities (Safeguards Glossary 2022, §3.19)</h2>
<table>
<tr><th>Material</th><th>SQ</th><th>Category</th><th>IAEA Timeliness Goal</th></tr>
<tr><td>Plutonium (Pu)</td><td>8 kg</td><td>Direct-use</td><td><strong>30 days</strong></td></tr>
<tr><td>Uranium-233</td><td>8 kg</td><td>Direct-use</td><td><strong>30 days</strong></td></tr>
<tr><td>HEU (≥20% U-235)</td><td>25 kg U-235</td><td>Direct-use</td><td><strong>30 days</strong></td></tr>
<tr><td>LEU / UF6 / UO2 (&lt;20%)</td><td>75 kg U-235</td><td>Indirect-use</td><td>90 days</td></tr>
<tr><td>Natural Uranium</td><td>10,000 kg U</td><td>Indirect-use</td><td>1 year</td></tr>
<tr><td>Depleted Uranium</td><td>20,000 kg U</td><td>Indirect-use</td><td>1 year</td></tr>
<tr><td>Thorium</td><td>20,000 kg Th</td><td>Indirect-use</td><td>1 year</td></tr>
</table>

<h2>Direct vs Indirect Use Materials</h2>
<p><strong>Direct-use materials</strong> (Pu, U-233, HEU) can be used in a nuclear explosive device <em>without further enrichment or processing</em>. They require the most stringent safeguards and shortest timeliness goals (30 days).</p>
<p><strong>Indirect-use materials</strong> (LEU, NatU, DU, Th) require additional processing (enrichment or transmutation) before they could be used in a weapon. They are less proliferation-sensitive, hence longer timeliness goals.</p>

<h2>Timeliness Goals and MBP Duration</h2>
<div class="warn">
The IAEA expects that material balance periods for direct-use materials should not exceed <strong>30 days</strong>. If a facility closes its balance only every 6 months, a diversion of one full SQ of Pu could go undetected for up to 6 months.<br><br>
The system flags this with an <span class="badge b-info">INFO</span> TIMELINESS-GOAL message. For Pu and HEU facilities, consider monthly or even weekly MBPs.
</div>

<h2>SQ Fraction — The Dashboard Widget</h2>
<p>The dashboard shows |MUF| as a percentage of one Significant Quantity for the selected material. Alert thresholds:</p>
<table>
<tr><th>SQ Fraction</th><th>Colour</th><th>Flag</th></tr>
<tr><td>&lt; 10%</td><td style="color:#15803d">Green</td><td>No SQ flag</td></tr>
<tr><td>10%–25%</td><td style="color:#b45309">Yellow</td><td>SQ-10PCT info</td></tr>
<tr><td>25%–50%</td><td style="color:#b45309">Amber</td><td>SQ-25PCT warning</td></tr>
<tr><td>50%–100%</td><td style="color:#c2410c">Orange</td><td>SQ-50PCT alert</td></tr>
<tr><td>≥ 100%</td><td style="color:#b91c1c">Red</td><td>SQ-EXCEEDED CRITICAL</td></tr>
</table>

<div class="quiz">
<strong>Self-Check:</strong>
<ol>
<li>A Pu reprocessing facility has MUF = 2 kg Pu. What is the SQ fraction? What flag fires?</li>
<li>Why does HEU have a lower SQ (25 kg) than natural uranium (10,000 kg)?</li>
<li>A LEU enrichment plant closes its books annually. Does the IAEA's timeliness goal for LEU permit this?</li>
</ol>
<details><summary style="color:#1a3a5c;font-weight:700;cursor:pointer;">Show Answers</summary>
<p><strong>1.</strong> SQ fraction = 2/8 × 100% = 25% → SQ-25PCT WARNING</p>
<p><strong>2.</strong> HEU contains sufficient U-235 (≥20%) to be used directly in a weapon; natural uranium must first be enriched, making it much harder to convert. Hence HEU SQ is far smaller.</p>
<p><strong>3.</strong> The IAEA timeliness goal for LEU is 90 days — annual closures do <em>not</em> meet this goal. The system will flag TIMELINESS-GOAL INFO.</p>
</details>
</div>
)"});

    // ── LESSON 6 ──────────────────────────────────────────────────────
    m_lessons.append({"Diversion Detection: CUSUM & SITMUF", "[6]", R"(
<h1>Lesson 6: Advanced Diversion Detection Methods</h1>
<div class="obj">
<strong>Learning Objectives:</strong><br>
Understand why single-period MUF testing is insufficient for protracted diversion, calculate CUSUM S+(t), interpret SITMUF and CUMUF alarms.
</div>

<h2>The Protracted Diversion Problem</h2>
<p>Imagine a saboteur who steals 0.3σ(MUF) of nuclear material each period — not enough to trigger a 3σ alarm in any single period. After 50 periods, they have stolen 15σ worth of material. Single-period testing would never catch this.</p>
<p>This is called <strong>protracted diversion</strong> — small, systematic theft across many periods. The solution requires <em>sequential statistical tests</em> that accumulate evidence over time.</p>

<h2>Page's CUSUM Test (1954)</h2>
<p>The Cumulative Sum (CUSUM) test accumulates normalised MUF signals across periods:</p>
<pre>SITMUF(t) = MUF(t) / σ(MUF)(t)   ← normalised signal

S⁺(t) = max(0,  S⁺(t-1) + SITMUF(t) − k)   ← detects material LOSS
S⁻(t) = max(0,  S⁻(t-1) − SITMUF(t) − k)   ← detects material GAIN

Alarm if S⁺(t) > h  or  S⁻(t) > h
Typical values: k = 0.5 (reference value), h = 5 (detection threshold)</pre>
<div class="ok">
<strong>Intuition:</strong> S⁺ is a running count of how much more material is being lost than expected. If MUF is consistently slightly positive (0.3σ each period), S⁺ climbs by 0.3 per period. After ~17 periods it crosses h = 5 and alarms — even though each individual MUF was insignificant.
</div>

<h2>Worked CUSUM Example</h2>
<pre>Period | SITMUF | S⁺ before | S⁺ after  | Status
  1    |  +0.4  |    0      | max(0, 0+0.4−0.5) = 0   | OK
  2    |  +0.7  |    0      | max(0, 0+0.7−0.5) = 0.2  | OK
  3    |  +0.6  |    0.2    | max(0, 0.2+0.6−0.5) = 0.3| OK
  ...  |  +0.5  |    ...    | accumulating...           | ...
  18   |  +0.5  |    4.7    | max(0, 4.7+0.5−0.5) = 4.7| OK
  19   |  +0.7  |    4.7    | max(0, 4.7+0.7−0.5) = 4.9| OK
  20   |  +0.6  |    4.9    | max(0, 4.9+0.6−0.5) = 5.0| ALARM!</pre>
<div class="crit">
<strong>CUSUM-ALARM-LOSS fired.</strong> Systematic material loss detected across 20 periods. In reality, no single period exceeded 2σ, but the cumulative pattern is unmistakeable evidence of protracted diversion.
</div>

<h2>SITMUF (Shipley-Picard, 1979)</h2>
<p>The Standardised Independently Transformed MUF is simply:</p>
<pre>SITMUF = MUF / σ(MUF)</pre>
<p>This normalises each period's MUF to its own uncertainty. |SITMUF| > 1.645 is significant at the 5% level (α = 0.05). SITMUF feeds directly into the CUSUM calculation above.</p>

<h2>CUMUF — Cumulative MUF</h2>
<p>The simple running total of MUF across all periods:</p>
<pre>CUMUF(t) = Σ MUF(i) for i = 1 to t</pre>
<p>If CUMUF > 2σ_CUMUF (where σ_CUMUF = √Σσ²(MUF)), the system raises a WARNING. A consistent upward trend in CUMUF suggests systematic measurement bias or protracted diversion.</p>

<div class="quiz">
<strong>Self-Check:</strong>
<ol>
<li>Why do sequential tests (CUSUM) outperform single-period tests for detecting protracted diversion?</li>
<li>In the CUSUM formula, what does the parameter k = 0.5 represent?</li>
<li>CUMUF after 5 periods = +120 kg. Each period's σ(MUF) ≈ 30 kg. Is this alarming?</li>
</ol>
<details><summary style="color:#1a3a5c;font-weight:700;cursor:pointer;">Show Answers</summary>
<p><strong>1.</strong> Single tests evaluate each period independently; sequential tests accumulate evidence across periods. Small consistent signals that are individually insignificant become collectively unmistakeable.</p>
<p><strong>2.</strong> k is the reference value — the expected signal size under the null hypothesis (no diversion). Setting k = 0.5σ makes the test insensitive to very small fluctuations while still sensitive to deliberate diversion.</p>
<p><strong>3.</strong> σ_CUMUF = √(5 × 30²) = √4500 ≈ 67 kg. CUMUF/σ_CUMUF = 120/67 ≈ 1.79σ — elevated but under 2σ. No CUMUF alarm yet, but worth monitoring.</p>
</details>
</div>
)"});

    // ── LESSON 7 ──────────────────────────────────────────────────────
    m_lessons.append({"Virtual Facility Simulator", "[7]", R"(
<h1>Lesson 7: Using the Virtual Facility Simulator</h1>
<div class="obj">
<strong>Learning Objectives:</strong><br>
Navigate the Virtual Facility Simulator, load and calculate scenarios, understand the pedagogical purpose of each facility type and scenario difficulty level.
</div>

<h2>What is the Virtual Facility Simulator?</h2>
<p>The Virtual Facility Simulator provides <strong>8 pre-built nuclear facility replicas</strong> with authentic NMA datasets derived from IAEA reference designs. Each facility has multiple scenarios at different difficulty levels, designed to teach specific safeguards concepts.</p>
<p>Access it via: <code>Tools → Virtual Facility Simulator</code> or the sidebar button.</p>

<h2>The Eight Facility Types</h2>
<table>
<tr><th>Facility</th><th>Material</th><th>Key Learning</th></tr>
<tr><td>UF6 Enrichment Plant</td><td>LEU / UF6</td><td>Basic NMA cycle, missing outputs, critical MUF</td></tr>
<tr><td>MOX Fuel Fabrication</td><td>Pu, MOX</td><td>Direct-use material, tight SQ limits, Pu accounting</td></tr>
<tr><td>PUREX Reprocessing</td><td>Pu, HEU, waste</td><td>Complex streams, shipper-receiver differences</td></tr>
<tr><td>Research Reactor</td><td>HEU/LEU fuel</td><td>Burn-up accounting, high uncertainty holdup</td></tr>
<tr><td>Uranium Mine/Mill</td><td>Natural U, yellowcake</td><td>Large-quantity, low-sensitivity accounting</td></tr>
<tr><td>UO2 Fuel Fabrication</td><td>LEU pellets</td><td>Conversion factor errors, waste streams</td></tr>
<tr><td>Pu Metal Store</td><td>Pu metal</td><td>Highest-sensitivity: every gram counts</td></tr>
<tr><td>Thorium Research Facility</td><td>Th / U-233</td><td>Emerging materials, indirect-use transition</td></tr>
</table>

<h2>Difficulty Levels</h2>
<div class="step">
<span class="step-num">B</span>
<strong>Beginner</strong> — Normal operating scenarios. All measurements within tolerance. MUF within 1σ. Teaches the standard NMA workflow without any alarms.
</div>
<div class="step">
<span class="step-num">I</span>
<strong>Intermediate</strong> — Anomalous scenarios: missing output streams, timing errors, elevated MUF in 2σ–3σ range. Teaches diagnostic flag interpretation and record review.
</div>
<div class="step">
<span class="step-num">A</span>
<strong>Advanced</strong> — Critical scenarios: simulated diversion signatures, CUSUM alarms, SQ-exceeded flags. Teaches mandatory reporting pathways and regulatory response.
</div>

<h2>Workflow: Loading a Scenario</h2>
<ol>
<li>Open the Simulator from the Tools menu</li>
<li>Select a facility from the left panel</li>
<li>Read the facility description and NMA structure in the centre panel</li>
<li>Select a scenario from the right panel</li>
<li>Read the scenario description, learning objective, and expected outcome</li>
<li>Click the <strong>NMA Data</strong> tab to see the full data table and preview calculation</li>
<li>Click <code>▶ Load into Calculator</code> to automatically populate all wizard fields</li>
<li>Verify the data in the wizard, then click <code>Calculate MUF</code></li>
<li>Compare the diagnostic output to the scenario's expected outcome</li>
</ol>

<h2>Recommended Learning Path</h2>
<div class="ok">
<strong>Start Here:</strong> UF6 Enrichment Plant → Scenario A (Beginner)<br>
Learn the normal NMA cycle before encountering anomalies.<br><br>
<strong>Then:</strong> UF6 Enrichment Plant → Scenario B (Intermediate)<br>
See what happens when an output is omitted.<br><br>
<strong>Then:</strong> MOX Fabrication → Scenario A (Beginner)<br>
Experience Pu accounting with its much smaller SQ (8 kg).<br><br>
<strong>Challenge:</strong> Pu Metal Store → Scenario C (Advanced)<br>
The most demanding safeguards scenario — every gram of Pu matters.
</div>
)"});

    // ── LESSON 8 ──────────────────────────────────────────────────────
    m_lessons.append({"Assessment & Certification", "[8]", R"(
<h1>Lesson 8: Assessment and Competency Check</h1>
<div class="obj">
<strong>Learning Objectives:</strong><br>
Demonstrate mastery of all key NMA concepts, apply the MUF calculator correctly to novel scenarios, interpret diagnostic output and recommend appropriate regulatory action.
</div>

<h2>Competency Areas</h2>
<p>After completing all lessons, you should be able to demonstrate competency in the following IAEA-defined NMA skills:</p>
<table>
<tr><th>Competency</th><th>Assessed Via</th></tr>
<tr><td>Calculate MUF, IB, σ(MUF) correctly</td><td>Lessons 1–2 + any scenario in the simulator</td></tr>
<tr><td>Navigate the 5-page data entry wizard</td><td>Lesson 3 + Load Example exercise</td></tr>
<tr><td>Interpret all diagnostic flags</td><td>Lesson 4 + Advanced simulator scenarios</td></tr>
<tr><td>Apply Significant Quantity thresholds</td><td>Lesson 5 + MOX/Pu simulator scenarios</td></tr>
<tr><td>Understand CUSUM/SITMUF</td><td>Lesson 6 + multi-period CUSUM scenarios</td></tr>
<tr><td>Use the Virtual Facility Simulator</td><td>Lesson 7 + complete all 3 enrichment plant scenarios</td></tr>
</table>

<h2>Final Practice Exercise</h2>
<div class="quiz">
<strong>Complete Scenario:</strong> A LEU fuel fabrication facility reports the following data for Q4 2025:<br><br>
BI = 12,450.0 kg ± 37.35 kg<br>
Inputs: Batch-1 3,200 kg ± 9.6 kg; Batch-2 2,800 kg ± 8.4 kg<br>
Outputs: Product pellets 4,100 kg ± 12.3 kg; Scrap 1,580 kg ± 15.8 kg; Samples 2.5 kg ± 0.075 kg<br>
EI = 12,754 kg ± 38.26 kg<br><br>
<strong>Tasks:</strong>
<ol>
<li>Calculate IB</li>
<li>Calculate MUF</li>
<li>Calculate σ(MUF)</li>
<li>Calculate the significance ratio</li>
<li>Determine what flags would fire</li>
<li>State what action (if any) is required</li>
</ol>
<details><summary style="color:#1a3a5c;font-weight:700;cursor:pointer;">Show Full Solution</summary>
<pre>ΣIn  = 3200 + 2800 = 6,000.0 kg
ΣOut = 4100 + 1580 + 2.5 = 5,682.5 kg

IB  = BI + ΣIn − ΣOut = 12450 + 6000 − 5682.5 = 12,767.5 kg
MUF = IB − EI = 12767.5 − 12754 = +13.5 kg

σ²(BI)  = 37.35² = 1395.0
σ²(In)  = 9.6² + 8.4² = 92.16 + 70.56 = 162.72
σ²(Out) = 12.3² + 15.8² + 0.075² = 151.29 + 249.64 + 0.006 ≈ 400.94
σ²(EI)  = 38.26² = 1463.83

σ²(MUF) = 1395.0 + 162.72 + 400.94 + 1463.83 = 3422.49
σ(MUF)  = √3422.49 ≈ 58.50 kg

Ratio = |13.5| / 58.50 ≈ 0.23σ  → NORMAL

SQ fraction (LEU, SQ=75 kg): 13.5/75 × 100% = 18% → INFO level

Flag: MUF-NORMAL (OK), SQ-10PCT (INFO)
Action: Routine accountancy report only. Monitor next MBP.
</pre>
</details>
</div>

<h2>Further Learning Resources</h2>
<ul>
<li>IAEA Safeguards Glossary 2022: <em>available from iaea.org</em></li>
<li>IAEA INFCIRC/153 (The Comprehensive Safeguards Agreement model)</li>
<li>Burr et al. (2013): "Exploratory Data Analysis for Radiation Portal Monitor Data" — doi:10.1155/2013/961360</li>
<li>IAEA-TECDOC-1169: Measurement uncertainty guidance for nuclear material accountancy</li>
<li>OSTI-1461843 (ORNL 2018): Safeguards Performance Analysis</li>
</ul>

<div class="ok">
<strong>Congratulations!</strong> You have completed the MUF Calculator Student Tutorial. Use the Virtual Facility Simulator to practise with realistic data, and the other tabs (Quick Start, IAEA Theory, Diagnostics Guide, Glossary, FAQ) as reference material throughout your studies.
</div>
)"});

    m_totalLessons = m_lessons.size();
    m_completed.fill(false, m_totalLessons);
}

QWidget* UserGuideDialog::buildInteractiveTutorial() {
    buildLessons();

    auto* page = new QWidget();
    auto* root = new QVBoxLayout(page);
    root->setContentsMargins(0,0,0,0); root->setSpacing(0);

    // ── Progress bar strip ────────────────────────────────────────────
    auto* progFrame = new QFrame();
    progFrame->setStyleSheet("QFrame{background:#f4f5f7;border-bottom:1px solid #e5e7eb;}");
    auto* progL = new QHBoxLayout(progFrame); progL->setContentsMargins(20,12,20,12);
    auto* progTitle = new QLabel("Tutorial Progress");
    progTitle->setStyleSheet("color:#1a3a5c;font-weight:700;font-size:13px;");
    m_progressLbl = new QLabel("0 / 8 lessons completed");
    m_progressLbl->setStyleSheet("color:#111827;font-size:13px;");
    m_progress = new QProgressBar();
    m_progress->setRange(0, m_totalLessons);
    m_progress->setValue(0);
    m_progress->setFixedHeight(10);
    m_progress->setStyleSheet(
        "QProgressBar{background:#e5e7eb;border-radius:5px;border:none;}"
        "QProgressBar::chunk{background:#5da832;border-radius:5px;}");
    m_progress->setFixedWidth(260);
    progL->addWidget(progTitle);
    progL->addSpacing(20);
    progL->addWidget(m_progress);
    progL->addSpacing(12);
    progL->addWidget(m_progressLbl);
    progL->addStretch();

    // Reset button
    auto* btnReset = new QPushButton("Reset Progress");
    btnReset->setStyleSheet(
        "QPushButton{background:white;color:#111827;border:1px solid #d1d5db;"
        "border-radius:6px;padding:6px 14px;font-size:12px;}"
        "QPushButton:hover{background:#f0f2f5;}");
    connect(btnReset, &QPushButton::clicked, this, [this]{
        m_completed.fill(false);
        m_progress->setValue(0);
        m_progressLbl->setText("0 / " + QString::number(m_totalLessons) + " lessons completed");
        // Refresh list icons
        for (int i = 0; i < m_lessonList->count(); ++i) {
            auto* it = m_lessonList->item(i);
            auto txt = it->text();
            if (txt.startsWith("✓ "))
                it->setText(m_lessons[i].icon + "  " + m_lessons[i].title);
        }
    });
    progL->addWidget(btnReset);
    root->addWidget(progFrame);

    // ── Splitter: lesson list | lesson content ────────────────────────
    auto* splitter = new QSplitter(Qt::Horizontal);
    splitter->setHandleWidth(1);

    // LEFT: lesson list
    auto* leftPanel = new QFrame();
    leftPanel->setFixedWidth(270);
    leftPanel->setStyleSheet("QFrame{background:#f4f5f7;border-right:1px solid #e5e7eb;}");
    auto* leftL = new QVBoxLayout(leftPanel);
    leftL->setContentsMargins(0,0,0,0); leftL->setSpacing(0);

    auto* listHdr = new QLabel("  LESSONS");
    listHdr->setStyleSheet("background:#1a3a5c;color:#a5c8e8;font-size:12px;font-weight:700;"
                           "letter-spacing:1.5px;padding:10px 16px;");
    leftL->addWidget(listHdr);

    m_lessonList = new QListWidget();
    m_lessonList->setStyleSheet(
        "QListWidget{background:#f4f5f7;border:none;font-size:13px;}"
        "QListWidget::item{padding:13px 16px;border-bottom:1px solid #e5e7eb;color:#374151;}"
        "QListWidget::item:selected{background:#1a3a5c;color:white;font-weight:700;}"
        "QListWidget::item:hover:!selected{background:#dce7f0;color:#0f2a4a;}");

    for (int i = 0; i < m_lessons.size(); ++i) {
        m_lessonList->addItem(m_lessons[i].icon + "  " + m_lessons[i].title);
    }
    connect(m_lessonList, &QListWidget::currentRowChanged, this, &UserGuideDialog::showLesson);
    leftL->addWidget(m_lessonList, 1);
    splitter->addWidget(leftPanel);

    // RIGHT: lesson content + navigation
    auto* rightPanel = new QWidget();
    auto* rightL = new QVBoxLayout(rightPanel);
    rightL->setContentsMargins(0,0,0,0); rightL->setSpacing(0);

    m_lessonStack = new QStackedWidget();
    for (int i = 0; i < m_lessons.size(); ++i) {
        m_lessonStack->addWidget(makeBrowser(CSS + m_lessons[i].html));
    }
    rightL->addWidget(m_lessonStack, 1);

    // Navigation bar
    auto* navBar = new QFrame();
    navBar->setStyleSheet("QFrame{background:white;border-top:1px solid #e5e7eb;}");
    auto* navL = new QHBoxLayout(navBar); navL->setContentsMargins(20,12,20,12);

    m_btnPrev = new QPushButton("← Previous Lesson");
    m_btnPrev->setStyleSheet(Styles::secondaryButtonStyle());
    m_btnPrev->setEnabled(false);

    m_btnNext = new QPushButton("Next Lesson →");
    m_btnNext->setStyleSheet(Styles::greenButtonStyle());

    auto* btnMarkDone = new QPushButton("Mark as Complete ✓");
    btnMarkDone->setStyleSheet(
        "QPushButton{background:#f0fdf4;color:#15803d;border:1.5px solid #16a34a;"
        "border-radius:8px;padding:10px 20px;font-size:13px;font-weight:700;}"
        "QPushButton:hover{background:#dcfce7;}");

    connect(m_btnPrev, &QPushButton::clicked, this, [this]{
        if (m_currentLesson > 0) {
            m_currentLesson--;
            m_lessonList->setCurrentRow(m_currentLesson);
        }
    });
    connect(m_btnNext, &QPushButton::clicked, this, [this]{
        markComplete(m_currentLesson);
        if (m_currentLesson < m_totalLessons - 1) {
            m_currentLesson++;
            m_lessonList->setCurrentRow(m_currentLesson);
        }
    });
    connect(btnMarkDone, &QPushButton::clicked, this, [this]{
        markComplete(m_currentLesson);
    });

    navL->addWidget(m_btnPrev);
    navL->addStretch();
    navL->addWidget(btnMarkDone);
    navL->addSpacing(12);
    navL->addWidget(m_btnNext);
    rightL->addWidget(navBar);

    splitter->addWidget(rightPanel);
    splitter->setSizes({270, 800});
    root->addWidget(splitter, 1);

    // Select first lesson
    m_lessonList->setCurrentRow(0);
    return page;
}

void UserGuideDialog::showLesson(int idx) {
    if (idx < 0 || idx >= m_totalLessons) return;
    m_currentLesson = idx;
    m_lessonStack->setCurrentIndex(idx);
    m_btnPrev->setEnabled(idx > 0);
    m_btnNext->setText(idx == m_totalLessons - 1 ? "Finish Tutorial" : "Next Lesson →");
}

void UserGuideDialog::markComplete(int idx) {
    if (idx < 0 || idx >= m_totalLessons) return;
    if (!m_completed[idx]) {
        m_completed[idx] = true;
        auto* item = m_lessonList->item(idx);
        if (item) item->setText("✓ " + m_lessons[idx].title);
        int done = m_completed.count(true);
        m_progress->setValue(done);
        m_progressLbl->setText(QString::number(done) + " / " + QString::number(m_totalLessons) + " lessons completed");
    }
}

// ═══════════════════════════════════════════════════════════════════════
//  TAB 1 — How to Use (Visual Walkthrough Landing Page)
// ═══════════════════════════════════════════════════════════════════════
QWidget* UserGuideDialog::buildHowToUse() {
    auto* page   = new QWidget();
    page->setStyleSheet("background:#f4f5f7;");
    auto* scroll = new QScrollArea();
    scroll->setWidgetResizable(true);
    scroll->setStyleSheet("QScrollArea{border:none;background:#f4f5f7;}");

    auto* inner  = new QWidget();
    inner->setStyleSheet("background:#f4f5f7;");
    auto* vl     = new QVBoxLayout(inner);
    vl->setContentsMargins(40,32,40,32); vl->setSpacing(24);

    // ── Hero title ────────────────────────────────────────────────────
    auto* heroFrame = new QFrame();
    heroFrame->setStyleSheet("background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
                             "stop:0 #1a3a5c,stop:1 #2d5680);border-radius:14px;");
    auto* heroL = new QVBoxLayout(heroFrame); heroL->setContentsMargins(36,28,36,28);
    auto* ht = new QLabel("Welcome to the MUF Calculator");
    ht->setStyleSheet("color:white;font-size:29px;font-weight:900;");
    auto* hs = new QLabel("A step-by-step guide to performing IAEA Nuclear Material Accounting");
    hs->setStyleSheet("color:#a5c8e8;font-size:13px;");
    heroL->addWidget(ht); heroL->addWidget(hs);
    vl->addWidget(heroFrame);

    // ── 6-step visual walkthrough cards ──────────────────────────────
    auto* stepsTitle = new QLabel("How to Perform a MUF Calculation in 6 Steps");
    stepsTitle->setStyleSheet("color:#1a3a5c;font-size:13px;font-weight:900;"
                              "border-bottom:3px solid #1a3a5c;padding-bottom:10px;");
    vl->addWidget(stepsTitle);

    struct Step { QString num; QString title; QString desc; QString tip; QString color; };
    QVector<Step> steps = {
        {"1", "Open the Calculation Wizard",
         "Click <b>▶ New Calculation</b> in the left sidebar, or go to Calculate → New Calculation. "
         "The 5-page wizard will open. You can also click <b>Load Example</b> to see a pre-filled "
         "UF6 enrichment plant scenario — ideal for your first run.",
         "Shortcut: Ctrl+N opens a new calculation instantly.",
         "#1a3a5c"},

        {"2", "Enter Facility Information (Page 1)",
         "Fill in the facility name, IAEA facility ID, material type (UF6, HEU, Pu, etc.), "
         "operator name, Material Balance Period (MBP) identifier, and the period start and end dates. "
         "All fields turn <b style='color:#16a34a'>green</b> when valid.",
         "The material type affects which Significant Quantity (SQ) threshold is used for diversion detection.",
         "#2d5680"},

        {"3", "Enter Beginning Inventory — BI (Page 2)",
         "Enter the nuclear material quantity measured at the start of the period (in kg) "
         "and its 1σ measurement uncertainty (also in kg). "
         "Example: 5,000.0 kg ± 12.5 kg. Uncertainty must be greater than zero.",
         "Never enter zero for uncertainty — every measurement instrument has some error. "
         "Use 0.5% of the quantity as a starting estimate if you are unsure.",
         "#5da832"},

        {"4", "Enter All Transfers — Inputs & Outputs (Pages 3 & 4)",
         "Add every material receipt (input) and every shipment or waste stream (output). "
         "Click <b>+ Add Row</b> for each batch. Include process samples, analytical samples "
         "shipped to labs, tails at enrichment plants, scrap, and process hold-up changes. "
         "Missing outputs cause large false negative MUF values.",
         "Forgotten outputs are the most common data entry mistake. "
         "Use the IAEA KMP checklist: every material stream crossing a KMP boundary must appear here.",
         "#b45309"},

        {"5", "Enter Ending Inventory — EI (Page 5) and Calculate",
         "Enter the physically measured ending inventory with its uncertainty. "
         "As you type, the page shows a live preview of IB, MUF, and σ(MUF). "
         "When all fields are green, click <b>▶ Calculate MUF</b> to run the full analysis.",
         "The live preview on Page 5 lets you catch obvious mistakes before running the full diagnostic engine.",
         "#c2410c"},

        {"6", "Review Results and Diagnostic Flags",
         "The results panel shows 4 tabs: <b>Summary</b> (MUF value and status), "
         "<b>Equations</b> (step-by-step calculation), "
         "<b>Uncertainty</b> (which measurement contributes most to σ(MUF)), and "
         "<b>Diagnostics</b> (all safeguards checks with IAEA references). "
         "Go to <b>File → Export Report</b> to save an HTML report.",
         "If the ratio exceeds 3σ, regulatory reporting under INFCIRC/153 Article 25 is required. "
         "This is not a software decision — it is a legal obligation.",
         "#b91c1c"},
    };

    for (auto& s : steps) {
        auto* card = new QFrame();
        card->setStyleSheet("QFrame{background:white;border:1px solid #e5e7eb;border-radius:12px;"
                            "border-left:6px solid " + s.color + ";}");
        auto* cl = new QHBoxLayout(card); cl->setContentsMargins(0,0,20,0); cl->setSpacing(0);

        // Number badge
        auto* badge = new QLabel(s.num);
        badge->setFixedSize(72,72);
        badge->setAlignment(Qt::AlignCenter);
        badge->setStyleSheet("background:" + s.color + ";color:white;font-size:32px;font-weight:900;"
                             "border-radius:6px 0 0 6px;min-width:72px;max-width:72px;");
        cl->addWidget(badge);
        cl->addSpacing(18);

        auto* textL = new QVBoxLayout(); textL->setSpacing(6);
        textL->setContentsMargins(0,16,0,16);
        auto* title = new QLabel(s.title);
        title->setStyleSheet("color:#1a3a5c;font-size:13px;font-weight:800;");
        auto* desc = new QLabel(s.desc);
        desc->setStyleSheet("color:#374151;font-size:13px;line-height:1.5;");
        desc->setWordWrap(true);
        desc->setTextFormat(Qt::RichText);

        auto* tipFrame = new QLabel("Tip: " + s.tip);
        tipFrame->setStyleSheet("background:#eff6ff;color:#1d4ed8;border:1px solid #bfdbfe;"
                                "border-radius:6px;padding:6px 12px;font-size:12px;");
        tipFrame->setWordWrap(true);

        textL->addWidget(title); textL->addWidget(desc); textL->addWidget(tipFrame);
        cl->addLayout(textL, 1);
        vl->addWidget(card);
    }

    // ── Quick access section ──────────────────────────────────────────
    auto* qaTitle = new QLabel("Quick Access to Other Features");
    qaTitle->setStyleSheet("color:#1a3a5c;font-size:13px;font-weight:900;"
                           "border-bottom:3px solid #1a3a5c;padding-bottom:10px;");
    vl->addWidget(qaTitle);

    auto* qaRow = new QHBoxLayout(); qaRow->setSpacing(16);

    auto makeQACard = [](const QString& icon, const QString& title, const QString& desc,
                         const QString& color) -> QFrame* {
        auto* c = new QFrame();
        c->setStyleSheet("QFrame{background:white;border:1px solid #e5e7eb;border-radius:10px;"
                         "border-top:4px solid " + color + ";}");
        auto* l = new QVBoxLayout(c); l->setContentsMargins(18,16,18,16); l->setSpacing(8);
        auto* ico = new QLabel(icon);
        ico->setStyleSheet("font-size:36px;");
        auto* t = new QLabel(title);
        t->setStyleSheet("color:#1a3a5c;font-size:13px;font-weight:800;");
        auto* d = new QLabel(desc);
        d->setStyleSheet("color:#111827;font-size:12px;");
        d->setWordWrap(true);
        l->addWidget(ico); l->addWidget(t); l->addWidget(d); l->addStretch();
        return c;
    };

    qaRow->addWidget(makeQACard("⚙", "Virtual Facility Simulator",
        "Practice with 4 realistic nuclear facilities and 12 pre-built NMA scenarios. "
        "Beginner, Intermediate, and Advanced difficulty levels.",
        "#1a3a5c"));

    qaRow->addWidget(makeQACard("📚", "Interactive Lessons",
        "8 structured lessons covering NMA theory, uncertainty propagation, "
        "diversion detection, and CUSUM sequential testing.",
        "#5da832"));

    qaRow->addWidget(makeQACard("📄", "IAEA Theory Reference",
        "Complete theoretical background including all IAEA equations, MBA/KMP definitions, "
        "INFCIRC/153 requirements, and Significant Quantity tables.",
        "#2d5680"));

    qaRow->addWidget(makeQACard("⚠", "Diagnostics Guide",
        "All 12 safeguards checks explained with thresholds, what triggers each flag, "
        "and the regulatory action required for each severity level.",
        "#b45309"));

    vl->addLayout(qaRow);

    scroll->setWidget(inner);
    auto* outerL = new QVBoxLayout(page); outerL->setContentsMargins(0,0,0,0);
    outerL->addWidget(scroll);
    return page;
}

// ═══════════════════════════════════════════════════════════════════════
//  TAB 2 — Quick Start
// ═══════════════════════════════════════════════════════════════════════
QWidget* UserGuideDialog::buildQuickStart() {
    return makeBrowser(R"(
<h1>Quick Start Reference</h1>
<p>A condensed reference for performing a MUF calculation. See the <strong>Interactive Tutorial</strong>
tab for detailed explanations of each step.</p>

<h2>5-Step Workflow</h2>
<div class="step"><span class="step-num">1</span>
<strong>Launch Wizard</strong> — Click <code>▶ New Calculation</code> in the sidebar.<br>
Use <code>⚡ Load Example</code> for a pre-filled UF6 demonstration.
</div>
<div class="step"><span class="step-num">2</span>
<strong>Facility Info (Page 1)</strong> — Enter facility name, ID, material type, operator, MBP ID, and period dates.
All fields must turn green before proceeding.
</div>
<div class="step"><span class="step-num">3</span>
<strong>Inventories (Pages 2 &amp; 5)</strong> — Enter BI and EI with quantities and 1σ uncertainties (must be &gt; 0).
</div>
<div class="step"><span class="step-num">4</span>
<strong>Transfers (Pages 3 &amp; 4)</strong> — Add ALL inputs and outputs. Include every waste stream, sample, and hold-up.
</div>
<div class="step"><span class="step-num">5</span>
<strong>Calculate &amp; Interpret</strong> — Click <code>▶ Calculate MUF</code>. Review the 4 result tabs.
Check diagnostic flags and take required regulatory action.
</div>

<h2>Alert Thresholds Summary</h2>
<table>
<tr><th>|MUF|/σ</th><th>Flag</th><th>Action</th></tr>
<tr><td>≤ 1σ</td><td><span class="badge b-ok">OK</span></td><td>Routine report only</td></tr>
<tr><td>1σ–2σ</td><td><span class="badge b-info">INFO</span></td><td>Monitor next period</td></tr>
<tr><td>2σ–3σ</td><td><span class="badge b-warn">WARNING</span></td><td>Review all records</td></tr>
<tr><td>&gt;3σ</td><td><span class="badge b-crit">CRITICAL</span></td><td>Report to IAEA immediately</td></tr>
</table>

<h2>IAEA Significant Quantities (Quick Reference)</h2>
<table>
<tr><th>Material</th><th>SQ</th><th>Timeliness Goal</th></tr>
<tr><td>Pu / U-233</td><td>8 kg</td><td>30 days</td></tr>
<tr><td>HEU ≥20%</td><td>25 kg U-235</td><td>30 days</td></tr>
<tr><td>LEU / UF6 &lt;20%</td><td>75 kg U-235</td><td>90 days</td></tr>
<tr><td>Natural U</td><td>10,000 kg</td><td>1 year</td></tr>
<tr><td>Depleted U / Th</td><td>20,000 kg</td><td>1 year</td></tr>
</table>
)");
}

// ═══════════════════════════════════════════════════════════════════════
//  TAB 3 — IAEA Theory
// ═══════════════════════════════════════════════════════════════════════
QWidget* UserGuideDialog::buildTheory() {
    return makeBrowser(R"(
<h1>IAEA Theory &mdash; Nuclear Material Accounting</h1>

<h2>1. What is Nuclear Material Accounting (NMA)?</h2>
<p>NMA is the system by which a State and the IAEA track every kilogram of nuclear material through the fuel cycle. It is the <strong>most fundamental safeguards measure</strong> under IAEA INFCIRC/153. The principle is conservation of matter: any material entering or leaving a defined area must be recorded.</p>

<h2>2. Material Balance Area (MBA)</h2>
<p>A <strong>Material Balance Area (MBA)</strong> is a defined region of a nuclear facility for which all inputs, outputs, and inventory can be measured. Key Measurement Points (KMPs) are located at boundaries where material is quantified. Each MBP begins and ends with a physical inventory taking (PIV) at every KMP.</p>

<h2>3. The Four IAEA Equations</h2>

<h3>Equation 1: Book Inventory (IB)</h3>
<pre>IB = BI + ΣInputs − ΣOutputs</pre>
<p>The <strong>Book Inventory</strong> is what accounting records say should be present. It is calculated, not measured.</p>

<h3>Equation 2: Material Unaccounted For (MUF)</h3>
<pre>MUF = IB − EI</pre>
<p>The difference between what the books say (IB) and what is physically measured (EI).</p>

<h3>Equation 3: Uncertainty Propagation — σ(MUF)</h3>
<pre>σ(MUF) = √[σ²(BI) + σ²(ΣInputs) + σ²(ΣOutputs) + σ²(EI)]</pre>

<h3>Equation 4: Significance Ratio</h3>
<pre>Ratio = |MUF| / σ(MUF)</pre>

<h3>Equation 5: Uncertainty Contribution (Cᵢ)</h3>
<pre>Cᵢ = [σ²(i) / σ²(MUF)] × 100%</pre>

<h2>4. Significant Quantity (SQ)</h2>
<table>
<tr><th>Material</th><th>SQ (kg)</th><th>Category</th><th>Timeliness</th></tr>
<tr><td>Plutonium (Pu)</td><td>8</td><td>Direct-use</td><td>30 days</td></tr>
<tr><td>U-233</td><td>8</td><td>Direct-use</td><td>30 days</td></tr>
<tr><td>HEU (≥20% U-235)</td><td>25</td><td>Direct-use</td><td>30 days</td></tr>
<tr><td>LEU / UF6 / UO2 (&lt;20%)</td><td>75</td><td>Indirect-use</td><td>90 days</td></tr>
<tr><td>Natural Uranium</td><td>10,000</td><td>Indirect-use</td><td>1 year</td></tr>
<tr><td>Depleted Uranium</td><td>20,000</td><td>Indirect-use</td><td>1 year</td></tr>
<tr><td>Thorium</td><td>20,000</td><td>Indirect-use</td><td>1 year</td></tr>
</table>

<h2>5. Diversion Detection Methods</h2>

<h3>5.1 Page's CUSUM Test</h3>
<pre>S⁺(t) = max(0, S⁺(t-1) + SITMUF(t) − k)
S⁻(t) = max(0, S⁻(t-1) − SITMUF(t) − k)</pre>
<p>Detects <strong>protracted diversion</strong> — small thefts across many MBPs. Alarm when S⁺ or S⁻ &gt; h = 5. (Source: Burr et al. 2013)</p>

<h3>5.2 SITMUF</h3>
<pre>SITMUF = MUF / σ(MUF)</pre>
<p>|SITMUF| &gt; 1.645 significant at 5% level. Introduced by Shipley and Picard (1979).</p>

<h3>5.3 CUMUF</h3>
<p>Cumulative sum of MUF across all MBPs. Consistent positive trend indicates systematic loss or bias.</p>
)");
}

// ═══════════════════════════════════════════════════════════════════════
//  TAB 4 — Diagnostics Guide
// ═══════════════════════════════════════════════════════════════════════
QWidget* UserGuideDialog::buildDiagnosticsGuide() {
    return makeBrowser(R"(
<h1>Diagnostics Guide</h1>
<p>The system runs up to <strong>12 diagnostic checks</strong> after every calculation. This guide explains every flag.</p>

<h2>Sigma-Based Flags</h2>

<div class="ok">
<strong><span class="badge b-ok">OK</span> MUF-NORMAL</strong><br>
|MUF| &le; 1σ. Normal variation. File routine accountancy report.
</div>

<div class="tip">
<strong><span class="badge b-info">INFO</span> MUF-EXCEED-1σ</strong><br>
1σ &lt; |MUF| &le; 2σ. Elevated but statistically normal ~32% of the time. Monitor next MBP.
</div>

<div class="warn">
<strong><span class="badge b-warn">WARNING</span> MUF-EXCEED-2σ</strong><br>
2σ &lt; |MUF| &le; 3σ. Statistically significant (5% level). Review ALL transfer records and schedule early inspection.
</div>

<div class="crit">
<strong><span class="badge b-crit">CRITICAL</span> MUF-EXCEED-3σ</strong><br>
|MUF| &gt; 3σ. Only 0.3% probability by chance. <strong>Immediately</strong> notify IAEA Safeguards and State authority. Initiate full PIV.
</div>

<h2>Physical Integrity Flags</h2>

<div class="crit">
<strong><span class="badge b-crit">CRITICAL</span> ZERO-SIGMA</strong><br>
σ(MUF) = 0. All uncertainties are zero — physically impossible. Enter realistic uncertainties.
</div>

<div class="warn">
<strong><span class="badge b-alert">ALERT</span> NEGATIVE-MUF</strong><br>
MUF significantly negative. Causes: omitted output, overstated EI, undeclared input. Verify all output records.
</div>

<h2>Significant Quantity Flags</h2>

<div class="crit">
<strong><span class="badge b-crit">CRITICAL</span> SQ-EXCEEDED</strong><br>
|MUF| &ge; 1 SQ. Weapons-relevant quantity unaccounted for. Report immediately under INFCIRC/153 Art.25.
</div>

<div class="warn">
<strong><span class="badge b-alert">ALERT</span> SQ-50PCT</strong><br>
|MUF| between 50%–100% of 1 SQ. Notify authorities and perform verification inventory immediately.
</div>

<div class="warn">
<strong><span class="badge b-warn">WARNING</span> SQ-25PCT</strong><br>
|MUF| between 25%–50% of 1 SQ. Increase measurement frequency next period.
</div>

<h2>Statistical Trend Flags</h2>

<div class="crit">
<strong><span class="badge b-crit">CRITICAL</span> CUSUM-ALARM-LOSS</strong><br>
S⁺(t) exceeded threshold. Systematic material loss across multiple MBPs. Pattern consistent with protracted diversion. Initiate special inspection.
</div>

<div class="warn">
<strong><span class="badge b-alert">ALERT</span> CUSUM-ALARM-GAIN</strong><br>
S⁻(t) exceeded threshold. Systematic material gain — possible undeclared inputs. Verify all receipts and SRD values.
</div>

<div class="warn">
<strong><span class="badge b-warn">WARNING</span> CUMUF-ELEVATED</strong><br>
Cumulative MUF &gt; 2σ across periods. Review all historical balance periods for systematic pattern.
</div>

<h2>Measurement Quality Flags</h2>

<div class="warn">
<strong><span class="badge b-warn">WARNING</span> DOMINANT-UNCERTAINTY-SOURCE</strong><br>
One term contributes &ge; 50% to σ²(MUF). Prioritise recalibration of that instrument. Reference: IAEA-TECDOC-1169.
</div>

<div class="warn">
<strong><span class="badge b-alert">ALERT</span> HIGH-MUF-TO-THROUGHPUT</strong><br>
|MUF| &gt; 5% of total throughput. Absolute discrepancy is operationally significant.
</div>

<div class="tip">
<strong><span class="badge b-info">INFO</span> TIMELINESS-GOAL</strong><br>
MBP duration exceeds IAEA timeliness goal: 30 days for Pu/HEU, 90 days for LEU/NatU. Consider shorter balance periods.
</div>
)");
}

// ═══════════════════════════════════════════════════════════════════════
//  TAB 5 — Glossary
// ═══════════════════════════════════════════════════════════════════════
QWidget* UserGuideDialog::buildGlossary() {
    return makeBrowser(R"(
<h1>Nuclear Safeguards Glossary</h1>
<p>Key terms based on IAEA Safeguards Glossary 2022 unless otherwise noted.</p>
<dl>
<dt>Beginning Inventory (BI)</dt><dd>Physically measured nuclear material at the start of a MBP, verified during opening PIV.</dd>
<dt>Book Inventory (IB)</dt><dd>IB = BI + ΣInputs − ΣOutputs. Calculated, not measured.</dd>
<dt>CUSUM</dt><dd>Cumulative Sum Control Chart — detects protracted diversion across multiple MBPs. Developed by Page (1954), applied to safeguards by Burr et al. (2013).</dd>
<dt>Diversion</dt><dd>Use of nuclear material for purposes other than those declared — specifically to manufacture nuclear weapons.</dd>
<dt>Ending Inventory (EI)</dt><dd>Physically measured nuclear material at end of MBP, verified during closing PIV.</dd>
<dt>HEU</dt><dd>Highly Enriched Uranium — ≥20% U-235. SQ = 25 kg. Direct-use material.</dd>
<dt>INFCIRC/153</dt><dd>Standard IAEA safeguards agreement model — the legal foundation for comprehensive safeguards.</dd>
<dt>Key Measurement Point (KMP)</dt><dd>A location where nuclear material can be measured as it crosses an MBA boundary.</dd>
<dt>LEU</dt><dd>Low Enriched Uranium — 0.711%–20% U-235. SQ = 75 kg U-235. Indirect-use.</dd>
<dt>Material Balance Area (MBA)</dt><dd>A defined facility area for which a complete material balance can be drawn up.</dd>
<dt>Material Balance Period (MBP)</dt><dd>Time interval between consecutive Physical Inventory Verifications.</dd>
<dt>MOX</dt><dd>Mixed Oxide Fuel — contains Pu (SQ = 8 kg Pu). Direct-use material.</dd>
<dt>MUF</dt><dd>Material Unaccounted For = IB − EI. Also called Inventory Difference (ID) in some national systems.</dd>
<dt>PIV</dt><dd>Physical Inventory Verification — IAEA inspection activity where all nuclear material is located, identified, and measured.</dd>
<dt>Plutonium (Pu)</dt><dd>Reactor-produced transuranic element. SQ = 8 kg (&lt;80% Pu-238). Direct-use material — most proliferation-sensitive in bulk facilities.</dd>
<dt>Protracted Diversion</dt><dd>Small thefts across many MBPs, each insufficient to trigger a single-period alarm. Detected by CUSUM and CUMUF.</dd>
<dt>Shipper-Receiver Difference (SRD)</dt><dd>Difference between declared and measured quantity at a transfer. Large SRDs indicate measurement bias or undeclared transfers.</dd>
<dt>Significant Quantity (SQ)</dt><dd>Approximate amount from which a nuclear explosive device cannot be excluded (Safeguards Glossary 2022 §3.19).</dd>
<dt>SITMUF</dt><dd>Standardised Independently Transformed MUF = MUF/σ(MUF). |SITMUF| &gt; 1.645 is significant at 5%. Introduced by Shipley and Picard (1979).</dd>
<dt>UF6</dt><dd>Uranium Hexafluoride — gaseous compound used in centrifuge enrichment. Uranium content ≈67.6% by mass.</dd>
<dt>σ(MUF)</dt><dd>1-sigma uncertainty of MUF = √[σ²(BI) + σ²(ΣIn) + σ²(ΣOut) + σ²(EI)].</dd>
</dl>
)");
}

// ═══════════════════════════════════════════════════════════════════════
//  TAB 6 — FAQ
// ═══════════════════════════════════════════════════════════════════════
QWidget* UserGuideDialog::buildFAQ() {
    return makeBrowser(R"(
<h1>Frequently Asked Questions</h1>

<h2>Getting Started</h2>
<h3>Q: I don't have real facility data — what should I do?</h3>
<p>Use the <strong>Virtual Facility Simulator</strong> (Tools menu). It provides 8 facility types with authentic NMA datasets. Alternatively, click <code>Load Example</code> in the wizard for a quick UF6 demonstration.</p>

<h3>Q: What units does the system use?</h3>
<p>All quantities are in <strong>kilograms (kg)</strong> of nuclear material element weight. For UF6, note that 1 kg UF6 contains approximately 0.676 kg uranium — enter the uranium mass, not the compound mass.</p>

<h3>Q: What is a realistic uncertainty value?</h3>
<table>
<tr><th>Measurement Method</th><th>Typical 1σ (%)</th></tr>
<tr><td>Cylinder weighing (UF6)</td><td>0.05%</td></tr>
<tr><td>Uranium in solution</td><td>0.1–0.3%</td></tr>
<tr><td>Plutonium by calorimetry</td><td>0.1–0.5%</td></tr>
<tr><td>Reactor burn-up calculation</td><td>1–3%</td></tr>
<tr><td>Process hold-up measurement</td><td>10–30%</td></tr>
</table>

<h2>Understanding Results</h2>
<h3>Q: My MUF is positive — does that mean material was stolen?</h3>
<p>Not necessarily. Positive MUF means EI &lt; IB — measured inventory is less than expected. The most common causes are measurement errors and unaccounted process hold-up. Only if |MUF| &gt; 2σ or 3σ should you be concerned, and even then, clerical error is far more likely than actual diversion.</p>

<h3>Q: My MUF is negative — is that normal?</h3>
<p>A moderately negative MUF (within 1–2σ) is normal — EI was measured slightly higher than expected. A large negative MUF suggests: an omitted output, high-biased EI measurement, or (rarely) undeclared material input.</p>

<h3>Q: The CUSUM flag fired but my individual MUF looks normal. Why?</h3>
<p>CUSUM detects <em>trends across multiple periods</em>. If MUF is consistently slightly positive over many periods, CUSUM accumulates evidence and alarms even though each individual period looks acceptable. This is by design — it catches protracted diversion.</p>

<h2>Data Entry</h2>
<h3>Q: Why are some characters blocked?</h3>
<p>The system blocks injection-attack characters: <code>&lt; &gt; ; " ' \ | ` $ { }</code>. Use letters, numbers, spaces, hyphens, dots, and underscores in all text fields.</p>

<h3>Q: How many rows can I add per table?</h3>
<p>Maximum 100 rows per table — matching the practical limit of real nuclear material accounting records for one MBP.</p>

<h3>Q: The period end date is rejected — why?</h3>
<p>Future dates are not allowed (you cannot close a balance for a period that hasn't happened). Periods longer than 5 years are also rejected as likely data entry errors.</p>

<h2>Technical</h2>
<h3>Q: Is the calculation identical to the IAEA standard?</h3>
<p>Yes. The four core equations (IB, MUF, σ(MUF), Cᵢ) are implemented exactly as specified in IAEA INFCIRC/153 and the Safeguards Glossary 2022. Diagnostic checks follow Burr et al. (2013) for CUSUM/SITMUF and OSTI-1461843 for CUMUF analysis.</p>

<h3>Q: How do I reset the CUSUM accumulator?</h3>
<p>The CUSUM accumulator resets when you start a new session or use <code>Calculate → Clear History</code>. In real safeguards practice the CUSUM is carried forward across all MBPs without reset unless a PIV verifies the balance to zero.</p>

<h3>Q: Can I save my data?</h3>
<p>The application exports HTML and text reports via <code>File → Export Report</code>. Export immediately after each calculation to preserve all results.</p>
)");
}

// ─────────────────────────────────────────────────────────────────
//  IAEA watermark – 15 % opacity backdrop
// ─────────────────────────────────────────────────────────────────
void UserGuideDialog::paintEvent(QPaintEvent* e)
{
    QDialog::paintEvent(e);
    QPainter p(this);
    IAEAWatermark::paint(p, rect(), 0.15);
}

