#pragma once
#include "BlockDiagramStudioTab.h"


// =============================================================================
// Engine 7 & Tab 8: Frequency-Domain Suite (Bode, Straight-Line Asymptotes & Nyquist)
// =============================================================================
struct BodeCornerFactor {
    double wc = 1.0;      // Break frequency |r| or wn (rad/s)
    int order = 1;        // 1 = real factor, 2 = complex-conjugate quadratic pair
    bool is_zero = false; // true = zero, false = pole
    bool is_rhp = false;  // Re(r) > 0 (non-minimum-phase / unstable: flips phase slope)
    bool is_jw = false;   // Purely imaginary root pair on jw-axis (zeta ~ 0)
    double zeta = 1.0;    // Damping ratio for quadratic pairs
};

struct FreqResponseResult {
    int mode_idx = 0; // 0 = Continuous Open-Loop L(jw), 1 = Continuous Closed-Loop T(jw), 2 = Discrete H(e^{jwTs})
    std::vector<double> w;
    std::vector<double> mag_abs;
    std::vector<double> mag_db;
    std::vector<double> asymp_db;
    std::vector<double> phase_deg;
    std::vector<double> asymp_phase_deg;
    std::vector<Complex> c_vals;

    // Optional Uncompensated Ghost Curve (G(s)H(s) or T_uncomp(s) when Gc(s)/PID is active)
    bool has_ghost = false;
    std::vector<double> ghost_mag_db;
    std::vector<double> ghost_phase_deg;
    std::vector<Complex> ghost_c_vals;
    std::optional<double> ghost_pm_deg;
    std::optional<double> ghost_gm_db;

    // Bode Corner Metadata & Origin Multiplicity
    int n_zero_orig = 0;
    int n_pole_orig = 0;
    std::vector<double> corner_freqs;
    std::vector<std::string> corner_labels;

    // Open-Loop Stability Margins (Evaluated on L(jw) across extended sweep)
    std::optional<double> w_gc;       // Critical Gain Crossover (0 dB)
    std::optional<double> pm_deg;     // Phase Margin in (-180, +180] deg
    double pm_ref_deg = -180.0;       // Nearest odd-180 deg reference line at w_gc
    std::optional<double> w_pc;       // Critical Phase Crossover ((2k+1)*180 deg)
    std::optional<double> gm_db;      // Gain Margin (dB)
    double pc_ref_deg = -180.0;       // Phase line crossed at w_pc
    int num_gc_crossings = 0;
    int num_pc_crossings = 0;

    // Algebraic Pole Stability Checks
    int ol_rhp_poles = 0;
    int cl_rhp_poles = 0;
    bool cl_stable = true;
    double max_z_pole_mag = 0.0;

    // True DC Gain, Closed-Loop Resonant Peak Mr & -3 dB Bandwidth
    std::optional<double> dc_gain_db; // Finite DC gain (nullopt if origin pole/zero makes |H(0)| = inf or 0)
    double peak_mag_db = 0.0;         // Peak of plotted curve (or CL peak in Mode 0)
    double mr_db = 0.0;               // Closed-loop resonant peak |T(j*wr)|_dB
    double w_peak = 0.0;              // Resonant peak frequency wr (rad/s)
    std::optional<double> w_bw;       // Closed-loop (or filter) -3 dB bandwidth (rad/s)
    double bw_ref_db = -3.0103;       // Horizontal dB level used for -3 dB bandwidth

    // Exact Analytical Cursor Inspection
    double cursor_w = 1.0;
    double cursor_mag_db = 0.0;
    double cursor_phase_deg = 0.0;
    Complex cursor_c = {1.0, 0.0};
};

class FrequencyDomainEngine {
public:
    struct FactoredSModel {
        int n_zero_orig = 0;
        int n_pole_orig = 0;
        double k_bode = 1.0; // Signed Bode gain c_num0 / c_den0
        std::vector<BodeCornerFactor> factors;
        std::vector<double> corner_freqs;
        std::vector<std::string> corner_labels;

        double baseDb() const {
            return 20.0 * std::log10(std::max(std::abs(k_bode), 1e-15));
        }

        double basePhaseDeg() const {
            return double(n_zero_orig - n_pole_orig) * 90.0 + ((k_bode < 0.0) ? -180.0 : 0.0);
        }

        double evalAsympDb(double w) const {
            double db = baseDb() + 20.0 * double(n_zero_orig - n_pole_orig) * std::log10(std::max(w, 1e-30));
            for (const auto& f : factors) {
                if (w > f.wc) {
                    double sign = f.is_zero ? 1.0 : -1.0;
                    db += sign * 20.0 * double(f.order) * std::log10(w / f.wc);
                }
            }
            return db;
        }

        double evalAsympPhaseDeg(double w) const {
            double ph = basePhaseDeg();
            for (const auto& f : factors) {
                double dir = f.is_zero ? (f.is_rhp ? -1.0 : 1.0) : (f.is_rhp ? 1.0 : -1.0);
                double total_deg = dir * 90.0 * double(f.order);
                if (f.is_jw) {
                    if (w >= f.wc) ph += total_deg;
                } else {
                    double w_lo = 0.1 * f.wc;
                    double w_hi = 10.0 * f.wc;
                    if (w >= w_hi) ph += total_deg;
                    else if (w > w_lo) ph += total_deg * 0.5 * (std::log10(w / f.wc) + 1.0);
                }
            }
            return ph;
        }

        // Exact continuous Bode-factored phase (used to anchor the 360° branch sheet unambiguously)
        double evalExactFactoredPhaseRad(double w) const {
            double ph = double(n_zero_orig - n_pole_orig) * (0.5 * PI) + ((k_bode < 0.0) ? -PI : 0.0);
            for (const auto& f : factors) {
                double sgn = f.is_zero ? 1.0 : -1.0;
                if (f.order == 1) {
                    // Factor (1 + j*w/wc) for LHP, (1 - j*w/wc) for RHP
                    double im = f.is_rhp ? (-w / f.wc) : (w / f.wc);
                    ph += sgn * std::atan2(im, 1.0);
                } else {
                    if (f.is_jw) {
                        if (w >= f.wc) ph += sgn * PI;
                    } else {
                        double u = w / f.wc;
                        double re = 1.0 - u * u;
                        // LHP has zeta > 0 -> imag > 0; RHP has is_rhp -> imag < 0
                        double z_signed = f.is_rhp ? -std::abs(f.zeta) : std::abs(f.zeta);
                        double im = 2.0 * z_signed * u;
                        ph += sgn * std::atan2(im, re);
                    }
                }
            }
            return ph;
        }
    };

    static FactoredSModel factorizeContinuousTF(const Poly& raw_num, const Poly& raw_den) {
        FactoredSModel fm;
        Poly num = polyTrim(raw_num, 1e-13);
        Poly den = polyTrim(raw_den, 1e-13);

        double max_n = 0.0, max_d = 0.0;
        for (const auto& c : num) max_n = std::max(max_n, std::abs(c));
        for (const auto& c : den) max_d = std::max(max_d, std::abs(c));
        if (max_n <= 1e-15 || max_d <= 1e-15) return fm;

        // 1. Deflate exact origin roots s^{n_z0} and s^{n_p0} BEFORE root-finding
        while (num.size() > 1 && std::abs(num.back()) <= 1e-11 * max_n) {
            num.pop_back();
            ++fm.n_zero_orig;
        }
        while (den.size() > 1 && std::abs(den.back()) <= 1e-11 * max_d) {
            den.pop_back();
            ++fm.n_pole_orig;
        }

        double num_c0 = num.back().real();
        double den_c0 = den.back().real();
        if (std::abs(den_c0) < 1e-15) den_c0 = 1.0;
        fm.k_bode = num_c0 / den_c0;

        auto z_roots = solvePolynomialRoots(num);
        auto p_roots = solvePolynomialRoots(den);

        // Catch any residual tiny roots near s = 0
        std::vector<Complex> nz_zeros, nz_poles;
        for (const auto& z : z_roots) {
            if (std::abs(z) < 1e-6) ++fm.n_zero_orig;
            else nz_zeros.push_back(z);
        }
        for (const auto& p : p_roots) {
            if (std::abs(p) < 1e-6) ++fm.n_pole_orig;
            else nz_poles.push_back(p);
        }
        int common_orig = std::min(fm.n_zero_orig, fm.n_pole_orig);
        fm.n_zero_orig -= common_orig;
        fm.n_pole_orig -= common_orig;

        // 2. Cancel matching pole-zero pairs so cancelled roots don't pollute Bode corners/asymptotes
        std::vector<bool> z_used(nz_zeros.size(), false);
        std::vector<Complex> eff_poles, eff_zeros;
        for (const auto& p : nz_poles) {
            bool cancelled = false;
            double tol = 1e-4 * std::max(1.0, std::abs(p));
            for (size_t zi = 0; zi < nz_zeros.size(); ++zi) {
                if (!z_used[zi] && std::abs(p - nz_zeros[zi]) < tol) {
                    z_used[zi] = true;
                    cancelled = true;
                    break;
                }
            }
            if (!cancelled) eff_poles.push_back(p);
        }
        for (size_t zi = 0; zi < nz_zeros.size(); ++zi) {
            if (!z_used[zi]) eff_zeros.push_back(nz_zeros[zi]);
        }

        // 3. Group roots into real 1st-order factors & complex-conjugate 2nd-order factors
        auto appendFactors = [&](const std::vector<Complex>& roots, bool is_z) {
            std::vector<bool> used(roots.size(), false);
            for (size_t i = 0; i < roots.size(); ++i) {
                if (used[i]) continue;
                Complex r = roots[i];
                double wc = std::max(std::abs(r), 1e-9);
                if (std::abs(r.imag()) <= 1e-5 * wc) {
                    used[i] = true;
                    BodeCornerFactor bf;
                    bf.wc = std::abs(r.real());
                    bf.order = 1;
                    bf.is_zero = is_z;
                    bf.is_rhp = (r.real() > 0.0);
                    bf.is_jw = false;
                    bf.zeta = 1.0;
                    fm.factors.push_back(bf);
                } else {
                    // Pair with its complex conjugate
                    size_t mate = i;
                    double best_d = 1e30;
                    for (size_t j = i + 1; j < roots.size(); ++j) {
                        if (!used[j]) {
                            double d = std::abs(roots[j] - std::conj(r));
                            if (d < best_d) { best_d = d; mate = j; }
                        }
                    }
                    used[i] = true;
                    if (mate != i && best_d < 1e-2 * wc) {
                        used[mate] = true;
                        BodeCornerFactor bf;
                        bf.wc = wc;
                        bf.order = 2;
                        bf.is_zero = is_z;
                        bf.is_jw = (std::abs(r.real()) <= 1e-5 * wc);
                        bf.is_rhp = (!bf.is_jw && r.real() > 0.0);
                        bf.zeta = bf.is_jw ? 0.0 : (std::abs(r.real()) / wc);
                        fm.factors.push_back(bf);
                    } else {
                        BodeCornerFactor bf;
                        bf.wc = wc;
                        bf.order = 1;
                        bf.is_zero = is_z;
                        bf.is_rhp = (r.real() > 0.0);
                        fm.factors.push_back(bf);
                    }
                }
            }
        };
        appendFactors(eff_zeros, true);
        appendFactors(eff_poles, false);

        std::sort(fm.factors.begin(), fm.factors.end(), [](const BodeCornerFactor& a, const BodeCornerFactor& b) {
            return a.wc < b.wc;
        });

        for (const auto& f : fm.factors) {
            if (fm.corner_freqs.empty() || std::abs(f.wc - fm.corner_freqs.back()) > 1e-4 * f.wc) {
                fm.corner_freqs.push_back(f.wc);
            }
            std::string lbl = PRECISION.fmt(f.wc);
            if (f.order == 2) {
                lbl += " (ζ=" + PRECISION.fmt(f.zeta) + ")";
            } else if (f.is_rhp) {
                lbl += " (RHP)";
            }
            fm.corner_labels.push_back(lbl);
        }
        return fm;
    }

    static Complex evalRatS(const Poly& num, const Poly& den, double w) {
        Complex s(0.0, w);
        Complex n_v = polyVal(num, s);
        Complex d_v = polyVal(den, s);
        if (std::abs(d_v) > 1e-18) return n_v / d_v;
        // Perturb slightly for exact jw-axis pole evaluation
        Complex s_eps(1e-12 * std::max(1.0, w), w);
        return polyVal(num, s_eps) / polyVal(den, s_eps);
    }

    static Complex evalRatZ(const Poly& num, const Poly& den, double w, double ts) {
        Complex z = std::exp(Complex(0.0, w * ts));
        Complex n_v = polyVal(num, z);
        Complex d_v = polyVal(den, z);
        if (std::abs(d_v) > 1e-18) return n_v / d_v;
        Complex z_eps = std::exp(Complex(1e-12, w * ts));
        return polyVal(num, z_eps) / polyVal(den, z_eps);
    }

    static void sampleContinuousSweep(
        const Poly& num, const Poly& den, const FactoredSModel& fm,
        const std::vector<double>& w_vec,
        std::vector<Complex>& c_out,
        std::vector<double>& mag_abs_out,
        std::vector<double>& mag_db_out,
        std::vector<double>& phase_deg_out)
    {
        size_t n = w_vec.size();
        c_out.resize(n);
        mag_abs_out.resize(n);
        mag_db_out.resize(n);
        phase_deg_out.resize(n);
        if (n == 0) return;

        double prev_rad = 0.0;
        double unwrap_offset = 0.0;

        for (size_t i = 0; i < n; ++i) {
            Complex val = evalRatS(num, den, w_vec[i]);
            c_out[i] = val;
            double m = std::abs(val);
            mag_abs_out[i] = m;
            mag_db_out[i]  = 20.0 * std::log10(std::max(m, 1e-15));

            double rad = std::arg(val);
            if (i == 0) {
                // Anchor initial 2*pi*k branch to the exact factored Bode phase sheet!
                double exact_rad0 = fm.evalExactFactoredPhaseRad(w_vec[0]);
                double k_turns = std::round((exact_rad0 - rad) / (2.0 * PI));
                unwrap_offset = k_turns * (2.0 * PI);
            } else {
                double diff = rad - prev_rad;
                if (std::abs(diff) > 0.85 * PI) {
                    // Near a +/-pi discontinuity (e.g. undamped jw pair) or steep wrap: lock sheet to factored model
                    double exact_radi = fm.evalExactFactoredPhaseRad(w_vec[i]);
                    double k_turns = std::round((exact_radi - rad) / (2.0 * PI));
                    unwrap_offset = k_turns * (2.0 * PI);
                } else {
                    if (diff > PI) unwrap_offset -= 2.0 * PI;
                    else if (diff < -PI) unwrap_offset += 2.0 * PI;
                }
            }
            prev_rad = rad;
            phase_deg_out[i] = (rad + unwrap_offset) * (180.0 / PI);
        }
    }

    static FreqResponseResult evaluate(
        const SharedSystemModel* model,
        int mode_idx, // 0 = Continuous Open-Loop L(jw), 1 = Continuous Closed-Loop T(jw), 2 = Discrete H(e^{jwTs})
        bool include_pid,
        double w_min_exp,
        double w_max_exp,
        double cursor_w,
        int n_pts = 750)
    {
        FreqResponseResult res;
        res.mode_idx = mode_idx;
        n_pts = std::max(n_pts, 200);
        double ts = std::max(model->ts, 1e-5);

        if (mode_idx == 0 || mode_idx == 1) {
            // 1. Build Uncompensated & Compensated Continuous Loop Polynomials
            auto [num_g, den_g] = ComplexTransferFunctionSimulator::buildPlantPolynomials(
                model->poles, model->zeros, model->plant_gain);

            Poly num_h = {Complex(1.0, 0.0)}, den_h = {Complex(1.0, 0.0)};
            if (!model->unity_feedback_s) {
                std::tie(num_h, den_h) = ComplexTransferFunctionSimulator::buildPlantPolynomials(
                    model->h_poles, model->h_zeros, model->h_gain);
            }

            // Active series controller: Gc(s) takes precedence when Enable Gc(s) is checked;
            // otherwise Include PID uses Tab 2's PID C(s).
            bool ctrl_active = model->comp_enabled_s || include_pid;
            Poly num_c = {Complex(1.0, 0.0)}, den_c = {Complex(1.0, 0.0)};
            if (model->comp_enabled_s) {
                std::tie(num_c, den_c) = ComplexTransferFunctionSimulator::buildPlantPolynomials(
                    model->c_poles, model->c_zeros, model->c_gain);
            } else if (include_pid) {
                std::tie(num_c, den_c) = ComplexTransferFunctionSimulator::buildPidPolynomials(
                    model->kp, model->ki, model->kd, model->deriv_filter_n, true);
            }

            Poly num_f  = polyTrim(polyMul(num_c, num_g));
            Poly den_f  = polyTrim(polyMul(den_c, den_g));
            Poly num_l  = polyTrim(polyMul(num_f, num_h));
            Poly den_l  = polyTrim(polyMul(den_f, den_h));
            Poly num_cl = polyTrim(polyMul(num_f, den_h));
            Poly den_cl = polyTrim(polyAdd(den_l, num_l));

            // Uncompensated baseline (C(s) = 1) for Ghost overlay
            Poly num_l_un  = polyTrim(polyMul(num_g, num_h));
            Poly den_l_un  = polyTrim(polyMul(den_g, den_h));
            Poly num_cl_un = polyTrim(polyMul(num_g, den_h));
            Poly den_cl_un = polyTrim(polyAdd(den_l_un, num_l_un));

            // Algebraic RHP pole counts for open-loop L(s) and closed-loop T(s)
            auto ol_poles_all = solvePolynomialRoots(den_l);
            auto cl_poles_all = solvePolynomialRoots(den_cl);
            for (const auto& p : ol_poles_all) if (p.real() > 1e-6) ++res.ol_rhp_poles;
            for (const auto& p : cl_poles_all) if (p.real() > 1e-5) ++res.cl_rhp_poles;
            res.cl_stable = (res.cl_rhp_poles == 0);

            const Poly& plot_num = (mode_idx == 0) ? num_l : num_cl;
            const Poly& plot_den = (mode_idx == 0) ? den_l : den_cl;
            FactoredSModel fm_plot = factorizeContinuousTF(plot_num, plot_den);
            FactoredSModel fm_loop = (mode_idx == 0) ? fm_plot : factorizeContinuousTF(num_l, den_l);
            FactoredSModel fm_cl   = (mode_idx == 1) ? fm_plot : factorizeContinuousTF(num_cl, den_cl);

            res.n_zero_orig   = fm_plot.n_zero_orig;
            res.n_pole_orig   = fm_plot.n_pole_orig;
            res.corner_freqs  = fm_plot.corner_freqs;
            res.corner_labels = fm_plot.corner_labels;

            // True DC Gain of plotted transfer function at s = 0
            if (fm_plot.n_zero_orig == 0 && fm_plot.n_pole_orig == 0 && std::abs(fm_plot.k_bode) > 1e-15) {
                res.dc_gain_db = 20.0 * std::log10(std::abs(fm_plot.k_bode));
            }

            // 2. Evaluate Primary Viewport Grid [10^w_min_exp, 10^w_max_exp]
            res.w.resize(n_pts);
            res.asymp_db.resize(n_pts);
            res.asymp_phase_deg.resize(n_pts);
            for (int i = 0; i < n_pts; ++i) {
                res.w[i] = std::pow(10.0, w_min_exp + (w_max_exp - w_min_exp) * double(i) / double(n_pts - 1));
                res.asymp_db[i]        = fm_plot.evalAsympDb(res.w[i]);
                res.asymp_phase_deg[i] = fm_plot.evalAsympPhaseDeg(res.w[i]);
            }
            sampleContinuousSweep(plot_num, plot_den, fm_plot, res.w, res.c_vals, res.mag_abs, res.mag_db, res.phase_deg);

            // 3. Extract Open-Loop Margins (GM, PM, w_gc, w_pc) on Extended Sweep [10^-6, 10^+6]
            auto extractLoopMargins = [&](const Poly& nl, const Poly& dl, const FactoredSModel& fml,
                                          std::optional<double>& out_wgc, std::optional<double>& out_pm, double& out_pm_ref,
                                          std::optional<double>& out_wpc, std::optional<double>& out_gm, double& out_pc_ref,
                                          int& out_n_gc, int& out_n_pc)
            {
                double ext_lo = std::min(w_min_exp, -6.0);
                double ext_hi = std::max(w_max_exp,  6.0);
                const int n_ext = 1200;
                std::vector<double> w_ext(n_ext), m_abs, m_db, ph_deg;
                std::vector<Complex> c_ext;
                for (int i = 0; i < n_ext; ++i) {
                    w_ext[i] = std::pow(10.0, ext_lo + (ext_hi - ext_lo) * double(i) / double(n_ext - 1));
                }
                sampleContinuousSweep(nl, dl, fml, w_ext, c_ext, m_abs, m_db, ph_deg);

                // Gain Crossovers: |L(j*w)| == 0 dB
                double best_pm_score = 1e30;
                for (int i = 0; i + 1 < n_ext; ++i) {
                    double m1 = m_db[i], m2 = m_db[i + 1];
                    if (std::abs(m1 - m2) < 1e-9) continue; // Ignore constant 0 dB line
                    if ((m1 >= 0.0 && m2 < 0.0) || (m1 <= 0.0 && m2 > 0.0)) {
                        double la = std::log10(w_ext[i]), lb = std::log10(w_ext[i + 1]);
                        double fa = m1;
                        for (int it = 0; it < 28; ++it) {
                            double lm = 0.5 * (la + lb);
                            double fm_val = 20.0 * std::log10(std::max(std::abs(evalRatS(nl, dl, std::pow(10.0, lm))), 1e-18));
                            if ((fa >= 0.0 && fm_val >= 0.0) || (fa <= 0.0 && fm_val <= 0.0)) { la = lm; fa = fm_val; }
                            else { lb = lm; }
                        }
                        double wgc = std::pow(10.0, 0.5 * (la + lb));
                        double rad_gc = std::arg(evalRatS(nl, dl, wgc));
                        double rad_i  = ph_deg[i] * (PI / 180.0);
                        double ph_gc_deg = (rad_i + std::remainder(rad_gc - rad_i, 2.0 * PI)) * (180.0 / PI);

                        double pm = std::remainder(180.0 + ph_gc_deg, 360.0);
                        if (pm <= -180.0 + 1e-9) pm += 360.0; // Wrap strictly into (-180, +180]
                        double ref_deg = ph_gc_deg - pm;

                        ++out_n_gc;
                        // Prioritize negative PM (instability) or smallest |PM|
                        double score = (pm <= 0.0) ? (-1000.0 + pm) : pm;
                        if (score < best_pm_score) {
                            best_pm_score = score;
                            out_wgc    = wgc;
                            out_pm     = pm;
                            out_pm_ref = ref_deg;
                        }
                    }
                }

                // Phase Crossovers: ∠L(j*w) == (2k + 1) * 180 deg
                double best_gm_score = 1e30;
                for (int i = 0; i + 1 < n_ext; ++i) {
                    double p1 = ph_deg[i], p2 = ph_deg[i + 1];
                    if (std::abs(p2 - p1) < 1e-6) continue; // Ignore constant -180° phase lines (e.g. 1/s^2 or K < 0)
                    double u1 = (p1 - 180.0) / 360.0;
                    double u2 = (p2 - 180.0) / 360.0;
                    int k_min = static_cast<int>(std::ceil(std::min(u1, u2) - 1e-9));
                    int k_max = static_cast<int>(std::floor(std::max(u1, u2) + 1e-9));
                    for (int k = k_min; k <= k_max; ++k) {
                        double target_deg = 180.0 + 360.0 * double(k);
                        bool crosses = (p1 > target_deg && p2 <= target_deg) || (p1 < target_deg && p2 >= target_deg);
                        if (!crosses) continue;

                        double la = std::log10(w_ext[i]), lb = std::log10(w_ext[i + 1]);
                        double fa = p1 - target_deg;
                        double anchor_rad = p1 * (PI / 180.0);
                        for (int it = 0; it < 28; ++it) {
                            double lm = 0.5 * (la + lb);
                            double rad_m = std::arg(evalRatS(nl, dl, std::pow(10.0, lm)));
                            double pm_unw = (anchor_rad + std::remainder(rad_m - anchor_rad, 2.0 * PI)) * (180.0 / PI);
                            double fm_val = pm_unw - target_deg;
                            if ((fa >= 0.0 && fm_val >= 0.0) || (fa <= 0.0 && fm_val <= 0.0)) { la = lm; fa = fm_val; }
                            else { lb = lm; }
                        }
                        double wpc = std::pow(10.0, 0.5 * (la + lb));
                        double db_pc = 20.0 * std::log10(std::max(std::abs(evalRatS(nl, dl, wpc)), 1e-18));
                        double gm = -db_pc;

                        ++out_n_pc;
                        double score = (gm <= 0.0) ? (-1000.0 + gm) : gm;
                        if (score < best_gm_score) {
                            best_gm_score = score;
                            out_wpc    = wpc;
                            out_gm     = gm;
                            out_pc_ref = target_deg;
                        }
                    }
                }
            };

            extractLoopMargins(num_l, den_l, fm_loop,
                               res.w_gc, res.pm_deg, res.pm_ref_deg,
                               res.w_pc, res.gm_db, res.pc_ref_deg,
                               res.num_gc_crossings, res.num_pc_crossings);

            // 4. Compute True Closed-Loop Resonant Peak Mr & -3 dB Bandwidth on T(jw)
            double ext_lo = std::min(w_min_exp, -5.0);
            double ext_hi = std::max(w_max_exp,  5.0);
            const int n_cl = 1000;
            std::vector<double> w_cl(n_cl), cl_db(n_cl);
            for (int i = 0; i < n_cl; ++i) {
                w_cl[i]  = std::pow(10.0, ext_lo + (ext_hi - ext_lo) * double(i) / double(n_cl - 1));
                cl_db[i] = 20.0 * std::log10(std::max(std::abs(evalRatS(num_cl, den_cl, w_cl[i])), 1e-15));
            }

            int pk_i = 0;
            for (int i = 1; i < n_cl; ++i) if (cl_db[i] > cl_db[pk_i]) pk_i = i;
            res.mr_db  = cl_db[pk_i];
            res.w_peak = w_cl[pk_i];
            if (pk_i > 0 && pk_i + 1 < n_cl) {
                // Golden-section polish for exact resonant peak wr & Mr
                double a = std::log10(w_cl[pk_i - 1]), b = std::log10(w_cl[pk_i + 1]);
                const double gr = 0.61803398875;
                for (int it = 0; it < 24; ++it) {
                    double c1 = b - gr * (b - a), c2 = a + gr * (b - a);
                    double f1 = std::abs(evalRatS(num_cl, den_cl, std::pow(10.0, c1)));
                    double f2 = std::abs(evalRatS(num_cl, den_cl, std::pow(10.0, c2)));
                    if (f1 < f2) a = c1; else b = c2;
                }
                res.w_peak = std::pow(10.0, 0.5 * (a + b));
                res.mr_db  = 20.0 * std::log10(std::max(std::abs(evalRatS(num_cl, den_cl, res.w_peak)), 1e-15));
            }
            res.peak_mag_db = res.mr_db;

            // Closed-Loop -3 dB reference relative to T(0) if finite & non-zero, else relative to peak
            double cl_ref_db = (fm_cl.n_zero_orig == 0 && fm_cl.n_pole_orig == 0 && std::abs(fm_cl.k_bode) > 1e-12)
                ? (20.0 * std::log10(std::abs(fm_cl.k_bode)))
                : res.mr_db;
            res.bw_ref_db = cl_ref_db - 3.01029995664;
            for (int i = 0; i + 1 < n_cl; ++i) {
                if (cl_db[i] >= res.bw_ref_db && cl_db[i + 1] < res.bw_ref_db) {
                    double la = std::log10(w_cl[i]), lb = std::log10(w_cl[i + 1]);
                    for (int it = 0; it < 26; ++it) {
                        double lm = 0.5 * (la + lb);
                        double fm_val = 20.0 * std::log10(std::max(std::abs(evalRatS(num_cl, den_cl, std::pow(10.0, lm))), 1e-18));
                        if (fm_val >= res.bw_ref_db) la = lm; else lb = lm;
                    }
                    res.w_bw = std::pow(10.0, 0.5 * (la + lb));
                    break;
                }
            }

            // 5. Evaluate Uncompensated Ghost Curve when Gc(s) or PID is Active
            if (ctrl_active) {
                res.has_ghost = true;
                const Poly& g_num = (mode_idx == 0) ? num_l_un : num_cl_un;
                const Poly& g_den = (mode_idx == 0) ? den_l_un : den_cl_un;
                FactoredSModel fm_ghost = factorizeContinuousTF(g_num, g_den);
                std::vector<double> dummy_abs;
                sampleContinuousSweep(g_num, g_den, fm_ghost, res.w, res.ghost_c_vals, dummy_abs, res.ghost_mag_db, res.ghost_phase_deg);

                FactoredSModel fm_lun = (mode_idx == 0) ? fm_ghost : factorizeContinuousTF(num_l_un, den_l_un);
                std::optional<double> gwgc, gwpc;
                double gpm_r = -180.0, gpc_r = -180.0;
                int gngc = 0, gnpc = 0;
                extractLoopMargins(num_l_un, den_l_un, fm_lun, gwgc, res.ghost_pm_deg, gpm_r, gwpc, res.ghost_gm_db, gpc_r, gngc, gnpc);
            }

            // 6. Exact Analytical Cursor Evaluation at cursor_w
            res.cursor_w = std::max(1e-8, cursor_w);
            res.cursor_c = evalRatS(plot_num, plot_den, res.cursor_w);
            res.cursor_mag_db = 20.0 * std::log10(std::max(std::abs(res.cursor_c), 1e-15));
            double c_rad = std::arg(res.cursor_c);
            double c_exact = fm_plot.evalExactFactoredPhaseRad(res.cursor_w);
            double c_turns = std::round((c_exact - c_rad) / (2.0 * PI));
            res.cursor_phase_deg = (c_rad + c_turns * 2.0 * PI) * (180.0 / PI);
            return res;
        }

        // =========================================================================
        // Mode 2: Discrete Digital Filter Frequency Response H(e^{j*w*Ts})
        // =========================================================================
        Poly num_h = polyScale(polyFromRoots(model->z_zeros), Complex(model->filter_gain, 0.0));
        Poly den_h = polyFromRoots(model->z_poles);
        auto [b_h, a_h] = DiscreteFilterSimulator::toCausalBA(num_h, den_h);

        Poly b_fb = {Complex(1.0, 0.0)}, a_fb = {Complex(1.0, 0.0)};
        if (!model->unity_feedback_z) {
            Poly n_fb = polyScale(polyFromRoots(model->hz_fb_zeros), Complex(model->hz_fb_gain, 0.0));
            Poly d_fb = polyFromRoots(model->hz_fb_poles);
            std::tie(b_fb, a_fb) = DiscreteFilterSimulator::toCausalBA(n_fb, d_fb);
        }

        Poly num, den;
        if (!include_pid) {
            num = b_h;
            den = a_h;
        } else {
            auto [num_c, den_c] = DiscreteFilterSimulator::buildDiscretePid(model->d_kp, model->d_ki, model->d_kd, ts, true);
            Poly nf = polyMul(num_c, b_h);
            Poly df = polyMul(den_c, a_h);
            Poly nl = polyMul(nf, b_fb);
            Poly dl = polyMul(df, a_fb);
            num = polyTrim(polyMul(nf, a_fb));
            den = polyTrim(polyAdd(dl, nl));
        }

        auto z_act_poles = solvePolynomialRoots(den);
        res.max_z_pole_mag = 0.0;
        for (const auto& zp : z_act_poles) res.max_z_pole_mag = std::max(res.max_z_pole_mag, std::abs(zp));
        res.cl_stable = (res.max_z_pole_mag < 1.0 - 1e-5);

        double w_nyq = PI / ts;
        double w0 = std::max(1e-4, w_nyq * 1e-3);
        double log_min = std::log10(w0), log_max = std::log10(w_nyq);

        res.w.resize(n_pts);
        res.mag_abs.resize(n_pts);
        res.mag_db.resize(n_pts);
        res.phase_deg.resize(n_pts);
        res.c_vals.resize(n_pts);

        double prev_rad = 0.0, unwrap_offset = 0.0;
        for (int i = 0; i < n_pts; ++i) {
            res.w[i] = std::pow(10.0, log_min + (log_max - log_min) * double(i) / double(n_pts - 1));
            Complex val = evalRatZ(num, den, res.w[i], ts);
            res.c_vals[i]  = val;
            double m = std::abs(val);
            res.mag_abs[i] = m;
            res.mag_db[i]  = 20.0 * std::log10(std::max(m, 1e-15));

            double rad = std::arg(val);
            if (i > 0) {
                double diff = rad - prev_rad;
                if (diff > PI) unwrap_offset -= 2.0 * PI;
                else if (diff < -PI) unwrap_offset += 2.0 * PI;
            }
            prev_rad = rad;
            res.phase_deg[i] = (rad + unwrap_offset) * (180.0 / PI);
        }

        // Discrete Ghost overlay when Discrete PID is active
        if (include_pid) {
            res.has_ghost = true;
            res.ghost_mag_db.resize(n_pts);
            res.ghost_phase_deg.resize(n_pts);
            res.ghost_c_vals.resize(n_pts);
            double gp_prev = 0.0, gp_off = 0.0;
            for (int i = 0; i < n_pts; ++i) {
                Complex gv = evalRatZ(b_h, a_h, res.w[i], ts);
                res.ghost_c_vals[i] = gv;
                res.ghost_mag_db[i] = 20.0 * std::log10(std::max(std::abs(gv), 1e-15));
                double gr = std::arg(gv);
                if (i > 0) {
                    double d = gr - gp_prev;
                    if (d > PI) gp_off -= 2.0 * PI;
                    else if (d < -PI) gp_off += 2.0 * PI;
                }
                gp_prev = gr;
                res.ghost_phase_deg[i] = (gr + gp_off) * (180.0 / PI);
            }
        }

        // True Discrete DC Gain at z = 1 (w = 0)
        Complex d_at_1 = polyVal(den, Complex(1.0, 0.0));
        Complex n_at_1 = polyVal(num, Complex(1.0, 0.0));
        if (std::abs(d_at_1) > 1e-10 && std::abs(n_at_1) > 1e-12) {
            res.dc_gain_db = 20.0 * std::log10(std::abs(n_at_1 / d_at_1));
        }

        res.peak_mag_db = -1e30;
        for (int i = 0; i < n_pts; ++i) {
            if (res.mag_db[i] > res.peak_mag_db) {
                res.peak_mag_db = res.mag_db[i];
                res.w_peak = res.w[i];
            }
        }
        res.mr_db = res.peak_mag_db;

        double ref_db = res.dc_gain_db.value_or(res.peak_mag_db);
        // For highpass / bandpass filters where DC gain is deeply attenuated, reference -3 dB to passband peak
        if (res.peak_mag_db - ref_db > 6.0) ref_db = res.peak_mag_db;
        res.bw_ref_db = ref_db - 3.01029995664;
        for (int i = 0; i + 1 < n_pts; ++i) {
            if ((res.mag_db[i] >= res.bw_ref_db && res.mag_db[i + 1] < res.bw_ref_db) ||
                (res.mag_db[i] <= res.bw_ref_db && res.mag_db[i + 1] > res.bw_ref_db)) {
                double frac = (res.mag_db[i] - res.bw_ref_db) /
                              ((std::abs(res.mag_db[i] - res.mag_db[i + 1]) > 1e-12) ? (res.mag_db[i] - res.mag_db[i + 1]) : 1.0);
                res.w_bw = res.w[i] + std::clamp(frac, 0.0, 1.0) * (res.w[i + 1] - res.w[i]);
                break;
            }
        }

        // Exact Discrete Cursor Evaluation
        res.cursor_w = std::clamp(cursor_w, res.w.front(), res.w.back());
        res.cursor_c = evalRatZ(num, den, res.cursor_w, ts);
        res.cursor_mag_db = 20.0 * std::log10(std::max(std::abs(res.cursor_c), 1e-15));
        int near_i = 0;
        double best_d = 1e30;
        for (int i = 0; i < n_pts; ++i) {
            double d = std::abs(std::log10(res.w[i]) - std::log10(res.cursor_w));
            if (d < best_d) { best_d = d; near_i = i; }
        }
        double near_rad = res.phase_deg[near_i] * (PI / 180.0);
        double cur_rad  = std::arg(res.cursor_c);
        res.cursor_phase_deg = (near_rad + std::remainder(cur_rad - near_rad, 2.0 * PI)) * (180.0 / PI);
        return res;
    }
};

class BodeNyquistCanvas : public QWidget {
    Q_OBJECT
public:
    bool nyquist_mode = false;
    bool show_asymptotes = true;
    bool show_margins = true;
    bool show_mirror_nyquist = true;
    bool show_ghost = true;
    std::optional<FreqResponseResult> freq_res;
    std::function<void(double)> onFrequencySelected;

    // Nyquist Pan/Zoom bounds
    bool nyq_user_bounds = false;
    double n_min_r = -2.5, n_max_r = 2.5, n_min_i = -2.5, n_max_i = 2.5;

    // Bode vertical Y-zoom/pan bounds
    bool bode_user_bounds = false;
    double b_db_min = -60.0, b_db_max = 40.0;
    double b_ph_min = -270.0, b_ph_max = 0.0;

    bool panning = false;
    bool drag_moved = false;
    QPoint pan_start;
    QPoint press_px;

    explicit BodeNyquistCanvas(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumSize(260, 260);
        setCursor(Qt::CrossCursor);
    }

    void resetView() {
        nyq_user_bounds  = false;
        bode_user_bounds = false;
        n_min_r = -2.5; n_max_r = 2.5; n_min_i = -2.5; n_max_i = 2.5;
        update();
    }

    void zoomKeyboard(bool zoom_in) {
        double factor = zoom_in ? 0.82 : 1.22;
        if (nyquist_mode) {
            ensureNyquistUserBounds();
            double mr = 0.5 * (n_min_r + n_max_r), mi = 0.5 * (n_min_i + n_max_i);
            double sr = std::clamp((n_max_r - n_min_r) * factor * 0.5, 1e-3, 1e5);
            double si = std::clamp((n_max_i - n_min_i) * factor * 0.5, 1e-3, 1e5);
            n_min_r = mr - sr; n_max_r = mr + sr;
            n_min_i = mi - si; n_max_i = mi + si;
        } else {
            ensureBodeUserBounds();
            double mc = 0.5 * (b_db_min + b_db_max), ms = std::clamp((b_db_max - b_db_min) * factor * 0.5, 2.0, 400.0);
            double pc = 0.5 * (b_ph_min + b_ph_max), ps = std::clamp((b_ph_max - b_ph_min) * factor * 0.5, 10.0, 1440.0);
            b_db_min = mc - ms; b_db_max = mc + ms;
            b_ph_min = pc - ps; b_ph_max = pc + ps;
        }
        update();
    }

    void panByFraction(double fx, double fy) {
        if (nyquist_mode) {
            ensureNyquistUserBounds();
            double dr = fx * (n_max_r - n_min_r);
            double di = fy * (n_max_i - n_min_i);
            n_min_r += dr; n_max_r += dr;
            n_min_i += di; n_max_i += di;
        } else {
            ensureBodeUserBounds();
            double d_db = fy * (b_db_max - b_db_min);
            double d_ph = fy * (b_ph_max - b_ph_min);
            b_db_min += d_db; b_db_max += d_db;
            b_ph_min += d_ph; b_ph_max += d_ph;
        }
        update();
    }

protected:
    void computeAutoBodeBounds(const FreqResponseResult& r, double& db_min, double& db_max, double& ph_min, double& ph_max) const {
        db_min = -40.0; db_max = 20.0;
        ph_min = -180.0; ph_max = 0.0;
        bool has_m = false, has_p = false;

        auto accumMag = [&](double v) {
            if (!std::isfinite(v) || std::abs(v) > 260.0) return;
            if (!has_m) { db_min = std::min(v, 0.0); db_max = std::max(v, 0.0); has_m = true; }
            else { db_min = std::min(db_min, v); db_max = std::max(db_max, v); }
        };
        auto accumPh = [&](double v) {
            if (!std::isfinite(v) || std::abs(v) > 1440.0) return;
            if (!has_p) { ph_min = v; ph_max = v; has_p = true; }
            else { ph_min = std::min(ph_min, v); ph_max = std::max(ph_max, v); }
        };

        for (size_t i = 0; i < r.w.size(); ++i) {
            accumMag(r.mag_db[i]);
            accumPh(r.phase_deg[i]);
            if (show_ghost && r.has_ghost && i < r.ghost_mag_db.size()) {
                accumMag(r.ghost_mag_db[i]);
                accumPh(r.ghost_phase_deg[i]);
            }
        }
        if (r.mode_idx == 0) {
            // Keep -180° (or active PM reference) in view for open-loop margin inspection
            accumPh(r.w_gc.has_value() ? r.pm_ref_deg : -180.0);
        }

        db_min = std::floor((db_min - 10.0) / 20.0) * 20.0;
        db_max = std::ceil((db_max + 10.0) / 20.0) * 20.0;
        if (db_max - db_min < 40.0) db_max = db_min + 40.0;

        ph_min = std::floor((ph_min - 20.0) / 45.0) * 45.0;
        ph_max = std::ceil((ph_max + 20.0) / 45.0) * 45.0;
        if (ph_max - ph_min < 90.0) ph_max = ph_min + 90.0;
    }

    void computeAutoNyquistBounds(const FreqResponseResult& r, double& r0, double& r1, double& i0, double& i1) const {
        double min_re = -1.35, max_re = 1.35, min_im = -1.35, max_im = 1.35;
        auto accumC = [&](Complex c) {
            if (!std::isfinite(c.real()) || !std::isfinite(c.imag())) return;
            if (std::abs(c) > 10.0) return; // Ignore asymptotic tails to infinity during auto-fit
            min_re = std::min(min_re, c.real()); max_re = std::max(max_re, c.real());
            min_im = std::min(min_im, c.imag()); max_im = std::max(max_im, c.imag());
            if (show_mirror_nyquist) {
                min_im = std::min(min_im, -c.imag());
                max_im = std::max(max_im, -c.imag());
            }
        };
        for (const auto& c : r.c_vals) accumC(c);
        if (show_ghost && r.has_ghost) {
            for (const auto& c : r.ghost_c_vals) accumC(c);
        }
        double pad_r = 0.16 * std::max(1.0, max_re - min_re);
        double pad_i = 0.16 * std::max(1.0, max_im - min_im);
        r0 = min_re - pad_r; r1 = max_re + pad_r;
        i0 = min_im - pad_i; i1 = max_im + pad_i;
    }

    void ensureNyquistUserBounds() {
        if (!nyq_user_bounds && freq_res.has_value()) {
            n_min_r = last_nyq_r0; n_max_r = last_nyq_r1;
            n_min_i = last_nyq_i0; n_max_i = last_nyq_i1;
            nyq_user_bounds = true;
        }
    }

    void ensureBodeUserBounds() {
        if (!bode_user_bounds && freq_res.has_value()) {
            computeAutoBodeBounds(freq_res.value(), b_db_min, b_db_max, b_ph_min, b_ph_max);
            bode_user_bounds = true;
        }
    }

    static void drawChevronArrow(QPainter& p, QPointF tip, double angle_rad, double len, const QColor& col) {
        double wing = 0.48; // ~27.5 deg half-angle
        QPointF p1(tip.x() - len * std::cos(angle_rad - wing), tip.y() - len * std::sin(angle_rad - wing));
        QPointF p2(tip.x() - len * std::cos(angle_rad + wing), tip.y() - len * std::sin(angle_rad + wing));
        QPainterPath tri;
        tri.moveTo(tip); tri.lineTo(p1); tri.lineTo(p2); tri.closeSubpath();
        p.save();
        p.setPen(QPen(QColor(Theme::BG_PLOT), 1.0));
        p.setBrush(col);
        p.drawPath(tri);
        p.restore();
    }

    static void drawBoundedBadge(QPainter& p, const QRectF& pane, QPointF anchor, const QString& txt, const QColor& fg, double dy_pref = -10.0) {
        QFontMetricsF fm(p.font());
        double tw = fm.horizontalAdvance(txt) + 10.0;
        double th = fm.height() + 4.0;
        double x = anchor.x() + 8.0;
        if (x + tw > pane.right() - 4.0) x = anchor.x() - tw - 8.0;
        x = std::clamp(x, pane.left() + 4.0, std::max(pane.left() + 4.0, pane.right() - tw - 4.0));

        double y = (dy_pref < 0.0) ? (anchor.y() - th - 4.0) : (anchor.y() + 6.0);
        if (y < pane.top() + 3.0) y = anchor.y() + 6.0;
        if (y + th > pane.bottom() - 3.0) y = anchor.y() - th - 4.0;
        y = std::clamp(y, pane.top() + 2.0, std::max(pane.top() + 2.0, pane.bottom() - th - 2.0));

        QRectF box(x, y, tw, th);
        QColor bg(Theme::BG_PANEL);
        bg.setAlpha(215);
        p.save();
        p.setPen(QPen(fg, 1.0));
        p.setBrush(bg);
        p.drawRoundedRect(box, 3.5, 3.5);
        p.setPen(fg);
        p.drawText(box, Qt::AlignCenter, txt);
        p.restore();
    }

    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.fillRect(rect(), QColor(Theme::BG_PLOT));
        if (!freq_res.has_value() || freq_res->w.empty()) return;

        if (!nyquist_mode) drawBodePlot(p, freq_res.value());
        else drawNyquistPlot(p, freq_res.value());
    }

    void drawBodePlot(QPainter& p, const FreqResponseResult& r) {
        double pad_l = 62.0, pad_r = 22.0, pad_t = 18.0, pad_b = 34.0, gap = 24.0;
        double ew = std::max(20.0, width() - pad_l - pad_r);
        double eh_tot = std::max(40.0, height() - pad_t - pad_b - gap);
        double eh_mag = eh_tot * 0.52;
        double eh_ph  = eh_tot * 0.48;

        QRectF rect_mag(pad_l, pad_t, ew, eh_mag);
        QRectF rect_ph(pad_l, pad_t + eh_mag + gap, ew, eh_ph);

        double l_min = std::log10(r.w.front());
        double l_max = std::log10(r.w.back());

        double db_min, db_max, ph_min, ph_max;
        if (bode_user_bounds) {
            db_min = b_db_min; db_max = b_db_max;
            ph_min = b_ph_min; ph_max = b_ph_max;
        } else {
            computeAutoBodeBounds(r, db_min, db_max, ph_min, ph_max);
        }

        auto toMagPx = [&](double w, double db) {
            double x = rect_mag.left() + (std::log10(w) - l_min) / std::max(l_max - l_min, 1e-9) * rect_mag.width();
            double y = rect_mag.top()  + (db_max - db) / std::max(db_max - db_min, 1e-9) * rect_mag.height();
            return QPointF(x, y);
        };
        auto toPhPx = [&](double w, double deg) {
            double x = rect_ph.left() + (std::log10(w) - l_min) / std::max(l_max - l_min, 1e-9) * rect_ph.width();
            double y = rect_ph.top()  + (ph_max - deg) / std::max(ph_max - ph_min, 1e-9) * rect_ph.height();
            return QPointF(x, y);
        };

        // Logarithmic vertical gridlines (decades + sub-decades)
        p.setFont(QFont("Monospace", 8));
        for (int dec = static_cast<int>(std::floor(l_min)); dec <= static_cast<int>(std::ceil(l_max)); ++dec) {
            for (int m = 1; m <= 9; ++m) {
                double w_v = std::pow(10.0, dec) * m;
                double lw = std::log10(w_v);
                if (lw < l_min - 1e-9 || lw > l_max + 1e-9) continue;
                double x = toMagPx(w_v, 0.0).x();
                p.setPen(QPen(QColor(Theme::GRID_MAJOR), m == 1 ? 1.2 : 0.6));
                p.drawLine(QPointF(x, rect_mag.top()), QPointF(x, rect_mag.bottom()));
                p.drawLine(QPointF(x, rect_ph.top()),  QPointF(x, rect_ph.bottom()));
                if (m == 1) {
                    p.setPen(QColor(Theme::TEXT_SECONDARY));
                    p.drawText(QRectF(x - 30, rect_ph.bottom() + 5, 60, 18), Qt::AlignCenter, QString("10^%1").arg(dec));
                }
            }
        }
        p.setPen(QColor(Theme::TEXT_MUTED));
        p.drawText(QRectF(rect_ph.right() - 120, rect_ph.bottom() + 18, 120, 14), Qt::AlignRight, "ω (rad/s)");

        // Horizontal Magnitude Grid
        double db_span = db_max - db_min;
        double db_step = (db_span > 160.0) ? 40.0 : ((db_span < 35.0) ? 5.0 : ((db_span < 70.0) ? 10.0 : 20.0));
        for (double db = std::ceil(db_min / db_step) * db_step; db <= db_max + 1e-6; db += db_step) {
            double y = toMagPx(r.w.front(), db).y();
            bool is_zero_db = std::abs(db) < 1e-3;
            p.setPen(QPen(QColor(is_zero_db ? Theme::AXIS_LINE : Theme::GRID_MAJOR), is_zero_db ? 1.6 : 1.0));
            p.drawLine(QPointF(rect_mag.left(), y), QPointF(rect_mag.right(), y));
            p.setPen(QColor(Theme::TEXT_SECONDARY));
            p.drawText(QRectF(2, y - 8, pad_l - 6, 16), Qt::AlignRight | Qt::AlignVCenter, QString("%1 dB").arg(db, 0, 'f', 0));
        }

        // Horizontal Phase Grid
        double ph_span = ph_max - ph_min;
        double ph_step = (ph_span > 540.0) ? 90.0 : ((ph_span < 120.0) ? 15.0 : 45.0);
        for (double deg = std::ceil(ph_min / ph_step) * ph_step; deg <= ph_max + 1e-6; deg += ph_step) {
            double y = toPhPx(r.w.front(), deg).y();
            bool is_odd_180 = (r.mode_idx == 0) && (std::abs(std::remainder(deg - 180.0, 360.0)) < 1e-2);
            p.setPen(QPen(QColor(is_odd_180 ? Theme::ACCENT_POLE : Theme::GRID_MAJOR),
                          is_odd_180 ? 1.3 : 1.0,
                          is_odd_180 ? Qt::DashLine : Qt::SolidLine));
            p.drawLine(QPointF(rect_ph.left(), y), QPointF(rect_ph.right(), y));
            if (static_cast<int>(std::round(deg)) % 90 == 0 || is_odd_180 || ph_step <= 15.0) {
                p.setPen(QColor(is_odd_180 ? Theme::ACCENT_POLE : Theme::TEXT_SECONDARY));
                p.drawText(QRectF(2, y - 8, pad_l - 6, 16), Qt::AlignRight | Qt::AlignVCenter, QString("%1°").arg(deg, 0, 'f', 0));
            }
        }

        // Pane Borders
        p.setPen(QPen(QColor(Theme::BORDER_PANEL), 1.0));
        p.setBrush(Qt::NoBrush);
        p.drawRect(rect_mag);
        p.drawRect(rect_ph);

        // -------------------- TOP PANE: MAGNITUDE (dB) --------------------
        p.save();
        p.setClipRect(rect_mag);

        // 1. Uncompensated Ghost Magnitude Curve
        if (show_ghost && r.has_ghost && r.ghost_mag_db.size() == r.w.size()) {
            QPainterPath g_mag;
            for (size_t i = 0; i < r.w.size(); ++i) {
                QPointF pt = toMagPx(r.w[i], r.ghost_mag_db[i]);
                if (i == 0) g_mag.moveTo(pt); else g_mag.lineTo(pt);
            }
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor(Theme::TEXT_MUTED), 1.6, Qt::DashLine));
            p.drawPath(g_mag);
        }

        // 2. Straight-Line Magnitude Asymptotes + Corner Break Dots
        if (show_asymptotes && r.mode_idx != 2 && r.asymp_db.size() == r.w.size()) {
            QPainterPath asymp_path;
            for (size_t i = 0; i < r.w.size(); ++i) {
                QPointF pt = toMagPx(r.w[i], r.asymp_db[i]);
                if (i == 0) asymp_path.moveTo(pt); else asymp_path.lineTo(pt);
            }
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor(Theme::ACCENT_OPEN), 1.5, Qt::DashLine));
            p.drawPath(asymp_path);

            p.setPen(QPen(QColor(Theme::BG_PLOT), 1.0));
            p.setBrush(QColor(Theme::ACCENT_OPEN));
            for (double wc : r.corner_freqs) {
                if (std::log10(wc) < l_min || std::log10(wc) > l_max) continue;
                // Interpolate asymptote dB at wc
                auto it = std::lower_bound(r.w.begin(), r.w.end(), wc);
                size_t idx = (it == r.w.end()) ? (r.w.size() - 1) : static_cast<size_t>(std::distance(r.w.begin(), it));
                QPointF cpt = toMagPx(wc, r.asymp_db[idx]);
                p.drawEllipse(cpt, 3.2, 3.2);
            }
        }

        // 3. Primary Exact Magnitude Curve
        QPainterPath mag_path;
        for (size_t i = 0; i < r.w.size(); ++i) {
            QPointF pt = toMagPx(r.w[i], r.mag_db[i]);
            if (i == 0) mag_path.moveTo(pt); else mag_path.lineTo(pt);
        }
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(QColor(Theme::ACCENT_ZERO), 2.3));
        p.drawPath(mag_path);
        p.restore();

        // -------------------- BOTTOM PANE: PHASE (deg) --------------------
        p.save();
        p.setClipRect(rect_ph);

        // 1. Uncompensated Ghost Phase Curve
        if (show_ghost && r.has_ghost && r.ghost_phase_deg.size() == r.w.size()) {
            QPainterPath g_ph;
            for (size_t i = 0; i < r.w.size(); ++i) {
                QPointF pt = toPhPx(r.w[i], r.ghost_phase_deg[i]);
                if (i == 0) g_ph.moveTo(pt); else g_ph.lineTo(pt);
            }
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor(Theme::TEXT_MUTED), 1.6, Qt::DashLine));
            p.drawPath(g_ph);
        }

        // 2. Straight-Line Phase Asymptotes (±45°/dec over [0.1*wc, 10*wc])
        if (show_asymptotes && r.mode_idx != 2 && r.asymp_phase_deg.size() == r.w.size()) {
            QPainterPath ph_asymp_path;
            for (size_t i = 0; i < r.w.size(); ++i) {
                QPointF pt = toPhPx(r.w[i], r.asymp_phase_deg[i]);
                if (i == 0) ph_asymp_path.moveTo(pt); else ph_asymp_path.lineTo(pt);
            }
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor(Theme::ACCENT_OPEN), 1.5, Qt::DashLine));
            p.drawPath(ph_asymp_path);
        }

        // 3. Primary Exact Phase Curve
        QPainterPath ph_path;
        for (size_t i = 0; i < r.w.size(); ++i) {
            QPointF pt = toPhPx(r.w[i], r.phase_deg[i]);
            if (i == 0) ph_path.moveTo(pt); else ph_path.lineTo(pt);
        }
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(QColor(Theme::ACCENT_LOCUS), 2.2));
        p.drawPath(ph_path);
        p.restore();

        // -------------------- CALLOUT OVERLAYS: GM/PM (Mode 0) OR Mr/-3dB BW (Modes 1 & 2) --------------------
        p.setFont(QFont("Monospace", 8, QFont::Bold));
        if (show_margins) {
            if (r.mode_idx == 0) {
                // Open-Loop Gain Crossover -> Phase Margin
                if (r.w_gc.has_value() && r.pm_deg.has_value()) {
                    double wgc = r.w_gc.value();
                    double lw = std::log10(wgc);
                    if (lw >= l_min && lw <= l_max) {
                        QPointF m0     = toMagPx(wgc, 0.0);
                        QPointF p_ref  = toPhPx(wgc, r.pm_ref_deg);
                        QPointF p_act  = toPhPx(wgc, r.pm_ref_deg + r.pm_deg.value());
                        p.setPen(QPen(QColor(Theme::ACCENT_PID), 1.3, Qt::DotLine));
                        p.drawLine(m0, p_act);
                        p.setPen(QPen(QColor(Theme::ACCENT_PID), 2.5));
                        p.drawLine(p_ref, p_act);
                        p.setBrush(QColor(Theme::ACCENT_PID));
                        p.drawEllipse(m0, 4.2, 4.2);
                        p.drawEllipse(p_act, 4.2, 4.2);
                        drawBoundedBadge(p, rect_ph, p_act,
                            QString("PM=%1° @ ωgc=%2")
                                .arg(QString::fromStdString(PRECISION.fmt(r.pm_deg, true)))
                                .arg(QString::fromStdString(PRECISION.fmt(wgc))),
                            QColor(Theme::ACCENT_PID), -10.0);
                    }
                }
                // Open-Loop Phase Crossover -> Gain Margin
                if (r.w_pc.has_value() && r.gm_db.has_value()) {
                    double wpc = r.w_pc.value();
                    double lw = std::log10(wpc);
                    if (lw >= l_min && lw <= l_max) {
                        QPointF m0    = toMagPx(wpc, 0.0);
                        QPointF m_act = toMagPx(wpc, -r.gm_db.value());
                        QPointF p_ref = toPhPx(wpc, r.pc_ref_deg);
                        p.setPen(QPen(QColor(Theme::ACCENT_POLE), 1.3, Qt::DotLine));
                        p.drawLine(m_act, p_ref);
                        p.setPen(QPen(QColor(Theme::ACCENT_POLE), 2.5));
                        p.drawLine(m0, m_act);
                        p.setBrush(QColor(Theme::ACCENT_POLE));
                        p.drawEllipse(m_act, 4.2, 4.2);
                        p.drawEllipse(p_ref, 4.2, 4.2);
                        drawBoundedBadge(p, rect_mag, m_act,
                            QString("GM=%1 dB @ ωpc=%2")
                                .arg(QString::fromStdString(PRECISION.fmt(r.gm_db, true)))
                                .arg(QString::fromStdString(PRECISION.fmt(wpc))),
                            QColor(Theme::ACCENT_POLE), 10.0);
                    }
                }
            } else {
                // Closed-Loop T(jw) or Discrete H(e^{jwTs}): -3 dB Bandwidth & Resonant Peak Mr Callouts
                p.save();
                p.setClipRect(rect_mag);
                double y_bw = toMagPx(r.w.front(), r.bw_ref_db).y();
                p.setPen(QPen(QColor(Theme::ACCENT_PID), 1.2, Qt::DashDotLine));
                p.drawLine(QPointF(rect_mag.left(), y_bw), QPointF(rect_mag.right(), y_bw));
                p.restore();

                if (r.w_bw.has_value()) {
                    double wbw = r.w_bw.value();
                    double lw = std::log10(wbw);
                    if (lw >= l_min && lw <= l_max) {
                        QPointF pt_bw = toMagPx(wbw, r.bw_ref_db);
                        p.setPen(QPen(QColor(Theme::ACCENT_PID), 1.4, Qt::DotLine));
                        p.drawLine(pt_bw, QPointF(pt_bw.x(), rect_mag.bottom()));
                        p.setBrush(QColor(Theme::ACCENT_PID));
                        p.drawEllipse(pt_bw, 4.4, 4.4);
                        drawBoundedBadge(p, rect_mag, pt_bw,
                            QString("-3dB BW: ω=%1").arg(QString::fromStdString(PRECISION.fmt(wbw))),
                            QColor(Theme::ACCENT_PID), 10.0);
                    }
                }
                if (r.w_peak > 0.0) {
                    double lwp = std::log10(r.w_peak);
                    if (lwp >= l_min && lwp <= l_max) {
                        QPointF pt_pk = toMagPx(r.w_peak, r.mr_db);
                        p.setPen(QPen(QColor(Theme::ACCENT_ARROW), 1.8));
                        p.setBrush(QColor(Theme::ACCENT_ARROW));
                        p.drawEllipse(pt_pk, 4.2, 4.2);
                        drawBoundedBadge(p, rect_mag, pt_pk,
                            QString("Mr=%1 dB @ %2").arg(QString::fromStdString(PRECISION.fmt(r.mr_db)))
                                                    .arg(QString::fromStdString(PRECISION.fmt(r.w_peak))),
                            QColor(Theme::ACCENT_ARROW), -10.0);
                    }
                }
            }
        }

        // -------------------- INTERACTIVE FREQUENCY CURSOR --------------------
        double l_cur = std::log10(r.cursor_w);
        if (l_cur >= l_min - 1e-6 && l_cur <= l_max + 1e-6) {
            QPointF cm = toMagPx(r.cursor_w, r.cursor_mag_db);
            QPointF cp = toPhPx(r.cursor_w, r.cursor_phase_deg);
            p.setPen(QPen(QColor(Theme::ACCENT_PID), 1.0, Qt::DashLine));
            p.drawLine(QPointF(cm.x(), rect_mag.top()), QPointF(cm.x(), rect_ph.bottom()));

            if (rect_mag.contains(cm)) {
                p.setBrush(QColor(Theme::BG_PLOT));
                p.setPen(QPen(QColor(Theme::ACCENT_ZERO), 2.2));
                p.drawEllipse(cm, 5.0, 5.0);
                drawBoundedBadge(p, rect_mag, cm,
                    QString("ω=%1 r/s | %2 dB")
                        .arg(QString::fromStdString(PRECISION.fmt(r.cursor_w)))
                        .arg(QString::fromStdString(PRECISION.fmt(r.cursor_mag_db))),
                    QColor(Theme::ACCENT_ZERO), -10.0);
            }
            if (rect_ph.contains(cp)) {
                p.setBrush(QColor(Theme::BG_PLOT));
                p.setPen(QPen(QColor(Theme::ACCENT_LOCUS), 2.2));
                p.drawEllipse(cp, 5.0, 5.0);
                drawBoundedBadge(p, rect_ph, cp,
                    QString("∠=%1°").arg(QString::fromStdString(PRECISION.fmt(r.cursor_phase_deg))),
                    QColor(Theme::ACCENT_LOCUS), -10.0);
            }
        }
    }

    void drawNyquistPlot(QPainter& p, const FreqResponseResult& r) {
        double pad = 26.0;
        double ew = std::max(20.0, width() - 2.0 * pad);
        double eh = std::max(20.0, height() - 2.0 * pad);

        double base_r0 = n_min_r, base_r1 = n_max_r, base_i0 = n_min_i, base_i1 = n_max_i;
        if (!nyq_user_bounds) {
            computeAutoNyquistBounds(r, base_r0, base_r1, base_i0, base_i1);
        }

        // Enforce 1:1 isotropic aspect ratio for Nyquist Unit Circle
        double span_r = std::max(1e-4, base_r1 - base_r0);
        double span_i = std::max(1e-4, base_i1 - base_i0);
        // Lock scale to vertical span once user-zoomed so splitter dragging keeps zoom constant
        double upp = nyq_user_bounds ? (span_i / eh) : std::max(span_r / ew, span_i / eh);
        double mr = 0.5 * (base_r0 + base_r1), mi = 0.5 * (base_i0 + base_i1);
        double r0 = mr - 0.5 * upp * ew, r1 = mr + 0.5 * upp * ew;
        double i0 = mi - 0.5 * upp * eh, i1 = mi + 0.5 * upp * eh;

        last_nyq_upp = upp;
        last_nyq_r0  = r0; last_nyq_r1 = r1;
        last_nyq_i0  = i0; last_nyq_i1 = i1;

        auto toPx = [&](Complex c) {
            double x = pad + (c.real() - r0) / (r1 - r0) * ew;
            double y = pad + (i1 - c.imag()) / (i1 - i0) * eh;
            return QPointF(x, y);
        };

        // Adaptive Grid & Axes
        double view_span = std::max(r1 - r0, i1 - i0);
        double raw_step = view_span / 8.0;
        double mag_p10 = std::pow(10.0, std::floor(std::log10(std::max(raw_step, 1e-6))));
        double norm_s = raw_step / mag_p10;
        double step = (norm_s < 1.5) ? mag_p10 : ((norm_s < 3.5) ? 2.0 * mag_p10 : ((norm_s < 7.5) ? 5.0 * mag_p10 : 10.0 * mag_p10));

        p.setFont(QFont("Monospace", 8));
        p.setPen(QPen(QColor(Theme::GRID_MAJOR), 1.0));
        for (double rv = std::floor(r0 / step) * step; rv <= r1; rv += step) {
            double x = toPx({rv, 0.0}).x();
            p.drawLine(QPointF(x, 0), QPointF(x, height()));
        }
        for (double iv = std::floor(i0 / step) * step; iv <= i1; iv += step) {
            double y = toPx({0.0, iv}).y();
            p.drawLine(QPointF(0, y), QPointF(width(), y));
        }

        QPointF orig = toPx({0.0, 0.0});
        p.setPen(QPen(QColor(Theme::AXIS_LINE), 1.5));
        p.drawLine(QPointF(0, orig.y()), QPointF(width(), orig.y()));
        p.drawLine(QPointF(orig.x(), 0), QPointF(orig.x(), height()));

        // Axis tick labels
        p.setPen(QColor(Theme::TEXT_MUTED));
        for (double rv = std::floor(r0 / step) * step; rv <= r1; rv += step) {
            if (std::abs(rv) < 1e-9) continue;
            double x = toPx({rv, 0.0}).x();
            double y_lbl = std::clamp(orig.y() + 4.0, 4.0, height() - 18.0);
            p.drawText(QRectF(x - 24, y_lbl, 48, 14), Qt::AlignCenter, QString::number(rv, 'g', 4));
        }
        for (double iv = std::floor(i0 / step) * step; iv <= i1; iv += step) {
            if (std::abs(iv) < 1e-9) continue;
            double y = toPx({0.0, iv}).y();
            double x_lbl = std::clamp(orig.x() + 5.0, 4.0, width() - 52.0);
            p.drawText(QRectF(x_lbl, y - 7, 48, 14), Qt::AlignLeft | Qt::AlignVCenter, QString("%1j").arg(iv, 0, 'g', 4));
        }

        // Unit Circle |L(jw)| = 1 (0 dB contour)
        QPointF uc_tl = toPx({-1.0, 1.0});
        QPointF uc_br = toPx({1.0, -1.0});
        p.setPen(QPen(QColor(Theme::ACCENT_ASYMP), 1.4, Qt::DashLine));
        p.setBrush(Qt::NoBrush);
        p.drawEllipse(QRectF(uc_tl, uc_br));

        // Critical Point (-1 + j0)
        QPointF crit_px = toPx({-1.0, 0.0});
        p.setPen(QPen(QColor(Theme::ACCENT_POLE), 2.5));
        p.drawLine(crit_px + QPointF(-6, -6), crit_px + QPointF(6, 6));
        p.drawLine(crit_px + QPointF(-6,  6), crit_px + QPointF(6, -6));
        p.setFont(QFont("Monospace", 9, QFont::Bold));
        p.drawText(crit_px + QPointF(-44, -8), "-1+j0");

        // Dynamic clipping radius & schematic Gamma_inf radius for origin poles (Type >= 1)
        double max_corner_dist = std::max({std::hypot(r0, i0), std::hypot(r0, i1), std::hypot(r1, i0), std::hypot(r1, i1)});
        double r_inf_vis = 0.42 * std::min(r1 - r0, i1 - i0);
        bool draw_inf_detour = show_mirror_nyquist && (r.mode_idx == 0) && (r.n_pole_orig > 0) &&
                               (std::abs(r.c_vals.front()) > r_inf_vis) && (r_inf_vis > 1.15);
        double clip_rad = std::max(50.0, 3.0 * max_corner_dist);

        // Helper to draw directional arrows spaced cleanly in screen pixels along a contour
        auto drawContourWithArrows = [&](const std::vector<Complex>& pts, bool conj_mirror, bool reverse_dir,
                                         const QColor& line_col, Qt::PenStyle style, double width_px,
                                         const QColor& arrow_col, double low_w_clip)
        {
            QPainterPath path;
            bool st = false;
            std::vector<QPointF> vis_px;
            vis_px.reserve(pts.size());

            // Determine where initial origin-pole tail drops inside low_w_clip
            size_t start_idx = 0;
            if (low_w_clip > 0.0) {
                while (start_idx + 1 < pts.size() && std::abs(pts[start_idx]) > low_w_clip) ++start_idx;
            }

            for (size_t idx = start_idx; idx < pts.size(); ++idx) {
                size_t i = reverse_dir ? (pts.size() - 1 - (idx - start_idx)) : idx;
                Complex c = conj_mirror ? std::conj(pts[i]) : pts[i];
                if (!std::isfinite(c.real()) || !std::isfinite(c.imag()) || std::abs(c) > clip_rad) {
                    st = false;
                    continue;
                }
                QPointF pt = toPx(c);
                if (!st) { path.moveTo(pt); st = true; }
                else path.lineTo(pt);
                if (rect().adjusted(-20, -20, 20, 20).contains(pt.toPoint())) {
                    if (vis_px.empty() || std::hypot(pt.x() - vis_px.back().x(), pt.y() - vis_px.back().y()) >= 3.0) {
                        vis_px.push_back(pt);
                    }
                }
            }
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(line_col, width_px, style));
            p.drawPath(path);

            // Place 2–4 evenly spaced arrowheads along visible contour
            double tot_px = 0.0;
            std::vector<double> cum_d(vis_px.size(), 0.0);
            for (size_t i = 1; i < vis_px.size(); ++i) {
                tot_px += std::hypot(vis_px[i].x() - vis_px[i - 1].x(), vis_px[i].y() - vis_px[i - 1].y());
                cum_d[i] = tot_px;
            }
            if (tot_px > 55.0) {
                int n_arr = std::clamp(static_cast<int>(tot_px / 140.0), 1, 4);
                for (int a = 1; a <= n_arr; ++a) {
                    double target_d = tot_px * double(a) / double(n_arr + 1);
                    auto it = std::lower_bound(cum_d.begin(), cum_d.end(), target_d);
                    size_t k = (it == cum_d.end()) ? (vis_px.size() - 1) : static_cast<size_t>(std::distance(cum_d.begin(), it));
                    if (k == 0) k = 1;
                    double dx = vis_px[k].x() - vis_px[k - 1].x();
                    double dy = vis_px[k].y() - vis_px[k - 1].y();
                    if (std::hypot(dx, dy) > 1.5) {
                        drawChevronArrow(p, vis_px[k], std::atan2(dy, dx), 8.5, arrow_col);
                    }
                }
            }
            return start_idx;
        };

        // 1. Uncompensated Ghost Nyquist Contour
        if (show_ghost && r.has_ghost && !r.ghost_c_vals.empty()) {
            QPainterPath g_nyq;
            bool st = false;
            for (const auto& c : r.ghost_c_vals) {
                if (!std::isfinite(c.real()) || !std::isfinite(c.imag()) || std::abs(c) > clip_rad) { st = false; continue; }
                QPointF pt = toPx(c);
                if (!st) { g_nyq.moveTo(pt); st = true; } else g_nyq.lineTo(pt);
            }
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor(Theme::TEXT_MUTED), 1.5, Qt::DashLine));
            p.drawPath(g_nyq);
        }

        double detour_clip = draw_inf_detour ? r_inf_vis : 0.0;

        // 2. Mirror Nyquist Contour (w < 0: sweeps -inf -> 0^-)
        if (show_mirror_nyquist) {
            drawContourWithArrows(r.c_vals, true, true,
                                  QColor(Theme::TEXT_MUTED), Qt::DashLine, 1.5,
                                  QColor(Theme::TEXT_MUTED), detour_clip);
        }

        // 3. Primary Nyquist Contour (w > 0: sweeps 0^+ -> +inf)
        size_t clip_i = drawContourWithArrows(r.c_vals, false, false,
                                              QColor(Theme::ACCENT_LOCUS), Qt::SolidLine, 2.3,
                                              QColor(Theme::ACCENT_ARROW), detour_clip);

        // 4. Schematic Clockwise Infinite-Radius Detour Gamma_inf for Origin Poles (s = 0)
        if (draw_inf_detour && clip_i < r.c_vals.size()) {
            Complex c_pos = r.c_vals[clip_i];
            Complex c_neg = std::conj(c_pos);
            double R_arc = std::abs(c_pos);
            double th_start = std::arg(c_neg);
            double th_end   = std::arg(c_pos);
            int q = r.n_pole_orig;
            // Exact CW angular sweep (-q * pi modulo 2*pi) from w = 0^- to w = 0^+
            double delta_cw = -double(q) * PI + std::remainder((th_end - th_start) - (-double(q) * PI), 2.0 * PI);

            const int n_arc = 90 * q;
            QPainterPath inf_path;
            QPointF mid_px, mid_prev_px;
            for (int k = 0; k <= n_arc; ++k) {
                double u = double(k) / double(n_arc);
                // Slight radial breathing when q >= 2 so multiple 180° CW turns don't overlap identically
                double r_u = R_arc * (1.0 + ((q >= 2) ? (0.08 * std::sin(PI * u)) : 0.0));
                double th  = th_start + u * delta_cw;
                QPointF pt = toPx( std::polar(r_u, th) );
                if (k == 0) inf_path.moveTo(pt); else inf_path.lineTo(pt);
                if (k == n_arc / 2) {
                    mid_px = pt;
                    double th_p = th_start + (double(k - 1) / double(n_arc)) * delta_cw;
                    mid_prev_px = toPx(std::polar(r_u, th_p));
                }
            }
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor(Theme::ACCENT_OPEN), 1.4, Qt::DotLine));
            p.drawPath(inf_path);
            if (std::hypot(mid_px.x() - mid_prev_px.x(), mid_px.y() - mid_prev_px.y()) > 0.5) {
                drawChevronArrow(p, mid_px, std::atan2(mid_px.y() - mid_prev_px.y(), mid_px.x() - mid_prev_px.x()),
                                 8.5, QColor(Theme::ACCENT_OPEN));
            }
            drawBoundedBadge(p, QRectF(8, 8, width() - 16, height() - 16), mid_px,
                             QString("Γ∞ (%1×180° CW, R→∞)").arg(q), QColor(Theme::ACCENT_OPEN), -10.0);
        }

        // 5. Gain & Phase Margin Vectors on Nyquist Polar (Mode 0)
        if (show_margins && r.mode_idx == 0) {
            if (r.w_gc.has_value() && r.pm_deg.has_value()) {
                double ph_rad = (r.pm_ref_deg + r.pm_deg.value()) * (PI / 180.0);
                Complex pm_pt = std::polar(1.0, ph_rad);
                QPointF pm_px = toPx(pm_pt);
                p.setPen(QPen(QColor(Theme::ACCENT_PID), 1.6, Qt::DotLine));
                p.drawLine(orig, pm_px);
                p.setBrush(QColor(Theme::ACCENT_PID));
                p.drawEllipse(pm_px, 4.2, 4.2);
            }
            if (r.w_pc.has_value() && r.gm_db.has_value()) {
                double mag_pc = std::pow(10.0, -r.gm_db.value() / 20.0);
                QPointF pc_px = toPx({-mag_pc, 0.0});
                p.setPen(QPen(QColor(Theme::ACCENT_POLE), 2.0));
                p.setBrush(QColor(Theme::ACCENT_POLE));
                p.drawEllipse(pc_px, 4.2, 4.2);
            }
        }

        // 6. Interactive Cursor Point on Nyquist Contour
        if (std::isfinite(r.cursor_c.real()) && std::isfinite(r.cursor_c.imag()) && std::abs(r.cursor_c) <= clip_rad) {
            QPointF c_px = toPx(r.cursor_c);
            p.setBrush(QColor(Theme::BG_PLOT));
            p.setPen(QPen(QColor(Theme::ACCENT_PID), 2.3));
            p.drawEllipse(c_px, 5.5, 5.5);
            p.setFont(QFont("Monospace", 8, QFont::Bold));
            drawBoundedBadge(p, QRectF(6, 6, width() - 12, height() - 12), c_px,
                QString("ω=%1 r/s (%2)")
                    .arg(QString::fromStdString(PRECISION.fmt(r.cursor_w)))
                    .arg(QString::fromStdString(PRECISION.fmtComplex(r.cursor_c))),
                QColor(Theme::ACCENT_PID), -10.0);
        }
    }

    void mousePressEvent(QMouseEvent* e) override {
        press_px   = e->pos();
        pan_start  = e->pos();
        drag_moved = false;

        if (!nyquist_mode && e->button() == Qt::LeftButton) {
            updateFreqFromBodePx(e->pos().x());
        } else if (nyquist_mode && (e->button() == Qt::LeftButton || e->button() == Qt::RightButton)) {
            ensureNyquistUserBounds();
            panning = true;
        } else if (!nyquist_mode && e->button() == Qt::RightButton) {
            ensureBodeUserBounds();
            panning = true;
        }
    }

    void mouseMoveEvent(QMouseEvent* e) override {
        if ((e->pos() - press_px).manhattanLength() > 4) {
            drag_moved = true;
            if (panning) setCursor(Qt::ClosedHandCursor);
        }

        if (!nyquist_mode && (e->buttons() & Qt::LeftButton)) {
            updateFreqFromBodePx(e->pos().x());
        } else if (nyquist_mode && panning && drag_moved) {
            double dr = -double(e->pos().x() - pan_start.x()) * last_nyq_upp;
            double di =  double(e->pos().y() - pan_start.y()) * last_nyq_upp;
            n_min_r += dr; n_max_r += dr;
            n_min_i += di; n_max_i += di;
            pan_start = e->pos();
            update();
        } else if (!nyquist_mode && panning && drag_moved) {
            double eh = std::max(40.0, height() - 76.0);
            double dy_frac = double(e->pos().y() - pan_start.y()) / (0.5 * eh);
            double d_db = dy_frac * (b_db_max - b_db_min);
            double d_ph = dy_frac * (b_ph_max - b_ph_min);
            b_db_min += d_db; b_db_max += d_db;
            b_ph_min += d_ph; b_ph_max += d_ph;
            pan_start = e->pos();
            update();
        }
    }

    void mouseReleaseEvent(QMouseEvent* e) override {
        if (nyquist_mode && e->button() == Qt::LeftButton && !drag_moved) {
            // Stationary left-click on Nyquist Polar snaps cursor ω to nearest point on contour!
            updateFreqFromNyquistPx(e->pos());
        }
        panning = false;
        setCursor(Qt::CrossCursor);
    }

    void wheelEvent(QWheelEvent* e) override {
        double factor = (e->angleDelta().y() > 0) ? 0.82 : 1.22;
        zoomKeyboard(factor < 1.0);
    }

private:
    double last_nyq_upp = 0.01;
    double last_nyq_r0 = -2.5, last_nyq_r1 = 2.5, last_nyq_i0 = -2.5, last_nyq_i1 = 2.5;

    void updateFreqFromBodePx(double px) {
        if (!freq_res.has_value() || freq_res->w.empty()) return;
        double pad_l = 62.0, pad_r = 22.0;
        double ew = std::max(20.0, width() - pad_l - pad_r);
        double frac = std::clamp((px - pad_l) / ew, 0.0, 1.0);
        double l_min = std::log10(freq_res->w.front());
        double l_max = std::log10(freq_res->w.back());
        double w_sel = std::pow(10.0, l_min + frac * (l_max - l_min));
        if (onFrequencySelected) onFrequencySelected(w_sel);
    }

    void updateFreqFromNyquistPx(const QPoint& click_pt) {
        if (!freq_res.has_value() || freq_res->w.empty()) return;
        const auto& r = freq_res.value();
        double pad = 26.0;
        double ew = std::max(20.0, width() - 2.0 * pad);
        double eh = std::max(20.0, height() - 2.0 * pad);

        double best_d2 = 1e30;
        size_t best_i = 0;
        for (size_t i = 0; i < r.c_vals.size(); ++i) {
            Complex c = r.c_vals[i];
            if (!std::isfinite(c.real()) || !std::isfinite(c.imag())) continue;
            double x = pad + (c.real() - last_nyq_r0) / std::max(last_nyq_r1 - last_nyq_r0, 1e-9) * ew;
            double y = pad + (last_nyq_i1 - c.imag()) / std::max(last_nyq_i1 - last_nyq_i0, 1e-9) * eh;
            double d2 = (x - click_pt.x()) * (x - click_pt.x()) + (y - click_pt.y()) * (y - click_pt.y());
            if (d2 < best_d2) { best_d2 = d2; best_i = i; }
        }
        if (best_d2 < 3600.0 && onFrequencySelected) { // Within 60 px of contour
            onFrequencySelected(r.w[best_i]);
        }
    }
};