// SPDX-License-Identifier: MIT
// Copyright (c) 2026 Jaden Foote
//
// CoreAndWidgets.h
// Description: ezPZ Core & Widgets Header

#pragma once
#include "CommonIncludes.h"

#include <QFontDatabase>

// =============================================================================
// Unified Control Systems, Digital Filter, Block Diagram & Scientific Calc Suite
// Native C++17 / Qt6 / Eigen3 Complete Implementation (7 Synchronized Tabs)
// =============================================================================
using Complex = std::complex<double>;
using Poly = std::vector<Complex>;

static constexpr double PI = 3.14159265358979323846;

namespace Theme {
    inline int current_theme = 0;
    inline bool is_light     = false;
    // Default theme definition (Midnight Slate)
    inline QString BG_WINDOW      = "#0b0e14";
    inline QString BG_HEADER      = "#080b10";
    inline QString BG_PANEL       = "#121722";
    inline QString BG_PANEL_ALT   = "#171e2b";
    inline QString BG_INPUT       = "#090d16";
    inline QString BG_PLOT        = "#0d111a";
    inline QString BG_KEY_NUM     = "#1e2636";
    inline QString BG_KEY_FN      = "#182030";
    inline QString BG_KEY_ACCENT  = "#0369a2";

    inline QString BORDER_PANEL   = "#1f293d";
    inline QString BORDER_INPUT   = "#2b364f";
    inline QString BORDER_FOCUS   = "#0284c7";
    inline QString BORDER_BTN     = "#334155";

    inline QString TEXT_PRIMARY   = "#f1f5f9";
    inline QString TEXT_SECONDARY = "#cbd5e1";
    inline QString TEXT_MUTED     = "#94a3b8";

    inline QString ACCENT_POLE    = "#f87171";
    inline QString ACCENT_ZERO    = "#38bdf8";
    inline QString ACCENT_PID     = "#34d399";
    inline QString ACCENT_LOCUS   = "#c084fc";
    inline QString ACCENT_ARROW   = "#facc15";
    inline QString ACCENT_ASYMP   = "#22d3ee";
    inline QString ACCENT_OPEN    = "#fb923c";
    inline QString ACCENT_IMAG    = "#f472b6";

    inline QString BANNER_OK_BG   = "#062e22";
    inline QString BANNER_OK_BD   = "#10b981";
    inline QString BANNER_OK_FG   = "#34d399";

    inline QString BANNER_WARN_BG = "#332407";
    inline QString BANNER_WARN_BD = "#f59e0b";
    inline QString BANNER_WARN_FG = "#fbbf24";

    inline QString BANNER_ERR_BG  = "#361117";
    inline QString BANNER_ERR_BD  = "#ef4444";
    inline QString BANNER_ERR_FG  = "#f87171";

    inline QString GRID_MAJOR     = "#1a2233";
    inline QString AXIS_LINE      = "#3b4963";

    inline QStringList themeNames() {
        return {
            "Midnight Slate (Default Dark)",
            "Crisp Daylight (Light)",
            "Catppuccin Mocha",
            "Nord Polar",
            "Gruvbox Retro Dark",
            "Solarized Paper (Warm Light)",
            "Tokyo Night",
            "Retro CRT",
            "Rose Pine Dawn"
        };
    }

    inline std::vector<QString> currentPaletteVector() {
        return {
            BG_WINDOW, BG_HEADER, BG_PANEL, BG_PANEL_ALT,
            BG_INPUT, BG_PLOT, BG_KEY_NUM, BG_KEY_FN, BG_KEY_ACCENT,
            BORDER_PANEL, BORDER_INPUT, BORDER_FOCUS, BORDER_BTN,
            TEXT_PRIMARY, TEXT_SECONDARY, TEXT_MUTED,
            ACCENT_POLE, ACCENT_ZERO, ACCENT_PID, ACCENT_LOCUS,
            ACCENT_ARROW, ACCENT_ASYMP, ACCENT_OPEN, ACCENT_IMAG,
            BANNER_OK_BG, BANNER_OK_BD, BANNER_OK_FG,
            BANNER_WARN_BG, BANNER_WARN_BD, BANNER_WARN_FG,
            BANNER_ERR_BG, BANNER_ERR_BD, BANNER_ERR_FG,
            GRID_MAJOR, AXIS_LINE
        };
    }

    inline void setTheme(int idx) {
        current_theme = idx;
        is_light = (idx == 1 || idx == 5 || idx == 8);

        switch (idx) {
        case 1: // Crisp Daylight (Light)
            BG_WINDOW      = "#f1f5f8"; BG_HEADER      = "#e2e8ef";
            BG_PANEL       = "#ffffff"; BG_PANEL_ALT   = "#f8fafc";
            BG_INPUT       = "#fffffe"; BG_PLOT        = "#fcfdff";
            BG_KEY_NUM     = "#e2e8f0"; BG_KEY_FN      = "#edf2f7";
            BG_KEY_ACCENT  = "#0284c8";
            BORDER_PANEL   = "#cbd5e0"; BORDER_INPUT   = "#94a3b7";
            BORDER_FOCUS   = "#0369a1"; BORDER_BTN     = "#64748a";
            TEXT_PRIMARY   = "#0f172a"; TEXT_SECONDARY = "#334154";
            TEXT_MUTED     = "#475569";
            ACCENT_POLE    = "#dc2626"; ACCENT_ZERO    = "#0284c7";
            ACCENT_PID     = "#059669"; ACCENT_LOCUS   = "#7c3aed";
            ACCENT_ARROW   = "#d97706"; ACCENT_ASYMP   = "#0891b2";
            ACCENT_OPEN    = "#ea580c"; ACCENT_IMAG    = "#db2777";
            BANNER_OK_BG   = "#dcfce7"; BANNER_OK_BD   = "#16a34a"; BANNER_OK_FG   = "#15803d";
            BANNER_WARN_BG = "#fef3c7"; BANNER_WARN_BD = "#d97706"; BANNER_WARN_FG = "#b45309";
            BANNER_ERR_BG  = "#fee2e2"; BANNER_ERR_BD  = "#dc2626"; BANNER_ERR_FG  = "#b91c1c";
            GRID_MAJOR     = "#e2e8f1"; AXIS_LINE      = "#94a3b9";
            break;

        case 2: // Catppuccin Mocha
            BG_WINDOW      = "#1e1e2e"; BG_HEADER      = "#11111b";
            BG_PANEL       = "#181825"; BG_PANEL_ALT   = "#313244";
            BG_INPUT       = "#141420"; BG_PLOT        = "#191926";
            BG_KEY_NUM     = "#45475a"; BG_KEY_FN      = "#313245";
            BG_KEY_ACCENT  = "#89b4fa";
            BORDER_PANEL   = "#45475b"; BORDER_INPUT   = "#585b70";
            BORDER_FOCUS   = "#74c7ec"; BORDER_BTN     = "#6c7086";
            TEXT_PRIMARY   = "#cdd6f4"; TEXT_SECONDARY = "#bac2de";
            TEXT_MUTED     = "#a6adc8";
            ACCENT_POLE    = "#f38ba8"; ACCENT_ZERO    = "#89dceb";
            ACCENT_PID     = "#a6e3a1"; ACCENT_LOCUS   = "#cba6f7";
            ACCENT_ARROW   = "#f9e2af"; ACCENT_ASYMP   = "#94e2d5";
            ACCENT_OPEN    = "#fab387"; ACCENT_IMAG    = "#f5c2e7";
            BANNER_OK_BG   = "#1e3a2f"; BANNER_OK_BD   = "#a6e3a2"; BANNER_OK_FG   = "#a6e3a3";
            BANNER_WARN_BG = "#3b3222"; BANNER_WARN_BD = "#f9e2ae"; BANNER_WARN_FG = "#f9e2ad";
            BANNER_ERR_BG  = "#3c232e"; BANNER_ERR_BD   = "#f38ba7"; BANNER_ERR_FG  = "#f38ba6";
            GRID_MAJOR     = "#2a2b3d"; AXIS_LINE      = "#585b71";
            break;

        case 3: // Nord Polar
            BG_WINDOW      = "#2e3440"; BG_HEADER      = "#242933";
            BG_PANEL       = "#3b4252"; BG_PANEL_ALT   = "#434c5e";
            BG_INPUT       = "#292e39"; BG_PLOT        = "#2b303c";
            BG_KEY_NUM     = "#4c566a"; BG_KEY_FN      = "#3f4758";
            BG_KEY_ACCENT  = "#5e81ac";
            BORDER_PANEL   = "#4c566b"; BORDER_INPUT   = "#576279";
            BORDER_FOCUS   = "#88c0d0"; BORDER_BTN     = "#616e88";
            TEXT_PRIMARY   = "#eceff4"; TEXT_SECONDARY = "#e5e9f0";
            TEXT_MUTED     = "#d8dee9";
            ACCENT_POLE    = "#bf616a"; ACCENT_ZERO    = "#88c0d1";
            ACCENT_PID     = "#a3be8c"; ACCENT_LOCUS   = "#b48ead";
            ACCENT_ARROW   = "#ebcb8b"; ACCENT_ASYMP   = "#8fbcbb";
            ACCENT_OPEN    = "#d08770"; ACCENT_IMAG    = "#c997c0";
            BANNER_OK_BG   = "#2d3b32"; BANNER_OK_BD   = "#a3be8d"; BANNER_OK_FG   = "#b5d19c";
            BANNER_WARN_BG = "#3f382b"; BANNER_WARN_BD = "#ebcb8c"; BANNER_WARN_FG = "#f0d49a";
            BANNER_ERR_BG  = "#422b30"; BANNER_ERR_BD  = "#bf616b"; BANNER_ERR_FG  = "#d57780";
            GRID_MAJOR     = "#363d4c"; AXIS_LINE      = "#616e87";
            break;

        case 4: // Gruvbox Retro Dark
            BG_WINDOW      = "#1d2021"; BG_HEADER      = "#141617";
            BG_PANEL       = "#282828"; BG_PANEL_ALT   = "#32302f";
            BG_INPUT       = "#181a1b"; BG_PLOT        = "#1b1d1e";
            BG_KEY_NUM     = "#504945"; BG_KEY_FN      = "#3c3836";
            BG_KEY_ACCENT  = "#458588";
            BORDER_PANEL   = "#504946"; BORDER_INPUT   = "#665c54";
            BORDER_FOCUS   = "#83a598"; BORDER_BTN     = "#7c6f64";
            TEXT_PRIMARY   = "#fbf1c7"; TEXT_SECONDARY = "#ebdbb2";
            TEXT_MUTED     = "#bdae93";
            ACCENT_POLE    = "#fb4934"; ACCENT_ZERO    = "#83a599";
            ACCENT_PID     = "#b8bb26"; ACCENT_LOCUS   = "#d3869b";
            ACCENT_ARROW   = "#fabd2f"; ACCENT_ASYMP   = "#8ec07c";
            ACCENT_OPEN    = "#fe8019"; ACCENT_IMAG    = "#d65d0e";
            BANNER_OK_BG   = "#2c341d"; BANNER_OK_BD   = "#98971a"; BANNER_OK_FG   = "#b8bb27";
            BANNER_WARN_BG = "#3d311a"; BANNER_WARN_BD = "#d79921"; BANNER_WARN_FG = "#fabd2e";
            BANNER_ERR_BG  = "#3c1f1e"; BANNER_ERR_BD  = "#cc241d"; BANNER_ERR_FG  = "#fb4935";
            GRID_MAJOR     = "#2e2c2b"; AXIS_LINE      = "#665c55";
            break;

        case 5: // Solarized Paper (Warm Light)
            BG_WINDOW      = "#fdf6e3"; BG_HEADER      = "#eee8d5";
            BG_PANEL       = "#faf3e0"; BG_PANEL_ALT   = "#f2ecda";
            BG_INPUT       = "#fffdf7"; BG_PLOT        = "#fefbf1";
            BG_KEY_NUM     = "#e6dfcc"; BG_KEY_FN      = "#ede6d3";
            BG_KEY_ACCENT  = "#268bd2";
            BORDER_PANEL   = "#d5ceb9"; BORDER_INPUT   = "#b8b19d";
            BORDER_FOCUS   = "#268bd3"; BORDER_BTN     = "#93a1a1";
            TEXT_PRIMARY   = "#073642"; TEXT_SECONDARY = "#586e75";
            TEXT_MUTED     = "#657b83";
            ACCENT_POLE    = "#dc322f"; ACCENT_ZERO    = "#268bd4";
            ACCENT_PID     = "#859900"; ACCENT_LOCUS   = "#6c71c4";
            ACCENT_ARROW   = "#b58900"; ACCENT_ASYMP   = "#2aa198";
            ACCENT_OPEN    = "#cb4b16"; ACCENT_IMAG    = "#d33682";
            BANNER_OK_BG   = "#e9f2d5"; BANNER_OK_BD   = "#859901"; BANNER_OK_FG   = "#5f6e00";
            BANNER_WARN_BG = "#f9edd0"; BANNER_WARN_BD = "#b58901"; BANNER_WARN_FG = "#8c6900";
            BANNER_ERR_BG  = "#f9dedd"; BANNER_ERR_BD  = "#dc322e"; BANNER_ERR_FG  = "#a8201d";
            GRID_MAJOR     = "#eae3cf"; AXIS_LINE      = "#93a1a2";
            break;
        case 6: // Tokyo Night
            BG_WINDOW      = "#1a1b26"; BG_HEADER      = "#13141c";
            BG_PANEL       = "#1f2335"; BG_PANEL_ALT   = "#24283b";
            BG_INPUT       = "#16161e"; BG_PLOT        = "#151621";
            BG_KEY_NUM     = "#292e42"; BG_KEY_FN      = "#222638";
            BG_KEY_ACCENT  = "#3d59a1";
            BORDER_PANEL   = "#2f3549"; BORDER_INPUT   = "#3b4261";
            BORDER_FOCUS   = "#7aa2f7"; BORDER_BTN     = "#414868";
            TEXT_PRIMARY   = "#c0caf5"; TEXT_SECONDARY = "#a9b1d6";
            TEXT_MUTED     = "#737aa2";
            ACCENT_POLE    = "#f7768e"; ACCENT_ZERO    = "#7dcfff";
            ACCENT_PID     = "#9ece6a"; ACCENT_LOCUS   = "#bb9af7";
            ACCENT_ARROW   = "#e0af68"; ACCENT_ASYMP   = "#2ac3de";
            ACCENT_OPEN    = "#ff9e64"; ACCENT_IMAG    = "#ff007c";
            BANNER_OK_BG   = "#1c312d"; BANNER_OK_BD   = "#73daca"; BANNER_OK_FG   = "#9ece6b";
            BANNER_WARN_BG = "#332b22"; BANNER_WARN_BD = "#e0af67"; BANNER_WARN_FG = "#f0c674";
            BANNER_ERR_BG  = "#362029"; BANNER_ERR_BD  = "#db4b4b"; BANNER_ERR_FG  = "#f7768d";
            GRID_MAJOR     = "#23273a"; AXIS_LINE      = "#565f89";
            break;
        case 7: // Phosphor Oscilloscope (Retro CRT)
            BG_WINDOW      = "#050a06"; BG_HEADER      = "#030604";
            BG_PANEL       = "#08110a"; BG_PANEL_ALT   = "#0c190f";
            BG_INPUT       = "#040805"; BG_PLOT        = "#020503";
            BG_KEY_NUM     = "#112416"; BG_KEY_FN      = "#0d1c11";
            BG_KEY_ACCENT  = "#166534";
            BORDER_PANEL   = "#14331d"; BORDER_INPUT   = "#1b4326";
            BORDER_FOCUS   = "#39ff14"; BORDER_BTN     = "#235330";
            TEXT_PRIMARY   = "#b8ffbe"; TEXT_SECONDARY = "#74e382";
            TEXT_MUTED     = "#3e8e4c";
            ACCENT_POLE    = "#ff5533"; ACCENT_ZERO    = "#00ffcc";
            ACCENT_PID     = "#39ff15"; ACCENT_LOCUS   = "#ccff00";
            ACCENT_ARROW   = "#ffe600"; ACCENT_ASYMP   = "#00d9a6";
            ACCENT_OPEN    = "#ffaa00"; ACCENT_IMAG    = "#ff3399";
            BANNER_OK_BG   = "#092612"; BANNER_OK_BD   = "#22c55e"; BANNER_OK_FG   = "#4ade80";
            BANNER_WARN_BG = "#261f06"; BANNER_WARN_BD = "#eab308"; BANNER_WARN_FG = "#facc16";
            BANNER_ERR_BG  = "#290c0a"; BANNER_ERR_BD  = "#ef4445"; BANNER_ERR_FG  = "#f87172";
            GRID_MAJOR     = "#0e2213"; AXIS_LINE      = "#296639";
            break;

        case 8: // Rosé Pine Dawn (Soft Light)
            BG_WINDOW      = "#faf4ed"; BG_HEADER      = "#f2e9e1";
            BG_PANEL       = "#fffaf3"; BG_PANEL_ALT   = "#f4ede8";
            BG_INPUT       = "#ffffff"; BG_PLOT        = "#fdf9f3";
            BG_KEY_NUM     = "#ebe0d8"; BG_KEY_FN      = "#f1e7df";
            BG_KEY_ACCENT  = "#286983";
            BORDER_PANEL   = "#dfdad9"; BORDER_INPUT   = "#cecacd";
            BORDER_FOCUS   = "#56949f"; BORDER_BTN     = "#9893a5";
            TEXT_PRIMARY   = "#575279"; TEXT_SECONDARY = "#6e6a86";
            TEXT_MUTED     = "#797593";
            ACCENT_POLE    = "#b4637a"; ACCENT_ZERO    = "#286984";
            ACCENT_PID     = "#3e8fb0"; ACCENT_LOCUS   = "#907aa9";
            ACCENT_ARROW   = "#ea9d34"; ACCENT_ASYMP   = "#56949e";
            ACCENT_OPEN    = "#d7827e"; ACCENT_IMAG    = "#c44b70";
            BANNER_OK_BG   = "#e2f0ec"; BANNER_OK_BD   = "#286982"; BANNER_OK_FG   = "#1f5368";
            BANNER_WARN_BG = "#faecd8"; BANNER_WARN_BD = "#ea9d35"; BANNER_WARN_FG = "#b57419";
            BANNER_ERR_BG  = "#f7e2e7"; BANNER_ERR_BD  = "#b4637b"; BANNER_ERR_FG  = "#8f4258";
            GRID_MAJOR     = "#ece4dc"; AXIS_LINE      = "#b8b3c2";
            break;
        case 0: // Midnight Slate (Default Dark)
        default:
            current_theme  = 0;
            is_light       = false;
            BG_WINDOW      = "#0b0e14"; BG_HEADER      = "#080b10";
            BG_PANEL       = "#121722"; BG_PANEL_ALT   = "#171e2b";
            BG_INPUT       = "#090d16"; BG_PLOT        = "#0d111a";
            BG_KEY_NUM     = "#1e2636"; BG_KEY_FN      = "#182030";
            BG_KEY_ACCENT  = "#0369a2";
            BORDER_PANEL   = "#1f293d"; BORDER_INPUT   = "#2b364f";
            BORDER_FOCUS   = "#0284c7"; BORDER_BTN     = "#334155";
            TEXT_PRIMARY   = "#f1f5f9"; TEXT_SECONDARY = "#cbd5e1";
            TEXT_MUTED     = "#94a3b8";
            ACCENT_POLE    = "#f87171"; ACCENT_ZERO    = "#38bdf8";
            ACCENT_PID     = "#34d399"; ACCENT_LOCUS   = "#c084fc";
            ACCENT_ARROW   = "#facc15"; ACCENT_ASYMP   = "#22d3ee";
            ACCENT_OPEN    = "#fb923c"; ACCENT_IMAG    = "#f472b6";
            BANNER_OK_BG   = "#062e22"; BANNER_OK_BD   = "#10b981"; BANNER_OK_FG   = "#34d399";
            BANNER_WARN_BG = "#332407"; BANNER_WARN_BD = "#f59e0b"; BANNER_WARN_FG = "#fbbf24";
            BANNER_ERR_BG  = "#361117"; BANNER_ERR_BD  = "#ef4444"; BANNER_ERR_FG  = "#f87171";
            GRID_MAJOR     = "#1a2233"; AXIS_LINE      = "#3b4963";
            break;
        }
    }
}

inline void exportWidgetToSvg(QWidget* canvas, const QString& default_filename, bool force_light_theme = true) {
    if (!canvas) return;
    QString path = QFileDialog::getSaveFileName(
        canvas,
        "Export Plot as SVG",
        default_filename,
        "SVG Vector Graphics (*.svg)"
    );
    if (path.isEmpty()) return;
    if (!path.endsWith(".svg", Qt::CaseInsensitive)) path += ".svg";

    int prev_theme = Theme::current_theme;
    if (force_light_theme) {
        Theme::setTheme(1); // Crisp Daylight for note exports
    }

    QSvgGenerator generator;
    generator.setFileName(path);
    generator.setSize(canvas->size());
    generator.setViewBox(QRect(0, 0, canvas->width(), canvas->height()));
    generator.setTitle("ezPZ Plot");

    canvas->render(&generator);
    // Reset theme back to whatever the user had before
    if (force_light_theme) {
        Theme::setTheme(prev_theme);
        canvas->update();
    }
}

inline void attachSvgExportMenu(QWidget* canvas, const QString& default_svg_name) {
    if (!canvas) return;
    canvas->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(canvas, &QWidget::customContextMenuRequested, canvas, [canvas, default_svg_name](const QPoint& pos) {
        QMenu menu(canvas);
        QAction* act_light = menu.addAction("Export SVG (Light Theme)");
        QAction* act_dark  = menu.addAction("Export SVG (Dark Theme)");
        QAction* chosen = menu.exec(canvas->mapToGlobal(pos));
        if (chosen == act_light) {
            exportWidgetToSvg(canvas, default_svg_name, true);
        } else if (chosen == act_dark) {
            exportWidgetToSvg(canvas, default_svg_name, false);
        }
    });
}


// Engine 00: Decimal Precision & Formatting Configuration
struct PrecisionConfig {
    // Default decimal precision settings
    std::string mode = "class";
    int dec_sub1 = 3;
    int dec_ge1  = 2;
    bool step_round = false;
    bool omit_trailing_zeros = false;

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
        if (std::abs(fval) < 1e-10) fval = 0.0;

        int d = decimalsFor(fval);
        double av = std::abs(fval);

        // Use compact scientific notation for micro-scale (< 1e-4) or large (>= 1e5) values
        if (av > 0.0 && (av < 1e-4 || av >= 1e5)) {
            std::ostringstream s_exp;
            if (signed_fmt && fval >= 0.0) s_exp << "+";
            s_exp << std::scientific << std::setprecision(std::max(2, d)) << fval;
            std::string es = s_exp.str();
            if (strip_zeros || omit_trailing_zeros) {
                auto e_pos = es.find('e');
                if (e_pos != std::string::npos && es.find('.') < e_pos) {
                    std::string mant = es.substr(0, e_pos);
                    std::string exp_part = es.substr(e_pos);
                    while (!mant.empty() && mant.back() == '0') mant.pop_back();
                    if (!mant.empty() && mant.back() == '.') mant.pop_back();
                    es = mant + exp_part;
                }
            }
            return es;
        }

        // Expand decimal places for small sub-unity values (1e-4 <= |x| < 1) so leading zeros don't eat all sig figs
        if (av >= 1e-4 && av < 1.0) {
            int lead_zeros = static_cast<int>(std::floor(-std::log10(av)));
            d = std::max(d, lead_zeros + std::max(d, 3));
        }

        std::ostringstream oss;
        if (signed_fmt && fval >= 0.0) oss << "+";
        oss << std::fixed << std::setprecision(d) << fval;
        std::string out = oss.str();

        if ((strip_zeros || omit_trailing_zeros) && out.find('.') != std::string::npos) {
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
        double re_clean = (std::abs(val.real()) < std::max(1e-9, 1e-6 * std::abs(val.imag()))) ? 0.0 : val.real();
        double im_clean  = (std::abs(val.imag()) < std::max(1e-6, 1e-6 * std::abs(val.real()))) ? 0.0 : val.imag();
        std::string re_s = fmt(re_clean, false, strip_zeros);
        if (im_clean == 0.0) return re_s;
        if (re_clean == 0.0) return fmt(im_clean, false, strip_zeros) + "j";
        return re_s + fmt(im_clean, true, strip_zeros) + "j";
    }

    std::string summaryLabel() const {
        std::string s;
        if (mode == "class") {
            s = "0<|x|<1: " + std::to_string(dec_sub1) + " dec, |x|≥1: " + std::to_string(dec_ge1) + " dec";
        } else if (mode == "fixed") {
            s = "Fixed " + std::to_string(dec_ge1) + " dec";
        } else {
            s = "Auto Precision";
        }
        if (step_round) s += " • Step-Round ON";
        if (omit_trailing_zeros) s += " • Omit Trailing 0s";
        return s;
    }
};

static PrecisionConfig PRECISION;

inline double parseNumericInput(const std::string& raw, double def = 0.0) {
    std::string s = raw;
    s.erase(std::remove(s.begin(), s.end(), ','), s.end());
    auto ltrim = [](std::string& str) { str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) { return !std::isspace(ch); })); };
    auto rtrim = [](std::string& str) { str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) { return !std::isspace(ch); }).base(), str.end()); };
    ltrim(s); rtrim(s);
    if (s.empty()) return def;

    const std::vector<std::string> units = {"rad/s", "r/s", "deg", "sec", "%", "°", "s"};
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
    try { return std::stod(s); } catch (...) { return def; }
}

inline std::optional<std::pair<long long, long long>> toFraction(double val, long long max_den = 10000) {
    if (!std::isfinite(val) || std::abs(val) < 1e-9 || std::abs(val) > 1e6) return std::nullopt;
    double sign = (val < 0) ? -1.0 : 1.0;
    double x = std::abs(val);
    if (std::abs(x - std::round(x)) < 1e-10) return std::nullopt;

    long long h1 = 1, h2 = 0, k1 = 0, k2 = 1;
    double b = x;
    do {
        long long a = static_cast<long long>(std::floor(b));
        long long aux = h1; h1 = a * h1 + h2; h2 = aux;
        aux = k1; k1 = a * k1 + k2; k2 = aux;
        if (std::abs(b - a) < 1e-12) break;
        b = 1.0 / (b - a);
    } while (k1 <= max_den);

    if (k1 > max_den) { h1 = h2; k1 = k2; }
    if (k1 <= 1) return std::nullopt;
    if (std::abs(double(h1) / double(k1) - x) <= 1e-9 * std::max(1.0, x)) {
        return std::make_pair(static_cast<long long>(sign) * h1, k1);
    }
    return std::nullopt;
}

// =============================================================================
// Native 2D LaTeX Mathematical Typesetter Widget (Stacked \frac, \sqrt, ^, _)
// =============================================================================
class MathFormulaWidget : public QWidget {
public:
    QString raw_latex;
    QColor fg_color;
    QColor bg_color;
    int base_pt;

    explicit MathFormulaWidget(const QString& latex = "",
                               const QColor& fg = QColor(Theme::ACCENT_ZERO),
                               const QColor& bg = QColor(Theme::BG_INPUT),
                               int pt = 10,
                               QWidget* parent = nullptr)
        : QWidget(parent), fg_color(fg), bg_color(bg), base_pt(pt)
    {
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        setLatex(latex);
    }

    void setLatex(const QString& ltx) {
        if (raw_latex == ltx && !root_node.children.empty()) return;
        raw_latex = ltx;
        root_node = parseSegment(cleanLatexTop(raw_latex.toStdString()));
        QFont f = makeLatexBaseFont(base_pt + 1);
        auto m = measureNode(root_node, f);
        cached_size = QSize(static_cast<int>(std::ceil(m.w + 16.0)),
                            static_cast<int>(std::ceil(std::max(m.asc + m.desc + 8.0, 24.0))));
        setMinimumHeight(cached_size.height());
        updateGeometry();
        update();
    }

    QSize sizeHint() const override { return cached_size; }

    QSize minimumSizeHint() const override { return cached_size; }

protected:
    struct MathNode {
        enum Type { TEXT, SEQ, FRAC, SQRT, SUP, SUB, SUPSUB } type = TEXT;
        QString text;
        std::vector<MathNode> children;
    };

    struct NodeMetrics {
        double w = 0.0;
        double asc = 10.0;
        double desc = 4.0;
    };

    struct SafeFontMetrics {
        double asc;
        double desc;
        double axis;
        double em;
    };

    struct StyledSpan {
        QString text;
        bool italic = false;
        double pre_pad = 0.0;
        double post_pad = 0.0;
    };

    MathNode root_node;
    QSize cached_size = QSize(100, 28);

    static QFont makeLatexBaseFont(int pt) {
        static QString chosen_family;
        if (chosen_family.isEmpty()) {
            const QStringList installed = QFontDatabase::families();
            // Avoid both "Cambria Math" and "Cambria" — cambria.ttc on Windows reports ~3.4x WinAscent / ~2.4x WinDescent!
            const QStringList candidates = {
                "Latin Modern Roman", "CMU Serif", "STIX Two Text",
                "Palatino Linotype", "Times New Roman",
                "Liberation Serif", "DejaVu Serif", "Georgia"
            };
            for (const auto& c : candidates) {
                if (installed.contains(c, Qt::CaseInsensitive)) {
                    chosen_family = c;
                    break;
                }
            }
        }
        QFont f = chosen_family.isEmpty() ? QFont("Serif", pt) : QFont(chosen_family, pt);
        f.setPointSizeF(pt);
        f.setWeight(QFont::Normal);
        f.setStyleHint(QFont::Times, QFont::PreferAntialias);
        return f;
    }

    static SafeFontMetrics safeMetrics(const QFontMetricsF& fm) {
        double em = std::max(6.0, static_cast<double>(fm.horizontalAdvance(QStringLiteral("M"))));
        double cap = fm.capHeight();
        if (cap <= 2.0) {
            cap = fm.tightBoundingRect(QStringLiteral("H")).height();
        }
        if (cap <= 2.0) {
            cap = em * 0.72;
        }
        // Clamp inflated Windows OS/2 WinAscent/WinDescent bounds to real typographic proportions
        double asc  = std::min(static_cast<double>(fm.ascent()),  cap * 1.32);
        double desc = std::min(static_cast<double>(fm.descent()), cap * 0.40);
        QRectF plus_r = fm.tightBoundingRect(QStringLiteral("+"));
        double axis = (!plus_r.isEmpty())
            ? std::clamp(static_cast<double>(-plus_r.center().y()), cap * 0.25, cap * 0.55)
            : (asc * 0.34);
        return {asc, desc, axis, em};
    }

    static std::vector<StyledSpan> tokenizeMathText(const QString& s, double em_px) {
        std::vector<StyledSpan> spans;
        static const std::vector<QString> upright_words = {
            "sin", "cos", "tan", "sec", "csc", "cot", "arcsin", "arccos", "arctan",
            "sinh", "cosh", "tanh", "ln", "log", "exp", "lim", "max", "min", "arg",
            "Re", "Im", "det", "rad", "deg", "dB", "OS", "ss", "eval", "sym",
            "ol", "cl", "pos", "vel", "fb", "test", "RL", "Uncompensated", "None"
        };

        int n = s.length();
        int i = 0;
        while (i < n) {
            QChar c = s[i];
            if (c.isSpace()) {
                spans.push_back({" ", false, 0.0, 0.0});
                while (i < n && s[i].isSpace()) ++i;
            } else if (c.isLetter()) {
                int st = i;
                while (i < n && s[i].isLetter()) ++i;
                QString word = s.mid(st, i - st);
                bool use_italic = true;
                if (word.length() > 1 &&
                    std::find(upright_words.begin(), upright_words.end(), word) != upright_words.end()) {
                    use_italic = false;
                }
                // Greek uppercase (Δ, Ω, Σ, Π) stay upright; lowercase Greek & Latin variables are italic
                if (word.length() == 1 && word[0].unicode() >= 0x0391 && word[0].unicode() <= 0x03A9) {
                    use_italic = false;
                }
                spans.push_back({word, use_italic, 0.0, use_italic ? 0.03 * em_px : 0.0});
            } else if (c.isDigit() || c == '.') {
                int st = i;
                while (i < n && (s[i].isDigit() || s[i] == '.')) ++i;
                spans.push_back({s.mid(st, i - st), false, 0.0, 0.0});
            } else if (c == '+' || c == '-' || c == QChar(0x2212)) {
                bool is_unary = spans.empty();
                if (!is_unary) {
                    QString prev = spans.back().text.trimmed();
                    if (prev.endsWith("(") || prev.endsWith("[") || prev.endsWith("=") || prev.endsWith(","))
                        is_unary = true;
                }
                QString sym = (c == '-') ? QString(QChar(0x2212)) : QString(c); // True Unicode Minus U+2212
                double pad = is_unary ? (0.04 * em_px) : (0.20 * em_px);
                spans.push_back({sym, false, is_unary ? 0.0 : pad, pad});
                ++i;
            } else if (c == '=') {
                spans.push_back({"=", false, 0.24 * em_px, 0.24 * em_px});
                ++i;
            } else if (c == ',') {
                spans.push_back({",", false, 0.0, 0.20 * em_px});
                ++i;
            } else {
                bool is_greek_lower = (c.unicode() >= 0x03B1 && c.unicode() <= 0x03C9);
                spans.push_back({QString(c), is_greek_lower, 0.0, 0.0});
                ++i;
            }
        }
        return spans;
    }

    static std::string cleanLatexTop(std::string s) {
        while (!s.empty() && (s.front() == ' ' || s.front() == '$')) s.erase(s.begin());
        while (!s.empty() && (s.back() == ' ' || s.back() == '$')) s.pop_back();

        const std::vector<std::pair<std::string, std::string>> reps = {
            {"\\left(", "("}, {"\\right)", ")"}, {"\\left[", "["}, {"\\right]", "]"},
            {"\\left|", "|"}, {"\\right|", "|"}, {"\\,", " "}, {"\\;", " "},
            {"\\quad", "   "}, {"\\qquad", "     "}, {"\\!", ""},
            {"\\zeta", "ζ"}, {"\\omega_n", "ω_n"}, {"\\omega_d", "ω_d"}, {"\\omega", "ω"},
            {"\\sigma_d", "σ_d"}, {"\\sigma_a", "σ_a"}, {"\\sigma", "σ"},
            {"\\theta_a", "θ_a"}, {"\\theta", "θ"}, {"\\alpha_r", "α_r"}, {"\\alpha", "α"},
            {"\\Omega_c", "Ω_c"}, {"\\Omega", "Ω"}, {"\\Delta", "Δ"}, {"\\pi", "π"},
            {"\\tau", "τ"}, {"\\phi", "φ"}, {"\\infty", "∞"}, {"\\pm", "±"},
            {"\\approx", "≈"}, {"\\geq", "≥"}, {"\\ge", "≥"}, {"\\leq", "≤"}, {"\\le", "≤"},
            {"\\neq", "≠"}, {"\\ne", "≠"}, {"\\ll", "≪"}, {"\\gg", "≫"},
            {"\\Rightarrow", " ⇒ "}, {"\\implies", " ⇒ "}, {"\\iff", " ⇔ "}, {"\\Leftrightarrow", " ⇔ "},
            {"\\rightarrow", " → "}, {"\\to", " → "}, {"\\uparrow", "↑"}, {"\\downarrow", "↓"},
            {"\\sum", "∑"}, {"\\prod", "∏"}, {"\\angle", "∠"}, {"\\times", "×"},
            {"\\cdot", "·"}, {"\\dots", "…"}, {"\\mathcal{L}", "ℒ"}, {"\\mathcal{Z}", "ℤ"},
            {"\\%", "%"}, {"\\ln", "ln"}, {"\\log", "log"}, {"\\sin", "sin"},
            {"\\cos", "cos"}, {"\\tan", "tan"}, {"\\exp", "exp"}, {"\\lim", "lim"},
            {"\\max", "max"}, {"^\\circ", "°"}
        };
        for (const auto& [k, v] : reps) {
            size_t p = 0;
            while ((p = s.find(k, p)) != std::string::npos) {
                s.replace(p, k.size(), v);
                p += v.size();
            }
        }
        return s;
    }

    static std::pair<std::string, size_t> extractGroup(const std::string& s, size_t i) {
        while (i < s.size() && s[i] == ' ') ++i;
        if (i >= s.size()) return {"", i};
        if (s[i] == '{') {
            int depth = 0;
            size_t start = i + 1;
            while (i < s.size()) {
                if (s[i] == '{') ++depth;
                else if (s[i] == '}') {
                    --depth;
                    if (depth == 0) return {s.substr(start, i - start), i + 1};
                }
                ++i;
            }
            return {s.substr(start), s.size()};
        }
        size_t start = i;
        unsigned char c = static_cast<unsigned char>(s[i]);
        if (c >= 0x80) {
            ++i;
            while (i < s.size() && (static_cast<unsigned char>(s[i]) & 0xC0) == 0x80) ++i;
        } else {
            ++i;
        }
        return {s.substr(start, i - start), i};
    }

    static MathNode parseSegment(const std::string& s) {
        MathNode seq;
        seq.type = MathNode::SEQ;
        size_t i = 0;
        std::string cur_txt;

        auto flushText = [&]() {
            if (!cur_txt.empty()) {
                MathNode tn; tn.type = MathNode::TEXT; tn.text = QString::fromStdString(cur_txt);
                seq.children.push_back(tn);
                cur_txt.clear();
            }
        };

        while (i < s.size()) {
            if (s.compare(i, 5, "\\frac") == 0) {
                flushText();
                i += 5;
                auto [num_s, i1] = extractGroup(s, i);
                auto [den_s, i2] = extractGroup(s, i1);
                i = i2;
                MathNode fn; fn.type = MathNode::FRAC;
                fn.children.push_back(parseSegment(num_s));
                fn.children.push_back(parseSegment(den_s));
                seq.children.push_back(fn);
            } else if (s.compare(i, 5, "\\sqrt") == 0) {
                flushText();
                i += 5;
                if (i < s.size() && s[i] == '[') {
                    while (i < s.size() && s[i] != ']') ++i;
                    if (i < s.size()) ++i;
                }
                auto [rad_s, i1] = extractGroup(s, i);
                i = i1;
                MathNode sn; sn.type = MathNode::SQRT;
                sn.children.push_back(parseSegment(rad_s));
                seq.children.push_back(sn);
            } else if (s.compare(i, 7, "\\mathrm") == 0 || s.compare(i, 5, "\\text") == 0 || s.compare(i, 4, "\\big") == 0 || s.compare(i, 4, "\\Big") == 0) {
                size_t cmd_len = (s.compare(i, 7, "\\mathrm") == 0) ? 7 : ((s.compare(i, 5, "\\text") == 0) ? 5 : 4);
                i += cmd_len;
                if (i < s.size() && s[i] == '{') {
                    auto [inner_s, i1] = extractGroup(s, i);
                    cur_txt += inner_s;
                    i = i1;
                }
            } else if (s[i] == '^' || s[i] == '_') {
                char kind = s[i++];
                MathNode base_node;
                if (!cur_txt.empty()) {
                    size_t char_start = cur_txt.size() - 1;
                    while (char_start > 0 && (static_cast<unsigned char>(cur_txt[char_start]) & 0xC0) == 0x80) --char_start;
                    std::string last_ch = cur_txt.substr(char_start);
                    cur_txt.erase(char_start);
                    flushText();
                    base_node.type = MathNode::TEXT;
                    base_node.text = QString::fromStdString(last_ch);
                } else if (!seq.children.empty()) {
                    base_node = seq.children.back();
                    seq.children.pop_back();
                } else {
                    base_node.type = MathNode::TEXT;
                    base_node.text = "";
                }

                auto extractScriptArg = [&](size_t pos) -> std::pair<std::string, size_t> {
                    while (pos < s.size() && s[pos] == ' ') ++pos;
                    if (pos < s.size() && s[pos] == '{') {
                        return extractGroup(s, pos);
                    }
                    size_t st = pos;
                    if (kind == '_') {
                        while (pos < s.size() && std::isalnum(static_cast<unsigned char>(s[pos]))) ++pos;
                    } else {
                        if (pos < s.size() && (s[pos] == '+' || s[pos] == '-')) ++pos;
                        while (pos < s.size() && std::isdigit(static_cast<unsigned char>(s[pos]))) ++pos;
                    }
                    if (pos > st) return {s.substr(st, pos - st), pos};
                    return extractGroup(s, st);
                };

                auto [script_s, i1] = extractScriptArg(i);
                i = i1;
                MathNode sc_node = parseSegment(script_s);

                if (i < s.size() && ((kind == '^' && s[i] == '_') || (kind == '_' && s[i] == '^'))) {
                    char kind2 = s[i++];
                    std::swap(kind, kind2);
                    auto [script2_s, i2] = extractScriptArg(i);
                    std::swap(kind, kind2);
                    i = i2;
                    MathNode sc2_node = parseSegment(script2_s);
                    MathNode ss; ss.type = MathNode::SUPSUB;
                    ss.children.push_back(base_node);
                    ss.children.push_back(kind == '^' ? sc_node : sc2_node);
                    ss.children.push_back(kind == '_' ? sc_node : sc2_node);
                    seq.children.push_back(ss);
                } else {
                    MathNode sn;
                    sn.type = (kind == '^') ? MathNode::SUP : MathNode::SUB;
                    sn.children.push_back(base_node);
                    sn.children.push_back(sc_node);
                    seq.children.push_back(sn);
                }
            } else if (s[i] == '{' || s[i] == '}') {
                ++i;
            } else if (s[i] == '\\') {
                ++i;
            } else {
                cur_txt.push_back(s[i++]);
            }
        }
        flushText();
        return seq;
    }

    static QFont shrinkFont(const QFont& f, double ratio = 0.76) {
        QFont sf = f;
        sf.setPointSizeF(std::max(7.0, f.pointSizeF() * ratio));
        return sf;
    }

    NodeMetrics measureNode(const MathNode& n, const QFont& f) const {
        QFontMetricsF fm(f);
        const auto sm_f = safeMetrics(fm);
        switch (n.type) {
            case MathNode::TEXT: {
                QFont f_it = f; f_it.setItalic(true);
                QFontMetricsF fm_it(f_it);
                double w = 0.0;
                auto spans = tokenizeMathText(n.text, sm_f.em);
                for (const auto& sp : spans) {
                    w += sp.pre_pad + (sp.italic ? fm_it.horizontalAdvance(sp.text) : fm.horizontalAdvance(sp.text)) + sp.post_pad;
                }
                return {w, sm_f.asc, sm_f.desc};
            }
            case MathNode::SEQ: {
                double w = 0.0, asc = sm_f.asc, desc = sm_f.desc;
                for (const auto& c : n.children) {
                    auto m = measureNode(c, f);
                    w += m.w;
                    asc = std::max(asc, m.asc);
                    desc = std::max(desc, m.desc);
                }
                return {w, asc, desc};
            }
            case MathNode::FRAC: {
                QFont sf = shrinkFont(f, 0.90);
                auto nm = measureNode(n.children[0], sf);
                auto dm = measureNode(n.children[1], sf);
                double w = std::max(nm.w, dm.w) + 12.0;
                double gap = 3.0;
                double asc = sm_f.axis + nm.asc + nm.desc + gap;
                double desc = std::max(0.0, (dm.asc + dm.desc + gap) - sm_f.axis);
                return {w, asc, desc};
            }
            case MathNode::SQRT: {
                auto im = measureNode(n.children[0], f);
                return {im.w + 13.0, im.asc + 4.0, im.desc + 1.0};
            }
            case MathNode::SUP: {
                auto bm = measureNode(n.children[0], f);
                auto sm = measureNode(n.children[1], shrinkFont(f, 0.72));
                return {bm.w + sm.w + 1.5, std::max(bm.asc, bm.asc * 0.50 + sm.asc + sm.desc), bm.desc};
            }
            case MathNode::SUB: {
                auto bm = measureNode(n.children[0], f);
                auto sm = measureNode(n.children[1], shrinkFont(f, 0.72));
                return {bm.w + sm.w + 1.5, bm.asc, std::max(bm.desc, bm.desc * 0.35 + sm.asc + sm.desc)};
            }
            case MathNode::SUPSUB: {
                auto bm = measureNode(n.children[0], f);
                QFont sf = shrinkFont(f, 0.72);
                auto supm = measureNode(n.children[1], sf);
                auto subm = measureNode(n.children[2], sf);
                return {bm.w + std::max(supm.w, subm.w) + 2.0,
                        std::max(bm.asc, bm.asc * 0.50 + supm.asc + supm.desc),
                        std::max(bm.desc, bm.desc * 0.35 + subm.asc + subm.desc)};
            }
        }
        return {0, 10, 4};
    }

    double drawNode(QPainter& p, const MathNode& n, double x, double y_base, const QFont& f) const {
        QFontMetricsF fm(f);
        const auto sm_f = safeMetrics(fm);
        switch (n.type) {
            case MathNode::TEXT: {
                QFont f_it = f; f_it.setItalic(true);
                QFontMetricsF fm_it(f_it);
                double cur_x = x;
                auto spans = tokenizeMathText(n.text, sm_f.em);
                for (const auto& sp : spans) {
                    cur_x += sp.pre_pad;
                    p.setFont(sp.italic ? f_it : f);
                    p.drawText(QPointF(cur_x, y_base), sp.text);
                    cur_x += (sp.italic ? fm_it.horizontalAdvance(sp.text) : fm.horizontalAdvance(sp.text)) + sp.post_pad;
                }
                return cur_x - x;
            }
            case MathNode::SEQ: {
                double cur_x = x;
                for (const auto& c : n.children) cur_x += drawNode(p, c, cur_x, y_base, f);
                return cur_x - x;
            }
            case MathNode::FRAC: {
                QFont sf = shrinkFont(f, 0.90);
                auto nm = measureNode(n.children[0], sf);
                auto dm = measureNode(n.children[1], sf);
                double w = std::max(nm.w, dm.w) + 12.0;
                double y_bar = y_base - sm_f.axis;
                double gap = 3.0;
                p.save();
                p.setPen(QPen(fg_color, 1.1));
                p.drawLine(QPointF(x + 2.0, y_bar), QPointF(x + w - 2.0, y_bar));
                p.restore();
                drawNode(p, n.children[0], x + 0.5 * (w - nm.w), y_bar - gap - nm.desc, sf);
                drawNode(p, n.children[1], x + 0.5 * (w - dm.w), y_bar + gap + dm.asc, sf);
                return w;
            }
            case MathNode::SQRT: {
                auto im = measureNode(n.children[0], f);
                double top_y = y_base - im.asc - 2.5;
                double bot_y = y_base + im.desc;
                double mid_y = 0.5 * (top_y + bot_y);
                p.save();
                p.setPen(QPen(fg_color, 1.15));
                QPainterPath rpath;
                rpath.moveTo(x + 1.0, mid_y + 2.0);
                rpath.lineTo(x + 3.5, mid_y);
                rpath.lineTo(x + 6.5, bot_y);
                rpath.lineTo(x + 10.5, top_y);
                rpath.lineTo(x + 11.5 + im.w, top_y);
                p.drawPath(rpath);
                p.restore();
                drawNode(p, n.children[0], x + 11.0, y_base, f);
                return im.w + 13.0;
            }
            case MathNode::SUP: {
                double bw = drawNode(p, n.children[0], x, y_base, f);
                auto bm = measureNode(n.children[0], f);
                QFont sf = shrinkFont(f, 0.72);
                double sw = drawNode(p, n.children[1], x + bw + 0.5, y_base - bm.asc * 0.46, sf);
                return bw + sw + 1.5;
            }
            case MathNode::SUB: {
                double bw = drawNode(p, n.children[0], x, y_base, f);
                auto bm = measureNode(n.children[0], f);
                QFont sf = shrinkFont(f, 0.72);
                auto sm = measureNode(n.children[1], sf);
                double sw = drawNode(p, n.children[1], x + bw + 0.5, y_base + bm.desc * 0.30 + sm.asc * 0.60, sf);
                return bw + sw + 1.5;
            }
            case MathNode::SUPSUB: {
                double bw = drawNode(p, n.children[0], x, y_base, f);
                auto bm = measureNode(n.children[0], f);
                QFont sf = shrinkFont(f, 0.72);
                auto subm = measureNode(n.children[2], sf);
                double sup_w = drawNode(p, n.children[1], x + bw + 0.5, y_base - bm.asc * 0.46, sf);
                double sub_w = drawNode(p, n.children[2], x + bw + 0.5, y_base + bm.desc * 0.30 + subm.asc * 0.60, sf);
                return bw + std::max(sup_w, sub_w) + 2.0;
            }
        }
        return 0.0;
    }

    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setRenderHint(QPainter::TextAntialiasing, true);
        if (bg_color.isValid() && bg_color.alpha() > 0) {
            p.fillRect(rect(), bg_color);
        }
        p.setPen(fg_color);

        QFont f = makeLatexBaseFont(base_pt + 1);
        auto m = measureNode(root_node, f);
        double y_base = 0.5 * (height() + m.asc - m.desc);
        drawNode(p, root_node, 8.0, y_base, f);
    }
};

// Circular 'ⓘ' Info Tooltip Badge
class InfoBadgeButton : public QPushButton {
public:
    explicit InfoBadgeButton(const QString& title, const QString& desc, const QString& ref = "", QWidget* parent = nullptr)
        : QPushButton("i", parent)
    {
        setFixedSize(22, 22);
        setCursor(Qt::WhatsThisCursor);
        QString tip = QString("<div style='background-color:#0f172a; color:#f8fafc; padding:6px;'>"
                              "<b>ⓘ %1</b> %2<hr/>%3</div>")
                      .arg(title,
                           ref.isEmpty() ? "" : QString("<span style='color:#34d399;'>[%1]</span>").arg(ref),
                           desc.toHtmlEscaped().replace("\n", "<br/>"));
        setToolTip(tip);
        setStyleSheet(
            "QPushButton { background-color: #111827; color: #38bdf8; border: 1.5px solid #38bdf8; border-radius: 11px; font-family: Georgia; font-weight: bold; font-style: italic; font-size: 11px; }"
            "QPushButton:hover { background-color: #1e3a5f; color: #ffffff; border-color: #34d399; }"
        );
        connect(this, &QPushButton::clicked, this, [this, tip]() {
            QToolTip::showText(QCursor::pos(), tip, this);
        });
    }
};

// =============================================================================
// Engine 0: Polynomial Algebra & Root Solver (Durand-Kerner + Algebraic Parser)
// =============================================================================
inline Poly polyTrim(const Poly& p, double tol = 1e-13) {
    auto it = std::find_if(p.begin(), p.end(), [tol](const Complex& c) { return std::abs(c) > tol; });
    if (it == p.end()) return {Complex(0.0, 0.0)};
    return Poly(it, p.end());
}

inline Poly polyAdd(const Poly& a, const Poly& b) {
    size_t n = std::max(a.size(), b.size());
    Poly res(n, Complex(0.0, 0.0));
    for (size_t i = 0; i < a.size(); ++i) res[n - a.size() + i] += a[i];
    for (size_t i = 0; i < b.size(); ++i) res[n - b.size() + i] += b[i];
    return res;
}

inline Poly polySub(const Poly& a, const Poly& b) {
    Poly nb = b;
    for (auto& c : nb) c = -c;
    return polyAdd(a, nb);
}

inline Poly polyMul(const Poly& a, const Poly& b) {
    if (a.empty() || b.empty()) return {Complex(0.0, 0.0)};
    Poly res(a.size() + b.size() - 1, Complex(0.0, 0.0));
    for (size_t i = 0; i < a.size(); ++i)
        for (size_t j = 0; j < b.size(); ++j)
            res[i + j] += a[i] * b[j];
    return res;
}

inline Poly polyScale(const Poly& a, Complex s) {
    Poly res = a;
    for (auto& c : res) c *= s;
    return res;
}

inline Complex polyVal(const Poly& p, Complex x) {
    Complex acc(0.0, 0.0);
    for (const auto& c : p) acc = acc * x + c;
    return acc;
}

inline Poly polyDeriv(const Poly& p) {
    if (p.size() <= 1) return {Complex(0.0, 0.0)};
    size_t deg = p.size() - 1;
    Poly d(deg);
    for (size_t i = 0; i < deg; ++i) {
        d[i] = p[i] * Complex(double(deg - i), 0.0);
    }
    return polyTrim(d);
}

inline Poly polyFromRoots(const std::vector<Complex>& roots) {
    Poly res = {Complex(1.0, 0.0)};
    for (const auto& r : roots) res = polyMul(res, {Complex(1.0, 0.0), -r});
    return res;
}

inline std::vector<Complex> solvePolynomialRoots(
    const Poly& coeffs,
    const std::vector<Complex>* warm_start = nullptr)
{
    Poly p = polyTrim(coeffs);
    if (p.size() <= 1) return {};
    size_t deg = p.size() - 1;
    Complex lead = p[0];
    if (std::abs(lead) < 1e-14) return {};
    for (auto& c : p) c /= lead;

    // Exact closed-form for degree 1
    if (deg == 1) {
        Complex r = -p[1];
        if (std::abs(r.imag()) < 1e-9) r.imag(0.0);
        if (std::abs(r.real()) < 1e-12) r.real(0.0);
        return {r};
    }

    // Exact closed-form for degree 2
    if (deg == 2) {
        Complex b = p[1], c = p[2];
        Complex disc = std::sqrt(b * b - 4.0 * c);
        std::vector<Complex> r2 = {(-b + disc) / 2.0, (-b - disc) / 2.0};
        for (auto& r : r2) {
            if (std::abs(r.imag()) < 1e-9) r.imag(0.0);
            if (std::abs(r.real()) < 1e-12) r.real(0.0);
        }
        return r2;
    }

    std::vector<Complex> roots(deg);
    if (warm_start && warm_start->size() == deg) {
        for (size_t k = 0; k < deg; ++k) {
            double angle = 2.0 * M_PI * k / static_cast<double>(deg) + 0.137;
            roots[k] = (*warm_start)[k] + std::polar(1e-7, angle);
        }
    } else {
        // Cauchy bound radius for better initial guess scaling
        double max_c = 0.0;
        for (size_t k = 1; k <= deg; ++k) max_c = std::max(max_c, std::abs(p[k]));
        double radius = 0.4 + std::pow(std::max(max_c, 0.1), 1.0 / static_cast<double>(deg));
        for (size_t k = 0; k < deg; ++k) {
            double angle = 2.0 * M_PI * k / static_cast<double>(deg) + 0.137;
            roots[k] = std::polar(radius, angle);
        }
    }

    // Helper lambda for Horner evaluation of monic polynomial p
    auto evalMonic = [&p](Complex x) -> Complex {
        Complex acc(0.0, 0.0);
        for (const auto& c : p) acc = acc * x + c;
        return acc;
    };

    // Durand-Kerner iterations
    for (int iter = 0; iter < 400; ++iter) {
        double max_delta = 0.0;
        for (size_t i = 0; i < deg; ++i) {
            Complex val = evalMonic(roots[i]);
            Complex denom(1.0, 0.0);
            for (size_t j = 0; j < deg; ++j) {
                if (i != j) {
                    Complex diff = roots[i] - roots[j];
                    if (std::abs(diff) < 1e-14) diff = Complex(1e-14, 1e-14);
                    denom *= diff;
                }
            }
            Complex delta = val / denom;
            roots[i] -= delta;
            max_delta = std::max(max_delta, std::abs(delta));
        }
        if (max_delta < 1e-14) break;
    }

    // Multiplicity Cluster Detection + (m-1)th Derivative Newton Polishing
    // For an m-fold root, P(s) suffers catastrophic cancellation O(eps^(1/m)),
    // whereas P^(m-1)(s) has a simple root at the exact same location with O(eps) accuracy!
    std::vector<bool> visited(deg, false);
    for (size_t i = 0; i < deg; ++i) {
        if (visited[i]) continue;
        std::vector<size_t> cluster = {i};
        visited[i] = true;
        Complex sum = roots[i];
        // Expand cluster transitively within radius 2.5e-2
        for (size_t k = 0; k < cluster.size(); ++k) {
            Complex curr = roots[cluster[k]];
            for (size_t j = 0; j < deg; ++j) {
                if (!visited[j] && std::abs(roots[j] - curr) < 2.5e-2) {
                    cluster.push_back(j);
                    visited[j] = true;
                    sum += roots[j];
                }
            }
        }
        size_t m = cluster.size();
        if (m > 1) {
            Complex centroid = sum / static_cast<double>(m);

            // Track the best (smallest) residual among the individual unmerged roots in this cluster.
            // If the roots are actually distinct simple roots (like -1 and -1.01), Durand-Kerner
            // already converged each one to ~1e-15, whereas the midpoint centroid has |P| ~ 2.5e-5!
            double min_root_resid = 1e300;
            double max_root_resid = 0.0;
            for (size_t idx : cluster) {
                double r_res = std::abs(evalMonic(roots[idx]));
                min_root_resid = std::min(min_root_resid, r_res);
                max_root_resid = std::max(max_root_resid, r_res);
            }

            // Build derivatives P^(1), P^(2), ..., P^(m-1)
            std::vector<Poly> derivs;
            derivs.push_back(p); // derivs[0] = P(s)
            for (size_t d = 1; d < m; ++d) {
                const Poly& prev = derivs.back();
                if (prev.size() <= 1) break;
                size_t cur_deg = prev.size() - 1;
                Poly next_dp(cur_deg);
                for (size_t idx = 0; idx < cur_deg; ++idx) {
                    next_dp[idx] = prev[idx] * static_cast<double>(cur_deg - idx);
                }
                derivs.push_back(next_dp);
            }

            // Polish centroid using Newton-Raphson on dp(s) = P^(m-1)(s) = 0
            const Poly& dp = derivs.back();
            Complex polished = centroid;
            for (int n_it = 0; n_it < 8; ++n_it) {
                Complex f_val(0.0, 0.0), df_val(0.0, 0.0);
                size_t dp_deg = dp.size() - 1;
                for (size_t idx = 0; idx <= dp_deg; ++idx) {
                    f_val = f_val * polished + dp[idx];
                    if (idx < dp_deg) {
                        df_val = df_val * polished + dp[idx] * static_cast<double>(dp_deg - idx);
                    }
                }
                if (std::abs(df_val) < 1e-14) break;
                Complex step = f_val / df_val;
                polished -= step;
                if (std::abs(step) < 1e-15) break;
            }

            // Clean tiny imaginary/decimal dust on the polished root
            if (std::abs(polished.imag()) < 1e-7) polished.imag(0.0);
            double r_round = std::round(polished.real() * 1e6) / 1e6;
            double i_round = std::round(polished.imag() * 1e6) / 1e6;
            if (std::abs(polished.real() - r_round) < 1e-7) polished.real(r_round);
            if (std::abs(polished.imag() - i_round) < 1e-7) polished.imag(i_round);

            // Evaluate |P(polished)| and check all intermediate derivatives P^(k)(polished)
            double polished_resid = std::abs(evalMonic(polished));
            bool all_derivs_vanish = true;
            for (size_t d = 0; d < derivs.size(); ++d) {
                Complex dv(0.0, 0.0);
                for (const auto& c : derivs[d]) dv = dv * polished + c;
                // A true m-fold root has P^(d)(r) == 0 (within floating-point cancellation noise ~1e-9)
                if (std::abs(dv) > 1e-8) {
                    all_derivs_vanish = false;
                    break;
                }
            }

            // Accept the merge ONLY if:
            // 1. All derivatives 0..m-1 vanish at `polished` (|P^(d)(polished)| <= 1e-8), AND
            // 2. `polished` is at least as accurate as the unmerged roots (or within machine noise 1e-11)
            if (all_derivs_vanish &&
                std::abs(polished - centroid) < 5e-2 &&
                polished_resid <= std::max(max_root_resid * 2.0, 1e-11)) {
                for (size_t idx : cluster) roots[idx] = polished;
            }
        }
    }

    for (auto& r : roots) {
        if (std::abs(r.imag()) < 1e-7) r.imag(0.0);
        if (std::abs(r.real()) < 1e-12) r.real(0.0);
    }
    return roots;
}

class AlgebraicPolyParser {
    std::string src;
    size_t pos = 0;
    void skipSpaces() { while (pos < src.size() && std::isspace(static_cast<unsigned char>(src[pos]))) ++pos; }
public:
    explicit AlgebraicPolyParser(std::string s) : src(std::move(s)) {}
    Poly parse() {
        Poly res = parseExpr();
        skipSpaces();
        if (pos < src.size()) throw std::runtime_error("Unexpected token near '" + src.substr(pos) + "'");
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
            } else break;
        }
        return left;
    }
    Poly parseTerm() {
        Poly left = parsePower();
        while (true) {
            skipSpaces();
            if (pos < src.size() && src[pos] == '*') { ++pos; left = polyMul(left, parsePower()); }
            else if (pos < src.size() && src[pos] == '/') {
                ++pos;
                Poly right = polyTrim(parsePower());
                if (right.size() != 1 || std::abs(right[0]) < 1e-14) throw std::runtime_error("Scalar denominator required");
                left = polyScale(left, 1.0 / right[0]);
            } else if (pos < src.size() && (src[pos] == '(' || src[pos] == 's' || src[pos] == 'S' || src[pos] == 'z' || src[pos] == 'Z')) {
                left = polyMul(left, parsePower());
            } else break;
        }
        return left;
    }
    Poly parsePower() {
        Poly base = parseUnary();
        skipSpaces();
        if (pos < src.size() && src[pos] == '^') {
            ++pos;
            Poly exp_p = polyTrim(parseUnary());
            int n = static_cast<int>(std::round(exp_p[0].real()));
            if (n < 0) throw std::runtime_error("Non-negative integer exponent required");
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
            if (pos >= src.size() || src[pos] != ')') throw std::runtime_error("Missing ')'");
            ++pos;
            return inner;
        }
        if (src[pos] == 's' || src[pos] == 'S' || src[pos] == 'z' || src[pos] == 'Z') {
            ++pos;
            return {Complex(1.0, 0.0), Complex(0.0, 0.0)};
        }
        if (src[pos] == 'i' || src[pos] == 'j') { ++pos; return {Complex(0.0, 1.0)}; }
        if (std::isdigit(static_cast<unsigned char>(src[pos])) || src[pos] == '.') {
            size_t st = pos;
            while (pos < src.size() && (std::isdigit(static_cast<unsigned char>(src[pos])) || src[pos] == '.')) ++pos;
            // Consume scientific notation exponent (e.g. 1e5, 1.25e+05, 2.5e-05)
            if (pos < src.size() && (src[pos] == 'e' || src[pos] == 'E')) {
                size_t exp_pos = pos + 1;
                if (exp_pos < src.size() && (src[exp_pos] == '+' || src[exp_pos] == '-')) ++exp_pos;
                if (exp_pos < src.size() && std::isdigit(static_cast<unsigned char>(src[exp_pos]))) {
                    pos = exp_pos;
                    while (pos < src.size() && std::isdigit(static_cast<unsigned char>(src[pos]))) ++pos;
                }
            }
            double v = std::stod(src.substr(st, pos - st));
            if (pos < src.size() && (src[pos] == 'i' || src[pos] == 'j')) { ++pos; return {Complex(0.0, v)}; }
            return {Complex(v, 0.0)};
        }
        throw std::runtime_error(std::string("Unexpected char '") + src[pos] + "'");
    }
};

struct ParsedPolynomial {
    Poly coeffs;
    std::vector<Complex> roots;
    double leading_gain = 1.0;
};

inline ParsedPolynomial parsePolynomialInput(const std::string& raw_str) {
    std::string s = raw_str;
    bool has_var_or_ops = false;
    for (char c : s) {
        if (c == 's' || c == 'S' || c == 'z' || c == 'Z' || c == 'x' || c == 'X' ||
            c == '(' || c == ')' || c == '*' || c == '^') {
            has_var_or_ops = true;
            break;
        }
    }

    if (!has_var_or_ops) {
        // Normalize commas and brackets into spaces
        std::string norm = s;
        for (char& c : norm) {
            if (c == ',' || c == '[' || c == ']') c = ' ';
        }
        std::istringstream iss(norm);
        std::vector<std::string> tokens;
        std::string tok;
        while (iss >> tok) tokens.push_back(tok);

        // Treat as a descending coefficient list if 2+ numbers (e.g. "2 4 -10 3")
        // or if commas/brackets were explicitly used
        if (tokens.size() >= 2 || s.find(',') != std::string::npos || s.find('[') != std::string::npos) {
            Poly coeffs;
            bool all_valid = true;
            for (const auto& item : tokens) {
                try {
                    size_t idx = 0;
                    double v = std::stod(item, &idx);
                    if (idx != item.size()) { all_valid = false; break; }
                    coeffs.push_back(Complex(v, 0.0));
                } catch (...) {
                    all_valid = false;
                    break;
                }
            }
            if (all_valid && !coeffs.empty()) {
                coeffs = polyTrim(coeffs);
                if (coeffs.empty() || (coeffs.size() == 1 && std::abs(coeffs[0]) <= 1e-14)) {
                    return {{Complex(0.0, 0.0)}, {}, 0.0};
                }
                double lead = coeffs[0].real();
                return {coeffs, solvePolynomialRoots(coeffs), lead};
            }
        }
    }

    AlgebraicPolyParser parser(s);
    Poly coeffs = parser.parse();
    if (coeffs.empty() || (coeffs.size() == 1 && std::abs(coeffs[0]) <= 1e-14)) {
        return {{Complex(0.0, 0.0)}, {}, 0.0};
    }
    double lead = coeffs[0].real();
    return {coeffs, solvePolynomialRoots(coeffs), lead};
}

inline std::string formatPolynomialFromRoots(const std::vector<Complex>& roots, const std::string& var_name = "s", double scale = 1.0) {
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
        if (out.empty()) out += (is_neg ? "-" : "") + term;
        else out += (is_neg ? " - " : " + ") + term;
    }
    return out.empty() ? "1" : out;
}

// =============================================================================
// Symbolic 1D Rational Function Engine: R(v) = num(v) / den(v)
// =============================================================================
inline std::pair<Poly, Poly> polyDivMod(const Poly& u, const Poly& v) {
    Poly num = polyTrim(u, 1e-11);
    Poly den = polyTrim(v, 1e-11);
    if (den.empty() || (den.size() == 1 && std::abs(den[0]) < 1e-12)) {
        return {{Complex(0.0, 0.0)}, num};
    }
    if (num.size() < den.size()) return {{Complex(0.0, 0.0)}, num};

    size_t deg_q = num.size() - den.size();
    Poly q(deg_q + 1, Complex(0.0, 0.0));
    Poly rem = num;

    while (rem.size() >= den.size() && !(rem.size() == 1 && std::abs(rem[0]) < 1e-11)) {
        size_t shift = rem.size() - den.size();
        Complex factor = rem[0] / den[0];
        q[q.size() - 1 - shift] = factor;
        for (size_t i = 0; i < den.size(); ++i) {
            rem[i] -= factor * den[i];
        }
        rem = polyTrim(rem, 1e-10);
        if (shift == 0) break;
    }
    return {polyTrim(q, 1e-11), polyTrim(rem, 1e-11)};
}

inline Poly polyGCD(Poly a, Poly b) {
    a = polyTrim(a, 1e-10);
    b = polyTrim(b, 1e-10);
    // CRITICAL: If either polynomial is a constant (degree 0), their GCD is 1!
    if (a.size() <= 1 || b.size() <= 1) {
        return {Complex(1.0, 0.0)};
    }
    for (int iter = 0; iter < 32; ++iter) {
        if (b.size() == 1 && std::abs(b[0]) < 1e-9) break;
        if (b.size() == 1) {
            // Remainder reached a non-zero constant -> polynomials are coprime (GCD = 1)
            return {Complex(1.0, 0.0)};
        }
        auto [q, r] = polyDivMod(a, b);
        double norm_a = 0.0;
        for (const auto& c : a) norm_a = std::max(norm_a, std::abs(c));
        bool r_is_zero = (r.size() == 1 && std::abs(r[0]) <= 1e-7 * std::max(1.0, norm_a));
        if (r_is_zero) {
            a = b;
            break;
        }
        a = b;
        b = r;
    }
    if (!a.empty() && std::abs(a[0]) > 1e-12) {
        a = polyScale(a, Complex(1.0, 0.0) / a[0]);
    }
    return a;
}

struct SymRat {
    Poly num; // Descending powers: c_n * x^n + ... + c_0
    Poly den;

    SymRat(double v = 0.0) : num({Complex(v, 0.0)}), den({Complex(1.0, 0.0)}) {}
    SymRat(Complex v) : num({v}), den({Complex(1.0, 0.0)}) {}
    SymRat(const Poly& n, const Poly& d = {Complex(1.0, 0.0)}) : num(n), den(d) { normalize(); }

    static SymRat variable() {
        return SymRat({Complex(1.0, 0.0), Complex(0.0, 0.0)}, {Complex(1.0, 0.0)});
    }

    bool isZero(double tol = 1e-9) const {
        for (const auto& c : num) if (std::abs(c) > tol) return false;
        return true;
    }

    bool isConstant(double tol = 1e-9) const {
        return num.size() <= 1 && den.size() <= 1 && std::abs(den[0]) > tol;
    }

    Complex toConstant() const {
        if (den.empty() || std::abs(den.back()) < 1e-15) return {0.0, 0.0};
        return num.empty() ? Complex(0.0, 0.0) : (num.back() / den.back());
    }

    Complex eval(Complex x) const {
        Complex d = polyVal(den, x);
        if (std::abs(d) < 1e-15) return {std::numeric_limits<double>::quiet_NaN(), 0.0};
        return polyVal(num, x) / d;
    }

    void normalize() {
        num = polyTrim(num, 1e-10);
        den = polyTrim(den, 1e-10);
        if (den.empty() || (den.size() == 1 && std::abs(den[0]) < 1e-14)) {
            den = {Complex(1.0, 0.0)};
            return;
        }
        if (isZero()) {
            num = {Complex(0.0, 0.0)};
            den = {Complex(1.0, 0.0)};
            return;
        }
        Poly g = polyGCD(num, den);
        if (g.size() > 1) {
            num = polyDivMod(num, g).first;
            den = polyDivMod(den, g).first;
        }
        // Normalize so leading denominator coefficient is +1 (or real positive)
        Complex lead = den[0];
        if (std::abs(lead) > 1e-12) {
            num = polyScale(num, Complex(1.0, 0.0) / lead);
            den = polyScale(den, Complex(1.0, 0.0) / lead);
        }
        for (auto& c : num) {
            if (std::abs(c.real()) < 1e-10) c.real(0.0);
            if (std::abs(c.imag()) < 1e-10) c.imag(0.0);
        }
        for (auto& c : den) {
            if (std::abs(c.real()) < 1e-10) c.real(0.0);
            if (std::abs(c.imag()) < 1e-10) c.imag(0.0);
        }
    }

    friend SymRat operator+(const SymRat& a, const SymRat& b) {
        return SymRat(polyAdd(polyMul(a.num, b.den), polyMul(b.num, a.den)), polyMul(a.den, b.den));
    }
    friend SymRat operator-(const SymRat& a, const SymRat& b) {
        return SymRat(polySub(polyMul(a.num, b.den), polyMul(b.num, a.den)), polyMul(a.den, b.den));
    }
    friend SymRat operator*(const SymRat& a, const SymRat& b) {
        return SymRat(polyMul(a.num, b.num), polyMul(a.den, b.den));
    }
    friend SymRat operator/(const SymRat& a, const SymRat& b) {
        if (b.isZero()) throw std::runtime_error("Symbolic division by zero");
        return SymRat(polyMul(a.num, b.den), polyMul(a.den, b.num));
    }

    static std::string polyToStr(const Poly& p, const std::string& var, bool latex) {
        int deg = static_cast<int>(p.size()) - 1;
        if (deg < 0) return "0";
        std::string out;
        for (int i = 0; i <= deg; ++i) {
            Complex c = p[i];
            int pwr = deg - i;
            if (std::abs(c) < 1e-9) continue;

            std::string term;
            bool is_neg = false;
            if (std::abs(c.imag()) < 1e-8) {
                double rv = c.real();
                is_neg = (rv < 0.0);
                double av = std::abs(rv);
                if (pwr == 0 || std::abs(av - 1.0) > 1e-7) term = PRECISION.fmt(av);
            } else {
                term = "(" + PRECISION.fmtComplex(c) + ")";
            }

            if (pwr > 0) {
                term += var;
                if (pwr > 1) term += latex ? ("^{" + std::to_string(pwr) + "}") : ("^" + std::to_string(pwr));
            }
            if (term.empty()) term = "1";

            if (out.empty()) {
                out = (is_neg ? "-" : "") + term;
            } else {
                out += (is_neg ? " - " : " + ") + term;
            }
        }
        return out.empty() ? "0" : out;
    }

    std::string toString(const std::string& var = "K") const {
        std::string ns = polyToStr(num, var, false);
        if (den.size() <= 1) return ns;
        std::string ds = polyToStr(den, var, false);
        bool n_par = (num.size() > 1);
        return (n_par ? "(" + ns + ")" : ns) + " / (" + ds + ")";
    }

    std::string toLatex(const std::string& var = "K") const {
        std::string ns = polyToStr(num, var, true);
        if (den.size() <= 1) return ns;
        std::string ds = polyToStr(den, var, true);
        return "\\frac{" + ns + "}{" + ds + "}";
    }
};

inline std::string formatTransferFunctionLatex(const std::vector<Complex>& poles, const std::vector<Complex>& zeros, double gain, const std::string& var_name = "s", const std::string& sym = "G") {
    std::string den_s = formatPolynomialFromRoots(poles, var_name, 1.0);
    std::string num_s;
    if (zeros.empty()) {
        num_s = PRECISION.fmt(gain, false, true);
    } else {
        std::string pz = formatPolynomialFromRoots(zeros, var_name, 1.0);
        if (std::abs(gain - 1.0) < 1e-9) num_s = pz;
        else num_s = PRECISION.fmt(gain, false, true) + "(" + pz + ")";
    }
    return sym + "(" + var_name + ") = \\frac{" + num_s + "}{" + den_s + "}";
}

// =============================================================================
// Engine 0B & Engine 1: Nise Ch. 4 Equations, Root Locus & Damping Metrics
// =============================================================================
inline double niseZetaFromOs(double os_pct) {
    if (os_pct <= 0.0) return 1.0;
    if (os_pct >= 100.0) return 0.0;
    double ln_os = std::log(os_pct / 100.0);
    return -ln_os / std::sqrt(PI * PI + ln_os * ln_os);
}

inline double niseOsFromZeta(double zeta) {
    if (zeta >= 1.0) return 0.0;
    if (zeta <= 0.0) return 100.0;
    return 100.0 * std::exp(-zeta * PI / std::sqrt(std::max(1e-12, 1.0 - zeta * zeta)));
}

inline double niseNormalizedTrFromZeta(double zeta) {
    double z = std::max(0.0, zeta);
    return 1.76 * z * z * z - 0.417 * z * z + 1.039 * z + 1.0;
}

inline double niseTrFromZetaWn(double zeta, double wn) {
    return niseNormalizedTrFromZeta(zeta) / std::max(wn, 1e-9);
}

inline std::vector<Complex> polesFromZetaWn(double zeta, double wn) {
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

inline std::vector<Complex> zPolesFromZetaWn(double zeta, double wn, double ts) {
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

inline DominantMetrics computeDominantSMetrics(const std::vector<Complex>& poles, double deriv_filter_n = 20.0) {
    if (poles.empty()) return {};
    std::vector<Complex> non_origin;
    for (const auto& p : poles) if (std::abs(p) > 1e-8) non_origin.push_back(p);
    if (non_origin.empty()) return {0.0, 0.0, 0.0, Complex(0.0, 0.0)};

    double cutoff = -0.85 * std::max(deriv_filter_n, 15.0);
    std::vector<Complex> candidates;
    for (const auto& p : non_origin)
        if (!(std::abs(p.imag()) < 1e-4 && p.real() <= cutoff)) candidates.push_back(p);
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

inline DominantMetrics computeDominantZMetrics(const std::vector<Complex>& z_poles, double ts = 0.05) {
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

inline double computeAngleDeficiency(double net_angle_deg) {
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
    bool at_zero = false;
};

// Note: matches Python evaluate_transfer_function where Root Locus angle/mag conditions
// are evaluated on the monic ratio prod(v - z_i)/prod(v - p_j) when checking K = 1/|G(v)|.
inline TFEvalResult evaluateTransferFunction(
    const std::vector<Complex>& poles,
    const std::vector<Complex>& zeros,
    Complex test_pt,
    double gain = 1.0)
{
    Complex num_val(gain, 0.0);
    double num_mag = std::abs(gain);
    double zero_ang = (gain >= 0.0) ? 0.0 : PI;
    bool hit_zero = false;
    for (const auto& z : zeros) {
        Complex d = test_pt - z;
        double md = std::abs(d);
        if (md < 1e-11) {
            hit_zero = true;
            num_val = Complex(0.0, 0.0);
            num_mag = 0.0;
        } else {
            num_val *= d;
            num_mag *= md;
            zero_ang += std::arg(d);
        }
    }

    Complex den_val(1.0, 0.0);
    double den_mag = 1.0;
    double pole_ang = 0.0;
    for (const auto& p : poles) {
        Complex d = test_pt - p;
        double md = std::abs(d);
        if (md < 1e-11) {
            return {180.0, std::numeric_limits<double>::infinity(), 0.0, Complex(0.0, 0.0), true, hit_zero};
        }
        den_val *= d;
        den_mag *= md;
        pole_ang += std::arg(d);
    }

    if (hit_zero) {
        return {180.0, 0.0, std::numeric_limits<double>::infinity(), Complex(0.0, 0.0), false, true};
    }

    Complex val = num_val / den_val;
    double mag = num_mag / den_mag;
    double k_rl = (mag > 1e-15) ? (1.0 / mag) : std::numeric_limits<double>::infinity();
    double net_deg = (zero_ang - pole_ang) * (180.0 / PI);
    net_deg = std::fmod(net_deg + 180.0, 360.0);
    if (net_deg < 0.0) net_deg += 360.0;
    net_deg -= 180.0;
    return {net_deg, mag, k_rl, val, false, false};
}

inline std::vector<std::vector<Complex>> computeRootLocusBranches(
    const std::vector<Complex>& poles,
    const std::vector<Complex>& zeros,
    int num_steps = 360)
{
    size_t deg = std::max(poles.size(), zeros.size());
    if (deg == 0) return {};

    Poly den_p = polyFromRoots(poles);
    Poly num_p = polyFromRoots(zeros);
    Poly den_pad(deg + 1, Complex(0.0, 0.0));
    Poly num_pad(deg + 1, Complex(0.0, 0.0));
    for (size_t i = 0; i < den_p.size(); ++i) den_pad[deg + 1 - den_p.size() + i] = den_p[i];
    for (size_t i = 0; i < num_p.size(); ++i) num_pad[deg + 1 - num_p.size() + i] = num_p[i];

    // Compute characteristic gain scale using geometric mean of pole/zero distances
    // (Prevents a single distant zero like z = -30 from blowing up k_center = 30^5!)
    double span = 1.0;
    double log_pole_sum = 0.0; int n_nz_poles = 0;
    double log_zero_sum = 0.0; int n_nz_zeros = 0;
    for (const auto& p : poles) {
        double a = std::abs(p);
        span = std::max(span, a);
        if (a > 1e-3) { log_pole_sum += std::log(a); n_nz_poles++; }
    }
    for (const auto& z : zeros) {
        double a = std::abs(z);
        span = std::max(span, a);
        if (a > 1e-3) { log_zero_sum += std::log(a); n_nz_zeros++; }
    }
    double r_pole = (n_nz_poles > 0) ? std::exp(log_pole_sum / n_nz_poles) : 1.0;
    double r_zero = (n_nz_zeros > 0) ? std::exp(log_zero_sum / n_nz_zeros) : r_pole;
    double k_center = std::pow(r_pole, static_cast<double>(poles.size())) /
                      std::max(1e-12, std::pow(r_zero, static_cast<double>(zeros.size())));
    k_center = std::clamp(k_center, 1e-4, 1e8);

    std::vector<double> k_vals;
    k_vals.reserve(num_steps + 128);
    k_vals.push_back(0.0);
    for (int i = 0; i < num_steps; ++i) {
        double exp_v = -5.2 + 10.4 * double(i) / double(num_steps - 1);
        k_vals.push_back(k_center * std::pow(10.0, exp_v));
    }

    // Inject exact Breakaway / Break-In gains Kb (and dense micro-steps around Kb)
    if (!poles.empty()) {
        Poly dD = polyDeriv(den_p);
        Poly dN = polyDeriv(num_p);
        Poly char_brk = polySub(polyMul(num_p, dD), polyMul(den_p, dN));
        auto cand_roots = solvePolynomialRoots(char_brk);
        for (const auto& r : cand_roots) {
            if (std::abs(r.imag()) > 1e-3) continue;
            Complex n_val = polyVal(num_p, Complex(r.real(), 0.0));
            Complex d_val = polyVal(den_p, Complex(r.real(), 0.0));
            if (std::abs(n_val) < 1e-9) continue;
            double kb = -(d_val / n_val).real();
            if (kb > 1e-6 && std::isfinite(kb)) {
                for (double delta : {-0.02, -0.005, -0.001, -1e-5, 0.0, 1e-5, 0.001, 0.005, 0.02}) {
                    double k_inj = kb * (1.0 + delta);
                    if (k_inj > 0.0) k_vals.push_back(k_inj);
                }
            }
        }
    }
    std::sort(k_vals.begin(), k_vals.end());

    // Helper to solve and match roots at a given K to the previous root ordering
    auto solveAndMatch = [&](double k, const std::vector<Complex>& prev) -> std::vector<Complex> {
        Poly char_poly = polyAdd(den_pad, polyScale(num_pad, Complex(k, 0.0)));
        auto roots = solvePolynomialRoots(char_poly, prev.empty() ? nullptr : &prev);
        if (roots.size() != deg) roots = solvePolynomialRoots(char_poly, nullptr);
        if (roots.size() != deg || prev.empty()) return roots;

        std::vector<Complex> ordered(deg);
        std::vector<bool> used(deg, false);
        for (size_t b = 0; b < deg; ++b) {
            Complex last_pt = prev[b];
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
        return ordered;
    };

    std::vector<std::vector<Complex>> branches(deg);
    std::vector<Complex> prev_roots = (poles.size() == deg) ? poles : std::vector<Complex>{};
    double prev_k = 0.0;

    // Maximum allowed s-plane distance before adaptive K-subdivision kicks in
    const double max_arc_step = 0.022 * span;
    const double view_bound   = 4.5 * span;

    auto appendSubdivided = [&](auto& self, double k1, const std::vector<Complex>& r1,
                                double k2, const std::vector<Complex>& r2, int depth) -> void {
        if (depth < 6 && !r1.empty() && r2.size() == deg) {
            bool needs_split = false;
            for (size_t b = 0; b < deg; ++b) {
                // Subdivide if visible branch segment is too long in the s-plane
                if (std::abs(r1[b]) < view_bound || std::abs(r2[b]) < view_bound) {
                    if (std::abs(r2[b] - r1[b]) > max_arc_step) {
                        needs_split = true;
                        break;
                    }
                }
            }
            if (needs_split) {
                double k_mid = (k1 <= 0.0) ? (0.25 * k2) : std::sqrt(k1 * k2);
                auto r_mid = solveAndMatch(k_mid, r1);
                if (r_mid.size() == deg) {
                    // Re-match r2 against r_mid for cleaner continuity around sharp bends
                    auto r2_refined = solveAndMatch(k2, r_mid);
                    self(self, k1, r1, k_mid, r_mid, depth + 1);
                    self(self, k_mid, r_mid, k2, r2_refined, depth + 1);
                    return;
                }
            }
        }
        for (size_t b = 0; b < deg; ++b) branches[b].push_back(r2[b]);
    };

    for (size_t idx = 0; idx < k_vals.size(); ++idx) {
        double k = k_vals[idx];
        auto ordered = solveAndMatch(k, prev_roots);
        if (ordered.size() != deg) continue;

        if (branches[0].empty()) {
            for (size_t b = 0; b < deg; ++b) branches[b].push_back(ordered[b]);
        } else {
            appendSubdivided(appendSubdivided, prev_k, prev_roots, k, ordered, 0);
            for (size_t b = 0; b < deg; ++b) prev_roots[b] = branches[b].back();
            prev_k = k;
            continue;
        }
        prev_roots = ordered;
        prev_k = k;
    }
    return branches;
}

// =============================================================================
// Engine 2 & 3: Continuous & Discrete Time Simulators (With PID Disable Option)
// =============================================================================
struct SimulationConfig {
    std::vector<Complex> poles = {{-2.0, 2.0}, {-2.0, -2.0}};
    std::vector<Complex> zeros = {{-8.0, 0.0}};
    double plant_gain = 1.0;
    bool unity_feedback = true;
    std::vector<Complex> h_poles;
    std::vector<Complex> h_zeros;
    double h_gain = 1.0;
    bool comp_enabled = false;
    bool use_exact_comp_tf = false;
    std::vector<Complex> c_poles;
    std::vector<Complex> c_zeros;
    double c_gain = 1.0;
    double input_magnitude = 1.0;
    bool pid_enabled = true;
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
    std::optional<double> zeta, omega_n, omega_d, ol_zeta, ol_omega_n;
    Complex y_steady = {1.0, 0.0};
    double steady_state_error = 0.0;
    double relative_error_frac = 0.0;
    double relative_error_pct = 0.0;
    std::optional<double> rise_time, settling_time, settling_time_5pct, overshoot_pct;
    double peak_time = 0.0;
    double peak_value = 0.0;
    int system_type = 0;
    double kp_pos = 0.0, kv_vel = 0.0, ka_acc = 0.0, dc_gain_cl = 1.0;
    std::optional<double> sigma_d, time_constant_tau, theo_os_pct, theo_tp_sec, theo_ts2_sec, theo_tr_sec;
    std::string damping_regime = "Underdamped";
    double cursor_t = 2.5;
    Complex cursor_y = {1.0, 0.0}, cursor_u = {1.0, 0.0};
    double cursor_r = 1.0, cursor_abs_err = 0.0, cursor_rel_err_pct = 0.0;
    bool cursor_within_tol = true;
};

class ComplexTransferFunctionSimulator {
public:
    static std::pair<Poly, Poly> buildPlantPolynomials(const std::vector<Complex>& poles, const std::vector<Complex>& zeros, double gain) {
        return {polyScale(polyFromRoots(zeros), Complex(gain, 0.0)), polyFromRoots(poles)};
    }

    static std::pair<Poly, Poly> buildPidPolynomials(double kp, double ki, double kd, double n_filter = 20.0, bool pid_enabled = true) {
        if (!pid_enabled) return {{Complex(1.0, 0.0)}, {Complex(1.0, 0.0)}};
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

    static std::vector<Complex> simulateTF(
        Poly num, Poly den,
        const std::vector<double>& t,
        const std::vector<double>& r,
        const std::string& sig_type,
        double in_mag)
    {
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
                double dt0 = (n_pts > 1) ? std::max(1e-12, t[1] - t[0]) : 1.0;
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
        for (int i = 0; i < n; ++i) C(i) = num_pad[n - i] - b0 * den[n - i];

        Eigen::VectorXcd x = Eigen::VectorXcd::Zero(n);
        if (sig_type == "Impulse") x = B * Complex(in_mag, 0.0);

        double max_pole_est = 1.0;
        for (int i = 1; i <= n; ++i) {
            max_pole_est = std::max(max_pole_est, 2.0 * std::pow(std::abs(den[i]), 1.0 / double(i)));
        }

        for (size_t k = 0; k < n_pts; ++k) {
            double u_cur = (sig_type == "Impulse") ? 0.0 : r[k];
            y[k] = C.dot(x) + b0 * u_cur;
            if (k + 1 < n_pts) {
                double dt_k = std::max(0.0, t[k + 1] - t[k]);
                if (dt_k <= 0.0) continue;

                double u_next = (sig_type == "Impulse") ? 0.0 : r[k + 1];
                double req_steps = std::ceil((dt_k * max_pole_est) / 0.15);

                if (req_steps <= 2000.0) {
                    int sub_steps = std::max(2, static_cast<int>(req_steps));
                    double h = dt_k / double(sub_steps);
                    for (int s = 0; s < sub_steps; ++s) {
                        double f0 = double(s) / double(sub_steps);
                        double fm = (double(s) + 0.5) / double(sub_steps);
                        double f1 = double(s + 1) / double(sub_steps);
                        double u0 = u_cur + f0 * (u_next - u_cur);
                        double um = u_cur + fm * (u_next - u_cur);
                        double u1 = u_cur + f1 * (u_next - u_cur);

                        Eigen::VectorXcd k1 = A * x + B * u0;
                        Eigen::VectorXcd k2 = A * (x + 0.5 * h * k1) + B * um;
                        Eigen::VectorXcd k3 = A * (x + 0.5 * h * k2) + B * um;
                        Eigen::VectorXcd k4 = A * (x + h * k3) + B * u1;
                        x += (h / 6.0) * (k1 + 2.0 * k2 + 2.0 * k3 + k4);
                    }
                } else {
                    // Unconditionally A-stable Crank-Nicolson (Tustin) step for ultra-stiff intervals
                    int sub_steps = 16;
                    double h = dt_k / double(sub_steps);
                    Eigen::MatrixXcd I = Eigen::MatrixXcd::Identity(n, n);
                    Eigen::PartialPivLU<Eigen::MatrixXcd> lu(I - 0.5 * h * A);
                    Eigen::MatrixXcd M_rhs = I + 0.5 * h * A;
                    for (int s = 0; s < sub_steps; ++s) {
                        double f0 = double(s) / double(sub_steps);
                        double f1 = double(s + 1) / double(sub_steps);
                        double u0 = u_cur + f0 * (u_next - u_cur);
                        double u1 = u_cur + f1 * (u_next - u_cur);
                        x = lu.solve(M_rhs * x + (0.5 * h * (u0 + u1)) * B);
                    }
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
        double t_max = std::max(cfg.t_max, 1e-6);
        int n_base = std::max(cfg.num_points, 200);

        auto [num_g, den_g] = buildPlantPolynomials(cfg.poles, cfg.zeros, cfg.plant_gain);
        
        Poly num_c = {Complex(1.0, 0.0)};
        Poly den_c = {Complex(1.0, 0.0)};
        if (cfg.closed_loop && cfg.pid_enabled) {
            if (cfg.use_exact_comp_tf) {
                // Exact Lead / Lag / Pole-Zero Compensator Gc(s) = Kc * Num_c(s) / Den_c(s)
                std::tie(num_c, den_c) = buildPlantPolynomials(cfg.c_poles, cfg.c_zeros, cfg.c_gain);
            } else if (std::abs(cfg.ki) > 1e-9) {
                num_c = {Complex(cfg.kd, 0.0), Complex(cfg.kp, 0.0), Complex(cfg.ki, 0.0)};
                den_c = {Complex(1.0, 0.0), Complex(0.0, 0.0)};
            } else {
                num_c = {Complex(cfg.kd, 0.0), Complex(cfg.kp, 0.0)};
                den_c = {Complex(1.0, 0.0)};
            }
        }

        Poly num_h = {Complex(1.0, 0.0)};
        Poly den_h = {Complex(1.0, 0.0)};
        if (!cfg.unity_feedback) {
            std::tie(num_h, den_h) = buildPlantPolynomials(cfg.h_poles, cfg.h_zeros, cfg.h_gain);
        }

        // Forward path F(s) = C(s)*G(s), Loop L(s) = C(s)*G(s)*H(s)
        Poly num_f  = polyMul(num_c, num_g);
        Poly den_f  = polyMul(den_c, den_g);
        Poly num_l  = polyMul(num_f, num_h);
        Poly den_l  = polyMul(den_f, den_h);

        // Closed-loop T(s) = F(s) / (1 + L(s)) = (num_f * den_h) / (den_f * den_h + num_f * num_h)
        Poly num_cl = polyMul(num_f, den_h);
        Poly den_cl = polyAdd(den_l, num_l);
        // Control effort U(s)/R(s) = C(s) / (1 + L(s)) = (num_c * den_g * den_h) / den_cl
        Poly num_u  = polyMul(num_c, polyMul(den_g, den_h));

        res.cl_poles = solvePolynomialRoots(den_cl);
        res.cl_zeros = solvePolynomialRoots(num_cl);
        res.ol_poles = cfg.poles;
        res.ol_zeros = cfg.zeros;

        // Build multi-scale time grid: uniform [0, t_max] + dense sub-grids over fast pole transients
        std::vector<double> t_grid;
        t_grid.reserve(n_base + 1024);
        for (int i = 0; i < n_base; ++i) {
            t_grid.push_back(t_max * double(i) / double(n_base - 1));
        }

        std::vector<double> cand_wins;
        auto addPoleWindows = [&](const std::vector<Complex>& plist) {
            for (const auto& p : plist) {
                double wp = std::abs(p);
                if (wp > 1e-4) {
                    double w_osc = (12.0 * PI) / wp; // ~6 cycles to resolve Tr, Tp, & %OS
                    if (w_osc < 0.45 * t_max) cand_wins.push_back(w_osc);
                }
                if (p.real() < -1e-4) {
                    double w_dec = 7.0 / std::abs(p.real()); // 7 time constants to resolve Ts
                    if (w_dec < 0.45 * t_max) cand_wins.push_back(w_dec);
                }
            }
        };
        addPoleWindows(res.cl_poles);
        addPoleWindows(res.ol_poles);
        if (cfg.signal_type == "Sine" && cfg.sine_freq_rad > 1e-3) {
            double w_sin = (12.0 * PI) / cfg.sine_freq_rad;
            if (w_sin < 0.45 * t_max) cand_wins.push_back(w_sin);
        }

        std::sort(cand_wins.begin(), cand_wins.end());
        std::vector<double> merged_wins;
        for (double w : cand_wins) {
            if (w <= 1e-9) continue;
            if (merged_wins.empty() || w > 1.8 * merged_wins.back()) {
                merged_wins.push_back(w);
                if (merged_wins.size() >= 4) break;
            }
        }
        for (double w : merged_wins) {
            const int n_sub = 320;
            for (int i = 1; i < n_sub; ++i) {
                t_grid.push_back(w * double(i) / double(n_sub - 1));
            }
        }

        std::sort(t_grid.begin(), t_grid.end());
        res.t.clear();
        res.t.reserve(t_grid.size());
        double min_dt_sep = t_max * 1e-11;
        for (double tv : t_grid) {
            if (res.t.empty() || (tv - res.t.back()) > min_dt_sep) {
                res.t.push_back(tv);
            }
        }
        res.t.back() = t_max;

        int n_pts = static_cast<int>(res.t.size());
        res.r.resize(n_pts);
        for (int i = 0; i < n_pts; ++i) {
            if (cfg.signal_type == "Step") res.r[i] = cfg.input_magnitude;
            else if (cfg.signal_type == "Impulse") res.r[i] = 0.0;
            else if (cfg.signal_type == "Ramp") res.r[i] = cfg.input_magnitude * res.t[i];
            else res.r[i] = cfg.input_magnitude * std::sin(cfg.sine_freq_rad * res.t[i]);
        }

        res.y_closed  = simulateTF(num_cl, den_cl, res.t, res.r, cfg.signal_type, cfg.input_magnitude);
        res.y_open    = simulateTF(num_g, den_g, res.t, res.r, cfg.signal_type, cfg.input_magnitude);
        res.u_control = simulateTF(num_u, den_cl, res.t, res.r, cfg.signal_type, cfg.input_magnitude);

        if (!cfg.closed_loop) {
            for (int i = 0; i < n_pts; ++i) res.u_control[i] = Complex(res.r[i], 0.0);
        }

        res.y_primary = cfg.closed_loop ? res.y_closed : res.y_open;
        const auto& raw_active_poles = cfg.closed_loop ? res.cl_poles : res.ol_poles;
        const auto& raw_active_zeros = cfg.closed_loop ? res.cl_zeros : res.ol_zeros;

        // Filter out exact pole-zero cancellations (within 1e-3) for effective dynamic metrics
        std::vector<Complex> active_poles;
        std::vector<bool> z_used(raw_active_zeros.size(), false);
        for (const auto& p : raw_active_poles) {
            bool cancelled = false;
            for (size_t zi = 0; zi < raw_active_zeros.size(); ++zi) {
                if (!z_used[zi] && std::abs(p - raw_active_zeros[zi]) < 1e-3) {
                    z_used[zi] = true;
                    cancelled = true;
                    break;
                }
            }
            if (!cancelled) active_poles.push_back(p);
        }

        auto dom_m = computeDominantSMetrics(active_poles, cfg.deriv_filter_n);
        res.zeta = dom_m.zeta; res.omega_n = dom_m.omega_n; res.omega_d = dom_m.omega_d;
        auto ol_m = computeDominantSMetrics(res.ol_poles, cfg.deriv_filter_n);
        res.ol_zeta = ol_m.zeta; res.ol_omega_n = ol_m.omega_n;

        // Stability must still check all algebraic poles (so unstable RHP cancellations are never hidden!)
        const auto& stab_poles = raw_active_poles;
        if (stab_poles.empty()) {
            res.max_real_pole = -std::numeric_limits<double>::infinity();
            res.stability_state = "STABLE";
        } else {
            res.max_real_pole = -1e30;
            for (const auto& p : stab_poles) res.max_real_pole = std::max(res.max_real_pole, p.real());
            if (res.max_real_pole < -1e-5) res.stability_state = "STABLE";
            else if (res.max_real_pole <= 1e-5) res.stability_state = "MARGINAL";
            else res.stability_state = "UNSTABLE";
        }

        const Poly& act_num = cfg.closed_loop ? num_cl : num_g;
        const Poly& act_den = cfg.closed_loop ? den_cl : den_g;

        if (res.stability_state == "STABLE" && cfg.signal_type == "Step") {
            if (std::abs(act_den.back()) > 1e-12) res.y_steady = Complex(cfg.input_magnitude, 0.0) * (act_num.back() / act_den.back());
            else res.y_steady = res.y_primary.back();
            res.steady_state_error = std::abs(cfg.input_magnitude - res.y_steady);
        } else {
            res.y_steady = res.y_primary.back();
            res.steady_state_error = std::abs(res.r.back() - res.y_primary.back());
        }

        double abs_mag = std::max(std::abs(cfg.input_magnitude), 1e-12);
        res.relative_error_frac = res.steady_state_error / abs_mag;
        res.relative_error_pct  = res.relative_error_frac * 100.0;

        size_t pk_idx = 0;
        for (size_t i = 0; i < res.y_primary.size(); ++i)
            if (std::abs(res.y_primary[i].real()) > std::abs(res.y_primary[pk_idx].real())) pk_idx = i;
        res.peak_time = res.t[pk_idx];
        res.peak_value = res.y_primary[pk_idx].real();

        double tol_frac = std::max(cfg.settle_tol_pct, 0.05) / 100.0;
        double target_re = res.y_steady.real();

        if (res.stability_state == "STABLE" && cfg.signal_type == "Step" && std::abs(target_re) > 1e-6) {
            bool has_10 = false, has_90 = false;
            double t10 = 0.0, t90 = 0.0;
            int last_out_2 = -1, last_out_5 = -1;
            double max_norm = -1e30;
            size_t pk_step = 0;
            double yss_scale = std::max(std::abs(res.y_steady), 1e-9);

            for (int i = 0; i < n_pts; ++i) {
                double yn = res.y_primary[i].real() / target_re;
                if (!has_10 && yn >= 0.10) {
                    if (i == 0) t10 = res.t[0];
                    else {
                        double yn0 = res.y_primary[i - 1].real() / target_re;
                        double frac = (std::abs(yn - yn0) > 1e-15) ? std::clamp((0.10 - yn0) / (yn - yn0), 0.0, 1.0) : 0.0;
                        t10 = res.t[i - 1] + frac * (res.t[i] - res.t[i - 1]);
                    }
                    has_10 = true;
                }
                if (!has_90 && yn >= 0.90) {
                    if (i == 0) t90 = res.t[0];
                    else {
                        double yn0 = res.y_primary[i - 1].real() / target_re;
                        double frac = (std::abs(yn - yn0) > 1e-15) ? std::clamp((0.90 - yn0) / (yn - yn0), 0.0, 1.0) : 0.0;
                        t90 = res.t[i - 1] + frac * (res.t[i] - res.t[i - 1]);
                    }
                    has_90 = true;
                }
                if (yn > max_norm) { max_norm = yn; pk_step = static_cast<size_t>(i); }

                double err_b = std::abs(res.y_primary[i] - res.y_steady) / yss_scale;
                if (err_b > tol_frac) last_out_2 = i;
                if (err_b > 0.05)     last_out_5 = i;
            }

            // 3-point parabolic vertex refinement around pk_step for exact Peak, Tp, & %OS
            double pk_t = res.t[pk_step];
            double pk_val = res.y_primary[pk_step].real();
            if (pk_step > 0 && pk_step + 1 < static_cast<size_t>(n_pts)) {
                double x1 = res.t[pk_step - 1] - res.t[pk_step];
                double x3 = res.t[pk_step + 1] - res.t[pk_step];
                double d1 = (res.y_primary[pk_step - 1].real() / target_re) - max_norm;
                double d3 = (res.y_primary[pk_step + 1].real() / target_re) - max_norm;
                double denom = x1 * x3 * (x1 - x3);
                if (std::abs(denom) > 1e-30) {
                    double a = (d1 * x3 - d3 * x1) / denom;
                    double b = (d3 * x1 * x1 - d1 * x3 * x3) / denom;
                    if (a < -1e-15) {
                        double xv = std::clamp(-b / (2.0 * a), x1, x3);
                        double yn_v = max_norm + a * xv * xv + b * xv;
                        if (yn_v >= max_norm) {
                            max_norm = yn_v;
                            pk_t = res.t[pk_step] + xv;
                            pk_val = max_norm * target_re;
                        }
                    }
                }
            }

            if (has_10 && has_90 && t90 >= t10) res.rise_time = t90 - t10;
            res.overshoot_pct = std::max(0.0, (max_norm - 1.0) * 100.0);
            res.peak_time = pk_t;
            res.peak_value = pk_val;

            auto interpSettle = [&](int last_idx, double thr) -> std::optional<double> {
                if (last_idx < 0) return 0.0;
                if (last_idx >= n_pts - 1) return std::nullopt;
                double e0 = std::abs(res.y_primary[last_idx] - res.y_steady) / yss_scale;
                double e1 = std::abs(res.y_primary[last_idx + 1] - res.y_steady) / yss_scale;
                double frac = (e0 - e1 > 1e-15) ? std::clamp((e0 - thr) / (e0 - e1), 0.0, 1.0) : 1.0;
                return res.t[last_idx] + frac * (res.t[last_idx + 1] - res.t[last_idx]);
            };
            res.settling_time      = interpSettle(last_out_2, tol_frac);
            res.settling_time_5pct = interpSettle(last_out_5, 0.05);
        } else if (res.stability_state == "STABLE" && cfg.signal_type == "Impulse") {
            double pk_abs = std::max(std::abs(res.peak_value), 1e-9);
            int last_out_2 = -1, last_out_5 = -1;
            for (int i = 0; i < n_pts; ++i) {
                double err_b = std::abs(res.y_primary[i].real()) / pk_abs;
                if (err_b > tol_frac) last_out_2 = i;
                if (err_b > 0.05)     last_out_5 = i;
            }
            if (last_out_2 < 0) res.settling_time = 0.0;
            else if (last_out_2 < n_pts - 1) res.settling_time = res.t[last_out_2 + 1];
            if (last_out_5 < 0) res.settling_time_5pct = 0.0;
            else if (last_out_5 < n_pts - 1) res.settling_time_5pct = res.t[last_out_5 + 1];
        }

        Poly num_loop = polyTrim(cfg.closed_loop ? num_l : num_g);
        Poly den_loop = polyTrim(cfg.closed_loop ? den_l : den_g);
        int sys_t = 0;
        for (auto it = den_loop.rbegin(); it != den_loop.rend(); ++it) {
            if (std::abs(*it) < 1e-10) ++sys_t; else break;
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

        if (active_poles.empty()) {
            res.theo_os_pct = 0.0;
            res.theo_tr_sec = 0.0;
            res.theo_ts2_sec = 0.0;
            res.damping_regime = "Pure Gain (Static)";
        } else if (res.zeta.has_value() && res.omega_n.has_value() && res.omega_n.value() > 1e-9) {
            double z = res.zeta.value(), wn = res.omega_n.value();
            // For overdamped (z > 1), dominant decay rate is the slowest pole |Re(dom_pole)|, not z*wn!
            res.sigma_d = (z > 1.0 && dom_m.dom_pole.has_value())
                ? std::abs(dom_m.dom_pole->real())
                : (z * wn);
            res.time_constant_tau = (res.sigma_d.value() > 1e-9) ? (1.0 / res.sigma_d.value()) : std::numeric_limits<double>::infinity();
            res.theo_ts2_sec = (res.sigma_d.value() > 1e-9) ? (4.0 / res.sigma_d.value()) : std::numeric_limits<double>::infinity();
            res.theo_os_pct = (active_poles.size() == 1) ? 0.0 : niseOsFromZeta(z);
            res.theo_tp_sec = (res.omega_d.has_value() && res.omega_d.value() > 1e-9) ? (PI / res.omega_d.value()) : std::numeric_limits<double>::infinity();
            res.theo_tr_sec = (active_poles.size() == 1) ? (2.2 / std::max(res.sigma_d.value(), 1e-9)) : niseTrFromZetaWn(std::clamp(z, 0.0, 1.2), wn);
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

        res.cursor_t = std::clamp(cfg.cursor_time, 0.0, t_max);
        auto it_hi = std::lower_bound(res.t.begin(), res.t.end(), res.cursor_t);
        if (it_hi == res.t.begin()) {
            res.cursor_y = res.y_primary.front();
            res.cursor_u = res.u_control.front();
            res.cursor_r = res.r.front();
        } else if (it_hi == res.t.end()) {
            res.cursor_y = res.y_primary.back();
            res.cursor_u = res.u_control.back();
            res.cursor_r = res.r.back();
        } else {
            size_t i1 = static_cast<size_t>(std::distance(res.t.begin(), it_hi));
            size_t i0 = i1 - 1;
            double dt_seg = res.t[i1] - res.t[i0];
            double alpha = (dt_seg > 1e-15) ? ((res.cursor_t - res.t[i0]) / dt_seg) : 0.0;
            res.cursor_y = res.y_primary[i0] + alpha * (res.y_primary[i1] - res.y_primary[i0]);
            res.cursor_u = res.u_control[i0] + alpha * (res.u_control[i1] - res.u_control[i0]);
            res.cursor_r = res.r[i0] + alpha * (res.r[i1] - res.r[i0]);
        }
        res.cursor_abs_err = std::abs(res.cursor_r - res.cursor_y);
        res.cursor_rel_err_pct = (res.cursor_abs_err / abs_mag) * 100.0;
        res.cursor_within_tol = (std::abs(res.cursor_y - res.y_steady) / std::max(std::abs(res.y_steady), 1.0)) <= tol_frac;
        return res;
    }
};

struct DiscreteSimulationConfig {
    std::vector<Complex> z_poles = {{0.68, 0.32}, {0.68, -0.32}};
    std::vector<Complex> z_zeros = {{-1.0, 0.0}, {-1.0, 0.0}};
    double filter_gain = 0.0955;
    bool unity_feedback = true;
    std::vector<Complex> hz_fb_poles;
    std::vector<Complex> hz_fb_zeros;
    double hz_fb_gain = 1.0;
    double input_magnitude = 1.0;
    double ts = 0.05;
    bool pid_enabled = true;
    double kp = 1.0, ki = 0.5, kd = 0.02;
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
    std::vector<Complex> y_primary, y_closed, y_open, u_control;
    std::vector<Complex> cl_z_poles, ol_z_poles, ol_z_zeros;
    Poly b_coeffs, a_coeffs;
    std::string diff_eq_str;
    std::string stability_state = "STABLE";
    double max_pole_mag = 0.75;
    std::optional<double> zeta, omega_n;
    Complex y_steady = {1.0, 0.0};
    double steady_state_error = 0.0;
    std::optional<int> rise_samples, settling_samples;
    std::optional<double> rise_time_sec, settling_time_sec, overshoot_pct;
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
        if (!pid_enabled) return {{Complex(1.0, 0.0)}, {Complex(1.0, 0.0)}};
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
            if (!std::isfinite(acc.real()) || !std::isfinite(acc.imag())) {
                acc = (k > 0) ? y[k - 1] : Complex(0.0, 0.0);
            }
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
            return {zp, zz, (ev.magnitude > 1e-12) ? (1.0 / ev.magnitude) : 1.0};
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
        if (key == "fir_ma4") return {{{0,0},{0,0},{0,0}}, {{-1,0},{0,1},{0,-1}}, 0.25};
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
            else if (cfg.signal_type == "Two-Tone") {
                double f_low = 1.0 / std::max(n_pts * ts / 3.0, 1e-3);
                res.r_seq[k] = cfg.input_magnitude * (0.75 * std::sin(2.0 * PI * f_low * res.t_sec[k]) + 0.35 * std::cos(0.75 * PI * k));
            } else res.r_seq[k] = cfg.input_magnitude * std::sin(2.0 * PI * cfg.sine_freq_hz * res.t_sec[k]);
        }

        Poly num_h = polyScale(polyFromRoots(cfg.z_zeros), Complex(cfg.filter_gain, 0.0));
        Poly den_h = polyFromRoots(cfg.z_poles);
        auto [b_h, a_h] = toCausalBA(num_h, den_h);

        Poly b_fb = {Complex(1.0, 0.0)};
        Poly a_fb = {Complex(1.0, 0.0)};
        if (!cfg.unity_feedback) {
            Poly n_fb = polyScale(polyFromRoots(cfg.hz_fb_zeros), Complex(cfg.hz_fb_gain, 0.0));
            Poly d_fb = polyFromRoots(cfg.hz_fb_poles);
            std::tie(b_fb, a_fb) = toCausalBA(n_fb, d_fb);
        }

        auto [num_c, den_c] = buildDiscretePid(cfg.kp, cfg.ki, cfg.kd, ts, cfg.pid_enabled);
        Poly num_f  = polyMul(num_c, b_h);
        Poly den_f  = polyMul(den_c, a_h);
        Poly num_l  = polyMul(num_f, b_fb);
        Poly den_l  = polyMul(den_f, a_fb);

        Poly num_cl = polyMul(num_f, a_fb);
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
            if (i10 >= 0 && i90 >= i10) {
                res.rise_samples = i90 - i10;
                res.rise_time_sec = double(res.rise_samples.value()) * ts;
            }
            if (last_out < 0) {
                res.settling_samples = 0;
                res.settling_time_sec = 0.0;
            } else if (last_out < n_pts - 1) {
                res.settling_samples = last_out + 1;
                res.settling_time_sec = double(res.settling_samples.value()) * ts;
            }
            res.overshoot_pct = std::max(0.0, (max_n - 1.0) * 100.0);
        } else if (res.stability_state == "STABLE" && cfg.signal_type == "Impulse") {
            double pk_abs = 1e-9;
            for (int k = 0; k < n_pts; ++k) {
                double a = std::abs(res.y_primary[k].real());
                if (a > pk_abs) { pk_abs = a; res.peak_k = k; res.peak_value = res.y_primary[k].real(); }
            }
            int last_out = -1;
            for (int k = 0; k < n_pts; ++k) {
                if (std::abs(res.y_primary[k].real()) / pk_abs > tol_frac) last_out = k;
            }
            if (last_out < 0) {
                res.settling_samples = 0;
                res.settling_time_sec = 0.0;
            } else if (last_out < n_pts - 1) {
                res.settling_samples = last_out + 1;
                res.settling_time_sec = double(res.settling_samples.value()) * ts;
            }
        }

        res.cursor_k = std::clamp(cfg.cursor_k, 0, n_pts - 1);
        res.cursor_t = res.cursor_k * ts;
        res.cursor_y = res.y_primary[res.cursor_k];
        res.cursor_within_tol = (std::abs(res.cursor_y - res.y_steady) / std::max(std::abs(res.y_steady), 1.0)) <= tol_frac;
        return res;
    }
};

// =============================================================================
// Shared System Model
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
    double kp = 1.5, ki = 2.2, kd = 0.05, deriv_filter_n = 20.0;
    double max_k_slider = 100.0; // Configurable endpoint for the s-Plane & PID K-sliders

    std::vector<Complex> z_poles, z_zeros;
    double filter_gain = 0.0955;
    double discrete_input_magnitude = 1.0;
    double ts = 0.05;
    Complex z_test_point = {0.68, 0.32};
    double z_rl_tol_deg = 0.5;

    bool d_pid_enabled = true;
    double d_kp = 1.0, d_ki = 0.5, d_kd = 0.02;

    std::optional<SimulationResult> last_sim_result;
    std::optional<DiscreteSimulationResult> last_discrete_result;
    bool sync_enabled = true;
    std::vector<std::function<void(const std::string&)>> listeners;

    int routh_mode = 0;
    std::string routh_custom_poly = "s^4 + 6s^3 + 21s^2 + (26 + K)s + 6K";
    std::string routh_k_eval = "10.0";

    int theme_index = 0;
    bool light_mode = false;

    bool comp_enabled_s = false;
    std::string c_num_str = "1"; std::string c_den_str = "1";
    std::vector<Complex> c_poles, c_zeros;
    double c_gain = 1.0;

    // Checks if Gc(s) is a finite-pole Lead/Lag (cannot be represented by pure Kp + Ki/s + Kd*s alone)
    bool isFinitePoleCompensatorS() const {
        if (!comp_enabled_s) return false;
        if (c_poles.size() == 1 && std::abs(c_poles[0]) < 1e-6 && c_zeros.size() <= 2) return false; // Ideal PI or PID
        if (c_poles.empty() && c_zeros.size() <= 1) return false; // Pure P or Ideal PD
        return !c_poles.empty() || c_zeros.size() > 1;
    }

    // Converts Root Locus Gc(s) = Kc * Num_c(s) / Den_c(s) into Time-Domain (Kp, Ki, Kd)
    void syncCompensatorToPidGains() {
        if (!comp_enabled_s) return;
        pid_enabled = true;

        // Case 1: Pure Proportional Gc(s) = Kc
        if (c_zeros.empty() && c_poles.empty()) {
            kp = c_gain;
            ki = 0.0;
            kd = 0.0;
            return;
        }
        // Case 2: Ideal PD Gc(s) = Kc * (s - z0) = Kc*s + Kc*(-z0)
        if (c_zeros.size() == 1 && c_poles.empty()) {
            double zc = -c_zeros[0].real();
            kd = c_gain;
            kp = c_gain * zc;
            ki = 0.0;
            return;
        }
        // Case 3: Ideal PI Gc(s) = Kc * (s - z0) / s = Kc + (Kc * (-z0)) / s
        if (c_zeros.size() == 1 && c_poles.size() == 1 && std::abs(c_poles[0]) < 1e-6) {
            double zc = -c_zeros[0].real();
            kp = c_gain;
            ki = c_gain * zc;
            kd = 0.0;
            return;
        }
        // Case 4: Ideal PID Gc(s) = Kc * (s - z1)(s - z2) / s
        if (c_zeros.size() == 2 && c_poles.size() == 1 && std::abs(c_poles[0]) < 1e-6) {
            Poly num_p = polyFromRoots(c_zeros); // s^2 + b1*s + b0
            kd = c_gain;
            kp = c_gain * num_p[1].real();
            ki = c_gain * num_p[2].real();
            return;
        }
        // Case 5: Lead / Lag / General Pole-Zero Gc(s) = Kc * (s + zc) / (s + pc)
        // Set Kp to the DC gain Gc(0) (or high-freq Kc if pole at 0) for display,
        // while the time-domain simulator uses the exact Gc(s) poles & zeros!
        Poly num_p = polyFromRoots(c_zeros);
        Poly den_p = polyFromRoots(c_poles);
        double n0 = num_p.back().real();
        double d0 = den_p.back().real();
        kp = (std::abs(d0) > 1e-7) ? (c_gain * n0 / d0) : c_gain;
        ki = 0.0;
        kd = 0.0;
    }

    // Uncompensated open-loop poles/zeros: G(s)H(s) only (ignores Gc(s))
    std::vector<Complex> uncompensatedLoopPolesS() const {
        auto out = poles;
        if (!unity_feedback_s) out.insert(out.end(), h_poles.begin(), h_poles.end());
        return out;
    }

    std::vector<Complex> uncompensatedLoopZerosS() const {
        auto out = zeros;
        if (!unity_feedback_s) out.insert(out.end(), h_zeros.begin(), h_zeros.end());
        return out;
    }

    SharedSystemModel() {
        std::tie(z_poles, z_zeros, filter_gain) = DiscreteFilterSimulator::designFilterPreset("butter_lp", 0.25, 0.05);
        if (!z_poles.empty()) z_test_point = z_poles[0];
    }

    static QString serializeComplexVec(const std::vector<Complex>& v) {
        QStringList list;
        for (const auto& c : v) {
            list << QString("%1,%2").arg(c.real(), 0, 'g', 8).arg(c.imag(), 0, 'g', 8);
        }
        return list.join(";");
    }

    static std::vector<Complex> deserializeComplexVec(const QString& s) {
        std::vector<Complex> out;
        if (s.trimmed().isEmpty()) return out;
        for (const QString& pair : s.split(";", Qt::SkipEmptyParts)) {
            QStringList parts = pair.split(",");
            if (parts.size() == 2) {
                out.emplace_back(parts[0].toDouble(), parts[1].toDouble());
            }
        }
        return out;
    }

    void resetToDefaults() {
        routh_mode = 0;
        routh_custom_poly = "s^4 + 6s^3 + 21s^2 + (26 + K)s + 6K";
        routh_k_eval = "10.0";
        poles = {{-2.0, 2.0}, {-2.0, -2.0}};
        zeros = {{-8.0, 0.0}};
        plant_gain = 1.0;
        input_magnitude = 1.0;
        test_point = {-2.0, 2.0};
        rl_tol_deg = 0.5;
        pid_enabled = true;
        kp = 1.5; ki = 2.2; kd = 0.05; deriv_filter_n = 20.0;
        max_k_slider = 100.0;

        unity_feedback_s = true;
        h_num_str = "1"; h_den_str = "1";
        h_poles.clear(); h_zeros.clear(); h_gain = 1.0;

        std::tie(z_poles, z_zeros, filter_gain) = DiscreteFilterSimulator::designFilterPreset("butter_lp", 0.25, 0.05);
        discrete_input_magnitude = 1.0;
        ts = 0.05;
        z_test_point = z_poles.empty() ? Complex(0.68, 0.32) : z_poles[0];
        z_rl_tol_deg = 0.5;
        d_pid_enabled = true;
        d_kp = 1.0; d_ki = 0.5; d_kd = 0.02;

        unity_feedback_z = true;
        hz_fb_num_str = "1"; hz_fb_den_str = "1";
        hz_fb_poles.clear(); hz_fb_zeros.clear(); hz_fb_gain = 1.0;
    }

    void saveToSettings(QSettings& s) const {
        s.setValue("app/theme_index", theme_index);
        s.setValue("app/light_mode", light_mode);

        s.setValue("s/poles", serializeComplexVec(poles));
        s.setValue("s/zeros", serializeComplexVec(zeros));
        s.setValue("s/plant_gain", plant_gain);
        s.setValue("s/input_mag", input_magnitude);
        s.setValue("s/tp_re", test_point.real());
        s.setValue("s/tp_im", test_point.imag());
        s.setValue("s/rl_tol", rl_tol_deg);
        s.setValue("s/pid_enabled", pid_enabled);
        s.setValue("s/kp", kp);
        s.setValue("s/ki", ki);
        s.setValue("s/kd", kd);
        s.setValue("s/max_k_slider", max_k_slider);
        s.setValue("s/unity_fb", unity_feedback_s);
        s.setValue("s/h_num", QString::fromStdString(h_num_str));
        s.setValue("s/h_den", QString::fromStdString(h_den_str));

        s.setValue("z/poles", serializeComplexVec(z_poles));
        s.setValue("z/zeros", serializeComplexVec(z_zeros));
        s.setValue("z/filter_gain", filter_gain);
        s.setValue("z/input_mag", discrete_input_magnitude);
        s.setValue("z/ts", ts);
        s.setValue("z/tp_re", z_test_point.real());
        s.setValue("z/tp_im", z_test_point.imag());
        s.setValue("z/rl_tol", z_rl_tol_deg);
        s.setValue("z/pid_enabled", d_pid_enabled);
        s.setValue("z/kp", d_kp);
        s.setValue("z/ki", d_ki);
        s.setValue("z/kd", d_kd);
        s.setValue("z/unity_fb", unity_feedback_z);
        s.setValue("z/h_num", QString::fromStdString(hz_fb_num_str));
        s.setValue("z/h_den", QString::fromStdString(hz_fb_den_str));

        s.setValue("app/prec_mode", QString::fromStdString(PRECISION.mode));
        s.setValue("app/dec_sub1", PRECISION.dec_sub1);
        s.setValue("app/dec_ge1", PRECISION.dec_ge1);
        s.setValue("app/step_round", PRECISION.step_round);
        s.setValue("app/sync_enabled", sync_enabled);
        s.setValue("app/omit_zeros", PRECISION.omit_trailing_zeros);

        s.setValue("routh/mode", routh_mode);
        s.setValue("routh/custom_poly", QString::fromStdString(routh_custom_poly));
        s.setValue("routh/k_eval", QString::fromStdString(routh_k_eval));
    }

    void loadFromSettings(QSettings& s) {
        theme_index = s.value("app/theme_index", 0).toInt();
        light_mode  = Theme::is_light;
        Theme::setTheme(theme_index);
        PRECISION.mode       = s.value("app/prec_mode", "class").toString().toStdString();
        PRECISION.dec_sub1   = s.value("app/dec_sub1", 3).toInt();
        PRECISION.dec_ge1    = s.value("app/dec_ge1", 2).toInt();
        PRECISION.step_round = s.value("app/step_round", false).toBool();
        PRECISION.omit_trailing_zeros = s.value("app/omit_zeros", false).toBool();
        sync_enabled         = s.value("app/sync_enabled", true).toBool();
        if (!s.contains("s/plant_gain")) return; // First run: keep defaults

        poles = deserializeComplexVec(s.value("s/poles").toString());
        zeros = deserializeComplexVec(s.value("s/zeros").toString());
        plant_gain = s.value("s/plant_gain", 1.0).toDouble();
        input_magnitude = s.value("s/input_mag", 1.0).toDouble();
        test_point = Complex(s.value("s/tp_re", -2.0).toDouble(), s.value("s/tp_im", 2.0).toDouble());
        rl_tol_deg = s.value("s/rl_tol", 0.5).toDouble();
        pid_enabled = s.value("s/pid_enabled", true).toBool();
        kp = s.value("s/kp", 1.5).toDouble();
        ki = s.value("s/ki", 2.2).toDouble();
        kd = s.value("s/kd", 0.05).toDouble();
        max_k_slider = s.value("s/max_k_slider", 100.0).toDouble();
        unity_feedback_s = s.value("s/unity_fb", true).toBool();
        h_num_str = s.value("s/h_num", "1").toString().toStdString();
        h_den_str = s.value("s/h_den", "1").toString().toStdString();
        if (!unity_feedback_s) {
            try {
                auto pn = parsePolynomialInput(h_num_str);
                auto pd = parsePolynomialInput(h_den_str);
                h_zeros = pn.roots; h_poles = pd.roots;
                h_gain = pn.leading_gain / std::max(std::abs(pd.leading_gain), 1e-12);
            } catch (...) {}
        }

        z_poles = deserializeComplexVec(s.value("z/poles").toString());
        z_zeros = deserializeComplexVec(s.value("z/zeros").toString());
        filter_gain = s.value("z/filter_gain", 0.0955).toDouble();
        discrete_input_magnitude = s.value("z/input_mag", 1.0).toDouble();
        ts = s.value("z/ts", 0.05).toDouble();
        z_test_point = Complex(s.value("z/tp_re", 0.68).toDouble(), s.value("z/tp_im", 0.32).toDouble());
        z_rl_tol_deg = s.value("z/rl_tol", 0.5).toDouble();
        d_pid_enabled = s.value("z/pid_enabled", true).toBool();
        d_kp = s.value("z/kp", 1.0).toDouble();
        d_ki = s.value("z/ki", 0.5).toDouble();
        d_kd = s.value("z/kd", 0.02).toDouble();
        unity_feedback_z = s.value("z/unity_fb", true).toBool();
        hz_fb_num_str = s.value("z/h_num", "1").toString().toStdString();
        hz_fb_den_str = s.value("z/h_den", "1").toString().toStdString();
        if (!unity_feedback_z) {
            try {
                auto pn = parsePolynomialInput(hz_fb_num_str);
                auto pd = parsePolynomialInput(hz_fb_den_str);
                hz_fb_zeros = pn.roots; hz_fb_poles = pd.roots;
                hz_fb_gain = pn.leading_gain / std::max(std::abs(pd.leading_gain), 1e-12);
            } catch (...) {}
        }

        routh_mode        = std::clamp(s.value("routh/mode", 0).toInt(), 0, 2);
        routh_custom_poly = s.value("routh/custom_poly", "s^4 + 6s^3 + 21s^2 + (26 + K)s + 6K").toString().toStdString();
        routh_k_eval      = s.value("routh/k_eval", "10.0").toString().toStdString();
    }

    void notify(const std::string& source_tab) {
        if (!sync_enabled) return;
        for (auto& cb : listeners) {
            cb(source_tab);
        }
    }

    // Continuous Feedback Sensor H(s) (Unity H(s)=1 when unity_feedback_s == true)
    bool unity_feedback_s = true;
    std::string h_num_str = "1";
    std::string h_den_str = "1";
    std::vector<Complex> h_poles;
    std::vector<Complex> h_zeros;
    double h_gain = 1.0;

    // Discrete Feedback Sensor H_fb(z) (Unity H_fb(z)=1 when unity_feedback_z == true)
    bool unity_feedback_z = true;
    std::string hz_fb_num_str = "1";
    std::string hz_fb_den_str = "1";
    std::vector<Complex> hz_fb_poles;
    std::vector<Complex> hz_fb_zeros;
    double hz_fb_gain = 1.0;

    std::vector<Complex> effectiveLoopPolesS() const {
        auto out = uncompensatedLoopPolesS();
        if (comp_enabled_s) out.insert(out.end(), c_poles.begin(), c_poles.end());
        return out;
    }
    std::vector<Complex> effectiveLoopZerosS() const {
        auto out = uncompensatedLoopZerosS();
        if (comp_enabled_s) out.insert(out.end(), c_zeros.begin(), c_zeros.end());
        return out;
    }
    std::vector<Complex> effectiveLoopPolesZ() const {
        auto out = z_poles;
        if (!unity_feedback_z) out.insert(out.end(), hz_fb_poles.begin(), hz_fb_poles.end());
        return out;
    }
    std::vector<Complex> effectiveLoopZerosZ() const {
        auto out = z_zeros;
        if (!unity_feedback_z) out.insert(out.end(), hz_fb_zeros.begin(), hz_fb_zeros.end());
        return out;
    }
};

// =============================================================================
// Helper UI Factory Functions & Reusable Pole/Zero Table Widget
// =============================================================================
inline QFrame* makeCard(QWidget* parent = nullptr) {
    auto* f = new QFrame(parent);
    f->setStyleSheet(QString("QFrame { background-color: %1; border: 1px solid %2; border-radius: 6px; }")
                     .arg(Theme::BG_PANEL, Theme::BORDER_PANEL));
    return f;
}

inline QLineEdit* makeDarkEntry(const QString& init_val = "", int width = 80) {
    auto* e = new QLineEdit(init_val);
    if (width > 0) e->setMinimumWidth(width);
    e->setStyleSheet(QString(
        "QLineEdit { background-color: %1; color: %2; border: 1px solid %3; border-radius: 4px; padding: 3px 6px; font-family: Monospace; font-weight: bold; font-size: 11px; }"
        "QLineEdit:focus { border: 1px solid %4; }"
        "QLineEdit:disabled { color: %5; background-color: %6; }"
    ).arg(Theme::BG_INPUT, Theme::TEXT_PRIMARY, Theme::BORDER_INPUT, Theme::ACCENT_ZERO, Theme::TEXT_MUTED, Theme::BG_PANEL_ALT));
    return e;
}

inline QPushButton* makeActionBtn(const QString& text, const QString& accent_fg, const QString& bg = Theme::BG_INPUT) {
    auto* b = new QPushButton(text);
    b->setCursor(Qt::PointingHandCursor);
    bool is_filled = (bg != Theme::BG_INPUT);
    QString border_col = is_filled ? bg : accent_fg;
    QString hover_fg   = is_filled ? bg : accent_fg;
    b->setStyleSheet(QString(
        "QPushButton { background-color: %1; color: %2; border: 1px solid %5; border-radius: 4px; padding: 4px 9px; font-weight: bold; font-size: 11px; }"
        "QPushButton:hover { background-color: %4; color: %6; border: 1px solid %6; }"
        "QPushButton:disabled { color: %3; border-color: %3; }"
    ).arg(bg, accent_fg, Theme::TEXT_MUTED, Theme::BG_KEY_NUM, border_col, hover_fg));
    return b;
}

inline QScrollArea* wrapInLeftScroll(QWidget* left_w, int min_w = 380) {
    auto* sa = new QScrollArea();
    sa->setWidgetResizable(true);
    sa->setFrameShape(QFrame::NoFrame);
    sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sa->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    sa->setMinimumWidth(min_w);
    sa->setStyleSheet("QScrollArea { background: transparent; border: none; }");
    left_w->setMinimumWidth(min_w - 16);
    sa->setWidget(left_w);
    return sa;
}

// Reusable Editable Poles (×) & Zeros (○) Card with SpinBox Count & Scrollable Re/Im Rows
class PoleZeroEditorCard : public QFrame {
    Q_OBJECT
public:
    QSpinBox* count_spin;
    QVBoxLayout* rows_layout;
    std::vector<std::pair<QLineEdit*, QLineEdit*>> row_edits;
    double default_re;
    bool updating = false;
    std::function<void()> onValuesChanged;
    QScrollArea* scroll = nullptr;
    PoleZeroEditorCard* peer_card = nullptr;

    PoleZeroEditorCard(const QString& title, const QString& accent_col, double def_re = -1.0, QWidget* parent = nullptr)
        : QFrame(parent), default_re(def_re)
    {
        setStyleSheet(QString("QFrame { background-color: %1; border: 1px solid %2; border-radius: 6px; }")
                      .arg(Theme::BG_PANEL, Theme::BORDER_PANEL));
        // Prevent the card from stretching vertically and creating gaps around '# Re Im'
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

        auto* vl = new QVBoxLayout(this);
        vl->setContentsMargins(8, 6, 8, 6);
        vl->setSpacing(2);

        auto* hdr = new QHBoxLayout();
        hdr->setContentsMargins(0, 0, 0, 0);
        auto* t_lbl = new QLabel(title);
        t_lbl->setFixedHeight(22);
        t_lbl->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 12px; border: none;").arg(accent_col));
        auto* c_lbl = new QLabel("Count:");
        c_lbl->setFixedHeight(22);
        c_lbl->setStyleSheet("color: #94a3b8; font-size: 10px; border: none;");
        count_spin = new QSpinBox();
        count_spin->setRange(0, 10);
        count_spin->setFixedHeight(22);
        count_spin->setStyleSheet(QString(
            "QSpinBox { background-color: %1; color: %2; border: 1px solid %3; border-radius: 3px; padding: 1px 4px; font-family: Monospace; font-weight: bold; }"
        ).arg(Theme::BG_INPUT, Theme::TEXT_PRIMARY, Theme::BORDER_INPUT));

        hdr->addWidget(t_lbl);
        hdr->addStretch();
        hdr->addWidget(c_lbl);
        hdr->addWidget(count_spin);
        vl->addLayout(hdr);

        auto* col_hdr = new QHBoxLayout();
        col_hdr->setContentsMargins(0, 2, 0, 0);
        auto* h_idx = new QLabel("#");  h_idx->setFixedWidth(18); h_idx->setFixedHeight(16); h_idx->setStyleSheet("color: #94a3b8; font-size: 10px; font-weight: bold; border:none;");
        auto* h_re  = new QLabel("Re"); h_re->setFixedHeight(16); h_re->setStyleSheet("color: #94a3b8; font-size: 10px; font-weight: bold; border:none;");
        auto* h_im  = new QLabel("Im"); h_im->setFixedHeight(16); h_im->setStyleSheet("color: #94a3b8; font-size: 10px; font-weight: bold; border:none;");
        col_hdr->addWidget(h_idx);
        col_hdr->addWidget(h_re, 1);
        col_hdr->addWidget(h_im, 1);
        vl->addLayout(col_hdr);

        scroll = new QScrollArea();
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        scroll->setStyleSheet("QScrollArea { background: transparent; border: none; }");
        auto* container = new QWidget();
        container->setStyleSheet("background: transparent; border: none;");
        rows_layout = new QVBoxLayout(container);
        rows_layout->setContentsMargins(0, 0, 0, 0);
        rows_layout->setSpacing(3);
        rows_layout->setAlignment(Qt::AlignTop);
        scroll->setWidget(container);
        scroll->setMinimumHeight(34);
        vl->addWidget(scroll, 1);

        connect(count_spin, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int cnt) {
            if (updating) return;
            syncRowCount(cnt);
            if (onValuesChanged) onValuesChanged();
        });
    }

    void syncHeightWithPeer() {
        int my_rows = std::clamp(count_spin ? count_spin->value() : 1, 1, 4);
        int peer_rows = (peer_card && peer_card->count_spin)
                        ? std::clamp(peer_card->count_spin->value(), 1, 4)
                        : my_rows;
        int target_h = std::max(my_rows, peer_rows) * 27 + 6;
        if (scroll) scroll->setFixedHeight(target_h);
        if (peer_card && peer_card->scroll) peer_card->scroll->setFixedHeight(target_h);
    }

    void syncRowCount(int target) {
        target = std::clamp(target, 0, 10);
        while (static_cast<int>(row_edits.size()) > target) {
            auto [re_e, im_e] = row_edits.back();
            delete re_e->parentWidget();
            row_edits.pop_back();
        }
        while (static_cast<int>(row_edits.size()) < target) {
            int idx = static_cast<int>(row_edits.size());
            auto* rw = new QWidget();
            auto* hl = new QHBoxLayout(rw);
            hl->setContentsMargins(0, 0, 0, 0);
            hl->setSpacing(4);
            auto* il = new QLabel(QString::number(idx + 1));
            il->setFixedWidth(18);
            il->setStyleSheet("color: #94a3b8; font-family: Monospace; font-size: 10px; border:none;");
            double init_r = (default_re < 0.0) ? -(idx + 1.0) : default_re;
            auto* re_e = makeDarkEntry(QString::number(init_r, 'g', 4), 50);
            auto* im_e = makeDarkEntry("0", 50);
            hl->addWidget(il);
            hl->addWidget(re_e, 1);
            hl->addWidget(im_e, 1);
            rows_layout->addWidget(rw);
            row_edits.push_back({re_e, im_e});

            auto trig = [this]() { if (!updating && onValuesChanged) onValuesChanged(); };
            connect(re_e, &QLineEdit::editingFinished, this, trig);
            connect(im_e, &QLineEdit::editingFinished, this, trig);
            connect(re_e, &QLineEdit::returnPressed, this, trig);
            connect(im_e, &QLineEdit::returnPressed, this, trig);
        }
        syncHeightWithPeer();
    }

    void showEvent(QShowEvent* ev) override {
        QFrame::showEvent(ev);
        syncHeightWithPeer();
    }

    void setValues(const std::vector<Complex>& vals) {
        updating = true;
        count_spin->setValue(static_cast<int>(vals.size()));
        syncRowCount(static_cast<int>(vals.size()));
        for (size_t i = 0; i < vals.size(); ++i) {
            row_edits[i].first->setText(QString::number(vals[i].real(), 'g', 5));
            row_edits[i].second->setText(QString::number(vals[i].imag(), 'g', 5));
        }
        updating = false;
    }

    std::vector<Complex> getValues() const {
        std::vector<Complex> out;
        for (const auto& [re_e, im_e] : row_edits) {
            out.emplace_back(parseNumericInput(re_e->text().toStdString(), 0.0),
                             parseNumericInput(im_e->text().toStdString(), 0.0));
        }
        return out;
    }
};

struct PoleSweepEquivSystem {
    std::vector<Complex> eq_poles; // Roots at a = 0: s * D_rem(s) + K0 * N(s) = 0
    std::vector<Complex> eq_zeros; // Roots as a -> inf: D_rem(s) = 0
};

inline PoleSweepEquivSystem buildPoleSweepSystem(
    const std::vector<Complex>& loop_poles,
    const std::vector<Complex>& loop_zeros,
    int vary_pole_idx,
    double fixed_k0)
{
    PoleSweepEquivSystem out;
    if (loop_poles.empty()) return out;
    int idx = std::clamp(vary_pole_idx, 0, static_cast<int>(loop_poles.size()) - 1);

    std::vector<Complex> rem_poles;
    rem_poles.reserve(loop_poles.size() - 1);
    for (int i = 0; i < static_cast<int>(loop_poles.size()); ++i) {
        if (i != idx) rem_poles.push_back(loop_poles[i]);
    }
    out.eq_zeros = rem_poles;

    // s * D_rem(s) has roots at rem_poles plus a root at s = 0
    std::vector<Complex> s_drem_roots = rem_poles;
    s_drem_roots.push_back(Complex(0.0, 0.0));

    Poly d_eq_base = polyFromRoots(s_drem_roots);
    Poly n_orig    = polyFromRoots(loop_zeros);

    size_t deg = std::max(d_eq_base.size(), n_orig.size()) - 1;
    Poly d_pad(deg + 1, Complex(0.0, 0.0));
    Poly n_pad(deg + 1, Complex(0.0, 0.0));
    for (size_t i = 0; i < d_eq_base.size(); ++i) d_pad[deg + 1 - d_eq_base.size() + i] = d_eq_base[i];
    for (size_t i = 0; i < n_orig.size(); ++i)    n_pad[deg + 1 - n_orig.size() + i]    = n_orig[i];

    Poly d_eq_poly = polyAdd(d_pad, polyScale(n_pad, Complex(fixed_k0, 0.0)));
    out.eq_poles = solvePolynomialRoots(polyTrim(d_eq_poly));
    return out;
}