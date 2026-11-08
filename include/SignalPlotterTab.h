#pragma once
#include "CoreAndWidgets.h"
#include "ScientificCalculatorTab.h"
#include <QTimer>

// =============================================================================
// Fast Pre-Compiled RPN Signal Evaluator
// =============================================================================
class FastSignalEvaluator {
public:
    enum OpType {
        OP_NUM, OP_VAR_T, OP_VAR_X, OP_PARAM,
        OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_POW, OP_NEG, OP_GT, OP_LT,
        OP_SIN, OP_COS, OP_TAN, OP_EXP, OP_LOG, OP_LOG10, OP_SQRT, OP_ABS,
        OP_TANH, OP_SINH, OP_COSH, OP_SIGN, OP_ROUND, OP_FLOOR, OP_CEIL,
        OP_U, OP_DELTA, OP_RECT, OP_TRI, OP_SINC, OP_RAMP,
        OP_MIN, OP_MAX, OP_CLAMP
    };

    struct Inst {
        OpType op;
        double num_val = 0.0;
        int param_idx = -1; // 0..3 -> A,B,C,D;  4..7 -> a0,a1,a2,a3
    };

    std::vector<Inst> code;
    bool valid = true;
    std::string err_msg;

    static int paramIndex(const std::string& s) {
        if (s == "A")  return 0;
        if (s == "B")  return 1;
        if (s == "C")  return 2;
        if (s == "D")  return 3;
        if (s == "a0") return 4;
        if (s == "a1") return 5;
        if (s == "a2") return 6;
        if (s == "a3") return 7;
        if (s == "T")  return 8;
        if (s == "dt") return 9;
        return -1;
    }

    bool compile(const std::string& expr) {
        code.clear();
        valid = true;
        err_msg.clear();

        struct Tok {
            enum Kind { NUM, ID, OP, LPAREN, RPAREN, COMMA } kind;
            std::string text;
            double val = 0.0;
        };

        std::vector<Tok> tokens;
        size_t i = 0, n = expr.size();
        while (i < n) {
            char c = expr[i];
            if (std::isspace(static_cast<unsigned char>(c))) { ++i; continue; }
            if (std::isdigit(static_cast<unsigned char>(c)) || c == '.') {
                size_t start = i;
                bool has_digit = false;
                int dot_count = 0;
                while (i < n && (std::isdigit(static_cast<unsigned char>(expr[i])) || expr[i] == '.')) {
                    if (expr[i] == '.') ++dot_count;
                    else has_digit = true;
                    ++i;
                }
                if (!has_digit || dot_count > 1) {
                    valid = false;
                    err_msg = "Malformed numeric literal '" + expr.substr(start, i - start) + "'";
                    return false;
                }
                if (i < n && (expr[i] == 'e' || expr[i] == 'E')) {
                    ++i;
                    if (i < n && (expr[i] == '+' || expr[i] == '-')) ++i;
                    size_t exp_start = i;
                    while (i < n && std::isdigit(static_cast<unsigned char>(expr[i]))) ++i;
                    if (i == exp_start) {
                        valid = false;
                        err_msg = "Malformed exponent in number";
                        return false;
                    }
                }
                std::string tok_str = expr.substr(start, i - start);
                char* end_ptr = nullptr;
                double v = std::strtod(tok_str.c_str(), &end_ptr);
                if (!end_ptr || *end_ptr != '\0' || !std::isfinite(v)) {
                    valid = false;
                    err_msg = "Invalid number '" + tok_str + "'";
                    return false;
                }
                if (!tokens.empty() && (tokens.back().kind == Tok::NUM || tokens.back().kind == Tok::ID || tokens.back().kind == Tok::RPAREN)) {
                    tokens.push_back({Tok::OP, "*", 0.0});
                }
                tokens.push_back({Tok::NUM, "", v});
                continue;
            }
            if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
                size_t start = i;
                while (i < n && (std::isalnum(static_cast<unsigned char>(expr[i])) || expr[i] == '_')) ++i;
                std::string id = expr.substr(start, i - start);
                if (!tokens.empty() && (tokens.back().kind == Tok::NUM || tokens.back().kind == Tok::ID || tokens.back().kind == Tok::RPAREN)) {
                    tokens.push_back({Tok::OP, "*", 0.0});
                }
                if (id == "pi" || id == "PI") tokens.push_back({Tok::NUM, "", PI});
                else if (id == "e")           tokens.push_back({Tok::NUM, "", std::exp(1.0)});
                else                          tokens.push_back({Tok::ID, id, 0.0});
                continue;
            }
            if (c == '(') {
                if (!tokens.empty() && (tokens.back().kind == Tok::NUM || tokens.back().kind == Tok::RPAREN)) {
                    tokens.push_back({Tok::OP, "*", 0.0});
                }
                tokens.push_back({Tok::LPAREN, "(", 0.0});
                ++i; continue;
            }
            if (c == ')') { tokens.push_back({Tok::RPAREN, ")", 0.0}); ++i; continue; }
            if (c == ',') { tokens.push_back({Tok::COMMA, ",", 0.0}); ++i; continue; }
            if (c == '+' || c == '-' || c == '*' || c == '/' || c == '^' || c == '>' || c == '<') {
                tokens.push_back({Tok::OP, std::string(1, c), 0.0});
                ++i; continue;
            }
            valid = false;
            err_msg = std::string("Unexpected character '") + c + "'";
            return false;
        }

        auto prec = [](const std::string& op) -> int {
            if (op == ">" || op == "<") return 1;
            if (op == "+" || op == "-") return 2;
            if (op == "*" || op == "/") return 3;
            if (op == "u-" || op == "u+") return 4;
            if (op == "^") return 5;
            return 0;
        };
        auto rightAssoc = [](const std::string& op) -> bool {
            return op == "^" || op == "u-" || op == "u+";
        };
        auto emitOp = [&](const std::string& s) -> bool {
            if (s == "+")  { code.push_back({OP_ADD}); return true; }
            if (s == "-")  { code.push_back({OP_SUB}); return true; }
            if (s == "*")  { code.push_back({OP_MUL}); return true; }
            if (s == "/")  { code.push_back({OP_DIV}); return true; }
            if (s == "^")  { code.push_back({OP_POW}); return true; }
            if (s == "u-") { code.push_back({OP_NEG}); return true; }
            if (s == "u+") { return true; }
            if (s == ">")  { code.push_back({OP_GT}); return true; }
            if (s == "<")  { code.push_back({OP_LT}); return true; }
            if (s == "sin")   { code.push_back({OP_SIN}); return true; }
            if (s == "cos")   { code.push_back({OP_COS}); return true; }
            if (s == "tan")   { code.push_back({OP_TAN}); return true; }
            if (s == "exp")   { code.push_back({OP_EXP}); return true; }
            if (s == "ln" || s == "log") { code.push_back({OP_LOG}); return true; }
            if (s == "log10") { code.push_back({OP_LOG10}); return true; }
            if (s == "sqrt")  { code.push_back({OP_SQRT}); return true; }
            if (s == "abs")   { code.push_back({OP_ABS}); return true; }
            if (s == "tanh")  { code.push_back({OP_TANH}); return true; }
            if (s == "sinh")  { code.push_back({OP_SINH}); return true; }
            if (s == "cosh")  { code.push_back({OP_COSH}); return true; }
            if (s == "sign" || s == "sgn") { code.push_back({OP_SIGN}); return true; }
            if (s == "round") { code.push_back({OP_ROUND}); return true; }
            if (s == "floor") { code.push_back({OP_FLOOR}); return true; }
            if (s == "ceil")  { code.push_back({OP_CEIL}); return true; }
            if (s == "u" || s == "step") { code.push_back({OP_U}); return true; }
            if (s == "delta" || s == "imp" || s == "dirac") { code.push_back({OP_DELTA}); return true; }
            if (s == "rect" || s == "box") { code.push_back({OP_RECT}); return true; }
            if (s == "tri")   { code.push_back({OP_TRI}); return true; }
            if (s == "sinc")  { code.push_back({OP_SINC}); return true; }
            if (s == "ramp")  { code.push_back({OP_RAMP}); return true; }
            if (s == "min")   { code.push_back({OP_MIN}); return true; }
            if (s == "max")   { code.push_back({OP_MAX}); return true; }
            if (s == "clamp") { code.push_back({OP_CLAMP}); return true; }
            valid = false;
            err_msg = "Unknown function '" + s + "'";
            return false;
        };

        std::vector<std::string> op_stack;
        for (size_t k = 0; k < tokens.size(); ++k) {
            const auto& tk = tokens[k];
            if (tk.kind == Tok::NUM) {
                code.push_back({OP_NUM, tk.val, -1});
            } else if (tk.kind == Tok::ID) {
                if (k + 1 < tokens.size() && tokens[k + 1].kind == Tok::LPAREN) {
                    op_stack.push_back("FN:" + tk.text);
                } else if (tk.text == "t" || tk.text == "n" || tk.text == "k") {
                    code.push_back({OP_VAR_T});
                } else if (tk.text == "x") {
                    code.push_back({OP_VAR_X});
                } else {
                    int pidx = paramIndex(tk.text);
                    if (pidx >= 0) code.push_back({OP_PARAM, 0.0, pidx});
                    else {
                        valid = false;
                        err_msg = "Unknown variable '" + tk.text + "'";
                        return false;
                    }
                }
            } else if (tk.kind == Tok::LPAREN) {
                op_stack.push_back("(");
            } else if (tk.kind == Tok::COMMA) {
                while (!op_stack.empty() && op_stack.back() != "(") {
                    if (!emitOp(op_stack.back())) return false;
                    op_stack.pop_back();
                }
            } else if (tk.kind == Tok::RPAREN) {
                while (!op_stack.empty() && op_stack.back() != "(") {
                    if (!emitOp(op_stack.back())) return false;
                    op_stack.pop_back();
                }
                if (!op_stack.empty() && op_stack.back() == "(") op_stack.pop_back();
                if (!op_stack.empty() && op_stack.back().rfind("FN:", 0) == 0) {
                    std::string fn = op_stack.back().substr(3);
                    op_stack.pop_back();
                    if (!emitOp(fn)) return false;
                }
            } else if (tk.kind == Tok::OP) {
                std::string op = tk.text;
                bool is_unary = (k == 0 || tokens[k - 1].kind == Tok::LPAREN ||
                                 tokens[k - 1].kind == Tok::OP || tokens[k - 1].kind == Tok::COMMA);
                if (is_unary && (op == "-" || op == "+")) op = "u" + op;
                int p1 = prec(op);
                while (!op_stack.empty() && op_stack.back() != "(" && op_stack.back().rfind("FN:", 0) != 0) {
                    int p2 = prec(op_stack.back());
                    if (p2 > p1 || (p2 == p1 && !rightAssoc(op))) {
                        if (!emitOp(op_stack.back())) return false;
                        op_stack.pop_back();
                    } else break;
                }
                op_stack.push_back(op);
            }
        }
        while (!op_stack.empty()) {
            if (op_stack.back() == "(") { valid = false; err_msg = "Mismatched '('"; return false; }
            std::string top = op_stack.back();
            op_stack.pop_back();
            if (top.rfind("FN:", 0) == 0) top = top.substr(3);
            if (!emitOp(top)) return false;
        }

        // Validate stack depth and exact operand arity at compile time:
        int sim_sp = 0;
        for (const auto& ins : code) {
            int pops = 0, pushes = 1;
            switch (ins.op) {
                case OP_NUM: case OP_VAR_T: case OP_VAR_X: case OP_PARAM:
                    pops = 0; pushes = 1; break;
                case OP_ADD: case OP_SUB: case OP_MUL: case OP_DIV:
                case OP_POW: case OP_GT:  case OP_LT:  case OP_MIN: case OP_MAX:
                    pops = 2; pushes = 1; break;
                case OP_CLAMP:
                    pops = 3; pushes = 1; break;
                default: // All unary functions/operators
                    pops = 1; pushes = 1; break;
            }
            if (sim_sp < pops) {
                valid = false;
                err_msg = "Invalid expression or wrong number of function arguments";
                return false;
            }
            sim_sp = sim_sp - pops + pushes;
            if (sim_sp >= 64) {
                valid = false;
                err_msg = "Expression exceeds maximum evaluation stack depth (64)";
                return false;
            }
        }
        if (sim_sp != 1) {
            valid = false;
            err_msg = "Malformed expression (unbalanced arguments or operators)";
            return false;
        }
        return !code.empty();
    }

    double eval(double t_val, double x_val, const double params[10], bool is_discrete) const {
        double stk[64];
        int sp = 0;
        double dt = std::max(1e-6, params[9]);

        for (const auto& ins : code) {
            switch (ins.op) {
                case OP_NUM:   stk[sp++] = ins.num_val; break;
                case OP_VAR_T: stk[sp++] = t_val; break;
                case OP_VAR_X: stk[sp++] = x_val; break;
                case OP_PARAM: stk[sp++] = params[ins.param_idx]; break;
                case OP_ADD:   if (sp >= 2) { stk[sp - 2] += stk[sp - 1]; --sp; } break;
                case OP_SUB:   if (sp >= 2) { stk[sp - 2] -= stk[sp - 1]; --sp; } break;
                case OP_MUL:   if (sp >= 2) { stk[sp - 2] *= stk[sp - 1]; --sp; } break;
                case OP_DIV:   if (sp >= 2) { stk[sp - 2] /= (std::abs(stk[sp - 1]) < 1e-15 ? 1e-15 : stk[sp - 1]); --sp; } break;
                case OP_POW:   if (sp >= 2) { stk[sp - 2] = std::pow(stk[sp - 2], stk[sp - 1]); --sp; } break;
                case OP_GT:    if (sp >= 2) { stk[sp - 2] = (stk[sp - 2] > stk[sp - 1]) ? 1.0 : 0.0; --sp; } break;
                case OP_LT:    if (sp >= 2) { stk[sp - 2] = (stk[sp - 2] < stk[sp - 1]) ? 1.0 : 0.0; --sp; } break;
                case OP_NEG:   if (sp >= 1) stk[sp - 1] = -stk[sp - 1]; break;
                case OP_SIN:   if (sp >= 1) stk[sp - 1] = std::sin(stk[sp - 1]); break;
                case OP_COS:   if (sp >= 1) stk[sp - 1] = std::cos(stk[sp - 1]); break;
                case OP_TAN:   if (sp >= 1) stk[sp - 1] = std::tan(stk[sp - 1]); break;
                case OP_EXP:   if (sp >= 1) stk[sp - 1] = std::exp(std::clamp(stk[sp - 1], -80.0, 80.0)); break;
                case OP_LOG:   if (sp >= 1) stk[sp - 1] = std::log(std::max(1e-15, std::abs(stk[sp - 1]))); break;
                case OP_LOG10: if (sp >= 1) stk[sp - 1] = std::log10(std::max(1e-15, std::abs(stk[sp - 1]))); break;
                case OP_SQRT:  if (sp >= 1) stk[sp - 1] = std::sqrt(std::max(0.0, stk[sp - 1])); break;
                case OP_ABS:   if (sp >= 1) stk[sp - 1] = std::abs(stk[sp - 1]); break;
                case OP_TANH:  if (sp >= 1) stk[sp - 1] = std::tanh(stk[sp - 1]); break;
                case OP_SINH:  if (sp >= 1) stk[sp - 1] = std::sinh(std::clamp(stk[sp - 1], -40.0, 40.0)); break;
                case OP_COSH:  if (sp >= 1) stk[sp - 1] = std::cosh(std::clamp(stk[sp - 1], -40.0, 40.0)); break;
                case OP_SIGN:  if (sp >= 1) stk[sp - 1] = (stk[sp - 1] > 0.0) ? 1.0 : ((stk[sp - 1] < 0.0) ? -1.0 : 0.0); break;
                case OP_ROUND: if (sp >= 1) stk[sp - 1] = std::round(stk[sp - 1]); break;
                case OP_FLOOR: if (sp >= 1) stk[sp - 1] = std::floor(stk[sp - 1]); break;
                case OP_CEIL:  if (sp >= 1) stk[sp - 1] = std::ceil(stk[sp - 1]); break;
                case OP_U:     if (sp >= 1) stk[sp - 1] = (stk[sp - 1] >= -1e-9) ? 1.0 : 0.0; break;
                case OP_RECT:  if (sp >= 1) stk[sp - 1] = (std::abs(stk[sp - 1]) <= 0.5 + 1e-9) ? 1.0 : 0.0; break;
                case OP_TRI:   if (sp >= 1) stk[sp - 1] = std::max(0.0, 1.0 - std::abs(stk[sp - 1])); break;
                case OP_RAMP:  if (sp >= 1) stk[sp - 1] = std::max(0.0, stk[sp - 1]); break;
                case OP_SINC:  if (sp >= 1) {
                    double u = PI * stk[sp - 1];
                    stk[sp - 1] = (std::abs(u) < 1e-8) ? 1.0 : (std::sin(u) / u);
                } break;
                case OP_DELTA: if (sp >= 1) {
                    double u = stk[sp - 1];
                    if (is_discrete) stk[sp - 1] = (std::abs(u) < 0.5) ? 1.0 : 0.0;
                    else             stk[sp - 1] = (1.0 / dt) * std::exp(-PI * (u / dt) * (u / dt));
                } break;
                case OP_MIN:   if (sp >= 2) { stk[sp - 2] = std::min(stk[sp - 2], stk[sp - 1]); --sp; } break;
                case OP_MAX:   if (sp >= 2) { stk[sp - 2] = std::max(stk[sp - 2], stk[sp - 1]); --sp; } break;
                case OP_CLAMP: if (sp >= 3) { stk[sp - 3] = std::clamp(stk[sp - 3], stk[sp - 2], stk[sp - 1]); sp -= 2; } break;
            }
        }
        return (sp > 0 && std::isfinite(stk[sp - 1])) ? stk[sp - 1] : 0.0;
    }

    static std::string toCleanLatex(std::string s) {
        auto repAll = [&](const std::string& from, const std::string& to) {
            size_t p = 0;
            while ((p = s.find(from, p)) != std::string::npos) {
                s.replace(p, from.size(), to);
                p += to.size();
            }
        };
        repAll("2*pi*", "2\\pi ");
        repAll("2*pi", "2\\pi");
        repAll("pi", "\\pi");
        repAll("*", " \\cdot ");
        repAll("sqrt(", "\\sqrt{");
        repAll("exp(", "e^{(");
        repAll("a0", "a_0");
        repAll("a1", "a_1");
        repAll("a2", "a_2");
        repAll("a3", "a_3");
        return s;
    }
};

// =============================================================================
// Oscilloscope & Spectrum Analyzer State Struct
// =============================================================================
struct SignalEvalResult {
    bool is_discrete = false;
    bool nonlin_enabled = false;
    bool spectrum_pending = false;

    // Oscilloscope settings (10 horiz divs x 8 vert divs)
    double time_per_div = 0.2;
    double time_center  = 0.0;
    double ch1_mag_div  = 0.5;   // Input x(t) Volts/div
    double ch1_offset   = 0.0;   // Input x(t) vertical offset
    double ch2_mag_div  = 0.5;   // Output y(t) Volts/div
    double ch2_offset   = 0.0;   // Output y(t) vertical offset

    int num_samples = 1024;

    // ADC Sampling & DAC Reconstruction state
    bool sampling_sim_enabled = false;
    double fs_hz = 8.0;               // Sampling rate in Hz (Ts = 1 / fs_hz)
    int recon_mode = 0;               // 0 = Whittaker-Shannon Sinc, 1 = ZOH Staircase, 2 = FOH Linear
    std::vector<double> t_samples;    // Discrete sample instants k * Ts
    std::vector<double> y_samples;    // Sampled values y(k * Ts)
    std::vector<double> y_recon;      // Reconstructed continuous waveform y_hat(t)
    std::vector<Complex> Y_recon_spec;// Spectrum of reconstructed waveform
    bool is_aliasing = false;
    double aliased_freq = 0.0;

    // Spectrum Analyzer settings
    double sa_center_hz = 7.5;
    double sa_span_hz   = 15.0;
    double sa_max_hz    = 15.0;
    double rbw_hz       = 0.5;   // Effective Resolution Bandwidth (df)
    double acq_duration = 2.0;   // Effective time capture window used for spectrum

    std::vector<double> t_grid;
    std::vector<Complex> x_in;
    std::vector<Complex> y_out;

    std::vector<double> freq_grid;
    std::vector<Complex> X_spec;
    std::vector<Complex> Y_spec;

    std::vector<double> curve_x;
    std::vector<double> curve_y;

    double energy_x = 0.0, power_x = 0.0;
    double energy_y = 0.0, power_y = 0.0;
    double dc_y = 0.0, rms_y = 0.0;
    double fund_freq = 0.0, fund_mag = 0.0;
    double thd_pct = 0.0;
    std::string error_msg;
};

// =============================================================================
// Dual Oscilloscope (Top) + Spectrum Analyzer (Bottom) Canvas
// =============================================================================
class DualSignalCanvas : public QWidget {
public:
    SignalEvalResult data;
    bool show_input_overlay = true;
    bool show_even_odd = false;
    bool show_transfer_inset = true;
    bool mag_in_db = false;
    bool show_phase = false;

    double cursor_time = 0.0;
    double cursor_freq = 2.0;
    std::function<void(double, double)> onCursorsChanged;

    explicit DualSignalCanvas(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumSize(540, 480);
        setMouseTracking(true);
    }

protected:
    void mousePressEvent(QMouseEvent* ev) override { updateCursorFromMouse(ev->pos()); }
    void mouseMoveEvent(QMouseEvent* ev) override {
        if (ev->buttons() & Qt::LeftButton) updateCursorFromMouse(ev->pos());
    }

    void updateCursorFromMouse(const QPoint& pos) {
        double pad_l = 64.0, pad_r = 24.0, pad_t = 30.0, pad_b = 34.0, gap_mid = 48.0;
        double w = width(), h = height();
        double ew = std::max(10.0, w - pad_l - pad_r);
        double eh_each = std::max(20.0, (h - pad_t - pad_b - gap_mid) * 0.5);

        double frac_x = std::clamp((pos.x() - pad_l) / ew, 0.0, 1.0);
        if (pos.y() < pad_t + eh_each + gap_mid * 0.5) {
            if (!data.t_grid.empty()) {
                double t0 = data.t_grid.front(), t1 = data.t_grid.back();
                cursor_time = t0 + frac_x * (t1 - t0);
                if (data.is_discrete) cursor_time = std::round(cursor_time);
            }
        } else {
            if (!data.freq_grid.empty()) {
                double f0 = data.freq_grid.front(), f1 = data.freq_grid.back();
                cursor_freq = f0 + frac_x * (f1 - f0);
            }
        }
        if (onCursorsChanged) onCursorsChanged(cursor_time, cursor_freq);
        update();
    }

    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.fillRect(rect(), QColor(Theme::BG_PLOT));

        double w = width(), h = height();
        double pad_l = 64.0, pad_r = 24.0, pad_t = 30.0, pad_b = 34.0, gap_mid = 48.0;
        double ew = std::max(10.0, w - pad_l - pad_r);
        double eh = std::max(20.0, (h - pad_t - pad_b - gap_mid) * 0.5);

        QRectF rect_scope(pad_l, pad_t, ew, eh);
        QRectF rect_sa(pad_l, pad_t + eh + gap_mid, ew, eh);

        drawOscilloscopePane(p, rect_scope);
        drawSpectrumAnalyzerPane(p, rect_sa);
    }

private:
    void drawOscilloscopePane(QPainter& p, const QRectF& r) {
        if (data.t_grid.empty()) return;

        // 10 Horizontal Divisions x 8 Vertical Divisions (Standard DSO Graticule)
        const int NX_DIV = 10;
        const int NY_DIV = 8;
        double t_min = data.t_grid.front();
        double t_max = data.t_grid.back();
        if (std::abs(t_max - t_min) < 1e-12) t_max = t_min + 1.0;

        double ch1_div = std::max(1e-6, data.ch1_mag_div);
        double ch2_div = std::max(1e-6, data.ch2_mag_div);

        p.fillRect(r, QColor(Theme::BG_INPUT));
        p.setFont(QFont("Monospace", 8));

        // Vertical graticule lines (10 Time Divisions)
        for (int i = 0; i <= NX_DIV; ++i) {
            double frac = double(i) / NX_DIV;
            double x_px = r.left() + frac * r.width();
            bool is_center = (i == NX_DIV / 2);
            p.setPen(QPen(QColor(is_center ? Theme::AXIS_LINE : Theme::GRID_MAJOR),
                          is_center ? 1.4 : 1.0,
                          is_center ? Qt::SolidLine : Qt::DotLine));
            p.drawLine(QPointF(x_px, r.top()), QPointF(x_px, r.bottom()));

            if (i % 2 == 0) {
                double val = t_min + frac * (t_max - t_min);
                p.setPen(QColor(Theme::TEXT_SECONDARY));
                p.drawText(QRectF(x_px - 32, r.bottom() + 3, 64, 16), Qt::AlignCenter,
                           QString::number(std::abs(val) < 1e-9 ? 0.0 : val, 'g', 4));
            }
        }

        // Horizontal graticule lines (8 Vertical Divisions, -4 div to +4 div from screen center)
        for (int j = 0; j <= NY_DIV; ++j) {
            double frac = double(j) / NY_DIV;
            double y_px = r.bottom() - frac * r.height();
            bool is_center = (j == NY_DIV / 2);
            p.setPen(QPen(QColor(is_center ? Theme::AXIS_LINE : Theme::GRID_MAJOR),
                          is_center ? 1.4 : 1.0,
                          is_center ? Qt::SolidLine : Qt::DotLine));
            p.drawLine(QPointF(r.left(), y_px), QPointF(r.right(), y_px));

            // Show CH2 (primary output) voltage/value scale on left axis
            if (j % 2 == 0) {
                double div_from_mid = (j - NY_DIV * 0.5);
                double ch2_val = div_from_mid * ch2_div - data.ch2_offset;
                p.setPen(QColor(data.nonlin_enabled ? Theme::ACCENT_PID : Theme::ACCENT_ZERO));
                p.drawText(QRectF(2, y_px - 8, r.left() - 18, 16), Qt::AlignRight | Qt::AlignVCenter,
                           QString::number(std::abs(ch2_val) < 1e-9 ? 0.0 : ch2_val, 'g', 3));
            }
        }

        // Center crosshair minor tick marks (like a Tektronix/Keysight scope)
        double mid_x = r.center().x(), mid_y = r.center().y();
        p.setPen(QPen(QColor(Theme::AXIS_LINE), 1.0));
        for (int k = 0; k <= 50; ++k) {
            double tx = r.left() + (double(k) / 50.0) * r.width();
            p.drawLine(QPointF(tx, mid_y - 2.5), QPointF(tx, mid_y + 2.5));
        }
        for (int k = 0; k <= 40; ++k) {
            double ty = r.top() + (double(k) / 40.0) * r.height();
            p.drawLine(QPointF(mid_x - 2.5, ty), QPointF(mid_x + 2.5, ty));
        }

        p.setPen(QPen(QColor(Theme::BORDER_PANEL), 1.2));
        p.drawRect(r);

        // Map functions: screen spans [-4, +4] divisions around vertical center
        // A signal value `v` with offset `off` sits at `div = (v + off) / mag_per_div` divisions above center
        auto mapX = [&](double t) { return r.left() + ((t - t_min) / (t_max - t_min)) * r.width(); };
        auto mapY_CH1 = [&](double v) {
            double div = (v + data.ch1_offset) / ch1_div;
            return r.center().y() - (div / double(NY_DIV)) * r.height();
        };
        auto mapY_CH2 = [&](double v) {
            double div = (v + data.ch2_offset) / ch2_div;
            return r.center().y() - (div / double(NY_DIV)) * r.height();
        };

        // Draw Channel Ground/Offset Reference Markers (1▶ and 2▶) on left bezel
        p.setFont(QFont("Monospace", 8, QFont::Bold));
        if (show_input_overlay && data.nonlin_enabled) {
            double y_gnd1 = std::clamp(mapY_CH1(0.0), r.top() + 6.0, r.bottom() - 6.0);
            p.setPen(QColor(Theme::ACCENT_ZERO));
            p.drawText(QRectF(r.left() - 16, y_gnd1 - 7, 16, 14), Qt::AlignRight | Qt::AlignVCenter, "1▶");
        }
        double y_gnd2 = std::clamp(mapY_CH2(0.0), r.top() + 6.0, r.bottom() - 6.0);
        p.setPen(QColor(data.nonlin_enabled ? Theme::ACCENT_PID : Theme::ACCENT_ZERO));
        p.drawText(QRectF(r.left() - 16, y_gnd2 - 7, 16, 14), Qt::AlignRight | Qt::AlignVCenter, "2▶");

        // Top Oscilloscope Readout Bar (CH1, CH2, Time/div, Pos)
        p.setFont(QFont("Monospace", 8, QFont::Bold));
        QString scope_hdr = QString("OSCILLOSCOPE   •   Horiz: %1%2/div (Center: %3%2)   •   CH1(x): %4/div (Off: %5)   •   CH2(y): %6/div (Off: %7)")
            .arg(QString::number(data.time_per_div, 'g', 3))
            .arg(data.is_discrete ? " samp" : "s")
            .arg(QString::number(data.time_center, 'g', 3))
            .arg(QString::number(ch1_div, 'g', 3))
            .arg(QString::number(data.ch1_offset, 'g', 3))
            .arg(QString::number(ch2_div, 'g', 3))
            .arg(QString::number(data.ch2_offset, 'g', 3));
        p.setPen(QColor(Theme::TEXT_PRIMARY));
        p.drawText(QRectF(r.left(), r.top() - 20, r.width(), 18), Qt::AlignLeft | Qt::AlignVCenter, scope_hdr);

        p.setPen(QColor(Theme::TEXT_SECONDARY));
        p.drawText(QRectF(r.left(), r.bottom() + 16, r.width(), 16), Qt::AlignCenter,
                   data.is_discrete ? "Sample Index  n  (10 Divs)" : "Time  t (seconds, 10 Divs)");

        p.save();
        p.setClipRect(r);
        size_t N = data.t_grid.size();

        if (show_even_odd && N > 1) {
            QPainterPath path_even, path_odd;
            for (size_t i = 0; i < N; ++i) {
                double t_val = data.t_grid[i];
                double y_pos = data.y_out[i].real();
                double y_neg = 0.0;
                if (-t_val >= t_min && -t_val <= t_max) {
                    double frac = (-t_val - t_min) / (t_max - t_min);
                    size_t idx_neg = std::clamp<size_t>(std::llround(frac * (N - 1)), 0, N - 1);
                    y_neg = data.y_out[idx_neg].real();
                }
                double px = mapX(t_val);
                if (i == 0) {
                    path_even.moveTo(px, mapY_CH2(0.5 * (y_pos + y_neg)));
                    path_odd.moveTo(px,  mapY_CH2(0.5 * (y_pos - y_neg)));
                } else {
                    path_even.lineTo(px, mapY_CH2(0.5 * (y_pos + y_neg)));
                    path_odd.lineTo(px,  mapY_CH2(0.5 * (y_pos - y_neg)));
                }
            }
            p.setPen(QPen(QColor(Theme::ACCENT_OPEN), 1.3, Qt::DashLine));
            p.drawPath(path_even);
            p.setPen(QPen(QColor(Theme::ACCENT_LOCUS), 1.3, Qt::DotLine));
            p.drawPath(path_odd);
        }

        if (!data.is_discrete) {
            if (data.nonlin_enabled && show_input_overlay) {
                QPainterPath path_in;
                for (size_t i = 0; i < N; ++i) {
                    double px = mapX(data.t_grid[i]), py = mapY_CH1(data.x_in[i].real());
                    if (i == 0) path_in.moveTo(px, py); else path_in.lineTo(px, py);
                }
                p.setPen(QPen(QColor(Theme::ACCENT_ZERO), 1.5, Qt::DashLine));
                p.drawPath(path_in);
            }
            QPainterPath path_out;
            for (size_t i = 0; i < N; ++i) {
                double px = mapX(data.t_grid[i]), py = mapY_CH2(data.y_out[i].real());
                if (i == 0) path_out.moveTo(px, py); else path_out.lineTo(px, py);
            }
            p.setPen(QPen(QColor(data.nonlin_enabled ? Theme::ACCENT_PID : Theme::ACCENT_ZERO), 2.1));
            p.drawPath(path_out);

            // Overlay ADC Sample Dots & Reconstructed Signal y_hat(t)
            if (data.sampling_sim_enabled && data.y_recon.size() == N) {
                QPainterPath path_rec;
                for (size_t i = 0; i < N; ++i) {
                    double px = mapX(data.t_grid[i]), py = mapY_CH2(data.y_recon[i]);
                    if (i == 0) path_rec.moveTo(px, py); else path_rec.lineTo(px, py);
                }
                p.setPen(QPen(QColor(Theme::ACCENT_OPEN), 2.2)); // Bright gold reconstructed wave
                p.drawPath(path_rec);

                // Draw discrete ADC sample points (k * Ts) with vertical sample stems
                double y0_ch2 = mapY_CH2(0.0);
                p.setPen(QPen(QColor(Theme::ACCENT_OPEN), 1.0, Qt::DotLine));
                for (size_t k = 0; k < data.t_samples.size(); ++k) {
                    if (data.t_samples[k] < t_min || data.t_samples[k] > t_max) continue;
                    double sx = mapX(data.t_samples[k]);
                    double sy = mapY_CH2(data.y_samples[k]);
                    p.drawLine(QPointF(sx, y0_ch2), QPointF(sx, sy));
                }
                p.setPen(QPen(QColor(Theme::BG_PLOT), 1.2));
                p.setBrush(QColor(Theme::ACCENT_OPEN));
                for (size_t k = 0; k < data.t_samples.size(); ++k) {
                    if (data.t_samples[k] < t_min || data.t_samples[k] > t_max) continue;
                    double sx = mapX(data.t_samples[k]);
                    double sy = mapY_CH2(data.y_samples[k]);
                    p.drawEllipse(QPointF(sx, sy), 4.2, 4.2);
                }
            }

        } else {
            double y0_ch1 = mapY_CH1(0.0);
            double y0_ch2 = mapY_CH2(0.0);
            for (size_t i = 0; i < N; ++i) {
                double px = mapX(data.t_grid[i]);
                if (data.nonlin_enabled && show_input_overlay) {
                    double py_in = mapY_CH1(data.x_in[i].real());
                    p.setPen(QPen(QColor(Theme::ACCENT_ZERO), 1.2, Qt::DashLine));
                    p.drawLine(QPointF(px - 2, y0_ch1), QPointF(px - 2, py_in));
                    p.setBrush(QColor(Theme::ACCENT_ZERO));
                    p.drawEllipse(QPointF(px - 2, py_in), 3.0, 3.0);
                }
                double py_out = mapY_CH2(data.y_out[i].real());
                QColor col = QColor(data.nonlin_enabled ? Theme::ACCENT_PID : Theme::ACCENT_ZERO);
                p.setPen(QPen(col, 1.8));
                p.drawLine(QPointF(px, y0_ch2), QPointF(px, py_out));
                p.setBrush(col);
                p.drawEllipse(QPointF(px, py_out), 3.8, 3.8);
            }
        }

        if (cursor_time >= t_min && cursor_time <= t_max) {
            double cx = mapX(cursor_time);
            p.setPen(QPen(QColor(Theme::ACCENT_OPEN), 1.3, Qt::DashDotLine));
            p.drawLine(QPointF(cx, r.top()), QPointF(cx, r.bottom()));
        }

        if (data.nonlin_enabled && show_transfer_inset && !data.curve_x.empty()) {
            double iw = 118.0, ih = 92.0;
            QRectF ir(r.right() - iw - 10, r.top() + 8, iw, ih);
            p.fillRect(ir, QColor(13, 17, 26, 230));
            p.setPen(QPen(QColor(Theme::BORDER_FOCUS), 1.0));
            p.drawRect(ir);

            p.setPen(QPen(QColor(Theme::AXIS_LINE), 1, Qt::DotLine));
            p.drawLine(QPointF(ir.left(), ir.center().y()), QPointF(ir.right(), ir.center().y()));
            p.drawLine(QPointF(ir.center().x(), ir.top()), QPointF(ir.center().x(), ir.bottom()));

            p.setPen(QPen(QColor(Theme::TEXT_MUTED), 1, Qt::DashLine));
            p.drawLine(QPointF(ir.left() + 6, ir.bottom() - 6), QPointF(ir.right() - 6, ir.top() + 6));

            double cx_max = std::max(1e-6, std::abs(data.curve_x.back()));
            double cy_max = 1e-6;
            for (double v : data.curve_y) cy_max = std::max(cy_max, std::abs(v));
            double scale_xy = std::max(cx_max, cy_max);

            QPainterPath g_path;
            for (size_t i = 0; i < data.curve_x.size(); ++i) {
                double ux = (data.curve_x[i] / (2.2 * scale_xy)) + 0.5;
                double uy = (data.curve_y[i] / (2.2 * scale_xy)) + 0.5;
                double px = ir.left() + std::clamp(ux, 0.02, 0.98) * ir.width();
                double py = ir.bottom() - std::clamp(uy, 0.02, 0.98) * ir.height();
                if (i == 0) g_path.moveTo(px, py); else g_path.lineTo(px, py);
            }
            p.setPen(QPen(QColor(Theme::ACCENT_PID), 1.8));
            p.drawPath(g_path);

            p.setFont(QFont("Monospace", 7, QFont::Bold));
            p.setPen(QColor(Theme::ACCENT_PID));
            p.drawText(ir.adjusted(4, 2, -4, -2), Qt::AlignTop | Qt::AlignLeft, "y = g(x) Curve");
        }
        p.restore();
    }

    void drawSpectrumAnalyzerPane(QPainter& p, const QRectF& r) {
        if (data.freq_grid.empty()) return;
        double f_min = data.freq_grid.front(), f_max = data.freq_grid.back();
        if (std::abs(f_max - f_min) < 1e-12) f_max = f_min + 1.0;

        size_t M = data.freq_grid.size();
        std::vector<double> mag_in(M, 0.0), mag_out(M, 0.0), phase_out(M, 0.0);

        double max_lin = 1e-6;
        for (size_t i = 0; i < M; ++i) {
            double mi = std::abs(data.X_spec[i]);
            double mo = std::abs(data.Y_spec[i]);
            max_lin = std::max({max_lin, mi, mo});
            mag_in[i]  = mi;
            mag_out[i] = mo;
            phase_out[i] = (mo > 1e-4 * max_lin) ? (std::arg(data.Y_spec[i]) * 180.0 / PI) : 0.0;
        }

        double y_min = 0.0, y_max = 1.15 * max_lin;
        if (mag_in_db) {
            y_min = -80.0;
            y_max = 20.0 * std::log10(std::max(1e-6, max_lin)) + 10.0;
            for (size_t i = 0; i < M; ++i) {
                mag_in[i]  = std::max(y_min, 20.0 * std::log10(std::max(1e-9, mag_in[i])));
                mag_out[i] = std::max(y_min, 20.0 * std::log10(std::max(1e-9, mag_out[i])));
            }
        }

        p.fillRect(r, QColor(Theme::BG_INPUT));
        p.setFont(QFont("Monospace", 8));

        const int NX_DIV = 10, NY_DIV = 5;
        for (int i = 0; i <= NX_DIV; ++i) {
            double frac = double(i) / NX_DIV;
            double x_px = r.left() + frac * r.width();
            bool is_center = (i == NX_DIV / 2);
            p.setPen(QPen(QColor(is_center ? Theme::AXIS_LINE : Theme::GRID_MAJOR),
                          is_center ? 1.4 : 1.0,
                          is_center ? Qt::DashLine : Qt::DotLine));
            p.drawLine(QPointF(x_px, r.top()), QPointF(x_px, r.bottom()));

            if (i % 2 == 0) {
                double val = f_min + frac * (f_max - f_min);
                p.setPen(QColor(Theme::TEXT_SECONDARY));
                p.drawText(QRectF(x_px - 32, r.bottom() + 3, 64, 16), Qt::AlignCenter,
                           QString::number(std::abs(val) < 1e-9 ? 0.0 : val, 'g', 4));
            }
        }

        for (int j = 0; j <= NY_DIV; ++j) {
            double frac = double(j) / NY_DIV;
            double y_px = r.bottom() - frac * r.height();
            p.setPen(QPen(QColor(Theme::GRID_MAJOR), 1, Qt::DotLine));
            p.drawLine(QPointF(r.left(), y_px), QPointF(r.right(), y_px));

            double val = y_min + frac * (y_max - y_min);
            p.setPen(QColor(Theme::TEXT_SECONDARY));
            p.drawText(QRectF(2, y_px - 8, r.left() - 6, 16), Qt::AlignRight | Qt::AlignVCenter,
                       QString::number(std::abs(val) < 1e-9 ? 0.0 : val, 'g', 3));
        }

        p.setPen(QPen(QColor(Theme::BORDER_PANEL), 1.2));
        p.drawRect(r);

        QString f_unit = data.is_discrete ? "×π rad/s" : "Hz";
        QString sa_hdr = QString("SPECTRUM ANALYZER   •   Center: %1 %2   •   Span: %3 %2   •   RBW (Δf): %4 %2   •   T_acq: %5%6")
            .arg(QString::number(0.5 * (f_min + f_max), 'g', 4))
            .arg(f_unit)
            .arg(QString::number(f_max - f_min, 'g', 4))
            .arg(QString::number(data.rbw_hz, 'g', 3))
            .arg(QString::number(data.acq_duration, 'g', 3))
            .arg(data.is_discrete ? " samp" : "s");
        if (data.spectrum_pending) sa_hdr += "   [⟳ Sweeping...]";

        p.setFont(QFont("Monospace", 8, QFont::Bold));
        p.setPen(QColor(Theme::TEXT_PRIMARY));
        p.drawText(QRectF(r.left(), r.top() - 20, r.width(), 18), Qt::AlignLeft | Qt::AlignVCenter, sa_hdr);

        p.setPen(QColor(Theme::TEXT_SECONDARY));
        QString x_lbl = data.is_discrete
            ? "Normalized Digital Frequency  Ω / π  (rad/sample)"
            : QString("Frequency  f (Hz)   [Start: %1 Hz  |  Center: %2 Hz  |  Stop/Max: %3 Hz]")
                  .arg(QString::number(f_min, 'g', 4))
                  .arg(QString::number(0.5 * (f_min + f_max), 'g', 4))
                  .arg(QString::number(f_max, 'g', 4));
        p.drawText(QRectF(r.left(), r.bottom() + 16, r.width(), 16), Qt::AlignCenter, x_lbl);

        auto mapX = [&](double f) { return r.left() + ((f - f_min) / (f_max - f_min)) * r.width(); };
        auto mapY = [&](double v) { return r.bottom() - ((v - y_min) / (y_max - y_min)) * r.height(); };

        p.save();
        p.setClipRect(r);

        if (data.nonlin_enabled && show_input_overlay) {
            QPainterPath p_in;
            for (size_t i = 0; i < M; ++i) {
                double px = mapX(data.freq_grid[i]), py = mapY(mag_in[i]);
                if (i == 0) p_in.moveTo(px, py); else p_in.lineTo(px, py);
            }
            p.setPen(QPen(QColor(Theme::ACCENT_ZERO), 1.4, Qt::DashLine));
            p.drawPath(p_in);
        }

        if (show_phase) {
            QPainterPath p_ph;
            for (size_t i = 0; i < M; ++i) {
                double px = mapX(data.freq_grid[i]);
                double py = r.bottom() - ((phase_out[i] + 180.0) / 360.0) * r.height();
                if (i == 0) p_ph.moveTo(px, py); else p_ph.lineTo(px, py);
            }
            p.setPen(QPen(QColor(Theme::ACCENT_IMAG), 1.2, Qt::DotLine));
            p.drawPath(p_ph);
        }

        QPainterPath p_out, p_fill;
        double base_y = mapY(y_min);
        for (size_t i = 0; i < M; ++i) {
            double px = mapX(data.freq_grid[i]), py = mapY(mag_out[i]);
            if (i == 0) {
                p_out.moveTo(px, py);
                p_fill.moveTo(px, base_y);
                p_fill.lineTo(px, py);
            } else {
                p_out.lineTo(px, py);
                p_fill.lineTo(px, py);
            }
        }
        p_fill.lineTo(mapX(data.freq_grid.back()), base_y);
        p_fill.closeSubpath();

        QColor main_col = QColor(data.nonlin_enabled ? Theme::ACCENT_PID : Theme::ACCENT_LOCUS);
        if (data.spectrum_pending) main_col.setAlpha(150);
        QColor fill_col = main_col;
        fill_col.setAlpha(35);
        p.fillPath(p_fill, fill_col);
        p.setPen(QPen(main_col, 2.0));
        p.drawPath(p_out);

        // In Discrete Mode, overlay the N-point DFT bin markers X[k] at Omega_k / pi = 2*k / N
        if (data.is_discrete && data.num_samples >= 4 && data.num_samples <= 128 && !data.freq_grid.empty()) {
            int N_dft = data.num_samples;
            QColor dft_col = QColor(data.nonlin_enabled ? Theme::ACCENT_PID : Theme::ACCENT_LOCUS);
            p.setPen(QPen(QColor(Theme::BG_PLOT), 1.2));
            p.setBrush(dft_col);
            for (int k = -N_dft; k <= N_dft; ++k) {
                double f_bin = (2.0 * k) / double(N_dft); // Normalized Omega / pi
                if (f_bin < f_min || f_bin > f_max) continue;
                double frac = (f_bin - f_min) / std::max(1e-9, f_max - f_min);
                size_t idx = std::clamp<size_t>(std::llround(frac * (M - 1)), 0, M - 1);
                double px = mapX(f_bin);
                double py = mapY(mag_out[idx]);
                p.drawEllipse(QPointF(px, py), 3.8, 3.8);
            }
        }

        // Overlay Reconstructed Spectrum |Y_hat(f)| and Nyquist Folding Lines (fs/2, fs)
        if (!data.is_discrete && data.sampling_sim_enabled && data.Y_recon_spec.size() == M) {
            QPainterPath p_rec;
            for (size_t i = 0; i < M; ++i) {
                double mr = std::abs(data.Y_recon_spec[i]);
                double val = mag_in_db ? std::max(y_min, 20.0 * std::log10(std::max(1e-9, mr))) : mr;
                double px = mapX(data.freq_grid[i]), py = mapY(val);
                if (i == 0) p_rec.moveTo(px, py); else p_rec.lineTo(px, py);
            }
            p.setPen(QPen(QColor(Theme::ACCENT_OPEN), 1.9, Qt::DashLine));
            p.drawPath(p_rec);

            // Draw Nyquist Limit f_N = fs / 2 and Sampling Rate fs markers
            double f_nyq = 0.5 * data.fs_hz;
            if (f_nyq >= f_min && f_nyq <= f_max) {
                double nx = mapX(f_nyq);
                p.setPen(QPen(QColor(Theme::ACCENT_POLE), 1.5, Qt::DashDotLine));
                p.drawLine(QPointF(nx, r.top()), QPointF(nx, r.bottom()));
                p.setFont(QFont("Monospace", 8, QFont::Bold));
                p.setPen(QColor(Theme::ACCENT_POLE));
                p.drawText(QRectF(nx + 4, r.top() + 4, 130, 14), Qt::AlignLeft,
                           QString("Nyquist fs/2 = %1 Hz").arg(QString::number(f_nyq, 'g', 3)));
            }
            if (data.fs_hz >= f_min && data.fs_hz <= f_max) {
                double sx = mapX(data.fs_hz);
                p.setPen(QPen(QColor(Theme::ACCENT_ASYMP), 1.3, Qt::DotLine));
                p.drawLine(QPointF(sx, r.top()), QPointF(sx, r.bottom()));
                p.setFont(QFont("Monospace", 8, QFont::Bold));
                p.setPen(QColor(Theme::ACCENT_ASYMP));
                p.drawText(QRectF(sx + 4, r.top() + 18, 110, 14), Qt::AlignLeft,
                           QString("fs = %1 Hz").arg(QString::number(data.fs_hz, 'g', 3)));
            }
        }

        if (cursor_freq >= f_min && cursor_freq <= f_max) {
            double cx = mapX(cursor_freq);
            p.setPen(QPen(QColor(Theme::ACCENT_OPEN), 1.3, Qt::DashDotLine));
            p.drawLine(QPointF(cx, r.top()), QPointF(cx, r.bottom()));
        }
        p.restore();
    }
};

// =============================================================================
// Main Tab Widget: SignalPlotterTab
// =============================================================================
class SignalPlotterTab : public QWidget {
public:
    SharedSystemModel* model;

    QRadioButton *cont_radio, *disc_radio;
    QComboBox *preset_combo, *nonlin_preset_combo, *window_combo;
    QLineEdit *x_expr_edit;
    MathFormulaWidget *x_math_w, *y_math_w;

    struct ParamSliderRow {
        std::string name;
        QWidget *row_widget;
        QLineEdit *val_edit, *min_edit, *max_edit;
        QSlider *slider;
        QCheckBox *disc_chk = nullptr;
    };
    QFrame *param_card;
    std::vector<ParamSliderRow> sig_sliders;

    QCheckBox *nonlin_chk;
    QRadioButton *poly_nl_radio, *custom_nl_radio;
    QWidget *nl_sliders_container, *custom_nl_container;
    std::vector<ParamSliderRow> nl_sliders;
    QLineEdit *custom_nl_edit;

    // Oscilloscope Controls (Time/div, Horiz Pos, CH1/CH2 Mag/div & Offset)
    QLineEdit *time_div_edit, *time_pos_edit;
    QLineEdit *ch1_div_edit, *ch1_off_edit;
    QLineEdit *ch2_div_edit, *ch2_off_edit;

    // Spectrum Analyzer Controls (Center, Span, Max, Auto/Manual RBW df)
    QLineEdit *sa_center_edit, *sa_span_edit, *sa_max_edit;
    QCheckBox *auto_rbw_chk;
    QLineEdit *rbw_edit;

    QLineEdit *n_pts_edit;

    // ADC Sampling & DAC Reconstruction Controls
    QCheckBox *samp_sim_chk;
    QLineEdit *fs_edit, *fs_max_edit;
    QSlider *fs_slider;
    QComboBox *recon_mode_combo;

    // Cached evaluators & parameters for decoupled SA acquisition
    FastSignalEvaluator cached_x_prog, cached_y_prog;
    double cached_params[10] = {0};

    QFrame *status_banner;
    QLabel *banner_left, *banner_right;
    QLabel *m_card1, *m_card2, *m_card3, *m_card4;

    DualSignalCanvas *canvas;
    QTimer *spectrum_timer;
    bool updating_ui = false;

    explicit SignalPlotterTab(SharedSystemModel* m, QWidget* parent = nullptr)
        : QWidget(parent), model(m)
    {
        spectrum_timer = new QTimer(this);
        spectrum_timer->setSingleShot(true);
        spectrum_timer->setInterval(90);
        connect(spectrum_timer, &QTimer::timeout, this, &SignalPlotterTab::computeSpectrumNow);

        auto* root_layout = new QHBoxLayout(this);
        root_layout->setContentsMargins(0, 0, 0, 0);
        auto* splitter = new QSplitter(Qt::Horizontal, this);
        root_layout->addWidget(splitter);

        auto* scroll = new QScrollArea();
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        auto* left_w = new QWidget();
        auto* left_v = new QVBoxLayout(left_w);
        left_v->setContentsMargins(0, 0, 6, 0);
        left_v->setSpacing(6);

        // =====================================================================
        // CARD 1: Input Signal x(t) / x[n]
        // =====================================================================
        auto* sig_card = makeCard();
        auto* sig_l = new QVBoxLayout(sig_card);
        sig_l->setContentsMargins(10, 7, 10, 7);
        sig_l->setSpacing(4);

        auto* hdr1 = new QHBoxLayout();
        auto* t1 = new QLabel("1. Input Signal  x(t) / x[n]");
        t1->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));
        cont_radio = new QRadioButton("Continuous x(t)"); cont_radio->setChecked(true);
        disc_radio = new QRadioButton("Discrete x[n]");
        for (auto* rb : {cont_radio, disc_radio}) {
            rb->setStyleSheet(QString("QRadioButton { color: %1; font-weight: bold; font-size: 11px; border:none; }").arg(Theme::TEXT_PRIMARY));
        }
        auto* eval_btn = makeActionBtn("Plot Signal ➔", Theme::ACCENT_PID);
        hdr1->addWidget(t1); hdr1->addStretch(); hdr1->addWidget(cont_radio); hdr1->addWidget(disc_radio); hdr1->addWidget(eval_btn);
        sig_l->addLayout(hdr1);

        auto* eq_grid = new QGridLayout();
        eq_grid->setVerticalSpacing(4);
        auto* pre_lbl = new QLabel("Preset:");
        pre_lbl->setStyleSheet("color: #94a3b8; font-family: Monospace; font-size: 10px; font-weight: bold; border:none;");
        preset_combo = new QComboBox();
        preset_combo->addItems({
            "Single Tone:   A * cos(2*pi*B*t)",
            "Two-Tone (IM3):   A * cos(2*pi*B*t) + C * cos(2*pi*D*t)",
            "AM Modulated:   (1 + A*cos(2*pi*B*t)) * cos(2*pi*C*t)",
            "Damped Causal Sine:   A * exp(-B*t) * cos(2*pi*C*t) * u(t)",
            "Rectangular Pulse:   A * rect(t / B)",
            "Sinc Wave Packet:   A * sinc(B * t)",
            "Square Harmonics:   A*sin(2*pi*B*t) + (A/3)*sin(6*pi*B*t) + (A/5)*sin(10*pi*B*t)",
            "Linear Chirp:   A * cos(2*pi * (B*t + C*t^2))"
        });
        preset_combo->setStyleSheet(QString("background-color: %1; color: %2; padding: 3px; font-weight: bold; font-size: 10px;")
                                    .arg(Theme::BG_INPUT, Theme::TEXT_PRIMARY));

        auto* eq_lbl = new QLabel("x(t) =");
        eq_lbl->setStyleSheet(QString("color: %1; font-family: Monospace; font-size: 11px; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));
        x_expr_edit = makeDarkEntry("A * cos(2*pi*B*t)");
        x_expr_edit->setStyleSheet(x_expr_edit->styleSheet() + "font-size: 12px; padding: 4px;");

        eq_grid->addWidget(pre_lbl, 0, 0); eq_grid->addWidget(preset_combo, 0, 1);
        eq_grid->addWidget(eq_lbl,  1, 0); eq_grid->addWidget(x_expr_edit,  1, 1);
        sig_l->addLayout(eq_grid);

        auto* fn_hint = new QLabel("Use symbols A, B, C, D to auto-spawn sliders • Functions: u(t), delta(t), rect(t), tri(t), sinc(t), ramp(t)");
        fn_hint->setStyleSheet("color: #64748b; font-size: 9px; border:none;");
        sig_l->addWidget(fn_hint);

        x_math_w = new MathFormulaWidget("", QColor(Theme::ACCENT_ZERO), QColor(Theme::BG_INPUT), 10);
        sig_l->addWidget(x_math_w);
        left_v->addWidget(sig_card);

        // =====================================================================
        // CARD 2: Dynamic Signal Parameter Sliders (A, B, C, D)
        // =====================================================================
        param_card = makeCard();
        auto* param_l = new QVBoxLayout(param_card);
        param_l->setContentsMargins(10, 5, 10, 5);
        param_l->setSpacing(2);
        auto* t2 = new QLabel("2. Active Signal Term Sliders  (Auto-detected from Equation)");
        t2->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border:none;").arg(Theme::ACCENT_ASYMP));
        param_l->addWidget(t2);

        struct SliderDef { std::string sym; double val, min_v, max_v; };
        std::vector<SliderDef> defs = {
            {"A", 1.0,  -5.0, 5.0},
            {"B", 2.0,   0.1, 20.0},
            {"C", 0.5,  -5.0, 5.0},
            {"D", 5.0,   0.1, 30.0}
        };

        for (size_t i = 0; i < defs.size(); ++i) {
            ParamSliderRow row;
            row.name = defs[i].sym;
            row.row_widget = new QWidget();
            auto* rl = new QHBoxLayout(row.row_widget);
            rl->setContentsMargins(0, 1, 0, 1);
            rl->setSpacing(5);

            auto* lbl = new QLabel(QString::fromStdString(defs[i].sym + " ="));
            lbl->setFixedWidth(26);
            lbl->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));

            row.val_edit = makeDarkEntry(QString::number(defs[i].val), 52);
            row.val_edit->setFixedWidth(52); // Lock width so slider gets the space!

            row.slider = new QSlider(Qt::Horizontal);
            row.slider->setRange(0, 1000);
            row.slider->setMinimumWidth(110);
            int init_pos = static_cast<int>(std::round(((defs[i].val - defs[i].min_v) / (defs[i].max_v - defs[i].min_v)) * 1000.0));
            row.slider->setValue(std::clamp(init_pos, 0, 1000));

            auto* min_l = new QLabel("Min:");
            min_l->setStyleSheet(QString("color:%1; font-size:9px; border:none;").arg(Theme::TEXT_SECONDARY));
            row.min_edit = makeDarkEntry(QString::number(defs[i].min_v), 38);
            row.min_edit->setFixedWidth(38); // Compact Min box

            auto* max_l = new QLabel("Max:");
            max_l->setStyleSheet(QString("color:%1; font-size:9px; border:none;").arg(Theme::TEXT_SECONDARY));
            row.max_edit = makeDarkEntry(QString::number(defs[i].max_v), 38);
            row.max_edit->setFixedWidth(38); // Compact Max box

            row.disc_chk = new QCheckBox("Int");
            row.disc_chk->setToolTip("Snap slider to discrete integer steps");
            row.disc_chk->setStyleSheet(QString("QCheckBox { color: %1; font-size: 9px; font-weight: bold; border:none; }").arg(Theme::TEXT_PRIMARY));

            rl->addWidget(lbl);
            rl->addWidget(row.val_edit);
            rl->addWidget(row.slider, 1);
            rl->addWidget(min_l); rl->addWidget(row.min_edit);
            rl->addWidget(max_l); rl->addWidget(row.max_edit);
            rl->addWidget(row.disc_chk);

            param_l->addWidget(row.row_widget);
            sig_sliders.push_back(row);
        }
        left_v->addWidget(param_card);

        // =====================================================================
        // CARD 3: Nonlinear Amplifier / Distortion y = g(x)
        // =====================================================================
        auto* nl_card = makeCard();
        auto* nl_l = new QVBoxLayout(nl_card);
        nl_l->setContentsMargins(10, 5, 10, 5);
        nl_l->setSpacing(3);

        nonlin_chk = new QCheckBox("3. Enable Nonlinear Amplifier / Distortion  y = g(x)");
        nonlin_chk->setChecked(true);
        nonlin_chk->setStyleSheet(QString("QCheckBox { color: %1; font-weight: bold; font-size: 11px; border:none; }").arg(Theme::ACCENT_PID));
        nl_l->addWidget(nonlin_chk);

        auto* nl_mode_row = new QHBoxLayout();
        poly_nl_radio   = new QRadioButton("Polynomial: a₀ + a₁x + a₂x² + a₃x³"); poly_nl_radio->setChecked(true);
        custom_nl_radio = new QRadioButton("Custom Nonlinear Model g(x)");
        for (auto* rb : {poly_nl_radio, custom_nl_radio}) {
            rb->setStyleSheet(QString("QRadioButton { color: %1; font-size: 10px; font-weight: bold; border:none; }").arg(Theme::TEXT_PRIMARY));
        }
        nl_mode_row->addWidget(poly_nl_radio); nl_mode_row->addWidget(custom_nl_radio);
        nl_l->addLayout(nl_mode_row);

        nl_sliders_container = new QWidget();
        auto* nl_sv = new QVBoxLayout(nl_sliders_container);
        nl_sv->setContentsMargins(0, 0, 0, 0);
        nl_sv->setSpacing(2);

        std::vector<SliderDef> nl_defs = {
            {"a0", 0.0, -2.0, 2.0},
            {"a1", 1.0, -3.0, 3.0},
            {"a2", 0.0, -2.0, 2.0},
            {"a3", 0.0, -2.0, 2.0}
        };
        const std::vector<QString> nl_desc = {
            "a₀ (DC):", "a₁ (Lin x):", "a₂ (Quad x²):", "a₃ (Cubic x³):"
        };

        for (size_t i = 0; i < nl_defs.size(); ++i) {
            ParamSliderRow row;
            row.name = nl_defs[i].sym;
            row.row_widget = new QWidget();
            auto* rl = new QHBoxLayout(row.row_widget);
            rl->setContentsMargins(0, 1, 0, 1);
            rl->setSpacing(5);

            auto* lbl = new QLabel(nl_desc[i]);
            lbl->setFixedWidth(72);
            lbl->setStyleSheet(QString("color: %1; font-family: Monospace; font-size: 10px; font-weight: bold; border:none;").arg(Theme::ACCENT_PID));

            row.val_edit = makeDarkEntry(QString::number(nl_defs[i].val), 48);
            row.val_edit->setFixedWidth(48);

            row.slider = new QSlider(Qt::Horizontal);
            row.slider->setRange(0, 1000);
            row.slider->setMinimumWidth(100);
            int init_pos = static_cast<int>(std::round(((nl_defs[i].val - nl_defs[i].min_v) / (nl_defs[i].max_v - nl_defs[i].min_v)) * 1000.0));
            row.slider->setValue(std::clamp(init_pos, 0, 1000));

            row.min_edit = makeDarkEntry(QString::number(nl_defs[i].min_v), 36);
            row.min_edit->setFixedWidth(36);

            row.max_edit = makeDarkEntry(QString::number(nl_defs[i].max_v), 36);
            row.max_edit->setFixedWidth(36);

            row.disc_chk = new QCheckBox("Int");
            row.disc_chk->setToolTip("Snap slider to discrete integer steps");
            row.disc_chk->setStyleSheet(QString("QCheckBox { color: %1; font-size: 9px; font-weight: bold; border:none; }").arg(Theme::TEXT_PRIMARY));

            rl->addWidget(lbl);
            rl->addWidget(row.val_edit);
            rl->addWidget(row.slider, 1);
            rl->addWidget(row.min_edit);
            rl->addWidget(row.max_edit);
            rl->addWidget(row.disc_chk);

            nl_sv->addWidget(row.row_widget);
            nl_sliders.push_back(row);
        }
        nl_l->addWidget(nl_sliders_container);

        custom_nl_container = new QWidget();
        auto* c_nl_v = new QVBoxLayout(custom_nl_container);
        c_nl_v->setContentsMargins(0, 2, 0, 0);
        c_nl_v->setSpacing(4);
        auto* c_nl_row = new QHBoxLayout();
        nonlin_preset_combo = new QComboBox();
        nonlin_preset_combo->addItems({
            "Polynomial:  a0 + a1*x + a2*x^2 + a3*x^3",
            "Soft Saturation (Tanh Clipper):  tanh(a1 * x)",
            "Hard Amplifier Clipping (±1):  clamp(a1 * x, -1, 1)",
            "Full-Wave Bridge Rectifier:  abs(x)",
            "Half-Wave Ideal Diode:  max(0, x)",
            "Exponential Shockley Diode:  0.2 * (exp(2*x) - 1)",
            "Deadband / Crossover Distortion:  (abs(x) > 0.25) * (x - 0.25*sign(x))",
            "3-Bit Quantizer (Staircase):  round(x * 4) / 4"
        });
        nonlin_preset_combo->setStyleSheet(preset_combo->styleSheet());
        c_nl_row->addWidget(new QLabel("Model:"));
        c_nl_row->addWidget(nonlin_preset_combo, 1);
        c_nl_v->addLayout(c_nl_row);

        custom_nl_edit = makeDarkEntry("a0 + a1*x + a2*x^2 + a3*x^3");
        c_nl_v->addWidget(custom_nl_edit);
        nl_l->addWidget(custom_nl_container);

        y_math_w = new MathFormulaWidget("", QColor(Theme::ACCENT_PID), QColor(Theme::BG_INPUT), 9);
        nl_l->addWidget(y_math_w);
        left_v->addWidget(nl_card);

        // =====================================================================
        // CARD 4: Oscilloscope & Spectrum Analyzer Front-Panel Controls
        // =====================================================================
        auto* inst_card = makeCard();
        auto* inst_l = new QVBoxLayout(inst_card);
        inst_l->setContentsMargins(10, 6, 10, 6);
        inst_l->setSpacing(5);

        // --- OSCILLOSCOPE SUB-HEADER ---
        auto* scope_hdr = new QHBoxLayout();
        auto* scope_title = new QLabel("4A. Oscilloscope Controls  (10 Horiz × 8 Vert Divs)");
        scope_title->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border:none;").arg(Theme::ACCENT_ZERO));
        auto* auto_scale_btn = makeActionBtn("Auto-Scale Scope", Theme::ACCENT_ZERO, Theme::BG_INPUT);
        scope_hdr->addWidget(scope_title); scope_hdr->addStretch(); scope_hdr->addWidget(auto_scale_btn);
        inst_l->addLayout(scope_hdr);

        auto* scope_grid = new QGridLayout();
        scope_grid->setVerticalSpacing(3);
        scope_grid->setHorizontalSpacing(6);

        time_div_edit = makeDarkEntry("0.2", 52);
        time_pos_edit = makeDarkEntry("0.0", 52);
        ch1_div_edit  = makeDarkEntry("0.5", 52);
        ch1_off_edit  = makeDarkEntry("0.0", 52);
        ch2_div_edit  = makeDarkEntry("0.5", 52);
        ch2_off_edit  = makeDarkEntry("0.0", 52);

        auto addScopeCell = [&](int r, int c, const QString& lbl, QWidget* w, const QString& col = Theme::TEXT_SECONDARY) {
            auto* l = new QLabel(lbl);
            l->setStyleSheet(QString("color: %1; font-size: 9px; font-weight: bold; border:none;").arg(col));
            scope_grid->addWidget(l, r, c * 2);
            scope_grid->addWidget(w, r, c * 2 + 1);
        };
        addScopeCell(0, 0, "Time / Div:",     time_div_edit, Theme::TEXT_PRIMARY);
        addScopeCell(0, 1, "Horiz Center:",   time_pos_edit, Theme::TEXT_PRIMARY);
        addScopeCell(1, 0, "CH1(x) Mag/Div:", ch1_div_edit,  Theme::ACCENT_ZERO);
        addScopeCell(1, 1, "CH1(x) Offset:",  ch1_off_edit,  Theme::ACCENT_ZERO);
        addScopeCell(2, 0, "CH2(y) Mag/Div:", ch2_div_edit,  Theme::ACCENT_PID);
        addScopeCell(2, 1, "CH2(y) Offset:",  ch2_off_edit,  Theme::ACCENT_PID);
        n_pts_edit = makeDarkEntry("1024", 52);
        addScopeCell(0, 2, "Samples N:", n_pts_edit, Theme::TEXT_PRIMARY);
        inst_l->addLayout(scope_grid);

        // Divider line
        auto* sep = new QFrame();
        sep->setFrameShape(QFrame::HLine);
        sep->setStyleSheet(QString("color: %1;").arg(Theme::BORDER_PANEL));
        inst_l->addWidget(sep);

        // --- SPECTRUM ANALYZER SUB-HEADER ---
        auto* sa_hdr = new QHBoxLayout();
        auto* sa_title = new QLabel("4B. Spectrum Analyzer  (Center / Span / Max & Auto RBW Δf)");
        sa_title->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border:none;").arg(Theme::ACCENT_LOCUS));
        window_combo = new QComboBox();
        window_combo->addItems({"Hann Window", "Hamming Window", "Rectangular (Exact)"});
        window_combo->setStyleSheet(preset_combo->styleSheet());
        sa_hdr->addWidget(sa_title); sa_hdr->addStretch(); sa_hdr->addWidget(window_combo);
        inst_l->addLayout(sa_hdr);

        auto* sa_grid = new QGridLayout();
        sa_grid->setVerticalSpacing(3);
        sa_grid->setHorizontalSpacing(5);

        sa_center_edit = makeDarkEntry("7.5",  52);
        sa_span_edit   = makeDarkEntry("15.0", 52);
        sa_max_edit    = makeDarkEntry("15.0", 52);
        auto_rbw_chk   = new QCheckBox("Auto Δf (RBW):");
        auto_rbw_chk->setChecked(true);
        auto_rbw_chk->setStyleSheet(QString("color: %1; font-size: 9px; font-weight: bold; border:none;").arg(Theme::ACCENT_PID));
        rbw_edit = makeDarkEntry("0.25", 52);
        rbw_edit->setEnabled(false);

        auto addSaCol = [&](int c, QWidget* top_w, QWidget* bot_w) {
            sa_grid->addWidget(top_w, 0, c);
            sa_grid->addWidget(bot_w, 1, c);
        };
        auto makeTinyLbl = [](const QString& txt) {
            auto* l = new QLabel(txt);
            l->setStyleSheet(QString("color: %1; font-size: 9px; font-weight: bold; border:none;").arg(Theme::TEXT_SECONDARY));
            return l;
        };
        addSaCol(0, makeTinyLbl("Center Freq (Hz)"), sa_center_edit);
        addSaCol(1, makeTinyLbl("Span (Hz)"),        sa_span_edit);
        addSaCol(2, makeTinyLbl("Max / Stop (Hz)"),  sa_max_edit);
        addSaCol(3, auto_rbw_chk,                    rbw_edit);
        inst_l->addLayout(sa_grid);

        // --- 4C. ADC SAMPLING & DAC RECONSTRUCTION SUB-PANEL ---
        auto* sep2 = new QFrame();
        sep2->setFrameShape(QFrame::HLine);
        sep2->setStyleSheet(QString("color: %1;").arg(Theme::BORDER_PANEL));
        inst_l->addWidget(sep2);

        auto* samp_hdr = new QHBoxLayout();
        samp_sim_chk = new QCheckBox("4C. Sample & Reconstruct Continuous Signal  (ADC f_s & Aliasing Explorer)");
        samp_sim_chk->setChecked(false);
        samp_sim_chk->setStyleSheet(QString("QCheckBox { color: %1; font-weight: bold; font-size: 11px; border:none; }").arg(Theme::ACCENT_OPEN));
        recon_mode_combo = new QComboBox();
        recon_mode_combo->addItems({
            "Ideal Sinc (Whittaker-Shannon)",
            "Zero-Order Hold (ZOH Staircase)",
            "First-Order Hold (Linear Interp)"
        });
        recon_mode_combo->setStyleSheet(preset_combo->styleSheet());
        samp_hdr->addWidget(samp_sim_chk); samp_hdr->addStretch(); samp_hdr->addWidget(recon_mode_combo);
        inst_l->addLayout(samp_hdr);

        auto* fs_row = new QHBoxLayout();
        auto* fs_lbl = new QLabel("f_s (Hz):");
        fs_lbl->setStyleSheet(QString("color: %1; font-family: Monospace; font-size: 10px; font-weight: bold; border:none;").arg(Theme::ACCENT_OPEN));
        fs_edit = makeDarkEntry("8.0", 50);
        fs_slider = new QSlider(Qt::Horizontal);
        fs_slider->setRange(1, 1000);
        fs_slider->setValue(267); // 8 Hz out of 30 Hz default max
        auto* fs_max_lbl = new QLabel("Max f_s:");
        fs_max_lbl->setStyleSheet(QString("color: %1; font-size: 9px; font-weight: bold; border:none;").arg(Theme::TEXT_SECONDARY));
        fs_max_edit = makeDarkEntry("30.0", 42);

        fs_row->addWidget(fs_lbl);
        fs_row->addWidget(fs_edit);
        fs_row->addWidget(fs_slider, 1);
        fs_row->addWidget(fs_max_lbl);
        fs_row->addWidget(fs_max_edit);
        inst_l->addLayout(fs_row);

        // Telemetry Banner & Readout Cards
        status_banner = new QFrame();
        status_banner->setFixedHeight(28);
        auto* sb_l = new QHBoxLayout(status_banner);
        sb_l->setContentsMargins(8, 2, 8, 2);
        banner_left  = new QLabel("✔ DSO & SPECTRUM ANALYZER READY");
        banner_right = new QLabel("THD = 0.00%");
        sb_l->addWidget(banner_left); sb_l->addStretch(); sb_l->addWidget(banner_right);
        inst_l->addWidget(status_banner);

        auto* mgrid = new QGridLayout();
        auto makeTel = [&](int r, int c, const QString& title, QLabel*& out) {
            auto* b = makeCard();
            b->setStyleSheet(QString("background-color: %1; border: 1px solid %2; padding: 4px;").arg(Theme::BG_INPUT, Theme::BORDER_INPUT));
            auto* vl = new QVBoxLayout(b); vl->setContentsMargins(6, 3, 6, 3);
            auto* tl = new QLabel(title); tl->setStyleSheet("color: #94a3b8; font-size: 9px; font-weight: bold; border:none;");
            out = new QLabel("—"); out->setStyleSheet(QString("color: %1; font-family: Monospace; font-size: 10px; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));
            vl->addWidget(tl); vl->addWidget(out);
            mgrid->addWidget(b, r, c);
        };
        makeTel(0, 0, "ENERGY E & AVERAGE POWER P", m_card1);
        makeTel(0, 1, "DC COMPONENT & RMS", m_card2);
        makeTel(1, 0, "FUNDAMENTAL, RBW Δf & THD%", m_card3);
        makeTel(1, 1, "SCOPE & SPECTRUM CURSORS", m_card4);
        inst_l->addLayout(mgrid);
        left_v->addWidget(inst_card);
        left_v->addStretch();

        scroll->setWidget(left_w);

        // =====================================================================
        // Right Dual-Stacked Canvas Panel
        // =====================================================================
        auto* right_card = makeCard();
        auto* right_l = new QVBoxLayout(right_card);
        auto* phdr = new QHBoxLayout();
        auto* pt = new QLabel("Oscilloscope & Spectrum Analyzer  (Click Top: Time Cursor • Click Bottom: Freq Cursor)");
        pt->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 12px; border:none;").arg(Theme::TEXT_PRIMARY));

        auto* chk_in_ov = new QCheckBox("CH1 Input Overlay"); chk_in_ov->setChecked(true); chk_in_ov->setStyleSheet("color: #38bdf8; font-weight: bold;");
        auto* chk_eo    = new QCheckBox("Even/Odd Parts");    chk_eo->setStyleSheet("color: #f59e0b; font-weight: bold;");
        auto* chk_inset = new QCheckBox("y=g(x) Inset");      chk_inset->setChecked(true); chk_inset->setStyleSheet("color: #34d399; font-weight: bold;");
        auto* chk_db    = new QCheckBox("Spectrum in dB");    chk_db->setStyleSheet("color: #a855f7; font-weight: bold;");
        auto* chk_ph    = new QCheckBox("Phase ∠Y(f)");       chk_ph->setStyleSheet("color: #ec4899; font-weight: bold;");

        phdr->addWidget(pt); phdr->addStretch();
        phdr->addWidget(chk_in_ov); phdr->addWidget(chk_eo); phdr->addWidget(chk_inset); phdr->addWidget(chk_db); phdr->addWidget(chk_ph);
        right_l->addLayout(phdr);

        canvas = new DualSignalCanvas();
        attachSvgExportMenu(canvas, "signal_oscilloscope_spectrum.svg");
        right_l->addWidget(canvas, 1);

        splitter->addWidget(scroll);
        splitter->addWidget(right_card);
        splitter->setStretchFactor(0, 0);
        splitter->setStretchFactor(1, 1);
        scroll->setMinimumWidth(510);

        // =====================================================================
        // Wire Up Sliders, Scope Controls & Bidirectional Center/Span/Max SA
        // =====================================================================
        auto wireSliderGroup = [this](std::vector<ParamSliderRow>& rows, bool is_nl_poly) {
            for (size_t i = 0; i < rows.size(); ++i) {
                connect(rows[i].slider, &QSlider::valueChanged, this, [this, &rows, i, is_nl_poly](int v) {
                    if (updating_ui) return;
                    double min_v = parseNumericInput(rows[i].min_edit->text().toStdString(), -5.0);
                    double max_v = parseNumericInput(rows[i].max_edit->text().toStdString(),  5.0);
                    if (std::abs(max_v - min_v) < 1e-9) max_v = min_v + 1.0;
                    double real_v = min_v + (v / 1000.0) * (max_v - min_v);

                    if (rows[i].disc_chk && rows[i].disc_chk->isChecked()) {
                        real_v = std::clamp(std::round(real_v), std::ceil(min_v), std::floor(max_v));
                    } else if (std::abs(real_v) < 1e-3 * (max_v - min_v)) {
                        real_v = 0.0;
                    }

                    updating_ui = true;
                    rows[i].val_edit->setText(QString::number(real_v, 'g', 4));
                    if (is_nl_poly && poly_nl_radio->isChecked()) {
                        custom_nl_edit->setText("a0 + a1*x + a2*x^2 + a3*x^3");
                    }
                    updating_ui = false;
                    evaluateTimeDomain(true);
                });

                auto syncEditToSlider = [this, &rows, i]() {
                    if (updating_ui) return;
                    double val   = parseNumericInput(rows[i].val_edit->text().toStdString(), 0.0);
                    double min_v = parseNumericInput(rows[i].min_edit->text().toStdString(), -5.0);
                    double max_v = parseNumericInput(rows[i].max_edit->text().toStdString(),  5.0);
                    if (std::abs(max_v - min_v) < 1e-9) max_v = min_v + 1.0;

                    if (rows[i].disc_chk && rows[i].disc_chk->isChecked()) {
                        val = std::round(val);
                    }
                    int pos = static_cast<int>(std::round(((val - min_v) / (max_v - min_v)) * 1000.0));
                    updating_ui = true;
                    rows[i].val_edit->setText(QString::number(val, 'g', 4));
                    rows[i].slider->setValue(std::clamp(pos, 0, 1000));
                    updating_ui = false;
                    evaluateTimeDomain(false);
                };
                connect(rows[i].val_edit, &QLineEdit::editingFinished, this, syncEditToSlider);
                connect(rows[i].min_edit, &QLineEdit::editingFinished, this, syncEditToSlider);
                connect(rows[i].max_edit, &QLineEdit::editingFinished, this, syncEditToSlider);
                if (rows[i].disc_chk) {
                    connect(rows[i].disc_chk, &QCheckBox::toggled, this, [syncEditToSlider](bool) {
                        syncEditToSlider();
                    });
                }
            }
        };

        wireSliderGroup(sig_sliders, false);
        wireSliderGroup(nl_sliders, true);

        connect(x_expr_edit, &QLineEdit::textChanged, this, [this](const QString&) { updateVisibleTermSliders(); });

        connect(preset_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
            if (updating_ui) return;
            static const std::vector<QString> exprs = {
                "A * cos(2*pi*B*t)",
                "A * cos(2*pi*B*t) + C * cos(2*pi*D*t)",
                "(1 + A*cos(2*pi*B*t)) * cos(2*pi*C*t)",
                "A * exp(-B*t) * cos(2*pi*C*t) * u(t)",
                "A * rect(t / B)",
                "A * sinc(B * t)",
                "A*sin(2*pi*B*t) + (A/3)*sin(6*pi*B*t) + (A/5)*sin(10*pi*B*t)",
                "A * cos(2*pi * (B*t + C*t^2))"
            };
            if (idx >= 0 && idx < static_cast<int>(exprs.size())) {
                x_expr_edit->setText(exprs[idx]);
                updateVisibleTermSliders();
                evaluateTimeDomain(false);
            }
        });

        connect(nonlin_preset_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
            if (updating_ui) return;
            static const std::vector<QString> nl_exprs = {
                "a0 + a1*x + a2*x^2 + a3*x^3",
                "tanh(a1 * x)",
                "clamp(a1 * x, -1, 1)",
                "abs(x)",
                "max(0, x)",
                "0.2 * (exp(2*x) - 1)",
                "(abs(x) > 0.25) * (x - 0.25*sign(x))",
                "round(x * 4) / 4"
            };
            if (idx >= 0 && idx < static_cast<int>(nl_exprs.size())) {
                updating_ui = true;
                custom_nl_edit->setText(nl_exprs[idx]);
                poly_nl_radio->setChecked(idx == 0);
                custom_nl_radio->setChecked(idx != 0);
                updating_ui = false;
                updateVisibleTermSliders();
                evaluateTimeDomain(false);
            }
        });

        connect(cont_radio, &QRadioButton::toggled, this, [this](bool is_cont) {
            if (updating_ui) return;
            updating_ui = true;
            if (is_cont) {
                time_div_edit->setText("0.2");
                time_pos_edit->setText("0.0");
                sa_center_edit->setText("7.5");
                sa_span_edit->setText("15.0");
                sa_max_edit->setText("15.0");
                QString cur = x_expr_edit->text();
                cur.replace(QRegularExpression("\\bn\\b"), "t");
                x_expr_edit->setText(cur);
            } else {
                time_div_edit->setText("6");
                time_pos_edit->setText("0");
                sa_center_edit->setText("0.0");
                sa_span_edit->setText("2.0");
                sa_max_edit->setText("1.0");
                QString cur = x_expr_edit->text();
                cur.replace(QRegularExpression("\\bt\\b"), "n");
                x_expr_edit->setText(cur);
            }
            updating_ui = false;
            evaluateTimeDomain(false);
            n_pts_edit->setText(is_cont ? "1024" : "32");
        });

        // Oscilloscope controls
        connect(time_div_edit, &QLineEdit::editingFinished, this, [this]() {
            if (disc_radio->isChecked() && !updating_ui) {
                double tdiv = std::max(0.5, parseNumericInput(time_div_edit->text().toStdString(), 3.2));
                updating_ui = true;
                n_pts_edit->setText(QString::number(std::clamp(static_cast<int>(std::round(10.0 * tdiv)) + 1, 4, 2048)));
                updating_ui = false;
            }
            evaluateTimeDomain(false);
        });
        for (auto* ed : {time_pos_edit, ch1_div_edit, ch1_off_edit, ch2_div_edit, ch2_off_edit}) {
            connect(ed, &QLineEdit::editingFinished, this, [this]() { evaluateTimeDomain(false); });
        }

        connect(auto_scale_btn, &QPushButton::clicked, this, [this]() {
            if (!cached_x_prog.valid) return;
            bool is_disc = disc_radio->isChecked();
            bool nl_on   = nonlin_chk->isChecked() && cached_y_prog.valid;

            auto evalY = [&](double tv) -> double {
                double xv = cached_x_prog.eval(tv, 0.0, cached_params, is_disc);
                return nl_on ? cached_y_prog.eval(tv, xv, cached_params, is_disc) : xv;
            };

            double new_center = 0.0;
            double new_t_div  = is_disc ? 3.2 : 0.2;
            bool   found_time = false;

            // 1. Multi-scale probe to detect periodic cycles OR finite pulse support
            std::vector<double> probe_spans = is_disc
                ? std::vector<double>{64.0, 256.0, 1024.0}
                : std::vector<double>{2.0, 20.0, 0.2, 200.0};

            for (double span : probe_spans) {
                int M_p = is_disc ? std::clamp(static_cast<int>(span) + 1, 32, 1025) : 2000;
                double t0 = -0.5 * span;
                double dt_p = span / double(M_p - 1);

                std::vector<double> ts(M_p), ys(M_p);
                double y_min = 1e30, y_max = -1e30, y_sum = 0.0;
                for (int i = 0; i < M_p; ++i) {
                    double tv = is_disc ? std::round(t0 + i * dt_p) : (t0 + i * dt_p);
                    double yv = evalY(tv);
                    ts[i] = tv;
                    ys[i] = yv;
                    y_min = std::min(y_min, yv);
                    y_max = std::max(y_max, yv);
                    y_sum += yv;
                }
                if (y_max - y_min < 1e-7) continue; // Flatline on this span

                double y_mean = y_sum / double(M_p);
                std::vector<double> zc_times;
                for (int i = 1; i < M_p; ++i) {
                    double a = ys[i - 1] - y_mean;
                    double b = ys[i]     - y_mean;
                    if ((a < 0.0 && b >= 0.0) || (a > 0.0 && b <= 0.0)) {
                        double frac = std::abs(a) / std::max(1e-12, std::abs(a) + std::abs(b));
                        zc_times.push_back(ts[i - 1] + frac * (ts[i] - ts[i - 1]));
                    }
                }

                // A. Periodic waveform: >= 4 zero crossings (>= 2 full cycles)
                // Also verify crossings aren't clustered in a tiny fraction of the window (like a sinc packet)
                if (zc_times.size() >= 4) {
                    double active_zc_span = zc_times.back() - zc_times.front();
                    if (active_zc_span > 0.35 * span && zc_times.size() <= 400) {
                        double avg_half_period = active_zc_span / double(zc_times.size() - 1);
                        double period = 2.0 * avg_half_period;
                        // Show ~3 full cycles across the 10 horizontal divisions (minimum 2+ cycles guaranteed)
                        new_t_div  = std::max(is_disc ? 1.0 : 1e-5, (3.0 * period) / 10.0);
                        new_center = 0.0;
                        found_time = true;
                        break;
                    }
                }
            }

            // B. Pulse / Transient fallback: find bounding box of non-baseline activity
            if (!found_time) {
                for (double span : (is_disc ? std::vector<double>{64.0, 256.0, 1024.0} : std::vector<double>{4.0, 20.0, 100.0, 0.4})) {
                    int M_p = is_disc ? std::clamp(static_cast<int>(span) + 1, 32, 1025) : 2000;
                    double t0 = -0.5 * span;
                    double dt_p = span / double(M_p - 1);

                    std::vector<double> ts(M_p), ys(M_p);
                    double y_min = 1e30, y_max = -1e30;
                    for (int i = 0; i < M_p; ++i) {
                        double tv = is_disc ? std::round(t0 + i * dt_p) : (t0 + i * dt_p);
                        double yv = evalY(tv);
                        ts[i] = tv;
                        ys[i] = yv;
                        y_min = std::min(y_min, yv);
                        y_max = std::max(y_max, yv);
                    }
                    double pk_to_pk = y_max - y_min;
                    if (pk_to_pk < 1e-7) continue;

                    // Baseline is typically 0.0 (or edge value if pulse is localized)
                    double baseline = (std::abs(ys.front()) < 0.1 * pk_to_pk) ? ys.front() : 0.0;
                    double thresh = 0.05 * std::max(pk_to_pk, std::max(std::abs(y_max - baseline), std::abs(y_min - baseline)));

                    int i_first = -1, i_last = -1;
                    for (int i = 0; i < M_p; ++i) {
                        if (std::abs(ys[i] - baseline) >= thresh) {
                            if (i_first < 0) i_first = i;
                            i_last = i;
                        }
                    }

                    // If the pulse starts and ends inside this probe window (or is a step u(t) starting at 0)
                    if (i_first >= 0 && i_last >= i_first) {
                        bool clipped_left  = (i_first <= 2);
                        bool clipped_right = (i_last  >= M_p - 3);
                        if (!clipped_left && !clipped_right) {
                            double width = std::max(is_disc ? 4.0 : (3.0 * dt_p), ts[i_last] - ts[i_first]);
                            new_center = 0.5 * (ts[i_first] + ts[i_last]);
                            new_t_div  = std::max(is_disc ? 1.0 : 1e-5, (1.6 * width) / 10.0); // Entire pulse + 60% padding
                            found_time = true;
                            break;
                        } else if (!clipped_left && clipped_right) {
                            // Causal step / decaying exponential u(t): frame from slightly before t_first
                            double t_start = ts[i_first];
                            // Find where transient settles or changes by 90%
                            double y_end = ys.back();
                            int i_settle = i_first;
                            for (int i = i_first; i < M_p; ++i) {
                                if (std::abs(ys[i] - y_end) > 0.05 * pk_to_pk) i_settle = i;
                            }
                            if (i_settle < M_p - 10) {
                                double width = std::max(is_disc ? 6.0 : (4.0 * dt_p), ts[i_settle] - t_start);
                                new_center = t_start + 0.45 * width;
                                new_t_div  = std::max(is_disc ? 1.0 : 1e-5, (1.5 * width) / 10.0);
                                found_time = true;
                                break;
                            }
                        }
                    }
                }
            }

            // Round Time/Div to a clean 1-2-5 oscilloscope step or clean 3-sig-fig value
            if (!is_disc) {
                double p10 = std::pow(10.0, std::floor(std::log10(std::max(1e-9, new_t_div))));
                double norm = new_t_div / p10;
                double nice = (norm <= 1.2) ? 1.0 : ((norm <= 2.2) ? 2.0 : ((norm <= 3.2) ? 2.5 : ((norm <= 5.5) ? 5.0 : 10.0)));
                new_t_div = nice * p10;
            } else {
                new_t_div  = std::max(1.0, std::round(new_t_div));
                new_center = std::round(new_center);
            }

            // 2. Update Horizontal UI fields and re-evaluate time domain over the new window
            updating_ui = true;
            time_div_edit->setText(QString::number(new_t_div, 'g', 4));
            time_pos_edit->setText(QString::number(std::abs(new_center) < 1e-9 ? 0.0 : new_center, 'g', 4));
            if (is_disc) {
                n_pts_edit->setText(QString::number(std::clamp(static_cast<int>(std::round(10.0 * new_t_div)) + 1, 8, 2048)));
            }
            updating_ui = false;

            evaluateTimeDomain(false);

            // 3. Fit Vertical CH1(x) & CH2(y) Mag/Div to the newly framed waveform
            const auto& d = canvas->data;
            double max_x = 0.1, max_y = 0.1;
            for (const auto& c : d.x_in)  max_x = std::max(max_x, std::abs(c.real()));
            for (const auto& c : d.y_out) max_y = std::max(max_y, std::abs(c.real()));

            updating_ui = true;
            ch1_div_edit->setText(QString::number(std::max(0.02, (2.2 * max_x) / 6.0), 'g', 3));
            ch1_off_edit->setText("0.0");
            ch2_div_edit->setText(QString::number(std::max(0.02, (2.2 * max_y) / 6.0), 'g', 3));
            ch2_off_edit->setText("0.0");
            updating_ui = false;

            evaluateTimeDomain(false);
        });

        // Spectrum Analyzer Bidirectional Coupling: Center + Span <-> Max
        auto onCenterOrSpanEdited = [this]() {
            if (updating_ui) return;
            double fc = parseNumericInput(sa_center_edit->text().toStdString(), 7.5);
            double sp = std::max(0.01, parseNumericInput(sa_span_edit->text().toStdString(), 15.0));
            if (!disc_radio->isChecked() && fc - 0.5 * sp < 0.0) {
                fc = 0.5 * sp; // Keep start frequency >= 0 Hz in continuous mode
            }
            double fmax = fc + 0.5 * sp;
            updating_ui = true;
            sa_center_edit->setText(QString::number(fc, 'g', 4));
            sa_span_edit->setText(QString::number(sp, 'g', 4));
            sa_max_edit->setText(QString::number(fmax, 'g', 4));
            updating_ui = false;
            computeSpectrumNow();
        };
        connect(sa_center_edit, &QLineEdit::editingFinished, this, onCenterOrSpanEdited);
        connect(sa_span_edit,   &QLineEdit::editingFinished, this, onCenterOrSpanEdited);

        connect(sa_max_edit, &QLineEdit::editingFinished, this, [this]() {
            if (updating_ui) return;
            double fmax = std::max(0.05, parseNumericInput(sa_max_edit->text().toStdString(), 15.0));
            double fstart = disc_radio->isChecked() ? -fmax : 0.0;
            double sp = fmax - fstart;
            double fc = 0.5 * (fstart + fmax);
            updating_ui = true;
            sa_center_edit->setText(QString::number(fc, 'g', 4));
            sa_span_edit->setText(QString::number(sp, 'g', 4));
            sa_max_edit->setText(QString::number(fmax, 'g', 4));
            updating_ui = false;
            computeSpectrumNow();
        });

        connect(auto_rbw_chk, &QCheckBox::toggled, this, [this](bool is_auto) {
            rbw_edit->setEnabled(!is_auto);
            computeSpectrumNow();
        });
        connect(rbw_edit, &QLineEdit::editingFinished, this, [this]() {
            if (!updating_ui) computeSpectrumNow();
        });

        connect(eval_btn, &QPushButton::clicked, this, [this]() { evaluateTimeDomain(false); });
        connect(x_expr_edit, &QLineEdit::returnPressed, this, [this]() { evaluateTimeDomain(false); });
        connect(custom_nl_edit, &QLineEdit::returnPressed, this, [this]() { evaluateTimeDomain(false); });
        connect(nonlin_chk, &QCheckBox::toggled, this, [this](bool) {
            updateVisibleTermSliders();
            evaluateTimeDomain(false);
        });
        connect(poly_nl_radio, &QRadioButton::toggled, this, [this](bool ch) {
            if (!updating_ui) {
                if (ch) custom_nl_edit->setText("a0 + a1*x + a2*x^2 + a3*x^3");
                updateVisibleTermSliders();
                evaluateTimeDomain(false);
            }
        });
        connect(window_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) { computeSpectrumNow(); });

        connect(chk_in_ov, &QCheckBox::toggled, this, [this](bool v) { canvas->show_input_overlay = v; canvas->update(); });
        connect(chk_eo,    &QCheckBox::toggled, this, [this](bool v) { canvas->show_even_odd = v; canvas->update(); });
        connect(chk_inset, &QCheckBox::toggled, this, [this](bool v) { canvas->show_transfer_inset = v; canvas->update(); });
        connect(chk_db,    &QCheckBox::toggled, this, [this](bool v) { canvas->mag_in_db = v; canvas->update(); });
        connect(chk_ph,    &QCheckBox::toggled, this, [this](bool v) { canvas->show_phase = v; canvas->update(); });

        connect(n_pts_edit, &QLineEdit::editingFinished, this, [this]() { evaluateTimeDomain(false); });
        connect(samp_sim_chk, &QCheckBox::toggled, this, [this](bool) { evaluateTimeDomain(false); });
        connect(recon_mode_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) { evaluateTimeDomain(false); });

        connect(fs_slider, &QSlider::valueChanged, this, [this](int v) {
            if (updating_ui) return;
            double max_fs = std::max(0.5, parseNumericInput(fs_max_edit->text().toStdString(), 30.0));
            double fs_val = std::max(0.1, (v / 1000.0) * max_fs);
            updating_ui = true;
            fs_edit->setText(QString::number(fs_val, 'g', 4));
            if (!samp_sim_chk->isChecked()) samp_sim_chk->setChecked(true);
            updating_ui = false;
            evaluateTimeDomain(true);
        });

        auto syncFsEdit = [this]() {
            if (updating_ui) return;
            double fs_val = std::max(0.1, parseNumericInput(fs_edit->text().toStdString(), 8.0));
            double max_fs = std::max(fs_val, parseNumericInput(fs_max_edit->text().toStdString(), 30.0));
            updating_ui = true;
            fs_max_edit->setText(QString::number(max_fs, 'g', 4));
            fs_slider->setValue(std::clamp(static_cast<int>(std::round((fs_val / max_fs) * 1000.0)), 1, 1000));
            updating_ui = false;
            evaluateTimeDomain(false);
        };
        connect(fs_edit,     &QLineEdit::editingFinished, this, syncFsEdit);
        connect(fs_max_edit, &QLineEdit::editingFinished, this, syncFsEdit);

        canvas->onCursorsChanged = [this](double, double) { updateTelemetryReadout(); };

        updateVisibleTermSliders();
        evaluateTimeDomain(false);
    }

    void updateVisibleTermSliders() {
        QString eq = x_expr_edit->text() + " " + (nonlin_chk->isChecked() ? custom_nl_edit->text() : "");
        int visible_count = 0;
        for (auto& row : sig_sliders) {
            QRegularExpression rx("\\b" + QString::fromStdString(row.name) + "\\b");
            bool present = rx.match(eq).hasMatch();
            row.row_widget->setVisible(present);
            if (present) ++visible_count;
        }
        param_card->setVisible(visible_count > 0);

        bool nl_on = nonlin_chk->isChecked();
        bool is_poly = poly_nl_radio->isChecked();
        nl_sliders_container->setVisible(nl_on && is_poly);
        custom_nl_container->setVisible(nl_on && !is_poly);
    }

    // Stage 1: Instant Oscilloscope Time-Domain Evaluation (< 0.2 ms)
    void evaluateTimeDomain(bool debounce_spectrum) {
        if (updating_ui) return;

        SignalEvalResult& res = canvas->data;
        res.is_discrete = disc_radio->isChecked();
        res.nonlin_enabled = nonlin_chk->isChecked();
        res.error_msg.clear();

        res.time_per_div = std::max(1e-5, parseNumericInput(time_div_edit->text().toStdString(), res.is_discrete ? 3.2 : 0.2));
        res.time_center  = parseNumericInput(time_pos_edit->text().toStdString(), 0.0);
        res.ch1_mag_div  = std::max(1e-5, parseNumericInput(ch1_div_edit->text().toStdString(), 0.5));
        res.ch1_offset   = parseNumericInput(ch1_off_edit->text().toStdString(), 0.0);
        res.ch2_mag_div  = std::max(1e-5, parseNumericInput(ch2_div_edit->text().toStdString(), 0.5));
        res.ch2_offset   = parseNumericInput(ch2_off_edit->text().toStdString(), 0.0);

        int N = std::clamp(static_cast<int>(parseNumericInput(n_pts_edit->text().toStdString(), res.is_discrete ? 32 : 1024)), 4, 8192);
        res.num_samples = N;

        double t_min = res.time_center - 5.0 * res.time_per_div;
        double t_max = res.time_center + 5.0 * res.time_per_div;

        if (res.is_discrete) {
            // In discrete mode, Samples N directly sets the number of sequence samples n!
            double n_center = std::round(res.time_center);
            t_min = n_center - std::floor((N - 1) * 0.5);
            t_max = t_min + (N - 1);
            res.time_per_div = std::max(0.5, (N - 1) / 10.0);
            updating_ui = true;
            time_div_edit->setText(QString::number(res.time_per_div, 'g', 3));
            updating_ui = false;
        }
        double dt = (N > 1) ? ((t_max - t_min) / double(N - 1)) : 1.0;

        for (size_t i = 0; i < sig_sliders.size(); ++i) {
            cached_params[i] = parseNumericInput(sig_sliders[i].val_edit->text().toStdString(), 0.0);
        }
        for (size_t i = 0; i < nl_sliders.size(); ++i) {
            cached_params[4 + i] = parseNumericInput(nl_sliders[i].val_edit->text().toStdString(), 0.0);
        }
        cached_params[8] = t_max - t_min;
        cached_params[9] = dt;

        std::string raw_x_expr = x_expr_edit->text().toStdString();
        std::string raw_y_expr = custom_nl_edit->text().toStdString();

        QString x_lhs = res.is_discrete ? "x[n] = " : "x(t) = ";
        x_math_w->setLatex(x_lhs + QString::fromStdString(FastSignalEvaluator::toCleanLatex(raw_x_expr)));
        y_math_w->setLatex("y(x) = " + QString::fromStdString(FastSignalEvaluator::toCleanLatex(raw_y_expr)));
        y_math_w->setVisible(res.nonlin_enabled);

        if (!cached_x_prog.compile(raw_x_expr)) {
            res.error_msg = cached_x_prog.err_msg;
            updateTelemetryReadout();
            return;
        }
        if (res.nonlin_enabled && !cached_y_prog.compile(raw_y_expr)) {
            res.error_msg = cached_y_prog.err_msg;
            updateTelemetryReadout();
            return;
        }

        res.t_grid.resize(N);
        res.x_in.resize(N);
        res.y_out.resize(N);

        double max_x_abs = 0.5;
        double sum_x2 = 0.0, sum_y2 = 0.0, sum_y = 0.0;

        for (int i = 0; i < N; ++i) {
            double tv = res.is_discrete ? (t_min + i) : (t_min + i * dt);
            res.t_grid[i] = tv;
            double xv = cached_x_prog.eval(tv, 0.0, cached_params, res.is_discrete);
            res.x_in[i] = Complex(xv, 0.0);
            max_x_abs = std::max(max_x_abs, std::abs(xv));

            double yv = res.nonlin_enabled ? cached_y_prog.eval(tv, xv, cached_params, res.is_discrete) : xv;
            res.y_out[i] = Complex(yv, 0.0);

            sum_x2 += xv * xv;
            sum_y2 += yv * yv;
            sum_y  += yv;
        }

        // Evaluate ADC Sampling & Continuous Reconstruction y_hat(t)
        res.sampling_sim_enabled = (!res.is_discrete && samp_sim_chk->isChecked());
        res.fs_hz = std::max(0.1, parseNumericInput(fs_edit->text().toStdString(), 8.0));
        res.recon_mode = recon_mode_combo->currentIndex();
        res.t_samples.clear();
        res.y_samples.clear();
        res.y_recon.clear();

        if (res.sampling_sim_enabled) {
            double Ts = 1.0 / res.fs_hz;
            // Include extra guard samples outside [t_min, t_max] so Sinc edge lobes are accurate
            int k_start = static_cast<int>(std::floor(t_min / Ts)) - 16;
            int k_end   = static_cast<int>(std::ceil(t_max  / Ts)) + 16;
            // Cap total sample count for ultra-fast interactive slider response
            if (k_end - k_start > 600) {
                int mid_k = static_cast<int>(std::round(res.time_center / Ts));
                k_start = mid_k - 300;
                k_end   = mid_k + 300;
            }
            for (int k = k_start; k <= k_end; ++k) {
                double tk = k * Ts;
                double xk = cached_x_prog.eval(tk, 0.0, cached_params, false);
                double yk = res.nonlin_enabled ? cached_y_prog.eval(tk, xk, cached_params, false) : xk;
                res.t_samples.push_back(tk);
                res.y_samples.push_back(yk);
            }

            res.y_recon.assign(N, 0.0);
            int M_s = static_cast<int>(res.t_samples.size());
            for (int i = 0; i < N; ++i) {
                double tv = res.t_grid[i];
                double y_hat = 0.0;
                if (res.recon_mode == 0) {
                    // Whittaker-Shannon Ideal Bandlimited Sinc Interpolation
                    for (int k = 0; k < M_s; ++k) {
                        double u = (tv - res.t_samples[k]) / Ts;
                        double sinc_u = (std::abs(u) < 1e-8) ? 1.0 : (std::sin(PI * u) / (PI * u));
                        y_hat += res.y_samples[k] * sinc_u;
                    }
                } else if (res.recon_mode == 1) {
                    // Zero-Order Hold (ZOH Staircase DAC)
                    double k_fl = std::floor(tv / Ts + 1e-9);
                    int idx = std::clamp(static_cast<int>(std::round(k_fl)) - k_start, 0, M_s - 1);
                    y_hat = res.y_samples[idx];
                } else {
                    // First-Order Hold (Linear Interpolation)
                    double u_pos = tv / Ts;
                    int k0 = static_cast<int>(std::floor(u_pos));
                    double frac = u_pos - k0;
                    int idx0 = std::clamp(k0 - k_start, 0, M_s - 1);
                    int idx1 = std::clamp(idx0 + 1,     0, M_s - 1);
                    y_hat = (1.0 - frac) * res.y_samples[idx0] + frac * res.y_samples[idx1];
                }
                res.y_recon[i] = y_hat;
            }
        }

        if (res.nonlin_enabled) {
            int K_curve = 81;
            res.curve_x.resize(K_curve);
            res.curve_y.resize(K_curve);
            for (int k = 0; k < K_curve; ++k) {
                double xv = -max_x_abs + (2.0 * max_x_abs * k) / double(K_curve - 1);
                res.curve_x[k] = xv;
                res.curve_y[k] = cached_y_prog.eval(0.0, xv, cached_params, res.is_discrete);
            }
        }

        if (res.is_discrete) {
            res.energy_x = sum_x2;
            res.power_x  = sum_x2 / double(N);
            res.energy_y = sum_y2;
            res.power_y  = sum_y2 / double(N);
        } else {
            res.energy_x = sum_x2 * dt;
            res.power_x  = res.energy_x / std::max(1e-9, t_max - t_min);
            res.energy_y = sum_y2 * dt;
            res.power_y  = res.energy_y / std::max(1e-9, t_max - t_min);
        }
        res.dc_y  = sum_y / double(N);
        res.rms_y = std::sqrt(sum_y2 / double(N));

        if (debounce_spectrum) {
            res.spectrum_pending = true;
            canvas->update();
            updateTelemetryReadout();
            spectrum_timer->start();
        } else {
            spectrum_timer->stop();
            computeSpectrumNow();
        }
    }

    // Stage 2: Spectrum Analyzer with Center/Span/Max & Automatic or User-Overridden RBW (df)
    void computeSpectrumNow() {
        SignalEvalResult& res = canvas->data;
        res.spectrum_pending = false;
        if (!cached_x_prog.valid || (res.nonlin_enabled && !cached_y_prog.valid)) return;

        double fc = parseNumericInput(sa_center_edit->text().toStdString(), res.is_discrete ? 0.0 : 7.5);
        double sp = std::max(1e-3, parseNumericInput(sa_span_edit->text().toStdString(), res.is_discrete ? 2.0 : 15.0));
        double f_lo = fc - 0.5 * sp;
        double f_hi = fc + 0.5 * sp;
        if (!res.is_discrete && f_lo < 0.0) {
            f_lo = 0.0;
            f_hi = sp;
            fc = 0.5 * sp;
        }
        res.sa_center_hz = fc;
        res.sa_span_hz   = sp;
        res.sa_max_hz    = f_hi;

        // Window Equivalent Noise Bandwidth (ENBW) factor
        int win_mode = window_combo->currentIndex(); // 0=Hann, 1=Hamming, 2=Rect
        double enbw_factor = (win_mode == 0) ? 1.50 : ((win_mode == 1) ? 1.36 : 1.00);

        double scope_T = std::max(1e-4, 10.0 * res.time_per_div);

        // Estimate fundamental frequency from zero-crossings of y_out on scope to auto-size RBW
        double est_f0 = 0.0;
        if (!res.is_discrete && res.y_out.size() > 4) {
            int zc = 0;
            for (size_t i = 1; i < res.y_out.size(); ++i) {
                double y0 = res.y_out[i - 1].real() - res.dc_y;
                double y1 = res.y_out[i].real()     - res.dc_y;
                if ((y0 < 0.0 && y1 >= 0.0) || (y0 > 0.0 && y1 <= 0.0)) ++zc;
            }
            if (zc >= 2) est_f0 = (0.5 * zc) / scope_T;
        }

        double T_acq = scope_T;
        int user_N = std::clamp(res.num_samples, 4, 8192);

        if (auto_rbw_chk->isChecked()) {
            if (!res.is_discrete) {
                // Ensure at least 8 cycles of the fundamental AND at least 120 RBW bins across the Span
                double T_for_span = (120.0 * enbw_factor) / std::max(0.5, sp);
                double T_for_f0   = (est_f0 > 0.05) ? (8.0 / est_f0) : 2.0;
                T_acq = std::clamp(std::max({scope_T, T_for_span, T_for_f0}), 0.1, 40.0);
                res.rbw_hz = enbw_factor / T_acq;
            } else {
                // In discrete mode, N-point window directly determines DTFT/DFT resolution!
                T_acq = static_cast<double>(user_N);
                res.rbw_hz = (2.0 * enbw_factor) / std::max(1.0, T_acq);
            }
            updating_ui = true;
            rbw_edit->setText(QString::number(res.rbw_hz, 'g', 3));
            updating_ui = false;
        } else {
            double user_rbw = std::max(1e-4, parseNumericInput(rbw_edit->text().toStdString(), 0.25));
            res.rbw_hz = user_rbw;
            if (!res.is_discrete) {
                T_acq = std::clamp(enbw_factor / user_rbw, 0.02, 100.0);
            } else {
                user_N = std::clamp(static_cast<int>(std::round((2.0 * enbw_factor) / user_rbw)), 4, 4096);
                T_acq = static_cast<double>(user_N);
            }
        }
        res.acq_duration = T_acq;

        // Use the user's Samples N directly!
        int N_sa = user_N;

        double t_sa_min = res.time_center - 0.5 * T_acq;
        double t_sa_max = res.time_center + 0.5 * T_acq;
        if (res.is_discrete) {
            double n_center = std::round(res.time_center);
            t_sa_min = n_center - std::floor((N_sa - 1) * 0.5);
            t_sa_max = t_sa_min + (N_sa - 1);
        }
        double dt_sa = (N_sa > 1) ? ((t_sa_max - t_sa_min) / double(N_sa - 1)) : 1.0;

        cached_params[8] = T_acq;
        cached_params[9] = dt_sa;

        std::vector<double> x_sa(N_sa, 0.0), y_sa(N_sa, 0.0), y_rec_sa(N_sa, 0.0), win(N_sa, 1.0);
        double win_sum = 0.0;

        // If sampling simulator is enabled, prepare sample train over SA window
        double Ts_sim = 1.0 / std::max(0.1, res.fs_hz);
        int ks_min = 0, ks_max = -1;
        std::vector<double> sa_ts_vec, sa_ys_vec;
        if (res.sampling_sim_enabled) {
            ks_min = static_cast<int>(std::floor(t_sa_min / Ts_sim)) - 12;
            ks_max = static_cast<int>(std::ceil(t_sa_max  / Ts_sim)) + 12;
            if (ks_max - ks_min > 800) {
                int km = static_cast<int>(std::round(res.time_center / Ts_sim));
                ks_min = km - 400; ks_max = km + 400;
            }
            for (int k = ks_min; k <= ks_max; ++k) {
                double tk = k * Ts_sim;
                double xk = cached_x_prog.eval(tk, 0.0, cached_params, false);
                double yk = res.nonlin_enabled ? cached_y_prog.eval(tk, xk, cached_params, false) : xk;
                sa_ts_vec.push_back(tk);
                sa_ys_vec.push_back(yk);
            }
        }

        int M_s_sa = static_cast<int>(sa_ts_vec.size());
        for (int i = 0; i < N_sa; ++i) {
            double tv = res.is_discrete ? (t_sa_min + i) : (t_sa_min + i * dt_sa);
            double xv = cached_x_prog.eval(tv, 0.0, cached_params, res.is_discrete);
            double yv = res.nonlin_enabled ? cached_y_prog.eval(tv, xv, cached_params, res.is_discrete) : xv;
            x_sa[i] = xv;
            y_sa[i] = yv;

            if (res.sampling_sim_enabled && M_s_sa > 0) {
                if (res.recon_mode == 0) {
                    double yh = 0.0;
                    for (int k = 0; k < M_s_sa; ++k) {
                        double u = (tv - sa_ts_vec[k]) / Ts_sim;
                        yh += sa_ys_vec[k] * ((std::abs(u) < 1e-8) ? 1.0 : (std::sin(PI * u) / (PI * u)));
                    }
                    y_rec_sa[i] = yh;
                } else if (res.recon_mode == 1) {
                    int idx = std::clamp(static_cast<int>(std::floor(tv / Ts_sim + 1e-9)) - ks_min, 0, M_s_sa - 1);
                    y_rec_sa[i] = sa_ys_vec[idx];
                } else {
                    double u_pos = tv / Ts_sim;
                    int k0 = static_cast<int>(std::floor(u_pos));
                    double frac = u_pos - k0;
                    int idx0 = std::clamp(k0 - ks_min, 0, M_s_sa - 1);
                    int idx1 = std::clamp(idx0 + 1,   0, M_s_sa - 1);
                    y_rec_sa[i] = (1.0 - frac) * sa_ys_vec[idx0] + frac * sa_ys_vec[idx1];
                }
            }

            double w_i = 1.0;
            if (win_mode == 0 && N_sa > 1)      w_i = 0.5 * (1.0 - std::cos(2.0 * PI * i / (N_sa - 1)));
            else if (win_mode == 1 && N_sa > 1) w_i = 0.54 - 0.46 * std::cos(2.0 * PI * i / (N_sa - 1));
            win[i] = w_i;
            win_sum += w_i;
        }

        // Normalize so a unit-amplitude cosine has spectral peak height ~ 1.0 (or 0.5 two-sided)
        double norm_scale = (win_sum > 1e-9) ? (2.0 / win_sum) : (1.0 / N_sa);

        int M_freq = 400;
        res.freq_grid.resize(M_freq);
        res.X_spec.assign(M_freq, Complex(0.0, 0.0));
        res.Y_spec.assign(M_freq, Complex(0.0, 0.0));
        res.Y_recon_spec.assign(M_freq, Complex(0.0, 0.0));

        double step_t = res.is_discrete ? 1.0 : dt_sa;

        for (int m = 0; m < M_freq; ++m) {
            double f_val = f_lo + (f_hi - f_lo) * double(m) / double(M_freq - 1);
            res.freq_grid[m] = f_val;
            double omega = res.is_discrete ? (f_val * PI) : (2.0 * PI * f_val);

            Complex acc_x(0.0, 0.0), acc_y(0.0, 0.0), acc_rec(0.0, 0.0);
            Complex w_cur  = std::polar(1.0, -omega * t_sa_min);
            Complex w_step = std::polar(1.0, -omega * step_t);

            for (int i = 0; i < N_sa; ++i) {
                double w_i = win[i];
                acc_x   += (x_sa[i]     * w_i) * w_cur;
                acc_y   += (y_sa[i]     * w_i) * w_cur;
                if (res.sampling_sim_enabled) acc_rec += (y_rec_sa[i] * w_i) * w_cur;
                w_cur *= w_step;
            }
            double dc_weight = (!res.is_discrete && std::abs(f_val) < 0.5 * res.rbw_hz) ? 0.5 : 1.0;
            res.X_spec[m]       = acc_x   * (norm_scale * dc_weight);
            res.Y_spec[m]       = acc_y   * (norm_scale * dc_weight);
            res.Y_recon_spec[m] = acc_rec * (norm_scale * dc_weight);
        }

        for (int m = 0; m < M_freq; ++m) {
            double f_val = f_lo + (f_hi - f_lo) * double(m) / double(M_freq - 1);
            res.freq_grid[m] = f_val;
            double omega = res.is_discrete ? (f_val * PI) : (2.0 * PI * f_val);

            Complex acc_x(0.0, 0.0), acc_y(0.0, 0.0);
            Complex w_cur  = std::polar(1.0, -omega * t_sa_min);
            Complex w_step = std::polar(1.0, -omega * step_t);

            for (int i = 0; i < N_sa; ++i) {
                double w_i = win[i];
                acc_x += (x_sa[i] * w_i) * w_cur;
                acc_y += (y_sa[i] * w_i) * w_cur;
                w_cur *= w_step;
            }
            // Half-weight at DC (f = 0) for single-sided amplitude consistency
            double dc_weight = (!res.is_discrete && std::abs(f_val) < 0.5 * res.rbw_hz) ? 0.5 : 1.0;
            res.X_spec[m] = acc_x * (norm_scale * dc_weight);
            res.Y_spec[m] = acc_y * (norm_scale * dc_weight);
        }

        double df_bin = (f_hi - f_lo) / double(M_freq - 1);
        double dc_guard = res.is_discrete ? 0.04 : std::max(1.5 * res.rbw_hz, 3.0 * df_bin);

        int peak_idx = -1;
        double peak_mag = 0.0;
        for (int m = 0; m < M_freq; ++m) {
            if (res.freq_grid[m] > dc_guard) {
                double mv = std::abs(res.Y_spec[m]);
                if (mv > peak_mag) { peak_mag = mv; peak_idx = m; }
            }
        }
        res.fund_freq = 0.0; res.fund_mag = 0.0; res.thd_pct = 0.0;
        if (peak_idx >= 0 && peak_mag > 1e-6) {
            res.fund_freq = res.freq_grid[peak_idx];
            res.fund_mag  = peak_mag;

            double harm_sq_sum = 0.0;
            double search_tol = std::max(1.5 * res.rbw_hz, 2.5 * df_bin);
            for (int h = 2; h <= 7; ++h) {
                double fh = h * res.fund_freq;
                if (fh >= f_hi) break;
                double local_max = 0.0;
                for (int m = 0; m < M_freq; ++m) {
                    if (std::abs(res.freq_grid[m] - fh) <= search_tol) {
                        local_max = std::max(local_max, std::abs(res.Y_spec[m]));
                    }
                }
                harm_sq_sum += local_max * local_max;
            }
            res.thd_pct = 100.0 * std::sqrt(harm_sq_sum) / res.fund_mag;
        }
        res.is_aliasing = false;
        res.aliased_freq = res.fund_freq;
        if (res.sampling_sim_enabled && res.fund_freq > 1e-3) {
            res.aliased_freq = std::abs(res.fund_freq - res.fs_hz * std::round(res.fund_freq / res.fs_hz));
            res.is_aliasing  = (res.fs_hz < 2.0 * res.fund_freq - 1e-3);
        }

        canvas->update();
        updateTelemetryReadout();
    }

    void updateTelemetryReadout() {
        const auto& d = canvas->data;
        if (!d.error_msg.empty()) {
            status_banner->setStyleSheet(QString("QFrame { background-color: %1; border: 1px solid %2; border-radius: 4px; }")
                                         .arg(Theme::BANNER_ERR_BG, Theme::BANNER_ERR_BD));
            banner_left->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border:none;").arg(Theme::BANNER_ERR_FG));
            banner_left->setText("⚠ Expression Error: " + QString::fromStdString(d.error_msg));
            return;
        }

        bool high_thd = d.nonlin_enabled && (d.thd_pct > 5.0);
        status_banner->setStyleSheet(QString("QFrame { background-color: %1; border: 1px solid %2; border-radius: 4px; }")
            .arg(high_thd ? Theme::BANNER_WARN_BG : Theme::BANNER_OK_BG,
                 high_thd ? Theme::BANNER_WARN_BD : Theme::BANNER_OK_BD));
        banner_left->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border:none;")
            .arg(high_thd ? Theme::BANNER_WARN_FG : Theme::BANNER_OK_FG));
        banner_right->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; font-size: 11px; border:none;")
            .arg(Theme::TEXT_PRIMARY));

        banner_left->setText(d.nonlin_enabled
            ? "⚡ NONLINEAR DISTORTION ACTIVE  •  y(t) = g(x(t))"
            : "✔ LINEAR SIGNAL MODE  •  y(t) = x(t)");
        banner_right->setText(d.spectrum_pending
            ? "⟳ Sweeping Spectrum..."
            : QString("f₀ = %1 %2  |  RBW = %3  |  THD = %4%")
                .arg(QString::fromStdString(PRECISION.fmt(d.fund_freq)))
                .arg(d.is_discrete ? "π" : "Hz")
                .arg(QString::fromStdString(PRECISION.fmt(d.rbw_hz)))
                .arg(QString::fromStdString(PRECISION.fmt(d.thd_pct))));

        m_card1->setText(QString("Out E_y: %1  |  P_y: %2\nIn  E_x: %3  |  P_x: %4")
            .arg(QString::fromStdString(PRECISION.fmt(d.energy_y)))
            .arg(QString::fromStdString(PRECISION.fmt(d.power_y)))
            .arg(QString::fromStdString(PRECISION.fmt(d.energy_x)))
            .arg(QString::fromStdString(PRECISION.fmt(d.power_x))));

        m_card2->setText(QString("DC Mean: %1\nRMS Value: %2")
            .arg(QString::fromStdString(PRECISION.fmt(d.dc_y)))
            .arg(QString::fromStdString(PRECISION.fmt(d.rms_y))));

        m_card3->setText(QString("f₀: %1 %2 (|Y|=%3)\nRBW Δf: %4 %2  |  THD: %5%")
            .arg(QString::fromStdString(PRECISION.fmt(d.fund_freq)))
            .arg(d.is_discrete ? "×π" : "Hz")
            .arg(QString::fromStdString(PRECISION.fmt(d.fund_mag)))
            .arg(QString::fromStdString(PRECISION.fmt(d.rbw_hz)))
            .arg(QString::fromStdString(PRECISION.fmt(d.thd_pct))));

        double ct = canvas->cursor_time;
        double cf = canvas->cursor_freq;
        Complex xt_val(0, 0), yt_val(0, 0), Yf_val(0, 0);
        if (!d.t_grid.empty()) {
            double frac = std::clamp((ct - d.t_grid.front()) / std::max(1e-9, d.t_grid.back() - d.t_grid.front()), 0.0, 1.0);
            size_t idx = std::clamp<size_t>(std::llround(frac * (d.t_grid.size() - 1)), 0, d.t_grid.size() - 1);
            xt_val = d.x_in[idx];
            yt_val = d.y_out[idx];
        }
        if (!d.freq_grid.empty()) {
            double frac = std::clamp((cf - d.freq_grid.front()) / std::max(1e-9, d.freq_grid.back() - d.freq_grid.front()), 0.0, 1.0);
            size_t idx = std::clamp<size_t>(std::llround(frac * (d.freq_grid.size() - 1)), 0, d.freq_grid.size() - 1);
            Yf_val = d.Y_spec[idx];
        }

        m_card4->setText(QString("@ %1=%2:  CH1=%3, CH2=%4\n@ f=%5:  |Y|=%6, ∠=%7°")
            .arg(d.is_discrete ? "n" : "t")
            .arg(QString::fromStdString(PRECISION.fmt(ct)))
            .arg(QString::fromStdString(PRECISION.fmt(xt_val.real())))
            .arg(QString::fromStdString(PRECISION.fmt(yt_val.real())))
            .arg(QString::fromStdString(PRECISION.fmt(cf)))
            .arg(QString::fromStdString(PRECISION.fmt(std::abs(Yf_val))))
            .arg(QString::fromStdString(PRECISION.fmt(std::arg(Yf_val) * 180.0 / PI))));
    }
};