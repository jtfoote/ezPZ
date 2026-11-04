// =============================================================================
// Unified Control Systems, Digital Filter & Desmos-Style Scientific Calculator
// Native C++17 / Qt6 / Eigen3 Desktop & CLI Suite
// =============================================================================

#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QFrame>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QSlider>
#include <QScrollArea>
#include <QStackedWidget>
#include <QTextEdit>
#include <QDialog>
#include <QMessageBox>
#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QShortcut>
#include <QFontDatabase>

#include <Eigen/Dense>

#include <algorithm>
#include <cmath>
#include <complex>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <numeric>
#include <optional>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using Complex = std::complex<double>;
using Poly = std::vector<Complex>; // Descending powers: [a_n, a_{n-1}, ..., a_0]

static constexpr double PI = 3.14159265358979323846;

// =============================================================================
// Theme Palette (Dark-Slate Engineering Theme)
// =============================================================================
namespace Theme {
    const QString BG_WINDOW      = "#0b0e14";
    const QString BG_HEADER      = "#0f141d";
    const QString BG_PANEL       = "#131822";
    const QString BG_PANEL_ALT   = "#171e2b";
    const QString BG_INPUT       = "#0d111a";
    const QString BG_PLOT        = "#090c12";
    const QString BG_KEY_NUM     = "#1e2636";
    const QString BG_KEY_FN      = "#151b26";
    const QString BG_KEY_ACCENT  = "#2563eb";
    const QString BORDER_PANEL   = "#222b3a";
    const QString BORDER_INPUT   = "#252f42";
    const QString BORDER_FOCUS   = "#38bdf8";
    const QString BORDER_BTN     = "#4b586e";
    const QString TEXT_PRIMARY   = "#f0f6fc";
    const QString TEXT_SECONDARY = "#94a3b8";
    const QString TEXT_MUTED     = "#64748b";
    const QString ACCENT_POLE    = "#ff5f56";
    const QString ACCENT_ZERO    = "#38bdf8";
    const QString ACCENT_PID     = "#34d399";
    const QString ACCENT_LOCUS   = "#a855f7";
    const QString ACCENT_ARROW   = "#d8b4fe";
    const QString ACCENT_ASYMP   = "#22d3ee";
    const QString ACCENT_OPEN    = "#f59e0b";
    const QString ACCENT_IMAG    = "#ec4899";
    const QString BANNER_OK_BG   = "#0d2e22";
    const QString BANNER_OK_BD   = "#2ea043";
    const QString BANNER_OK_FG   = "#3fb950";
    const QString BANNER_WARN_BG = "#33270f";
    const QString BANNER_WARN_BD = "#d29922";
    const QString BANNER_WARN_FG = "#e3b341";
    const QString BANNER_ERR_BG  = "#361519";
    const QString BANNER_ERR_BD  = "#f85149";
    const QString BANNER_ERR_FG  = "#ff7b72";
    const QString GRID_MAJOR     = "#172030";
    const QString AXIS_LINE      = "#3b4659";
}

// =============================================================================
// Engine 00: Classroom Decimal Precision & Numeric Parser
// =============================================================================
struct PrecisionConfig {
    std::string mode = "class"; // "class", "fixed", "auto"
    int dec_sub1 = 3;           // 0 < |x| < 1
    int dec_ge1  = 2;           // |x| >= 1
    bool step_round = false;

    int decimalsFor(double val) const {
        double av = std::abs(val);
        if (mode == "fixed") return std::max(0, dec_ge1);
        if (mode == "auto")  return (av > 0.0 && av < 1.0) ? 4 : 3;
        if (av > 0.0 && av < 1.0 - 1e-12) return std::max(0, dec_sub1);
        return std::max(0, dec_ge1);
    }

    double roundVal(double val) const {
        if (!std::isfinite(val)) return val;
        if (std::abs(val) < 1e-14) return 0.0;
        double scale = std::pow(10.0, decimalsFor(val));
        return std::round(val * scale) / scale;
    }

    double step(double val) const {
        return step_round ? roundVal(val) : val;
    }

    std::string fmt(std::optional<double> val, bool signed_fmt = false, bool strip_zeros = false, const std::string& fallback = "—") const {
        if (!val.has_value()) return fallback;
        double fval = val.value();
        if (std::isnan(fval)) return "—";
        if (std::isinf(fval)) return (fval > 0) ? (signed_fmt ? "+∞" : "∞") : "-∞";
        if (std::abs(fval) < 1e-12) fval = 0.0;

        int d = decimalsFor(fval);
        std::ostringstream oss;
        if (signed_fmt && fval >= 0.0) oss << "+";
        oss << std::fixed << std::setprecision(d) << fval;
        std::string out = oss.str();

        if (strip_zeros && out.find('.') != std::string::npos) {
            while (!out.empty() && out.back() == '0') out.pop_back();
            if (!out.empty() && out.back() == '.') out.pop_back();
            if (out.empty() || out == "-" || out == "+") out = "0";
        }
        if (out == "-0" || out == "-0.00" || out == "-0.000") {
            out.erase(0, 1);
            if (signed_fmt && out[0] != '+') out = "+" + out;
        }
        return out;
    }

    std::string fmtComplex(Complex val, bool strip_zeros = false) const {
        if (!std::isfinite(val.real()) || !std::isfinite(val.imag())) return "Divergent";
        std::string re_s = fmt(val.real(), false, strip_zeros);
        if (std::abs(val.imag()) < 1e-4) return re_s;
        std::string im_s = fmt(val.imag(), true, strip_zeros);
        if (std::abs(val.real()) < 1e-9) {
            return fmt(val.imag(), false, strip_zeros) + "j";
        }
        return re_s + im_s + "j";
    }

    std::string summaryLabel() const {
        if (mode == "class") {
            return "0<|x|<1: " + std::to_string(dec_sub1) + " dec, |x|≥1: " + std::to_string(dec_ge1) + " dec" + (step_round ? " • Step-Round ON" : "");
        }
        if (mode == "fixed") return "Fixed " + std::to_string(dec_ge1) + " dec";
        return "Auto Precision";
    }
};

static PrecisionConfig PRECISION;

double parseNumericInput(const std::string& raw, double def = 0.0) {
    std::string s = raw;
    s.erase(std::remove(s.begin(), s.end(), ','), s.end());
    auto ltrim = [](std::string& str) { str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) { return !std::isspace(ch); })); };
    auto rtrim = [](std::string& str) { str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), str.end()); };
    ltrim(s); rtrim(s);
    if (s.empty()) return def;

    const std::vector<std::string> units = {"rad/s", "r/s", "deg", "sec", "%", "s"};
    for (const auto& u : units) {
        if (s.size() >= u.size()) {
            std::string tail = s.substr(s.size() - u.size());
            std::transform(tail.begin(), tail.end(), tail.begin(), ::tolower);
            if (tail == u) {
                s = s.substr(0, s.size() - u.size());
                rtrim(s);
                break;
            }
        }
    }
    try {
        return std::stod(s);
    } catch (...) {
        return def;
    }
}

// =============================================================================
// Engine 0: Polynomial Algebra & Root Solver (Weierstrass Durand-Kerner + Eigen)
// =============================================================================
Poly polyTrim(const Poly& p, double tol = 1e-13) {
    auto it = std::find_if(p.begin(), p.end(), [tol](const Complex& c) { return std::abs(c) > tol; });
    if (it == p.end()) return {Complex(0.0, 0.0)};
    return Poly(it, p.end());
}

Poly polyAdd(const Poly& a, const Poly& b) {
    size_t n = std::max(a.size(), b.size());
    Poly res(n, Complex(0.0, 0.0));
    for (size_t i = 0; i < a.size(); ++i) res[n - a.size() + i] += a[i];
    for (size_t i = 0; i < b.size(); ++i) res[n - b.size() + i] += b[i];
    return res;
}

Poly polySub(const Poly& a, const Poly& b) {
    Poly neg_b = b;
    for (auto& c : neg_b) c = -c;
    return polyAdd(a, neg_b);
}

Poly polyMul(const Poly& a, const Poly& b) {
    if (a.empty() || b.empty()) return {Complex(0.0, 0.0)};
    Poly res(a.size() + b.size() - 1, Complex(0.0, 0.0));
    for (size_t i = 0; i < a.size(); ++i) {
        for (size_t j = 0; j < b.size(); ++j) {
            res[i + j] += a[i] * b[j];
        }
    }
    return res;
}

Poly polyScale(const Poly& a, Complex s) {
    Poly res = a;
    for (auto& c : res) c *= s;
    return res;
}

Complex polyVal(const Poly& p, Complex x) {
    Complex acc(0.0, 0.0);
    for (const auto& c : p) acc = acc * x + c;
    return acc;
}

Poly polyFromRoots(const std::vector<Complex>& roots) {
    Poly res = {Complex(1.0, 0.0)};
    for (const auto& r : roots) {
        res = polyMul(res, {Complex(1.0, 0.0), -r});
    }
    return res;
}

std::vector<Complex> solvePolynomialRoots(const Poly& coeffs_in, const std::vector<Complex>* warm_start = nullptr) {
    Poly c = polyTrim(coeffs_in, 1e-14);
    if (c.size() <= 1) return {};
    size_t deg = c.size() - 1;
    Complex lead = c[0];
    for (auto& val : c) val /= lead;

    if (deg == 1) return {-c[1]};
    if (deg == 2) {
        Complex b = c[1], d = c[2];
        Complex disc = std::sqrt(b * b - 4.0 * d);
        return {(-b + disc) / 2.0, (-b - disc) / 2.0};
    }

    std::vector<Complex> roots(deg);
    if (warm_start && warm_start->size() == deg) {
        for (size_t i = 0; i < deg; ++i) {
            double ang = 2.0 * PI * double(i) / double(deg) + 0.37;
            roots[i] = (*warm_start)[i] + std::polar(1e-7, ang);
        }
    } else {
        double max_c = 0.0;
        for (size_t i = 1; i <= deg; ++i) max_c = std::max(max_c, std::abs(c[i]));
        double radius = 0.4 + std::pow(std::max(max_c, 1.0), 1.0 / double(deg));
        for (size_t i = 0; i < deg; ++i) {
            double ang = 2.0 * PI * double(i) / double(deg) + 0.4123;
            roots[i] = std::polar(radius, ang);
        }
    }

    for (int iter = 0; iter < 80; ++iter) {
        double max_delta = 0.0;
        for (size_t i = 0; i < deg; ++i) {
            Complex f_val = polyVal(c, roots[i]);
            Complex prod(1.0, 0.0);
            for (size_t j = 0; j < deg; ++j) {
                if (i == j) continue;
                Complex diff = roots[i] - roots[j];
                if (std::abs(diff) < 1e-14) diff = std::polar(1e-12, 0.73 * double(i + j));
                prod *= diff;
            }
            Complex delta = f_val / prod;
            roots[i] -= delta;
            max_delta = std::max(max_delta, std::abs(delta));
        }
        if (max_delta < 1e-10) break;
    }

    for (auto& r : roots) {
        if (std::abs(r.real()) < 1e-10) r.real(0.0);
        if (std::abs(r.imag()) < 1e-10) r.imag(0.0);
    }
    return roots;
}

// Recursive descent parser for algebraic polynomials in s or z (e.g. "s^2 + 2s + 2", "(s+1)(s+3)")
class AlgebraicPolyParser {
    std::string src;
    size_t pos = 0;

    void skipSpaces() {
        while (pos < src.size() && std::isspace(static_cast<unsigned char>(src[pos]))) ++pos;
    }

public:
    explicit AlgebraicPolyParser(std::string s) : src(std::move(s)) {}

    Poly parse() {
        Poly res = parseExpr();
        skipSpaces();
        if (pos < src.size()) throw std::runtime_error("Unexpected token in polynomial near '" + src.substr(pos) + "'");
        return polyTrim(res, 1e-13);
    }

private:
    Poly parseExpr() {
        Poly left = parseTerm();
        while (true) {
            skipSpaces();
            if (pos < src.size() && (src[pos] == '+' || src[pos] == '-')) {
                char op = src[pos++];
                Poly right = parseTerm();
                left = (op == '+') ? polyAdd(left, right) : polySub(left, right);
            } else {
                break;
            }
        }
        return left;
    }

    Poly parseTerm() {
        Poly left = parsePower();
        while (true) {
            skipSpaces();
            if (pos < src.size() && src[pos] == '*') {
                ++pos;
                left = polyMul(left, parsePower());
            } else if (pos < src.size() && src[pos] == '/') {
                ++pos;
                Poly right = parsePower();
                right = polyTrim(right);
                if (right.size() != 1 || std::abs(right[0]) < 1e-14)
                    throw std::runtime_error("Polynomial division only supports non-zero scalar denominators");
                left = polyScale(left, 1.0 / right[0]);
            } else if (pos < src.size() && (src[pos] == '(' || src[pos] == 's' || src[pos] == 'S' || src[pos] == 'z' || src[pos] == 'Z')) {
                // Implicit multiplication: e.g. 2s, (s+1)(s+2), s(s+2)
                left = polyMul(left, parsePower());
            } else {
                break;
            }
        }
        return left;
    }

    Poly parsePower() {
        Poly base = parseUnary();
        skipSpaces();
        if (pos < src.size() && src[pos] == '^') {
            ++pos;
            Poly exp_p = parseUnary();
            exp_p = polyTrim(exp_p);
            if (exp_p.size() != 1) throw std::runtime_error("Exponent must be a non-negative integer");
            int n = static_cast<int>(std::round(exp_p[0].real()));
            if (n < 0 || std::abs(exp_p[0].real() - n) > 1e-7)
                throw std::runtime_error("Polynomial exponent must be a non-negative integer");
            Poly res = {Complex(1.0, 0.0)};
            for (int i = 0; i < n; ++i) res = polyMul(res, base);
            return res;
        }
        return base;
    }

    Poly parseUnary() {
        skipSpaces();
        if (pos < src.size() && src[pos] == '+') { ++pos; return parseUnary(); }
        if (pos < src.size() && src[pos] == '-') { ++pos; return polyScale(parseUnary(), Complex(-1.0, 0.0)); }
        return parsePrimary();
    }

    Poly parsePrimary() {
        skipSpaces();
        if (pos >= src.size()) throw std::runtime_error("Unexpected end of polynomial");
        if (src[pos] == '(') {
            ++pos;
            Poly inner = parseExpr();
            skipSpaces();
            if (pos >= src.size() || src[pos] != ')') throw std::runtime_error("Missing closing ')'");
            ++pos;
            return inner;
        }
        if (src[pos] == 's' || src[pos] == 'S' || src[pos] == 'z' || src[pos] == 'Z') {
            ++pos;
            return {Complex(1.0, 0.0), Complex(0.0, 0.0)};
        }
        if (src[pos] == 'i' || src[pos] == 'j') {
            ++pos;
            return {Complex(0.0, 1.0)};
        }
        if (std::isdigit(static_cast<unsigned char>(src[pos])) || src[pos] == '.') {
            size_t start = pos;
            while (pos < src.size() && (std::isdigit(static_cast<unsigned char>(src[pos])) || src[pos] == '.')) ++pos;
            if (pos < src.size() && (src[pos] == 'e' || src[pos] == 'E')) {
                ++pos;
                if (pos < src.size() && (src[pos] == '+' || src[pos] == '-')) ++pos;
                while (pos < src.size() && std::isdigit(static_cast<unsigned char>(src[pos]))) ++pos;
            }
            double val = std::stod(src.substr(start, pos - start));
            if (pos < src.size() && (src[pos] == 'i' || src[pos] == 'j')) {
                ++pos;
                return {Complex(0.0, val)};
            }
            return {Complex(val, 0.0)};
        }
        throw std::runtime_error(std::string("Unexpected character '") + src[pos] + "'");
    }
};

struct ParsedPolynomial {
    Poly coeffs;
    std::vector<Complex> roots;
    double leading_gain = 1.0;
};

ParsedPolynomial parsePolynomialInput(const std::string& raw_str) {
    std::string s = raw_str;
    // Check if comma-separated coefficient list without 's' or 'z'
    bool has_var = false;
    for (char c : s) {
        if (c == 's' || c == 'S' || c == 'z' || c == 'Z') { has_var = true; break; }
    }
    if (!has_var && s.find(',') != std::string::npos) {
        Poly coeffs;
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, ',')) {
            coeffs.push_back(Complex(parseNumericInput(item, 0.0), 0.0));
        }
        coeffs = polyTrim(coeffs);
        double lead = std::abs(coeffs[0].real()) > 1e-14 ? coeffs[0].real() : 1.0;
        return {coeffs, solvePolynomialRoots(coeffs), lead};
    }

    AlgebraicPolyParser parser(s);
    Poly coeffs = parser.parse();
    double lead = std::abs(coeffs[0].real()) > 1e-14 ? coeffs[0].real() : 1.0;
    return {coeffs, solvePolynomialRoots(coeffs), lead};
}

std::string formatPolynomialFromRoots(const std::vector<Complex>& roots, const std::string& var_name = "s", double scale = 1.0) {
    if (roots.empty()) return PRECISION.fmt(scale, false, true);
    Poly coeffs = polyScale(polyFromRoots(roots), Complex(scale, 0.0));
    int deg = static_cast<int>(coeffs.size()) - 1;
    std::string out;

    for (size_t idx = 0; idx < coeffs.size(); ++idx) {
        int power = deg - static_cast<int>(idx);
        double re_v = std::abs(coeffs[idx].real()) < 1e-10 ? 0.0 : coeffs[idx].real();
        double im_v = std::abs(coeffs[idx].imag()) < 1e-8  ? 0.0 : coeffs[idx].imag();
        if (re_v == 0.0 && im_v == 0.0) continue;

        bool is_neg = re_v < 0.0;
        double abs_v = std::abs(re_v);
        std::string num_s = (power == 0 || std::abs(abs_v - 1.0) > 1e-9) ? PRECISION.fmt(abs_v, false, true) : "";
        std::string var_s = (power == 0) ? "" : ((power == 1) ? var_name : (var_name + "^" + std::to_string(power)));
        std::string term = num_s + var_s;

        if (out.empty()) {
            out += (is_neg ? "-" : "") + term;
        } else {
            out += (is_neg ? " - " : " + ") + term;
        }
    }
    return out.empty() ? "1" : out;
}

// =============================================================================
// Engine 0B & Engine 1: Nise Ch. 4 Equations, Root Locus & Damping Metrics
// =============================================================================
double niseZetaFromOs(double os_pct) {
    if (os_pct <= 0.0) return 1.0;
    if (os_pct >= 100.0) return 0.0;
    double ln_os = std::log(os_pct / 100.0);
    return -ln_os / std::sqrt(PI * PI + ln_os * ln_os);
}

double niseOsFromZeta(double zeta) {
    if (zeta >= 1.0) return 0.0;
    if (zeta <= 0.0) return 100.0;
    return 100.0 * std::exp(-zeta * PI / std::sqrt(std::max(1e-12, 1.0 - zeta * zeta)));
}

double niseNormalizedTrFromZeta(double zeta) {
    double z = std::max(0.0, zeta);
    return 1.76 * z * z * z - 0.417 * z * z + 1.039 * z + 1.0;
}

double niseTrFromZetaWn(double zeta, double wn) {
    return niseNormalizedTrFromZeta(zeta) / std::max(wn, 1e-9);
}

std::vector<Complex> polesFromZetaWn(double zeta, double wn) {
    double w = std::max(wn, 1e-6);
    double disc = zeta * zeta - 1.0;
    if (disc < -1e-12) {
        double sigma = -zeta * w;
        double wd = w * std::sqrt(std::max(0.0, 1.0 - zeta * zeta));
        return {Complex(sigma, wd), Complex(sigma, -wd)};
    }
    double sq = std::sqrt(std::max(0.0, disc));
    return {Complex((-zeta + sq) * w, 0.0), Complex((-zeta - sq) * w, 0.0)};
}

std::vector<Complex> zPolesFromZetaWn(double zeta, double wn, double ts) {
    double t = std::max(ts, 1e-6);
    auto sp = polesFromZetaWn(zeta, wn);
    return {std::exp(sp[0] * t), std::exp(sp[1] * t)};
}

struct DominantMetrics {
    std::optional<double> zeta;
    std::optional<double> omega_n;
    std::optional<double> omega_d;
    std::optional<Complex> dom_pole;
};

DominantMetrics computeDominantSMetrics(const std::vector<Complex>& poles, double deriv_filter_n = 20.0) {
    if (poles.empty()) return {};
    std::vector<Complex> non_origin;
    for (const auto& p : poles) if (std::abs(p) > 1e-8) non_origin.push_back(p);
    if (non_origin.empty()) return {0.0, 0.0, 0.0, Complex(0.0, 0.0)};

    double cutoff = -0.85 * std::max(deriv_filter_n, 15.0);
    std::vector<Complex> candidates;
    for (const auto& p : non_origin) {
        if (!(std::abs(p.imag()) < 1e-4 && p.real() <= cutoff)) candidates.push_back(p);
    }
    if (candidates.empty()) candidates = non_origin;

    std::vector<Complex> complex_pool;
    for (const auto& p : candidates) if (p.imag() > 1e-4) complex_pool.push_back(p);

    if (!complex_pool.empty()) {
        auto dom_c = *std::max_element(complex_pool.begin(), complex_pool.end(), [](const Complex& a, const Complex& b) {
            if (std::abs(a.real() - b.real()) > 1e-5) return a.real() < b.real();
            return std::abs(a.imag()) < std::abs(b.imag());
        });
        double wn = std::abs(dom_c);
        double zeta = (wn > 1e-12) ? (-dom_c.real() / wn) : 0.0;
        return {zeta, wn, std::abs(dom_c.imag()), dom_c};
    }

    std::sort(candidates.begin(), candidates.end(), [](const Complex& a, const Complex& b) { return a.real() > b.real(); });
    Complex dom_p = candidates[0];
    if (candidates.size() >= 2 && candidates[0].real() < -1e-9 && candidates[1].real() < -1e-9) {
        double r1 = std::abs(candidates[0].real());
        double r2 = std::abs(candidates[1].real());
        if (r2 <= 4.95 * r1) {
            double wn_od = std::sqrt(r1 * r2);
            double zeta_od = (r1 + r2) / (2.0 * std::max(wn_od, 1e-12));
            return {zeta_od, wn_od, 0.0, dom_p};
        }
    }
    double wn = std::abs(dom_p);
    double zeta = (wn > 1e-12) ? (-dom_p.real() / wn) : 0.0;
    return {zeta, wn, std::abs(dom_p.imag()), dom_p};
}

DominantMetrics computeDominantZMetrics(const std::vector<Complex>& z_poles, double ts = 0.05) {
    if (z_poles.empty()) return {};
    double ts_val = std::max(ts, 1e-6);
    std::vector<Complex> pool;
    for (const auto& p : z_poles) if (std::abs(p) > 1e-6 && p.imag() >= -1e-6) pool.push_back(p);
    if (pool.empty()) pool = z_poles;

    Complex dom_z = *std::max_element(pool.begin(), pool.end(), [](const Complex& a, const Complex& b) {
        return std::abs(a) < std::abs(b);
    });
    if (std::abs(dom_z) < 1e-9) return {1.0, 0.0, 0.0, dom_z};

    Complex s_eq = std::log(dom_z) / ts_val;
    double wn = std::abs(s_eq);
    if (wn < 1e-12) return {1.0, 0.0, 0.0, dom_z};
    return {-s_eq.real() / wn, wn, std::abs(s_eq.imag()), dom_z};
}

double computeAngleDeficiency(double net_angle_deg) {
    double diff = std::fmod(180.0 - net_angle_deg, 360.0);
    if (diff < 0.0) diff += 360.0;
    if (diff > 180.0) diff -= 360.0;
    return diff;
}

struct TFEvalResult {
    double angle_deg = 0.0;
    double magnitude = 0.0;
    double rl_gain_k = 0.0;
    Complex complex_val = {0.0, 0.0};
    bool at_pole = false;
};

TFEvalResult evaluateTransferFunction(const std::vector<Complex>& poles, const std::vector<Complex>& zeros, Complex test_pt, double gain = 1.0) {
    Complex num_val(gain, 0.0);
    double num_mag = std::abs(gain);
    double zero_ang = (gain >= 0.0) ? 0.0 : PI;
    for (const auto& z : zeros) {
        Complex d = test_pt - z;
        num_val *= d;
        num_mag *= std::abs(d);
        if (std::abs(d) > 0.0) zero_ang += std::arg(d);
    }

    Complex den_val(1.0, 0.0);
    double den_mag = 1.0;
    double pole_ang = 0.0;
    for (const auto& p : poles) {
        Complex d = test_pt - p;
        double md = std::abs(d);
        if (md < 1e-12) {
            return {180.0, std::numeric_limits<double>::infinity(), 0.0, Complex(0.0, 0.0), true};
        }
        den_val *= d;
        den_mag *= md;
        pole_ang += std::arg(d);
    }

    Complex val = num_val / den_val;
    double mag = num_mag / den_mag;
    double k_rl = (mag > 1e-15) ? (1.0 / mag) : std::numeric_limits<double>::infinity();
    double net_deg = (zero_ang - pole_ang) * (180.0 / PI);
    net_deg = std::fmod(net_deg + 180.0, 360.0);
    if (net_deg < 0.0) net_deg += 360.0;
    net_deg -= 180.0;
    return {net_deg, mag, k_rl, val, false};
}

std::vector<std::vector<Complex>> computeRootLocusBranches(const std::vector<Complex>& poles, const std::vector<Complex>& zeros, int num_steps = 200) {
    size_t deg = std::max(poles.size(), zeros.size());
    if (deg == 0) return {};

    Poly den_p = polyFromRoots(poles);
    Poly num_p = polyFromRoots(zeros);
    Poly den_pad(deg + 1, Complex(0.0, 0.0));
    Poly num_pad(deg + 1, Complex(0.0, 0.0));
    for (size_t i = 0; i < den_p.size(); ++i) den_pad[deg + 1 - den_p.size() + i] = den_p[i];
    for (size_t i = 0; i < num_p.size(); ++i) num_pad[deg + 1 - num_p.size() + i] = num_p[i];

    double span = 1.0;
    for (const auto& p : poles) span = std::max(span, std::abs(p));
    for (const auto& z : zeros) span = std::max(span, std::abs(z));
    int p_diff = static_cast<int>(poles.size()) - static_cast<int>(zeros.size());
    double k_center = (p_diff != 0) ? std::pow(span, p_diff) : 1.0;

    std::vector<double> k_vals;
    k_vals.push_back(0.0);
    for (int i = 0; i < num_steps; ++i) {
        double exp_v = -3.3 + 6.6 * double(i) / double(num_steps - 1);
        k_vals.push_back(k_center * std::pow(10.0, exp_v));
    }

    std::vector<std::vector<Complex>> branches(deg);
    std::vector<Complex> prev_roots = (poles.size() == deg) ? poles : std::vector<Complex>{};

    for (double k : k_vals) {
        Poly char_poly = polyAdd(den_pad, polyScale(num_pad, Complex(k, 0.0)));
        auto roots = solvePolynomialRoots(char_poly, prev_roots.empty() ? nullptr : &prev_roots);
        if (roots.size() != deg) continue;

        std::vector<Complex> ordered(deg);
        if (prev_roots.empty() || branches[0].empty()) {
            ordered = roots;
        } else {
            std::vector<bool> used(deg, false);
            for (size_t b = 0; b < deg; ++b) {
                Complex last_pt = branches[b].back();
                double best_d = 1e30;
                size_t best_j = 0;
                for (size_t j = 0; j < deg; ++j) {
                    if (!used[j]) {
                        double d = std::abs(roots[j] - last_pt);
                        if (d < best_d) { best_d = d; best_j = j; }
                    }
                }
                used[best_j] = true;
                ordered[b] = roots[best_j];
            }
        }
        for (size_t b = 0; b < deg; ++b) branches[b].push_back(ordered[b]);
        prev_roots = ordered;
    }
    return branches;
}

// =============================================================================
// Engine 2: Continuous-Time State-Space Simulator (With PID Disable / Bypass)
// =============================================================================
struct SimulationConfig {
    std::vector<Complex> poles = {{-2.0, 2.0}, {-2.0, -2.0}};
    std::vector<Complex> zeros = {{-8.0, 0.0}};
    double plant_gain = 1.0;
    double input_magnitude = 1.0;

    bool pid_enabled = true;   // NEW: When false, C(s) = 1 (Unity Gain / PID Disabled)
    double kp = 1.5;
    double ki = 2.2;
    double kd = 0.05;
    double deriv_filter_n = 20.0;

    bool closed_loop = true;
    std::string signal_type = "Step";
    double sine_freq_rad = 2.0;
    double t_max = 10.0;
    int num_points = 600;
    double cursor_time = 2.5;
    double settle_tol_pct = 2.0;
};

struct SimulationResult {
    std::vector<double> t;
    std::vector<double> r;
    std::vector<Complex> y_primary;
    std::vector<Complex> y_closed;
    std::vector<Complex> y_open;
    std::vector<Complex> u_control;

    std::vector<Complex> cl_poles;
    std::vector<Complex> cl_zeros;
    std::vector<Complex> ol_poles;
    std::vector<Complex> ol_zeros;

    std::string stability_state = "STABLE";
    double max_real_pole = -2.0;

    std::optional<double> zeta;
    std::optional<double> omega_n;
    std::optional<double> omega_d;
    std::optional<double> ol_zeta;
    std::optional<double> ol_omega_n;

    Complex y_steady = {1.0, 0.0};
    double steady_state_error = 0.0;
    double relative_error_frac = 0.0;
    double relative_error_pct = 0.0;

    std::optional<double> rise_time;
    std::optional<double> settling_time;
    std::optional<double> settling_time_5pct;
    std::optional<double> overshoot_pct;
    double peak_time = 0.0;
    double peak_value = 0.0;

    int system_type = 0;
    double kp_pos = 0.0;
    double kv_vel = 0.0;
    double ka_acc = 0.0;
    double dc_gain_cl = 1.0;

    std::optional<double> sigma_d;
    std::optional<double> time_constant_tau;
    std::optional<double> theo_os_pct;
    std::optional<double> theo_tp_sec;
    std::optional<double> theo_ts2_sec;
    std::optional<double> theo_tr_sec;
    std::string damping_regime = "Underdamped";

    double cursor_t = 2.5;
    Complex cursor_y = {1.0, 0.0};
    Complex cursor_u = {1.0, 0.0};
    double cursor_r = 1.0;
    double cursor_abs_err = 0.0;
    double cursor_rel_err_pct = 0.0;
    bool cursor_within_tol = true;
};

class ComplexTransferFunctionSimulator {
public:
    static std::pair<Poly, Poly> buildPlantPolynomials(const std::vector<Complex>& poles, const std::vector<Complex>& zeros, double gain) {
        return {polyScale(polyFromRoots(zeros), Complex(gain, 0.0)), polyFromRoots(poles)};
    }

    static std::pair<Poly, Poly> buildPidPolynomials(double kp, double ki, double kd, double n_filter = 20.0, bool pid_enabled = true) {
        if (!pid_enabled) {
            // PID Disabled -> C(s) = 1 (Unity Gain)
            return {{Complex(1.0, 0.0)}, {Complex(1.0, 0.0)}};
        }
        const double eps = 1e-12;
        bool has_i = std::abs(ki) > eps;
        bool has_d = std::abs(kd) > eps;

        if (!has_i && !has_d) return {{Complex(kp, 0.0)}, {Complex(1.0, 0.0)}};
        if (has_i && !has_d)  return {{Complex(kp, 0.0), Complex(ki, 0.0)}, {Complex(1.0, 0.0), Complex(0.0, 0.0)}};
        double nf = std::max(n_filter, 1.0);
        if (!has_i && has_d)  return {{Complex(kp + kd * nf, 0.0), Complex(kp * nf, 0.0)}, {Complex(1.0, 0.0), Complex(nf, 0.0)}};

        return {
            {Complex(kp + kd * nf, 0.0), Complex(kp * nf + ki, 0.0), Complex(ki * nf, 0.0)},
            {Complex(1.0, 0.0), Complex(nf, 0.0), Complex(0.0, 0.0)}
        };
    }

    static std::vector<Complex> simulateTF(Poly num, Poly den, const std::vector<double>& t, const std::vector<double>& r, const std::string& sig_type, double in_mag) {
        num = polyTrim(num);
        den = polyTrim(den);
        int excess = static_cast<int>(num.size()) - static_cast<int>(den.size());
        double reg_pole = 200.0;
        if (excess > 0) {
            std::vector<Complex> r_poles(excess, Complex(-reg_pole, 0.0));
            den = polyMul(den, polyFromRoots(r_poles));
            num = polyScale(num, Complex(std::pow(reg_pole, excess), 0.0));
        }

        Complex lead = (std::abs(den[0]) > 1e-15) ? den[0] : Complex(1.0, 0.0);
        for (auto& c : den) c /= lead;
        int n = static_cast<int>(den.size()) - 1;

        Poly num_pad(n + 1, Complex(0.0, 0.0));
        for (size_t i = 0; i < num.size(); ++i) num_pad[n + 1 - num.size() + i] = num[i] / lead;

        Complex b0 = num_pad[0];
        size_t n_pts = t.size();
        std::vector<Complex> y(n_pts, Complex(0.0, 0.0));

        if (n == 0) {
            if (sig_type == "Impulse") {
                double dt0 = (n_pts > 1) ? (t[1] - t[0]) : 1.0;
                y[0] = (b0 * in_mag) / dt0;
                return y;
            }
            for (size_t k = 0; k < n_pts; ++k) y[k] = b0 * r[k];
            return y;
        }

        Eigen::MatrixXcd A = Eigen::MatrixXcd::Zero(n, n);
        if (n > 1) A.block(0, 1, n - 1, n - 1).setIdentity();
        for (int i = 0; i < n; ++i) A(n - 1, i) = -den[n - i];

        Eigen::VectorXcd B = Eigen::VectorXcd::Zero(n);
        B(n - 1) = Complex(1.0, 0.0);

        Eigen::RowVectorXcd C(n);
        for (int i = 0; i < n; ++i) {
            Complex a_val = den[n - i];
            Complex b_val = num_pad[n - i];
            C(i) = b_val - b0 * a_val;
        }

        double dt = (n_pts > 1) ? (t[1] - t[0]) : 0.01;
        Eigen::VectorXcd x = Eigen::VectorXcd::Zero(n);
        if (sig_type == "Impulse") {
            x = B * Complex(in_mag, 0.0);
        }

        // 4 sub-steps of RK4 per time step for rock-solid numerical stability
        int sub_steps = 4;
        double h = dt / double(sub_steps);

        for (size_t k = 0; k < n_pts; ++k) {
            double u_cur = (sig_type == "Impulse") ? 0.0 : r[k];
            y[k] = C.dot(x) + b0 * u_cur;
            if (k + 1 < n_pts) {
                double u_next = (sig_type == "Impulse") ? 0.0 : r[k + 1];
                for (int s = 0; s < sub_steps; ++s) {
                    double frac0 = double(s) / double(sub_steps);
                    double fracm = (double(s) + 0.5) / double(sub_steps);
                    double frac1 = double(s + 1) / double(sub_steps);
                    double u0 = u_cur + frac0 * (u_next - u_cur);
                    double um = u_cur + fracm * (u_next - u_cur);
                    double u1 = u_cur + frac1 * (u_next - u_cur);

                    Eigen::VectorXcd k1 = A * x + B * u0;
                    Eigen::VectorXcd k2 = A * (x + 0.5 * h * k1) + B * um;
                    Eigen::VectorXcd k3 = A * (x + 0.5 * h * k2) + B * um;
                    Eigen::VectorXcd k4 = A * (x + h * k3) + B * u1;
                    x += (h / 6.0) * (k1 + 2.0 * k2 + 2.0 * k3 + k4);
                }
                double max_norm = x.lpNorm<Eigen::Infinity>();
                if (!std::isfinite(max_norm)) {
                    for (size_t j = k + 1; j < n_pts; ++j) y[j] = y[k];
                    break;
                }
                if (max_norm > 1e12) x *= (1e12 / max_norm);
            }
        }
        return y;
    }

    static SimulationResult evaluate(const SimulationConfig& cfg) {
        SimulationResult res;
        double t_max = std::max(cfg.t_max, 0.1);
        int n_pts = std::max(cfg.num_points, 200);
        res.t.resize(n_pts);
        res.r.resize(n_pts);
        for (int i = 0; i < n_pts; ++i) {
            res.t[i] = t_max * double(i) / double(n_pts - 1);
            if (cfg.signal_type == "Step") res.r[i] = cfg.input_magnitude;
            else if (cfg.signal_type == "Impulse") res.r[i] = 0.0;
            else if (cfg.signal_type == "Ramp") res.r[i] = cfg.input_magnitude * res.t[i];
            else res.r[i] = cfg.input_magnitude * std::sin(cfg.sine_freq_rad * res.t[i]);
        }

        auto [num_g, den_g] = buildPlantPolynomials(cfg.poles, cfg.zeros, cfg.plant_gain);
        auto [num_c, den_c] = buildPidPolynomials(cfg.kp, cfg.ki, cfg.kd, cfg.deriv_filter_n, cfg.pid_enabled);

        Poly num_l = polyMul(num_c, num_g);
        Poly den_l = polyMul(den_c, den_g);
        Poly num_cl = num_l;
        Poly den_cl = polyAdd(den_l, num_l);
        Poly num_u = polyMul(num_c, den_g);

        res.cl_poles = solvePolynomialRoots(den_cl);
        res.cl_zeros = solvePolynomialRoots(num_cl);
        res.ol_poles = cfg.poles;
        res.ol_zeros = cfg.zeros;

        res.y_closed = simulateTF(num_cl, den_cl, res.t, res.r, cfg.signal_type, cfg.input_magnitude);
        res.y_open   = simulateTF(num_g, den_g, res.t, res.r, cfg.signal_type, cfg.input_magnitude);
        res.u_control = simulateTF(num_u, den_cl, res.t, res.r, cfg.signal_type, cfg.input_magnitude);

        res.y_primary = cfg.closed_loop ? res.y_closed : res.y_open;
        const auto& active_poles = cfg.closed_loop ? res.cl_poles : res.ol_poles;

        auto dom_m = computeDominantSMetrics(active_poles, cfg.deriv_filter_n);
        res.zeta = dom_m.zeta;
        res.omega_n = dom_m.omega_n;
        res.omega_d = dom_m.omega_d;

        auto ol_m = computeDominantSMetrics(res.ol_poles, cfg.deriv_filter_n);
        res.ol_zeta = ol_m.zeta;
        res.ol_omega_n = ol_m.omega_n;

        if (active_poles.empty()) {
            res.max_real_pole = -std::numeric_limits<double>::infinity();
            res.stability_state = "STABLE";
        } else {
            res.max_real_pole = -1e30;
            for (const auto& p : active_poles) res.max_real_pole = std::max(res.max_real_pole, p.real());
            if (res.max_real_pole < -1e-5) res.stability_state = "STABLE";
            else if (res.max_real_pole <= 1e-5) res.stability_state = "MARGINAL";
            else res.stability_state = "UNSTABLE";
        }

        const Poly& act_num = cfg.closed_loop ? num_cl : num_g;
        const Poly& act_den = cfg.closed_loop ? den_cl : den_g;

        if (res.stability_state == "STABLE" && cfg.signal_type == "Step") {
            if (std::abs(act_den.back()) > 1e-12) {
                res.y_steady = Complex(cfg.input_magnitude, 0.0) * (act_num.back() / act_den.back());
            } else {
                res.y_steady = res.y_primary.back();
            }
            res.steady_state_error = std::abs(cfg.input_magnitude - res.y_steady);
        } else {
            res.y_steady = res.y_primary.back();
            res.steady_state_error = std::abs(res.r.back() - res.y_primary.back());
        }

        double abs_mag = std::max(std::abs(cfg.input_magnitude), 1e-12);
        res.relative_error_frac = res.steady_state_error / abs_mag;
        res.relative_error_pct  = res.relative_error_frac * 100.0;

        // Peak, Rise, Settling
        size_t pk_idx = 0;
        for (size_t i = 0; i < res.y_primary.size(); ++i) {
            if (std::abs(res.y_primary[i].real()) > std::abs(res.y_primary[pk_idx].real())) pk_idx = i;
        }
        res.peak_time = res.t[pk_idx];
        res.peak_value = res.y_primary[pk_idx].real();

        double tol_frac = std::max(cfg.settle_tol_pct, 0.05) / 100.0;
        double target_re = res.y_steady.real();

        if (res.stability_state == "STABLE" && cfg.signal_type == "Step" && std::abs(target_re) > 1e-6) {
            int i10 = -1, i90 = -1;
            double max_norm = -1e30;
            size_t pk_step = 0;
            int last_out_2 = -1, last_out_5 = -1;

            for (int i = 0; i < n_pts; ++i) {
                double yn = res.y_primary[i].real() / target_re;
                if (i10 < 0 && yn >= 0.10) i10 = i;
                if (i90 < 0 && yn >= 0.90) i90 = i;
                if (yn > max_norm) { max_norm = yn; pk_step = i; }

                double err_b = std::abs(res.y_primary[i] - res.y_steady) / std::max(std::abs(res.y_steady), 1e-9);
                if (err_b > tol_frac) last_out_2 = i;
                if (err_b > 0.05)     last_out_5 = i;
            }
            if (i10 >= 0 && i90 >= i10) res.rise_time = std::max(0.0, res.t[i90] - res.t[i10]);
            res.overshoot_pct = std::max(0.0, (max_norm - 1.0) * 100.0);
            res.peak_time = res.t[pk_step];
            res.peak_value = res.y_primary[pk_step].real();
            if (last_out_2 < 0) res.settling_time = 0.0;
            else if (last_out_2 < n_pts - 1) res.settling_time = res.t[last_out_2 + 1];
            if (last_out_5 < 0) res.settling_time_5pct = 0.0;
            else if (last_out_5 < n_pts - 1) res.settling_time_5pct = res.t[last_out_5 + 1];
        }

        // System Type & Static Error Constants
        Poly num_loop = polyTrim(cfg.closed_loop ? num_l : num_g);
        Poly den_loop = polyTrim(cfg.closed_loop ? den_l : den_g);
        int sys_t = 0;
        for (auto it = den_loop.rbegin(); it != den_loop.rend(); ++it) {
            if (std::abs(*it) < 1e-10) ++sys_t;
            else break;
        }
        res.system_type = sys_t;
        int nz_idx = static_cast<int>(den_loop.size()) - 1 - sys_t;
        double den_dc = (nz_idx >= 0 && std::abs(den_loop[nz_idx]) > 1e-14) ? den_loop[nz_idx].real() : 1.0;
        double num_dc = num_loop.empty() ? 0.0 : num_loop.back().real();
        double lim_r = num_dc / den_dc;

        if (sys_t == 0) { res.kp_pos = lim_r; res.kv_vel = 0.0; res.ka_acc = 0.0; }
        else if (sys_t == 1) { res.kp_pos = std::numeric_limits<double>::infinity(); res.kv_vel = lim_r; res.ka_acc = 0.0; }
        else { res.kp_pos = std::numeric_limits<double>::infinity(); res.kv_vel = std::numeric_limits<double>::infinity(); res.ka_acc = lim_r; }

        res.dc_gain_cl = (std::abs(act_den.back()) > 1e-12) ? (act_num.back() / act_den.back()).real() : 0.0;

        if (res.zeta.has_value() && res.omega_n.has_value() && res.omega_n.value() > 1e-9) {
            double z = res.zeta.value();
            double wn = res.omega_n.value();
            res.sigma_d = z * wn;
            res.time_constant_tau = (res.sigma_d.value() > 1e-9) ? (1.0 / res.sigma_d.value()) : std::numeric_limits<double>::infinity();
            res.theo_ts2_sec = (res.sigma_d.value() > 1e-9) ? (4.0 / res.sigma_d.value()) : std::numeric_limits<double>::infinity();
            res.theo_os_pct = (active_poles.size() == 1) ? 0.0 : niseOsFromZeta(z);
            res.theo_tp_sec = (res.omega_d.has_value() && res.omega_d.value() > 1e-9) ? (PI / res.omega_d.value()) : std::numeric_limits<double>::infinity();
            res.theo_tr_sec = (active_poles.size() == 1) ? (2.2 / std::max(res.sigma_d.value(), 1e-9)) : niseTrFromZetaWn( std::clamp(z, 0.0, 1.2), wn);
        }

        if (res.stability_state == "UNSTABLE") res.damping_regime = "Unstable (RHP Pole)";
        else if (active_poles.size() == 1) res.damping_regime = "First-Order Exponential";
        else if (res.zeta.has_value()) {
            double z = res.zeta.value();
            if (std::abs(z) < 1e-4) res.damping_regime = "Undamped (ζ = 0)";
            else if (z < 0.995) res.damping_regime = "Underdamped (0 < ζ < 1)";
            else if (std::abs(z - 1.0) <= 0.01) res.damping_regime = "Critically Damped (ζ = 1)";
            else res.damping_regime = "Overdamped (ζ > 1)";
        }

        // Cursor interpolation
        res.cursor_t = std::clamp(cfg.cursor_time, 0.0, t_max);
        int c_idx = std::clamp(static_cast<int>(std::round((res.cursor_t / t_max) * (n_pts - 1))), 0, n_pts - 1);
        res.cursor_y = res.y_primary[c_idx];
        res.cursor_u = res.u_control[c_idx];
        res.cursor_r = res.r[c_idx];
        res.cursor_abs_err = std::abs(res.cursor_r - res.cursor_y);
        res.cursor_rel_err_pct = (res.cursor_abs_err / abs_mag) * 100.0;
        res.cursor_within_tol = (std::abs(res.cursor_y - res.y_steady) / std::max(std::abs(res.y_steady), 1.0)) <= tol_frac;

        return res;
    }
};

// =============================================================================
// Engine 3: Discrete-Time & Digital Filter Simulator (With PID Disable / Bypass)
// =============================================================================
struct DiscreteSimulationConfig {
    std::vector<Complex> z_poles = {{0.68, 0.32}, {0.68, -0.32}};
    std::vector<Complex> z_zeros = {{-1.0, 0.0}, {-1.0, 0.0}};
    double filter_gain = 0.0955;
    double input_magnitude = 1.0;
    double ts = 0.05;

    bool pid_enabled = true;   // NEW: When false in closed-loop, C(z) = 1 (PID Disabled)
    double kp = 1.0;
    double ki = 0.5;
    double kd = 0.02;

    bool closed_loop = false;
    std::string signal_type = "Step";
    double sine_freq_hz = 1.0;
    int num_samples = 80;
    int cursor_k = 15;
    double settle_tol_pct = 2.0;
};

struct DiscreteSimulationResult {
    std::vector<int> k_idx;
    std::vector<double> t_sec;
    std::vector<double> r_seq;
    std::vector<Complex> y_primary;
    std::vector<Complex> y_closed;
    std::vector<Complex> y_open;
    std::vector<Complex> u_control;

    std::vector<Complex> cl_z_poles;
    std::vector<Complex> ol_z_poles;
    std::vector<Complex> ol_z_zeros;

    Poly b_coeffs;
    Poly a_coeffs;
    std::string diff_eq_str;

    std::string stability_state = "STABLE";
    double max_pole_mag = 0.75;

    std::optional<double> zeta;
    std::optional<double> omega_n;
    Complex y_steady = {1.0, 0.0};
    double steady_state_error = 0.0;
    std::optional<int> rise_samples;
    std::optional<int> settling_samples;
    std::optional<double> overshoot_pct;
    int peak_k = 0;
    double peak_value = 1.0;

    int cursor_k = 15;
    double cursor_t = 0.75;
    Complex cursor_y = {1.0, 0.0};
    bool cursor_within_tol = true;
};

class DiscreteFilterSimulator {
public:
    static std::pair<Poly, Poly> toCausalBA(const Poly& num_z, const Poly& den_z) {
        Poly n_trim = polyTrim(num_z);
        Poly d_trim = polyTrim(den_z);
        size_t ord = std::max(n_trim.size(), d_trim.size());
        Poly d_pad = d_trim;
        if (d_pad.size() < ord) d_pad.resize(ord, Complex(0.0, 0.0));
        Poly n_pad(ord, Complex(0.0, 0.0));
        for (size_t i = 0; i < n_trim.size(); ++i) n_pad[ord - n_trim.size() + i] = n_trim[i];

        Complex lead = (std::abs(d_pad[0]) > 1e-15) ? d_pad[0] : Complex(1.0, 0.0);
        for (auto& c : n_pad) c /= lead;
        for (auto& c : d_pad) c /= lead;
        return {n_pad, d_pad};
    }

    static std::string formatDifferenceEquation(const Poly& b, const Poly& a, size_t max_terms = 4) {
        std::ostringstream oss;
        oss << "y[k] = ";
        bool first = true;
        for (size_t m = 0; m < std::min(b.size(), max_terms); ++m) {
            if (std::abs(b[m]) < 1e-7 && m > 0) continue;
            std::string cs = PRECISION.fmt(b[m].real(), !first, true);
            oss << (first ? "" : " ") << cs << (m == 0 ? "·x[k]" : ("·x[k-" + std::to_string(m) + "]"));
            first = false;
        }
        for (size_t n = 1; n < std::min(a.size(), max_terms + 1); ++n) {
            double av = -a[n].real();
            if (std::abs(av) < 1e-7) continue;
            oss << " " << PRECISION.fmt(av, true, true) << "·y[k-" << n << "]";
        }
        return oss.str();
    }

    static std::pair<Poly, Poly> buildDiscretePid(double kp, double ki, double kd, double ts, bool pid_enabled = true) {
        if (!pid_enabled) {
            // Discrete PID Disabled -> C(z) = 1
            return {{Complex(1.0, 0.0)}, {Complex(1.0, 0.0)}};
        }
        double t = std::max(ts, 1e-5);
        bool has_i = std::abs(ki) > 1e-12;
        bool has_d = std::abs(kd) > 1e-12;
        if (!has_i && !has_d) return {{Complex(kp, 0.0)}, {Complex(1.0, 0.0)}};
        if (has_i && !has_d)  return {{Complex(kp + ki * t, 0.0), Complex(-kp, 0.0)}, {Complex(1.0, 0.0), Complex(-1.0, 0.0)}};
        if (!has_i && has_d) {
            double kdt = kd / t;
            return {{Complex(kp + kdt, 0.0), Complex(-kdt, 0.0)}, {Complex(1.0, 0.0), Complex(0.0, 0.0)}};
        }
        double kdt = kd / t, kit = ki * t;
        return {
            {Complex(kp + kit + kdt, 0.0), Complex(-(kp + 2.0 * kdt), 0.0), Complex(kdt, 0.0)},
            {Complex(1.0, 0.0), Complex(-1.0, 0.0), Complex(0.0, 0.0)}
        };
    }

    static std::vector<Complex> simulateDiffEq(const Poly& num_z, const Poly& den_z, const std::vector<double>& x_seq) {
        auto [b, a] = toCausalBA(num_z, den_z);
        size_t n_pts = x_seq.size();
        std::vector<Complex> y(n_pts, Complex(0.0, 0.0));
        for (size_t k = 0; k < n_pts; ++k) {
            Complex acc(0.0, 0.0);
            for (size_t m = 0; m < std::min(b.size(), k + 1); ++m) acc += b[m] * x_seq[k - m];
            for (size_t n = 1; n < std::min(a.size(), k + 1); ++n) acc -= a[n] * y[k - n];
            if (std::abs(acc) > 1e12) acc *= (1e12 / std::abs(acc));
            y[k] = acc;
        }
        return y;
    }

    static std::tuple<std::vector<Complex>, std::vector<Complex>, double> designFilterPreset(
        const std::string& key, double cutoff_norm, double ts,
        const std::vector<Complex>& s_poles = {}, const std::vector<Complex>& s_zeros = {}, double s_gain = 1.0)
    {
        double wc = std::clamp(cutoff_norm, 0.02, 0.95);
        double t = std::max(ts, 1e-5);

        if (key == "butter_lp" || key == "butter_hp") {
            double wa = (2.0 / t) * std::tan(PI * wc / 2.0);
            auto sp = polesFromZetaWn(std::sqrt(0.5), wa);
            std::vector<Complex> zp;
            for (const auto& p : sp) zp.push_back((1.0 + p * t / 2.0) / (1.0 - p * t / 2.0));
            std::vector<Complex> zz = (key == "butter_lp") ? std::vector<Complex>{{-1,0},{-1,0}} : std::vector<Complex>{{1,0},{1,0}};
            Complex ref_z = (key == "butter_lp") ? Complex(1.0, 0.0) : Complex(-1.0, 0.0);
            auto ev = evaluateTransferFunction(zp, zz, ref_z, 1.0);
            double kd = (ev.magnitude > 1e-12) ? (1.0 / ev.magnitude) : 1.0;
            return {zp, zz, kd};
        }
        if (key == "biquad_bp") {
            double th = PI * wc;
            std::vector<Complex> zp = {std::polar(0.88, th), std::polar(0.88, -th)};
            std::vector<Complex> zz = {{1.0, 0.0}, {-1.0, 0.0}};
            auto ev = evaluateTransferFunction(zp, zz, std::polar(1.0, th), 1.0);
            return {zp, zz, (ev.magnitude > 1e-12) ? (1.0 / ev.magnitude) : 1.0};
        }
        if (key == "biquad_notch") {
            double th = PI * wc;
            std::vector<Complex> zz = {std::polar(1.0, th), std::polar(1.0, -th)};
            std::vector<Complex> zp = {std::polar(0.90, th), std::polar(0.90, -th)};
            auto ev = evaluateTransferFunction(zp, zz, Complex(1.0, 0.0), 1.0);
            return {zp, zz, (ev.magnitude > 1e-12) ? (1.0 / ev.magnitude) : 1.0};
        }
        if (key == "fir_ma4") {
            return {{{0,0},{0,0},{0,0}}, {{-1,0},{0,1},{0,-1}}, 0.25};
        }
        if (key == "tustin_from_s" && !s_poles.empty()) {
            std::vector<Complex> zp, zz;
            for (const auto& p : s_poles) zp.push_back((1.0 + p * t / 2.0) / (1.0 - p * t / 2.0));
            for (const auto& z : s_zeros) zz.push_back((1.0 + z * t / 2.0) / (1.0 - z * t / 2.0));
            while (zz.size() < zp.size()) zz.push_back({-1.0, 0.0});
            auto ev_s = evaluateTransferFunction(s_poles, s_zeros, Complex(0.0, 0.1), s_gain);
            Complex z_ref = (1.0 + Complex(0.0, 0.1) * t / 2.0) / (1.0 - Complex(0.0, 0.1) * t / 2.0);
            auto ev_z = evaluateTransferFunction(zp, zz, z_ref, 1.0);
            return {zp, zz, (ev_z.magnitude > 1e-12) ? (ev_s.magnitude / ev_z.magnitude) : s_gain};
        }
        if (key == "matched_from_s" && !s_poles.empty()) {
            std::vector<Complex> zp, zz;
            for (const auto& p : s_poles) zp.push_back(std::exp(p * t));
            for (const auto& z : s_zeros) zz.push_back(std::exp(z * t));
            auto ev_s = evaluateTransferFunction(s_poles, s_zeros, Complex(0.0, 0.1), s_gain);
            auto ev_z = evaluateTransferFunction(zp, zz, std::exp(Complex(0.0, 0.1) * t), 1.0);
            return {zp, zz, (ev_z.magnitude > 1e-12) ? (ev_s.magnitude / ev_z.magnitude) : s_gain};
        }
        return {{{0.68, 0.32}, {0.68, -0.32}}, {{-1.0, 0.0}, {-1.0, 0.0}}, 0.0955};
    }

    static DiscreteSimulationResult evaluate(const DiscreteSimulationConfig& cfg) {
        DiscreteSimulationResult res;
        int n_pts = std::max(cfg.num_samples, 10);
        double ts = std::max(cfg.ts, 1e-5);
        res.k_idx.resize(n_pts);
        res.t_sec.resize(n_pts);
        res.r_seq.resize(n_pts);

        for (int k = 0; k < n_pts; ++k) {
            res.k_idx[k] = k;
            res.t_sec[k] = k * ts;
            if (cfg.signal_type == "Step") res.r_seq[k] = cfg.input_magnitude;
            else if (cfg.signal_type == "Impulse") res.r_seq[k] = (k == 0) ? cfg.input_magnitude : 0.0;
            else if (cfg.signal_type == "Ramp") res.r_seq[k] = cfg.input_magnitude * res.t_sec[k];
            else res.r_seq[k] = cfg.input_magnitude * std::sin(2.0 * PI * cfg.sine_freq_hz * res.t_sec[k]);
        }

        Poly num_h = polyScale(polyFromRoots(cfg.z_zeros), Complex(cfg.filter_gain, 0.0));
        Poly den_h = polyFromRoots(cfg.z_poles);
        auto [b_h, a_h] = toCausalBA(num_h, den_h);

        auto [num_c, den_c] = buildDiscretePid(cfg.kp, cfg.ki, cfg.kd, ts, cfg.pid_enabled);
        Poly num_l = polyMul(num_c, b_h);
        Poly den_l = polyMul(den_c, a_h);
        Poly num_cl = num_l;
        Poly den_cl = polyAdd(den_l, num_l);

        res.cl_z_poles = solvePolynomialRoots(den_cl);
        res.ol_z_poles = cfg.z_poles;
        res.ol_z_zeros = cfg.z_zeros;

        res.y_open   = simulateDiffEq(b_h, a_h, res.r_seq);
        res.y_closed = simulateDiffEq(num_cl, den_cl, res.r_seq);
        res.y_primary = cfg.closed_loop ? res.y_closed : res.y_open;

        const Poly& act_num = cfg.closed_loop ? num_cl : b_h;
        const Poly& act_den = cfg.closed_loop ? den_cl : a_h;
        std::tie(res.b_coeffs, res.a_coeffs) = toCausalBA(act_num, act_den);
        res.diff_eq_str = formatDifferenceEquation(res.b_coeffs, res.a_coeffs);

        const auto& act_poles = cfg.closed_loop ? res.cl_z_poles : res.ol_z_poles;
        res.max_pole_mag = 0.0;
        for (const auto& p : act_poles) res.max_pole_mag = std::max(res.max_pole_mag, std::abs(p));
        if (res.max_pole_mag < 1.0 - 1e-5) res.stability_state = "STABLE";
        else if (res.max_pole_mag <= 1.0 + 1e-5) res.stability_state = "MARGINAL";
        else res.stability_state = "UNSTABLE";

        auto zm = computeDominantZMetrics(act_poles, ts);
        res.zeta = zm.zeta;
        res.omega_n = zm.omega_n;

        Complex den_1 = polyVal(act_den, Complex(1.0, 0.0));
        Complex num_1 = polyVal(act_num, Complex(1.0, 0.0));
        if (res.stability_state == "STABLE" && cfg.signal_type == "Step" && std::abs(den_1) > 1e-12) {
            res.y_steady = Complex(cfg.input_magnitude, 0.0) * (num_1 / den_1);
            res.steady_state_error = std::abs(cfg.input_magnitude - res.y_steady);
        } else {
            res.y_steady = res.y_primary.back();
            res.steady_state_error = std::abs(res.r_seq.back() - res.y_primary.back());
        }

        double target_re = res.y_steady.real();
        double tol_frac = std::max(cfg.settle_tol_pct, 0.05) / 100.0;
        if (res.stability_state == "STABLE" && cfg.signal_type == "Step" && std::abs(target_re) > 1e-6) {
            int i10 = -1, i90 = -1, last_out = -1;
            double max_n = -1e30;
            for (int k = 0; k < n_pts; ++k) {
                double yn = res.y_primary[k].real() / target_re;
                if (i10 < 0 && yn >= 0.10) i10 = k;
                if (i90 < 0 && yn >= 0.90) i90 = k;
                if (yn > max_n) { max_n = yn; res.peak_k = k; res.peak_value = res.y_primary[k].real(); }
                if (std::abs(res.y_primary[k] - res.y_steady) / std::max(std::abs(res.y_steady), 1e-9) > tol_frac) last_out = k;
            }
            if (i10 >= 0 && i90 >= i10) res.rise_samples = i90 - i10;
            if (last_out < 0) res.settling_samples = 0;
            else if (last_out < n_pts - 1) res.settling_samples = last_out + 1;
            res.overshoot_pct = std::max(0.0, (max_n - 1.0) * 100.0);
        }

        res.cursor_k = std::clamp(cfg.cursor_k, 0, n_pts - 1);
        res.cursor_t = res.cursor_k * ts;
        res.cursor_y = res.y_primary[res.cursor_k];
        res.cursor_within_tol = (std::abs(res.cursor_y - res.y_steady) / std::max(std::abs(res.y_steady), 1.0)) <= tol_frac;
        return res;
    }
};

// =============================================================================
// Shared System Model (Synchronizes s-Domain & z-Domain State Across All Tabs)
// =============================================================================
class SharedSystemModel {
public:
    std::vector<Complex> poles = {{-2.0, 2.0}, {-2.0, -2.0}};
    std::vector<Complex> zeros = {{-8.0, 0.0}};
    double plant_gain = 1.0;
    double input_magnitude = 1.0;
    Complex test_point = {-2.0, 2.0};
    double rl_tol_deg = 0.5;

    bool pid_enabled = true;
    double kp = 1.5;
    double ki = 2.2;
    double kd = 0.05;
    double deriv_filter_n = 20.0;

    std::vector<Complex> z_poles;
    std::vector<Complex> z_zeros;
    double filter_gain = 0.0955;
    double discrete_input_magnitude = 1.0;
    double ts = 0.05;
    Complex z_test_point = {0.68, 0.32};
    double z_rl_tol_deg = 0.5;

    bool d_pid_enabled = true;
    double d_kp = 1.0;
    double d_ki = 0.5;
    double d_kd = 0.02;

    std::optional<SimulationResult> last_sim_result;
    std::optional<DiscreteSimulationResult> last_discrete_result;
    bool sync_enabled = true;

    std::vector<std::function<void(const std::string&)>> listeners;

    SharedSystemModel() {
        std::tie(z_poles, z_zeros, filter_gain) = DiscreteFilterSimulator::designFilterPreset("butter_lp", 0.25, 0.05);
        if (!z_poles.empty()) z_test_point = z_poles[0];
    }

    void notify(const std::string& source_tab) {
        if (!sync_enabled) return;
        for (auto& cb : listeners) cb(source_tab);
    }
};

// =============================================================================
// Engine 4: Desmos-Style Scientific Calculator Engine
// =============================================================================
struct CalcEvalOutput {
    bool ok = true;
    Complex val = {0.0, 0.0};
    std::string formatted;
    std::string error_msg;
};

class ScientificCalcEngine {
public:
    bool deg_mode = true;
    std::map<std::string, Complex> vars;
    SharedSystemModel* model = nullptr;

    ScientificCalcEngine(SharedSystemModel* m = nullptr) : model(m) {
        resetConstants();
    }

    void resetConstants() {
        vars["pi"]  = Complex(PI, 0.0);
        vars["e"]   = Complex(std::exp(1.0), 0.0);
        vars["tau"] = Complex(2.0 * PI, 0.0);
        vars["phi"] = Complex((1.0 + std::sqrt(5.0)) / 2.0, 0.0);
        vars["i"]   = Complex(0.0, 1.0);
        vars["j"]   = Complex(0.0, 1.0);
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

    CalcEvalOutput evalLine(const std::string& raw_line) {
        syncFromModel();
        std::string s = raw_line;
        // Preprocess LaTeX shortcuts (\zeta, \omega_n, \pi, \frac{a}{b}, \sqrt{x})
        s = std::regex_replace(s, std::regex(R"(\\zeta)"), "zeta");
        s = std::regex_replace(s, std::regex(R"(\\omega_n)"), "wn");
        s = std::regex_replace(s, std::regex(R"(\\omega_d)"), "wd");
        s = std::regex_replace(s, std::regex(R"(\\sigma_d)"), "sigma_d");
        s = std::regex_replace(s, std::regex(R"(\\pi)"), "pi");
        s = std::regex_replace(s, std::regex(R"(\\sqrt\{([^{}]+)\})"), "sqrt($1)");
        s = std::regex_replace(s, std::regex(R"(\\frac\{([^{}]+)\}\{([^{}]+)\})"), "(($1)/($2))");
        s = std::regex_replace(s, std::regex(R"(\\ln)"), "ln");

        // Check variable assignment: var = expr
        std::string assigned_name;
        auto eq_pos = s.find('=');
        if (eq_pos != std::string::npos) {
            std::string lhs = s.substr(0, eq_pos);
            lhs.erase(std::remove_if(lhs.begin(), lhs.end(), ::isspace), lhs.end());
            if (!lhs.empty() && std::isalpha(static_cast<unsigned char>(lhs[0]))) {
                assigned_name = lhs;
                s = s.substr(eq_pos + 1);
            }
        }

        try {
            Pos = 0;
            Src = s;
            Complex v = parseE();
            vars["ans"] = v;
            if (!assigned_name.empty()) vars[assigned_name] = v;
            std::string prefix = assigned_name.empty() ? "= " : (assigned_name + " = ");
            return {true, v, prefix + PRECISION.fmtComplex(v), ""};
        } catch (const std::exception& e) {
            return {false, {0,0}, "", e.what()};
        }
    }

private:
    std::string Src;
    size_t Pos = 0;

    void skip() { while (Pos < Src.size() && std::isspace(static_cast<unsigned char>(Src[Pos]))) ++Pos; }

    Complex parseE() {
        Complex l = parseT();
        while (true) {
            skip();
            if (Pos < Src.size() && (Src[Pos] == '+' || Src[Pos] == '-')) {
                char op = Src[Pos++];
                Complex r = parseT();
                l = (op == '+') ? (l + r) : (l - r);
            } else break;
        }
        return l;
    }

    Complex parseT() {
        Complex l = parseP();
        while (true) {
            skip();
            if (Pos < Src.size() && (Src[Pos] == '*' || Src[Pos] == '/')) {
                char op = Src[Pos++];
                Complex r = parseP();
                if (op == '/' && std::abs(r) < 1e-15) throw std::runtime_error("Division by zero");
                l = (op == '*') ? (l * r) : (l / r);
            } else break;
        }
        return l;
    }

    Complex parseP() {
        Complex b = parseU();
        skip();
        if (Pos < Src.size() && Src[Pos] == '^') {
            ++Pos;
            Complex e = parseU();
            return std::pow(b, e);
        }
        return b;
    }

    Complex parseU() {
        skip();
        if (Pos < Src.size() && Src[Pos] == '+') { ++Pos; return parseU(); }
        if (Pos < Src.size() && Src[Pos] == '-') { ++Pos; return -parseU(); }
        return parseAtom();
    }

    std::vector<Complex> parseArgList() {
        std::vector<Complex> args;
        skip();
        if (Pos < Src.size() && Src[Pos] == ')') { ++Pos; return args; }
        while (true) {
            args.push_back(parseE());
            skip();
            if (Pos < Src.size() && Src[Pos] == ',') { ++Pos; continue; }
            if (Pos < Src.size() && Src[Pos] == ')') { ++Pos; break; }
            throw std::runtime_error("Expected ',' or ')' in function call");
        }
        return args;
    }

    Complex parseAtom() {
        skip();
        if (Pos >= Src.size()) throw std::runtime_error("Incomplete expression");
        if (Src[Pos] == '(' || Src[Pos] == '{') {
            char close_ch = (Src[Pos] == '(') ? ')' : '}';
            ++Pos;
            Complex v = parseE();
            skip();
            if (Pos < Src.size() && Src[Pos] == close_ch) ++Pos;
            return v;
        }
        if (std::isdigit(static_cast<unsigned char>(Src[Pos])) || Src[Pos] == '.') {
            size_t st = Pos;
            while (Pos < Src.size() && (std::isdigit(static_cast<unsigned char>(Src[Pos])) || Src[Pos] == '.')) ++Pos;
            double v = std::stod(Src.substr(st, Pos - st));
            if (Pos < Src.size() && (Src[Pos] == 'i' || Src[Pos] == 'j')) { ++Pos; return Complex(0.0, v); }
            return Complex(v, 0.0);
        }
        if (std::isalpha(static_cast<unsigned char>(Src[Pos])) || Src[Pos] == '_') {
            size_t st = Pos;
            while (Pos < Src.size() && (std::isalnum(static_cast<unsigned char>(Src[Pos])) || Src[Pos] == '_')) ++Pos;
            std::string name = Src.substr(st, Pos - st);
            skip();
            if (Pos < Src.size() && Src[Pos] == '(') {
                ++Pos;
                auto args = parseArgList();
                return callFunc(name, args);
            }
            if (vars.count(name)) return vars[name];
            throw std::runtime_error("Undefined symbol '" + name + "'");
        }
        throw std::runtime_error(std::string("Unexpected token '") + Src[Pos] + "'");
    }

    Complex callFunc(const std::string& fn, const std::vector<Complex>& a) {
        if (a.empty()) throw std::runtime_error(fn + "() requires arguments");
        double x = a[0].real();
        double to_rad = deg_mode ? (PI / 180.0) : 1.0;
        double from_rad = deg_mode ? (180.0 / PI) : 1.0;

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

        // Nise & Formula-Sheet Predefined Solver Functions
        if (fn == "solve_zeta_from_os" || fn == "zeta_from_os") return Complex(niseZetaFromOs(x), 0.0);
        if (fn == "solve_os" || fn == "os_from_zeta") return Complex(niseOsFromZeta(x), 0.0);
        if (fn == "solve_Ts" && a.size() >= 2) return Complex(4.0 / std::max(x * a[1].real(), 1e-9), 0.0);
        if (fn == "solve_Tp" && a.size() >= 2) {
            double wd = a[1].real() * std::sqrt(std::max(1e-9, 1.0 - x * x));
            return Complex(PI / wd, 0.0);
        }
        if (fn == "solve_Tr" && a.size() >= 2) return Complex(niseTrFromZetaWn(x, a[1].real()), 0.0);
        if (fn == "solve_wn_from_Ts_zeta" && a.size() >= 2) return Complex(4.0 / std::max(x * a[1].real(), 1e-9), 0.0);
        if (fn == "solve_wn_from_Tp_zeta" && a.size() >= 2) {
            double z = a[1].real();
            return Complex(PI / (std::max(x, 1e-9) * std::sqrt(std::max(1e-9, 1.0 - z * z))), 0.0);
        }
        if (fn == "solve_wd" && a.size() >= 2) return Complex(a[1].real() * std::sqrt(std::max(0.0, 1.0 - x * x)), 0.0);
        if (fn == "solve_sigma_d" && a.size() >= 2) return Complex(x * a[1].real(), 0.0);
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
        if (fn == "G" && model) {
            return evaluateTransferFunction(model->poles, model->zeros, a[0], model->plant_gain).complex_val;
        }
        if (fn == "K_rl" && model) {
            return Complex(evaluateTransferFunction(model->poles, model->zeros, a[0], model->plant_gain).rl_gain_k, 0.0);
        }
        if (fn == "H_z" && model) {
            return evaluateTransferFunction(model->z_poles, model->z_zeros, a[0], model->filter_gain).complex_val;
        }
        throw std::runtime_error("Unknown function '" + fn + "'");
    }
};

// =============================================================================
// Custom Interactive QPainter Canvases (Complex Plane & Time-Domain Plotter)
// =============================================================================
class ComplexPlaneCanvas : public QWidget {
    Q_OBJECT
public:
    bool is_z_plane = false;
    bool show_locus = true;
    bool show_asymp = true;
    bool show_unit_circle = false;
    bool show_zgrid = false;

    std::vector<Complex> poles;
    std::vector<Complex> zeros;
    Complex test_point = {-2.0, 2.0};
    bool is_on_rl = true;

    std::function<void(Complex)> onPointSelected;

    explicit ComplexPlaneCanvas(bool z_mode = false, QWidget* parent = nullptr)
        : QWidget(parent), is_z_plane(z_mode), show_unit_circle(z_mode), show_zgrid(z_mode)
    {
        setMinimumSize(420, 380);
        setCursor(Qt::CrossCursor);
        resetView();
    }

    void resetView() {
        user_bounds_set = false;
        update();
    }

protected:
    bool user_bounds_set = false;
    double min_r = -5.0, max_r = 1.0, min_i = -3.0, max_i = 3.0;
    QPoint pan_start_px;
    bool panning = false;

    void computeAutoBounds() {
        if (is_z_plane) {
            min_r = -1.55; max_r = 1.55; min_i = -1.55; max_i = 1.55;
            return;
        }
        std::vector<Complex> pts = poles;
        pts.insert(pts.end(), zeros.begin(), zeros.end());
        pts.push_back(test_point);
        pts.push_back({0.0, 0.0});
        double r0 = 0.0, r1 = 0.0, i0 = 0.0, i1 = 0.0;
        for (const auto& p : pts) {
            r0 = std::min(r0, p.real()); r1 = std::max(r1, p.real());
            i0 = std::min(i0, p.imag()); i1 = std::max(i1, p.imag());
        }
        double span = std::max({r1 - r0, i1 - i0, 2.5}) * 0.72;
        double mr = 0.5 * (r0 + r1), mi = 0.5 * (i0 + i1);
        min_r = mr - span; max_r = mr + span;
        min_i = mi - span; max_i = mi + span;
    }

    QPointF toPx(Complex c) const {
        double pad = 24.0;
        double ew = std::max(1.0, width() - 2.0 * pad);
        double eh = std::max(1.0, height() - 2.0 * pad);
        double x = (c.real() - min_r) / (max_r - min_r) * ew + pad;
        double y = (max_i - c.imag()) / (max_i - min_i) * eh + pad;
        return QPointF(x, y);
    }

    Complex pxToComplex(QPointF px) const {
        double pad = 24.0;
        double ew = std::max(1.0, width() - 2.0 * pad);
        double eh = std::max(1.0, height() - 2.0 * pad);
        double re = min_r + (px.x() - pad) / ew * (max_r - min_r);
        double im = max_i - (px.y() - pad) / eh * (max_i - min_i);
        return Complex(re, im);
    }

    void paintEvent(QPaintEvent*) override {
        if (!user_bounds_set) computeAutoBounds();

        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.fillRect(rect(), QColor(Theme::BG_PLOT));

        // Grid
        double span = std::max(max_r - min_r, max_i - min_i);
        double step = std::pow(10.0, std::floor(std::log10(std::max(span / 6.0, 1e-4))));
        if (span / step > 12.0) step *= 2.0;

        p.setPen(QPen(QColor(Theme::GRID_MAJOR), 1.0));
        for (double rv = std::floor(min_r / step) * step; rv <= max_r; rv += step) {
            double x = toPx({rv, 0.0}).x();
            p.drawLine(QPointF(x, 0), QPointF(x, height()));
        }
        for (double iv = std::floor(min_i / step) * step; iv <= max_i; iv += step) {
            double y = toPx({0.0, iv}).y();
            p.drawLine(QPointF(0, y), QPointF(width(), y));
        }

        // Axes
        QPointF orig = toPx({0.0, 0.0});
        p.setPen(QPen(QColor(Theme::AXIS_LINE), 1.5));
        p.drawLine(QPointF(0, orig.y()), QPointF(width(), orig.y()));
        p.drawLine(QPointF(orig.x(), 0), QPointF(orig.x(), height()));

        // Unit Circle for z-Plane
        if (show_unit_circle) {
            QPointF p_tl = toPx({-1.0, 1.0});
            QPointF p_br = toPx({1.0, -1.0});
            QPen uc_pen(QColor(Theme::ACCENT_ASYMP), 1.8, Qt::DashLine);
            p.setPen(uc_pen);
            p.setBrush(Qt::NoBrush);
            p.drawEllipse(QRectF(p_tl, p_br));
        }

        // Asymptotes
        if (show_asymp && poles.size() != zeros.size()) {
            int diff = static_cast<int>(poles.size()) - static_cast<int>(zeros.size());
            double sum_p = 0.0, sum_z = 0.0;
            for (const auto& pt : poles) sum_p += pt.real();
            for (const auto& zt : zeros) sum_z += zt.real();
            double sigma_a = (sum_p - sum_z) / double(diff);
            QPointF c_px = toPx({sigma_a, 0.0});
            double rad = span * 3.0;
            p.setPen(QPen(QColor(Theme::ACCENT_ASYMP), 1.3, Qt::DashLine));
            for (int k = 0; k < std::abs(diff); ++k) {
                double ang = (2.0 * k + 1.0) * PI / std::abs(diff);
                QPointF far_px = toPx({sigma_a + rad * std::cos(ang), rad * std::sin(ang)});
                p.drawLine(c_px, far_px);
            }
        }

        // Root Locus Branches
        if (show_locus) {
            auto branches = computeRootLocusBranches(poles, zeros, 160);
            p.setPen(QPen(QColor(Theme::ACCENT_LOCUS), 2.2));
            double max_jump = std::max(width(), height()) * 0.45;
            for (const auto& br : branches) {
                QPainterPath path;
                bool started = false;
                QPointF prev_px;
                for (const auto& pt : br) {
                    QPointF cur_px = toPx(pt);
                    if (!started) {
                        path.moveTo(cur_px);
                        started = true;
                    } else {
                        if (std::hypot(cur_px.x() - prev_px.x(), cur_px.y() - prev_px.y()) > max_jump) {
                            p.drawPath(path);
                            path = QPainterPath();
                            path.moveTo(cur_px);
                        } else {
                            path.lineTo(cur_px);
                        }
                    }
                    prev_px = cur_px;
                }
                p.drawPath(path);
            }
        }

        // Vector rays to Test Point
        QPointF tp_px = toPx(test_point);
        p.setPen(QPen(QColor("#1e40af"), 1.1, Qt::DotLine));
        for (const auto& z : zeros) p.drawLine(toPx(z), tp_px);
        p.setPen(QPen(QColor("#881337"), 1.1, Qt::DotLine));
        for (const auto& pl : poles) p.drawLine(toPx(pl), tp_px);

        // Zeros (o)
        p.setPen(QPen(QColor(Theme::ACCENT_ZERO), 2.2));
        p.setBrush(QColor(Theme::BG_PLOT));
        for (const auto& z : zeros) {
            p.drawEllipse(toPx(z), 6.0, 6.0);
        }

        // Poles (x)
        p.setPen(QPen(QColor(Theme::ACCENT_POLE), 2.2));
        for (const auto& pl : poles) {
            QPointF px = toPx(pl);
            p.drawLine(px + QPointF(-6, -6), px + QPointF(6, 6));
            p.drawLine(px + QPointF(-6, 6), px + QPointF(6, -6));
        }

        // Test Point
        QColor tp_col(is_on_rl ? Theme::ACCENT_PID : Theme::ACCENT_OPEN);
        p.setPen(QPen(Qt::white, 1.5));
        p.setBrush(tp_col);
        p.drawEllipse(tp_px, 5.5, 5.5);

        p.setPen(tp_col);
        p.setFont(QFont("Monospace", 9, QFont::Bold));
        QString tag = QString("%1 (%2) [%3]")
            .arg(is_z_plane ? "z" : "s")
            .arg(QString::fromStdString(PRECISION.fmtComplex(test_point)))
            .arg(is_on_rl ? "ON RL" : "OFF RL");
        p.drawText(tp_px + QPointF(10, -8), tag);
    }

    void mousePressEvent(QMouseEvent* e) override {
        if (e->button() == Qt::LeftButton) {
            user_bounds_set = true;
            test_point = pxToComplex(e->pos());
            if (onPointSelected) onPointSelected(test_point);
            update();
        } else if (e->button() == Qt::RightButton || e->button() == Qt::MiddleButton) {
            user_bounds_set = true;
            panning = true;
            pan_start_px = e->pos();
            setCursor(Qt::ClosedHandCursor);
        }
    }

    void mouseMoveEvent(QMouseEvent* e) override {
        if (e->buttons() & Qt::LeftButton) {
            test_point = pxToComplex(e->pos());
            if (onPointSelected) onPointSelected(test_point);
            update();
        } else if (panning) {
            double ew = std::max(1, width() - 48);
            double eh = std::max(1, height() - 48);
            double dr = -double(e->pos().x() - pan_start_px.x()) / ew * (max_r - min_r);
            double di =  double(e->pos().y() - pan_start_px.y()) / eh * (max_i - min_i);
            min_r += dr; max_r += dr;
            min_i += di; max_i += di;
            pan_start_px = e->pos();
            update();
        }
    }

    void mouseReleaseEvent(QMouseEvent*) override {
        panning = false;
        setCursor(Qt::CrossCursor);
    }

    void wheelEvent(QWheelEvent* e) override {
        user_bounds_set = true;
        double factor = (e->angleDelta().y() > 0) ? 0.82 : 1.22;
        Complex anchor = pxToComplex(e->position());
        min_r = anchor.real() - (anchor.real() - min_r) * factor;
        max_r = anchor.real() + (max_r - anchor.real()) * factor;
        min_i = anchor.imag() - (anchor.imag() - min_i) * factor;
        max_i = anchor.imag() + (max_i - anchor.imag()) * factor;
        update();
    }
};

class TimeResponseCanvas : public QWidget {
    Q_OBJECT
public:
    bool is_discrete = false;
    bool show_cl = true;
    bool show_ol = false;
    bool show_bounds = true;
    bool show_u = false;
    bool show_stems = true;

    std::optional<SimulationResult> sim_res;
    std::optional<DiscreteSimulationResult> dsim_res;
    double settle_tol_pct = 2.0;

    std::function<void(double)> onCursorChanged;

    explicit TimeResponseCanvas(bool discrete_mode = false, QWidget* parent = nullptr)
        : QWidget(parent), is_discrete(discrete_mode)
    {
        setMinimumSize(460, 380);
        setCursor(Qt::CrossCursor);
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.fillRect(rect(), QColor(Theme::BG_PLOT));

        double pad_l = 52.0, pad_r = 22.0, pad_t = 22.0, pad_b = 36.0;
        double ew = std::max(10.0, width() - pad_l - pad_r);
        double eh = std::max(10.0, height() - pad_t - pad_b);

        if (!is_discrete && !sim_res.has_value()) return;
        if (is_discrete && !dsim_res.has_value()) return;

        double x_min = 0.0, x_max = 10.0;
        double y_min = -0.2, y_max = 1.5;

        if (!is_discrete) {
            const auto& r = sim_res.value();
            x_max = r.t.empty() ? 10.0 : r.t.back();
            double ymin_v = 0.0, ymax_v = 1.0;
            for (size_t i = 0; i < r.t.size(); ++i) {
                double vy = r.y_primary[i].real();
                if (std::isfinite(vy)) { ymin_v = std::min(ymin_v, vy); ymax_v = std::max(ymax_v, vy); }
                ymin_v = std::min(ymin_v, r.r[i]); ymax_v = std::max(ymax_v, r.r[i]);
            }
            double span = std::max(ymax_v - ymin_v, 0.5);
            y_min = ymin_v - 0.12 * span;
            y_max = ymax_v + 0.18 * span;
        } else {
            const auto& r = dsim_res.value();
            x_min = -0.5;
            x_max = r.k_idx.empty() ? 50.0 : (r.k_idx.back() + 0.5);
            double ymin_v = 0.0, ymax_v = 1.0;
            for (size_t i = 0; i < r.k_idx.size(); ++i) {
                double vy = r.y_primary[i].real();
                if (std::isfinite(vy)) { ymin_v = std::min(ymin_v, vy); ymax_v = std::max(ymax_v, vy); }
                ymin_v = std::min(ymin_v, r.r_seq[i]); ymax_v = std::max(ymax_v, r.r_seq[i]);
            }
            double span = std::max(ymax_v - ymin_v, 0.5);
            y_min = ymin_v - 0.14 * span;
            y_max = ymax_v + 0.20 * span;
        }

        auto toPx = [&](double x, double y) {
            double px = pad_l + (x - x_min) / std::max(x_max - x_min, 1e-9) * ew;
            double py = pad_t + (y_max - y) / std::max(y_max - y_min, 1e-9) * eh;
            return QPointF(px, py);
        };

        // Gridlines
        p.setPen(QPen(QColor(Theme::GRID_MAJOR), 1.0));
        for (int i = 0; i <= 6; ++i) {
            double gx = pad_l + ew * double(i) / 6.0;
            p.drawLine(QPointF(gx, pad_t), QPointF(gx, pad_t + eh));
            double xv = x_min + (x_max - x_min) * double(i) / 6.0;
            p.setPen(QColor(Theme::TEXT_SECONDARY));
            p.setFont(QFont("Monospace", 8));
            p.drawText(QRectF(gx - 25, pad_t + eh + 6, 50, 20), Qt::AlignCenter, QString::number(xv, 'g', 3));
            p.setPen(QPen(QColor(Theme::GRID_MAJOR), 1.0));
        }
        for (int i = 0; i <= 5; ++i) {
            double gy = pad_t + eh * double(i) / 5.0;
            p.drawLine(QPointF(pad_l, gy), QPointF(pad_l + ew, gy));
            double yv = y_max - (y_max - y_min) * double(i) / 5.0;
            p.setPen(QColor(Theme::TEXT_SECONDARY));
            p.setFont(QFont("Monospace", 8));
            p.drawText(QRectF(2, gy - 10, pad_l - 6, 20), Qt::AlignRight | Qt::AlignVCenter, QString::number(yv, 'g', 3));
            p.setPen(QPen(QColor(Theme::GRID_MAJOR), 1.0));
        }

        // Zero Axis
        double y0_px = toPx(0.0, 0.0).y();
        if (y0_px >= pad_t && y0_px <= pad_t + eh) {
            p.setPen(QPen(QColor(Theme::AXIS_LINE), 1.3));
            p.drawLine(QPointF(pad_l, y0_px), QPointF(pad_l + ew, y0_px));
        }

        if (!is_discrete) {
            const auto& r = sim_res.value();
            if (show_bounds) {
                QPainterPath ref_p;
                for (size_t i = 0; i < r.t.size(); ++i) {
                    QPointF pt = toPx(r.t[i], r.r[i]);
                    if (i == 0) ref_p.moveTo(pt); else ref_p.lineTo(pt);
                }
                p.setPen(QPen(QColor(Theme::ACCENT_ZERO), 1.2, Qt::DashLine));
                p.drawPath(ref_p);

                if (r.stability_state == "STABLE") {
                    double yss = r.y_steady.real();
                    double band = std::abs(yss) * (settle_tol_pct / 100.0);
                    p.setPen(QPen(QColor(Theme::ACCENT_ZERO), 0.9, Qt::DotLine));
                    p.drawLine(toPx(x_min, yss + band), toPx(x_max, yss + band));
                    p.drawLine(toPx(x_min, yss - band), toPx(x_max, yss - band));
                }
            }
            if (show_ol) {
                QPainterPath ol_p;
                for (size_t i = 0; i < r.t.size(); ++i) {
                    QPointF pt = toPx(r.t[i], r.y_open[i].real());
                    if (i == 0) ol_p.moveTo(pt); else ol_p.lineTo(pt);
                }
                p.setPen(QPen(QColor(Theme::ACCENT_OPEN), 1.8));
                p.drawPath(ol_p);
            }
            if (show_cl) {
                QPainterPath cl_p;
                for (size_t i = 0; i < r.t.size(); ++i) {
                    QPointF pt = toPx(r.t[i], r.y_closed[i].real());
                    if (i == 0) cl_p.moveTo(pt); else cl_p.lineTo(pt);
                }
                p.setPen(QPen(QColor(Theme::ACCENT_LOCUS), 2.3));
                p.drawPath(cl_p);
            }
            if (show_u) {
                QPainterPath u_p;
                for (size_t i = 0; i < r.t.size(); ++i) {
                    QPointF pt = toPx(r.t[i], r.u_control[i].real());
                    if (i == 0) u_p.moveTo(pt); else u_p.lineTo(pt);
                }
                p.setPen(QPen(QColor(Theme::ACCENT_POLE), 1.4, Qt::DashLine));
                p.drawPath(u_p);
            }

            // Cursor
            QPointF c_px = toPx(r.cursor_t, r.cursor_y.real());
            QColor b_col(r.cursor_within_tol ? Theme::ACCENT_PID : Theme::ACCENT_OPEN);
            p.setPen(QPen(b_col, 1.0, Qt::DashLine));
            p.drawLine(QPointF(c_px.x(), pad_t), QPointF(c_px.x(), pad_t + eh));
            p.setBrush(Qt::white);
            p.setPen(QPen(b_col, 2.2));
            p.drawEllipse(c_px, 5.5, 5.5);

            QString tag = QString("t=%1s, y=%2")
                .arg(QString::fromStdString(PRECISION.fmt(r.cursor_t)))
                .arg(QString::fromStdString(PRECISION.fmtComplex(r.cursor_y)));
            p.setFont(QFont("Monospace", 9, QFont::Bold));
            p.drawText(c_px + QPointF(10, -10), tag);
        } else {
            const auto& r = dsim_res.value();
            if (show_bounds) {
                QPainterPath ref_p;
                for (size_t i = 0; i < r.k_idx.size(); ++i) {
                    QPointF pt = toPx(r.k_idx[i], r.r_seq[i]);
                    if (i == 0) ref_p.moveTo(pt); else ref_p.lineTo(pt);
                }
                p.setPen(QPen(QColor(Theme::ACCENT_ZERO), 1.2, Qt::DashLine));
                p.drawPath(ref_p);
            }
            const auto& seq = r.y_primary;
            QColor seq_col(show_cl ? Theme::ACCENT_LOCUS : Theme::ACCENT_OPEN);
            QPainterPath path;
            for (size_t i = 0; i < r.k_idx.size(); ++i) {
                QPointF pt = toPx(r.k_idx[i], seq[i].real());
                if (i == 0) path.moveTo(pt); else path.lineTo(pt);
                if (show_stems) {
                    p.setPen(QPen(seq_col, 1.0));
                    p.drawLine(toPx(r.k_idx[i], 0.0), pt);
                    p.setBrush(seq_col);
                    p.drawEllipse(pt, 3.0, 3.0);
                }
            }
            p.setPen(QPen(seq_col, 2.0));
            p.drawPath(path);

            QPointF c_px = toPx(r.cursor_k, r.cursor_y.real());
            QColor b_col(r.cursor_within_tol ? Theme::ACCENT_PID : Theme::ACCENT_OPEN);
            p.setPen(QPen(b_col, 1.0, Qt::DashLine));
            p.drawLine(QPointF(c_px.x(), pad_t), QPointF(c_px.x(), pad_t + eh));
            p.setBrush(Qt::white);
            p.setPen(QPen(b_col, 2.2));
            p.drawEllipse(c_px, 5.5, 5.5);

            QString tag = QString("k=%1 (t=%2s), y=%3")
                .arg(r.cursor_k)
                .arg(QString::fromStdString(PRECISION.fmt(r.cursor_t)))
                .arg(QString::fromStdString(PRECISION.fmtComplex(r.cursor_y)));
            p.setFont(QFont("Monospace", 9, QFont::Bold));
            p.drawText(c_px + QPointF(10, -10), tag);
        }
    }

    void mousePressEvent(QMouseEvent* e) override {
        if (e->button() == Qt::LeftButton) updateCursorFromPx(e->pos().x());
    }
    void mouseMoveEvent(QMouseEvent* e) override {
        if (e->buttons() & Qt::LeftButton) updateCursorFromPx(e->pos().x());
    }

private:
    void updateCursorFromPx(double px) {
        double pad_l = 52.0, pad_r = 22.0;
        double ew = std::max(10.0, width() - pad_l - pad_r);
        double frac = std::clamp((px - pad_l) / ew, 0.0, 1.0);
        if (!is_discrete && sim_res.has_value() && !sim_res->t.empty()) {
            if (onCursorChanged) onCursorChanged(frac * sim_res->t.back());
        } else if (is_discrete && dsim_res.has_value() && !dsim_res->k_idx.empty()) {
            if (onCursorChanged) onCursorChanged( std::round(frac * dsim_res->k_idx.back()) );
        }
    }
};

// =============================================================================
// Helper UI Factory Functions
// =============================================================================
QFrame* makeCard(QWidget* parent = nullptr) {
    auto* f = new QFrame(parent);
    f->setStyleSheet(QString("QFrame { background-color: %1; border: 1px solid %2; border-radius: 6px; }")
                     .arg(Theme::BG_PANEL, Theme::BORDER_PANEL));
    return f;
}

QLineEdit* makeDarkEntry(const QString& init_val = "", int width = 90) {
    auto* e = new QLineEdit(init_val);
    if (width > 0) e->setMinimumWidth(width);
    e->setStyleSheet(QString(
        "QLineEdit { background-color: %1; color: %2; border: 1px solid %3; border-radius: 4px; padding: 4px 6px; font-family: Monospace; font-weight: bold; }"
        "QLineEdit:focus { border: 1px solid %4; }"
        "QLineEdit:disabled { color: %5; background-color: #080a0f; }"
    ).arg(Theme::BG_INPUT, Theme::TEXT_PRIMARY, Theme::BORDER_INPUT, Theme::ACCENT_ZERO, Theme::TEXT_MUTED));
    return e;
}

QPushButton* makeActionBtn(const QString& text, const QString& accent_fg, const QString& bg = Theme::BG_INPUT) {
    auto* b = new QPushButton(text);
    b->setCursor(Qt::PointingHandCursor);
    b->setStyleSheet(QString(
        "QPushButton { background-color: %1; color: %2; border: 1px solid %2; border-radius: 4px; padding: 5px 10px; font-weight: bold; font-size: 11px; }"
        "QPushButton:hover { background-color: #1e293b; }"
        "QPushButton:disabled { color: %3; border-color: %3; }"
    ).arg(bg, accent_fg, Theme::TEXT_MUTED));
    return b;
}

// =============================================================================
// Tab 1: s-Plane Root Locus & Frequency Response Tab
// =============================================================================
class RootLocusTab : public QWidget {
    Q_OBJECT
public:
    SharedSystemModel* model;
    std::function<void(double)> onSendGainToPid;

    QLineEdit *num_edit, *den_edit, *tp_re_edit, *tp_im_edit, *tol_edit;
    QLabel *poly_status_lbl, *banner_title, *banner_right, *banner_sub;
    QLabel *val_angle, *val_mag, *val_gain;
    ComplexPlaneCanvas* canvas;
    double last_gain = 1.0;

    RootLocusTab(SharedSystemModel* m, std::function<void(double)> send_cb, QWidget* parent = nullptr)
        : QWidget(parent), model(m), onSendGainToPid(std::move(send_cb))
    {
        auto* root_layout = new QHBoxLayout(this);
        root_layout->setContentsMargins(0, 0, 0, 0);

        auto* splitter = new QSplitter(Qt::Horizontal, this);
        root_layout->addWidget(splitter);

        auto* left_w = new QWidget();
        auto* left_v = new QVBoxLayout(left_w);
        left_v->setContentsMargins(0, 0, 6, 0);

        // 1. Num(s) / Den(s) Polynomial Auto-Solver Card
        auto* poly_card = makeCard();
        auto* poly_l = new QVBoxLayout(poly_card);
        auto* poly_hdr = new QHBoxLayout();
        auto* title_l = new QLabel("Transfer Function  G(s) = Num(s) / Den(s)");
        title_l->setStyleSheet(QString("color: %1; font-weight: bold; border: none;").arg(Theme::ACCENT_ZERO));
        auto* solve_btn = makeActionBtn("Solve Zeros & Poles ➔", Theme::ACCENT_PID);
        poly_hdr->addWidget(title_l);
        poly_hdr->addStretch();
        poly_hdr->addWidget(solve_btn);
        poly_l->addLayout(poly_hdr);

        auto* pgrid = new QGridLayout();
        auto* nl = new QLabel("Num(s):"); nl->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; border: none;").arg(Theme::ACCENT_ZERO));
        num_edit = makeDarkEntry(QString::fromStdString(formatPolynomialFromRoots(model->zeros, "s")));
        auto* dl = new QLabel("Den(s):"); dl->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; border: none;").arg(Theme::ACCENT_POLE));
        den_edit = makeDarkEntry(QString::fromStdString(formatPolynomialFromRoots(model->poles, "s")));
        pgrid->addWidget(nl, 0, 0); pgrid->addWidget(num_edit, 0, 1);
        pgrid->addWidget(dl, 0, 2); pgrid->addWidget(den_edit, 0, 3);
        poly_l->addLayout(pgrid);

        poly_status_lbl = new QLabel("Enter s-polynomials (e.g. s^2+2s+2 or (s+1)(s+3)) and press Enter / Solve");
        poly_status_lbl->setStyleSheet(QString("color: %1; font-size: 11px; border: none;").arg(Theme::TEXT_SECONDARY));
        poly_l->addWidget(poly_status_lbl);
        left_v->addWidget(poly_card);

        // 2. Test Point s & Angle Tolerance Card
        auto* tp_card = makeCard();
        auto* tp_l = new QVBoxLayout(tp_card);
        auto* tp_title = new QLabel("Test Point  s = σ + jω  &  Angle Tolerance");
        tp_title->setStyleSheet(QString("color: %1; font-weight: bold; border: none;").arg(Theme::ACCENT_PID));
        tp_l->addWidget(tp_title);

        auto* tp_grid = new QGridLayout();
        tp_re_edit = makeDarkEntry(QString::number(model->test_point.real()));
        tp_im_edit = makeDarkEntry(QString::number(model->test_point.imag()));
        tol_edit   = makeDarkEntry(QString::number(model->rl_tol_deg), 60);
        auto* l_re = new QLabel("Real (σ):"); l_re->setStyleSheet("color: #94a3b8; border:none;");
        auto* l_im = new QLabel("Imag (jω):"); l_im->setStyleSheet("color: #94a3b8; border:none;");
        auto* l_tl = new QLabel("Tol (±°):"); l_tl->setStyleSheet("color: #94a3b8; border:none;");
        tp_grid->addWidget(l_re, 0, 0); tp_grid->addWidget(tp_re_edit, 1, 0);
        tp_grid->addWidget(l_im, 0, 1); tp_grid->addWidget(tp_im_edit, 1, 1);
        tp_grid->addWidget(l_tl, 0, 2); tp_grid->addWidget(tol_edit, 1, 2);
        tp_l->addLayout(tp_grid);
        left_v->addWidget(tp_card);

        // 3. Evaluation Results Card
        auto* res_card = makeCard();
        auto* res_l = new QVBoxLayout(res_card);
        auto* res_hdr = new QHBoxLayout();
        auto* res_title = new QLabel("Root Locus Evaluation Results");
        res_title->setStyleSheet(QString("color: %1; font-weight: bold; border: none;").arg(Theme::TEXT_PRIMARY));
        auto* send_btn = makeActionBtn("Send Gain K → PID Simulator ➔", Theme::ACCENT_PID);
        res_hdr->addWidget(res_title);
        res_hdr->addStretch();
        res_hdr->addWidget(send_btn);
        res_l->addLayout(res_hdr);

        auto* banner = new QFrame();
        banner->setStyleSheet(QString("background-color: %1; border: 1px solid %2; border-radius: 5px; padding: 6px;")
                              .arg(Theme::BANNER_OK_BG, Theme::BANNER_OK_BD));
        auto* b_l = new QVBoxLayout(banner);
        auto* b_top = new QHBoxLayout();
        banner_title = new QLabel("✔ ON ROOT LOCUS");
        banner_title->setStyleSheet(QString("color: %1; font-weight: bold; border: none;").arg(Theme::BANNER_OK_FG));
        banner_right = new QLabel("Δθ = 0.00°");
        banner_right->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; border: none;").arg(Theme::TEXT_PRIMARY));
        b_top->addWidget(banner_title); b_top->addStretch(); b_top->addWidget(banner_right);
        banner_sub = new QLabel("Satisfies ∠G(s) = ±180°(2k+1)");
        banner_sub->setStyleSheet("color: #a7f3d0; font-size: 11px; border: none;");
        b_l->addLayout(b_top);
        b_l->addWidget(banner_sub);
        res_l->addWidget(banner);

        auto* m_grid = new QHBoxLayout();
        auto makeMetric = [&](const QString& lbl, QLabel*& out_lbl) {
            auto* box = makeCard();
            box->setStyleSheet(QString("background-color: %1; border: 1px solid %2; border-radius: 4px; padding: 6px;").arg(Theme::BG_INPUT, Theme::BORDER_INPUT));
            auto* vl = new QVBoxLayout(box);
            auto* t = new QLabel(lbl); t->setStyleSheet("color: #94a3b8; font-size: 10px; font-weight: bold; border:none;");
            out_lbl = new QLabel("—"); out_lbl->setStyleSheet(QString("color: %1; font-family: Monospace; font-size: 13px; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));
            vl->addWidget(t); vl->addWidget(out_lbl);
            m_grid->addWidget(box);
        };
        makeMetric("NET ANGLE ∠G(s)", val_angle);
        makeMetric("MAGNITUDE |G(s)|", val_mag);
        makeMetric("GAIN K = 1/|G(s)|", val_gain);
        res_l->addLayout(m_grid);
        left_v->addWidget(res_card);
        left_v->addStretch();

        // Right Plot Canvas
        auto* right_card = makeCard();
        auto* right_l = new QVBoxLayout(right_card);
        auto* plot_hdr = new QHBoxLayout();
        auto* plot_title = new QLabel("Complex s-Plane Root Locus Diagram (L-Click: Test Pt • Scroll: Zoom • R-Drag: Pan)");
        plot_title->setStyleSheet(QString("color: %1; font-weight: bold; border: none;").arg(Theme::TEXT_PRIMARY));
        auto* fit_btn = makeActionBtn("Fit View", Theme::TEXT_PRIMARY, Theme::BG_PANEL);
        plot_hdr->addWidget(plot_title);
        plot_hdr->addStretch();
        plot_hdr->addWidget(fit_btn);
        right_l->addLayout(plot_hdr);

        canvas = new ComplexPlaneCanvas(false);
        right_l->addWidget(canvas, 1);

        splitter->addWidget(left_w);
        splitter->addWidget(right_card);
        splitter->setStretchFactor(0, 0);
        splitter->setStretchFactor(1, 1);
        left_w->setMinimumWidth(450);

        // Connections
        auto solveFn = [this]() { applyPolyNumDen(); };
        connect(solve_btn, &QPushButton::clicked, this, solveFn);
        connect(num_edit, &QLineEdit::returnPressed, this, solveFn);
        connect(den_edit, &QLineEdit::returnPressed, this, solveFn);

        auto evalFn = [this]() { evaluateCurrent(true); };
        connect(tp_re_edit, &QLineEdit::textEdited, this, evalFn);
        connect(tp_im_edit, &QLineEdit::textEdited, this, evalFn);
        connect(tol_edit, &QLineEdit::textEdited, this, evalFn);
        connect(fit_btn, &QPushButton::clicked, this, [this]() { canvas->resetView(); });
        connect(send_btn, &QPushButton::clicked, this, [this]() {
            if (std::isfinite(last_gain) && last_gain >= 0.0 && onSendGainToPid) onSendGainToPid(last_gain);
        });

        canvas->onPointSelected = [this](Complex pt) {
            tp_re_edit->setText(QString::number(pt.real(), 'g', 4));
            tp_im_edit->setText(QString::number(pt.imag(), 'g', 4));
            evaluateCurrent(true);
        };

        loadFromModel();
    }

    void applyPolyNumDen() {
        try {
            auto p_num = parsePolynomialInput(num_edit->text().toStdString());
            auto p_den = parsePolynomialInput(den_edit->text().toStdString());
            model->zeros = p_num.roots;
            model->poles = p_den.roots;
            double ratio = p_num.leading_gain / std::max(std::abs(p_den.leading_gain), 1e-12);
            if (std::abs(ratio) > 1e-12) model->plant_gain = ratio;
            poly_status_lbl->setText(QString("✔ Solved %1 Poles & %2 Zeros (K = %3)")
                                     .arg(model->poles.size()).arg(model->zeros.size()).arg(model->plant_gain));
            evaluateCurrent(true);
        } catch (const std::exception& e) {
            poly_status_lbl->setText(QString("⚠ Polynomial error: %1").arg(e.what()));
        }
    }

    void loadFromModel() {
        num_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->zeros, "s")));
        den_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->poles, "s")));
        tp_re_edit->setText(QString::number(model->test_point.real(), 'g', 4));
        tp_im_edit->setText(QString::number(model->test_point.imag(), 'g', 4));
        tol_edit->setText(QString::number(model->rl_tol_deg, 'g', 3));
        evaluateCurrent(false);
    }

    void evaluateCurrent(bool notify) {
        Complex tp(parseNumericInput(tp_re_edit->text().toStdString(), 0.0),
                   parseNumericInput(tp_im_edit->text().toStdString(), 0.0));
        double tol = std::abs(parseNumericInput(tol_edit->text().toStdString(), 0.5));
        model->test_point = tp;
        model->rl_tol_deg = tol;

        auto ev = evaluateTransferFunction(model->poles, model->zeros, tp, model->plant_gain);
        last_gain = ev.rl_gain_k;
        double def = computeAngleDeficiency(ev.angle_deg);
        bool on_rl = ev.at_pole || (std::abs(def) <= tol);

        val_angle->setText(QString::fromStdString(PRECISION.fmt(ev.angle_deg) + "°"));
        val_mag->setText(QString::fromStdString(PRECISION.fmt(ev.magnitude)));
        val_gain->setText(QString::fromStdString(PRECISION.fmt(ev.rl_gain_k)));

        banner_title->setText(on_rl ? "✔ ON ROOT LOCUS" : "✖ NOT ON ROOT LOCUS");
        banner_right->setText(QString("Δθ = %1°").arg(QString::fromStdString(PRECISION.fmt(def, true))));
        banner_sub->setText(on_rl
            ? QString("Satisfies ∠G(s) = ±180°(2k+1) — Closed-loop pole at s for K = %1").arg(QString::fromStdString(PRECISION.fmt(ev.rl_gain_k)))
            : QString("Requires %1° compensator phase lead to place closed-loop pole at s").arg(QString::fromStdString(PRECISION.fmt(def, true))));

        canvas->poles = model->poles;
        canvas->zeros = model->zeros;
        canvas->test_point = tp;
        canvas->is_on_rl = on_rl;
        canvas->update();

        if (notify) model->notify("root_locus");
    }
};

// =============================================================================
// Tab 2: Time Response & PID Simulator Tab (With PID Disable / Bypass Option)
// =============================================================================
class PIDTimeResponseTab : public QWidget {
    Q_OBJECT
public:
    SharedSystemModel* model;
    SimulationConfig cfg;

    QLineEdit *num_edit, *den_edit, *os_target_edit, *zeta_target_edit;
    QLineEdit *kp_edit, *ki_edit, *kd_edit;
    QSlider *kp_slider, *ki_slider, *kd_slider;
    QCheckBox *enable_pid_chk;
    QComboBox *loop_mode_combo, *sig_combo;
    QLineEdit *mag_edit, *gain_edit, *cursor_edit, *tmax_edit, *tol_edit;
    QLabel *poly_status_lbl, *status_pill;
    QLabel *m_card1, *m_card2, *m_card3, *m_card4, *m_card5, *m_card6;
    TimeResponseCanvas* canvas;
    bool updating_ui = false;

    explicit PIDTimeResponseTab(SharedSystemModel* m, QWidget* parent = nullptr)
        : QWidget(parent), model(m)
    {
        auto* root_layout = new QHBoxLayout(this);
        root_layout->setContentsMargins(0, 0, 0, 0);
        auto* splitter = new QSplitter(Qt::Horizontal, this);
        root_layout->addWidget(splitter);

        auto* left_w = new QWidget();
        auto* left_v = new QVBoxLayout(left_w);
        left_v->setContentsMargins(0, 0, 6, 0);

        // 1. Plant G(s) Polynomial Solver Card
        auto* poly_card = makeCard();
        auto* poly_l = new QVBoxLayout(poly_card);
        auto* poly_hdr = new QHBoxLayout();
        auto* t1 = new QLabel("1. Plant Transfer Function  G(s) = K · Num(s) / Den(s)");
        t1->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));
        auto* solve_btn = makeActionBtn("Solve ➔", Theme::ACCENT_PID);
        poly_hdr->addWidget(t1); poly_hdr->addStretch(); poly_hdr->addWidget(solve_btn);
        poly_l->addLayout(poly_hdr);

        auto* pgrid = new QGridLayout();
        num_edit = makeDarkEntry();
        den_edit = makeDarkEntry();
        auto* nl = new QLabel("Num(s):"); nl->setStyleSheet("color: #38bdf8; font-family: Monospace; font-weight: bold; border:none;");
        auto* dl = new QLabel("Den(s):"); dl->setStyleSheet("color: #ff5f56; font-family: Monospace; font-weight: bold; border:none;");
        pgrid->addWidget(nl, 0, 0); pgrid->addWidget(num_edit, 0, 1);
        pgrid->addWidget(dl, 0, 2); pgrid->addWidget(den_edit, 0, 3);
        poly_l->addLayout(pgrid);
        poly_status_lbl = new QLabel("Type Num(s) & Den(s) (e.g. (s+2)(s+3)) and press Enter.");
        poly_status_lbl->setStyleSheet("color: #94a3b8; font-size: 11px; border:none;");
        poly_l->addWidget(poly_status_lbl);
        left_v->addWidget(poly_card);

        // 2. PID Controller & Bypass / Loop Mode Card
        auto* pid_card = makeCard();
        auto* pid_l = new QVBoxLayout(pid_card);
        auto* pid_hdr = new QHBoxLayout();
        auto* t2 = new QLabel("2. PID Controller & Loop Configuration");
        t2->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::ACCENT_PID));

        // NEW: Explicit Checkbox to Disable / Bypass PID Controller Entirely
        enable_pid_chk = new QCheckBox("Enable PID Controller C(s)");
        enable_pid_chk->setChecked(true);
        enable_pid_chk->setStyleSheet(QString("QCheckBox { color: %1; font-weight: bold; border: none; }").arg(Theme::ACCENT_PID));

        loop_mode_combo = new QComboBox();
        loop_mode_combo->addItems({
            "Closed-Loop (with PID)",
            "Closed-Loop (No PID, C(s)=1)",
            "Open-Loop Plant Only (No PID)"
        });
        loop_mode_combo->setStyleSheet(QString("background-color: %1; color: %2; border: 1px solid %3; padding: 3px; font-weight: bold;")
                                       .arg(Theme::BG_INPUT, Theme::TEXT_PRIMARY, Theme::BORDER_INPUT));

        pid_hdr->addWidget(t2);
        pid_hdr->addStretch();
        pid_hdr->addWidget(enable_pid_chk);
        pid_hdr->addWidget(loop_mode_combo);
        pid_l->addLayout(pid_hdr);

        // Quick %OS / Zeta Synthesizer Bar
        auto* synth_bar = new QHBoxLayout();
        auto* l_os = new QLabel("Target %OS:"); l_os->setStyleSheet("color: #94a3b8; font-size: 11px; border:none;");
        os_target_edit = makeDarkEntry("20.0", 55);
        auto* l_zt = new QLabel("or ζ:"); l_zt->setStyleSheet("color: #94a3b8; font-size: 11px; border:none;");
        zeta_target_edit = makeDarkEntry("0.456", 55);
        auto* synth_p_btn  = makeActionBtn("Design P (Kp) ➔", Theme::ACCENT_PID);
        auto* synth_pd_btn = makeActionBtn("Design PD (Kd) ➔", Theme::ACCENT_ZERO);
        synth_bar->addWidget(l_os); synth_bar->addWidget(os_target_edit);
        synth_bar->addWidget(l_zt); synth_bar->addWidget(zeta_target_edit);
        synth_bar->addWidget(synth_p_btn); synth_bar->addWidget(synth_pd_btn);
        pid_l->addLayout(synth_bar);

        // Kp, Ki, Kd Sliders
        auto* gains_grid = new QHBoxLayout();
        auto makeGainCol = [&](const QString& title, QLineEdit*& ed, QSlider*& sl, double max_v) {
            auto* col = new QVBoxLayout();
            auto* lbl = new QLabel(title); lbl->setStyleSheet("color: #94a3b8; font-size: 11px; font-weight: bold; border:none;");
            ed = makeDarkEntry("1.0", 70);
            sl = new QSlider(Qt::Horizontal);
            sl->setRange(0, static_cast<int>(max_v * 100));
            col->addWidget(lbl); col->addWidget(ed); col->addWidget(sl);
            gains_grid->addLayout(col);
        };
        makeGainCol("Kp (Proportional)", kp_edit, kp_slider, 50.0);
        makeGainCol("Ki (Integral)",     ki_edit, ki_slider, 25.0);
        makeGainCol("Kd (Derivative)",   kd_edit, kd_slider, 10.0);
        pid_l->addLayout(gains_grid);

        // Excitation & Simulation Parameters Row
        auto* sim_row = new QGridLayout();
        sig_combo = new QComboBox();
        sig_combo->addItems({"Step", "Impulse", "Ramp", "Sine"});
        sig_combo->setStyleSheet(loop_mode_combo->styleSheet());
        mag_edit    = makeDarkEntry("1.0", 55);
        gain_edit   = makeDarkEntry("1.0", 55);
        cursor_edit = makeDarkEntry("2.5", 55);
        tmax_edit   = makeDarkEntry("10.0", 55);
        tol_edit    = makeDarkEntry("2.0", 55);

        auto addLabeled = [&](int col, const QString& txt, QWidget* w) {
            auto* l = new QLabel(txt); l->setStyleSheet("color: #94a3b8; font-size: 10px; font-weight: bold; border:none;");
            sim_row->addWidget(l, 0, col);
            sim_row->addWidget(w, 1, col);
        };
        addLabeled(0, "Input r(t)", sig_combo);
        addLabeled(1, "Amp M", mag_edit);
        addLabeled(2, "Gain K", gain_edit);
        addLabeled(3, "Cursor t", cursor_edit);
        addLabeled(4, "T_max (s)", tmax_edit);
        addLabeled(5, "Tol (±%)", tol_edit);
        pid_l->addLayout(sim_row);
        left_v->addWidget(pid_card);

        // 3. Unified 6-Card System Telemetry Grid
        auto* tel_card = makeCard();
        auto* tel_l = new QVBoxLayout(tel_card);
        auto* tel_hdr = new QHBoxLayout();
        auto* t3 = new QLabel("3. System Telemetry");
        t3->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::TEXT_PRIMARY));
        status_pill = new QLabel("● Stable • Underdamped");
        status_pill->setStyleSheet(QString("background-color: %1; color: %2; padding: 2px 8px; border-radius: 4px; font-weight: bold; font-size: 11px;")
                                   .arg(Theme::BANNER_OK_BG, Theme::BANNER_OK_FG));
        tel_hdr->addWidget(t3); tel_hdr->addWidget(status_pill); tel_hdr->addStretch();
        tel_l->addLayout(tel_hdr);

        auto* mgrid = new QGridLayout();
        auto makeTelCard = [&](int r, int c, const QString& title, QLabel*& val_lbl) {
            auto* b = makeCard();
            b->setStyleSheet(QString("background-color: %1; border: 1px solid %2; border-radius: 4px; padding: 5px;").arg(Theme::BG_INPUT, Theme::BORDER_INPUT));
            auto* vl = new QVBoxLayout(b);
            auto* tl = new QLabel(title); tl->setStyleSheet("color: #94a3b8; font-size: 9px; font-weight: bold; border:none;");
            val_lbl = new QLabel("—"); val_lbl->setStyleSheet(QString("color: %1; font-family: Monospace; font-size: 11px; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));
            vl->addWidget(tl); vl->addWidget(val_lbl);
            mgrid->addWidget(b, r, c);
        };
        makeTelCard(0, 0, "OVERSHOOT, S.S.E. & RELATIVE ERROR", m_card1);
        makeTelCard(0, 1, "RESPONSE TIMES (Tr, Tp & Ts)", m_card2);
        makeTelCard(1, 0, "DAMPING ζ & FREQUENCIES (ωn, ωd, σd)", m_card3);
        makeTelCard(1, 1, "STEADY-STATE y_ss, TYPE & Kp,pos / Kv", m_card4);
        makeTelCard(2, 0, "CLOSED-LOOP & OPEN-LOOP POLES", m_card5);
        makeTelCard(2, 1, "CURSOR INSPECTION @ t", m_card6);
        tel_l->addLayout(mgrid);
        left_v->addWidget(tel_card, 1);

        // Right Plot Panel
        auto* right_card = makeCard();
        auto* right_l = new QVBoxLayout(right_card);
        auto* plot_hdr = new QHBoxLayout();
        auto* pt = new QLabel("Time-Domain Response  y(t)  (L-Click: Move Cursor t)");
        pt->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::TEXT_PRIMARY));
        auto* chk_ol = new QCheckBox("Overlay Open-Loop"); chk_ol->setStyleSheet("color: #f59e0b; font-weight: bold;");
        auto* chk_u  = new QCheckBox("Control u(t)");      chk_u->setStyleSheet("color: #ff5f56; font-weight: bold;");
        plot_hdr->addWidget(pt); plot_hdr->addStretch(); plot_hdr->addWidget(chk_ol); plot_hdr->addWidget(chk_u);
        right_l->addLayout(plot_hdr);

        canvas = new TimeResponseCanvas(false);
        right_l->addWidget(canvas, 1);

        splitter->addWidget(left_w);
        splitter->addWidget(right_card);
        splitter->setStretchFactor(0, 0);
        splitter->setStretchFactor(1, 1);
        left_w->setMinimumWidth(480);

        // Signal Connections
        connect(solve_btn, &QPushButton::clicked, this, &PIDTimeResponseTab::applyPolyNumDen);
        connect(num_edit, &QLineEdit::returnPressed, this, &PIDTimeResponseTab::applyPolyNumDen);
        connect(den_edit, &QLineEdit::returnPressed, this, &PIDTimeResponseTab::applyPolyNumDen);

        // PID Enable / Disable & Loop Mode synchronization
        connect(enable_pid_chk, &QCheckBox::toggled, this, [this](bool checked) {
            if (updating_ui) return;
            updating_ui = true;
            if (!checked && loop_mode_combo->currentIndex() == 0) {
                loop_mode_combo->setCurrentIndex(1); // Switch to Closed-Loop (No PID, C(s)=1)
            } else if (checked && loop_mode_combo->currentIndex() == 1) {
                loop_mode_combo->setCurrentIndex(0); // Switch to Closed-Loop (with PID)
            }
            updating_ui = false;
            updatePidWidgetsState();
            runSimulation(true);
        });

        connect(loop_mode_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
            if (updating_ui) return;
            updating_ui = true;
            if (idx == 0) enable_pid_chk->setChecked(true);
            else if (idx == 1) enable_pid_chk->setChecked(false);
            updating_ui = false;
            updatePidWidgetsState();
            runSimulation(true);
        });

        auto bindSliderEdit = [this](QSlider* sl, QLineEdit* ed) {
            connect(sl, &QSlider::valueChanged, this, [this, ed](int v) {
                if (updating_ui) return;
                updating_ui = true;
                ed->setText(QString::number(v / 100.0, 'g', 4));
                updating_ui = false;
                runSimulation(true);
            });
            connect(ed, &QLineEdit::editingFinished, this, [this, sl, ed]() {
                if (updating_ui) return;
                double val = parseNumericInput(ed->text().toStdString(), 0.0);
                updating_ui = true;
                sl->setValue(static_cast<int>(std::round(val * 100.0)));
                updating_ui = false;
                runSimulation(true);
            });
        };
        bindSliderEdit(kp_slider, kp_edit);
        bindSliderEdit(ki_slider, ki_edit);
        bindSliderEdit(kd_slider, kd_edit);

        connect(synth_p_btn, &QPushButton::clicked, this, [this]() { synthesizeController(false); });
        connect(synth_pd_btn, &QPushButton::clicked, this, [this]() { synthesizeController(true); });

        for (auto* ed : {mag_edit, gain_edit, cursor_edit, tmax_edit, tol_edit}) {
            connect(ed, &QLineEdit::editingFinished, this, [this]() { runSimulation(true); });
        }
        connect(sig_combo, &QComboBox::currentTextChanged, this, [this]() { runSimulation(true); });
        connect(chk_ol, &QCheckBox::toggled, this, [this](bool v) { canvas->show_ol = v; canvas->update(); });
        connect(chk_u,  &QCheckBox::toggled, this, [this](bool v) { canvas->show_u  = v; canvas->update(); });

        canvas->onCursorChanged = [this](double t_val) {
            cursor_edit->setText(QString::number(t_val, 'g', 4));
            runSimulation(false);
        };

        loadFromModel();
    }

    void updatePidWidgetsState() {
        bool pid_active = enable_pid_chk->isChecked() && (loop_mode_combo->currentIndex() == 0);
        for (QWidget* w : std::vector<QWidget*>{kp_edit, ki_edit, kd_edit, kp_slider, ki_slider, kd_slider}) {
            w->setEnabled(pid_active);
        }
    }

    void synthesizeController(bool design_kd) {
        double os_v = parseNumericInput(os_target_edit->text().toStdString(), 20.0);
        double z = PRECISION.step(niseZetaFromOs(os_v));
        zeta_target_edit->setText(QString::fromStdString(PRECISION.fmt(z)));

        auto [num_g, den_g] = ComplexTransferFunctionSimulator::buildPlantPolynomials(model->poles, model->zeros, model->plant_gain);
        if (den_g.size() >= 3) {
            double a1 = den_g[den_g.size() - 2].real();
            double a0 = den_g.back().real();
            double k0 = std::max(std::abs(num_g.back().real()), 1e-9);
            enable_pid_chk->setChecked(true);
            loop_mode_combo->setCurrentIndex(0);
            if (!design_kd) {
                double wn = PRECISION.step(a1 / (2.0 * std::max(z, 1e-5)));
                double kp_new = PRECISION.step((wn * wn - a0) / k0);
                kp_edit->setText(QString::fromStdString(PRECISION.fmt(kp_new)));
                ki_edit->setText("0");
                kd_edit->setText("0");
            } else {
                double kp_cur = parseNumericInput(kp_edit->text().toStdString(), 24.06);
                double wn = PRECISION.step(std::sqrt(std::max(1e-9, a0 + k0 * kp_cur)));
                double kd_new = PRECISION.step((2.0 * z * wn - a1) / k0);
                kd_edit->setText(QString::fromStdString(PRECISION.fmt(kd_new)));
                ki_edit->setText("0");
            }
            runSimulation(true);
        }
    }

    void applyPolyNumDen() {
        try {
            auto pn = parsePolynomialInput(num_edit->text().toStdString());
            auto pd = parsePolynomialInput(den_edit->text().toStdString());
            model->zeros = pn.roots;
            model->poles = pd.roots;
            double ratio = pn.leading_gain / std::max(std::abs(pd.leading_gain), 1e-12);
            if (std::abs(ratio) > 1e-12) {
                model->plant_gain = ratio;
                gain_edit->setText(QString::number(ratio, 'g', 5));
            }
            runSimulation(true);
        } catch (const std::exception& e) {
            poly_status_lbl->setText(QString("⚠ %1").arg(e.what()));
        }
    }

    void loadFromModel() {
        updating_ui = true;
        num_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->zeros, "s")));
        den_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->poles, "s")));
        gain_edit->setText(QString::number(model->plant_gain, 'g', 4));
        mag_edit->setText(QString::number(model->input_magnitude, 'g', 4));
        enable_pid_chk->setChecked(model->pid_enabled);
        kp_edit->setText(QString::number(model->kp, 'g', 4));
        ki_edit->setText(QString::number(model->ki, 'g', 4));
        kd_edit->setText(QString::number(model->kd, 'g', 4));
        kp_slider->setValue(static_cast<int>(model->kp * 100));
        ki_slider->setValue(static_cast<int>(model->ki * 100));
        kd_slider->setValue(static_cast<int>(model->kd * 100));
        updating_ui = false;
        updatePidWidgetsState();
        runSimulation(false);
    }

    void runSimulation(bool notify) {
        if (updating_ui) return;
        cfg.poles = model->poles;
        cfg.zeros = model->zeros;
        cfg.plant_gain = parseNumericInput(gain_edit->text().toStdString(), 1.0);
        cfg.input_magnitude = parseNumericInput(mag_edit->text().toStdString(), 1.0);

        int mode_idx = loop_mode_combo->currentIndex();
        cfg.closed_loop = (mode_idx != 2);
        cfg.pid_enabled = enable_pid_chk->isChecked() && (mode_idx == 0);

        cfg.kp = parseNumericInput(kp_edit->text().toStdString(), 1.5);
        cfg.ki = parseNumericInput(ki_edit->text().toStdString(), 0.0);
        cfg.kd = parseNumericInput(kd_edit->text().toStdString(), 0.0);
        cfg.signal_type = sig_combo->currentText().toStdString();
        cfg.cursor_time = parseNumericInput(cursor_edit->text().toStdString(), 2.5);
        cfg.t_max = std::max(0.2, parseNumericInput(tmax_edit->text().toStdString(), 10.0));
        cfg.settle_tol_pct = std::max(0.1, parseNumericInput(tol_edit->text().toStdString(), 2.0));

        SimulationResult res = ComplexTransferFunctionSimulator::evaluate(cfg);
        model->last_sim_result = res;

        if (notify) {
            model->plant_gain = cfg.plant_gain;
            model->input_magnitude = cfg.input_magnitude;
            model->pid_enabled = cfg.pid_enabled;
            model->kp = cfg.kp;
            model->ki = cfg.ki;
            model->kd = cfg.kd;
            model->notify("pid_sim");
        }

        // Update Telemetry Cards
        QString mode_tag = !cfg.closed_loop ? "Open-Loop" : (cfg.pid_enabled ? "CL (PID ON)" : "CL (PID OFF, C=1)");
        status_pill->setText(QString("● %1 • %2 • %3").arg(QString::fromStdString(res.stability_state), mode_tag, QString::fromStdString(res.damping_regime)));

        m_card1->setText(QString("%OS: %1% (Nise: %2%) | Peak: %3\nS.S.E.: %4 | Rel. Err: %5%")
            .arg(QString::fromStdString(PRECISION.fmt(res.overshoot_pct)))
            .arg(QString::fromStdString(PRECISION.fmt(res.theo_os_pct)))
            .arg(QString::fromStdString(PRECISION.fmt(res.peak_value)))
            .arg(QString::fromStdString(PRECISION.fmt(res.steady_state_error)))
            .arg(QString::fromStdString(PRECISION.fmt(res.relative_error_pct))));

        m_card2->setText(QString("Tr: %1s (Nise: %2s) | Tp: %3s\nTs(±2%): %4s (4/σd: %5s) | Ts(±5%): %6s")
            .arg(QString::fromStdString(PRECISION.fmt(res.rise_time)))
            .arg(QString::fromStdString(PRECISION.fmt(res.theo_tr_sec)))
            .arg(QString::fromStdString(PRECISION.fmt(res.peak_time)))
            .arg(QString::fromStdString(PRECISION.fmt(res.settling_time, false, false, ">Tmax")))
            .arg(QString::fromStdString(PRECISION.fmt(res.theo_ts2_sec)))
            .arg(QString::fromStdString(PRECISION.fmt(res.settling_time_5pct, false, false, ">Tmax"))));

        m_card3->setText(QString("ζ: %1 (OL: %2) | ωn: %3 r/s\nωd: %4 r/s | σd: %5 s⁻¹ (τ=%6s)")
            .arg(QString::fromStdString(PRECISION.fmt(res.zeta)))
            .arg(QString::fromStdString(PRECISION.fmt(res.ol_zeta)))
            .arg(QString::fromStdString(PRECISION.fmt(res.omega_n)))
            .arg(QString::fromStdString(PRECISION.fmt(res.omega_d)))
            .arg(QString::fromStdString(PRECISION.fmt(res.sigma_d)))
            .arg(QString::fromStdString(PRECISION.fmt(res.time_constant_tau))));

        m_card4->setText(QString("y_ss: %1 (M=%2) | Type: %3\nKp,pos: %4 | Kv: %5 | T(0): %6")
            .arg(QString::fromStdString(PRECISION.fmtComplex(res.y_steady)))
            .arg(QString::fromStdString(PRECISION.fmt(cfg.input_magnitude)))
            .arg(res.system_type)
            .arg(QString::fromStdString(PRECISION.fmt(res.kp_pos)))
            .arg(QString::fromStdString(PRECISION.fmt(res.kv_vel)))
            .arg(QString::fromStdString(PRECISION.fmt(res.dc_gain_cl))));

        std::string cl_p_s, ol_p_s;
        for (size_t i = 0; i < std::min<size_t>(3, res.cl_poles.size()); ++i) cl_p_s += (i ? ", " : "") + PRECISION.fmtComplex(res.cl_poles[i]);
        for (size_t i = 0; i < std::min<size_t>(3, res.ol_poles.size()); ++i) ol_p_s += (i ? ", " : "") + PRECISION.fmtComplex(res.ol_poles[i]);
        m_card5->setText(QString("CL Poles: [%1]\nOL Poles: [%2]").arg(QString::fromStdString(cl_p_s), QString::fromStdString(ol_p_s)));

        m_card6->setText(QString("y(%1s): %2 | e(t): %3 (%4%)\nu(%1s): %5 | r(t): %6")
            .arg(QString::fromStdString(PRECISION.fmt(res.cursor_t)))
            .arg(QString::fromStdString(PRECISION.fmtComplex(res.cursor_y)))
            .arg(QString::fromStdString(PRECISION.fmt(res.cursor_abs_err)))
            .arg(QString::fromStdString(PRECISION.fmt(res.cursor_rel_err_pct)))
            .arg(QString::fromStdString(PRECISION.fmtComplex(res.cursor_u)))
            .arg(QString::fromStdString(PRECISION.fmt(res.cursor_r))));

        canvas->sim_res = res;
        canvas->show_cl = cfg.closed_loop;
        if (!cfg.closed_loop) canvas->show_ol = true;
        canvas->settle_tol_pct = cfg.settle_tol_pct;
        canvas->update();
    }
};

// =============================================================================
// Tab 3: z-Plane & Digital Filter Root Locus Tab
// =============================================================================
class ZPlaneFilterTab : public QWidget {
    Q_OBJECT
public:
    SharedSystemModel* model;
    QLineEdit *num_edit, *den_edit, *cutoff_edit, *kd_edit, *ts_edit, *tp_re_edit, *tp_im_edit;
    QComboBox *preset_combo;
    QLabel *diff_eq_lbl, *val_angle, *val_mag, *val_gain, *val_zeta;
    ComplexPlaneCanvas* canvas;

    explicit ZPlaneFilterTab(SharedSystemModel* m, QWidget* parent = nullptr)
        : QWidget(parent), model(m)
    {
        auto* root_layout = new QHBoxLayout(this);
        root_layout->setContentsMargins(0, 0, 0, 0);
        auto* splitter = new QSplitter(Qt::Horizontal, this);
        root_layout->addWidget(splitter);

        auto* left_w = new QWidget();
        auto* left_v = new QVBoxLayout(left_w);
        left_v->setContentsMargins(0, 0, 6, 0);

        // 1. Num(z) / Den(z) Solver Card
        auto* poly_card = makeCard();
        auto* poly_l = new QVBoxLayout(poly_card);
        auto* poly_hdr = new QHBoxLayout();
        auto* t1 = new QLabel("Digital Filter  H(z) = Kd · Num(z) / Den(z)");
        t1->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::ACCENT_ASYMP));
        auto* solve_btn = makeActionBtn("Solve z-Poles & Zeros ➔", Theme::ACCENT_PID);
        poly_hdr->addWidget(t1); poly_hdr->addStretch(); poly_hdr->addWidget(solve_btn);
        poly_l->addLayout(poly_hdr);

        auto* pgrid = new QGridLayout();
        num_edit = makeDarkEntry();
        den_edit = makeDarkEntry();
        auto* nl = new QLabel("Num(z):"); nl->setStyleSheet("color: #38bdf8; font-family: Monospace; font-weight: bold; border:none;");
        auto* dl = new QLabel("Den(z):"); dl->setStyleSheet("color: #ff5f56; font-family: Monospace; font-weight: bold; border:none;");
        pgrid->addWidget(nl, 0, 0); pgrid->addWidget(num_edit, 0, 1);
        pgrid->addWidget(dl, 0, 2); pgrid->addWidget(den_edit, 0, 3);
        poly_l->addLayout(pgrid);
        left_v->addWidget(poly_card);

        // 2. Digital Filter Calculator & Test Point z
        auto* synth_card = makeCard();
        auto* synth_l = new QVBoxLayout(synth_card);
        auto* shdr = new QHBoxLayout();
        auto* st = new QLabel("Filter Synthesizer & Test Point z");
        st->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::ACCENT_PID));
        preset_combo = new QComboBox();
        preset_combo->addItems({
            "butter_lp", "butter_hp", "biquad_bp", "biquad_notch", "fir_ma4", "tustin_from_s", "matched_from_s"
        });
        preset_combo->setStyleSheet(QString("background-color: %1; color: %2; padding: 3px; font-weight: bold;").arg(Theme::BG_INPUT, Theme::TEXT_PRIMARY));
        auto* synth_btn = makeActionBtn("Synthesize H(z)", Theme::ACCENT_PID);
        shdr->addWidget(st); shdr->addStretch(); shdr->addWidget(preset_combo); shdr->addWidget(synth_btn);
        synth_l->addLayout(shdr);

        auto* sgrid = new QGridLayout();
        cutoff_edit = makeDarkEntry("0.25", 60);
        kd_edit     = makeDarkEntry(QString::number(model->filter_gain), 70);
        ts_edit     = makeDarkEntry(QString::number(model->ts), 60);
        tp_re_edit  = makeDarkEntry(QString::number(model->z_test_point.real()), 70);
        tp_im_edit  = makeDarkEntry(QString::number(model->z_test_point.imag()), 70);

        auto addCell = [&](int r, int c, const QString& lbl, QWidget* w) {
            auto* l = new QLabel(lbl); l->setStyleSheet("color: #94a3b8; font-size: 10px; font-weight: bold; border:none;");
            sgrid->addWidget(l, r, c * 2); sgrid->addWidget(w, r, c * 2 + 1);
        };
        addCell(0, 0, "Ωc/π:", cutoff_edit);
        addCell(0, 1, "Gain Kd:", kd_edit);
        addCell(0, 2, "Ts (s):", ts_edit);
        addCell(1, 0, "Re(z):", tp_re_edit);
        addCell(1, 1, "Im(z):", tp_im_edit);
        synth_l->addLayout(sgrid);
        left_v->addWidget(synth_card);

        // 3. Evaluation & Difference Equation Card
        auto* res_card = makeCard();
        auto* res_l = new QVBoxLayout(res_card);
        diff_eq_lbl = new QLabel("y[k] = ...");
        diff_eq_lbl->setStyleSheet(QString("background-color: %1; color: #a7f3d0; font-family: Monospace; font-weight: bold; padding: 8px; border-radius: 4px;")
                                   .arg(Theme::BANNER_OK_BG));
        res_l->addWidget(diff_eq_lbl);

        auto* mgrid = new QGridLayout();
        auto makeM = [&](int r, int c, const QString& t, QLabel*& out) {
            auto* b = makeCard();
            b->setStyleSheet(QString("background-color: %1; border: 1px solid %2; padding: 6px;").arg(Theme::BG_INPUT, Theme::BORDER_INPUT));
            auto* vl = new QVBoxLayout(b);
            auto* tl = new QLabel(t); tl->setStyleSheet("color: #94a3b8; font-size: 10px; font-weight: bold; border:none;");
            out = new QLabel("—"); out->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));
            vl->addWidget(tl); vl->addWidget(out);
            mgrid->addWidget(b, r, c);
        };
        makeM(0, 0, "NET ANGLE ∠H(z)", val_angle);
        makeM(0, 1, "MAGNITUDE |H(z)|", val_mag);
        makeM(1, 0, "RL GAIN K = 1/|H(z)|", val_gain);
        makeM(1, 1, "EQUIV DAMPING ζ / ωn", val_zeta);
        res_l->addLayout(mgrid);
        left_v->addWidget(res_card);
        left_v->addStretch();

        // Right z-Plane Canvas
        auto* right_card = makeCard();
        auto* right_l = new QVBoxLayout(right_card);
        auto* phdr = new QHBoxLayout();
        auto* pt = new QLabel("Complex z-Plane & Unit Circle |z| = 1 Stability Diagram");
        pt->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::TEXT_PRIMARY));
        auto* fit_b = makeActionBtn("Fit View", Theme::TEXT_PRIMARY, Theme::BG_PANEL);
        phdr->addWidget(pt); phdr->addStretch(); phdr->addWidget(fit_b);
        right_l->addLayout(phdr);

        canvas = new ComplexPlaneCanvas(true);
        right_l->addWidget(canvas, 1);

        splitter->addWidget(left_w);
        splitter->addWidget(right_card);
        splitter->setStretchFactor(0, 0);
        splitter->setStretchFactor(1, 1);
        left_w->setMinimumWidth(450);

        connect(solve_btn, &QPushButton::clicked, this, [this]() {
            try {
                auto pn = parsePolynomialInput(num_edit->text().toStdString());
                auto pd = parsePolynomialInput(den_edit->text().toStdString());
                model->z_zeros = pn.roots;
                model->z_poles = pd.roots;
                evaluateCurrent(true);
            } catch (...) {}
        });
        connect(synth_btn, &QPushButton::clicked, this, [this]() {
            double wc = parseNumericInput(cutoff_edit->text().toStdString(), 0.25);
            double ts = parseNumericInput(ts_edit->text().toStdString(), 0.05);
            std::tie(model->z_poles, model->z_zeros, model->filter_gain) =
                DiscreteFilterSimulator::designFilterPreset(preset_combo->currentText().toStdString(), wc, ts, model->poles, model->zeros, model->plant_gain);
            model->ts = ts;
            loadFromModel();
            model->notify("z_plane");
        });
        connect(fit_b, &QPushButton::clicked, this, [this]() { canvas->resetView(); });
        for (auto* ed : {kd_edit, ts_edit, tp_re_edit, tp_im_edit}) {
            connect(ed, &QLineEdit::editingFinished, this, [this]() { evaluateCurrent(true); });
        }
        canvas->onPointSelected = [this](Complex zt) {
            tp_re_edit->setText(QString::number(zt.real(), 'g', 4));
            tp_im_edit->setText(QString::number(zt.imag(), 'g', 4));
            evaluateCurrent(true);
        };

        loadFromModel();
    }

    void loadFromModel() {
        num_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->z_zeros, "z")));
        den_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->z_poles, "z")));
        kd_edit->setText(QString::number(model->filter_gain, 'g', 4));
        ts_edit->setText(QString::number(model->ts, 'g', 4));
        tp_re_edit->setText(QString::number(model->z_test_point.real(), 'g', 4));
        tp_im_edit->setText(QString::number(model->z_test_point.imag(), 'g', 4));
        evaluateCurrent(false);
    }

    void evaluateCurrent(bool notify) {
        model->filter_gain = parseNumericInput(kd_edit->text().toStdString(), 1.0);
        model->ts = std::max(1e-5, parseNumericInput(ts_edit->text().toStdString(), 0.05));
        Complex zt(parseNumericInput(tp_re_edit->text().toStdString(), 0.68),
                   parseNumericInput(tp_im_edit->text().toStdString(), 0.32));
        model->z_test_point = zt;

        Poly nz = polyScale(polyFromRoots(model->z_zeros), Complex(model->filter_gain, 0.0));
        Poly dz = polyFromRoots(model->z_poles);
        auto [b, a] = DiscreteFilterSimulator::toCausalBA(nz, dz);
        diff_eq_lbl->setText(QString::fromStdString(DiscreteFilterSimulator::formatDifferenceEquation(b, a)));

        auto ev = evaluateTransferFunction(model->z_poles, model->z_zeros, zt, model->filter_gain);
        double def = computeAngleDeficiency(ev.angle_deg);
        bool on_rl = ev.at_pole || (std::abs(def) <= model->z_rl_tol_deg);

        val_angle->setText(QString::fromStdString(PRECISION.fmt(ev.angle_deg) + "°"));
        val_mag->setText(QString::fromStdString(PRECISION.fmt(ev.magnitude)));
        val_gain->setText(QString::fromStdString(PRECISION.fmt(ev.rl_gain_k)));

        auto zm = computeDominantZMetrics(model->z_poles, model->ts);
        val_zeta->setText(QString("ζ=%1 | ωn=%2 r/s")
            .arg(QString::fromStdString(PRECISION.fmt(zm.zeta)))
            .arg(QString::fromStdString(PRECISION.fmt(zm.omega_n))));

        canvas->poles = model->z_poles;
        canvas->zeros = model->z_zeros;
        canvas->test_point = zt;
        canvas->is_on_rl = on_rl;
        canvas->update();

        if (notify) model->notify("z_plane");
    }
};

// =============================================================================
// Tab 4: Discrete Time Response & Filter Simulator Tab (With PID Disable Option)
// =============================================================================
class DiscreteTimeResponseTab : public QWidget {
    Q_OBJECT
public:
    SharedSystemModel* model;
    DiscreteSimulationConfig cfg;

    QLineEdit *num_edit, *den_edit, *kp_edit, *ki_edit, *kd_edit;
    QLineEdit *mag_edit, *kd_filt_edit, *ts_edit, *cursor_edit, *nsamp_edit;
    QCheckBox *enable_dpid_chk;
    QComboBox *mode_combo, *sig_combo;
    QLabel *diff_eq_lbl, *m_card1, *m_card2, *m_card3, *m_card4;
    TimeResponseCanvas* canvas;
    bool updating_ui = false;

    explicit DiscreteTimeResponseTab(SharedSystemModel* m, QWidget* parent = nullptr)
        : QWidget(parent), model(m)
    {
        auto* root_layout = new QHBoxLayout(this);
        root_layout->setContentsMargins(0, 0, 0, 0);
        auto* splitter = new QSplitter(Qt::Horizontal, this);
        root_layout->addWidget(splitter);

        auto* left_w = new QWidget();
        auto* left_v = new QVBoxLayout(left_w);
        left_v->setContentsMargins(0, 0, 6, 0);

        // 1. Num(z) / Den(z) Card
        auto* poly_card = makeCard();
        auto* poly_l = new QVBoxLayout(poly_card);
        auto* phdr = new QHBoxLayout();
        auto* t1 = new QLabel("1. Digital Filter  H(z) = Kd · Num(z) / Den(z)");
        t1->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::ACCENT_ASYMP));
        auto* solve_btn = makeActionBtn("Solve ➔", Theme::ACCENT_PID);
        phdr->addWidget(t1); phdr->addStretch(); phdr->addWidget(solve_btn);
        poly_l->addLayout(phdr);

        auto* pgrid = new QGridLayout();
        num_edit = makeDarkEntry();
        den_edit = makeDarkEntry();
        auto* nl = new QLabel("Num(z):"); nl->setStyleSheet("color: #38bdf8; font-family: Monospace; font-weight: bold; border:none;");
        auto* dl = new QLabel("Den(z):"); dl->setStyleSheet("color: #ff5f56; font-family: Monospace; font-weight: bold; border:none;");
        pgrid->addWidget(nl, 0, 0); pgrid->addWidget(num_edit, 0, 1);
        pgrid->addWidget(dl, 0, 2); pgrid->addWidget(den_edit, 0, 3);
        poly_l->addLayout(pgrid);
        left_v->addWidget(poly_card);

        // 2. Discrete PID & Bypass / Mode Card
        auto* pid_card = makeCard();
        auto* pid_l = new QVBoxLayout(pid_card);
        auto* mhdr = new QHBoxLayout();
        auto* t2 = new QLabel("2. Discrete Loop & PID Configuration");
        t2->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::ACCENT_PID));

        // NEW: Explicit Checkbox to Disable Discrete PID Controller
        enable_dpid_chk = new QCheckBox("Enable Discrete PID C(z)");
        enable_dpid_chk->setChecked(false);
        enable_dpid_chk->setStyleSheet(QString("QCheckBox { color: %1; font-weight: bold; border: none; }").arg(Theme::ACCENT_PID));

        mode_combo = new QComboBox();
        mode_combo->addItems({
            "Direct Digital Filter H(z) (No PID)",
            "Closed-Loop (No PID, C(z)=1)",
            "Closed-Loop Discrete PID"
        });
        mode_combo->setStyleSheet(QString("background-color: %1; color: %2; padding: 3px; font-weight: bold;").arg(Theme::BG_INPUT, Theme::TEXT_PRIMARY));

        mhdr->addWidget(t2); mhdr->addStretch(); mhdr->addWidget(enable_dpid_chk); mhdr->addWidget(mode_combo);
        pid_l->addLayout(mhdr);

        auto* ggrid = new QGridLayout();
        kp_edit = makeDarkEntry("1.0", 60);
        ki_edit = makeDarkEntry("0.5", 60);
        kd_edit = makeDarkEntry("0.02", 60);
        sig_combo = new QComboBox();
        sig_combo->addItems({"Step", "Impulse", "Ramp", "Sine"});
        sig_combo->setStyleSheet(mode_combo->styleSheet());
        mag_edit     = makeDarkEntry("1.0", 55);
        kd_filt_edit = makeDarkEntry("0.0955", 60);
        ts_edit      = makeDarkEntry("0.05", 55);
        cursor_edit  = makeDarkEntry("15", 50);
        nsamp_edit   = makeDarkEntry("80", 50);

        auto addC = [&](int r, int c, const QString& lbl, QWidget* w) {
            auto* l = new QLabel(lbl); l->setStyleSheet("color: #94a3b8; font-size: 10px; font-weight: bold; border:none;");
            ggrid->addWidget(l, r * 2, c); ggrid->addWidget(w, r * 2 + 1, c);
        };
        addC(0, 0, "Discrete Kp", kp_edit);
        addC(0, 1, "Discrete Ki", ki_edit);
        addC(0, 2, "Discrete Kd", kd_edit);
        addC(0, 3, "Input r[k]", sig_combo);
        addC(1, 0, "r[k] Mag", mag_edit);
        addC(1, 1, "Filter Kd", kd_filt_edit);
        addC(1, 2, "Ts (s)", ts_edit);
        addC(1, 3, "Cursor k", cursor_edit);
        addC(1, 4, "Samples N", nsamp_edit);
        pid_l->addLayout(ggrid);
        left_v->addWidget(pid_card);

        // 3. Telemetry Card
        auto* tel_card = makeCard();
        auto* tel_l = new QVBoxLayout(tel_card);
        diff_eq_lbl = new QLabel("y[k] = ...");
        diff_eq_lbl->setStyleSheet(QString("background-color: %1; color: #a7f3d0; font-family: Monospace; font-weight: bold; padding: 6px; border-radius: 4px;")
                                   .arg(Theme::BANNER_OK_BG));
        tel_l->addWidget(diff_eq_lbl);

        auto* mgrid = new QGridLayout();
        auto makeM = [&](int r, int c, const QString& t, QLabel*& out) {
            auto* b = makeCard();
            b->setStyleSheet(QString("background-color: %1; border: 1px solid %2; padding: 6px;").arg(Theme::BG_INPUT, Theme::BORDER_INPUT));
            auto* vl = new QVBoxLayout(b);
            auto* tl = new QLabel(t); tl->setStyleSheet("color: #94a3b8; font-size: 9px; font-weight: bold; border:none;");
            out = new QLabel("—"); out->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));
            vl->addWidget(tl); vl->addWidget(out);
            mgrid->addWidget(b, r, c);
        };
        makeM(0, 0, "RISE / SETTLING SAMPLES", m_card1);
        makeM(0, 1, "OVERSHOOT / PEAK & S.S.E.", m_card2);
        makeM(1, 0, "EQUIV DAMPING ζ / ωn & max|z|", m_card3);
        makeM(1, 1, "CURSOR SAMPLE y[k]", m_card4);
        tel_l->addLayout(mgrid);
        left_v->addWidget(tel_card);
        left_v->addStretch();

        // Right Plot Canvas
        auto* right_card = makeCard();
        auto* right_l = new QVBoxLayout(right_card);
        auto* r_hdr = new QHBoxLayout();
        auto* rt = new QLabel("Discrete Time Sequence  y[k]  (L-Click: Select Sample k)");
        rt->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::TEXT_PRIMARY));
        auto* stems_chk = new QCheckBox("Show Stems ●"); stems_chk->setChecked(true);
        stems_chk->setStyleSheet("color: #38bdf8; font-weight: bold;");
        r_hdr->addWidget(rt); r_hdr->addStretch(); r_hdr->addWidget(stems_chk);
        right_l->addLayout(r_hdr);

        canvas = new TimeResponseCanvas(true);
        right_l->addWidget(canvas, 1);

        splitter->addWidget(left_w);
        splitter->addWidget(right_card);
        splitter->setStretchFactor(0, 0);
        splitter->setStretchFactor(1, 1);
        left_w->setMinimumWidth(470);

        connect(solve_btn, &QPushButton::clicked, this, [this]() {
            try {
                auto pn = parsePolynomialInput(num_edit->text().toStdString());
                auto pd = parsePolynomialInput(den_edit->text().toStdString());
                model->z_zeros = pn.roots;
                model->z_poles = pd.roots;
                runSimulation(true);
            } catch (...) {}
        });

        connect(enable_dpid_chk, &QCheckBox::toggled, this, [this](bool checked) {
            if (updating_ui) return;
            updating_ui = true;
            if (checked) mode_combo->setCurrentIndex(2);
            else if (mode_combo->currentIndex() == 2) mode_combo->setCurrentIndex(0);
            updating_ui = false;
            updatePidWidgetsState();
            runSimulation(true);
        });

        connect(mode_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
            if (updating_ui) return;
            updating_ui = true;
            enable_dpid_chk->setChecked(idx == 2);
            updating_ui = false;
            updatePidWidgetsState();
            runSimulation(true);
        });

        for (auto* ed : {kp_edit, ki_edit, kd_edit, mag_edit, kd_filt_edit, ts_edit, cursor_edit, nsamp_edit}) {
            connect(ed, &QLineEdit::editingFinished, this, [this]() { runSimulation(true); });
        }
        connect(sig_combo, &QComboBox::currentTextChanged, this, [this]() { runSimulation(true); });
        connect(stems_chk, &QCheckBox::toggled, this, [this](bool v) { canvas->show_stems = v; canvas->update(); });

        canvas->onCursorChanged = [this](double k_val) {
            cursor_edit->setText(QString::number(static_cast<int>(k_val)));
            runSimulation(false);
        };

        loadFromModel();
    }

    void updatePidWidgetsState() {
        bool pid_on = enable_dpid_chk->isChecked() && (mode_combo->currentIndex() == 2);
        kp_edit->setEnabled(pid_on);
        ki_edit->setEnabled(pid_on);
        kd_edit->setEnabled(pid_on);
    }

    void loadFromModel() {
        updating_ui = true;
        num_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->z_zeros, "z")));
        den_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->z_poles, "z")));
        kd_filt_edit->setText(QString::number(model->filter_gain, 'g', 4));
        ts_edit->setText(QString::number(model->ts, 'g', 4));
        kp_edit->setText(QString::number(model->d_kp, 'g', 4));
        ki_edit->setText(QString::number(model->d_ki, 'g', 4));
        kd_edit->setText(QString::number(model->d_kd, 'g', 4));
        updating_ui = false;
        updatePidWidgetsState();
        runSimulation(false);
    }

    void runSimulation(bool notify) {
        if (updating_ui) return;
        cfg.z_poles = model->z_poles;
        cfg.z_zeros = model->z_zeros;
        cfg.filter_gain = parseNumericInput(kd_filt_edit->text().toStdString(), 0.0955);
        cfg.input_magnitude = parseNumericInput(mag_edit->text().toStdString(), 1.0);
        cfg.ts = std::max(1e-5, parseNumericInput(ts_edit->text().toStdString(), 0.05));

        int m_idx = mode_combo->currentIndex();
        cfg.closed_loop = (m_idx != 0);
        cfg.pid_enabled = (m_idx == 2) && enable_dpid_chk->isChecked();

        cfg.kp = parseNumericInput(kp_edit->text().toStdString(), 1.0);
        cfg.ki = parseNumericInput(ki_edit->text().toStdString(), 0.5);
        cfg.kd = parseNumericInput(kd_edit->text().toStdString(), 0.02);
        cfg.signal_type = sig_combo->currentText().toStdString();
        cfg.num_samples = std::max(10, static_cast<int>(parseNumericInput(nsamp_edit->text().toStdString(), 80)));
        cfg.cursor_k = std::clamp(static_cast<int>(parseNumericInput(cursor_edit->text().toStdString(), 15)), 0, cfg.num_samples - 1);

        auto res = DiscreteFilterSimulator::evaluate(cfg);
        model->last_discrete_result = res;

        if (notify) {
            model->filter_gain = cfg.filter_gain;
            model->ts = cfg.ts;
            model->d_kp = cfg.kp;
            model->d_ki = cfg.ki;
            model->d_kd = cfg.kd;
            model->notify("discrete_sim");
        }

        diff_eq_lbl->setText(QString::fromStdString(res.diff_eq_str));
        m_card1->setText(QString("kr: %1 samples | ks: %2 samples")
            .arg(res.rise_samples.has_value() ? QString::number(res.rise_samples.value()) : "N/A")
            .arg(res.settling_samples.has_value() ? QString::number(res.settling_samples.value()) : ">N"));
        m_card2->setText(QString("%OS: %1% | Peak: %2\nS.S.E.: %3")
            .arg(QString::fromStdString(PRECISION.fmt(res.overshoot_pct)))
            .arg(QString::fromStdString(PRECISION.fmt(res.peak_value)))
            .arg(QString::fromStdString(PRECISION.fmt(res.steady_state_error))));
        m_card3->setText(QString("ζ: %1 | ωn: %2 r/s\nmax|z_p|: %3 (%4)")
            .arg(QString::fromStdString(PRECISION.fmt(res.zeta)))
            .arg(QString::fromStdString(PRECISION.fmt(res.omega_n)))
            .arg(QString::fromStdString(PRECISION.fmt(res.max_pole_mag)))
            .arg(QString::fromStdString(res.stability_state)));
        m_card4->setText(QString("y[%1] = %2  (t = %3s)")
            .arg(res.cursor_k)
            .arg(QString::fromStdString(PRECISION.fmtComplex(res.cursor_y)))
            .arg(QString::fromStdString(PRECISION.fmt(res.cursor_t))));

        canvas->dsim_res = res;
        canvas->show_cl = cfg.closed_loop;
        canvas->update();
    }
};

// =============================================================================
// Tab 5: Desmos-Style Scientific Calculator Tab
// =============================================================================
class ScientificCalculatorTab : public QWidget {
    Q_OBJECT
public:
    SharedSystemModel* model;
    ScientificCalcEngine engine;
    QVBoxLayout* rows_layout;
    std::vector<std::pair<QLineEdit*, QLabel*>> rows;
    QLineEdit* active_edit = nullptr;
    QLabel* live_vars_lbl;

    explicit ScientificCalculatorTab(SharedSystemModel* m, QWidget* parent = nullptr)
        : QWidget(parent), model(m), engine(m)
    {
        auto* root_l = new QHBoxLayout(this);
        root_l->setContentsMargins(0, 0, 0, 0);

        auto* left_card = makeCard();
        auto* left_l = new QVBoxLayout(left_card);

        auto* hdr = new QHBoxLayout();
        auto* title = new QLabel("Desmos-Style Scientific Calculator  •  LaTeX & Formula Sheet Solver");
        title->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 13px; border:none;").arg(Theme::TEXT_PRIMARY));
        auto* deg_btn = makeActionBtn("DEG Mode", Theme::ACCENT_PID);
        auto* add_btn = makeActionBtn("+ Add Line (Enter)", Theme::ACCENT_ZERO);
        hdr->addWidget(title); hdr->addStretch(); hdr->addWidget(deg_btn); hdr->addWidget(add_btn);
        left_l->addLayout(hdr);

        auto* scroll = new QScrollArea();
        scroll->setWidgetResizable(true);
        scroll->setStyleSheet(QString("background-color: %1; border: 1px solid %2;").arg(Theme::BG_INPUT, Theme::BORDER_INPUT));
        auto* container = new QWidget();
        rows_layout = new QVBoxLayout(container);
        rows_layout->setAlignment(Qt::AlignTop);
        scroll->setWidget(container);
        left_l->addWidget(scroll, 3);

        // Keypad Dock
        auto* kgrid = new QGridLayout();
        const std::vector<std::vector<std::pair<QString, QString>>> keys = {
            {{"solve_zeta_from_os(", "solve_zeta_from_os(20)"}, {"solve_os(", "solve_os(0.456)"}, {"solve_Ts(", "solve_Ts(zeta, wn)"}, {"solve_Tp(", "solve_Tp(zeta, wn)"}, {"7","7"}, {"8","8"}, {"9","9"}, {"/","/"}},
            {{"solve_kp_for_os_p(", "solve_kp_for_os_p(20, 5, 6, 1)"}, {"solve_kd_for_os_pd(", "solve_kd_for_os_pd(10, 24.06, 5, 6, 1)"}, {"sqrt(", "sqrt("}, {"^2", "^2"}, {"4","4"}, {"5","5"}, {"6","6"}, {"*","*"}},
            {{"zeta", "zeta"}, {"wn", "wn"}, {"G(s_test)", "G(s_test)"}, {"H_z(z_test)", "H_z(z_test)"}, {"1","1"}, {"2","2"}, {"3","3"}, {"-","-"}},
            {{"pi", "pi"}, {"i", "i"}, {"ln(", "ln("}, {"ans", "ans"}, {"0","0"}, {".","."}, {"="," = "}, {"+","+"}}
        };
        for (size_t r = 0; r < keys.size(); ++r) {
            for (size_t c = 0; c < keys[r].size(); ++c) {
                auto* b = makeActionBtn(keys[r][c].first, (c >= 4) ? Theme::TEXT_PRIMARY : Theme::ACCENT_ZERO, (c >= 4) ? Theme::BG_KEY_NUM : Theme::BG_KEY_FN);
                QString tok = keys[r][c].second;
                connect(b, &QPushButton::clicked, this, [this, tok]() {
                    if (!active_edit && !rows.empty()) active_edit = rows.back().first;
                    if (active_edit) { active_edit->insert(tok); evaluateAll(); }
                });
                kgrid->addWidget(b, static_cast<int>(r), static_cast<int>(c));
            }
        }
        left_l->addLayout(kgrid, 2);

        // Right Sidebar: Live System Context
        auto* right_card = makeCard();
        right_card->setMaximumWidth(380);
        auto* right_l = new QVBoxLayout(right_card);
        auto* rt = new QLabel("Synchronized System Variables & Functions");
        rt->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::ACCENT_PID));
        right_l->addWidget(rt);

        live_vars_lbl = new QLabel();
        live_vars_lbl->setStyleSheet(QString("background-color: %1; color: %2; font-family: Monospace; padding: 8px; border-radius: 4px;")
                                     .arg(Theme::BG_INPUT, Theme::ACCENT_ZERO));
        live_vars_lbl->setWordWrap(true);
        right_l->addWidget(live_vars_lbl);

        auto* help_txt = new QTextEdit();
        help_txt->setReadOnly(true);
        help_txt->setStyleSheet(QString("background-color: %1; color: %2; font-family: Monospace; font-size: 11px; border: 1px solid %3;")
                                .arg(Theme::BG_INPUT, Theme::TEXT_SECONDARY, Theme::BORDER_INPUT));
        help_txt->setPlainText(
            "Predefined Formula Sheet Solvers:\n"
            "• solve_zeta_from_os(OS_pct)\n"
            "• solve_os(zeta)\n"
            "• solve_Ts(zeta, wn)\n"
            "• solve_Tp(zeta, wn)\n"
            "• solve_Tr(zeta, wn)\n"
            "• solve_wn_from_Ts_zeta(Ts, zeta)\n"
            "• solve_wn_from_Tp_zeta(Tp, zeta)\n"
            "• solve_kp_for_os_p(OS, a1, a0, K0)\n"
            "• solve_kd_for_os_pd(OS, Kp, a1, a0, K0)\n"
            "• G(s_test), K_rl(s_test), H_z(z_test)\n"
            "• LaTeX: \\frac{a}{b}, \\sqrt{x}, \\zeta, \\omega_n"
        );
        right_l->addWidget(help_txt, 1);

        root_l->addWidget(left_card, 1);
        root_l->addWidget(right_card);

        connect(add_btn, &QPushButton::clicked, this, [this]() { addRow(""); });
        connect(deg_btn, &QPushButton::clicked, this, [this, deg_btn]() {
            engine.deg_mode = !engine.deg_mode;
            deg_btn->setText(engine.deg_mode ? "DEG Mode" : "RAD Mode");
            evaluateAll();
        });

        addRow("solve_zeta_from_os(20)");
        addRow("solve_kp_for_os_p(20, 5, 6, 1)");
        addRow("solve_kd_for_os_pd(10, 24.06, 5, 6, 1)");
        addRow("G(s_test)");
        evaluateAll();
    }

    void addRow(const QString& init_expr) {
        auto* row_w = new QFrame();
        row_w->setStyleSheet(QString("background-color: %1; border: 1px solid %2; border-radius: 4px; padding: 4px;")
                             .arg(Theme::BG_PANEL_ALT, Theme::BORDER_PANEL));
        auto* hl = new QHBoxLayout(row_w);
        auto* idx_l = new QLabel(QString::number(rows.size() + 1));
        idx_l->setStyleSheet("color: #64748b; font-family: Monospace; font-weight: bold; border:none;");
        auto* ed = makeDarkEntry(init_expr, 240);
        auto* res_l = new QLabel("= —");
        res_l->setStyleSheet(QString("color: %1; font-family: Monospace; font-size: 13px; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));

        hl->addWidget(idx_l);
        hl->addWidget(ed, 1);
        hl->addWidget(res_l);
        rows_layout->addWidget(row_w);
        rows.push_back({ed, res_l});
        active_edit = ed;

        connect(ed, &QLineEdit::textChanged, this, &ScientificCalculatorTab::evaluateAll);
        connect(ed, &QLineEdit::returnPressed, this, [this]() { addRow(""); });
    }

    void evaluateAll() {
        engine.resetConstants();
        engine.syncFromModel();
        for (auto& [ed, lbl] : rows) {
            std::string txt = ed->text().toStdString();
            if (txt.empty()) { lbl->setText(""); continue; }
            auto out = engine.evalLine(txt);
            if (out.ok) {
                lbl->setStyleSheet(QString("color: %1; font-family: Monospace; font-size: 13px; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));
                lbl->setText(QString::fromStdString(out.formatted));
            } else {
                lbl->setStyleSheet(QString("color: %1; font-family: Monospace; font-size: 11px; border:none;").arg(Theme::ACCENT_OPEN));
                lbl->setText(QString::fromStdString("⚠ " + out.error_msg));
            }
        }
        live_vars_lbl->setText(QString(
            "ζ = %1   |   ωn = %2 rad/s\n"
            "Kp = %3  |   Ki = %4  |   Kd = %5\n"
            "s_test = %6   |   z_test = %7"
        ).arg(QString::fromStdString(PRECISION.fmt(engine.vars["zeta"].real())))
         .arg(QString::fromStdString(PRECISION.fmt(engine.vars["wn"].real())))
         .arg(model->kp).arg(model->ki).arg(model->kd)
         .arg(QString::fromStdString(PRECISION.fmtComplex(model->test_point)))
         .arg(QString::fromStdString(PRECISION.fmtComplex(model->z_test_point))));
    }
};

// =============================================================================
// Tab 6: Engineering Formulas & Variable Definitions Cheat Sheet Tab
// =============================================================================
class FormulaCheatSheetTab : public QWidget {
    Q_OBJECT
public:
    SharedSystemModel* model;
    QLabel* live_banner_lbl;

    explicit FormulaCheatSheetTab(SharedSystemModel* m, QWidget* parent = nullptr)
        : QWidget(parent), model(m)
    {
        auto* vl = new QVBoxLayout(this);
        vl->setContentsMargins(0, 0, 0, 0);

        auto* top_card = makeCard();
        auto* tl = new QVBoxLayout(top_card);
        auto* th = new QLabel("Live Synchronized System Equations & Nise Ch. 4 / Ch. 7 Summary");
        th->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::ACCENT_PID));
        live_banner_lbl = new QLabel();
        live_banner_lbl->setStyleSheet(QString("background-color: %1; color: %2; font-family: Monospace; padding: 8px; border-radius: 4px;")
                                       .arg(Theme::BG_INPUT, Theme::ACCENT_ZERO));
        tl->addWidget(th); tl->addWidget(live_banner_lbl);
        vl->addWidget(top_card);

        auto* scroll = new QScrollArea();
        scroll->setWidgetResizable(true);
        scroll->setStyleSheet(QString("background-color: %1; border: 1px solid %2;").arg(Theme::BG_PANEL, Theme::BORDER_PANEL));
        auto* body = new QWidget();
        auto* bl = new QVBoxLayout(body);

        const std::vector<std::tuple<QString, QString, QString>> sections = {
            {"1. First-Order Systems (Nise Sec. 4.3)",
             "G(s) = K / (s + a)   |   T = 1/a   |   Tr (10-90%) = ln(9)/a ≈ 2.2/a   |   Ts (±2%) = 4/a   |   c(t) = c_final · (1 - e^{-at})",
             Theme::ACCENT_PID},
            {"2. Second-Order Systems & Transient Specifications (Nise Sec. 4.4–4.6)",
             "G(s) = ωn² / (s² + 2ζωn s + ωn²)   |   s_{1,2} = -ζωn ± jωn√(1 - ζ²) = -σd ± jωd\n"
             "%OS = 100 · exp(-ζπ / √(1 - ζ²))   |   ζ = -ln(%OS/100) / √(π² + ln²(%OS/100))\n"
             "Tp = π / ωd   |   Ts (±2%) = 4 / (ζωn) = 4 / σd   |   ωn·Tr ≈ 1.76ζ³ - 0.417ζ² + 1.039ζ + 1",
             Theme::ACCENT_LOCUS},
            {"3. Steady-State Error (S.S.E.), Relative Error & System Type (Nise Ch. 7)",
             "e_ss = |M - y_ss| = M / (1 + Kp,pos)   |   %e_ss = (e_ss / M) × 100% (Independent of step amplitude M!)\n"
             "Type 0 (P/PD): e_ss(Step) = M·a0 / (a0 + Kp·K0)   |   Type 1 (PI/PID, Ki > 0): e_ss(Step) = 0, e_ss(Ramp) = M / Kv",
             Theme::ACCENT_POLE},
            {"4. P, PD, PI & PID Controller Synthesis (CNTL-1016 Assignment 4)",
             "• P-Control on K0/(s² + a1·s + a0):  ωn = a1 / (2ζ),   Kp = (ωn² - a0) / K0\n"
             "• PD-Control (Kept Kp, solve Kd):    ωn = √(a0 + K0·Kp),   Kd = (2ζωn - a1) / K0\n"
             "• PD-Control (Kept Kd, solve Kp):    ωn = (a1 + K0·Kd) / (2ζ),   Kp = (ωn² - a0) / K0\n"
             "• PI-Control on 1st-Order K0/(s+a0): ωn = (a0 + K0·Kp) / (2ζ),   Ki = ωn² / K0",
             Theme::ACCENT_ZERO},
            {"5. Root Locus & Discrete z-Domain Digital Filters (Nise Ch. 8 & Ch. 13)",
             "Angle Condition: ∠G(s) = (2k+1)180°   |   Gain: K = 1 / |G(s)|   |   Asymptotes: σa = (∑p - ∑z)/(n-m)\n"
             "Exact s-to-z Mapping: z = e^{s·Ts}   |   Tustin: s = (2/Ts)·(z-1)/(z+1)   |   BIBO Stable iff max|z_p| < 1",
             Theme::ACCENT_ASYMP}
        };

        for (const auto& [title, eqs, col] : sections) {
            auto* card = makeCard();
            auto* cl = new QVBoxLayout(card);
            auto* t = new QLabel(title);
            t->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 13px; border:none;").arg(col));
            auto* e = new QLabel(eqs);
            e->setStyleSheet(QString("background-color: %1; color: %2; font-family: Monospace; padding: 8px; border-radius: 4px;")
                             .arg(Theme::BG_INPUT, Theme::TEXT_PRIMARY));
            cl->addWidget(t); cl->addWidget(e);
            bl->addWidget(card);
        }
        bl->addStretch();
        scroll->setWidget(body);
        vl->addWidget(scroll, 1);

        refreshBanner();
    }

    void refreshBanner() {
        std::string num_s = formatPolynomialFromRoots(model->zeros, "s");
        std::string den_s = formatPolynomialFromRoots(model->poles, "s");
        auto dm = computeDominantSMetrics(model->poles);
        live_banner_lbl->setText(QString(
            "G(s) = %1 · (%2) / (%3)   |   Open-Loop ζ = %4, ωn = %5 rad/s   |   Rounding: %6"
        ).arg(model->plant_gain)
         .arg(QString::fromStdString(num_s))
         .arg(QString::fromStdString(den_s))
         .arg(QString::fromStdString(PRECISION.fmt(dm.zeta)))
         .arg(QString::fromStdString(PRECISION.fmt(dm.omega_n)))
         .arg(QString::fromStdString(PRECISION.summaryLabel())));
    }
};

// =============================================================================
// Main Unified Application Window
// =============================================================================
class UnifiedControlSuiteWindow : public QMainWindow {
    Q_OBJECT
public:
    SharedSystemModel model;
    QStackedWidget* stack;
    std::vector<QPushButton*> tab_btns;

    RootLocusTab* rl_tab;
    PIDTimeResponseTab* pid_tab;
    ZPlaneFilterTab* zp_tab;
    DiscreteTimeResponseTab* dsim_tab;
    ScientificCalculatorTab* calc_tab;
    FormulaCheatSheetTab* cheat_tab;

    explicit UnifiedControlSuiteWindow(QWidget* parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("Control Systems, Digital Filter & Scientific Calculator Suite (C++17 / Qt)");
        resize(1440, 860);
        setStyleSheet(QString("QMainWindow { background-color: %1; }").arg(Theme::BG_WINDOW));

        auto* central = new QWidget(this);
        setCentralWidget(central);
        auto* main_v = new QVBoxLayout(central);
        main_v->setContentsMargins(12, 8, 12, 10);

        // Top Navigation Bar
        auto* nav = makeCard();
        nav->setStyleSheet(QString("background-color: %1; border: 1px solid %2; border-radius: 6px; padding: 6px;")
                           .arg(Theme::BG_HEADER, Theme::BORDER_PANEL));
        auto* nav_l = new QHBoxLayout(nav);

        auto* brand = new QLabel("Control Systems & Digital Filter Suite (C++)");
        brand->setStyleSheet(QString("color: %1; font-size: 14px; font-weight: bold; border:none;").arg(Theme::TEXT_PRIMARY));
        nav_l->addWidget(brand);
        nav_l->addSpacing(12);

        const std::vector<QString> tab_names = {
            "1. s-Plane Locus",
            "2. Time & PID Sim",
            "3. z-Plane & Filters",
            "4. Discrete Time Sim",
            "5. Scientific Calc",
            "6. Formulas & Cheat Sheet"
        };

        for (size_t i = 0; i < tab_names.size(); ++i) {
            auto* b = new QPushButton(tab_names[i]);
            b->setCursor(Qt::PointingHandCursor);
            connect(b, &QPushButton::clicked, this, [this, i]() { switchTab(static_cast<int>(i)); });
            nav_l->addWidget(b);
            tab_btns.push_back(b);
        }

        nav_l->addStretch();

        auto* step_round_chk = new QCheckBox("Step-Round");
        step_round_chk->setStyleSheet(QString("color: %1; font-weight: bold;").arg(Theme::ACCENT_PID));
        connect(step_round_chk, &QCheckBox::toggled, this, [this](bool v) {
            PRECISION.step_round = v;
            refreshAllTabs();
        });
        nav_l->addWidget(step_round_chk);
        main_v->addWidget(nav);

        // Stacked Content Area
        stack = new QStackedWidget();
        rl_tab = new RootLocusTab(&model, [this](double k_val) {
            model.pid_enabled = true;
            model.kp = k_val;
            model.ki = 0.0;
            model.kd = 0.0;
            pid_tab->loadFromModel();
            switchTab(1);
        });
        pid_tab   = new PIDTimeResponseTab(&model);
        zp_tab    = new ZPlaneFilterTab(&model);
        dsim_tab  = new DiscreteTimeResponseTab(&model);
        calc_tab  = new ScientificCalculatorTab(&model);
        cheat_tab = new FormulaCheatSheetTab(&model);

        stack->addWidget(rl_tab);
        stack->addWidget(pid_tab);
        stack->addWidget(zp_tab);
        stack->addWidget(dsim_tab);
        stack->addWidget(calc_tab);
        stack->addWidget(cheat_tab);
        main_v->addWidget(stack, 1);

        model.listeners.push_back([this](const std::string& src) {
            if (src != "root_locus")   rl_tab->loadFromModel();
            if (src != "pid_sim")      pid_tab->loadFromModel();
            if (src != "z_plane")      zp_tab->loadFromModel();
            if (src != "discrete_sim") dsim_tab->loadFromModel();
            calc_tab->evaluateAll();
            cheat_tab->refreshBanner();
        });

        switchTab(0);
    }

    void switchTab(int idx) {
        stack->setCurrentIndex(idx);
        for (size_t i = 0; i < tab_btns.size(); ++i) {
            bool act = (static_cast<int>(i) == idx);
            tab_btns[i]->setStyleSheet(QString(
                "QPushButton { background-color: %1; color: %2; border: 1px solid %3; border-radius: 4px; padding: 6px 10px; font-weight: bold; font-size: 11px; }"
            ).arg(act ? Theme::BG_KEY_ACCENT : Theme::BG_INPUT,
                  act ? "#ffffff" : Theme::TEXT_SECONDARY,
                  Theme::BORDER_INPUT));
        }
    }

    void refreshAllTabs() {
        rl_tab->loadFromModel();
        pid_tab->loadFromModel();
        zp_tab->loadFromModel();
        dsim_tab->loadFromModel();
        calc_tab->evaluateAll();
        cheat_tab->refreshBanner();
    }
};

// =============================================================================
// CLI Runner & Entry Point
// =============================================================================
int runCli(int argc, char* argv[]) {
    SharedSystemModel model;
    bool do_sim = false, do_dsim = false, no_pid = false;
    std::optional<Complex> test_pt;
    std::vector<std::string> calc_exprs;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        auto nextVal = [&]() -> std::string { return (i + 1 < argc) ? argv[++i] : ""; };

        if (arg == "--num") model.zeros = parsePolynomialInput(nextVal()).roots;
        else if (arg == "--den") model.poles = parsePolynomialInput(nextVal()).roots;
        else if (arg == "--znum") model.z_zeros = parsePolynomialInput(nextVal()).roots;
        else if (arg == "--zden") model.z_poles = parsePolynomialInput(nextVal()).roots;
        else if (arg == "--gain") model.plant_gain = std::stod(nextVal());
        else if (arg == "--rmag") model.input_magnitude = std::stod(nextVal());
        else if (arg == "--kp") model.kp = std::stod(nextVal());
        else if (arg == "--ki") model.ki = std::stod(nextVal());
        else if (arg == "--kd") model.kd = std::stod(nextVal());
        else if (arg == "--no-pid") { no_pid = true; model.pid_enabled = false; model.d_pid_enabled = false; }
        else if (arg == "--simulate") do_sim = true;
        else if (arg == "--dsimulate") do_dsim = true;
        else if (arg == "--point") {
            std::string p = nextVal();
            auto comma = p.find(',');
            double re = std::stod(p.substr(0, comma));
            double im = (comma != std::string::npos) ? std::stod(p.substr(comma + 1)) : 0.0;
            test_pt = Complex(re, im);
        } else if (arg == "--calc") {
            while (i + 1 < argc && argv[i + 1][0] != '-') calc_exprs.push_back(argv[++i]);
        }
    }

    if (test_pt.has_value()) {
        auto ev = evaluateTransferFunction(model.poles, model.zeros, test_pt.value(), model.plant_gain);
        std::cout << "=== 1. s-Plane Root Locus Evaluation ===\n"
                  << "  Test Point s : " << PRECISION.fmtComplex(test_pt.value()) << "\n"
                  << "  Angle ∠G(s)  : " << PRECISION.fmt(ev.angle_deg) << " deg\n"
                  << "  Mag |G(s)|   : " << PRECISION.fmt(ev.magnitude) << "\n"
                  << "  RL Gain K    : " << PRECISION.fmt(ev.rl_gain_k) << "\n";
    }
    if (do_sim) {
        SimulationConfig cfg;
        cfg.poles = model.poles; cfg.zeros = model.zeros;
        cfg.plant_gain = model.plant_gain; cfg.input_magnitude = model.input_magnitude;
        cfg.pid_enabled = !no_pid;
        cfg.kp = model.kp; cfg.ki = model.ki; cfg.kd = model.kd;
        auto res = ComplexTransferFunctionSimulator::evaluate(cfg);
        model.last_sim_result = res;
        std::cout << "=== 2. Continuous Time Response Simulation ===\n"
                  << "  Controller   : " << (cfg.pid_enabled ? "PID Active" : "PID Disabled (C(s) = 1)") << "\n"
                  << "  Stability    : " << res.stability_state << " (" << res.damping_regime << ")\n"
                  << "  Damping ζ    : " << PRECISION.fmt(res.zeta) << " | ωn = " << PRECISION.fmt(res.omega_n) << " rad/s\n"
                  << "  Overshoot    : " << PRECISION.fmt(res.overshoot_pct) << "% | Peak = " << PRECISION.fmt(res.peak_value) << "\n"
                  << "  Settling Ts  : " << PRECISION.fmt(res.settling_time) << " s | Rise Tr = " << PRECISION.fmt(res.rise_time) << " s\n"
                  << "  S.S.E.       : " << PRECISION.fmt(res.steady_state_error) << " (" << PRECISION.fmt(res.relative_error_pct) << "%)\n";
    }
    if (do_dsim) {
        DiscreteSimulationConfig dcfg;
        dcfg.z_poles = model.z_poles; dcfg.z_zeros = model.z_zeros;
        dcfg.filter_gain = model.filter_gain; dcfg.ts = model.ts;
        dcfg.pid_enabled = !no_pid;
        auto dres = DiscreteFilterSimulator::evaluate(dcfg);
        std::cout << "=== 4. Discrete Time Response Simulation ===\n"
                  << "  Diff Eq      : " << dres.diff_eq_str << "\n"
                  << "  Stability    : " << dres.stability_state << " (max|z| = " << PRECISION.fmt(dres.max_pole_mag) << ")\n"
                  << "  S.S.E.       : " << PRECISION.fmt(dres.steady_state_error) << "\n";
    }
    if (!calc_exprs.empty()) {
        ScientificCalcEngine eng(&model);
        std::cout << "=== 5. Scientific Calculator ===\n";
        for (const auto& ex : calc_exprs) {
            auto out = eng.evalLine(ex);
            std::cout << "  " << ex << "  -->  " << (out.ok ? out.formatted : ("ERROR: " + out.error_msg)) << "\n";
        }
    }
    return 0;
}

int main(int argc, char* argv[]) {
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--simulate" || a == "--dsimulate" || a == "--point" || a == "--calc") {
            return runCli(argc, argv);
        }
    }
    QApplication app(argc, argv);
    UnifiedControlSuiteWindow win;
    win.show();
    return app.exec();
}

#include "main.moc"