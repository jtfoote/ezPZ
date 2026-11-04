#pragma once
#include "ScientificCalculatorTab.h"


// =============================================================================
// Tab 6: Engineering Formulas & Variable Definitions Cheat Sheet (9 Sections + LaTeX)
// =============================================================================
struct CheatItemSpec {
    std::string label;
    std::string latex;
    std::string notes;
    std::string badge;
};

struct CheatSectionSpec {
    std::string key;
    std::string title;
    std::string accent;
    std::string subtitle;
    std::vector<CheatItemSpec> items;
};

static const std::vector<CheatSectionSpec> CHEAT_SECTIONS = {
    {"variables", "1. Variable & Symbol Definitions (Complete Reference)", Theme::ACCENT_ZERO.toStdString(),
     "Core notation, units, and physical meanings across s-domain, z-domain, PID control, and error analysis", {
        {"Complex s-Domain Variable", "s = \\sigma + j\\omega \\quad (\\sigma \\text{ in Np/s}, \\; \\omega \\text{ in rad/s})", "Laplace complex frequency variable.", "Nise Ch. 2 & 4"},
        {"Complex z-Domain Variable", "z = r e^{j\\Omega} = e^{s T_s}, \\quad r = |z| = e^{\\sigma T_s}, \\quad \\Omega = \\omega T_s", "Discrete-time z-transform variable.", "Discrete DSP"},
        {"Open-Loop & Cascade Plant G(s)", "G(s) = \\frac{K \\mathrm{Num}(s)}{\\mathrm{Den}(s)} = \\frac{K_0}{s^2 + a_1 s + a_0}", "Rational continuous plant transfer function.", "Nise Sec. 4.2"},
        {"Absolute & Relative Steady-State Error", "e_{\\mathrm{ss}} = |M - y_{\\mathrm{ss}}|, \\quad \\%e_{\\mathrm{ss}} = \\frac{e_{\\mathrm{ss}}}{M} \\times 100\\%", "Steady-state tracking discrepancy and normalized percentage error.", "Nise Ch. 7 • A4 Q1"}
    }},
    {"first_order", "2. First-Order Systems Equations (Nise Sec. 4.3 • A3 P4.2 & P4.29a)", Theme::ACCENT_PID.toStdString(),
     "Pole-form & time-constant transfer functions, step response, Tr, Ts, and step-graph identification", {
        {"Standard & Pole-Form Transfer Functions", "G(s) = \\frac{K}{s + a} = \\frac{K_{\\mathrm{dc}}}{T s + 1} \\quad \\left(T = \\frac{1}{a}, \\; K_{\\mathrm{dc}} = \\frac{K}{a}\\right)", "Single real pole at s = -a with time constant T = 1/a.", "Nise Eq. (4.1)"},
        {"First-Order Rise & Settling Times", "T_r = \\frac{\\ln(9)}{a} \\approx \\frac{2.2}{a} = 2.2 T, \\qquad T_s(\\pm 2\\%) = \\frac{-\\ln(0.02)}{a} \\approx \\frac{4}{a} = 4 T", "10–90% rise time and ±2% settling time.", "Nise Eq. (4.7)–(4.8)"}
    }},
    {"second_order", "3. Second-Order Systems & Transient Specs (Nise Sec. 4.4–4.6 • A3 & A4)", Theme::ACCENT_LOCUS.toStdString(),
     "Standard forms, damping classifications, pole geometry, Tp, %OS, Ts, and Nise cubic Tr", {
        {"Canonical Second-Order Form", "G(s) = \\frac{K_0}{s^2 + a_1 s + a_0} = \\frac{K_{\\mathrm{dc}} \\omega_n^2}{s^2 + 2\\zeta\\omega_n s + \\omega_n^2} \\Rightarrow \\omega_n = \\sqrt{a_0}, \\; \\zeta = \\frac{a_1}{2\\sqrt{a_0}}", "Matches denominator s² + a1·s + a0 to canonical parameters (ζ, ωn).", "Nise Eq. (4.22)"},
        {"Pole Locations & Geometry", "s_{1,2} = -\\zeta\\omega_n \\pm j\\omega_n\\sqrt{1 - \\zeta^2} = -\\sigma_d \\pm j\\omega_d", "Complex conjugate underdamped poles for 0 < ζ < 1.", "Nise Eq. (4.24)"},
        {"Peak Time, Overshoot & Settling Time", "T_p = \\frac{\\pi}{\\omega_d}, \\quad \\%OS = e^{-\\left(\\frac{\\zeta\\pi}{\\sqrt{1-\\zeta^2}}\\right)} \\times 100\\%, \\quad \\zeta = \\frac{-\\ln(\\%OS/100)}{\\sqrt{\\pi^2 + \\ln^2(\\%OS/100)}}, \\quad T_s = \\frac{4}{\\zeta\\omega_n}", "Core transient specification equations.", "Nise Eq. (4.34)–(4.42)"},
        {"Nise Cubic Rise Time Formula", "T_r \\approx \\frac{1.76\\zeta^3 - 0.417\\zeta^2 + 1.039\\zeta + 1}{\\omega_n}", "Nise Fig. 4.16 Footnote 5 cubic polynomial for 10–90% rise time.", "Nise Fig. 4.16"}
    }},
    {"sse_relative_error", "4. Steady-State Error (S.S.E.), Relative Error & System Type (Nise Ch. 7 • A4)", Theme::ACCENT_POLE.toStdString(),
     "Absolute S.S.E., Relative Error (%e_ss), amplitude scaling M, Kp/Kv/Ka constants, and S.S.E. redesign", {
        {"Step S.S.E. & Relative Error for Type 0 Plant", "K_{p,\\mathrm{pos}} = \\frac{K_p K_0}{a_0} \\Rightarrow e_{\\mathrm{ss}} = \\frac{M}{1 + K_{p,\\mathrm{pos}}} = \\frac{M a_0}{a_0 + K_p K_0}, \\quad \\%e_{\\mathrm{ss}} = \\frac{e_{\\mathrm{ss}}}{M} \\times 100\\%", "Relative error %e_ss is independent of step amplitude M!", "A4 Q1(6,8)"},
        {"Redesigning Kp to Reduce S.S.E. by p%", "e_{\\mathrm{ss,new}} = \\left(1 - \\frac{p}{100}\\right)e_{\\mathrm{ss,old}}, \\quad K_{p,\\mathrm{new}} = \\frac{a_0}{K_0}\\left(\\frac{M}{e_{\\mathrm{ss,new}}} - 1\\right)", "Increasing Kp reduces S.S.E. but decreases ζ, increasing %OS.", "A4 Q1(9–10)"}
    }},
    {"a4_pid_design", "5. P, PD, PI & PID Controller Design Equations (CNTL-1016 Assignment 4)", Theme::ACCENT_PID.toStdString(),
     "Closed-loop T(s) derivations and analytical Kp/Kd/Ki design equations for %OS and ζ", {
        {"P-Controller Closed-Loop T(s) & Kp Design", "T(s) = \\frac{K_p K_0}{s^2 + a_1 s + (a_0 + K_p K_0)}, \\quad \\omega_n = \\frac{a_1}{2\\zeta}, \\quad K_p = \\frac{\\omega_n^2 - a_0}{K_0}", "Solves A4 Q1(4–5) for target %OS.", "A4 Q1(4–5)"},
        {"PD-Controller Closed-Loop T(s) & Kd/Kp Design", "T(s) = \\frac{K_0(K_d s + K_p)}{s^2 + (a_1 + K_0 K_d)s + (a_0 + K_0 K_p)}, \\quad K_d = \\frac{2\\zeta\\sqrt{a_0 + K_0 K_p} - a_1}{K_0}", "Solves A4 Q2(1–5) for PD control.", "A4 Q2(1–5)"},
        {"PI-Controller on 1st-Order Plant K0/(s+a0)", "T(s) = \\frac{K_0(K_p s + K_i)}{s^2 + (a_0 + K_0 K_p)s + K_0 K_i}, \\quad \\omega_n = \\frac{a_0 + K_0 K_p}{2\\zeta}, \\quad K_i = \\frac{\\omega_n^2}{K_0}, \\quad e_{\\mathrm{ss}} = 0", "Solves A4 Q4(1–7); integrator drives step S.S.E. to 0.", "A4 Q4(1–7)"}
    }},
    {"approximations", "6. Nise Higher-Order & Pole-Zero Approximations (Nise Sec. 4.7–4.8)", Theme::ACCENT_OPEN.toStdString(),
     "5× dominant-pole rule, DC-gain matching, and pole-zero cancellation", {
        {"5× Dominant-Pole Rule & DC-Gain Reduction", "\\alpha_r \\geq 5\\zeta\\omega_n = 5\\sigma_d, \\qquad T_{\\mathrm{approx}}(s) = \\frac{K / \\alpha_r}{s^2 + 2\\zeta\\omega_n s + \\omega_n^2}", "Preserves steady-state DC gain T(0) when discarding fast pole (s + αr).", "Nise Sec. 4.7"}
    }},
    {"root_locus", "7. Root Locus, Routh-Hurwitz Stability & Frequency Response", Theme::ACCENT_ZERO.toStdString(),
     "Angle/magnitude conditions, asymptotes, and compensator deficiency", {
        {"Root Locus Angle & Magnitude Conditions", "1 + K G(s) = 0 \\iff \\angle G(s) = (2k+1)180^\\circ, \\quad K = \\frac{1}{|G(s)|}, \\quad \\sigma_a = \\frac{\\sum p_j - \\sum z_i}{n - m}", "Fundamental Evans Root Locus conditions.", "Nise Sec. 8.1–8.2"}
    }},
    {"discrete_z", "8. Discrete-Time Systems & Digital Filters (z-Plane)", Theme::ACCENT_ASYMP.toStdString(),
     "Exact s-to-z mapping, Unit Circle BIBO stability, Tustin transform, and Difference Equations", {
        {"s-to-z Mapping, Tustin & Difference Equation", "z = e^{s T_s}, \\quad s = \\frac{2}{T_s}\\frac{z - 1}{z + 1}, \\quad y[k] = \\sum_{m=0}^M b_m x[k-m] - \\sum_{n=1}^N a_n y[k-n]", "Causal digital filter recurrence and discretization.", "Nise Ch. 13"}
    }},
    {"transforms", "9. Laplace & z-Transform Pairs and Theorems Table", Theme::ACCENT_LOCUS.toStdString(),
     "Essential time-domain waveforms, Laplace transforms F(s), z-transforms F(z), and Final Value Theorem", {
        {"Step, Exponential & Damped Sinusoid Pairs", "\\mathcal{L}\\{M u(t)\\} = \\frac{M}{s}, \\quad \\mathcal{L}\\{e^{-at}\\} = \\frac{1}{s+a}, \\quad \\mathcal{L}\\{e^{-\\sigma_d t}\\cos(\\omega_d t)\\} = \\frac{s+\\sigma_d}{(s+\\sigma_d)^2 + \\omega_d^2}", "Core Laplace transform pairs.", "Nise Table 2.1"},
        {"Final Value Theorem (Continuous & Discrete)", "f(\\infty) = \\lim_{s\\to 0} s F(s) = \\lim_{z\\to 1} (z - 1) F(z)", "Finds steady-state output y_ss and error e_ss directly.", "Nise Sec. 7.2"}
    }}
};

class FormulaCheatSheetTab : public QWidget {
    Q_OBJECT
public:
    SharedSystemModel* model;
    MathFormulaWidget *live_row_a, *live_row_b;
    std::string active_sec_key = "all";
    std::map<std::string, QPushButton*> sec_btns;
    QLineEdit* search_edit;
    QVBoxLayout* sections_layout;

    explicit FormulaCheatSheetTab(SharedSystemModel* m, QWidget* parent = nullptr)
        : QWidget(parent), model(m)
    {
        auto* vl = new QVBoxLayout(this);
        vl->setContentsMargins(0, 0, 0, 0);
        vl->setSpacing(6);

        // 1. Live Synchronized LaTeX Banner
        auto* top_card = makeCard();
        auto* tl = new QVBoxLayout(top_card);
        tl->setContentsMargins(12, 8, 12, 8);
        auto* th = new QLabel("Live Active System Equations, S.S.E. / Relative Error & Nise Specs (Synchronized from Tabs 1–4)");
        th->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 12px; border:none;").arg(Theme::ACCENT_PID));
        live_row_a = new MathFormulaWidget("", QColor(Theme::ACCENT_ZERO), QColor(Theme::BG_INPUT), 10);
        live_row_b = new MathFormulaWidget("", QColor(Theme::ACCENT_LOCUS), QColor(Theme::BG_INPUT), 10);
        tl->addWidget(th);
        tl->addWidget(live_row_a);
        tl->addWidget(live_row_b);
        vl->addWidget(top_card);

        // 2. Section Filter Bar + Keyword Search
        auto* filter_card = makeCard();
        auto* fl = new QHBoxLayout(filter_card);
        fl->setContentsMargins(10, 6, 10, 6);
        const std::vector<std::pair<std::string, QString>> sec_opts = {
            {"all", "All (9)"}, {"variables", "1. Variables"}, {"first_order", "2. 1st-Order"},
            {"second_order", "3. 2nd-Order"}, {"sse_relative_error", "4. S.S.E. & Rel Err"},
            {"a4_pid_design", "5. P/PD/PI/PID (A4)"}, {"approximations", "6. Nise Approx"},
            {"root_locus", "7. RL & Routh"}, {"discrete_z", "8. z-Domain"}, {"transforms", "9. Transforms"}
        };
        for (const auto& [sk, sl] : sec_opts) {
            auto* b = makeActionBtn(sl, sk == "all" ? "#ffffff" : Theme::TEXT_SECONDARY, sk == "all" ? Theme::BG_KEY_ACCENT : Theme::BG_INPUT);
            connect(b, &QPushButton::clicked, this, [this, sk]() { selectSection(sk); });
            fl->addWidget(b);
            sec_btns[sk] = b;
        }
        fl->addStretch();
        auto* fl_lbl = new QLabel("Filter:"); fl_lbl->setStyleSheet("color: #94a3b8; font-weight: bold; border:none;");
        search_edit = makeDarkEntry("", 140);
        fl->addWidget(fl_lbl);
        fl->addWidget(search_edit);
        vl->addWidget(filter_card);

        // 3. Scrollable LaTeX Sections Container
        auto* scroll = new QScrollArea();
        scroll->setWidgetResizable(true);
        scroll->setStyleSheet(QString("background-color: %1; border: 1px solid %2;").arg(Theme::BG_WINDOW, Theme::BORDER_PANEL));
        auto* body = new QWidget();
        sections_layout = new QVBoxLayout(body);
        sections_layout->setAlignment(Qt::AlignTop);
        sections_layout->setSpacing(10);
        scroll->setWidget(body);
        vl->addWidget(scroll, 1);

        connect(search_edit, &QLineEdit::textChanged, this, [this]() { renderSections(); });

        refreshBanner();
        renderSections();
    }

    void selectSection(const std::string& sk) {
        active_sec_key = sk;
        for (auto& [k, b] : sec_btns) {
            bool sel = (k == sk);
            b->setStyleSheet(QString("QPushButton { background-color: %1; color: %2; border: 1px solid #4b586e; border-radius: 4px; padding: 4px 8px; font-weight: bold; font-size: 10px; }")
                             .arg(sel ? Theme::BG_KEY_ACCENT : Theme::BG_INPUT, sel ? "#ffffff" : Theme::TEXT_SECONDARY));
        }
        renderSections();
    }

    void refreshBanner() {
        std::string gs = formatTransferFunctionLatex(model->poles, model->zeros, model->plant_gain, "s", "G");
        std::string hz = formatTransferFunctionLatex(model->z_poles, model->z_zeros, model->filter_gain, "z", "H");
        live_row_a->setLatex(QString::fromStdString(gs + " \\qquad | \\qquad " + hz));

        auto dm = computeDominantSMetrics(model->poles);
        double z = dm.zeta.value_or(0.7071), wn = dm.omega_n.value_or(2.8284), wd = dm.omega_d.value_or(2.0);
        double os = niseOsFromZeta(z);
        double ts = (z * wn > 1e-9) ? (4.0 / (z * wn)) : 0.0;
        std::string specs = "\\zeta = " + PRECISION.fmt(z) + ", \\; \\omega_n = " + PRECISION.fmt(wn) + " \\text{ rad/s}, \\; \\omega_d = " +
                            PRECISION.fmt(wd) + " \\text{ rad/s}, \\; T_s = \\frac{4}{\\zeta\\omega_n} = " + PRECISION.fmt(ts) +
                            " \\text{ s}, \\; \\%OS = " + PRECISION.fmt(os) + "\\%";
        if (model->last_sim_result.has_value()) {
            const auto& r = model->last_sim_result.value();
            specs += " \\quad | \\quad e_{\\mathrm{ss}} = " + PRECISION.fmt(r.steady_state_error) +
                     ", \\; \\%e_{\\mathrm{ss}} = " + PRECISION.fmt(r.relative_error_pct) + "\\%";
        }
        live_row_b->setLatex(QString::fromStdString(specs));
    }

    void renderSections() {
        QLayoutItem* item;
        while ((item = sections_layout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        std::string q = search_edit->text().toLower().toStdString();

        for (const auto& sec : CHEAT_SECTIONS) {
            if (active_sec_key != "all" && sec.key != active_sec_key) continue;
            std::vector<CheatItemSpec> matched;
            for (const auto& it : sec.items) {
                if (q.empty()) { matched.push_back(it); continue; }
                std::string hay = sec.title + " " + it.label + " " + it.latex + " " + it.notes + " " + it.badge;
                std::transform(hay.begin(), hay.end(), hay.begin(), ::tolower);
                if (hay.find(q) != std::string::npos) matched.push_back(it);
            }
            if (matched.empty()) continue;

            auto* scard = makeCard();
            auto* sl = new QVBoxLayout(scard);
            sl->setContentsMargins(12, 8, 12, 8);
            auto* shdr = new QHBoxLayout();
            auto* st = new QLabel(QString::fromStdString(sec.title));
            st->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 13px; border:none;").arg(QString::fromStdString(sec.accent)));
            auto* ssub = new QLabel(QString::fromStdString(sec.subtitle));
            ssub->setStyleSheet("color: #94a3b8; font-size: 10px; border:none;");
            shdr->addWidget(st); shdr->addStretch(); shdr->addWidget(ssub);
            sl->addLayout(shdr);

            for (size_t idx = 0; idx < matched.size(); ++idx) {
                const auto& it = matched[idx];
                QString rbg = (idx % 2 == 0) ? Theme::BG_INPUT : Theme::BG_PANEL_ALT;
                auto* rf = new QFrame();
                rf->setStyleSheet(QString("QFrame { background-color: %1; border: 1px solid %2; border-radius: 4px; }").arg(rbg, Theme::BORDER_INPUT));
                auto* rl = new QHBoxLayout(rf);
                rl->setContentsMargins(10, 5, 10, 5);

                auto* left_b = new QVBoxLayout();
                auto* ll = new QLabel(QString::fromStdString(it.label));
                ll->setFixedWidth(230);
                ll->setWordWrap(true);
                ll->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border:none;").arg(Theme::TEXT_PRIMARY));
                auto* bl = new QLabel(QString::fromStdString(it.badge));
                bl->setStyleSheet("color: #94a3b8; font-family: Monospace; font-size: 9px; border:none;");
                left_b->addWidget(ll); left_b->addWidget(bl);
                rl->addLayout(left_b);

                auto* mw = new MathFormulaWidget(QString::fromStdString(it.latex), QColor(QString::fromStdString(sec.accent)), QColor(rbg), 11);
                rl->addWidget(mw, 1);

                auto* info_b = new InfoBadgeButton(QString::fromStdString(it.label), QString::fromStdString(it.notes), QString::fromStdString(it.badge));
                rl->addWidget(info_b);
                sl->addWidget(rf);
            }
            sections_layout->addWidget(scard);
        }
    }
};

