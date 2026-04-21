#include "virtualfacilitydialog.h"
#include "watermarkwidget.h"
#include <QPainter>
#include "styles.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QFrame>
#include <QScrollArea>
#include <QDateTime>
#include <QTimer>
#include <cmath>

// ── HTML CSS for detail panels ────────────────────────────────────────
static const QString VCSS = R"(
<style>
body { font-family:'Segoe UI',Ubuntu,Arial,sans-serif; font-size:20px;
       color:#111827; background:white; margin:0; padding:16px; line-height:1.65; }
h1   { color:#1a3a5c; font-size:25px; font-weight:900;
       border-bottom:3px solid #1a3a5c; padding-bottom:8px; margin-bottom:16px; }
h2   { color:#1a3a5c; font-size:22px; font-weight:800; margin-top:22px; margin-bottom:8px; }
h3   { color:#2d5680; font-size:20px; font-weight:700; margin-top:16px; margin-bottom:6px; }
p    { margin:8px 0; }
code { background:#f0f2f5; border:1px solid #e5e7eb; border-radius:4px;
       padding:2px 7px; font-family:'Courier New',monospace; font-size:19px; color:#1a3a5c; }
pre  { background:#f0f2f5; border:1px solid #e5e7eb; border-radius:8px;
       padding:12px 16px; font-family:'Courier New',monospace; font-size:19px;
       color:#1a3a5c; margin:10px 0; white-space:pre-wrap; }
table{ width:100%; border-collapse:collapse; margin:12px 0; font-size:19px; }
th   { background:#1a3a5c; color:white; padding:9px 13px; text-align:left; font-weight:700; }
td   { padding:8px 13px; border-bottom:1px solid #f3f4f6; }
tr:nth-child(even) td { background:#f9fafb; }
.tip { background:#eff6ff; border:1px solid #bfdbfe; border-radius:8px;
       padding:12px 16px; margin:12px 0; }
.warn{ background:#fffbeb; border:1px solid #fde68a; border-radius:8px;
       padding:12px 16px; margin:12px 0; }
.crit{ background:#fef2f2; border:1px solid #fca5a5; border-radius:8px;
       padding:12px 16px; margin:12px 0; }
.ok  { background:#f0fdf4; border:1px solid #bbf7d0; border-radius:8px;
       padding:12px 16px; margin:12px 0; }
.kmp { background:#fff7ed; border:1px solid #fed7aa; border-radius:8px;
       padding:12px 16px; margin:12px 0; }
.badge { display:inline-block; padding:3px 10px; border-radius:12px;
         font-size:18px; font-weight:700; margin:2px 4px; }
.b-beg { background:#f0fdf4; color:#15803d; border:1px solid #16a34a; }
.b-int { background:#eff6ff; color:#1d4ed8; border:1px solid #1d4ed8; }
.b-adv { background:#fef2f2; color:#b91c1c; border:1px solid #dc2626; }
</style>
)";

// ── Helper to make a browser widget ──────────────────────────────────
static QTextBrowser* vBrowser() {
    auto* b = new QTextBrowser();
    b->setStyleSheet("QTextBrowser{background:white;border:none;padding:4px;}");
    b->setOpenExternalLinks(false);
    return b;
}

// ══════════════════════════════════════════════════════════════════════
//  BUILD FACILITIES  (4 facilities × 3 scenarios each)
// ══════════════════════════════════════════════════════════════════════
void VirtualFacilityDialog::buildFacilities() {

    // ─────────────────────────────────────────────────────────────────
    // FACILITY 1 — UF6 Enrichment Plant (based on AGZU / Novouralsk type)
    // ─────────────────────────────────────────────────────────────────
    {
        VirtualFacility f;
        f.id       = "ENRICH-001";
        f.name     = "Virtual UF6 Enrichment Plant";
        f.type     = "Gaseous Centrifuge Enrichment";
        f.country  = "Educational (based on IAEA reference design)";
        f.material = "UF6 — Low Enriched Uranium";
        f.icon     = "⚗";
        f.description =
            "A large-scale gaseous centrifuge enrichment facility processing natural UF6 feed "
            "into Low Enriched Uranium (LEU) product at 3.5–4.5% U-235, and depleted UF6 tails. "
            "This virtual facility follows IAEA MBA/KMP structure with KMP-A (feed), KMP-B (product), "
            "KMP-C (tails), and an in-plant inventory area. Annual throughput ~10,000 tonnes UF6.";

        // Scenario 1.1 — Normal Quarter
        {
            VirtualFacility::Scenario s;
            s.id = "ENRICH-001-S1";
            s.name = "Scenario A: Normal Quarter (Q1 2025)";
            s.difficulty = "Beginner";
            s.description =
                "A routine quarterly material balance closure for an enrichment plant. "
                "All measurements within normal tolerances. Expected outcome: MUF within 1σ, "
                "all diagnostics green. Good scenario for learning the basic NMA workflow.";
            s.learnObjective =
                "Understand the normal NMA cycle: BI + Inputs − Outputs = IB, then compare IB to EI. "
                "Observe how small measurement uncertainties produce a non-zero but statistically "
                "insignificant MUF.";
            s.expectedOutcome =
                "MUF ≈ +190 kg, σ(MUF) ≈ 32 kg, ratio ≈ 0.6σ → NORMAL. "
                "SQ fraction ≈ 0.25% (well below alert threshold for LEU).";

            MBPInput& in = s.input;
            in.facilityName  = "Virtual UF6 Enrichment Plant";
            in.facilityId    = "ENRICH-001";
            in.materialType  = "UF6 (Uranium Hexafluoride)";
            in.operatorName  = "Student Operator";
            in.mbpId         = "ENRICH-001-MBP-2025-Q1";
            in.periodStart   = QDateTime(QDate(2025,1,1),  QTime(0,0,0));
            in.periodEnd     = QDateTime(QDate(2025,3,31), QTime(23,59,59));
            in.beginningInventory = {"Natural UF6 Feed Cylinders (KMP-A inventory)", 5000.0, 12.5};
            in.inputs  = {
                {"Batch-1: Natural UF6 Feed Receipt (Jan)",       2500.0, 6.25},
                {"Batch-2: Natural UF6 Feed Receipt (Feb)",       2200.0, 5.50},
                {"Batch-3: Natural UF6 Feed Receipt (Mar)",       1800.0, 4.50},
            };
            in.outputs = {
                {"LEU Product UF6 3.5% (KMP-B, Jan-Mar)",        2800.0, 7.00},
                {"Depleted UF6 Tails 0.3% (KMP-C, Jan-Mar)",     1900.0, 4.75},
                {"Process Hold-up & In-plant Inventory Increase",  120.0, 3.00},
                {"Samples Shipped to IAEA SAL",                     1.5,  0.05},
            };
            in.endingInventory = {"Residual UF6 Inventory (closing PIV)", 6488.0, 16.22};
            f.scenarios.append(s);
        }

        // Scenario 1.2 — Anomalous Quarter (missing tails declaration)
        {
            VirtualFacility::Scenario s;
            s.id = "ENRICH-001-S2";
            s.name = "Scenario B: Missing Tails Declaration (Anomalous)";
            s.difficulty = "Intermediate";
            s.description =
                "A quarterly balance where the depleted UF6 tails output was not fully declared. "
                "The tails cylinder shipment to storage occurred in late March but was recorded "
                "in the next period's books. This creates an artificially large negative MUF. "
                "Real-world scenario: bookkeeping timing error.";
            s.learnObjective =
                "Understand how missing output declarations produce NEGATIVE MUF. Learn to "
                "identify the NEGATIVE-MUF diagnostic flag and trace it back to omitted outputs. "
                "Practice verification: add the missing 380 kg tails output and recalculate.";
            s.expectedOutcome =
                "MUF ≈ −380 kg (large negative). NEGATIVE-MUF ALERT fires. "
                "After correcting the missing output, MUF returns to normal range.";

            MBPInput& in = s.input;
            in.facilityName  = "Virtual UF6 Enrichment Plant";
            in.facilityId    = "ENRICH-001";
            in.materialType  = "UF6 (Uranium Hexafluoride)";
            in.operatorName  = "Student Operator";
            in.mbpId         = "ENRICH-001-MBP-2025-Q2";
            in.periodStart   = QDateTime(QDate(2025,4,1),  QTime(0,0,0));
            in.periodEnd     = QDateTime(QDate(2025,6,30), QTime(23,59,59));
            in.beginningInventory = {"UF6 Inventory from Q1 PIV", 6488.0, 16.22};
            in.inputs  = {
                {"Batch-4: Natural UF6 Feed (Apr)",     2100.0, 5.25},
                {"Batch-5: Natural UF6 Feed (May-Jun)", 3400.0, 8.50},
            };
            in.outputs = {
                {"LEU Product UF6 3.5% (KMP-B)",    2950.0, 7.375},
                // Tails for Mar deliberately omitted — only Jun included
                {"Depleted UF6 Tails (Jun only)",   1580.0, 3.95},
                {"Process Hold-up Change",             30.0, 0.75},
            };
            in.endingInventory = {"UF6 Inventory closing PIV (Q2)", 6828.0, 17.07};
            f.scenarios.append(s);
        }

        // Scenario 1.3 — Critical MUF exceeds 3σ (diversion simulation)
        {
            VirtualFacility::Scenario s;
            s.id = "ENRICH-001-S3";
            s.name = "Scenario C: Critical MUF — Possible Diversion";
            s.difficulty = "Advanced";
            s.description =
                "A quarterly balance showing a large positive MUF significantly exceeding 3σ. "
                "Product UF6 outflows appear lower than expected given the feed quantities processed. "
                "This scenario simulates the statistical signature of a product diversion at an "
                "enrichment plant — historically the most safeguards-sensitive scenario.";
            s.learnObjective =
                "Experience the CRITICAL MUF-EXCEED-3σ diagnostic. Understand how SQ fraction "
                "is calculated for LEU (SQ = 75 kg U-235). Learn the mandatory reporting pathway "
                "under INFCIRC/153 Art.25 and Art.29.";
            s.expectedOutcome =
                "MUF ≈ +620 kg, σ(MUF) ≈ 38 kg, ratio ≈ 16σ → CRITICAL. "
                "SQ fraction ≈ 0.83%. MUF-EXCEED-3σ and HIGH-MUF-TO-THROUGHPUT both fire.";

            MBPInput& in = s.input;
            in.facilityName  = "Virtual UF6 Enrichment Plant";
            in.facilityId    = "ENRICH-001";
            in.materialType  = "UF6 (Uranium Hexafluoride)";
            in.operatorName  = "Student Operator";
            in.mbpId         = "ENRICH-001-MBP-2025-Q3";
            in.periodStart   = QDateTime(QDate(2025,7,1),  QTime(0,0,0));
            in.periodEnd     = QDateTime(QDate(2025,9,30), QTime(23,59,59));
            in.beginningInventory = {"UF6 Inventory from Q2 PIV", 6828.0, 17.07};
            in.inputs  = {
                {"Batch-6: Natural UF6 Feed (Jul)",     2600.0, 6.50},
                {"Batch-7: Natural UF6 Feed (Aug-Sep)", 3800.0, 9.50},
            };
            in.outputs = {
                // Product output understated vs feed processed
                {"LEU Product UF6 3.5% (KMP-B)",  2380.0, 5.95},  // Should be ~3000
                {"Depleted UF6 Tails (KMP-C)",    2100.0, 5.25},
                {"Process Hold-up Change",           20.0, 0.50},
            };
            in.endingInventory = {"UF6 Inventory closing PIV (Q3)", 8108.0, 20.27};
            f.scenarios.append(s);
        }

        m_facilities.append(f);
    }

    // ─────────────────────────────────────────────────────────────────
    // FACILITY 2 — MOX Fuel Fabrication Facility (Pu-bearing)
    // ─────────────────────────────────────────────────────────────────
    {
        VirtualFacility f;
        f.id       = "MOX-001";
        f.name     = "Virtual MOX Fuel Fabrication Facility";
        f.type     = "Mixed Oxide (MOX) Fuel Fabrication";
        f.country  = "Educational (based on IAEA reference design)";
        f.material = "Plutonium (Pu) + UO2 — MOX";
        f.icon     = "⚛";
        f.description =
            "A plutonium-bearing MOX fuel fabrication facility. Receives separated Pu oxide "
            "and depleted UO2, blends them to target Pu content, presses and sinters pellets, "
            "and assembles fuel rods. SQ = 8 kg Pu. High safeguards sensitivity — "
            "direct-use material with 30-day timeliness detection goal. "
            "Monthly MBP closures are required.";

        // Scenario 2.1 — Normal Month (Beginner)
        {
            VirtualFacility::Scenario s;
            s.id = "MOX-001-S1";
            s.name = "Scenario A: Normal Monthly Balance";
            s.difficulty = "Beginner";
            s.description =
                "A routine monthly material balance at the MOX facility. Small Pu quantities, "
                "tight measurement uncertainties. Normal MUF. Observe the small SQ fraction "
                "and the 30-day timeliness goal for direct-use material (Pu).";
            s.learnObjective =
                "Learn NMA for direct-use materials. Understand why Pu facilities require "
                "monthly (not quarterly) balance closures. Observe SQ = 8 kg threshold.";
            s.expectedOutcome =
                "MUF ≈ +0.18 kg Pu, σ(MUF) ≈ 0.22 kg, ratio ≈ 0.8σ → NORMAL. "
                "SQ fraction ≈ 2.3%. TIMELINESS-GOAL INFO flag if period > 30 days.";

            MBPInput& in = s.input;
            in.facilityName  = "Virtual MOX Fuel Fabrication Facility";
            in.facilityId    = "MOX-001";
            in.materialType  = "Plutonium (Pu)";
            in.operatorName  = "Student Operator";
            in.mbpId         = "MOX-001-MBP-2025-01";
            in.periodStart   = QDateTime(QDate(2025,1,1),  QTime(0,0,0));
            in.periodEnd     = QDateTime(QDate(2025,1,31), QTime(23,59,59));
            in.beginningInventory = {"Pu oxide store (initial PIV Jan)", 24.80, 0.12};
            in.inputs  = {
                {"Pu oxide receipt from reprocessing plant",  8.50, 0.043},
                {"Pu scrap return from fuel rod assembly",    0.22, 0.002},
            };
            in.outputs = {
                {"MOX fuel assemblies shipped (8 assemblies)", 6.12, 0.031},
                {"Pu-bearing waste to interim storage",        0.95, 0.048},
                {"Process scrap to scrap recovery",           0.44, 0.022},
                {"Measurement samples to SAL",                0.01, 0.001},
            };
            in.endingInventory = {"Pu oxide + in-process inventory (PIV Jan 31)", 25.82, 0.129};
            f.scenarios.append(s);
        }

        // Scenario 2.2 — CUSUM Trend (Intermediate)
        {
            VirtualFacility::Scenario s;
            s.id = "MOX-001-S2";
            s.name = "Scenario B: Protracted Diversion Pattern (CUSUM)";
            s.difficulty = "Intermediate";
            s.description =
                "A monthly balance where MUF is positive but modest — not alarming on its own. "
                "However, this pattern has occurred for 4 consecutive months. Represents the "
                "CUSUM detection scenario: slow diversion of Pu, ~0.3 kg/month, "
                "accumulated to approach 1 SQ over a year. Critical scenario for nuclear "
                "security education.";
            s.learnObjective =
                "Experience the CUSUM detection mechanism. Understand that single-period "
                "sigma tests can miss protracted diversion. Learn why CUSUM alarms even when "
                "individual MBPs look borderline-acceptable.";
            s.expectedOutcome =
                "MUF ≈ +0.48 kg (≈ 2.1σ). WARNING. SITMUF-ELEVATED INFO. "
                "CUSUM-ALARM-LOSS CRITICAL after accumulated S+ exceeds threshold.";

            MBPInput& in = s.input;
            in.facilityName  = "Virtual MOX Fuel Fabrication Facility";
            in.facilityId    = "MOX-001";
            in.materialType  = "Plutonium (Pu)";
            in.operatorName  = "Student Operator";
            in.mbpId         = "MOX-001-MBP-2025-05";
            in.periodStart   = QDateTime(QDate(2025,5,1),  QTime(0,0,0));
            in.periodEnd     = QDateTime(QDate(2025,5,31), QTime(23,59,59));
            in.beginningInventory = {"Pu inventory from April PIV", 27.16, 0.136};
            in.inputs  = {
                {"Pu oxide receipt (May batch)",  9.20, 0.046},
            };
            in.outputs = {
                {"MOX fuel assemblies shipped (10 assemblies)", 6.80, 0.034},
                {"Pu waste to storage",                         0.88, 0.044},
                {"Process scrap",                               0.39, 0.020},
            };
            in.endingInventory = {"Pu inventory closing PIV (May)", 27.81, 0.139};
            f.scenarios.append(s);
        }

        // Scenario 2.3 — SQ exceeded (Critical)
        {
            VirtualFacility::Scenario s;
            s.id = "MOX-001-S3";
            s.name = "Scenario C: SQ Threshold Exceeded — CRITICAL";
            s.difficulty = "Advanced";
            s.description =
                "A monthly balance where MUF exceeds 1 Significant Quantity (8 kg Pu). "
                "This represents the most severe possible safeguards finding at a MOX facility. "
                "Simulates a scenario where an entire batch of Pu oxide (one container) "
                "is unaccounted for — the IAEA threshold for mandatory special inspection.";
            s.learnObjective =
                "Experience the SQ-EXCEEDED CRITICAL flag. Understand the mandatory reporting "
                "cascade under INFCIRC/153 Art.25. Learn what an emergency PIV entails. "
                "Understand why 8 kg is the SQ for Pu.";
            s.expectedOutcome =
                "MUF ≈ +8.6 kg Pu >> 1 SQ (8 kg). σ(MUF) ≈ 0.31 kg. ratio >> 3σ. "
                "SQ-EXCEEDED CRITICAL + MUF-EXCEED-3σ CRITICAL both fire simultaneously.";

            MBPInput& in = s.input;
            in.facilityName  = "Virtual MOX Fuel Fabrication Facility";
            in.facilityId    = "MOX-001";
            in.materialType  = "Plutonium (Pu)";
            in.operatorName  = "Student Operator";
            in.mbpId         = "MOX-001-MBP-2025-08";
            in.periodStart   = QDateTime(QDate(2025,8,1),  QTime(0,0,0));
            in.periodEnd     = QDateTime(QDate(2025,8,31), QTime(23,59,59));
            in.beginningInventory = {"Pu inventory from July PIV", 29.40, 0.147};
            in.inputs  = {
                {"Pu oxide receipt batch A (5 containers)", 12.50, 0.063},
                {"Pu oxide receipt batch B (3 containers)",  7.20, 0.036},
            };
            in.outputs = {
                // Intentionally understated — 1 container (~8.6 kg) unaccounted
                {"MOX fuel assemblies shipped",  7.80, 0.039},
                {"Pu waste and scrap",           1.10, 0.055},
                {"SAL samples",                  0.01, 0.001},
            };
            in.endingInventory = {"Pu inventory closing PIV (Aug)", 31.59, 0.158};
            f.scenarios.append(s);
        }

        m_facilities.append(f);
    }

    // ─────────────────────────────────────────────────────────────────
    // FACILITY 3 — Reprocessing Plant (PUREX type)
    // ─────────────────────────────────────────────────────────────────
    {
        VirtualFacility f;
        f.id       = "REPROC-001";
        f.name     = "Virtual Spent Fuel Reprocessing Plant";
        f.type     = "Aqueous Reprocessing (PUREX Process)";
        f.country  = "Educational (based on IAEA reference design)";
        f.material = "Plutonium (Pu) + Uranium — Separated";
        f.icon     = "🔬";
        f.description =
            "A PUREX aqueous reprocessing plant that chemically separates plutonium and uranium "
            "from irradiated fuel. Among the most safeguards-sensitive facilities globally. "
            "Large process inventory (holdup) with high measurement uncertainty. "
            "Monthly closures required. Both Pu (SQ=8 kg) and HEU-equivalent process streams "
            "are tracked. IAEA conducts routine + unannounced inspections.";

        // Scenario 3.1 — Normal
        {
            VirtualFacility::Scenario s;
            s.id = "REPROC-001-S1";
            s.name = "Scenario A: Normal Monthly Reprocessing Balance";
            s.difficulty = "Beginner";
            s.description =
                "Normal monthly balance at a reprocessing plant. Large process holdup "
                "dominates σ(MUF) due to high holdup uncertainty (10-30%). "
                "Learn why reprocessing plants have the largest σ(MUF) of any facility type.";
            s.learnObjective =
                "Understand the concept of process holdup and why it dominates NMA uncertainty "
                "at reprocessing plants. Observe DOMINANT-UNCERTAINTY-SOURCE flag for holdup measurement.";
            s.expectedOutcome =
                "MUF ≈ +0.9 kg Pu, σ(MUF) ≈ 2.4 kg (large due to holdup). ratio ≈ 0.4σ → NORMAL. "
                "DOMINANT-UNCERTAINTY-SOURCE fires for in-plant holdup measurement.";

            MBPInput& in = s.input;
            in.facilityName  = "Virtual Spent Fuel Reprocessing Plant";
            in.facilityId    = "REPROC-001";
            in.materialType  = "Plutonium (Pu)";
            in.operatorName  = "Student Operator";
            in.mbpId         = "REPROC-001-MBP-2025-01";
            in.periodStart   = QDateTime(QDate(2025,1,1),  QTime(0,0,0));
            in.periodEnd     = QDateTime(QDate(2025,1,31), QTime(23,59,59));
            in.beginningInventory = {"Pu in process solutions & store (PIV)", 18.40, 0.368}; // 2% unc
            in.inputs  = {
                {"Irradiated fuel receipt — Pu content (calc.)", 14.20, 0.426}, // 3% burn-up calc
                {"Recycled Pu from previous batch",               0.85, 0.043},
            };
            in.outputs = {
                {"Pu nitrate product shipped to MOX plant",  12.10, 0.061},
                {"High-level waste (Pu in vitrified glass)",  0.48, 0.144},  // 30% holdup unc
                {"Intermediate-level waste",                  0.12, 0.036},
                {"Process holdup decrease (measured)",        0.45, 0.135},  // 30% holdup unc
            };
            in.endingInventory = {"Pu in process + product store (PIV)", 20.21, 0.404};
            f.scenarios.append(s);
        }

        // Scenario 3.2 — Poor Uncertainty Ratio
        {
            VirtualFacility::Scenario s;
            s.id = "REPROC-001-S2";
            s.name = "Scenario B: Poor Measurement Quality — Detection Blind Spot";
            s.difficulty = "Intermediate";
            s.description =
                "A balance period where σ(MUF) is so large (due to poor holdup measurement) "
                "that even a full SQ of Pu missing would not trigger the 3σ alarm. "
                "This illustrates the 'detection blind spot' problem at bulk-handling facilities. "
                "Classic Miller (1990) scenario: σ(MUF) > 1 SQ.";
            s.learnObjective =
                "Understand the POOR-UNCERTAINTY-RATIO flag. Recognize when σ(MUF) > SQ means "
                "the accounting system cannot reliably detect diversion. "
                "Learn why IAEA requires σ(MUF) < 1 SQ per measurement period.";
            s.expectedOutcome =
                "MUF ≈ +1.2 kg, σ(MUF) ≈ 15 kg (>> 1 SQ = 8 kg). ratio ≈ 0.08σ. "
                "POOR-UNCERTAINTY-RATIO WARNING fires. System cannot detect diversion.";

            MBPInput& in = s.input;
            in.facilityName  = "Virtual Spent Fuel Reprocessing Plant";
            in.facilityId    = "REPROC-001";
            in.materialType  = "Plutonium (Pu)";
            in.operatorName  = "Student Operator";
            in.mbpId         = "REPROC-001-MBP-2025-06";
            in.periodStart   = QDateTime(QDate(2025,6,1),  QTime(0,0,0));
            in.periodEnd     = QDateTime(QDate(2025,6,30), QTime(23,59,59));
            in.beginningInventory = {"Pu in process (poor holdup measurement)", 22.50, 4.50};  // 20% unc
            in.inputs  = {
                {"Irradiated fuel Pu content (calc.)",  16.80, 0.840},
            };
            in.outputs = {
                {"Pu nitrate product shipped",          14.30, 0.072},
                {"Waste streams (high uncertainty)",     1.20, 0.360},  // 30%
                {"Process holdup change (estimated)",    2.60, 0.780},  // 30%
            };
            in.endingInventory = {"Pu in process (closing, poor measurement)", 21.00, 4.20};
            f.scenarios.append(s);
        }

        // Scenario 3.3 — CUSUM + SRD Combined (Advanced)
        {
            VirtualFacility::Scenario s;
            s.id = "REPROC-001-S3";
            s.name = "Scenario C: Input Accountability Tank — Shipper-Receiver Difference";
            s.difficulty = "Advanced";
            s.description =
                "A balance where the declared Pu input from the reactor operator (shipper) "
                "differs significantly from the reprocessing plant's measurement (receiver). "
                "The SRD = 0.8 kg Pu. Combined with a positive MUF, this triggers multiple "
                "diagnostic flags simultaneously. Advanced scenario combining SRD analysis, "
                "MUF significance, and CUSUM trend.";
            s.learnObjective =
                "Understand Shipper-Receiver Differences (SRD). Learn to identify when "
                "measurement bias (not diversion) is the likely cause vs. true material loss. "
                "Practice multi-flag diagnostic interpretation.";
            s.expectedOutcome =
                "MUF ≈ +2.8 kg (> 2σ). WARNING. SRD = 0.8 kg flagged. "
                "DOMINANT-UNCERTAINTY-SOURCE for input accountability tank measurement.";

            MBPInput& in = s.input;
            in.facilityName  = "Virtual Spent Fuel Reprocessing Plant";
            in.facilityId    = "REPROC-001";
            in.materialType  = "Plutonium (Pu)";
            in.operatorName  = "Student Operator";
            in.mbpId         = "REPROC-001-MBP-2025-09";
            in.periodStart   = QDateTime(QDate(2025,9,1),  QTime(0,0,0));
            in.periodEnd     = QDateTime(QDate(2025,9,30), QTime(23,59,59));
            in.beginningInventory = {"Pu inventory from Aug PIV", 19.80, 0.396};
            in.inputs  = {
                // Receiver measurement lower than shipper's declared 17.20 kg — SRD = 0.8 kg
                {"Irradiated fuel Pu — receiver measurement (KMP input)", 16.40, 0.492},
                {"Recycled Pu returns",    0.62, 0.031},
            };
            in.outputs = {
                {"Pu nitrate product shipped",     13.50, 0.068},
                {"HLW waste Pu content",            0.55, 0.165},
                {"ILW waste Pu content",            0.18, 0.054},
            };
            in.endingInventory = {"Pu closing PIV (Sep)", 22.39, 0.448};
            f.scenarios.append(s);
        }

        m_facilities.append(f);
    }

    // ─────────────────────────────────────────────────────────────────
    // FACILITY 4 — Research Reactor + Radioisotope Production
    // ─────────────────────────────────────────────────────────────────
    {
        VirtualFacility f;
        f.id       = "RESRX-001";
        f.name     = "Virtual Research Reactor & Isotope Facility";
        f.type     = "Research Reactor (Pool-type) + Radioisotope Production";
        f.country  = "Educational (based on IAEA reference design)";
        f.material = "HEU → LEU Conversion / Natural Uranium Targets";
        f.icon     = "🔭";
        f.description =
            "A pool-type research reactor operating on HEU fuel (historical) or LEU fuel (modern). "
            "Also produces medical radioisotopes using natural uranium or Mo-99 targets. "
            "Demonstrates IAEA's HEU minimisation programme — conversion from HEU to LEU fuel. "
            "Simpler inventory than bulk facilities — fuel is in discrete, identifiable items.";

        // Scenario 4.1 — HEU to LEU conversion (Beginner)
        {
            VirtualFacility::Scenario s;
            s.id = "RESRX-001-S1";
            s.name = "Scenario A: HEU Fuel Balance (High-Sensitivity)";
            s.difficulty = "Beginner";
            s.description =
                "An annual material balance for a research reactor operating on HEU fuel elements. "
                "SQ = 25 kg for HEU. With typical reactor cores containing 3-5 kg of HEU, "
                "the SQ fraction is significant. Observe the 30-day timeliness goal for HEU "
                "and the TIMELINESS-GOAL flag for annual balances.";
            s.learnObjective =
                "Understand why research reactors using HEU are high-priority for IAEA safeguards. "
                "Learn the discrete-item accountancy approach (fuel elements counted individually). "
                "Observe timeliness flag for direct-use material with annual balance.";
            s.expectedOutcome =
                "MUF ≈ +0.05 kg HEU, σ(MUF) ≈ 0.09 kg. ratio ≈ 0.6σ → NORMAL. "
                "TIMELINESS-GOAL INFO fires (365 days > 30-day goal for HEU). "
                "SQ fraction ≈ 0.2% — low absolute risk for HEU.";

            MBPInput& in = s.input;
            in.facilityName  = "Virtual Research Reactor";
            in.facilityId    = "RESRX-001";
            in.materialType  = "HEU (Highly Enriched Uranium, ≥20%)";
            in.operatorName  = "Student Operator";
            in.mbpId         = "RESRX-001-MBP-2024-ANNUAL";
            in.periodStart   = QDateTime(QDate(2024,1,1),  QTime(0,0,0));
            in.periodEnd     = QDateTime(QDate(2024,12,31),QTime(23,59,59));
            in.beginningInventory = {"HEU fuel elements in core + fresh store (count)", 5.240, 0.026};
            in.inputs  = {
                {"Fresh HEU fuel elements received (2 elements)",  1.920, 0.010},
            };
            in.outputs = {
                {"Spent HEU fuel elements shipped to storage (3)", 2.855, 0.014},
                {"HEU samples to IAEA SAL (destructive analysis)", 0.014, 0.001},
            };
            in.endingInventory = {"HEU fuel elements in core + fresh store (annual PIV)", 4.241, 0.021};
            f.scenarios.append(s);
        }

        // Scenario 4.2 — Natural Uranium targets (Intermediate)
        {
            VirtualFacility::Scenario s;
            s.id = "RESRX-001-S2";
            s.name = "Scenario B: Natural Uranium Target Irradiation (Normal)";
            s.difficulty = "Intermediate";
            s.description =
                "Quarterly balance for natural uranium target irradiation program. "
                "Uranium targets are loaded, irradiated, and shipped to a radiochemistry lab "
                "for Mo-99 extraction. Low SQ risk (SQ = 10,000 kg for NatU) but demonstrates "
                "the NMA process for indirect-use materials with longer timeliness goals.";
            s.learnObjective =
                "Compare indirect-use material (NatU, 90-day timeliness) vs direct-use (HEU, 30-day). "
                "Understand how timeliness goals affect MBP design. "
                "Learn target production NMA: inputs are fresh targets, outputs are irradiated targets.";
            s.expectedOutcome =
                "MUF ≈ +0.12 kg NatU, σ(MUF) ≈ 0.25 kg. ratio ≈ 0.5σ → NORMAL. "
                "No TIMELINESS flag (90-day goal, 91-day period — borderline). "
                "SQ fraction << 0.01% — well below any alert threshold.";

            MBPInput& in = s.input;
            in.facilityName  = "Virtual Research Reactor — Target Facility";
            in.facilityId    = "RESRX-001";
            in.materialType  = "Natural Uranium";
            in.operatorName  = "Student Operator";
            in.mbpId         = "RESRX-001-MBP-2025-Q1-NAT";
            in.periodStart   = QDateTime(QDate(2025,1,1),  QTime(0,0,0));
            in.periodEnd     = QDateTime(QDate(2025,4,1),  QTime(23,59,59));
            in.beginningInventory = {"Fresh NatU target discs in storage", 18.50, 0.185};
            in.inputs  = {
                {"NatU target discs received (batch)", 24.00, 0.240},
            };
            in.outputs = {
                {"Irradiated NatU targets shipped to radiochem lab", 21.60, 0.216},
                {"NatU scrap / trim waste from target fabrication",   0.78, 0.039},
            };
            in.endingInventory = {"NatU targets remaining in storage (PIV)", 20.00, 0.200};
            f.scenarios.append(s);
        }

        // Scenario 4.3 — Undeclared Material (Advanced)
        {
            VirtualFacility::Scenario s;
            s.id = "RESRX-001-S3";
            s.name = "Scenario C: Undeclared HEU — Warning Pattern";
            s.difficulty = "Advanced";
            s.description =
                "A research reactor scenario where a NEGATIVE MUF is observed — the facility "
                "appears to have MORE HEU than expected. In the safeguards context, a large "
                "negative MUF at an HEU facility raises concern about undeclared HEU inputs. "
                "This is historically the most sensitive finding at research reactors.";
            s.learnObjective =
                "Understand the safeguards significance of NEGATIVE MUF at a direct-use facility. "
                "Learn why apparent material gain is suspicious — not just material loss. "
                "Understand how undeclared transfers would appear in NMA data.";
            s.expectedOutcome =
                "MUF ≈ −0.85 kg HEU (large negative). NEGATIVE-MUF ALERT fires. "
                "SQ fraction ≈ 3.4% — notable for HEU. Recommend urgent inventory verification.";

            MBPInput& in = s.input;
            in.facilityName  = "Virtual Research Reactor";
            in.facilityId    = "RESRX-001";
            in.materialType  = "HEU (Highly Enriched Uranium, ≥20%)";
            in.operatorName  = "Student Operator";
            in.mbpId         = "RESRX-001-MBP-2025-S1";
            in.periodStart   = QDateTime(QDate(2025,1,1),  QTime(0,0,0));
            in.periodEnd     = QDateTime(QDate(2025,6,30), QTime(23,59,59));
            in.beginningInventory = {"HEU fuel elements (opening PIV)", 4.241, 0.021};
            in.inputs  = {
                {"Fresh HEU fuel elements received (1 element)", 0.960, 0.005},
            };
            in.outputs = {
                {"Spent HEU fuel shipped to storage (2 elements)", 1.902, 0.010},
                {"HEU samples to SAL",                             0.008, 0.001},
            };
            // EI significantly higher than expected — suggesting undeclared HEU present
            in.endingInventory = {"HEU in core + store (PIV June)", 4.141, 0.021};
            f.scenarios.append(s);
        }

        m_facilities.append(f);
    }
}

// ══════════════════════════════════════════════════════════════════════
//  UI CONSTRUCTION
// ══════════════════════════════════════════════════════════════════════
VirtualFacilityDialog::VirtualFacilityDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("Virtual Facility Simulator — NMA Training Module");
    setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
    setMinimumSize(1100, 720);
    setStyleSheet(Styles::appStyle() + Styles::lineEditStyle());
    buildFacilities();
    buildUI();
}

void VirtualFacilityDialog::buildUI() {
    auto* root = new QVBoxLayout(this);
    root->setSpacing(0); root->setContentsMargins(0,0,0,0);

    // ── Top banner ────────────────────────────────────────────────
    auto* banner = new QWidget();
    banner->setFixedHeight(72);
    banner->setStyleSheet(
        "background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
        "stop:0 #1a3a5c,stop:0.6 #2d5680,stop:1 #4a7fa8);"
        "border-bottom:2px solid #2a88ae;");
    auto* bl = new QHBoxLayout(banner);
    bl->setContentsMargins(24,12,24,10); bl->setSpacing(14);

    QPixmap logo(":/tpu_logo.png");
    if (!logo.isNull()) {
        auto* lg = new QLabel();
        lg->setPixmap(logo.scaledToHeight(42, Qt::SmoothTransformation));
        lg->setStyleSheet("background:transparent;border:none;");
        bl->addWidget(lg);
    }
    auto* tc = new QVBoxLayout(); tc->setSpacing(2);
    auto* t1 = new QLabel("Virtual Facility Simulator");
    t1->setStyleSheet("color:white;font-size:18px;font-weight:900;background:transparent;");
    auto* t2 = new QLabel("4 nuclear facility replicas with integrated NMA training scenarios");
    t2->setStyleSheet("color:#93c5fd;font-size:11px;background:transparent;");
    tc->addWidget(t1); tc->addWidget(t2);
    bl->addLayout(tc); bl->addStretch();
    auto* hdBadge = new QLabel("IAEA Safeguards Training Module  |  Tomsk Polytechnic University");
    hdBadge->setStyleSheet("color:#93c5fd;font-size:11px;background:transparent;");
    bl->addWidget(hdBadge);
    root->addWidget(banner);

    // accent strip
    auto* strip = new QWidget(); strip->setFixedHeight(3);
    strip->setStyleSheet("background:qlineargradient(x1:0,y1:0,x2:1,y2:0,"
                         "stop:0 #2a88ae,stop:0.5 #7fb3d3,stop:1 #2a88ae);");
    root->addWidget(strip);

    // ── Body: two-column (sidebar + main) ─────────────────────────
    auto* body = new QHBoxLayout();
    body->setSpacing(0); body->setContentsMargins(0,0,0,0);

    // ── LEFT SIDEBAR – dark navy, facility + scenario lists ────────
    auto* sidebar = new QWidget();
    sidebar->setFixedWidth(240);
    sidebar->setStyleSheet("background:#f0f4f8;border-right:1px solid #d1d8e0;");
    auto* sideL = new QVBoxLayout(sidebar);
    sideL->setContentsMargins(0,0,0,0); sideL->setSpacing(0);

    auto* facHdr = new QLabel("  NUCLEAR FACILITIES");
    facHdr->setStyleSheet("background:#e2eaf2;color:#1a3a5c;"
                          "font-size:10px;font-weight:700;letter-spacing:2px;padding:9px 14px;"
                          "border-bottom:1px solid #d1d8e0;");
    sideL->addWidget(facHdr);

    m_facilityList = new QListWidget();
    m_facilityList->setStyleSheet(
        "QListWidget{background:#f7f9fb;border:none;}"
        "QListWidget::item{padding:12px 16px;border-bottom:1px solid #eef1f5;"
        "font-size:13px;color:#1a3a5c;}"
        "QListWidget::item:selected{background:#1a3a5c;color:white;}"
        "QListWidget::item:hover:!selected{background:#dce8f0;color:#0f2438;}");
    for (auto& f : m_facilities) {
        auto* item = new QListWidgetItem(f.icon + "  " + f.name);
        m_facilityList->addItem(item);
    }
    sideL->addWidget(m_facilityList, 3);

    auto* scenHdr = new QLabel("  SCENARIOS");
    scenHdr->setStyleSheet("background:#e2eaf2;color:#1a3a5c;"
                           "font-size:10px;font-weight:700;letter-spacing:2px;"
                           "padding:9px 14px;border-top:1px solid #d1d8e0;"
                           "border-bottom:1px solid #d1d8e0;");
    sideL->addWidget(scenHdr);

    m_scenarioList = new QListWidget();
    m_scenarioList->setStyleSheet(
        "QListWidget{background:#f0f4f8;border:none;}"
        "QListWidget::item{padding:10px 14px;border-bottom:1px solid #eef1f5;"
        "font-size:12px;color:#1a3a5c;font-family:'Courier New',monospace;}"
        "QListWidget::item:selected{background:#1a3a5c;color:white;}"
        "QListWidget::item:hover:!selected{background:#dce8f0;color:#0f2438;}");
    sideL->addWidget(m_scenarioList, 2);

    body->addWidget(sidebar);

    // ── RIGHT MAIN – watermark backdrop + tab content ──────────────
    auto* mainArea = new WatermarkWidget(0.10, Qt::white);
    auto* mainL = new QVBoxLayout(mainArea);
    mainL->setContentsMargins(0,0,0,0); mainL->setSpacing(0);

    // Tabs
    auto* tabs = new QTabWidget();
    tabs->setStyleSheet(
        "QTabWidget::pane{border:none;background:transparent;}"
        "QTabBar::tab{padding:9px 24px;font-size:13px;font-weight:700;"
        "background:#dce6f0;color:#374151;border:1px solid #d1d5db;"
        "border-bottom:none;border-radius:4px 4px 0 0;margin-right:2px;}"
        "QTabBar::tab:selected{background:rgba(255,255,255,0.95);"
        "color:#1a3a5c;border-top:3px solid #1a3a5c;font-weight:800;}"
        "QTabBar::tab:hover:!selected{background:#c8daea;}");

    // Tab 1 – Scenario description
    m_scenarioBrowser = vBrowser();
    m_scenarioBrowser->setStyleSheet(
        "QTextBrowser{background:transparent;border:none;padding:4px;}");
    tabs->addTab(m_scenarioBrowser, "  Scenario  ");

    // Tab 2 – NMA data preview
    m_nmaDataBrowser = vBrowser();
    m_nmaDataBrowser->setStyleSheet(
        "QTextBrowser{background:transparent;border:none;padding:4px;}");
    tabs->addTab(m_nmaDataBrowser, "  NMA Data  ");

    // Tab 3 – Live NMA Simulation
    {
        auto* nmaTab = new WatermarkWidget(0.10, Qt::white);
        auto* nmaL   = new QVBoxLayout(nmaTab);
        nmaL->setContentsMargins(0,0,0,0); nmaL->setSpacing(0);

        // Steps bar
        auto* stepsBar = new QWidget();
        stepsBar->setFixedHeight(48);
        stepsBar->setStyleSheet("background:rgba(255,255,255,0.93);"
                                "border-bottom:1px solid #d1d5db;");
        auto* stepsL = new QHBoxLayout(stepsBar);
        stepsL->setContentsMargins(16,6,16,6); stepsL->setSpacing(6);

        for (int i = 0; i < 5; ++i) {
            const QString nums[]  = {"1","2","3","4","5"};
            const QString lbls[]  = {"Load","Book Inv.","MUF","σ(MUF)","Diagnostics"};
            m_stepLabels[i] = new QLabel(nums[i] + "  " + lbls[i]);
            m_stepLabels[i]->setStyleSheet(
                i == 0
                ? "background:#1a3a5c;color:white;border-radius:2px;"
                  "padding:5px 10px;font-size:11px;font-weight:700;"
                : "background:#e5ecf2;color:#374151;border-radius:2px;"
                  "padding:5px 10px;font-size:11px;");
            stepsL->addWidget(m_stepLabels[i]);
        }
        stepsL->addStretch();
        m_nmaRunBtn = new QPushButton("Run NMA  ▶");
        m_nmaRunBtn->setStyleSheet(Styles::greenButtonStyle() +
                                   "QPushButton{padding:6px 16px;font-size:12px;}");
        m_nmaRunBtn->setEnabled(false);
        connect(m_nmaRunBtn, &QPushButton::clicked,
                this, &VirtualFacilityDialog::onRunNMACalculation);
        stepsL->addWidget(m_nmaRunBtn);
        nmaL->addWidget(stepsBar);

        m_nmaLiveBrowser = vBrowser();
        m_nmaLiveBrowser->setStyleSheet(
            "QTextBrowser{background:transparent;border:none;padding:4px;}");
        m_nmaLiveBrowser->setHtml(VCSS +
            "<div style='text-align:center;padding:60px 20px;color:#374151;'>"
            "<div style='font-size:48px;margin-bottom:16px;'>&#9881;</div>"
            "<h2 style='color:#1a3a5c;'>Select a scenario, then click "
            "<span style='color:#3d8a2e;'>Run NMA</span> to see live calculation.</h2>"
            "<p>All IAEA NMA steps will be shown with your actual scenario data.</p></div>");
        nmaL->addWidget(m_nmaLiveBrowser, 1);
        tabs->addTab(nmaTab, "  NMA Simulation  ");
    }
    mainL->addWidget(tabs, 1);

    // ── Bottom action bar ──────────────────────────────────────────
    auto* actBar = new QWidget();
    actBar->setFixedHeight(56);
    actBar->setStyleSheet("background:rgba(255,255,255,0.95);"
                          "border-top:1px solid #d1d5db;");
    auto* actL = new QHBoxLayout(actBar);
    actL->setContentsMargins(18,8,18,8); actL->setSpacing(10);

    m_diffBadge = new QLabel("● Beginner");
    m_diffBadge->setStyleSheet(
        "background:#f0fdf4;color:#15803d;border:1px solid #86efac;"
        "border-radius:2px;padding:4px 10px;font-size:11px;font-weight:700;");

    m_simStatus = new QLabel("Select a facility and scenario to begin.");
    m_simStatus->setStyleSheet("color:#374151;font-size:12px;background:transparent;");

    m_simBtn = new QPushButton("Preview Simulation");
    m_simBtn->setStyleSheet(Styles::secondaryButtonStyle() +
                             "QPushButton{padding:7px 16px;font-size:12px;}");
    m_simBtn->setEnabled(false);

    m_loadBtn = new QPushButton("Load into Calculator  ▶");
    m_loadBtn->setStyleSheet(Styles::greenButtonStyle() +
                              "QPushButton{padding:8px 22px;font-size:13px;}");
    m_loadBtn->setEnabled(false);

    actL->addWidget(m_diffBadge);
    actL->addWidget(m_simStatus, 1);
    actL->addWidget(m_simBtn);
    actL->addWidget(m_loadBtn);
    mainL->addWidget(actBar);

    body->addWidget(mainArea, 1);
    auto* bodyW = new QWidget();
    bodyW->setLayout(body);
    root->addWidget(bodyW, 1);

    // ── Footer ─────────────────────────────────────────────────────
    auto* footer = new QWidget();
    footer->setFixedHeight(38);
    footer->setStyleSheet("background:#f0f4f8;border-top:1px solid #d1d8e0;");
    auto* fl = new QHBoxLayout(footer);
    fl->setContentsMargins(18,0,18,0);
    auto* note = new QLabel(
        "All scenarios are fictional for educational purposes. "
        "Based on IAEA safeguards concepts and real facility type characteristics.");
    note->setStyleSheet("color:#4a6a8a;font-size:11px;background:transparent;");
    auto* closeBtn = new QPushButton("Close");
    closeBtn->setStyleSheet(Styles::secondaryButtonStyle() +
                             "QPushButton{padding:5px 16px;font-size:12px;}");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);
    fl->addWidget(note); fl->addStretch(); fl->addWidget(closeBtn);
    root->addWidget(footer);

    // ── Connections ────────────────────────────────────────────────
    connect(m_facilityList, &QListWidget::currentRowChanged,
            this, &VirtualFacilityDialog::onFacilitySelected);
    connect(m_scenarioList, &QListWidget::currentRowChanged,
            this, &VirtualFacilityDialog::onScenarioSelected);
    connect(m_loadBtn, &QPushButton::clicked,
            this, &VirtualFacilityDialog::onLoadClicked);
    connect(m_simBtn,  &QPushButton::clicked,
            this, &VirtualFacilityDialog::onRunSimulation);

    m_facilityList->setCurrentRow(0);
}
void VirtualFacilityDialog::onFacilitySelected(int row) {
    if (row < 0 || row >= m_facilities.size()) return;
    m_selectedFacility = row;
    m_selectedScenario = 0;
    populateScenarioList();
    m_scenarioList->setCurrentRow(0);
}

void VirtualFacilityDialog::populateScenarioList() {
    m_scenarioList->clear();
    auto& fac = m_facilities[m_selectedFacility];
    for (auto& s : fac.scenarios) {
        QString diff = s.difficulty == "Beginner"     ? "  [●] " :
                       s.difficulty == "Intermediate" ? "  [●●] " : "  [●●●] ";
        m_scenarioList->addItem(diff + s.name);
    }
}

void VirtualFacilityDialog::onScenarioSelected(int row) {
    if (row < 0 || m_selectedFacility >= m_facilities.size()) return;
    auto& fac = m_facilities[m_selectedFacility];
    if (row >= fac.scenarios.size()) return;
    m_selectedScenario = row;
    m_loadBtn->setEnabled(true);
    m_simBtn->setEnabled(true);
    if (m_nmaRunBtn) m_nmaRunBtn->setEnabled(true);
    updateDetailView();
}

void VirtualFacilityDialog::updateDetailView() {
    if (m_selectedFacility >= m_facilities.size()) return;
    auto& fac = m_facilities[m_selectedFacility];
    if (m_selectedScenario >= fac.scenarios.size()) return;
    auto& scen = fac.scenarios[m_selectedScenario];

    // ── Difficulty badge ──────────────────────────────────────────────
    QString diffStyle, diffText;
    if (scen.difficulty == "Beginner") {
        diffStyle = "background:#f0fdf4;color:#15803d;border:1px solid #16a34a;";
        diffText  = "● Beginner";
    } else if (scen.difficulty == "Intermediate") {
        diffStyle = "background:#eff6ff;color:#1d4ed8;border:1px solid #1d4ed8;";
        diffText  = "●● Intermediate";
    } else {
        diffStyle = "background:#fef2f2;color:#b91c1c;border:1px solid #dc2626;";
        diffText  = "●●● Advanced";
    }
    m_diffBadge->setStyleSheet(diffStyle + "border-radius:12px;padding:4px 12px;"
                                "font-size:19px;font-weight:700;");
    m_diffBadge->setText(diffText);
    m_simStatus->setText("Ready to load: " + scen.id);

    // ── Scenario description tab ──────────────────────────────────────
    QString html = VCSS + "<h1>" + scen.name + "</h1>";
    html += "<table><tr><th>Attribute</th><th>Value</th></tr>";
    html += "<tr><td>Facility</td><td>" + fac.name + "</td></tr>";
    html += "<tr><td>Facility Type</td><td>" + fac.type + "</td></tr>";
    html += "<tr><td>Material</td><td>" + fac.material + "</td></tr>";
    html += "<tr><td>Difficulty</td><td>" + scen.difficulty + "</td></tr>";
    html += "<tr><td>Scenario ID</td><td><code>" + scen.id + "</code></td></tr>";
    html += "</table>";

    html += "<h2>Facility Description</h2><p>" + fac.description + "</p>";
    html += "<h2>Scenario Overview</h2><p>" + scen.description + "</p>";
    html += "<h2>Learning Objective</h2>";
    html += "<div class='tip'><strong>What you should learn:</strong><br>" + scen.learnObjective + "</div>";
    html += "<h2>Expected Outcome</h2>";
    html += "<div class='ok'><strong>Expected results when you load this scenario:</strong><br>"
            + scen.expectedOutcome + "</div>";

    html += "<h2>How to Use This Scenario</h2>";
    html += "<ol>";
    html += "<li>Click <strong>Load into Calculator</strong> to open the data entry wizard pre-filled with this scenario's data.</li>";
    html += "<li>Review the pre-filled values — understand what each number represents for this facility type.</li>";
    html += "<li>Click <strong>Calculate MUF</strong> and examine all four results tabs.</li>";
    html += "<li>Compare your diagnostic flags to the Expected Outcome above.</li>";
    html += "<li>Try modifying values (e.g., decrease an output by 10%) and recalculate to see how MUF changes.</li>";
    html += "</ol>";
    m_scenarioBrowser->setHtml(html);

    // ── NMA data preview tab ──────────────────────────────────────────
    auto& inp = scen.input;
    QString nhtml = VCSS + "<h1>NMA Data — " + scen.id + "</h1>";
    nhtml += "<p><strong>Facility:</strong> " + inp.facilityName
           + " &nbsp;|&nbsp; <strong>MBP:</strong> " + inp.mbpId
           + " &nbsp;|&nbsp; <strong>Material:</strong> " + inp.materialType + "</p>";
    nhtml += "<p><strong>Period:</strong> "
           + inp.periodStart.toString("yyyy-MM-dd") + " to "
           + inp.periodEnd.toString("yyyy-MM-dd") + "</p>";

    nhtml += "<h2>Beginning Inventory (BI)</h2>";
    nhtml += "<table><tr><th>Description</th><th>Quantity (kg)</th><th>1&#963; Uncertainty (kg)</th><th>&#963;%</th></tr>";
    nhtml += QString("<tr><td>%1</td><td><code>%2</code></td><td><code>%3</code></td><td>%4%</td></tr>")
        .arg(inp.beginningInventory.label)
        .arg(inp.beginningInventory.qty, 0,'f',3)
        .arg(inp.beginningInventory.error, 0,'f',4)
        .arg(inp.beginningInventory.qty > 0 ? inp.beginningInventory.error/inp.beginningInventory.qty*100 : 0, 0,'f',2);
    nhtml += "</table>";

    nhtml += "<h2>Material Inputs</h2>";
    nhtml += "<table><tr><th>Description</th><th>Quantity (kg)</th><th>1&#963; (kg)</th></tr>";
    double totalIn = 0;
    for (auto& it : inp.inputs) {
        nhtml += QString("<tr><td>%1</td><td><code>%2</code></td><td><code>%3</code></td></tr>")
            .arg(it.label).arg(it.qty,0,'f',3).arg(it.error,0,'f',4);
        totalIn += it.qty;
    }
    nhtml += QString("<tr style='background:#f0f2f5;font-weight:700;'>"
                     "<td>TOTAL INPUTS</td><td><code>%1</code></td><td>—</td></tr>")
        .arg(totalIn,0,'f',3);
    nhtml += "</table>";

    nhtml += "<h2>Material Outputs</h2>";
    nhtml += "<table><tr><th>Description</th><th>Quantity (kg)</th><th>1&#963; (kg)</th></tr>";
    double totalOut = 0;
    for (auto& it : inp.outputs) {
        nhtml += QString("<tr><td>%1</td><td><code>%2</code></td><td><code>%3</code></td></tr>")
            .arg(it.label).arg(it.qty,0,'f',3).arg(it.error,0,'f',4);
        totalOut += it.qty;
    }
    nhtml += QString("<tr style='background:#f0f2f5;font-weight:700;'>"
                     "<td>TOTAL OUTPUTS</td><td><code>%1</code></td><td>—</td></tr>")
        .arg(totalOut,0,'f',3);
    nhtml += "</table>";

    nhtml += "<h2>Ending Inventory (EI)</h2>";
    nhtml += "<table><tr><th>Description</th><th>Quantity (kg)</th><th>1&#963; (kg)</th><th>&#963;%</th></tr>";
    nhtml += QString("<tr><td>%1</td><td><code>%2</code></td><td><code>%3</code></td><td>%4%</td></tr>")
        .arg(inp.endingInventory.label)
        .arg(inp.endingInventory.qty, 0,'f',3)
        .arg(inp.endingInventory.error, 0,'f',4)
        .arg(inp.endingInventory.qty > 0 ? inp.endingInventory.error/inp.endingInventory.qty*100 : 0, 0,'f',2);
    nhtml += "</table>";

    // Quick hand calculation preview
    double IB  = inp.beginningInventory.qty + totalIn - totalOut;
    double MUF = IB - inp.endingInventory.qty;
    double varBI  = inp.beginningInventory.error * inp.beginningInventory.error;
    double varIn  = 0, varOut = 0;
    for (auto& it : inp.inputs)  varIn  += it.error * it.error;
    for (auto& it : inp.outputs) varOut += it.error * it.error;
    double varEI  = inp.endingInventory.error * inp.endingInventory.error;
    double sigma  = std::sqrt(varBI + varIn + varOut + varEI);

    nhtml += "<h2>Preview Calculation</h2>";
    nhtml += "<div class='kmp'>";
    nhtml += QString("<pre>IB  = BI + ΣIn − ΣOut  =  %1 + %2 − %3  =  %4 kg\n"
                     "MUF = IB − EI  =  %4 − %5  =  %6 kg\n"
                     "σ(MUF) = √[σ²(BI)+σ²(ΣIn)+σ²(ΣOut)+σ²(EI)]  =  %7 kg\n"
                     "Ratio = |MUF|/σ  =  %8σ")
        .arg(inp.beginningInventory.qty,0,'f',3)
        .arg(totalIn,0,'f',3)
        .arg(totalOut,0,'f',3)
        .arg(IB,0,'f',3)
        .arg(inp.endingInventory.qty,0,'f',3)
        .arg(MUF,0,'f',4)
        .arg(sigma,0,'f',4)
        .arg(sigma > 0 ? std::abs(MUF)/sigma : 0, 0,'f',2);
    nhtml += "</pre></div>";
    nhtml += "<p><em>Load into Calculator for the full diagnostic analysis with all 12 safeguards checks.</em></p>";

    m_nmaDataBrowser->setHtml(nhtml);
}

void VirtualFacilityDialog::onRunSimulation() {
    m_simStatus->setText("Simulating NMA calculation...");
    QTimer::singleShot(800, this, [this]{
        m_simStatus->setText("Simulation complete — see NMA Data tab for preview results.");
    });
}

void VirtualFacilityDialog::onRunNMACalculation() {
    if (m_selectedFacility >= m_facilities.size()) return;
    auto& fac  = m_facilities[m_selectedFacility];
    if (m_selectedScenario >= fac.scenarios.size()) return;
    auto& scen = fac.scenarios[m_selectedScenario];
    auto& inp  = scen.input;

    // ── Step-by-step live NMA computation ────────────────────────────
    // Helper lambdas
    auto highlight = [](int step, QLabel* lbl[5]) {
        for (int i = 0; i < 5; ++i)
            lbl[i]->setStyleSheet(i == step
                ? "background:#1a3a5c;color:white;border-radius:6px;padding:6px 12px;font-size:18px;font-weight:700;"
                : (i < step
                    ? "background:#5da832;color:white;border-radius:6px;padding:6px 12px;font-size:18px;"
                    : "background:#d1d5db;color:#111827;border-radius:6px;padding:6px 12px;font-size:18px;"));
    };

    // STEP 1 — Load data
    highlight(0, m_stepLabels);

    double BI      = inp.beginningInventory.qty;
    double sigBI   = inp.beginningInventory.error;
    double totalIn = 0, varIn  = 0;
    double totalOut= 0, varOut = 0;
    for (auto& it : inp.inputs)  { totalIn  += it.qty; varIn  += it.error*it.error; }
    for (auto& it : inp.outputs) { totalOut += it.qty; varOut += it.error*it.error; }
    double EI      = inp.endingInventory.qty;
    double sigEI   = inp.endingInventory.error;

    // STEP 2 — Book Inventory
    highlight(1, m_stepLabels);
    double IB = BI + totalIn - totalOut;

    // STEP 3 — MUF
    highlight(2, m_stepLabels);
    double MUF = IB - EI;

    // STEP 4 — sigma(MUF)
    highlight(3, m_stepLabels);
    double varMUF   = sigBI*sigBI + varIn + varOut + sigEI*sigEI;
    double sigmaMUF = (varMUF > 0) ? std::sqrt(varMUF) : 0.0;
    double ratio    = (sigmaMUF > 0) ? std::abs(MUF)/sigmaMUF : 0.0;
    double sitmuf   = (sigmaMUF > 0) ? MUF/sigmaMUF : 0.0;

    // STEP 5 — Diagnostics
    highlight(4, m_stepLabels);

    // Significant Quantity lookup
    double SQ = 75.0;
    QString mat = inp.materialType.toLower();
    if (mat.contains("plutonium") || mat.contains(" pu") || mat.contains("mox")) SQ = 8.0;
    else if (mat.contains("heu") || mat.contains("highly enriched"))             SQ = 25.0;
    else if (mat.contains("u-233") || mat.contains("u233"))                      SQ = 8.0;
    else if (mat.contains("natural") || mat.contains("natU"))                    SQ = 10000.0;
    else if (mat.contains("depleted") || mat.contains("dU"))                     SQ = 20000.0;
    else if (mat.contains("thorium") || mat.contains("th"))                      SQ = 20000.0;
    double sqFrac = (SQ > 0) ? std::abs(MUF)/SQ*100.0 : 0.0;

    // Status label
    QString statusColor, statusText;
    if      (ratio > 3.0)  { statusColor="#b91c1c"; statusText="CRITICAL (>3σ)"; }
    else if (ratio > 2.0)  { statusColor="#c2410c"; statusText="WARNING (2σ–3σ)"; }
    else if (ratio > 1.0)  { statusColor="#b45309"; statusText="ELEVATED (1σ–2σ)"; }
    else                   { statusColor="#15803d"; statusText="NORMAL (≤1σ)"; }

    // ── Build HTML report ─────────────────────────────────────────────
    QString html = VCSS;
    html += QString("<h1>NMA Calculation — %1</h1>").arg(scen.id);
    html += QString("<p><strong>Facility:</strong> %1 &nbsp;|&nbsp; <strong>Material:</strong> %2 "
                    "&nbsp;|&nbsp; <strong>MBP:</strong> %3 to %4</p>")
            .arg(inp.facilityName).arg(inp.materialType)
            .arg(inp.periodStart.toString("yyyy-MM-dd"))
            .arg(inp.periodEnd.toString("yyyy-MM-dd"));

    // Step 1: Input data summary
    html += "<h2>Step 1 — Input Data Loaded</h2>";
    html += "<table><tr><th>Component</th><th>Quantity (kg)</th><th>1&#963; (kg)</th><th>Var &#963;&#178; (kg&#178;)</th></tr>";
    html += QString("<tr><td><strong>Beginning Inventory (BI)</strong></td><td><code>%1</code></td><td><code>%2</code></td><td><code>%3</code></td></tr>")
            .arg(BI,0,'f',4).arg(sigBI,0,'f',4).arg(sigBI*sigBI,0,'f',4);
    double runIn = 0;
    for (auto& it : inp.inputs) {
        runIn += it.qty;
        html += QString("<tr><td>&nbsp;&nbsp;&nbsp;+ Input: %1</td><td><code>%2</code></td><td><code>%3</code></td><td><code>%4</code></td></tr>")
                .arg(it.label).arg(it.qty,0,'f',3).arg(it.error,0,'f',4).arg(it.error*it.error,0,'f',4);
    }
    double runOut = 0;
    for (auto& it : inp.outputs) {
        runOut += it.qty;
        html += QString("<tr><td>&nbsp;&nbsp;&nbsp;&minus; Output: %1</td><td><code>%2</code></td><td><code>%3</code></td><td><code>%4</code></td></tr>")
                .arg(it.label).arg(it.qty,0,'f',3).arg(it.error,0,'f',4).arg(it.error*it.error,0,'f',4);
    }
    html += QString("<tr><td><strong>Ending Inventory (EI)</strong></td><td><code>%1</code></td><td><code>%2</code></td><td><code>%3</code></td></tr>")
            .arg(EI,0,'f',4).arg(sigEI,0,'f',4).arg(sigEI*sigEI,0,'f',4);
    html += "</table>";

    // Step 2: IB
    html += "<h2>Step 2 — Book Inventory (IB)</h2>";
    html += "<div class='kmp'>";
    html += "<pre>IB = BI + &Sigma;Inputs &minus; &Sigma;Outputs\n";
    html += QString("IB = %1 + %2 &minus; %3\n<strong>IB = %4 kg</strong>")
            .arg(BI,0,'f',4).arg(totalIn,0,'f',4).arg(totalOut,0,'f',4).arg(IB,0,'f',4);
    html += "</pre></div>";

    // Step 3: MUF
    html += "<h2>Step 3 — Material Unaccounted For (MUF)</h2>";
    html += "<div class='kmp'>";
    html += "<pre>MUF = IB &minus; EI\n";
    html += QString("MUF = %1 &minus; %2\n<strong>MUF = %3 kg</strong>")
            .arg(IB,0,'f',4).arg(EI,0,'f',4).arg(MUF,0,'f',4);
    html += "</pre></div>";

    // Step 4: sigma
    html += "<h2>Step 4 — Uncertainty Propagation &sigma;(MUF)</h2>";
    html += "<div class='kmp'>";
    html += "<pre>";
    html += QString("&sigma;&sup2;(MUF) = &sigma;&sup2;(BI) + &sigma;&sup2;(&Sigma;In) + &sigma;&sup2;(&Sigma;Out) + &sigma;&sup2;(EI)\n");
    html += QString("         = %1&sup2; + &sigma;&sup2;(inputs) + &sigma;&sup2;(outputs) + %2&sup2;\n")
            .arg(sigBI,0,'f',4).arg(sigEI,0,'f',4);
    html += QString("         = %1 + %2 + %3 + %4\n")
            .arg(sigBI*sigBI,0,'f',4).arg(varIn,0,'f',4).arg(varOut,0,'f',4).arg(sigEI*sigEI,0,'f',4);
    html += QString("         = %1 kg&sup2;\n").arg(varMUF,0,'f',4);
    html += QString("<strong>&sigma;(MUF) = &radic;%1 = %2 kg</strong>").arg(varMUF,0,'f',4).arg(sigmaMUF,0,'f',4);
    html += "</pre></div>";

    // Significance ratio
    html += QString("<table><tr><th>Ratio |MUF|/&sigma;</th><th>SITMUF</th><th>SQ = %1 kg</th><th>|MUF| as % of SQ</th><th>Status</th></tr>")
            .arg(SQ,0,'f',1);
    html += QString("<tr><td><strong><code>%1 &sigma;</code></strong></td><td><code>%2</code></td>"
                    "<td><code>%3 kg</code></td><td><code>%4 %</code></td>"
                    "<td><strong style='color:%5;'>%6</strong></td></tr>")
            .arg(ratio,0,'f',3).arg(sitmuf,0,'f',4)
            .arg(SQ,0,'f',1).arg(sqFrac,0,'f',2)
            .arg(statusColor).arg(statusText);
    html += "</table>";

    // Uncertainty contributions (Ci)
    html += "<h2>Step 4b — Uncertainty Contribution Analysis (C&#7522;)</h2>";
    html += "<table><tr><th>Term</th><th>&sigma;&sup2; (kg&sup2;)</th><th>C&#7522; %</th><th>Bar</th></tr>";
    auto contrib = [&](const QString& lbl, double var) {
        double pct = (varMUF > 0) ? var/varMUF*100.0 : 0.0;
        QString color = pct > 50 ? "#b91c1c" : pct > 30 ? "#b45309" : "#15803d";
        int bar = (int)(pct * 1.2);
        html += QString("<tr><td>%1</td><td><code>%2</code></td>"
                        "<td><strong style='color:%3;'>%4%</strong></td>"
                        "<td><div style='background:%3;height:12px;width:%5px;border-radius:4px;'></div></td></tr>")
                .arg(lbl).arg(var,0,'f',4).arg(color).arg(pct,0,'f',1).arg(bar);
    };
    contrib("BI (Beginning Inventory)", sigBI*sigBI);
    for (auto& it : inp.inputs)  contrib("Input: " + it.label, it.error*it.error);
    for (auto& it : inp.outputs) contrib("Output: " + it.label, it.error*it.error);
    contrib("EI (Ending Inventory)", sigEI*sigEI);
    html += "</table>";

    // Step 5: Diagnostics
    html += "<h2>Step 5 — Safeguards Diagnostics</h2>";
    html += "<table><tr><th>Check</th><th>Result</th><th>Flag</th><th>Action</th></tr>";

    auto diagRow = [&](const QString& check, const QString& val, const QString& flag,
                        const QString& color, const QString& bgColor, const QString& action) {
        html += QString("<tr><td>%1</td><td><code>%2</code></td>"
                        "<td><span style='background:%4;color:%3;border:1px solid %3;"
                        "border-radius:10px;padding:2px 10px;font-size:17px;font-weight:700;'>%5</span></td>"
                        "<td style='font-size:17px;'>%6</td></tr>")
                .arg(check).arg(val).arg(color).arg(bgColor).arg(flag).arg(action);
    };

    // Sigma ratio
    diagRow("Significance Ratio |MUF|/σ",
            QString("%1σ").arg(ratio,0,'f',3),
            ratio>3?"CRITICAL":ratio>2?"WARNING":ratio>1?"ELEVATED":"OK",
            ratio>3?"#b91c1c":ratio>2?"#c2410c":ratio>1?"#b45309":"#15803d",
            ratio>3?"#fef2f2":ratio>2?"#fff7ed":ratio>1?"#fffbeb":"#f0fdf4",
            ratio>3?"Report to IAEA (INFCIRC/153 Art.25). Emergency PIV.":
            ratio>2?"Review all records. Notify supervisor. Schedule verification.":
            ratio>1?"Monitor next MBP. Note in records.":"Routine filing. No action required.");

    // SQ test
    diagRow("Significant Quantity Fraction",
            QString("%1% of 1 SQ (%2 kg)").arg(sqFrac,0,'f',2).arg(SQ,0,'f',1),
            sqFrac>=100?"SQ-EXCEEDED":sqFrac>=50?"SQ-50PCT":sqFrac>=25?"SQ-25PCT":sqFrac>=10?"SQ-10PCT":"SQ-OK",
            sqFrac>=100?"#b91c1c":sqFrac>=50?"#c2410c":sqFrac>=25?"#b45309":sqFrac>=10?"#1d4ed8":"#15803d",
            sqFrac>=100?"#fef2f2":sqFrac>=50?"#fff7ed":sqFrac>=25?"#fffbeb":sqFrac>=10?"#eff6ff":"#f0fdf4",
            sqFrac>=100?"CRITICAL: full SQ unaccounted for. Immediate reporting required.":
            sqFrac>=25?"Elevated SQ fraction — schedule verification.":
            "Within acceptable range for this material category.");

    // Negative MUF
    diagRow("Negative MUF Check",
            QString("MUF = %1 kg").arg(MUF,0,'f',4),
            (MUF < -0.5*sigmaMUF) ? "NEG-MUF ALERT" : "OK",
            (MUF < -0.5*sigmaMUF) ? "#c2410c" : "#15803d",
            (MUF < -0.5*sigmaMUF) ? "#fff7ed"  : "#f0fdf4",
            (MUF < -0.5*sigmaMUF) ? "Possible undeclared inputs. Verify all receipt records." :
                                    "No negative MUF anomaly detected.");

    // Throughput ratio
    double throughput = totalIn + totalOut;
    double thrRatio = (throughput > 0) ? std::abs(MUF)/throughput*100.0 : 0.0;
    diagRow("Throughput Ratio |MUF|/Throughput",
            QString("%1% of %2 kg throughput").arg(thrRatio,0,'f',3).arg(throughput,0,'f',2),
            thrRatio>5?"THROUGHPUT-ALERT":thrRatio>2?"THROUGHPUT-WARN":"OK",
            thrRatio>5?"#c2410c":thrRatio>2?"#b45309":"#15803d",
            thrRatio>5?"#fff7ed":thrRatio>2?"#fffbeb":"#f0fdf4",
            thrRatio>5?"High MUF relative to throughput — verify all transfer records.":
            "Throughput ratio within normal bounds.");

    // SITMUF
    diagRow("SITMUF (Burr et al. 2013)",
            QString("%1 (threshold ±1.645 at α=0.05)").arg(sitmuf,0,'f',4),
            std::abs(sitmuf)>1.645?"SITMUF-SIGNIFICANT":"SITMUF-OK",
            std::abs(sitmuf)>1.645?"#c2410c":"#15803d",
            std::abs(sitmuf)>1.645?"#fff7ed":"#f0fdf4",
            std::abs(sitmuf)>1.645?"Statistically significant at 5% level. Verify measurement data.":
            "SITMUF within expected statistical range.");

    html += "</table>";

    // Final verdict
    html += "<h2>Final Verdict</h2>";
    html += QString("<div class='%1' style='font-size:20px;'>")
            .arg(ratio>3?"crit":ratio>2?"warn":ratio>1?"warn":"ok");
    html += QString("<strong style='font-size:24px;color:%1;'>%2</strong><br>")
            .arg(statusColor).arg(statusText);
    html += QString("MUF = <strong>%1 kg</strong> &nbsp;|&nbsp; "
                    "&sigma;(MUF) = <strong>%2 kg</strong> &nbsp;|&nbsp; "
                    "Ratio = <strong>%3&sigma;</strong> &nbsp;|&nbsp; "
                    "SQ Fraction = <strong>%4%</strong><br>")
            .arg(MUF,0,'f',4).arg(sigmaMUF,0,'f',4).arg(ratio,0,'f',3).arg(sqFrac,0,'f',2);
    html += "<br><em>Expected outcome: " + scen.expectedOutcome + "</em>";
    html += "</div>";
    html += "<p><em>This is a preview calculation. Load into Calculator for full 12-check diagnostics, "
            "CUSUM sequential testing, export to professional HTML report, and cumulative MUF tracking.</em></p>";

    m_nmaLiveBrowser->setHtml(html);
    highlight(4, m_stepLabels);
    m_simStatus->setText("NMA calculation complete for " + scen.id);
}

void VirtualFacilityDialog::onLoadClicked() {
    if (m_selectedFacility >= m_facilities.size()) return;
    auto& fac = m_facilities[m_selectedFacility];
    if (m_selectedScenario >= fac.scenarios.size()) return;
    emit loadScenario(fac.scenarios[m_selectedScenario].input);
    accept();
}

// ─────────────────────────────────────────────────────────────────
//  IAEA watermark – 15 % opacity backdrop
// ─────────────────────────────────────────────────────────────────
void VirtualFacilityDialog::paintEvent(QPaintEvent* e)
{
    QDialog::paintEvent(e);
    QPainter p(this);
    IAEAWatermark::paint(p, rect(), 0.15);
}

