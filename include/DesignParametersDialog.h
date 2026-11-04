#pragma once
#include "TimeResponseCanvas.h"


// =============================================================================
// Tab 1: s-Plane Root Locus & Frequency Response View
// =============================================================================
class FeedbackConfigBar : public QWidget {
    Q_OBJECT
public:
    QRadioButton *unity_rb, *custom_rb;
    QLineEdit *h_num_edit, *h_den_edit;
    QWidget *custom_row;
    bool updating = false;
    std::function<void()> onChanged;

    explicit FeedbackConfigBar(const QString& var_sym = "s", const QString& fb_sym = "H", QWidget* parent = nullptr)
        : QWidget(parent)
    {
        auto* vl = new QVBoxLayout(this);
        vl->setContentsMargins(0, 2, 0, 2);
        vl->setSpacing(3);

        auto* top_r = new QHBoxLayout();
        auto* lbl = new QLabel(QString("Feedback Path %1(%2):").arg(fb_sym, var_sym));
        lbl->setStyleSheet("color: #94a3b8; font-size: 10px; font-weight: bold; border: none;");
        unity_rb  = new QRadioButton(QString("Unity (%1 = 1)").arg(fb_sym));
        custom_rb = new QRadioButton(QString("Custom %1(%2)").arg(fb_sym, var_sym));
        unity_rb->setChecked(true);
        for (auto* rb : {unity_rb, custom_rb}) {
            rb->setStyleSheet(QString("QRadioButton { color: %1; font-weight: bold; font-size: 10px; border: none; }").arg(Theme::TEXT_PRIMARY));
        }
        top_r->addWidget(lbl);
        top_r->addWidget(unity_rb);
        top_r->addWidget(custom_rb);
        top_r->addStretch();
        vl->addLayout(top_r);

        custom_row = new QWidget();
        auto* cr_l = new QHBoxLayout(custom_row);
        cr_l->setContentsMargins(0, 0, 0, 0);
        cr_l->setSpacing(6);
        auto* nl = new QLabel(QString("Num_%1(%2):").arg(fb_sym, var_sym));
        nl->setStyleSheet(QString("color: %1; font-family: Monospace; font-size: 10px; font-weight: bold; border:none;").arg(Theme::ACCENT_ASYMP));
        h_num_edit = makeDarkEntry("1", 70);
        auto* dl = new QLabel(QString("Den_%1(%2):").arg(fb_sym, var_sym));
        dl->setStyleSheet(QString("color: %1; font-family: Monospace; font-size: 10px; font-weight: bold; border:none;").arg(Theme::ACCENT_LOCUS));
        h_den_edit = makeDarkEntry(var_sym == "s" ? "s + 5" : "z - 0.5", 70);
        cr_l->addWidget(nl); cr_l->addWidget(h_num_edit, 1);
        cr_l->addWidget(dl); cr_l->addWidget(h_den_edit, 1);
        custom_row->setVisible(false);
        vl->addWidget(custom_row);

        connect(unity_rb, &QRadioButton::toggled, this, [this](bool is_unity) {
            custom_row->setVisible(!is_unity);
            if (!updating && onChanged) onChanged();
        });
        connect(h_num_edit, &QLineEdit::editingFinished, this, [this]() { if (!updating && onChanged) onChanged(); });
        connect(h_den_edit, &QLineEdit::editingFinished, this, [this]() { if (!updating && onChanged) onChanged(); });
        connect(h_num_edit, &QLineEdit::returnPressed,   this, [this]() { if (!updating && onChanged) onChanged(); });
        connect(h_den_edit, &QLineEdit::returnPressed,   this, [this]() { if (!updating && onChanged) onChanged(); });
    }

    void syncToModelS(SharedSystemModel* m) {
        m->unity_feedback_s = unity_rb->isChecked();
        m->h_num_str = h_num_edit->text().toStdString();
        m->h_den_str = h_den_edit->text().toStdString();
        if (!m->unity_feedback_s) {
            try {
                auto pn = parsePolynomialInput(m->h_num_str);
                auto pd = parsePolynomialInput(m->h_den_str);
                m->h_zeros = pn.roots;
                m->h_poles = pd.roots;
                m->h_gain  = pn.leading_gain / std::max(std::abs(pd.leading_gain), 1e-12);
            } catch (...) {}
        }
    }

    void loadFromModelS(const SharedSystemModel* m) {
        updating = true;
        unity_rb->setChecked(m->unity_feedback_s);
        custom_rb->setChecked(!m->unity_feedback_s);
        custom_row->setVisible(!m->unity_feedback_s);
        h_num_edit->setText(QString::fromStdString(m->h_num_str));
        h_den_edit->setText(QString::fromStdString(m->h_den_str));
        updating = false;
    }

    void syncToModelZ(SharedSystemModel* m) {
        m->unity_feedback_z = unity_rb->isChecked();
        m->hz_fb_num_str = h_num_edit->text().toStdString();
        m->hz_fb_den_str = h_den_edit->text().toStdString();
        if (!m->unity_feedback_z) {
            try {
                auto pn = parsePolynomialInput(m->hz_fb_num_str);
                auto pd = parsePolynomialInput(m->hz_fb_den_str);
                m->hz_fb_zeros = pn.roots;
                m->hz_fb_poles = pd.roots;
                m->hz_fb_gain  = pn.leading_gain / std::max(std::abs(pd.leading_gain), 1e-12);
            } catch (...) {}
        }
    }

    void loadFromModelZ(const SharedSystemModel* m) {
        updating = true;
        unity_rb->setChecked(m->unity_feedback_z);
        custom_rb->setChecked(!m->unity_feedback_z);
        custom_row->setVisible(!m->unity_feedback_z);
        h_num_edit->setText(QString::fromStdString(m->hz_fb_num_str));
        h_den_edit->setText(QString::fromStdString(m->hz_fb_den_str));
        updating = false;
    }
};

// =============================================================================
// Spec-Driven Keep (🔒) vs. Change (🔄) Parameter Matrix Engine & Dialog
// =============================================================================
struct ConstraintMatrixAnalysis {
    bool feasible = true;
    std::string controller_mode = "PD";
    std::string headline;
    std::string explanation;
    std::string formulas_latex;
    std::map<std::string, double> solved;
    std::map<std::string, std::string> notes;
};

class ParameterMatrixEngine {
public:
    struct ParamMeta {
        std::string key;
        QString name;
        QString unit;
        QString desc;
    };

    static const std::vector<ParamMeta>& metaList() {
        static const std::vector<ParamMeta> m = {
            {"kp",          "Proportional Gain Kp",   "",      "Controller gain on current error e(t). Controls natural frequency ωn and step S.S.E."},
            {"kd",          "Derivative Gain Kd",     "",      "Controller gain on de(t)/dt. Adds damping σd = (a1 + K0·Kd)/2 without changing step S.S.E."},
            {"ki",          "Integral Gain Ki",       "",      "Controller gain on ∫e(t)dt. Adds pole at s=0 (Type +1), driving step S.S.E. to 0."},
            {"os_pct",      "Percent Overshoot %OS",  "%",     "%OS = 100·exp(-ζπ/√(1-ζ²)). Depends strictly on ζ."},
            {"zeta",        "Damping Ratio ζ",        "",      "Dimensionless damping ratio (0<ζ<1 underdamped, ζ=1 critically damped, ζ>1 overdamped)."},
            {"wn",          "Natural Frequency ωn",   "rad/s", "Undamped natural frequency |s_p| = √(σd² + ωd²)."},
            {"ts",          "Settling Time Ts (±2%)", "s",     "2% settling time: Ts = 4/(ζωn) = 4/σd."},
            {"tp",          "Peak Time Tp",           "s",     "Time to first peak: Tp = π/ωd = π/(ωn√(1-ζ²))."},
            {"tr",          "Rise Time Tr (10–90%)",  "s",     "10–90% rise time via Nise cubic polynomial in ζ divided by ωn."},
            {"sigma_d",     "Decay Rate σd = ζωn",    "s⁻¹",   "Real envelope attenuation rate |Re(s_p)| = ζωn = 4/Ts."},
            {"omega_d",     "Damped Freq ωd",         "rad/s", "Sinusoidal oscillation frequency |Im(s_p)| = ωn√(1-ζ²) = π/Tp."},
            {"ess",         "Absolute S.S.E. (e_ss)", "",      "Steady-state error |r_ss - y_ss| = M / (1 + Kp,pos)."},
            {"rel_err_pct", "Relative Error (%e_ss)", "%",     "Normalized steady-state error (e_ss / M) × 100%."}
        };
        return m;
    }

    static ConstraintMatrixAnalysis evaluate(
        const std::map<std::string, bool>& locked,
        const std::map<std::string, double>& in,
        const SharedSystemModel* model)
    {
        ConstraintMatrixAnalysis out;
        double m_amp = std::max(std::abs(model->input_magnitude), 1e-9);
        auto [num_g, den_g] = ComplexTransferFunctionSimulator::buildPlantPolynomials(model->poles, model->zeros, model->plant_gain);

        size_t n_poles = model->poles.size();
        (void)model;
        double a1_ol = 1.0, a0_ol = 1.0, k0_ol = std::max(std::abs(num_g.back().real()), 1e-9);
        if (n_poles == 1) {
            a1_ol = 1.0;
            a0_ol = den_g[1].real();
        } else if (n_poles >= 2) {
            a1_ol = den_g[den_g.size() - 2].real();
            a0_ol = den_g.back().real();
        }

        int plant_integrators = 0;
        for (const auto& p : model->poles) if (std::abs(p) < 1e-7) ++plant_integrators;

        std::vector<std::string> conflicts;
        std::vector<std::string> formulas;

        auto isLk = [&](const std::string& k) { auto it = locked.find(k); return it != locked.end() && it->second; };
        auto getIn = [&](const std::string& k, double def) { auto it = in.find(k); return it != in.end() ? it->second : def; };

        // 1. Resolve zeta <-> os_pct
        std::optional<double> target_zeta;
        if (isLk("zeta") && isLk("os_pct")) {
            double z_in = std::max(0.0, getIn("zeta", 0.456));
            double os_in = std::max(0.0, getIn("os_pct", 20.0));
            double z_os = PRECISION.step(niseZetaFromOs(os_in));
            if (std::abs(z_in - z_os) > 0.015) {
                conflicts.push_back("• Conflict between Kept ζ (" + PRECISION.fmt(z_in) + ") and Kept %OS (" + PRECISION.fmt(os_in) + "%): Keeping %OS = " + PRECISION.fmt(os_in) + "% requires ζ = " + PRECISION.fmt(z_os) + ".");
            }
            target_zeta = z_os;
        } else if (isLk("os_pct")) {
            double os_in = std::max(0.0, getIn("os_pct", 20.0));
            target_zeta = PRECISION.step(niseZetaFromOs(os_in));
            out.notes["os_pct"] = "🔒 KEPT (Target Input)";
            out.notes["zeta"] = "🔄 Solved from %OS = " + PRECISION.fmt(os_in) + "%";
            formulas.push_back("\\zeta = \\frac{-\\ln(\\%OS/100)}{\\sqrt{\\pi^2 + \\ln^2(\\%OS/100)}} = " + PRECISION.fmt(target_zeta));
        } else if (isLk("zeta")) {
            target_zeta = PRECISION.step(std::max(0.0, getIn("zeta", 0.456)));
            out.notes["zeta"] = "🔒 KEPT (Target Input)";
            out.notes["os_pct"] = "🔄 Solved from ζ = " + PRECISION.fmt(target_zeta);
        }

        // 2. Resolve sigma_d <-> ts
        std::optional<double> target_sigma_d;
        if (isLk("ts")) {
            double ts_in = std::max(1e-6, getIn("ts", 1.6));
            target_sigma_d = PRECISION.step(4.0 / ts_in);
            out.notes["ts"] = "🔒 KEPT (Target Input)";
            out.notes["sigma_d"] = "🔄 Solved from Ts (4/Ts)";
            formulas.push_back("\\sigma_d = \\frac{4}{T_s} = " + PRECISION.fmt(target_sigma_d));
        } else if (isLk("sigma_d")) {
            target_sigma_d = PRECISION.step(std::max(1e-6, getIn("sigma_d", 2.5)));
            out.notes["sigma_d"] = "🔒 KEPT (Target Input)";
            out.notes["ts"] = "🔄 Solved from σd (4/σd)";
        }

        // 3. Resolve omega_d <-> tp
        std::optional<double> target_omega_d;
        if (isLk("tp")) {
            double tp_in = std::max(1e-6, getIn("tp", 0.64));
            target_omega_d = PRECISION.step(PI / tp_in);
            out.notes["tp"] = "🔒 KEPT (Target Input)";
            out.notes["omega_d"] = "🔄 Solved from Tp (π/Tp)";
            formulas.push_back("\\omega_d = \\frac{\\pi}{T_p} = " + PRECISION.fmt(target_omega_d));
        } else if (isLk("omega_d")) {
            target_omega_d = PRECISION.step(std::max(1e-6, getIn("omega_d", 4.88)));
            out.notes["omega_d"] = "🔒 KEPT (Target Input)";
            out.notes["tp"] = "🔄 Solved from ωd (π/ωd)";
        }

        // 4. Resolve ess <-> rel_err_pct
        std::optional<double> target_ess;
        if (isLk("ess")) {
            target_ess = PRECISION.step(std::max(0.0, getIn("ess", 0.2)));
            out.notes["ess"] = "🔒 KEPT (Target Input)";
        } else if (isLk("rel_err_pct")) {
            double rel_in = std::max(0.0, getIn("rel_err_pct", 20.0));
            target_ess = PRECISION.step(m_amp * (rel_in / 100.0));
            out.notes["rel_err_pct"] = "🔒 KEPT (Target Input)";
        }

        std::optional<double> target_wn;
        if (isLk("wn")) {
            target_wn = PRECISION.step(std::max(1e-4, getIn("wn", 5.48)));
            out.notes["wn"] = "🔒 KEPT (Target Input)";
        }
        std::optional<double> target_tr;
        if (isLk("tr")) {
            target_tr = PRECISION.step(std::max(1e-4, getIn("tr", 0.34)));
            out.notes["tr"] = "🔒 KEPT (Target Input)";
        }

        auto resolveZetaWn = [&]() -> std::pair<double, double> {
            if (target_zeta && target_wn) return {*target_zeta, *target_wn};
            if (target_zeta && target_sigma_d) return {*target_zeta, PRECISION.step(*target_sigma_d / std::max(*target_zeta, 1e-5))};
            if (target_zeta && target_omega_d) {
                double sq = std::sqrt(std::max(1e-6, 1.0 - (*target_zeta) * (*target_zeta)));
                return {*target_zeta, PRECISION.step(*target_omega_d / sq)};
            }
            if (target_zeta && target_tr) {
                double wn_tr = PRECISION.step(niseNormalizedTrFromZeta(*target_zeta));
                return {*target_zeta, PRECISION.step(wn_tr / std::max(*target_tr, 1e-6))};
            }
            if (target_sigma_d && target_omega_d) {
                double w = PRECISION.step(std::hypot(*target_sigma_d, *target_omega_d));
                return {PRECISION.step(*target_sigma_d / std::max(w, 1e-6)), w};
            }
            return {target_zeta.value_or(0.456), target_wn.value_or(5.48)};
        };

        bool kp_lk = isLk("kp"), kd_lk = isLk("kd"), ki_lk = isLk("ki");
        double kp_in = getIn("kp", model->kp);
        double kd_in = getIn("kd", model->kd);
        double ki_in = getIn("ki", model->ki);

        if (kp_lk) out.notes["kp"] = "🔒 KEPT (Kp = " + PRECISION.fmt(kp_in) + ")";
        if (kd_lk) out.notes["kd"] = "🔒 KEPT (Kd = " + PRECISION.fmt(kd_in) + ")";
        if (ki_lk) out.notes["ki"] = "🔒 KEPT (Ki = " + PRECISION.fmt(ki_in) + ")";

        std::vector<std::string> free_g;
        if (!kp_lk) free_g.push_back("kp");
        if (!kd_lk) free_g.push_back("kd");
        if (!ki_lk) free_g.push_back("ki");

        bool has_i = (!ki_lk) || (std::abs(ki_in) > 1e-7);
        bool has_d = (!kd_lk) || (std::abs(kd_in) > 1e-7);
        out.controller_mode = (has_i && has_d) ? "PID" : (has_i ? "PI" : (has_d ? "PD" : "P"));

        double kp_out = kp_in, kd_out = kd_in, ki_out = ki_in;
        double zeta_out = target_zeta.value_or(0.456);
        double wn_out = target_wn.value_or(5.48);

        if (free_g == std::vector<std::string>{"kd"}) {
            // Keep Kp & Ki, solve Kd
            wn_out = PRECISION.step(std::sqrt(std::max(1e-9, a0_ol + k0_ol * kp_out)));
            if (target_zeta) {
                zeta_out = *target_zeta;
                kd_out = PRECISION.step((2.0 * zeta_out * wn_out - a1_ol) / k0_ol);
            } else if (target_sigma_d) {
                kd_out = PRECISION.step((2.0 * (*target_sigma_d) - a1_ol) / k0_ol);
            } else if (target_omega_d) {
                double sig_req = std::sqrt(std::max(0.0, wn_out * wn_out - (*target_omega_d) * (*target_omega_d)));
                kd_out = PRECISION.step((2.0 * sig_req - a1_ol) / k0_ol);
            }
            zeta_out = PRECISION.step((a1_ol + k0_ol * kd_out) / (2.0 * wn_out));
            out.notes["kd"] = "🔄 Solved with Kept Kp=" + PRECISION.fmt(kp_out);
            formulas.push_back("\\omega_n = \\sqrt{a_0 + K_0 K_p} = " + PRECISION.fmt(wn_out) +
                               ", \\quad K_d = \\frac{2\\zeta\\omega_n - a_1}{K_0} = " + PRECISION.fmt(kd_out));
        } else if (free_g == std::vector<std::string>{"kp"}) {
            // Keep Kd & Ki, solve Kp (covers P-control and PD with fixed Kd)
            double eff_a1 = PRECISION.step(a1_ol + k0_ol * kd_out);
            double sig_fixed = PRECISION.step(eff_a1 / 2.0);
            if (target_sigma_d && std::abs(*target_sigma_d - sig_fixed) > 0.05 * std::max(1.0, sig_fixed)) {
                conflicts.push_back("• Since Kd is KEPT at " + PRECISION.fmt(kd_out) + ", σd = (a1 + K0·Kd)/2 is locked at " +
                                    PRECISION.fmt(sig_fixed) + " s⁻¹ (Ts = " + PRECISION.fmt(4.0 / sig_fixed) + "s). Switch Kd to 🔄 CHANGE to modify Ts!");
            }
            if (target_zeta && *target_zeta > 1e-5) {
                wn_out = PRECISION.step(eff_a1 / (2.0 * (*target_zeta)));
                kp_out = PRECISION.step((wn_out * wn_out - a0_ol) / k0_ol);
                formulas.push_back("\\omega_n = \\frac{a_1 + K_0 K_d}{2\\zeta} = " + PRECISION.fmt(wn_out) +
                                   ", \\quad K_p = \\frac{\\omega_n^2 - a_0}{K_0} = " + PRECISION.fmt(kp_out));
            } else if (target_ess && *target_ess > 1e-7 && plant_integrators == 0) {
                kp_out = PRECISION.step((a0_ol / k0_ol) * (m_amp / (*target_ess) - 1.0));
                formulas.push_back("K_p = \\frac{a_0}{K_0}\\left(\\frac{M}{e_{\\mathrm{ss}}} - 1\\right) = " + PRECISION.fmt(kp_out));
            } else if (target_omega_d) {
                wn_out = PRECISION.step(std::hypot(sig_fixed, *target_omega_d));
                kp_out = PRECISION.step((wn_out * wn_out - a0_ol) / k0_ol);
                formulas.push_back("\\omega_n = \\sqrt{\\sigma_d^2 + \\omega_d^2} = " + PRECISION.fmt(wn_out) +
                                   ", \\quad K_p = \\frac{\\omega_n^2 - a_0}{K_0} = " + PRECISION.fmt(kp_out));
            } else if (target_wn) {
                wn_out = *target_wn;
                kp_out = PRECISION.step((wn_out * wn_out - a0_ol) / k0_ol);
            }
            wn_out = PRECISION.step(std::sqrt(std::max(1e-9, a0_ol + k0_ol * kp_out)));
            zeta_out = PRECISION.step(eff_a1 / (2.0 * wn_out));
            out.notes["kp"] = "🔄 Solved with Kept Kd=" + PRECISION.fmt(kd_out);
        } else if (free_g == std::vector<std::string>{"ki"}) {
            // Keep Kp & Kd, solve Ki (e.g. PI on 1st-order plant)
            double two_zw = PRECISION.step(a0_ol + k0_ol * kp_out);
            zeta_out = target_zeta.value_or(0.456);
            wn_out = PRECISION.step(two_zw / (2.0 * std::max(zeta_out, 1e-5)));
            ki_out = PRECISION.step((wn_out * wn_out) / k0_ol);
            out.notes["ki"] = "🔄 Solved with Kept Kp=" + PRECISION.fmt(kp_out);
            formulas.push_back("\\omega_n = \\frac{a_0 + K_0 K_p}{2\\zeta} = " + PRECISION.fmt(wn_out) +
                               ", \\quad K_i = \\frac{\\omega_n^2}{K_0} = " + PRECISION.fmt(ki_out));
        } else if (!kp_lk && !kd_lk && ki_lk) {
            // Both Kp and Kd are set to 🔄 CHANGE (PD synthesis)
            std::tie(zeta_out, wn_out) = resolveZetaWn();
            if (target_ess && *target_ess > 1e-7 && plant_integrators == 0) {
                kp_out = PRECISION.step((a0_ol / k0_ol) * (m_amp / (*target_ess) - 1.0));
                wn_out = PRECISION.step(std::sqrt(std::max(1e-9, a0_ol + k0_ol * kp_out)));
            } else {
                kp_out = PRECISION.step((wn_out * wn_out - a0_ol) / k0_ol);
            }
            kd_out = PRECISION.step((2.0 * zeta_out * wn_out - a1_ol) / k0_ol);
            out.notes["kp"] = "🔄 Solved from (ζ, ωn)";
            out.notes["kd"] = "🔄 Solved from (ζ, ωn)";
            formulas.push_back("K_p = \\frac{\\omega_n^2 - a_0}{K_0} = " + PRECISION.fmt(kp_out) +
                               ", \\quad K_d = \\frac{2\\zeta\\omega_n - a_1}{K_0} = " + PRECISION.fmt(kd_out));
        } else {
            // Full PID 3-DOF pole placement (p3 = 5 * sigma_d)
            std::tie(zeta_out, wn_out) = resolveZetaWn();
            double sig_d = std::max(zeta_out * wn_out, 0.2 * wn_out);
            double p3 = 5.0 * sig_d;
            if (!kd_lk) kd_out = PRECISION.step((2.0 * zeta_out * wn_out + p3 - a1_ol) / k0_ol);
            if (!kp_lk) kp_out = PRECISION.step((wn_out * wn_out + 2.0 * zeta_out * wn_out * p3 - a0_ol) / k0_ol);
            if (!ki_lk) ki_out = PRECISION.step((wn_out * wn_out * p3) / k0_ol);
            formulas.push_back("p_3 = 5\\sigma_d = " + PRECISION.fmt(p3) +
                               " \\Rightarrow K_p = " + PRECISION.fmt(kp_out) +
                               ", \\; K_i = " + PRECISION.fmt(ki_out) +
                               ", \\; K_d = " + PRECISION.fmt(kd_out));
        }

        double sig_res = PRECISION.step(zeta_out * wn_out);
        double wd_res  = (zeta_out < 1.0) ? PRECISION.step(wn_out * std::sqrt(std::max(0.0, 1.0 - zeta_out * zeta_out))) : 0.0;
        double os_res  = (zeta_out < 1.0) ? PRECISION.step(niseOsFromZeta(zeta_out)) : 0.0;
        double ts_res  = (sig_res > 1e-9) ? PRECISION.step(4.0 / sig_res) : std::numeric_limits<double>::infinity();
        double tp_res  = (wd_res > 1e-9)  ? PRECISION.step(PI / wd_res)   : std::numeric_limits<double>::infinity();
        double tr_res  = PRECISION.step(niseTrFromZetaWn(std::clamp(zeta_out, 0.0, 1.2), wn_out));

        double ess_res = 0.0, rel_res = 0.0;
        if (std::abs(ki_out) <= 1e-9 && plant_integrators == 0) {
            double kp_pos = kp_out * (k0_ol / std::max(a0_ol, 1e-12));
            ess_res = PRECISION.step(m_amp / std::max(1.0 + kp_pos, 1e-9));
            rel_res = PRECISION.step((ess_res / m_amp) * 100.0);
        }

        out.solved = {
            {"kp", kp_out}, {"kd", kd_out}, {"ki", ki_out},
            {"os_pct", os_res}, {"zeta", zeta_out}, {"wn", wn_out},
            {"ts", ts_res}, {"tp", tp_res}, {"tr", tr_res},
            {"sigma_d", sig_res}, {"omega_d", wd_res},
            {"ess", ess_res}, {"rel_err_pct", rel_res}
        };
        for (const auto& m : metaList()) {
            if (!out.notes.count(m.key)) out.notes[m.key] = "🔄 Changed (Computed from Kept Specs)";
        }

        out.feasible = conflicts.empty();
        out.headline = out.feasible
            ? ("✔ POSSIBLE & SOLVED (" + out.controller_mode + "-Control):  Kp=" + PRECISION.fmt(kp_out) +
               ", Kd=" + PRECISION.fmt(kd_out) + ", Ki=" + PRECISION.fmt(ki_out) +
               "  •  %OS=" + PRECISION.fmt(os_res) + "%, Ts=" + PRECISION.fmt(ts_res) + "s, S.S.E.=" + PRECISION.fmt(ess_res))
            : ("✖ CONFLICT DETECTED (" + out.controller_mode + "-Control)");

        out.explanation.clear();
        for (const auto& c : conflicts) out.explanation += (out.explanation.empty() ? "" : "\n") + c;
        if (out.explanation.empty()) {
            out.explanation = "All 🔄 CHANGE parameters have been solved from your 🔒 KEEP specifications. Click 'Design Controller' to apply.";
        }

        out.formulas_latex.clear();
        for (size_t i = 0; i < formulas.size(); ++i) {
            out.formulas_latex += (i ? " \\quad | \\quad " : "") + formulas[i];
        }
        if (out.formulas_latex.empty()) {
            out.formulas_latex = "\\zeta = " + PRECISION.fmt(zeta_out) + ", \\; \\omega_n = " + PRECISION.fmt(wn_out) +
                                 ", \\; K_p = " + PRECISION.fmt(kp_out) + ", \\; K_d = " + PRECISION.fmt(kd_out);
        }
        return out;
    }
};

class DesignParametersDialog : public QDialog {
    Q_OBJECT
public:
    SharedSystemModel* model;
    std::map<std::string, bool>* lock_map;
    std::map<std::string, double>* input_map;
    std::function<void(const ConstraintMatrixAnalysis&)> onApplyDesign;
    std::function<void(double)> onMaxKChanged;

    std::map<std::string, QPushButton*> lock_btns;
    std::map<std::string, QLineEdit*> val_edits;
    std::map<std::string, QLabel*> solved_lbls;
    std::map<std::string, QLabel*> note_lbls;
    QLineEdit* max_k_edit;

    QFrame* verdict_banner;
    QLabel *verdict_title, *verdict_desc;
    MathFormulaWidget* formula_math_w;
    ConstraintMatrixAnalysis last_analysis;
    bool updating = false;

    DesignParametersDialog(SharedSystemModel* m,
                           std::map<std::string, bool>* lk,
                           std::map<std::string, double>* in,
                           std::function<void(const ConstraintMatrixAnalysis&)> apply_cb,
                           std::function<void(double)> maxk_cb,
                           QWidget* parent = nullptr)
        : QDialog(parent), model(m), lock_map(lk), input_map(in),
          onApplyDesign(std::move(apply_cb)), onMaxKChanged(std::move(maxk_cb))
    {
        setWindowTitle("Design Parameters & Keep (🔒) / Change (🔄) Controller Synthesizer");
        resize(960, 740);
        setStyleSheet(QString("QDialog { background-color: %1; }").arg(Theme::BG_WINDOW));

        auto* main_l = new QVBoxLayout(this);
        main_l->setContentsMargins(14, 10, 14, 12);
        main_l->setSpacing(8);

        // Top Header & K-Slider Endpoint Setting
        auto* hdr_card = makeCard();
        auto* hdr_l = new QHBoxLayout(hdr_card);
        auto* title_v = new QVBoxLayout();
        auto* t1 = new QLabel("Controller Design Parameters  •  Keep (🔒) vs. Change (🔄) Matrix");
        t1->setStyleSheet(QString("color: %1; font-size: 13px; font-weight: bold; border:none;").arg(Theme::TEXT_PRIMARY));
        auto* t2 = new QLabel("Lock (🔒 KEEP) the specifications and gains you want held fixed, and set the parameters to solve for to 🔄 CHANGE.");
        t2->setStyleSheet(QString("color: %1; font-size: 10px; border:none;").arg(Theme::ACCENT_ZERO));
        title_v->addWidget(t1); title_v->addWidget(t2);
        hdr_l->addLayout(title_v, 1);

        // Variable K-Slider Endpoint Control
        auto* kmax_box = new QHBoxLayout();
        auto* kmax_lbl = new QLabel("Max K-Slider Endpoint:");
        kmax_lbl->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border:none;").arg(Theme::ACCENT_PID));
        max_k_edit = makeDarkEntry(QString::number(model->max_k_slider, 'g', 5), 70);
        kmax_box->addWidget(kmax_lbl);
        kmax_box->addWidget(max_k_edit);
        hdr_l->addLayout(kmax_box);
        main_l->addWidget(hdr_card);

        // Quick Lock Pattern Presets + Apply Button Bar
        auto* preset_card = makeCard();
        auto* pr_l = new QHBoxLayout(preset_card);
        auto* pr_lbl = new QLabel("Quick Setup:");
        pr_lbl->setStyleSheet("color: #94a3b8; font-weight: bold; font-size: 10px; border:none;");
        pr_l->addWidget(pr_lbl);

        for (const auto& [btn_txt, pat] : std::vector<std::pair<QString, std::string>>{
            {"P: Change Kp (%OS)", "p_os"},
            {"PD: Keep Kp, Change Kd (%OS)", "pd_kd"},
            {"PD: Keep Kd, Change Kp (%OS)", "pd_kp"},
            {"PD: Change Kp & Kd (%OS + Ts)", "pd_both"},
            {"PI: Keep Kp, Change Ki (%OS)", "pi_ki"},
            {"PID: Change Kp, Ki, Kd (%OS + Ts)", "pid_all"}
        }) {
            auto* b = makeActionBtn(btn_txt, Theme::TEXT_PRIMARY);
            connect(b, &QPushButton::clicked, this, [this, pat]() { applyQuickPattern(pat); });
            pr_l->addWidget(b);
        }
        pr_l->addStretch();
        auto* design_btn = makeActionBtn("✔ Design & Apply Controller ➔", "#0b0e14", Theme::ACCENT_PID);
        pr_l->addWidget(design_btn);
        main_l->addWidget(preset_card);

        // Parameter Table
        auto* tbl_scroll = new QScrollArea();
        tbl_scroll->setWidgetResizable(true);
        tbl_scroll->setStyleSheet(QString("background-color: %1; border: 1px solid %2; border-radius: 6px;").arg(Theme::BG_PANEL, Theme::BORDER_PANEL));
        auto* tbl_w = new QWidget();
        auto* grid = new QGridLayout(tbl_w);
        grid->setContentsMargins(10, 8, 10, 8);
        grid->setVerticalSpacing(4);
        grid->setHorizontalSpacing(10);

        const std::vector<QString> headers = {"Click to Toggle", "Parameter", "Target / Kept Value", "Solved Value", "Role & Status"};
        for (int c = 0; c < static_cast<int>(headers.size()); ++c) {
            auto* hl = new QLabel(headers[c]);
            hl->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border:none;").arg(Theme::ACCENT_ZERO));
            grid->addWidget(hl, 0, c);
        }

        const auto& metas = ParameterMatrixEngine::metaList();
        for (int r = 0; r < static_cast<int>(metas.size()); ++r) {
            const auto& m = metas[r];
            bool is_lk = (*lock_map)[m.key];
            auto* lk_b = new QPushButton(is_lk ? "🔒 KEEP" : "🔄 CHANGE");
            lk_b->setCursor(Qt::PointingHandCursor);
            lk_b->setFixedWidth(105);
            lock_btns[m.key] = lk_b;

            QString unit_s = m.unit.isEmpty() ? "" : QString(" (%1)").arg(m.unit);
            auto* name_l = new QLabel(m.name + unit_s);
            name_l->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border:none;").arg(Theme::TEXT_PRIMARY));
            name_l->setToolTip(m.desc);

            auto* ed = makeDarkEntry(QString::number((*input_map)[m.key], 'g', 5), 95);
            val_edits[m.key] = ed;

            auto* sol_l = new QLabel("—");
            sol_l->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; font-size: 11px; border:none;").arg(Theme::ACCENT_ZERO));
            solved_lbls[m.key] = sol_l;

            auto* note_l = new QLabel("—");
            note_l->setStyleSheet("color: #94a3b8; font-size: 10px; border:none;");
            note_lbls[m.key] = note_l;

            grid->addWidget(lk_b,   r + 1, 0);
            grid->addWidget(name_l, r + 1, 1);
            grid->addWidget(ed,     r + 1, 2);
            grid->addWidget(sol_l,  r + 1, 3);
            grid->addWidget(note_l, r + 1, 4);

            std::string key_cp = m.key;
            connect(lk_b, &QPushButton::clicked, this, [this, key_cp]() {
                (*lock_map)[key_cp] = !(*lock_map)[key_cp];
                refreshEvaluation();
            });
            connect(ed, &QLineEdit::textEdited, this, [this, key_cp, ed]() {
                if (updating) return;
                (*input_map)[key_cp] = parseNumericInput(ed->text().toStdString(), 0.0);
                refreshEvaluation();
            });
        }
        tbl_scroll->setWidget(tbl_w);
        main_l->addWidget(tbl_scroll, 1);

        // Bottom Verdict & LaTeX Formulas Used Banner
        verdict_banner = new QFrame();
        auto* vb_l = new QVBoxLayout(verdict_banner);
        vb_l->setContentsMargins(12, 8, 12, 8);
        verdict_title = new QLabel("✔ POSSIBLE & SOLVED");
        verdict_desc  = new QLabel();
        verdict_desc->setWordWrap(true);
        auto* f_hdr = new QLabel("Formulas Used for Calculation:");
        f_hdr->setStyleSheet("color: #94a3b8; font-size: 10px; font-weight: bold; border: none;");
        formula_math_w = new MathFormulaWidget("", QColor(Theme::ACCENT_ZERO), QColor(Theme::BG_INPUT), 11);

        vb_l->addWidget(verdict_title);
        vb_l->addWidget(verdict_desc);
        vb_l->addWidget(f_hdr);
        vb_l->addWidget(formula_math_w);
        main_l->addWidget(verdict_banner);

        connect(max_k_edit, &QLineEdit::editingFinished, this, [this]() {
            double mk = std::max(1.0, parseNumericInput(max_k_edit->text().toStdString(), 100.0));
            model->max_k_slider = mk;
            if (onMaxKChanged) onMaxKChanged(mk);
        });

        connect(design_btn, &QPushButton::clicked, this, [this]() {
            double mk = std::max(1.0, parseNumericInput(max_k_edit->text().toStdString(), 100.0));
            model->max_k_slider = mk;
            if (onMaxKChanged) onMaxKChanged(mk);
            refreshEvaluation();
            if (onApplyDesign) onApplyDesign(last_analysis);
        });

        refreshEvaluation();
    }

    void applyQuickPattern(const std::string& pat) {
        for (auto& [k, v] : *lock_map) v = false;
        (*input_map)["kp"] = model->kp;
        (*input_map)["kd"] = model->kd;
        (*input_map)["ki"] = model->ki;

        if (pat == "p_os") {
            (*input_map)["kd"] = 0.0; (*input_map)["ki"] = 0.0;
            (*lock_map)["kd"] = true; (*lock_map)["ki"] = true; (*lock_map)["os_pct"] = true;
        } else if (pat == "pd_kd") {
            (*input_map)["ki"] = 0.0;
            (*lock_map)["kp"] = true; (*lock_map)["ki"] = true; (*lock_map)["os_pct"] = true;
        } else if (pat == "pd_kp") {
            (*input_map)["ki"] = 0.0;
            (*lock_map)["kd"] = true; (*lock_map)["ki"] = true; (*lock_map)["os_pct"] = true;
        } else if (pat == "pd_both") {
            (*input_map)["ki"] = 0.0;
            (*lock_map)["ki"] = true; (*lock_map)["os_pct"] = true; (*lock_map)["ts"] = true;
        } else if (pat == "pi_ki") {
            (*input_map)["kd"] = 0.0;
            (*lock_map)["kp"] = true; (*lock_map)["kd"] = true; (*lock_map)["os_pct"] = true;
        } else if (pat == "pid_all") {
            (*lock_map)["os_pct"] = true; (*lock_map)["ts"] = true;
        }
        updating = true;
        for (auto& [k, ed] : val_edits) ed->setText(QString::number((*input_map)[k], 'g', 5));
        updating = false;
        refreshEvaluation();
    }

    void refreshEvaluation() {
        last_analysis = ParameterMatrixEngine::evaluate(*lock_map, *input_map, model);
        for (auto& [k, btn] : lock_btns) {
            bool lk = (*lock_map)[k];
            btn->setText(lk ? "🔒 KEEP" : "🔄 CHANGE");
            btn->setStyleSheet(QString(
                "QPushButton { background-color: %1; color: %2; border: 1px solid %3; border-radius: 4px; padding: 4px 8px; font-family: Monospace; font-weight: bold; font-size: 10px; }"
            ).arg(lk ? Theme::ACCENT_PID : Theme::BG_KEY_ACCENT,
                  lk ? "#0b0e14" : "#ffffff",
                  Theme::BORDER_INPUT));
        }
        for (auto& [k, sl] : solved_lbls) {
            double v = last_analysis.solved[k];
            sl->setText(QString::fromStdString(PRECISION.fmt(v)));
            sl->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; font-size: 11px; border:none;")
                              .arg((*lock_map)[k] ? Theme::ACCENT_PID : Theme::ACCENT_ZERO));
        }
        for (auto& [k, nl] : note_lbls) {
            nl->setText(QString::fromStdString(last_analysis.notes[k]));
        }

        QString bg = last_analysis.feasible ? Theme::BANNER_OK_BG : Theme::BANNER_WARN_BG;
        QString bd = last_analysis.feasible ? Theme::BANNER_OK_BD : Theme::BANNER_WARN_BD;
        QString fg = last_analysis.feasible ? Theme::BANNER_OK_FG : Theme::BANNER_WARN_FG;

        verdict_banner->setStyleSheet(QString("QFrame { background-color: %1; border: 1.5px solid %2; border-radius: 6px; }").arg(bg, bd));
        verdict_title->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 12px; border:none;").arg(fg));
        verdict_desc->setStyleSheet(QString("color: %1; font-size: 10px; border:none;").arg(last_analysis.feasible ? "#a7f3d0" : "#fde68a"));

        verdict_title->setText(QString::fromStdString(last_analysis.headline));
        verdict_desc->setText(QString::fromStdString(last_analysis.explanation));
        formula_math_w->setLatex(QString::fromStdString(last_analysis.formulas_latex));
    }
};

