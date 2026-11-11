#pragma once
#include "DiscreteTimeResponseTab.h"


// =============================================================================
// Engine 4 & Tab 5: Desmos-Style Scientific Calculator (With LaTeX Formula Sidebar)
// =============================================================================
struct FormulaFunctionSpec {
    std::string section_key; // "zeta", "wn_wd", "transient", "first_order", "rl_approx", "pid_ess", "discrete"
    std::string name;
    std::string call_template;
    std::string signature_hint;
    std::string title;
    std::string latex_formula;
    std::string description;
    std::string ref_badge;
};

static const std::vector<FormulaFunctionSpec> FORMULA_SHEET_FUNCS = {
    {"zeta", "solve_zeta_from_os", "solve_zeta_from_os(12.3)", "(OS_pct)", "Solve ζ from %OS (Overshoot)",
     "\\zeta = \\frac{-\\ln(\\%OS/100)}{\\sqrt{\\pi^2 + \\ln^2(\\%OS/100)}}", "Plug in percent overshoot %OS to solve for damping ratio ζ.", "Nise Eq. (4.39)"},
    {"zeta", "solve_zeta_from_Ts_wn", "solve_zeta_from_Ts_wn(2.0, 4.0)", "(Ts, wn)", "Solve ζ from Ts & ωn",
     "\\zeta = \\frac{4}{T_s \\omega_n}", "Plug in 2% settling time Ts and natural frequency ωn.", "Nise Eq. (4.42)"},
    {"zeta", "solve_zeta_from_Tp_wn", "solve_zeta_from_Tp_wn(1.0, 4.0)", "(Tp, wn)", "Solve ζ from Tp & ωn",
     "\\zeta = \\sqrt{1 - \\left(\\frac{\\pi}{\\omega_n T_p}\\right)^2}", "Plug in peak time Tp and natural frequency ωn.", "Nise Eq. (4.34)"},
    {"zeta", "solve_zeta_from_Ts_Tp", "solve_zeta_from_Ts_Tp(2.0, 1.0)", "(Ts, Tp)", "Solve ζ from Ts & Tp",
     "\\sigma_d = \\frac{4}{T_s}, \\quad \\omega_d = \\frac{\\pi}{T_p} \\Rightarrow \\zeta = \\frac{\\sigma_d}{\\sqrt{\\sigma_d^2 + \\omega_d^2}}", "Plug in Ts and Tp to solve for ζ.", "Nise Sec. 4.6"},
    {"zeta", "solve_zeta_from_ab", "solve_zeta_from_ab(2, 2)", "(a, b)", "Solve ζ from s² + a·s + b",
     "G(s) = \\frac{b}{s^2 + a s + b} \\Rightarrow \\zeta = \\frac{a}{2\\sqrt{b}}", "Solve for ζ directly from denominator coefficients (a, b).", "Nise Eq. (4.22)"},
    {"wn_wd", "solve_wn_from_b", "solve_wn_from_b(16)", "(b)", "Solve ωn from s² + a·s + b",
     "\\omega_n = \\sqrt{b}", "Natural frequency ωn = √b from constant denominator term.", "Nise Eq. (4.22)"},
    {"wn_wd", "solve_wn_from_Ts_zeta", "solve_wn_from_Ts_zeta(2.0, 0.7071)", "(Ts, zeta)", "Solve ωn from Ts & ζ",
     "\\omega_n = \\frac{4}{\\zeta T_s}", "Solve for natural frequency ωn from settling time Ts and ζ.", "Nise Eq. (4.42)"},
    {"wn_wd", "solve_wn_from_Tp_zeta", "solve_wn_from_Tp_zeta(1.0, 0.5)", "(Tp, zeta)", "Solve ωn from Tp & ζ",
     "\\omega_n = \\frac{\\pi}{T_p \\sqrt{1 - \\zeta^2}}", "Solve for ωn from peak time Tp and ζ.", "Nise Eq. (4.34)"},
    {"wn_wd", "solve_wd", "solve_wd(0.5, 4.0)", "(zeta, wn)", "Solve Damped Freq ωd",
     "\\omega_d = \\omega_n \\sqrt{1 - \\zeta^2}", "Compute damped oscillation frequency ωd (rad/s).", "Nise Eq. (4.24)"},
    {"wn_wd", "solve_sigma_d", "solve_sigma_d(0.7071, 4.0)", "(zeta, wn)", "Solve Decay Rate σd",
     "\\sigma_d = \\zeta \\omega_n = \\frac{4}{T_s}", "Compute real envelope attenuation rate σd = ζωn.", "Nise Eq. (4.45)"},
    {"transient", "solve_os", "solve_os(0.5)", "(zeta)", "Solve %OS from ζ",
     "\\%OS = e^{-\\left(\\frac{\\zeta \\pi}{\\sqrt{1 - \\zeta^2}}\\right)} \\times 100\\%", "Compute percent overshoot %OS from damping ratio ζ.", "Nise Eq. (4.38)"},
    {"transient", "solve_Ts", "solve_Ts(0.5, 4.0)", "(zeta, wn)", "Solve Settling Time Ts (±2%)",
     "T_s = \\frac{4}{\\zeta \\omega_n} = \\frac{4}{\\sigma_d}", "Compute 2% settling time Ts = 4/(ζωn).", "Nise Eq. (4.42)"},
    {"transient", "solve_Tp", "solve_Tp(0.5, 4.0)", "(zeta, wn)", "Solve Peak Time Tp",
     "T_p = \\frac{\\pi}{\\omega_n \\sqrt{1 - \\zeta^2}} = \\frac{\\pi}{\\omega_d}", "Compute peak time Tp = π/ωd.", "Nise Eq. (4.34)"},
    {"transient", "solve_Tr", "solve_Tr(0.375, 4.0)", "(zeta, wn)", "Solve Rise Time Tr (Nise Cubic)",
     "T_r \\approx \\frac{1.76\\zeta^3 - 0.417\\zeta^2 + 1.039\\zeta + 1}{\\omega_n}", "Compute 10–90% rise time Tr via Nise Fig. 4.16 cubic polynomial.", "Nise Fig. 4.16"},
    {"first_order", "solve_T_1st", "solve_T_1st(5)", "(a)", "Solve Time Constant T = 1/a",
     "T = \\frac{1}{a}, \\quad c(T) \\approx 0.632 c_{\\mathrm{final}}", "Time constant T = 1/a for first-order pole s = -a.", "Nise Eq. (4.4)"},
    {"first_order", "solve_Tr_1st", "solve_Tr_1st(5)", "(a)", "Solve 1st-Order Rise Time Tr = 2.2/a",
     "T_r = \\frac{\\ln(9)}{a} \\approx \\frac{2.2}{a}", "10%–90% rise time for first-order system.", "Nise Eq. (4.7)"},
    {"first_order", "solve_Ts_1st", "solve_Ts_1st(5)", "(a)", "Solve 1st-Order Settling Time Ts = 4/a",
     "T_s = \\frac{-\\ln(0.02)}{a} \\approx \\frac{4}{a}", "±2% settling time for first-order system.", "Nise Eq. (4.8)"},
    {"rl_approx", "check_5x_rule", "check_5x_rule(12, 0.5, 4.0)", "(alpha_r, zeta, wn)", "Check 5× Dominant-Pole Ratio",
     "\\mathrm{Ratio} = \\frac{|\\alpha_r|}{\\zeta \\omega_n} \\geq 5", "Checks Nise 5× dominant-pole approximation validity.", "Nise Sec. 4.7"},
    {"rl_approx", "solve_sigma_a", "solve_sigma_a(-6, -8, 1)", "(sum_p, sum_z, n_m)", "Root Locus Asymptote Centroid σa",
     "\\sigma_a = \\frac{\\sum p_j - \\sum z_i}{n - m}", "Computes Root Locus asymptote real-axis intercept σa.", "Nise Sec. 8.2"},
    {"rl_approx", "solve_deficiency", "solve_deficiency(135)", "(angle_G_deg)", "Compensator Angular Deficiency Δθ",
     "\\Delta\\theta = 180^\\circ - \\angle G(s_d)", "Phase lead required from compensator to place root locus at s_d.", "Nise Ch. 9"},
    {"pid_ess", "solve_ess_step", "solve_ess_step(4.0, 1.0)", "(Kp_pos, M)", "Solve Step Error e_ss from Kp,pos",
     "e_{\\mathrm{ss}}(\\mathrm{Step}) = \\frac{M}{1 + K_{p,\\mathrm{pos}}}", "Steady-state error for step input of magnitude M.", "Nise Sec. 7.3"},
    {"pid_ess", "solve_rel_err_pct", "solve_rel_err_pct(0.1996, 1.0)", "(ess, M)", "Solve Relative Error %e_ss",
     "\\%e_{\\mathrm{ss}} = \\frac{e_{\\mathrm{ss}}}{M} \\times 100\\%", "Relative steady-state error percentage (independent of M).", "A4 Q1(6,8)"},
    {"pid_ess", "solve_kp_for_os_p", "solve_kp_for_os_p(20.0, 5.0, 6.0, 1.0)", "(OS, a1, a0, K0)", "Design P-Controller Kp for %OS",
     "\\omega_n = \\frac{a_1}{2\\zeta}, \\quad K_p = \\frac{\\omega_n^2 - a_0}{K_0}", "Solves A4 Q1(5) P-controller gain Kp for target %OS.", "A4 Q1(5)"},
    {"pid_ess", "solve_kd_for_os_pd", "solve_kd_for_os_pd(10.0, 24.06, 5.0, 6.0, 1.0)", "(OS, Kp, a1, a0, K0)", "Design PD-Controller Kd (Fixed Kp)",
     "\\omega_n = \\sqrt{a_0 + K_0 K_p}, \\quad K_d = \\frac{2\\zeta\\omega_n - a_1}{K_0}", "Solves A4 Q2(3) PD derivative gain Kd with fixed Kp.", "A4 Q2(3)"},
    {"pid_ess", "solve_kp_for_os_pd", "solve_kp_for_os_pd(20.0, 1.483, 5.0, 6.0, 1.0)", "(OS, Kd, a1, a0, K0)", "Design PD-Controller Kp (Fixed Kd)",
     "\\omega_n = \\frac{a_1 + K_0 K_d}{2\\zeta}, \\quad K_p = \\frac{\\omega_n^2 - a_0}{K_0}", "Solves A4 Q2(5) PD proportional gain Kp with fixed Kd.", "A4 Q2(5)"},
    {"pid_ess", "solve_ki_for_os_pi", "solve_ki_for_os_pi(20.0, 1.0, 2.0, 1.0)", "(OS, Kp, a0, K0)", "Design PI-Controller Ki on 1st-Order Plant",
     "\\omega_n = \\frac{a_0 + K_0 K_p}{2\\zeta}, \\quad K_i = \\frac{\\omega_n^2}{K_0}", "Solves A4 Q4(4) PI integral gain Ki on K0/(s+a0).", "A4 Q4(4)"},
    {"discrete", "solve_z_from_s", "solve_z_from_s(-2 + 2i, 0.05)", "(s_pole, Ts)", "Map s-Pole to z-Plane z = e^(s·Ts)",
     "z = e^{s T_s} = e^{\\sigma T_s} e^{j\\omega T_s}", "Maps continuous s-pole to exact z-plane location.", "Nise Ch. 13"},
    {"discrete", "solve_tustin_z", "solve_tustin_z(-2 + 2i, 0.05)", "(s_val, Ts)", "Bilinear (Tustin) s → z Mapping",
     "z = \\frac{1 + s T_s / 2}{1 - s T_s / 2}", "Maps s-plane point to z-plane via Bilinear (Tustin) transform.", "Nise Ch. 13"},
     {"rl_approx", "parallel", "parallel(10, 20, 30 - 40i)", "(Z1, Z2, ...)", "Parallel Equivalent Impedance / Resistance",
     "Z_{\\mathrm{eq}} = \\left(\\sum_{k=1}^{N} \\frac{1}{Z_k}\\right)^{-1}", "Computes the equivalent parallel resistance or complex impedance of any number of arguments.", "Circuit & Impedance Analysis"}
};

struct CalcEvalOutput {
    bool ok = true;
    Complex val = {0.0, 0.0};
    std::string formatted_dec;
    std::string formatted_frac;
    std::string latex_preview;
    std::string error_msg;
};

class ScientificCalcEngine {
public:
    bool deg_mode = true;
    std::map<std::string, Complex> vars;
    SharedSystemModel* model = nullptr;
    std::string active_sym_name = "K";
    std::map<std::string, SymRat> sym_vars;

    explicit ScientificCalcEngine(SharedSystemModel* m = nullptr) : model(m) { resetConstants(); }

    void resetConstants() {
        vars["pi"]  = Complex(PI, 0.0);
        vars["e"]   = Complex(std::exp(1.0), 0.0);
        vars["tau"] = Complex(2.0 * PI, 0.0);
        vars["phi"] = Complex((1.0 + std::sqrt(5.0)) / 2.0, 0.0);
        vars["i"]   = Complex(0.0, 1.0);
        vars["j"]   = Complex(0.0, 1.0);

        active_sym_name = "K";
        sym_vars.clear();
        sym_vars["K"] = SymRat::variable();
    }

    void syncFromModel() {
        if (!model) return;
        vars["K_plant"] = model->plant_gain;
        vars["R_mag"]   = model->input_magnitude;
        vars["Kp"]      = model->kp;
        vars["Ki"]      = model->ki;
        vars["Kd"]      = model->kd;
        vars["Ts_d"]    = model->ts;
        vars["s_test"]  = model->test_point;
        vars["z_test"]  = model->z_test_point;
        if (model->last_sim_result.has_value()) {
            const auto& r = model->last_sim_result.value();
            vars["zeta"] = r.zeta.value_or(0.7071);
            vars["wn"]   = r.omega_n.value_or(2.8284);
            vars["wd"]   = r.omega_d.value_or(2.0);
            vars["sigma_d"] = r.sigma_d.value_or(2.0);
            vars["OS"]   = r.overshoot_pct.value_or(0.0);
            vars["Ts"]   = r.settling_time.value_or(0.0);
            vars["Tp"]   = r.peak_time;
            vars["Tr"]   = r.rise_time.value_or(0.0);
            vars["ess"]  = r.steady_state_error;
            vars["SSE"]  = r.steady_state_error;
            vars["rel_err_pct"] = r.relative_error_pct;
        } else {
            auto dm = computeDominantSMetrics(model->poles);
            vars["zeta"] = dm.zeta.value_or(0.7071);
            vars["wn"]   = dm.omega_n.value_or(2.8284);
            vars["wd"]   = dm.omega_d.value_or(2.0);
            vars["sigma_d"] = vars["zeta"].real() * vars["wn"].real();
        }
    }

    static std::string toLatexPreview(std::string s) {
        s = std::regex_replace(s, std::regex(R"(\bsqrt\(([^()]+)\))"), "\\sqrt{$1}");
        s = std::regex_replace(s, std::regex(R"(\bzeta\b)"), "\\zeta");
        s = std::regex_replace(s, std::regex(R"(\bwn\b)"), "\\omega_n");
        s = std::regex_replace(s, std::regex(R"(\bwd\b)"), "\\omega_d");
        s = std::regex_replace(s, std::regex(R"(\bsigma_d\b)"), "\\sigma_d");
        s = std::regex_replace(s, std::regex(R"(\bpi\b)"), "\\pi");
        return s;
    }

    static std::string formatBaseString(long long val, const std::string& mode) {
        unsigned long long uval = static_cast<unsigned long long>(val);
        auto toHex = [](unsigned long long v) {
            std::ostringstream oss;
            oss << "0x" << std::uppercase << std::hex << v;
            return oss.str();
        };
        auto toOct = [](unsigned long long v) {
            std::ostringstream oss;
            oss << "0o" << std::oct << v;
            return oss.str();
        };
        auto toBin = [](unsigned long long v) {
            if (v == 0) return std::string("0b0");
            std::string bits;
            unsigned long long tmp = v;
            int count = 0;
            while (tmp > 0) {
                if (count > 0 && count % 4 == 0) bits.push_back('_');
                bits.push_back((tmp & 1ULL) ? '1' : '0');
                tmp >>= 1;
                ++count;
            }
            std::reverse(bits.begin(), bits.end());
            return "0b" + bits;
        };

        if (mode == "hex") return toHex(uval);
        if (mode == "oct") return toOct(uval);
        if (mode == "bin") return toBin(uval);
        return std::to_string(val) + "  |  " + toHex(uval) + "  |  " + toOct(uval) + "  |  " + toBin(uval);
    }

    CalcEvalOutput evalLine(const std::string& raw_line) {
        syncFromModel();
        std::string s = raw_line;
        std::string s_trim = s;
        s_trim.erase(0, s_trim.find_first_not_of(" \t\r\n"));
        if (!s_trim.empty()) s_trim.erase(s_trim.find_last_not_of(" \t\r\n") + 1);

        // Allow "sym x" or "var x" to declare a custom symbolic variable
        if (s_trim.rfind("sym ", 0) == 0 || s_trim.rfind("var ", 0) == 0) {
            std::string vname = s_trim.substr(4);
            vname.erase(std::remove_if(vname.begin(), vname.end(), ::isspace), vname.end());
            if (vname.empty() || !std::isalpha(static_cast<unsigned char>(vname[0]))) {
                return {false, {0,0}, "", "", s_trim, "Usage: sym <var> (e.g. sym K or sym x)"};
            }
            active_sym_name = vname;
            sym_vars.clear();
            sym_vars[vname] = SymRat::variable();
            vars.erase(vname);
            return {true, {0,0}, "= Symbolic (" + vname + ")", "", "\\mathrm{sym}\\; " + vname, ""};
        }

        // Handle poly_solver(a_n, a_{n-1}, ..., a_0) for arbitrary degree polynomials
        {
            std::string s_nospace = s_trim;
            // Support optional variable assignment like: r = poly_solver(2, 4, -7)
            std::string ps_call = s_trim;
            std::string ps_lhs;
            auto eq_p = s_trim.find('=');
            if (eq_p != std::string::npos) {
                std::string cand = s_trim.substr(0, eq_p);
                cand.erase(std::remove_if(cand.begin(), cand.end(), ::isspace), cand.end());
                if (!cand.empty() && (std::isalpha(static_cast<unsigned char>(cand[0])) || cand[0] == '_')) {
                    ps_lhs = cand;
                    ps_call = s_trim.substr(eq_p + 1);
                    ps_call.erase(0, ps_call.find_first_not_of(" \t\r\n"));
                }
            }

            if (ps_call.rfind("poly_solver(", 0) == 0 && !ps_call.empty() && ps_call.back() == ')') {
                std::string inner = ps_call.substr(12, ps_call.size() - 13);

                // Split arguments by top-level commas (so expressions like 2+3 or (1+2j) work inside)
                std::vector<std::string> arg_strs;
                int depth = 0;
                std::string cur;
                for (char c : inner) {
                    if (c == '(' || c == '[' || c == '{') ++depth;
                    else if (c == ')' || c == ']' || c == '}') --depth;
                    if (c == ',' && depth == 0) {
                        arg_strs.push_back(cur);
                        cur.clear();
                    } else {
                        cur.push_back(c);
                    }
                }
                if (!cur.empty() || (!inner.empty() && inner.back() == ',')) {
                    arg_strs.push_back(cur);
                }

                if (arg_strs.size() < 2) {
                    return {false, {0,0}, "", "", "\\mathrm{poly\\_solver}",
                            "Usage: poly_solver(a_n, ..., a_0) — requires at least 2 coefficients"};
                }

                try {
                    Poly coeffs;
                    for (const auto& astr : arg_strs) {
                        Pos = 0;
                        Src = astr;
                        SymRat sr = parseE();
                        skip();
                        if (Pos < Src.size()) {
                            throw std::runtime_error(std::string("Unexpected token '") + Src[Pos] + "'");
                        }
                        if (!sr.isConstant()) {
                            throw std::runtime_error("poly_solver coefficients must evaluate to numeric/complex constants");
                        }
                        coeffs.push_back(sr.toConstant());
                    }

                    // Trim leading zeros
                    size_t first_nz = 0;
                    while (first_nz + 1 < coeffs.size() && std::abs(coeffs[first_nz]) < 1e-12) {
                        ++first_nz;
                    }
                    Poly trimmed(coeffs.begin() + static_cast<std::ptrdiff_t>(first_nz), coeffs.end());
                    int deg = static_cast<int>(trimmed.size()) - 1;
                    if (deg < 1 || std::abs(trimmed[0]) < 1e-12) {
                        return {false, {0,0}, "", "", "", "Degenerate polynomial (leading coefficient is 0)"};
                    }

                    // Solve roots using the suite's polynomial root solver
                    std::vector<Complex> roots = solvePolynomialRoots(trimmed);

                    // Sort roots cleanly: real roots descending, or by real part then imaginary part
                    std::sort(roots.begin(), roots.end(), [](const Complex& a, const Complex& b) {
                        if (std::abs(a.real() - b.real()) > 1e-6) return a.real() > b.real();
                        return a.imag() > b.imag();
                    });

                    // Build nice LaTeX preview of the polynomial equation: 2x^2 + 4x - 7 = 0
                    std::string poly_ltx;
                    for (int i = 0; i <= deg; ++i) {
                        Complex c = trimmed[i];
                        if (std::abs(c) < 1e-12) continue;
                        int pwr = deg - i;
                        std::string term;
                        if (std::abs(c.imag()) < 1e-9) {
                            double rv = c.real();
                            bool is_neg = (rv < 0.0);
                            double av = std::abs(rv);
                            std::string num_s = (pwr > 0 && std::abs(av - 1.0) < 1e-9) ? "" : PRECISION.fmt(av);
                            if (poly_ltx.empty()) {
                                term = (is_neg ? "-" : "") + num_s;
                            } else {
                                term = (is_neg ? " - " : " + ") + num_s;
                            }
                        } else {
                            term = (poly_ltx.empty() ? "" : " + ") + std::string("(") + PRECISION.fmtComplex(c) + ")";
                        }
                        if (pwr > 0) {
                            term += "x";
                            if (pwr > 1) term += "^{" + std::to_string(pwr) + "}";
                        }
                        poly_ltx += term;
                    }
                    poly_ltx += " = 0";

                    // Format roots and store r1, r2, ... in vars so subsequent lines can reference them
                    std::string out_s = "= ";
                    std::string base_var = ps_lhs.empty() ? "r" : ps_lhs;
                    for (size_t i = 0; i < roots.size(); ++i) {
                        Complex rt = roots[i];
                        if (std::abs(rt.real()) < 1e-11) rt.real(0.0);
                        if (std::abs(rt.imag()) < 1e-11) rt.imag(0.0);

                        std::string r_name = base_var + std::to_string(i + 1);
                        vars[r_name] = rt;
                        sym_vars.erase(r_name);

                        if (i > 0) out_s += " ,  ";
                        out_s += r_name + " = " + PRECISION.fmtComplex(rt);
                    }
                    if (!roots.empty()) {
                        vars["ans"] = roots[0];
                        sym_vars.erase("ans");
                        if (!ps_lhs.empty()) {
                            vars[ps_lhs] = roots[0];
                            sym_vars.erase(ps_lhs);
                        }
                    }

                    return {true, roots.empty() ? Complex(0,0) : roots[0], out_s, "", poly_ltx, ""};
                } catch (const std::exception& e) {
                    return {false, {0,0}, "", "", "\\mathrm{poly\\_solver}", e.what()};
                }
            }
        }

        std::string ltx_prev = toLatexPreview(s);

        s = std::regex_replace(s, std::regex(R"(\\zeta)"), "zeta");
        s = std::regex_replace(s, std::regex(R"(\\omega_n)"), "wn");
        s = std::regex_replace(s, std::regex(R"(\\omega_d)"), "wd");
        s = std::regex_replace(s, std::regex(R"(\\sigma_d)"), "sigma_d");
        s = std::regex_replace(s, std::regex(R"(\\pi)"), "pi");
        s = std::regex_replace(s, std::regex(R"(\\sqrt\{([^{}]+)\})"), "sqrt($1)");
        s = std::regex_replace(s, std::regex(R"(\\frac\{([^{}]+)\}\{([^{}]+)\})"), "(($1)/($2))");
        s = std::regex_replace(s, std::regex(R"(\\ln\^2\(([^()]+)\))"), "((ln($1))^2)");
        s = std::regex_replace(s, std::regex(R"(\\ln)"), "ln");
        s = std::regex_replace(s, std::regex(R"(\\sin)"), "sin");
        s = std::regex_replace(s, std::regex(R"(\\cos)"), "cos");
        s = std::regex_replace(s, std::regex(R"(\\tan)"), "tan");

        std::string assigned_name;
        auto eq_pos = s.find('=');
        if (eq_pos != std::string::npos) {
            std::string lhs = s.substr(0, eq_pos);
            lhs.erase(std::remove_if(lhs.begin(), lhs.end(), ::isspace), lhs.end());
            if (!lhs.empty() && (std::isalpha(static_cast<unsigned char>(lhs[0])) || lhs[0] == '_')) {
                assigned_name = lhs;
                s = s.substr(eq_pos + 1);
            }
        }

        try {
            Pos = 0; Src = s;
            SymRat sr = parseE();
            skip();
            if (Pos < Src.size()) {
                throw std::runtime_error(std::string("Unexpected token '") + Src[Pos] + "'");
            }

            std::string prefix = assigned_name.empty() ? "= " : (assigned_name + " = ");

            // Case A: Result simplifies to a standard numeric/complex constant
            if (sr.isConstant()) {
                Complex v = sr.toConstant();
                vars["ans"] = v;
                sym_vars.erase("ans");
                if (!assigned_name.empty()) {
                    vars[assigned_name] = v;
                    sym_vars.erase(assigned_name);
                }

                // Check if the expression was wrapped in hex(...), bin(...), oct(...), or base(...)
                std::string s_clean = s;
                s_clean.erase(0, s_clean.find_first_not_of(" \t\r\n"));
                if (!s_clean.empty()) s_clean.erase(s_clean.find_last_not_of(" \t\r\n") + 1);
                std::string base_mode;
                for (const char* bm : {"hex", "bin", "oct", "base"}) {
                    std::string pfx = std::string(bm) + "(";
                    if (s_clean.rfind(pfx, 0) == 0 && !s_clean.empty() && s_clean.back() == ')') {
                        base_mode = bm;
                        break;
                    }
                }

                std::string dec_s;
                if (!base_mode.empty() && std::abs(v.imag()) < 1e-9) {
                    long long iv = static_cast<long long>(std::llround(v.real()));
                    dec_s = prefix + formatBaseString(iv, base_mode);
                } else {
                    dec_s = prefix + PRECISION.fmtComplex(v);
                }

                std::string frac_s;
                if (std::abs(v.imag()) < 1e-12) {
                    auto fr = toFraction(v.real());
                    if (fr.has_value()) frac_s = prefix + std::to_string(fr->first) + " / " + std::to_string(fr->second);
                }
                return {true, v, dec_s, frac_s, ltx_prev, ""};
            }

            // Case B: Result is a symbolic rational expression in active_sym_name
            sym_vars["ans"] = sr;
            if (!assigned_name.empty()) {
                sym_vars[assigned_name] = sr;
                vars.erase(assigned_name);
            }
            std::string sym_dec = prefix + sr.toString(active_sym_name);
            std::string sym_ltx = (assigned_name.empty() ? "" : (assigned_name + " = ")) + sr.toLatex(active_sym_name);
            return {true, {0.0, 0.0}, sym_dec, "", sym_ltx, ""};

        } catch (const std::exception& e) {
            return {false, {0,0}, "", "", ltx_prev, e.what()};
        }
    }

private:
    std::string Src;
    size_t Pos = 0;
    void skip() { while (Pos < Src.size() && std::isspace(static_cast<unsigned char>(Src[Pos]))) ++Pos; }

    SymRat parseE() {
        SymRat l = parseT();
        while (true) {
            skip();
            if (Pos < Src.size() && (Src[Pos] == '+' || Src[Pos] == '-')) {
                char op = Src[Pos++];
                SymRat r = parseT();
                l = (op == '+') ? (l + r) : (l - r);
            } else break;
        }
        return l;
    }

    SymRat parseT() {
        SymRat l = parseP();
        while (true) {
            skip();
            if (Pos < Src.size() && (Src[Pos] == '*' || Src[Pos] == '/')) {
                char op = Src[Pos++];
                SymRat r = parseP();
                if (op == '/' && r.isZero()) throw std::runtime_error("Division by zero");
                l = (op == '*') ? (l * r) : (l / r);
            } else if (Pos < Src.size() && (Src[Pos] == '(' || std::isalpha(static_cast<unsigned char>(Src[Pos])))) {
                l = l * parseP();
            } else break;
        }
        return l;
    }

    SymRat parseP() {
        SymRat b = parseU();
        skip();
        if (Pos < Src.size() && Src[Pos] == '^') {
            ++Pos;
            SymRat e = parseU();
            if (!e.isConstant()) throw std::runtime_error("Exponent must be numeric");
            Complex ec = e.toConstant();
            if (b.isConstant()) return SymRat(std::pow(b.toConstant(), ec));
            int p = static_cast<int>(std::round(ec.real()));
            if (std::abs(ec.imag()) > 1e-9 || std::abs(ec.real() - p) > 1e-7) {
                throw std::runtime_error("Symbolic base requires an integer exponent");
            }
            SymRat res(1.0);
            for (int i = 0; i < std::abs(p); ++i) res = res * b;
            return (p >= 0) ? res : (SymRat(1.0) / res);
        }
        return b;
    }

    SymRat parseU() {
        skip();
        if (Pos < Src.size() && Src[Pos] == '+') { ++Pos; return parseU(); }
        if (Pos < Src.size() && Src[Pos] == '-') { ++Pos; return SymRat(-1.0) * parseU(); }
        return parseAtom();
    }

    std::vector<SymRat> parseArgList() {
        std::vector<SymRat> args;
        skip();
        if (Pos < Src.size() && Src[Pos] == ')') { ++Pos; return args; }
        while (true) {
            args.push_back(parseE());
            skip();
            if (Pos < Src.size() && Src[Pos] == ',') { ++Pos; continue; }
            if (Pos < Src.size() && Src[Pos] == ')') { ++Pos; break; }
            throw std::runtime_error("Expected ',' or ')'");
        }
        return args;
    }

    SymRat parseAtom() {
        skip();
        if (Pos >= Src.size()) throw std::runtime_error("Incomplete expression");
        if (Src[Pos] == '(' || Src[Pos] == '{') {
            char close_ch = (Src[Pos] == '(') ? ')' : '}';
            ++Pos;
            SymRat v = parseE();
            skip();
            if (Pos < Src.size() && Src[Pos] == close_ch) ++Pos;
            return v;
        }
        if (std::isdigit(static_cast<unsigned char>(Src[Pos])) || Src[Pos] == '.') {
            if (Src[Pos] == '0' && Pos + 1 < Src.size()) {
                char pfx = static_cast<char>(std::tolower(static_cast<unsigned char>(Src[Pos + 1])));
                if (pfx == 'x' || pfx == 'b' || pfx == 'o') {
                    int radix = (pfx == 'x') ? 16 : ((pfx == 'b') ? 2 : 8);
                    Pos += 2;
                    std::string digits;
                    while (Pos < Src.size()) {
                        char c = Src[Pos];
                        if (c == '_') { ++Pos; continue; }
                        bool ok = (radix == 2  && (c == '0' || c == '1')) ||
                                  (radix == 8  && (c >= '0' && c <= '7')) ||
                                  (radix == 16 && std::isxdigit(static_cast<unsigned char>(c)));
                        if (!ok) break;
                        digits.push_back(c);
                        ++Pos;
                    }
                    if (digits.empty()) throw std::runtime_error("Invalid base literal");
                    unsigned long long uval = std::stoull(digits, nullptr, radix);
                    return SymRat(Complex(static_cast<double>(uval), 0.0));
                }
            }
            size_t st = Pos;
            while (Pos < Src.size() && (std::isdigit(static_cast<unsigned char>(Src[Pos])) || Src[Pos] == '.')) ++Pos;
            // Consume scientific notation exponent (e.g. 1e5, 2.5e-6, 1.2e+05) while preserving `2e` as 2 * Euler's e
            if (Pos < Src.size() && (Src[Pos] == 'e' || Src[Pos] == 'E')) {
                size_t exp_pos = Pos + 1;
                if (exp_pos < Src.size() && (Src[exp_pos] == '+' || Src[exp_pos] == '-')) ++exp_pos;
                if (exp_pos < Src.size() && std::isdigit(static_cast<unsigned char>(Src[exp_pos]))) {
                    Pos = exp_pos;
                    while (Pos < Src.size() && std::isdigit(static_cast<unsigned char>(Src[Pos]))) ++Pos;
                }
            }
            double v = std::stod(Src.substr(st, Pos - st));
            if (Pos < Src.size() && (Src[Pos] == 'i' || Src[Pos] == 'j')) { ++Pos; return SymRat(Complex(0.0, v)); }
            return SymRat(Complex(v, 0.0));
        }
        if (std::isalpha(static_cast<unsigned char>(Src[Pos])) || Src[Pos] == '_') {
            size_t st = Pos;
            while (Pos < Src.size() && (std::isalnum(static_cast<unsigned char>(Src[Pos])) || Src[Pos] == '_')) ++Pos;
            std::string name = Src.substr(st, Pos - st);
            skip();
            if (Pos < Src.size() && Src[Pos] == '(') {
                ++Pos;
                auto s_args = parseArgList();
                bool any_sym = false;
                for (const auto& a : s_args) if (!a.isConstant()) { any_sym = true; break; }

                if (any_sym) {
                    if (name == "parallel" || name == "par") {
                        if (s_args.empty()) throw std::runtime_error("parallel() requires arguments");
                        SymRat inv_sum(0.0);
                        for (const auto& z : s_args) {
                            if (z.isZero()) return SymRat(0.0);
                            inv_sum = inv_sum + (SymRat(1.0) / z);
                        }
                        return SymRat(1.0) / inv_sum;
                    }
                    if (name == "sum") {
                        SymRat tot(0.0);
                        for (const auto& z : s_args) tot = tot + z;
                        return tot;
                    }
                    if (name == "mean" || name == "avg") {
                        if (s_args.empty()) throw std::runtime_error("mean() requires arguments");
                        SymRat tot(0.0);
                        for (const auto& z : s_args) tot = tot + z;
                        return tot / SymRat(double(s_args.size()));
                    }
                    throw std::runtime_error("Function '" + name + "' requires numeric arguments");
                }

                std::vector<Complex> c_args;
                c_args.reserve(s_args.size());
                for (const auto& a : s_args) c_args.push_back(a.toConstant());
                return SymRat(callFunc(name, c_args));
            }
            if (sym_vars.count(name)) return sym_vars[name];
            if (vars.count(name)) return SymRat(vars[name]);
            throw std::runtime_error("Undefined symbol '" + name + "' (type 'sym " + name + "' to make it symbolic)");
        }
        throw std::runtime_error(std::string("Unexpected token '") + Src[Pos] + "'");
    }

    Complex callFunc(const std::string& fn, const std::vector<Complex>& a) {
        if (a.empty()) throw std::runtime_error(fn + "() requires arguments");
        double x = a[0].real();
        double to_rad = deg_mode ? (PI / 180.0) : 1.0;
        double from_rad = deg_mode ? (180.0 / PI) : 1.0;
        if ((fn == "hex" || fn == "bin" || fn == "oct" || fn == "base") && !a.empty()) return a[0];

        if (fn == "sqrt") return std::sqrt(a[0]);
        if (fn == "ln")   return std::log(a[0]);
        if (fn == "log")  return std::log10(a[0]);
        if (fn == "exp")  return std::exp(a[0]);
        if (fn == "abs")  return Complex(std::abs(a[0]), 0.0);
        if (fn == "real") return Complex(a[0].real(), 0.0);
        if (fn == "imag") return Complex(a[0].imag(), 0.0);
        if (fn == "arg")  return Complex(std::arg(a[0]) * from_rad, 0.0);
        if (fn == "sin")  return std::sin(a[0] * to_rad);
        if (fn == "cos")  return std::cos(a[0] * to_rad);
        if (fn == "tan")  return std::tan(a[0] * to_rad);
        if (fn == "asin" || fn == "arcsin") return std::asin(a[0]) * from_rad;
        if (fn == "acos" || fn == "arccos") return std::acos(a[0]) * from_rad;
        if (fn == "atan" || fn == "arctan") return std::atan(a[0]) * from_rad;
        if (fn == "dB" || fn == "db") return Complex(20.0 * std::log10(std::max(std::abs(a[0]), 1e-15)), 0.0);

        if (fn == "parallel" || fn == "par") {
            Complex inv_sum(0.0, 0.0);
            for (const auto& z : a) {
                if (std::abs(z) < 1e-15) return Complex(0.0, 0.0); // Short circuit (0 Ohm) in parallel yields 0
                inv_sum += Complex(1.0, 0.0) / z;
            }
            if (std::abs(inv_sum) < 1e-15) throw std::runtime_error("Parallel impedance sum of reciprocals is zero");
            return Complex(1.0, 0.0) / inv_sum;
        }
        if (fn == "sum") {
            Complex total(0.0, 0.0);
            for (const auto& z : a) total += z;
            return total;
        }
        if (fn == "mean" || fn == "avg") {
            Complex total(0.0, 0.0);
            for (const auto& z : a) total += z;
            return total / Complex(double(a.size()), 0.0);
        }

        if (fn == "solve_zeta_from_os" || fn == "zeta_from_os") return Complex(niseZetaFromOs(x), 0.0);
        if (fn == "solve_zeta_from_Ts_wn" && a.size() >= 2) return Complex(4.0 / std::max(x * a[1].real(), 1e-12), 0.0);
        if (fn == "solve_zeta_from_Tp_wn" && a.size() >= 2) {
            double r = PI / std::max(x * a[1].real(), 1e-12);
            return Complex(r >= 1.0 ? 0.0 : std::sqrt(std::max(0.0, 1.0 - r * r)), 0.0);
        }
        if (fn == "solve_zeta_from_Ts_Tp" && a.size() >= 2) {
            double sd = 4.0 / std::max(x, 1e-12), wd = PI / std::max(a[1].real(), 1e-12);
            return Complex(sd / std::hypot(sd, wd), 0.0);
        }
        if (fn == "solve_zeta_from_ab" && a.size() >= 2) return Complex(x / (2.0 * std::sqrt(std::max(a[1].real(), 1e-12))), 0.0);
        if (fn == "solve_wn_from_b") return Complex(std::sqrt(std::max(x, 0.0)), 0.0);
        if (fn == "solve_wn_from_Ts_zeta" && a.size() >= 2) return Complex(4.0 / std::max(x * a[1].real(), 1e-12), 0.0);
        if (fn == "solve_wn_from_Tp_zeta" && a.size() >= 2) {
            double z = a[1].real();
            return Complex(PI / (std::max(x, 1e-12) * std::sqrt(std::max(1e-12, 1.0 - z * z))), 0.0);
        }
        if (fn == "solve_wd" && a.size() >= 2) return Complex(a[1].real() * std::sqrt(std::max(0.0, 1.0 - x * x)), 0.0);
        if (fn == "solve_sigma_d" && a.size() >= 2) return Complex(x * a[1].real(), 0.0);
        if (fn == "solve_os" || fn == "os_from_zeta") return Complex(niseOsFromZeta(x), 0.0);
        if (fn == "solve_Ts" && a.size() >= 2) return Complex(4.0 / std::max(x * a[1].real(), 1e-9), 0.0);
        if (fn == "solve_Tp" && a.size() >= 2) {
            double wd = a[1].real() * std::sqrt(std::max(1e-9, 1.0 - x * x));
            return Complex(PI / wd, 0.0);
        }
        if (fn == "solve_Tr" && a.size() >= 2) return Complex(niseTrFromZetaWn(x, a[1].real()), 0.0);
        if (fn == "solve_T_1st") return Complex(1.0 / std::max(x, 1e-12), 0.0);
        if (fn == "solve_Tr_1st") return Complex(2.2 / std::max(x, 1e-12), 0.0);
        if (fn == "solve_Ts_1st") return Complex(4.0 / std::max(x, 1e-12), 0.0);
        if (fn == "check_5x_rule" && a.size() >= 3) return Complex(std::abs(x) / std::max(a[1].real() * a[2].real(), 1e-12), 0.0);
        if (fn == "solve_sigma_a" && a.size() >= 3) return Complex((x - a[1].real()) / std::max(std::abs(a[2].real()), 1e-12), 0.0);
        if (fn == "solve_deficiency") return Complex(computeAngleDeficiency(x), 0.0);
        if (fn == "solve_ess_step") {
            double m = (a.size() >= 2) ? a[1].real() : 1.0;
            return Complex(m / (1.0 + x), 0.0);
        }
        if (fn == "solve_rel_err_pct") {
            double m = (a.size() >= 2) ? a[1].real() : 1.0;
            return Complex((std::abs(x) / std::max(std::abs(m), 1e-12)) * 100.0, 0.0);
        }
        if (fn == "solve_kp_for_os_p" && a.size() >= 3) {
            double z = PRECISION.step(niseZetaFromOs(x));
            double wn_v = PRECISION.step(a[1].real() / (2.0 * std::max(z, 1e-9)));
            double k0 = (a.size() >= 4) ? a[3].real() : 1.0;
            return Complex((wn_v * wn_v - a[2].real()) / std::max(k0, 1e-9), 0.0);
        }
        if (fn == "solve_kd_for_os_pd" && a.size() >= 4) {
            double z = PRECISION.step(niseZetaFromOs(x));
            double k0 = (a.size() >= 5) ? a[4].real() : 1.0;
            double wn_v = PRECISION.step(std::sqrt(std::max(0.0, a[3].real() + k0 * a[1].real())));
            return Complex((2.0 * z * wn_v - a[2].real()) / std::max(k0, 1e-9), 0.0);
        }
        if (fn == "solve_kp_for_os_pd" && a.size() >= 4) {
            double z = PRECISION.step(niseZetaFromOs(x));
            double k0 = (a.size() >= 5) ? a[4].real() : 1.0;
            double wn_v = PRECISION.step((a[2].real() + k0 * a[1].real()) / (2.0 * std::max(z, 1e-9)));
            return Complex((wn_v * wn_v - a[3].real()) / std::max(k0, 1e-9), 0.0);
        }
        if (fn == "solve_ki_for_os_pi" && a.size() >= 3) {
            double z = PRECISION.step(niseZetaFromOs(x));
            double k0 = (a.size() >= 4) ? a[3].real() : 1.0;
            double wn_v = PRECISION.step((a[2].real() + k0 * a[1].real()) / (2.0 * std::max(z, 1e-9)));
            return Complex((wn_v * wn_v) / std::max(k0, 1e-9), 0.0);
        }
        if (fn == "solve_z_from_s") {
            double ts_v = (a.size() >= 2) ? a[1].real() : 0.05;
            return std::exp(a[0] * ts_v);
        }
        if (fn == "solve_tustin_z") {
            double ts_v = (a.size() >= 2) ? a[1].real() : 0.05;
            return (1.0 + a[0] * (0.5 * ts_v)) / (1.0 - a[0] * (0.5 * ts_v));
        }
        if (fn == "G" && model) return evaluateTransferFunction(model->poles, model->zeros, a[0], model->plant_gain).complex_val;
        if (fn == "K_rl" && model) return Complex(evaluateTransferFunction(model->poles, model->zeros, a[0], 1.0).rl_gain_k, 0.0);
        if (fn == "H_z" && model) return evaluateTransferFunction(model->z_poles, model->z_zeros, a[0], model->filter_gain).complex_val;
        throw std::runtime_error("Unknown function '" + fn + "'");
    }
};

class ScientificCalculatorTab : public QWidget {
    Q_OBJECT
public:
    struct CalcRowUI {
        QFrame* frame;
        QLineEdit* edit;
        MathFormulaWidget* math_prev;
        QPushButton* frac_btn;
        QLabel* res_lbl;
        bool use_frac = false;
        QWidget* res_container = nullptr;
        QHBoxLayout* inline_slot = nullptr;
        QHBoxLayout* wrapped_slot = nullptr;
        QWidget* wrapped_row_w = nullptr;
    };

    SharedSystemModel* model;
    ScientificCalcEngine engine;
    QVBoxLayout* rows_layout;
    std::vector<CalcRowUI> rows;
    QLineEdit* active_edit = nullptr;

    std::string active_keypad_mode = "main";
    std::map<std::string, QPushButton*> mode_btns;
    QWidget* keys_container;
    QGridLayout* keys_grid;

    std::string active_formula_cat = "all";
    std::map<std::string, QPushButton*> cat_btns;
    QLineEdit* formula_search_edit;
    QVBoxLayout* fn_cards_layout;
    QLabel* live_vars_lbl;
    bool show_latex_preview = true;

    QScrollArea* expr_scroll = nullptr;

    QLineEdit *base_dec_edit = nullptr;
    QLineEdit *base_hex_edit = nullptr;
    QLineEdit *base_oct_edit = nullptr;
    QLineEdit *base_bin_edit = nullptr;
    bool updating_base_conv = false;

    explicit ScientificCalculatorTab(SharedSystemModel* m, QWidget* parent = nullptr)
        : QWidget(parent), model(m), engine(m)
    {
        auto* root_l = new QHBoxLayout(this);
        root_l->setContentsMargins(0, 0, 0, 0);
        auto* splitter = new QSplitter(Qt::Horizontal, this);
        root_l->addWidget(splitter);

        // Left Column: Expression Notebook + Virtual Keypad Dock
        auto* left_w = new QWidget();
        auto* left_l = new QVBoxLayout(left_w);
        left_l->setContentsMargins(0, 0, 6, 0);

        auto* expr_card = makeCard();
        auto* expr_l = new QVBoxLayout(expr_card);
        auto* hdr = new QHBoxLayout();
        auto* title = new QLabel("Scientific Calculator  •  LaTeX Input & Formula Sheet Solver");
        title->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 13px; border:none;").arg(Theme::TEXT_PRIMARY));
        auto* ltx_btn = makeActionBtn("LaTeX View: ON", Theme::ACCENT_PID);
        auto* add_btn = makeActionBtn("+ Add Line (Enter)", Theme::ACCENT_ZERO);
        auto* clr_btn = makeActionBtn("Clear All", Theme::TEXT_SECONDARY);
        hdr->addWidget(title); hdr->addStretch(); hdr->addWidget(ltx_btn); hdr->addWidget(add_btn); hdr->addWidget(clr_btn);
        expr_l->addLayout(hdr);

        expr_scroll = new QScrollArea();
        expr_scroll->setWidgetResizable(true);
        expr_scroll->setStyleSheet(QString("background-color: %1; border: 1px solid %2;").arg(Theme::BG_INPUT, Theme::BORDER_INPUT));
        auto* container = new QWidget();
        rows_layout = new QVBoxLayout(container);
        rows_layout->setAlignment(Qt::AlignTop);
        rows_layout->setSpacing(4);
        expr_scroll->setWidget(container);
        expr_l->addWidget(expr_scroll, 1);
        left_l->addWidget(expr_card, 3);

        // Live 4-Way Programmer Base Converter Card (DEC / HEX / OCT / BIN)
        auto* base_card = makeCard();
        auto* bcl = new QVBoxLayout(base_card);
        bcl->setContentsMargins(10, 6, 10, 6);
        bcl->setSpacing(4);

        auto* bc_hdr = new QHBoxLayout();
        auto* bc_title = new QLabel("Base Converter  (DEC ⇄ HEX ⇄ OCT ⇄ BIN)");
        bc_title->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border:none;").arg(Theme::ACCENT_ZERO));
        auto* bc_hint = new QLabel("Inline: 0xFF, 0b1010, 0o77, hex(x), bin(x), oct(x), base(x)");
        bc_hint->setStyleSheet("color: #64748b; font-family: Monospace; font-size: 10px; border:none;");
        bc_hdr->addWidget(bc_title);
        bc_hdr->addStretch();
        bc_hdr->addWidget(bc_hint);
        bcl->addLayout(bc_hdr);

        auto* bc_grid = new QGridLayout();
        bc_grid->setHorizontalSpacing(6);
        bc_grid->setVerticalSpacing(4);

        auto updateFromValue = [this](long long val, QLineEdit* source) {
            if (updating_base_conv) return;
            updating_base_conv = true;
            unsigned long long uval = static_cast<unsigned long long>(val);

            if (source != base_dec_edit) base_dec_edit->setText(QString::number(val));
            if (source != base_hex_edit) base_hex_edit->setText(QString::number(uval, 16).toUpper());
            if (source != base_oct_edit) base_oct_edit->setText(QString::number(uval, 8));
            if (source != base_bin_edit) {
                QString raw_b = QString::number(uval, 2);
                QString grouped;
                int rem = raw_b.length() % 4;
                if (rem > 0) raw_b = QString(4 - rem, '0') + raw_b;
                for (int i = 0; i < raw_b.length(); ++i) {
                    if (i > 0 && i % 4 == 0) grouped += " ";
                    grouped += raw_b[i];
                }
                base_bin_edit->setText(grouped);
            }
            updating_base_conv = false;
        };

        auto makeBaseRow = [&](int row, int col_offset, const QString& lbl_txt, const QString& pfx, int radix, QLineEdit*& ed_out) {
            auto* l = new QLabel(lbl_txt);
            l->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; font-size: 10px; border:none;").arg(Theme::ACCENT_PID));
            ed_out = makeDarkEntry("0");
            auto* ins_b = makeActionBtn(pfx.isEmpty() ? "Ins" : pfx, Theme::TEXT_SECONDARY, Theme::BG_PANEL_ALT);
            ins_b->setToolTip("Insert into active calculator line");

            bc_grid->addWidget(l, row, col_offset);
            bc_grid->addWidget(ed_out, row, col_offset + 1);
            bc_grid->addWidget(ins_b, row, col_offset + 2);

            connect(ed_out, &QLineEdit::textEdited, this, [this, ed_out, radix, updateFromValue](const QString& txt) {
                QString clean = txt;
                clean.remove(' ').remove('_');
                if (clean.startsWith("0x", Qt::CaseInsensitive) || clean.startsWith("0b", Qt::CaseInsensitive) || clean.startsWith("0o", Qt::CaseInsensitive)) {
                    clean = clean.mid(2);
                }
                if (clean.isEmpty()) return;
                bool ok = false;
                long long v = (radix == 10 && clean.startsWith('-'))
                    ? clean.toLongLong(&ok, 10)
                    : static_cast<long long>(clean.toULongLong(&ok, radix));
                if (ok) updateFromValue(v, ed_out);
            });

            connect(ins_b, &QPushButton::clicked, this, [this, ed_out, pfx]() {
                QString clean = ed_out->text();
                clean.remove(' ');
                if (!active_edit && !rows.empty()) active_edit = rows.back().edit;
                if (active_edit) {
                    active_edit->insert(pfx + clean);
                    active_edit->setFocus();
                    evaluateAll();
                }
            });
        };

        makeBaseRow(0, 0, "DEC", "",   10, base_dec_edit);
        makeBaseRow(0, 3, "HEX", "0x", 16, base_hex_edit);
        makeBaseRow(1, 0, "OCT", "0o",  8, base_oct_edit);
        makeBaseRow(1, 3, "BIN", "0b",  2, base_bin_edit);
        bc_grid->setColumnStretch(1, 1);
        bc_grid->setColumnStretch(4, 1);

        bcl->addLayout(bc_grid);
        left_l->addWidget(base_card);

        // Keypad Dock Card (main | LaTeX \frac | Formula Sheet f(x) | abc | func)
        auto* pad_card = makeCard();
        auto* pad_l = new QVBoxLayout(pad_card);
        pad_l->setContentsMargins(12, 10, 12, 10);
        pad_l->setSpacing(8);

        auto* mode_bar = new QHBoxLayout();
        mode_bar->setContentsMargins(0, 0, 0, 0);
        mode_bar->setSpacing(5);

        const std::vector<std::pair<std::string, QString>> modes = {
            {"main", "main"},
            {"latex", "LaTeX \\frac"},
            {"formulas", "Formula Sheet f(x)"},
            {"abc", "abc"},
            {"func", "func"}
        };
        for (const auto& [m_id, m_lbl] : modes) {
            auto* b = new QPushButton(m_lbl);
            b->setCursor(Qt::PointingHandCursor);
            connect(b, &QPushButton::clicked, this, [this, m_id]() { switchKeypadMode(m_id); });
            mode_bar->addWidget(b);
            mode_btns[m_id] = b;
        }

        mode_bar->addSpacing(8);

        auto* deg_btn = new QPushButton("DEG  |  rad");
        deg_btn->setCursor(Qt::PointingHandCursor);
        deg_btn->setStyleSheet(QString(
            "QPushButton { background-color: %1; color: %2; border: 1px solid %2; border-radius: 2px; padding: 5px 10px; font-family: Monospace; font-weight: bold; font-size: 11px; }"
            "QPushButton:hover { background-color: #143529; }"
        ).arg(Theme::BG_INPUT, Theme::ACCENT_PID));
        mode_bar->addWidget(deg_btn);

        mode_bar->addStretch();

        auto* bs_btn = new QPushButton("⌫ Backspace");
        bs_btn->setCursor(Qt::PointingHandCursor);
        bs_btn->setStyleSheet(QString(
            "QPushButton { background-color: %1; color: %2; border: 1px solid %3; border-radius: 2px; padding: 5px 12px; font-weight: bold; font-size: 11px; }"
            "QPushButton:hover { background-color: #2d3b54; }"
        ).arg(Theme::BG_KEY_FN, Theme::TEXT_PRIMARY, Theme::BORDER_BTN));
        mode_bar->addWidget(bs_btn);

        auto* enter_btn = new QPushButton("↵ New Line");
        enter_btn->setCursor(Qt::PointingHandCursor);
        enter_btn->setStyleSheet(QString(
            "QPushButton { background-color: %1; color: #ffffff; border: 1px solid %1; border-radius: 2px; padding: 5px 14px; font-weight: bold; font-size: 11px; }"
            "QPushButton:hover { background-color: #1d4ed8; }"
        ).arg(Theme::BG_KEY_ACCENT));
        mode_bar->addWidget(enter_btn);

        pad_l->addLayout(mode_bar);

        keys_container = new QWidget();
        keys_container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        keys_grid = new QGridLayout(keys_container);
        keys_grid->setContentsMargins(0, 0, 0, 0);
        keys_grid->setHorizontalSpacing(4);
        keys_grid->setVerticalSpacing(4);
        pad_l->addWidget(keys_container, 1);
        left_l->addWidget(pad_card, 2);

        // Right Column: Formula Sheet Functions Browser (Plug & Solve)
        auto* right_card = makeCard();
        right_card->setMinimumWidth(420);
        auto* right_l = new QVBoxLayout(right_card);
        auto* rt = new QLabel("Formula Sheet Functions (Click to Insert & Solve)");
        rt->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 12px; border:none;").arg(Theme::ACCENT_PID));
        right_l->addWidget(rt);

        auto* cat_row1 = new QHBoxLayout();
        auto* cat_row2 = new QHBoxLayout();
        const std::vector<std::tuple<std::string, QString, QHBoxLayout*>> cats = {
            {"all", "All", cat_row1}, {"zeta", "Solve ζ", cat_row1}, {"wn_wd", "Solve ωn/ωd", cat_row1}, {"transient", "2nd-Order Specs", cat_row1},
            {"first_order", "1st-Order", cat_row2}, {"rl_approx", "Root Locus / 5×", cat_row2}, {"pid_ess", "PID & e_ss", cat_row2}, {"discrete", "z-Domain", cat_row2}
        };
        for (const auto& [ck, cl, bar] : cats) {
            auto* b = makeActionBtn(cl, ck == "all" ? "#ffffff" : Theme::TEXT_SECONDARY, ck == "all" ? Theme::BG_KEY_ACCENT : Theme::BG_INPUT);
            connect(b, &QPushButton::clicked, this, [this, ck]() { selectFormulaCategory(ck); });
            bar->addWidget(b);
            cat_btns[ck] = b;
        }
        right_l->addLayout(cat_row1);
        right_l->addLayout(cat_row2);

        auto* s_row = new QHBoxLayout();
        auto* sl = new QLabel("Filter Formulas:"); sl->setStyleSheet("color: #94a3b8; font-size: 10px; font-weight: bold; border:none;");
        formula_search_edit = makeDarkEntry("", 120);
        s_row->addWidget(sl); s_row->addWidget(formula_search_edit, 1);
        right_l->addLayout(s_row);

        auto* fn_scroll = new QScrollArea();
        fn_scroll->setWidgetResizable(true);
        fn_scroll->setStyleSheet(QString("background-color: %1; border: 1px solid %2;").arg(Theme::BG_INPUT, Theme::BORDER_INPUT));
        auto* fn_body = new QWidget();
        fn_cards_layout = new QVBoxLayout(fn_body);
        fn_cards_layout->setAlignment(Qt::AlignTop);
        fn_cards_layout->setSpacing(6);
        fn_scroll->setWidget(fn_body);
        right_l->addWidget(fn_scroll, 1);

        live_vars_lbl = new QLabel();
        live_vars_lbl->setStyleSheet(QString("background-color: %1; color: %2; font-family: Monospace; font-size: 10px; padding: 6px; border-radius: 4px;")
                                     .arg(Theme::BG_INPUT, Theme::ACCENT_ZERO));
        right_l->addWidget(live_vars_lbl);

        splitter->addWidget(left_w);
        splitter->addWidget(right_card);
        splitter->setStretchFactor(0, 1);
        splitter->setStretchFactor(1, 0);

        // Connections
        connect(ltx_btn, &QPushButton::clicked, this, [this, ltx_btn]() {
            show_latex_preview = !show_latex_preview;
            ltx_btn->setText(show_latex_preview ? "LaTeX View: ON" : "LaTeX View: OFF");
            evaluateAll();
        });
        connect(add_btn, &QPushButton::clicked, this, [this]() { addRow(""); });
        connect(clr_btn, &QPushButton::clicked, this, [this]() {
            for (auto& r : rows) delete r.frame;
            rows.clear();
            active_edit = nullptr;
            addRow("");
        });
        connect(deg_btn, &QPushButton::clicked, this, [this, deg_btn]() {
            engine.deg_mode = !engine.deg_mode;
            if (engine.deg_mode) {
                deg_btn->setText("DEG  |  rad");
                deg_btn->setStyleSheet(QString(
                    "QPushButton { background-color: %1; color: %2; border: 1px solid %2; border-radius: 2px; padding: 5px 10px; font-family: Monospace; font-weight: bold; font-size: 11px; }"
                ).arg(Theme::BG_INPUT, Theme::ACCENT_PID));
            } else {
                deg_btn->setText("deg  |  RAD");
                deg_btn->setStyleSheet(QString(
                    "QPushButton { background-color: %1; color: %2; border: 1px solid %2; border-radius: 2px; padding: 5px 10px; font-family: Monospace; font-weight: bold; font-size: 11px; }"
                ).arg(Theme::BG_INPUT, Theme::ACCENT_ZERO));
            }
            evaluateAll();
        });
        connect(bs_btn, &QPushButton::clicked, this, [this]() {
            if (active_edit) { active_edit->backspace(); evaluateAll(); }
        });
        connect(enter_btn, &QPushButton::clicked, this, [this]() { addRow(""); });
        connect(formula_search_edit, &QLineEdit::textChanged, this, [this]() { renderFormulaCards(); });

        switchKeypadMode("main");
        renderFormulaCards();

        addRow(R"(\frac{-\ln(12.3/100)}{\sqrt{\pi^2 + \ln^2(12.3/100)}})");
        addRow("solve_zeta_from_os(12.3)");
        addRow("solve_kp_for_os_p(20, 5, 6, 1)");
        addRow("solve_kd_for_os_pd(10, 24.06, 5, 6, 1)");
        addRow("G(s_test)");
        evaluateAll();
    }

    void switchKeypadMode(const std::string& mode) {
        active_keypad_mode = mode;
        for (auto& [m, b] : mode_btns) {
            bool is_active = (m == mode);
            b->setStyleSheet(QString(
                "QPushButton { background-color: %1; color: %2; border: 1px solid %3; border-radius: 2px; padding: 5px 12px; font-weight: bold; font-size: 11px; }"
                "QPushButton:hover { background-color: %4; }"
            ).arg(is_active ? Theme::BG_KEY_ACCENT : Theme::BG_INPUT,
                  Theme::TEXT_PRIMARY,
                  is_active ? Theme::BG_KEY_ACCENT : Theme::BORDER_BTN,
                  is_active ? "#1d4ed8" : "#2d3b54"));
        }
        renderKeypadGrid();
    }

    void renderKeypadGrid() {
        QLayoutItem* item;
        while ((item = keys_grid->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        // Reset previous row/column stretches
        for (int r = 0; r < 6; ++r) keys_grid->setRowStretch(r, 0);
        for (int c = 0; c < 12; ++c) keys_grid->setColumnStretch(c, 0);

        struct KeySpec {
            QString label;
            QString token;
            std::string kind; // "op" (navy/blue), "fn" (dark slate), "num" (lighter slate)
        };

        std::vector<std::vector<KeySpec>> layout;
        if (active_keypad_mode == "main") {
            layout = {
                {{"\\frac{a}{b}", "\\frac{}{}", "op"}, {"\\sqrt{x}", "\\sqrt{}", "fn"}, {"a²", "^2", "fn"}, {"aᵇ", "^{}", "fn"}, {"7", "7", "num"}, {"8", "8", "num"}, {"9", "9", "num"}, {"÷", "/", "op"}},
                {{"ζ (\\zeta)", "\\zeta", "fn"}, {"ωn (\\omega_n)", "\\omega_n", "fn"}, {"sin", "\\sin(", "fn"}, {"cos", "\\cos(", "fn"}, {"4", "4", "num"}, {"5", "5", "num"}, {"6", "6", "num"}, {"×", "*", "op"}},
                {{"π (\\pi)", "\\pi", "fn"}, {"(", "(", "fn"}, {")", ")", "fn"}, {",", ", ", "fn"}, {"1", "1", "num"}, {"2", "2", "num"}, {"3", "3", "num"}, {"−", "-", "op"}},
                {{"i (imag)", "i", "fn"}, {"ln", "\\ln(", "fn"}, {"eˣ", "e^{}", "fn"}, {"ans", "ans", "fn"}, {"0", "0", "num"}, {".", ".", "num"}, {"=", " = ", "op"}, {"+", "+", "op"}}
            };
        } else if (active_keypad_mode == "latex") {
            layout = {
                {{"\\frac{a}{b}", "\\frac{}{}", "op"}, {"\\sqrt{x}", "\\sqrt{}", "op"}, {"\\sqrt[n]{x}", "\\sqrt[]{}", "op"}, {"x^{n}", "^{}", "op"}, {"\\zeta", "\\zeta", "fn"}, {"\\omega_n", "\\omega_n", "fn"}, {"\\omega_d", "\\omega_d", "fn"}, {"\\sigma_d", "\\sigma_d", "fn"}},
                {{"\\pi", "\\pi", "fn"}, {"\\ln(x)", "\\ln(", "fn"}, {"\\ln^2(x)", "\\ln^2(", "fn"}, {"e^{-x}", "e^{-}", "fn"}, {"\\sin", "\\sin(", "fn"}, {"\\cos", "\\cos(", "fn"}, {"\\tan", "\\tan(", "fn"}, {"\\tan^{-1}", "atan(", "fn"}},
                {{"\\%OS", "OS", "fn"}, {"T_s", "Ts", "fn"}, {"T_p", "Tp", "fn"}, {"T_r", "Tr", "fn"}, {"K_p", "Kp", "fn"}, {"K_i", "Ki", "fn"}, {"K_d", "Kd", "fn"}, {"e_{ss}", "ess", "fn"}},
                {{"\\sigma_a", "sigma_a", "fn"}, {"\\theta_a", "theta_a", "fn"}, {"\\Omega_c", "Omega_c", "fn"}, {"|x|", "abs(", "fn"}, {"{", "{", "num"}, {"}", "}", "num"}, {"(", "(", "num"}, {")", ")", "num"}}
            };
        } else if (active_keypad_mode == "formulas") {
            layout = {
                {{"ζ(%OS)", "solve_zeta_from_os(", "op"}, {"ζ(Ts,ωn)", "solve_zeta_from_Ts_wn(", "op"}, {"ζ(Tp,ωn)", "solve_zeta_from_Tp_wn(", "op"}, {"ζ(Ts,Tp)", "solve_zeta_from_Ts_Tp(", "op"}, {"ζ(a,b)", "solve_zeta_from_ab(", "op"}, {"ωn(b)", "solve_wn_from_b(", "op"}},
                {{"ωn(Ts,ζ)", "solve_wn_from_Ts_zeta(", "fn"}, {"ωn(Tp,ζ)", "solve_wn_from_Tp_zeta(", "fn"}, {"ωd(ζ,ωn)", "solve_wd(", "fn"}, {"σd(ζ,ωn)", "solve_sigma_d(", "fn"}, {"%OS(ζ)", "solve_os(", "fn"}, {"Ts(ζ,ωn)", "solve_Ts(", "fn"}},
                {{"Tp(ζ,ωn)", "solve_Tp(", "fn"}, {"Tr(ζ,ωn)", "solve_Tr(", "fn"}, {"T_1st(a)", "solve_T_1st(", "fn"}, {"Tr_1st(a)", "solve_Tr_1st(", "fn"}, {"Ts_1st(a)", "solve_Ts_1st(", "fn"}, {"5× Rule", "check_5x_rule(", "fn"}},
                {{"Design Kp (P)", "solve_kp_for_os_p(", "fn"}, {"Design Kd (PD)", "solve_kd_for_os_pd(", "fn"}, {"Design Kp (PD)", "solve_kp_for_os_pd(", "fn"}, {"Design Ki (PI)", "solve_ki_for_os_pi(", "fn"}, {"e_ss Step", "solve_ess_step(", "fn"}, {"z = e^(sTs)", "solve_z_from_s(", "fn"}}
            };
        } else if (active_keypad_mode == "abc") {
            layout = {
                {{"q", "q", "num"}, {"w", "w", "num"}, {"e", "e", "num"}, {"r", "r", "num"}, {"t", "t", "num"}, {"y", "y", "num"}, {"u", "u", "num"}, {"i", "i", "num"}, {"o", "o", "num"}, {"p", "p", "num"}},
                {{"a", "a", "num"}, {"s", "s", "num"}, {"d", "d", "num"}, {"f", "f", "num"}, {"g", "g", "num"}, {"h", "h", "num"}, {"j", "j", "num"}, {"k", "k", "num"}, {"l", "l", "num"}, {"=", " = ", "op"}},
                {{"z", "z", "num"}, {"x", "x", "num"}, {"c", "c", "num"}, {"v", "v", "num"}, {"b", "b", "num"}, {"n", "n", "num"}, {"m", "m", "num"}, {"_", "_", "fn"}, {"(", "(", "fn"}, {")", ")", "fn"}},
                {{"zeta", "zeta", "fn"}, {"wn", "wn", "fn"}, {"Kp", "Kp", "fn"}, {"Ki", "Ki", "fn"}, {"Kd", "Kd", "fn"}, {"s_test", "s_test", "fn"}, {"z_test", "z_test", "fn"}, {"Ts_d", "Ts_d", "fn"}, {"R_mag", "R_mag", "fn"}, {"ans", "ans", "fn"}}
            };
        } else {
            layout = {
                {{"sin⁻¹", "asin(", "fn"}, {"cos⁻¹", "acos(", "fn"}, {"tan⁻¹", "atan(", "fn"}, {"ln", "ln(", "fn"}, {"log₁₀", "log(", "fn"}, {"eˣ", "exp(", "fn"}, {"10ˣ", "10^(", "fn"}, {"sqrt", "sqrt(", "fn"}},
                {{"real", "real(", "fn"}, {"imag", "imag(", "fn"}, {"arg ∠", "arg(", "fn"}, {"abs |z|", "abs(", "fn"}, {"dB", "dB(", "fn"}, {"Δθ", "solve_deficiency(", "fn"}, {"σa", "solve_sigma_a(", "fn"}, {"%e_ss", "solve_rel_err_pct(", "fn"}},
                {{"G(s)", "G(", "op"}, {"K_rl(s)", "K_rl(", "op"}, {"H_z(z)", "H_z(", "op"}, {"parallel", "parallel(", "op"}, {"mean", "mean(", "fn"}, {"s_test", "s_test", "fn"}, {"z_test", "z_test", "fn"}, {"ans", "ans", "fn"}}
            };
        }

        int n_rows = static_cast<int>(layout.size());
        int n_cols = layout.empty() ? 0 : static_cast<int>(layout[0].size());

        for (int r = 0; r < n_rows; ++r) keys_grid->setRowStretch(r, 1);
        for (int c = 0; c < n_cols; ++c) keys_grid->setColumnStretch(c, 1);

        for (int r = 0; r < n_rows; ++r) {
            for (int c = 0; c < static_cast<int>(layout[r].size()); ++c) {
                const auto& kspec = layout[r][c];
                QString bg, fg;
                if (kspec.kind == "num") {
                    bg = Theme::BG_KEY_NUM; // "#1e2636"
                    fg = Theme::TEXT_PRIMARY;
                } else if (kspec.kind == "op") {
                    bg = "#1e3a5f";
                    fg = Theme::ACCENT_ZERO; // "#38bdf8"
                } else {
                    bg = Theme::BG_KEY_FN;  // "#151b26"
                    fg = Theme::TEXT_PRIMARY;
                }

                auto* btn = new QPushButton(kspec.label);
                btn->setCursor(Qt::PointingHandCursor);
                btn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                btn->setMinimumHeight(38);
                btn->setStyleSheet(QString(
                    "QPushButton { background-color: %1; color: %2; border: 1px solid %3; border-radius: 2px; font-family: Monospace; font-weight: bold; font-size: 11px; }"
                    "QPushButton:hover { background-color: #2d3b54; }"
                ).arg(bg, fg, Theme::BORDER_INPUT));

                QString tok = kspec.token;
                connect(btn, &QPushButton::clicked, this, [this, tok]() {
                    if (!active_edit && !rows.empty()) active_edit = rows.back().edit;
                    if (!active_edit) return;
                    int pos = active_edit->cursorPosition();
                    active_edit->insert(tok);
                    if (tok == "\\frac{}{}") active_edit->setCursorPosition(pos + 6);
                    else if (tok == "\\sqrt{}" || tok == "^{}" || tok == "e^{}") active_edit->setCursorPosition(pos + tok.length() - 1);
                    else if (tok == "\\sqrt[]{}") active_edit->setCursorPosition(pos + 6);
                    active_edit->setFocus();
                    evaluateAll();
                });

                keys_grid->addWidget(btn, r, c);
            }
        }
    }

    void selectFormulaCategory(const std::string& cat) {
        active_formula_cat = cat;
        for (auto& [k, b] : cat_btns) {
            bool sel = (k == cat);
            b->setStyleSheet(QString("QPushButton { background-color: %1; color: %2; border: 1px solid #4b586e; border-radius: 4px; padding: 3px 7px; font-weight: bold; font-size: 10px; }")
                             .arg(sel ? Theme::BG_KEY_ACCENT : Theme::BG_INPUT, sel ? "#ffffff" : Theme::TEXT_SECONDARY));
        }
        renderFormulaCards();
    }

    void renderFormulaCards() {
        QLayoutItem* item;
        while ((item = fn_cards_layout->takeAt(0)) != nullptr) {
            delete item->widget();
            delete item;
        }
        std::string q = formula_search_edit->text().toLower().toStdString();
        for (const auto& spec : FORMULA_SHEET_FUNCS) {
            if (active_formula_cat != "all" && spec.section_key != active_formula_cat) continue;
            if (!q.empty()) {
                std::string hay = spec.name + " " + spec.title + " " + spec.description + " " + spec.ref_badge;
                std::transform(hay.begin(), hay.end(), hay.begin(), ::tolower);
                if (hay.find(q) == std::string::npos) continue;
            }

            auto* card = makeCard();
            card->setStyleSheet(QString("QFrame { background-color: %1; border: 1px solid %2; border-radius: 5px; }")
                                .arg(Theme::BG_PANEL, Theme::BORDER_INPUT));
            auto* cl = new QVBoxLayout(card);
            cl->setContentsMargins(8, 6, 8, 6);
            cl->setSpacing(3);

            auto* top_r = new QHBoxLayout();
            auto* tl = new QLabel(QString::fromStdString(spec.title));
            tl->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border:none;").arg(Theme::TEXT_PRIMARY));
            auto* ins_btn = makeActionBtn("Insert ➔", Theme::ACCENT_PID);
            auto* info_b  = new InfoBadgeButton(QString::fromStdString(spec.name + spec.signature_hint),
                                                QString::fromStdString(spec.description + "\n\nExample: " + spec.call_template),
                                                QString::fromStdString(spec.ref_badge));
            top_r->addWidget(tl); top_r->addStretch(); top_r->addWidget(ins_btn); top_r->addWidget(info_b);
            cl->addLayout(top_r);

            auto* sig_l = new QLabel(QString::fromStdString(spec.name + spec.signature_hint));
            sig_l->setStyleSheet(QString("color: %1; font-family: Monospace; font-size: 10px; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));
            cl->addWidget(sig_l);

            auto* math_w = new MathFormulaWidget(QString::fromStdString(spec.latex_formula), QColor(Theme::ACCENT_LOCUS), QColor(Theme::BG_PANEL), 10);
            cl->addWidget(math_w);

            QString tpl = QString::fromStdString(spec.call_template);
            connect(ins_btn, &QPushButton::clicked, this, [this, tpl]() { insertTemplate(tpl); });
            fn_cards_layout->addWidget(card);
        }
    }

    void insertTemplate(const QString& tpl) {
        if (rows.empty()) addRow(tpl);
        else if (active_edit && active_edit->text().trimmed().isEmpty()) {
            active_edit->setText(tpl);
        } else {
            addRow(tpl);
        }
        evaluateAll();
    }

    void removeRowAt(size_t idx) {
        if (rows.size() <= 1 || idx >= rows.size()) return;

        // Decide which remaining row should receive focus (prefer the line above)
        size_t next_idx = (idx > 0) ? (idx - 1) : 0;
        QWidget* w_to_delete = rows[idx].frame;

        rows.erase(rows.begin() + static_cast<std::ptrdiff_t>(idx));

        // Renumber the remaining line labels (1, 2, 3, ...)
        for (size_t i = 0; i < rows.size(); ++i) {
            if (auto* lbl = rows[i].frame->findChild<QLabel*>()) {
                lbl->setText(QString::number(i + 1));
            }
        }

        // Focus the target row and move the cursor to the end of its text
        active_edit = rows[next_idx].edit;
        active_edit->setFocus();
        active_edit->setCursorPosition(active_edit->text().length());

        // Safely schedule the widget destruction for the next event loop tick
        if (w_to_delete) {
            rows_layout->removeWidget(w_to_delete);
            w_to_delete->deleteLater();
        }

        evaluateAll();
        scrollToActiveRow();
    }

    bool eventFilter(QObject* obj, QEvent* ev) override {
        if (ev->type() == QEvent::FocusIn) {
            if (auto* ed = qobject_cast<QLineEdit*>(obj)) {
                active_edit = ed;
            }
        } else if (ev->type() == QEvent::KeyPress) {
            auto* ke = static_cast<QKeyEvent*>(ev);
            if (ke->key() == Qt::Key_Backspace) {
                if (auto* ed = qobject_cast<QLineEdit*>(obj)) {
                    if (ed->text().isEmpty() && rows.size() > 1) {
                        for (size_t i = 0; i < rows.size(); ++i) {
                            if (rows[i].edit == ed) {
                                removeRowAt(i);
                                return true; // Consume the Backspace event
                            }
                        }
                    }
                }
            }
        }
        return QWidget::eventFilter(obj, ev);
    }

    void scrollToActiveRow() {
        if (!expr_scroll) return;
        QTimer::singleShot(15, this, [this]() {
            if (!expr_scroll || !active_edit) return;
            // Force the scroll container to synchronously recompute its layout size
            if (expr_scroll->widget() && expr_scroll->widget()->layout()) {
                expr_scroll->widget()->layout()->activate();
                expr_scroll->widget()->adjustSize();
            }
            // Verify active_edit still belongs to a live row before scrolling to it
            for (const auto& r : rows) {
                if (r.edit == active_edit && r.frame) {
                    expr_scroll->ensureWidgetVisible(r.frame, 0, 16);
                    if (&r == &rows.back() && expr_scroll->verticalScrollBar()) {
                        expr_scroll->verticalScrollBar()->setValue(
                            expr_scroll->verticalScrollBar()->maximum()
                        );
                    }
                    break;
                }
            }
        });
    }

    void addRow(const QString& init_expr, bool focus_new = true) {
        auto* row_w = new QFrame();
        row_w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        row_w->setStyleSheet(QString("QFrame { background-color: %1; border: 1px solid %2; border-radius: 4px; }")
                             .arg(Theme::BG_PANEL_ALT, Theme::BORDER_PANEL));

        auto* outer_vl = new QVBoxLayout(row_w);
        outer_vl->setContentsMargins(8, 4, 8, 4);
        outer_vl->setSpacing(3);

        // Top line: Row # + Expression Input + Math Preview + (Inline Answer if short)
        auto* top_hl = new QHBoxLayout();
        top_hl->setContentsMargins(0, 0, 0, 0);
        top_hl->setSpacing(6);

        auto* idx_l = new QLabel(QString::number(rows.size() + 1));
        idx_l->setFixedWidth(20);
        idx_l->setStyleSheet("color: #64748b; font-family: Monospace; font-weight: bold; border:none;");

        auto* ed = makeDarkEntry(init_expr, 140);
        ed->setMinimumWidth(120);
        ed->installEventFilter(this);

        auto* math_prev = new MathFormulaWidget("", QColor(Theme::ACCENT_LOCUS), QColor(Theme::BG_PANEL_ALT), 9);
        math_prev->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Minimum);

        // Answer bundle (a/b button + result label) that can sit inline OR drop to line 2
        auto* res_cont = new QWidget();
        res_cont->setStyleSheet("background: transparent; border: none;");
        auto* res_hl = new QHBoxLayout(res_cont);
        res_hl->setContentsMargins(0, 0, 0, 0);
        res_hl->setSpacing(6);

        auto* frac_btn = makeActionBtn("a/b", Theme::TEXT_SECONDARY, Theme::BG_PANEL);
        frac_btn->setVisible(false);
        auto* res_l = new QLabel("= —");
        res_l->setWordWrap(true);
        res_l->setStyleSheet(QString("color: %1; font-family: Monospace; font-size: 12px; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));

        res_hl->addWidget(frac_btn);
        res_hl->addWidget(res_l);

        auto* inline_slot = new QHBoxLayout();
        inline_slot->setContentsMargins(0, 0, 0, 0);
        inline_slot->addWidget(res_cont);

        top_hl->addWidget(idx_l);
        top_hl->addWidget(ed, 1);
        top_hl->addWidget(math_prev);
        top_hl->addLayout(inline_slot);
        outer_vl->addLayout(top_hl);

        // Second line (hidden unless LaTeX or Answer is wide)
        auto* wrap_w = new QWidget();
        wrap_w->setStyleSheet("background: transparent; border: none;");
        auto* wrap_hl = new QHBoxLayout(wrap_w);
        wrap_hl->setContentsMargins(26, 2, 0, 0);
        wrap_hl->setSpacing(8);
        wrap_hl->addStretch();
        auto* wrapped_slot = new QHBoxLayout();
        wrapped_slot->setContentsMargins(0, 0, 0, 0);
        wrap_hl->addLayout(wrapped_slot);
        wrap_w->setVisible(false);
        outer_vl->addWidget(wrap_w);

        rows_layout->addWidget(row_w);

        size_t r_idx = rows.size();
        rows.push_back({row_w, ed, math_prev, frac_btn, res_l, false, res_cont, inline_slot, wrapped_slot, wrap_w});
        active_edit = ed;

        connect(ed, &QLineEdit::textChanged, this, &ScientificCalculatorTab::evaluateAll);
        connect(ed, &QLineEdit::returnPressed, this, [this, ed]() {
            for (size_t i = 0; i < rows.size(); ++i) {
                if (rows[i].edit == ed) {
                    if (i + 1 < rows.size()) {
                        rows[i + 1].edit->setFocus();
                        active_edit = rows[i + 1].edit;
                        scrollToActiveRow();
                    } else {
                        addRow("", true);
                    }
                    break;
                }
            }
        });
        connect(frac_btn, &QPushButton::clicked, this, [this, r_idx]() {
            if (r_idx < rows.size()) {
                rows[r_idx].use_frac = !rows[r_idx].use_frac;
                evaluateAll();
            }
        });

        if (focus_new) {
            ed->setFocus(Qt::OtherFocusReason);
            ed->setCursorPosition(ed->text().length());
            scrollToActiveRow();
        }
    }

    static std::string exprToLatexPreview(const std::string& raw) {
        std::string out;
        size_t i = 0, n = raw.size();

        static const std::map<std::string, std::string> token_map = {
            {"solve_zeta_from_os",  "\\zeta_{\\mathrm{OS}}"},
            {"solve_os_from_zeta",  "\\%OS_{\\zeta}"},
            {"solve_wn_from_ts",    "\\omega_{n,T_s}"},
            {"solve_wn_from_tp",    "\\omega_{n,T_p}"},
            {"solve_ts",            "T_s"},
            {"solve_tp",            "T_p"},
            {"solve_tr",            "T_r"},
            {"solve_kp_for_os_p",   "K_{p,\\mathrm{P}}"},
            {"solve_kd_for_os_pd",  "K_{d,\\mathrm{PD}}"},
            {"solve_kp_pd",         "K_{p,\\mathrm{PD}}"},
            {"solve_kd_pd",         "K_{d,\\mathrm{PD}}"},
            {"solve_ess_step",      "e_{\\mathrm{ss}}"},
            {"solve_rel_err_pct",   "\\%e_{\\mathrm{ss}}"},
            {"solve_kp_for_ess",    "K_{p,e_{\\mathrm{ss}}}"},
            {"solve_deficiency",    "\\Delta\\theta"},
            {"solve_sigma_a",       "\\sigma_a"},
            {"K_rl",                "K_{\\mathrm{RL}}"},
            {"H_z",                 "H_z"},
            {"s_test",              "s_{\\mathrm{test}}"},
            {"z_test",              "z_{\\mathrm{test}}"},
            {"parallel",            "\\mathrm{parallel}"},
            {"par",                 "\\mathrm{parallel}"},
            {"sqrt",                "\\sqrt"},
            {"pi",                  "\\pi"}
        };

        while (i < n) {
            if (std::isalpha(static_cast<unsigned char>(raw[i]))) {
                size_t st = i;
                while (i < n && (std::isalnum(static_cast<unsigned char>(raw[i])) || raw[i] == '_')) ++i;
                std::string ident = raw.substr(st, i - st);

                auto it = token_map.find(ident);
                if (it != token_map.end()) {
                    out += it->second;
                } else {
                    // For any other multi-part snake_case identifier, format first_rest as first_{rest}
                    size_t us = ident.find('_');
                    if (us != std::string::npos && us + 1 < ident.size()) {
                        std::string head = ident.substr(0, us);
                        std::string tail = ident.substr(us + 1);
                        // Replace any remaining underscores in tail with commas/spaces
                        for (char& c : tail) if (c == '_') c = ',';
                        out += head + "_{\\mathrm{" + tail + "}}";
                    } else {
                        out += ident;
                    }
                }
            } else if (raw[i] == '*') {
                out += " \\cdot ";
                ++i;
            } else {
                out.push_back(raw[i++]);
            }
        }
        return out;
    }

    void evaluateAll() {
        engine.resetConstants();
        engine.syncFromModel();
        for (auto& r : rows) {
            std::string txt = r.edit->text().toStdString();
            if (txt.empty()) {
                r.math_prev->setVisible(false);
                r.frac_btn->setVisible(false);
                r.res_lbl->setText("");
                if (r.wrapped_row_w) r.wrapped_row_w->setVisible(false);
                continue;
            }
            auto out = engine.evalLine(txt);
            r.math_prev->setVisible(show_latex_preview);
            if (show_latex_preview) {
                if (out.latex_preview.find("\\frac") != std::string::npos ||
                    out.latex_preview.find("\\mathrm{sym}") != std::string::npos ||
                    out.latex_preview.find(" = 0") != std::string::npos) {
                    r.math_prev->setLatex(QString::fromStdString(out.latex_preview));
                } else {
                    r.math_prev->setLatex(QString::fromStdString(exprToLatexPreview(out.latex_preview)));
                }
            }

            if (out.ok) {
                r.res_lbl->setStyleSheet(QString("color: %1; font-family: Monospace; font-size: 12px; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));
                bool has_fr = !out.formatted_frac.empty();
                r.frac_btn->setVisible(has_fr);
                std::string disp = (r.use_frac && has_fr) ? out.formatted_frac : out.formatted_dec;
                r.res_lbl->setText(QString::fromStdString(disp));
            } else {
                r.frac_btn->setVisible(false);
                r.res_lbl->setStyleSheet(QString("color: %1; font-family: Monospace; font-size: 11px; border:none;").arg(Theme::ACCENT_OPEN));
                r.res_lbl->setText(QString::fromStdString("⚠ " + out.error_msg));
            }
            // Check if the LaTeX preview + Answer is too wide for a single line
            int latex_w = (show_latex_preview && r.math_prev->isVisible()) ? r.math_prev->sizeHint().width() : 0;
            int ans_w   = r.res_lbl->fontMetrics().horizontalAdvance(r.res_lbl->text()) + (r.frac_btn->isVisible() ? 45 : 0);
            int avail_w = (r.frame->width() > 200) ? r.frame->width() : 520;

            bool needs_wrap = (latex_w + ans_w > avail_w - 190) || (ans_w > 220) || (latex_w > 180);
            if (r.inline_slot && r.wrapped_slot && r.wrapped_row_w && r.res_container) {
                r.inline_slot->removeWidget(r.res_container);
                r.wrapped_slot->removeWidget(r.res_container);
                if (needs_wrap) {
                    r.wrapped_slot->addWidget(r.res_container);
                    r.wrapped_row_w->setVisible(true);
                } else {
                    r.inline_slot->addWidget(r.res_container);
                    r.wrapped_row_w->setVisible(false);
                }
            }
        }
        live_vars_lbl->setText(QString(
            "Live Vars: ζ=%1, ωn=%2, ωd=%3, σd=%4 | Kp=%5, Ki=%6, Kd=%7 | Ts=%8s"
        ).arg(QString::fromStdString(PRECISION.fmt(engine.vars["zeta"].real())))
         .arg(QString::fromStdString(PRECISION.fmt(engine.vars["wn"].real())))
         .arg(QString::fromStdString(PRECISION.fmt(engine.vars["wd"].real())))
         .arg(QString::fromStdString(PRECISION.fmt(engine.vars["sigma_d"].real())))
         .arg(model->kp).arg(model->ki).arg(model->kd).arg(model->ts));
    }

    QStringList getExpressions() const {
        QStringList list;
        for (const auto& r : rows) {
            if (!r.edit->text().trimmed().isEmpty()) list << r.edit->text();
        }
        return list;
    }

    void setExpressions(const QStringList& list) {
        if (list.isEmpty()) return;
        for (auto& r : rows) delete r.frame;
        rows.clear();
        active_edit = nullptr;
        for (const QString& ex : list) addRow(ex, false);
        evaluateAll();
    }
};

