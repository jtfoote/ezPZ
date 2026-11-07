#pragma once
#include "FrequencyResponseTab.h"


// =============================================================================
// Engine 8 & Tab 9: Routh-Hurwitz Array Constructor, Special Cases & K-Solver
// =============================================================================
struct RouthRow {
    std::string label;           // e.g. "s^4", "s^3", ..., "s^0"
    int degree = 0;
    std::vector<double> vals;
    std::string special_note;    // "", "Case 1 (0 -> eps)", "Case 2 (Row of 0s -> dA/ds)"
    std::vector<std::string> sym_vals; // Symbolic formula in K for each cell
    bool sign_changed = false;
};

struct RouthAnalysisResult {
    std::vector<double> coeffs;  // Descending powers: a_n s^n + ... + a_0
    std::vector<RouthRow> rows;
    int sign_changes = 0;
    int rhp_poles = 0;
    int lhp_poles = 0;
    int jw_poles = 0;
    bool has_eps_case = false;
    bool has_aux_case = false;
    std::string aux_poly_latex;
    std::string aux_deriv_latex;
    std::vector<Complex> exact_roots;
    std::vector<Complex> jw_roots;

    // Symbolic K-sweep results for D_0(s) + K * D_K(s) = 0
    bool has_k_analysis = false;
    std::string k_range_summary;
    std::vector<std::pair<double, double>> k_crit_w_pairs; // (K_crit, w_osc)
    std::string symbolic_col1_latex;
};

class RouthHurwitzEngine {
public:
    // Parse a polynomial in s that may also depend linearly on K, e.g.:
    // "s^4 + 6s^3 + 21s^2 + (26 + K)s + 6K" -> returns P(s; K) = P0(s) + K * PK(s)
    static std::pair<std::vector<double>, std::vector<double>> parseSymbolicSPoly(const std::string& expr_raw) {
        auto evalAtK = [&](double k_val) -> std::vector<double> {
            std::string sub;
            for (size_t i = 0; i < expr_raw.size(); ++i) {
                char c = expr_raw[i];
                if (c == 'K' || c == 'k') {
                    bool prev_num_or_par = (i > 0) && (std::isdigit(static_cast<unsigned char>(expr_raw[i - 1])) || expr_raw[i - 1] == ')' || expr_raw[i - 1] == '.');
                    if (prev_num_or_par) sub += "*";
                    std::ostringstream oss;
                    oss << std::fixed << std::setprecision(6) << k_val;
                    sub += oss.str();
                    bool next_alnum = (i + 1 < expr_raw.size()) && (std::isalnum(static_cast<unsigned char>(expr_raw[i + 1])) || expr_raw[i + 1] == '(');
                    if (next_alnum) sub += "*";
                } else {
                    sub.push_back(c);
                }
            }
            auto parsed = parsePolynomialInput(sub);
            std::vector<double> out;
            for (const auto& c : parsed.coeffs) out.push_back(c.real());
            return out;
        };

        std::vector<double> p0 = evalAtK(0.0);
        std::vector<double> p1 = evalAtK(1.0);
        size_t n = std::max(p0.size(), p1.size());
        std::vector<double> p0_pad(n, 0.0), pk_pad(n, 0.0);
        for (size_t i = 0; i < p0.size(); ++i) p0_pad[n - p0.size() + i] = p0[i];
        for (size_t i = 0; i < p1.size(); ++i) pk_pad[n - p1.size() + i] = p1[i] - p0_pad[n - p1.size() + i];
        return {p0_pad, pk_pad};
    }

    static std::string formatPolyLatexFromRealCoeffs(const std::vector<double>& c, const std::string& var = "s") {
        int deg = static_cast<int>(c.size()) - 1;
        if (deg < 0) return "0";
        std::string out;
        for (int i = 0; i <= deg; ++i) {
            double v = c[i];
            int p = deg - i;
            if (std::abs(v) < 1e-10) continue;
            bool neg = (v < 0.0);
            double av = std::abs(v);
            if (!out.empty()) out += neg ? " - " : " + ";
            else if (neg) out += "-";

            bool show_num = (p == 0) || (std::abs(av - 1.0) > 1e-7);
            if (show_num) out += PRECISION.fmt(av);
            if (p > 0) {
                out += var;
                if (p > 1) out += "^{" + std::to_string(p) + "}";
            }
        }
        return out.empty() ? "0" : out;
    }

    static RouthAnalysisResult buildRouthArray(const std::vector<double>& raw_coeffs) {
        RouthAnalysisResult res;
        // Strip leading near-zero coefficients
        size_t start_idx = 0;
        while (start_idx + 1 < raw_coeffs.size() && std::abs(raw_coeffs[start_idx]) < 1e-12) ++start_idx;
        res.coeffs.assign(raw_coeffs.begin() + start_idx, raw_coeffs.end());

        int n = static_cast<int>(res.coeffs.size()) - 1;
        if (n < 0) return res;

        Poly c_poly;
        for (double v : res.coeffs) c_poly.emplace_back(v, 0.0);
        res.exact_roots = solvePolynomialRoots(c_poly);

        for (const auto& rt : res.exact_roots) {
            if ( std::abs(rt.real()) <= 1e-4 ) {
                res.jw_poles++;
                res.jw_roots.push_back(rt);
            } else if (rt.real() > 1e-4) {
                res.rhp_poles++;
            } else {
                res.lhp_poles++;
            }
        }

        int num_cols = (n + 2) / 2;
        res.rows.resize(n + 1);
        for (int r = 0; r <= n; ++r) {
            int deg = n - r;
            res.rows[r].degree = deg;
            res.rows[r].label = "s^" + std::to_string(deg);
            res.rows[r].vals.assign(num_cols, 0.0);
        }

        // Populate top two rows (s^n and s^{n-1})
        for (int i = 0; i <= n; ++i) {
            int r = i % 2;
            int c = i / 2;
            if (r <= n && c < num_cols) res.rows[r].vals[c] = res.coeffs[i];
        }

        const double EPS_SUB = 1e-5;

        // Compute subsequent rows s^{n-2} down to s^0
        for (int r = 1; r < n; ++r) {
            // Check if current row r is an ENTIRE ROW OF ZEROS (Case 2)
            bool all_zero_r = true;
            for (double v : res.rows[r].vals) {
                if (std::abs(v) > 1e-8) { all_zero_r = false; break; }
            }

            if (all_zero_r) {
                res.has_aux_case = true;
                int aux_deg = res.rows[r - 1].degree;
                res.rows[r].special_note = "Case 2: Row of Zeros -> Replaced by dA(s)/ds";

                std::vector<double> aux_full(aux_deg + 1, 0.0);
                for (int c = 0; c < num_cols; ++c) {
                    int p = aux_deg - 2 * c;
                    if (p >= 0) {
                        aux_full[aux_deg - p] = res.rows[r - 1].vals[c];
                        res.rows[r].vals[c] = res.rows[r - 1].vals[c] * double(p);
                    }
                }
                if (res.aux_poly_latex.empty()) {
                    res.aux_poly_latex = "A(s) = " + formatPolyLatexFromRealCoeffs(aux_full, "s") + " = 0";
                    std::vector<double> d_aux( std::max(1, aux_deg), 0.0 );
                    for (int p = aux_deg; p >= 1; --p) {
                        d_aux[aux_deg - p] = aux_full[aux_deg - p] * double(p);
                    }
                    res.aux_deriv_latex = "\\frac{dA(s)}{ds} = " + formatPolyLatexFromRealCoeffs(d_aux, "s");
                }
            } else if (std::abs(res.rows[r].vals[0]) < 1e-8) {
                // Case 1: First-column element is zero, but rest of row has non-zero elements
                res.has_eps_case = true;
                res.rows[r].vals[0] = EPS_SUB;
                res.rows[r].special_note = "Case 1: First Col 0 -> Replaced by +ε";
            }

            // Now compute row r + 1 using standard Routh determinant rule
            double piv = res.rows[r].vals[0];
            if (std::abs(piv) < 1e-12) piv = EPS_SUB;

            for (int c = 0; c + 1 < num_cols; ++c) {
                double u0 = res.rows[r - 1].vals[0];
                double u1 = res.rows[r - 1].vals[c + 1];
                double l1 = res.rows[r].vals[c + 1];
                double val = (piv * u1 - u0 * l1) / piv;
                if (std::abs(val) < 1e-9) val = 0.0;
                res.rows[r + 1].vals[c] = val;
            }
        }

        // Check final row s^0 for Case 2 / Case 1
        if (n >= 1 && std::abs(res.rows[n].vals[0]) < 1e-8) {
            if (res.rows[n - 1].degree == 1) {
                res.has_aux_case = true;
                res.rows[n].special_note = "s=0 Root (Constant Term a_0 = 0)";
            }
        }

        // Count first-column sign changes
        int prev_sgn = 0;
        for (int r = 0; r <= n; ++r) {
            double v = res.rows[r].vals[0];
            int sgn = (v > 1e-11) ? 1 : ((v < -1e-11) ? -1 : 0);
            if (sgn != 0) {
                if (prev_sgn != 0 && sgn != prev_sgn) {
                    res.sign_changes++;
                    res.rows[r].sign_changed = true;
                }
                prev_sgn = sgn;
            }
        }

        return res;
    }

    // Solve for K stability intervals and marginal K_crit values for P0(s) + K * PK(s) = 0
    static void analyzeSymbolicK(
        const std::vector<double>& p0,
        const std::vector<double>& pk,
        RouthAnalysisResult& out)
    {
        bool has_k = false;
        for (double v : pk) if (std::abs(v) > 1e-10) { has_k = true; break; }
        if (!has_k || p0.size() != pk.size()) return;
        out.has_k_analysis = true;

        auto evalPolyAtK = [&](double k) {
            std::vector<double> c(p0.size());
            for (size_t i = 0; i < p0.size(); ++i) c[i] = p0[i] + k * pk[i];
            return c;
        };

        auto isStableAtK = [&](double k) -> bool {
            auto c = evalPolyAtK(k);
            if (c.empty() || std::abs(c[0]) < 1e-10) return false;
            Poly cp;
            for (double v : c) cp.emplace_back(v, 0.0);
            auto rts = solvePolynomialRoots(cp);
            for (const auto& r : rts) if (r.real() >= -1e-5) return false;
            return true;
        };

        // Build non-uniform K test grid covering negative and positive K across 7 decades
        std::vector<double> k_grid;
        k_grid.push_back(0.0);
        for (int i = -160; i <= 220; ++i) {
            double exp_v = -3.0 + 6.5 * double(i + 160) / 380.0;
            double mag = std::pow(10.0, exp_v);
            k_grid.push_back(mag);
            if (i % 2 == 0) k_grid.push_back(-mag);
        }
        std::sort(k_grid.begin(), k_grid.end());

        std::vector<std::pair<double, double>> stable_intervals;
        bool prev_st = isStableAtK(k_grid[0]);
        double int_start = prev_st ? k_grid[0] : 0.0;

        for (size_t i = 1; i < k_grid.size(); ++i) {
            double k1 = k_grid[i - 1], k2 = k_grid[i];
            bool st2 = isStableAtK(k2);
            if (st2 != prev_st) {
                // Bisect in [k1, k2] to find exact K_crit boundary
                double lo = k1, hi = k2;
                bool st_lo = prev_st;
                for (int it = 0; it < 42; ++it) {
                    double mid = 0.5 * (lo + hi);
                    if (isStableAtK(mid) == st_lo) lo = mid;
                    else hi = mid;
                }
                double k_crit = 0.5 * (lo + hi);

                // Find corresponding jw oscillation frequency at k_crit
                Poly cp;
                for (double v : evalPolyAtK(k_crit)) cp.emplace_back(v, 0.0);
                auto rts = solvePolynomialRoots(cp);
                double best_jw_re = 1e9, w_osc = 0.0;
                for (const auto& rt : rts) {
                    if (std::abs(rt.real()) < best_jw_re) {
                        best_jw_re = std::abs(rt.real());
                        w_osc = std::abs(rt.imag());
                    }
                }
                if (best_jw_re < 1e-2) {
                    out.k_crit_w_pairs.push_back({k_crit, w_osc});
                }

                if (!prev_st && st2) {
                    int_start = k_crit;
                } else if (prev_st && !st2) {
                    stable_intervals.push_back({int_start, k_crit});
                }
                prev_st = st2;
            }
        }
        if (prev_st) {
            stable_intervals.push_back({int_start, std::numeric_limits<double>::infinity()});
        }

        if (stable_intervals.empty()) {
            out.k_range_summary = "No stable range of K found (Unstable for all K)";
        } else {
            std::string s_int;
            for (size_t i = 0; i < stable_intervals.size(); ++i) {
                double a = stable_intervals[i].first;
                double b = stable_intervals[i].second;
                std::string a_str = (a < -1e5) ? "-∞" : (std::abs(a) < 1e-4 ? "0" : PRECISION.fmt(a));
                std::string b_str = std::isinf(b) ? "+∞" : (std::abs(b) < 1e-4 ? "0" : PRECISION.fmt(b));
                s_int += (i ? "  ∪  " : "") + a_str + " < K < " + b_str;
            }
            out.k_range_summary = "Stable Range:  " + s_int;
        }
    }

    static void populateSymbolicRouthCells(
        const std::vector<double>& p0,
        const std::vector<double>& pk,
        RouthAnalysisResult& res)
    {
        bool has_k = false;
        for (double v : pk) if (std::abs(v) > 1e-10) { has_k = true; break; }
        int n = static_cast<int>(p0.size()) - 1;
        if (n < 0 || static_cast<int>(res.rows.size()) != n + 1) return;

        int num_cols = (n + 2) / 2;
        std::vector<std::vector<SymRat>> srows(n + 1, std::vector<SymRat>(num_cols, SymRat(0.0)));

        for (int i = 0; i <= n; ++i) {
            int r = i % 2, c = i / 2;
            Poly cell_p = {Complex(pk[i], 0.0), Complex(p0[i], 0.0)}; // pk*K + p0
            srows[r][c] = SymRat(cell_p);
        }

        for (int r = 1; r < n; ++r) {
            SymRat piv = srows[r][0];
            if (piv.isZero()) piv = SymRat(1e-5); // Case 1 epsilon fallback
            for (int c = 0; c + 1 < num_cols; ++c) {
                SymRat u0 = srows[r - 1][0];
                SymRat u1 = srows[r - 1][c + 1];
                SymRat l1 = srows[r][c + 1];
                srows[r + 1][c] = (piv * u1 - u0 * l1) / piv;
            }
        }

        for (int r = 0; r <= n; ++r) {
            res.rows[r].sym_vals.resize(num_cols);
            for (int c = 0; c < num_cols; ++c) {
                if (has_k && !srows[r][c].isConstant()) {
                    res.rows[r].sym_vals[c] = srows[r][c].toString("K");
                } else {
                    res.rows[r].sym_vals[c] = "";
                }
            }
        }
    }
};

class RouthHurwitzTab : public QWidget {
    Q_OBJECT
public:
    SharedSystemModel* model;
    QComboBox* source_combo;
    QLineEdit *custom_poly_edit, *k_eval_edit;
    MathFormulaWidget *char_eq_math_w, *aux_math_w;

    QFrame* verdict_banner;
    QLabel *banner_title, *banner_right, *banner_sub;
    QLabel *stat_rhp, *stat_lhp, *stat_jw, *stat_krange;

    QWidget* table_container;
    QGridLayout* routh_grid;
    QLabel* roots_list_lbl;
    bool updating_ui = false;

    explicit RouthHurwitzTab(SharedSystemModel* m, QWidget* parent = nullptr)
        : QWidget(parent), model(m)
    {
        auto* root_l = new QHBoxLayout(this);
        root_l->setContentsMargins(0, 0, 0, 0);
        auto* splitter = new QSplitter(Qt::Horizontal, this);
        root_l->addWidget(splitter);

        // Left Control & Telemetry Column
        auto* left_w = new QWidget();
        auto* left_v = new QVBoxLayout(left_w);
        left_v->setContentsMargins(0, 0, 6, 0);
        left_v->setSpacing(6);

        // 1. Characteristic Polynomial Input & Mode Card
        auto* cfg_card = makeCard();
        auto* cl = new QVBoxLayout(cfg_card);
        cl->setContentsMargins(10, 8, 10, 8);
        cl->setSpacing(5);

        auto* t1 = new QLabel("1. Characteristic Equation  1 + K·G(s)H(s) = 0  or  Custom P(s; K) = 0");
        t1->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));
        cl->addWidget(t1);

        source_combo = new QComboBox();
        source_combo->addItems({
            "Synced Closed-Loop:  Den(s) + K · Num(s) = 0  (from Plant G(s)H(s))",
            "Synced Active PID Loop:  1 + C_PID(s)G(s)H(s) = 0",
            "Custom Symbolic Characteristic Polynomial P(s; K) = 0"
        });
        source_combo->setStyleSheet(QString("background-color: %1; color: %2; padding: 4px; font-weight: bold; font-size: 11px;")
                                    .arg(Theme::BG_INPUT, Theme::TEXT_PRIMARY));
        cl->addWidget(source_combo);

        auto* p_row = new QHBoxLayout();
        auto* p_lbl = new QLabel("P(s; K) =");
        p_lbl->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; border:none;").arg(Theme::ACCENT_PID));
        custom_poly_edit = makeDarkEntry("s^4 + 6s^3 + 21s^2 + (26 + K)s + 6K");
        auto* k_lbl = new QLabel("Eval @ K =");
        k_lbl->setStyleSheet("color: #94a3b8; font-family: Monospace; font-weight: bold; border:none;");
        k_eval_edit = makeDarkEntry("10.0", 60);
        auto* solve_btn = makeActionBtn("Build Routh Table ➔", "#0b0e14", Theme::ACCENT_PID);

        p_row->addWidget(p_lbl);
        p_row->addWidget(custom_poly_edit, 1);
        p_row->addWidget(k_lbl);
        p_row->addWidget(k_eval_edit);
        p_row->addWidget(solve_btn);
        cl->addLayout(p_row);

        // Quick Textbook Presets Row (Normal, Case 1: Zero in Col 1, Case 2: Row of Zeros)
        auto* pr_row = new QHBoxLayout();
        auto* pr_lbl = new QLabel("Presets:");
        pr_lbl->setStyleSheet("color: #64748b; font-size: 10px; font-weight: bold; border:none;");
        pr_row->addWidget(pr_lbl);

        for (const auto& [b_txt, poly_str, k_str] : std::vector<std::tuple<QString, QString, QString>>{
            {"K-Range (4th Order)", "s^4 + 6s^3 + 21s^2 + (26 + K)s + 6K", "16.41"},
            {"Case 1: 0 in Col 1 (ε)", "s^5 + 2s^4 + 2s^3 + 4s^2 + 11s + 10", "1.0"},
            {"Case 2: Row of Zeros (A(s))", "s^5 + 2s^4 + 24s^3 + 48s^2 - 25s - 50", "1.0"},
            {"Sustained jω Osc", "s^3 + 2s^2 + 4s + 8", "1.0"}
        }) {
            auto* b = makeActionBtn(b_txt, Theme::TEXT_PRIMARY, Theme::BG_PANEL_ALT);
            connect(b, &QPushButton::clicked, this, [this, poly_str, k_str]() {
                updating_ui = true;
                source_combo->setCurrentIndex(2);
                custom_poly_edit->setText(poly_str);
                k_eval_edit->setText(k_str);
                updating_ui = false;
                evaluateCurrent();
            });
            pr_row->addWidget(b);
        }
        pr_row->addStretch();
        cl->addLayout(pr_row);

        char_eq_math_w = new MathFormulaWidget("", QColor(Theme::ACCENT_ZERO), QColor(Theme::BG_INPUT), 10);
        cl->addWidget(char_eq_math_w);
        left_v->addWidget(cfg_card);

        // 2. Stability Verdict & K-Range Telemetry Card
        auto* tel_card = makeCard();
        auto* tl = new QVBoxLayout(tel_card);
        tl->setContentsMargins(10, 8, 10, 8);
        tl->setSpacing(6);

        auto* t2 = new QLabel("2. Routh-Hurwitz Stability Verdict, Pole Distribution & K-Limits");
        t2->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::TEXT_PRIMARY));
        tl->addWidget(t2);

        verdict_banner = new QFrame();
        auto* bl = new QVBoxLayout(verdict_banner);
        bl->setContentsMargins(10, 6, 10, 6);
        auto* btop = new QHBoxLayout();
        banner_title = new QLabel("✔ ASYMPTOTICALLY STABLE");
        banner_right = new QLabel("0 Sign Changes");
        btop->addWidget(banner_title); btop->addStretch(); btop->addWidget(banner_right);
        banner_sub = new QLabel();
        banner_sub->setWordWrap(true);
        bl->addLayout(btop);
        bl->addWidget(banner_sub);
        tl->addWidget(verdict_banner);

        auto* sgrid = new QGridLayout();
        auto makeStat = [&](int r, int c, const QString& title, QLabel*& out, const QString& col) {
            auto* b = makeCard();
            b->setStyleSheet(QString("background-color: %1; border: 1px solid %2; padding: 5px;").arg(Theme::BG_INPUT, Theme::BORDER_INPUT));
            auto* vl = new QVBoxLayout(b);
            vl->setContentsMargins(6, 4, 6, 4);
            auto* l = new QLabel(title); l->setStyleSheet("color: #94a3b8; font-size: 9px; font-weight: bold; border:none;");
            out = new QLabel("—"); out->setStyleSheet(QString("color: %1; font-family: Monospace; font-size: 11px; font-weight: bold; border:none;").arg(col));
            vl->addWidget(l); vl->addWidget(out);
            sgrid->addWidget(b, r, c);
        };
        makeStat(0, 0, "RHP UNSTABLE POLES (Sign Flip Count)", stat_rhp, Theme::ACCENT_POLE);
        makeStat(0, 1, "LHP STABLE POLES (Re(s) < 0)",         stat_lhp, Theme::ACCENT_ZERO);
        makeStat(1, 0, "jω-AXIS MARGINAL ROOTS (Re(s) = 0)",   stat_jw,  Theme::ACCENT_PID);
        makeStat(1, 1, "CRITICAL GAIN K_crit & ω_osc",         stat_krange, Theme::ACCENT_LOCUS);
        tl->addLayout(sgrid);

        aux_math_w = new MathFormulaWidget("", QColor(Theme::ACCENT_PID), QColor(Theme::BG_INPUT), 10);
        aux_math_w->setVisible(false);
        tl->addWidget(aux_math_w);

        roots_list_lbl = new QLabel();
        roots_list_lbl->setWordWrap(true);
        roots_list_lbl->setStyleSheet(QString("background-color: %1; color: %2; font-family: Monospace; font-size: 10px; padding: 6px; border-radius: 4px; border: 1px solid %3;")
                                      .arg(Theme::BG_INPUT, Theme::TEXT_PRIMARY, Theme::BORDER_INPUT));
        verdict_banner->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        roots_list_lbl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        tl->addWidget(roots_list_lbl);
        left_v->addWidget(tel_card);
        left_v->addStretch(1);

        // Right Panel: Interactive Routh-Hurwitz Array Table
        auto* right_card = makeCard();
        auto* rl = new QVBoxLayout(right_card);
        rl->setContentsMargins(12, 10, 12, 10);
        auto* rt = new QLabel("Routh-Hurwitz Array  (First Column Highlighted for Sign-Change Inspection)");
        rt->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 13px; border:none;").arg(Theme::TEXT_PRIMARY));
        rl->addWidget(rt);

        auto* scroll = new QScrollArea();
        scroll->setWidgetResizable(true);
        scroll->setStyleSheet(QString("background-color: %1; border: 1px solid %2; border-radius: 6px;").arg(Theme::BG_PLOT, Theme::BORDER_PANEL));
        table_container = new QWidget();
        table_container = new QWidget();
        routh_grid = new QGridLayout(table_container);
        routh_grid->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        routh_grid->setContentsMargins(14, 12, 14, 12);
        routh_grid->setHorizontalSpacing(14);
        routh_grid->setVerticalSpacing(6);
        scroll->setWidget(table_container);
        rl->addWidget(scroll, 1);

        splitter->addWidget(wrapInLeftScroll(left_w, 380));
        splitter->addWidget(right_card);
        splitter->setStretchFactor(0, 0);
        splitter->setStretchFactor(1, 1);

        connect(source_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
            if (!updating_ui) evaluateCurrent();
        });
        connect(solve_btn, &QPushButton::clicked, this, [this]() { evaluateCurrent(); });
        connect(custom_poly_edit, &QLineEdit::textEdited, this, [this](const QString&) {
         if (source_combo->currentIndex() != 2) {
             updating_ui = true;
             source_combo->setCurrentIndex(2);
             updating_ui = false;
         }
     });
        connect(k_eval_edit, &QLineEdit::editingFinished, this, [this]() { evaluateCurrent(); });

        loadFromModel();
    }

    void loadFromModel() {
        updating_ui = true;
        source_combo->setCurrentIndex(std::clamp(model->routh_mode, 0, 2));
        custom_poly_edit->setText(QString::fromStdString(model->routh_custom_poly));
        k_eval_edit->setText(QString::fromStdString(model->routh_k_eval));
        updating_ui = false;
        evaluateCurrent();
    }

    void evaluateCurrent() {
        if (updating_ui) return;
        model->routh_mode        = source_combo->currentIndex();
        model->routh_custom_poly = custom_poly_edit->text().toStdString();
        model->routh_k_eval      = k_eval_edit->text().toStdString();
        int mode = source_combo->currentIndex();
        double k_val = parseNumericInput(k_eval_edit->text().toStdString(), 1.0);

        std::vector<double> p0, pk;
        try {
            if (mode == 0) {
                // 1 + K * G(s)H(s) = 0 -> Den_L(s) + K * Num_L(s) = 0
                auto lp = model->effectiveLoopPolesS();
                auto lz = model->effectiveLoopZerosS();
                double base_g = model->plant_gain * (model->unity_feedback_s ? 1.0 : model->h_gain);
                Poly d_poly = polyFromRoots(lp);
                Poly n_poly = polyScale(polyFromRoots(lz), Complex(base_g, 0.0));
                size_t deg = std::max(d_poly.size(), n_poly.size());
                p0.assign(deg, 0.0);
                pk.assign(deg, 0.0);
                for (size_t i = 0; i < d_poly.size(); ++i) p0[deg - d_poly.size() + i] = d_poly[i].real();
                for (size_t i = 0; i < n_poly.size(); ++i) pk[deg - n_poly.size() + i] = n_poly[i].real();
            } else if (mode == 1) {
                // Active PID Closed-Loop
                auto [num_g, den_g] = ComplexTransferFunctionSimulator::buildPlantPolynomials(model->poles, model->zeros, model->plant_gain);
                auto [num_c, den_c] = ComplexTransferFunctionSimulator::buildPidPolynomials(model->kp, model->ki, model->kd, model->deriv_filter_n, true);
                Poly num_h = {Complex(1.0, 0.0)}, den_h = {Complex(1.0, 0.0)};
                if (!model->unity_feedback_s) {
                    std::tie(num_h, den_h) = ComplexTransferFunctionSimulator::buildPlantPolynomials(model->h_poles, model->h_zeros, model->h_gain);
                }
                Poly dl = polyMul(polyMul(den_c, den_g), den_h);
                Poly nl = polyMul(polyMul(num_c, num_g), num_h);
                Poly cl_den = polyTrim(polyAdd(dl, nl));
                for (const auto& c : cl_den) p0.push_back(c.real());
                pk.assign(p0.size(), 0.0);
            } else {
                std::tie(p0, pk) = RouthHurwitzEngine::parseSymbolicSPoly(custom_poly_edit->text().toStdString());
            }
        } catch (const std::exception& e) {
            banner_title->setText("⚠ POLYNOMIAL PARSE ERROR");
            banner_sub->setText(QString::fromStdString(e.what()));
            return;
        }

        std::vector<double> eval_coeffs(p0.size(), 0.0);
        for (size_t i = 0; i < p0.size(); ++i) eval_coeffs[i] = p0[i] + k_val * pk[i];

        auto res = RouthHurwitzEngine::buildRouthArray(eval_coeffs);
        RouthHurwitzEngine::analyzeSymbolicK(p0, pk, res);
        RouthHurwitzEngine::populateSymbolicRouthCells(p0, pk, res);

        Poly p0_c(p0.size()), pk_c(pk.size()), p2_c(p0.size());
        for (size_t i = 0; i < p0.size(); ++i) {
            p0_c[i] = p0[i];
            pk_c[i] = p0[i] + pk[i];
            p2_c[i] = p0[i] + 2.0 * pk[i];
        }
        std::string sym_poly_s = BlockDiagramAlgebraEngine::formatPoly2DSK(p0_c, pk_c, p2_c);
        std::string poly_ltx = "P(s; K) = " + sym_poly_s + " = 0 \\quad \\big|_{K=" + PRECISION.fmt(k_val) + "} \\; ⟹ \\; " +
        RouthHurwitzEngine::formatPolyLatexFromRealCoeffs(res.coeffs, "s") + " = 0";
        char_eq_math_w->setLatex(QString::fromStdString(poly_ltx));

        if (!res.aux_poly_latex.empty()) {
            aux_math_w->setVisible(true);
            aux_math_w->setLatex(QString::fromStdString(res.aux_poly_latex + " \\quad ⟹ \\quad " + res.aux_deriv_latex));
        } else {
            aux_math_w->setVisible(false);
        }

        bool is_stable = (res.sign_changes == 0 && res.rhp_poles == 0 && res.jw_poles == 0);
        bool is_marginal = (res.sign_changes == 0 && res.rhp_poles == 0 && res.jw_poles > 0);

        QString bg = is_stable ? Theme::BANNER_OK_BG : (is_marginal ? Theme::BANNER_WARN_BG : Theme::BANNER_ERR_BG);
        QString bd = is_stable ? Theme::BANNER_OK_BD : (is_marginal ? Theme::BANNER_WARN_BD : Theme::BANNER_ERR_BD);
        QString fg = is_stable ? Theme::BANNER_OK_FG : (is_marginal ? Theme::BANNER_WARN_FG : Theme::BANNER_ERR_FG);

        verdict_banner->setStyleSheet(QString("QFrame { background-color: %1; border: 1.5px solid %2; border-radius: 5px; }").arg(bg, bd));
        banner_title->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 12px; border:none;").arg(fg));
        banner_right->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; font-size: 11px; border:none;").arg(Theme::TEXT_PRIMARY));
        banner_sub->setStyleSheet(QString("color: %1; font-size: 10px; font-weight: bold; border:none;").arg(fg));

        banner_title->setText(is_stable ? "✔ BIBO STABLE (All Roots in LHP)"
                                        : (is_marginal ? "⚠ MARGINALLY STABLE (Roots on jω-Axis)"
                                                       : "✖ UNSTABLE SYSTEM (RHP Poles Present)"));
        banner_right->setText(QString("%1 Col-1 Sign Change(s)").arg(res.sign_changes));
        banner_sub->setText(QString::fromStdString(
            res.has_k_analysis ? res.k_range_summary
                               : "First-column sign changes equal the number of Right-Half Plane (RHP) poles."
        ));

        stat_rhp->setText(QString("%1 RHP Pole(s)\n(%2 Sign Changes)").arg(res.rhp_poles).arg(res.sign_changes));
        stat_lhp->setText(QString("%1 LHP Pole(s)").arg(res.lhp_poles));
        stat_jw->setText(QString("%1 jω-Axis Root(s)").arg(res.jw_poles));

        if (!res.k_crit_w_pairs.empty()) {
            auto [kc, wc] = res.k_crit_w_pairs.front();
            stat_krange->setText(QString("K_crit = %1\nω_osc = ±%2j r/s")
                .arg(QString::fromStdString(PRECISION.fmt(kc)))
                .arg(QString::fromStdString(PRECISION.fmt(wc))));
        } else {
            stat_krange->setText(res.has_k_analysis ? "No jω Crossing" : "Fixed Coeffs");
        }

        // Exact Roots Summary
        std::string r_str = "Solved Characteristic Roots:  ";
        for (size_t i = 0; i < res.exact_roots.size(); ++i) {
            r_str += (i ? ",   " : "") + PRECISION.fmtComplex(res.exact_roots[i]);
        }
        roots_list_lbl->setText(QString::fromStdString(r_str));

        // Rebuild Routh Table Grid
        QLayoutItem* child;
        while ((child = routh_grid->takeAt(0)) != nullptr) {
            delete child->widget();
            delete child;
        }

        if (res.rows.empty()) return;
        int num_cols = static_cast<int>(res.rows[0].vals.size());

        // Column Headers
        auto* h_pow = new QLabel("Power");
        h_pow->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border:none;").arg(Theme::TEXT_SECONDARY));
        routh_grid->addWidget(h_pow, 0, 0);

        for (int c = 0; c < num_cols; ++c) {
            auto* hc = new QLabel(c == 0 ? "Col 1 (Pivot / Sign)" : QString("Col %1").arg(c + 1));
            hc->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border:none;")
                              .arg(c == 0 ? Theme::ACCENT_PID : Theme::ACCENT_ZERO));
            routh_grid->addWidget(hc, 0, c + 1);
        }
        auto* h_note = new QLabel("Row Status / Special Cases");
        h_note->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border:none;").arg(Theme::TEXT_SECONDARY));
        routh_grid->addWidget(h_note, 0, num_cols + 1);

        // Populate Routh Rows
        for (size_t r = 0; r < res.rows.size(); ++r) {
            const auto& row = res.rows[r];
            auto* p_lbl = new QLabel(QString::fromStdString(row.label));
            p_lbl->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; font-size: 12px; border:none;").arg(Theme::ACCENT_LOCUS));
            routh_grid->addWidget(p_lbl, static_cast<int>(r + 1), 0);

            for (int c = 0; c < num_cols; ++c) {
                double v = row.vals[c];
                bool is_c0 = (c == 0);
                QString val_s = QString::fromStdString(PRECISION.fmt(v, is_c0));
                if (c < static_cast<int>(row.sym_vals.size()) && !row.sym_vals[c].empty()) {
                    val_s = QString("%1   [%2]").arg(QString::fromStdString(row.sym_vals[c]), val_s);
                }
                if (is_c0 && row.special_note.find("Case 1") != std::string::npos) {
                    val_s = "+ε (≈0⁺)";
                }

                auto* cell = new QLabel(val_s);
                cell->setFixedHeight(30);
                cell->setMinimumWidth(95);
                cell->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
                QString bg_c = is_c0 ? (row.sign_changed ? Theme::BANNER_ERR_BG : Theme::BG_PANEL_ALT) : Theme::BG_INPUT;
                QString bd_c = is_c0 ? (row.sign_changed ? Theme::ACCENT_POLE   : Theme::ACCENT_PID)   : Theme::BORDER_INPUT;
                QString fg_c = is_c0 ? (v < 0.0 ? Theme::ACCENT_POLE : Theme::ACCENT_ZERO) : Theme::TEXT_PRIMARY;

                cell->setStyleSheet(QString(
                    "background-color: %1; color: %2; border: 1px solid %3; border-radius: 4px; padding: 5px 10px; font-family: Monospace; font-weight: bold; font-size: 12px;"
                ).arg(bg_c, fg_c, bd_c));
                routh_grid->addWidget(cell, static_cast<int>(r + 1), c + 1);
            }

            QString note_txt = QString::fromStdString(row.special_note);
            if (row.sign_changed) {
                note_txt = (note_txt.isEmpty() ? "" : note_txt + "  •  ") + "⚡ SIGN CHANGE (RHP Pole)";
            }
            auto* n_lbl = new QLabel(note_txt.isEmpty() ? "Standard Routh Row" : note_txt);
            n_lbl->setStyleSheet(QString("color: %1; font-family: Monospace; font-size: 10px; font-weight: bold; border:none;")
                                 .arg(row.sign_changed ? Theme::ACCENT_POLE : (!row.special_note.empty() ? Theme::ACCENT_PID : "#64748b")));
            routh_grid->addWidget(n_lbl, static_cast<int>(r + 1), num_cols + 1);
        }
    }
};

