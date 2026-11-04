#pragma once
#include "BlockDiagramStudioTab.h"


// =============================================================================
// Engine 7 & Tab 8: Frequency-Domain Suite (Bode, Straight-Line Asymptotes & Nyquist)
// =============================================================================
struct FreqResponseResult {
    std::vector<double> w;
    std::vector<double> mag_abs;
    std::vector<double> mag_db;
    std::vector<double> asymp_db;
    std::vector<double> phase_deg;
    std::vector<Complex> c_vals;
    std::vector<double> corner_freqs;

    std::optional<double> w_gc;   // Gain crossover (0 dB)
    std::optional<double> pm_deg; // Phase margin (deg)
    std::optional<double> w_pc;   // Phase crossover (-180 deg)
    std::optional<double> gm_db;  // Gain margin (dB)

    double dc_gain_db = 0.0;
    double peak_mag_db = 0.0;
    double w_peak = 0.0;
    std::optional<double> w_bw;   // -3 dB bandwidth

    double cursor_w = 1.0;
    double cursor_mag_db = 0.0;
    double cursor_phase_deg = 0.0;
    Complex cursor_c = {1.0, 0.0};
};

class FrequencyDomainEngine {
public:
    static FreqResponseResult evaluate(
        const SharedSystemModel* model,
        int mode_idx, // 0 = Continuous Open-Loop L(jw), 1 = Continuous Closed-Loop T(jw), 2 = Discrete H(e^{jwTs})
        bool include_pid,
        double w_min_exp,
        double w_max_exp,
        double cursor_w,
        int n_pts = 500)
    {
        FreqResponseResult res;
        res.w.resize(n_pts);
        res.mag_abs.resize(n_pts);
        res.mag_db.resize(n_pts);
        res.asymp_db.resize(n_pts, 0.0);
        res.phase_deg.resize(n_pts);
        res.c_vals.resize(n_pts);

        Poly num, den;
        std::vector<Complex> active_poles, active_zeros;
        double ts = std::max(model->ts, 1e-5);

        if (mode_idx == 0 || mode_idx == 1) {
            auto [num_g, den_g] = ComplexTransferFunctionSimulator::buildPlantPolynomials(model->poles, model->zeros, model->plant_gain);
            auto [num_c, den_c] = ComplexTransferFunctionSimulator::buildPidPolynomials(model->kp, model->ki, model->kd, model->deriv_filter_n, include_pid);
            Poly num_h = {Complex(1.0, 0.0)}, den_h = {Complex(1.0, 0.0)};
            if (!model->unity_feedback_s) {
                std::tie(num_h, den_h) = ComplexTransferFunctionSimulator::buildPlantPolynomials(model->h_poles, model->h_zeros, model->h_gain);
            }
            Poly num_f = polyMul(num_c, num_g);
            Poly den_f = polyMul(den_c, den_g);
            Poly num_l = polyMul(num_f, num_h);
            Poly den_l = polyMul(den_f, den_h);

            if (mode_idx == 0) {
                num = polyTrim(num_l);
                den = polyTrim(den_l);
            } else {
                num = polyTrim(polyMul(num_f, den_h));
                den = polyTrim(polyAdd(den_l, num_l));
            }
            active_zeros = solvePolynomialRoots(num);
            active_poles = solvePolynomialRoots(den);
        } else {
            // Discrete H(z) or Closed-Loop Discrete T(z)
            Poly num_h = polyScale(polyFromRoots(model->z_zeros), Complex(model->filter_gain, 0.0));
            Poly den_h = polyFromRoots(model->z_poles);
            auto [b_h, a_h] = DiscreteFilterSimulator::toCausalBA(num_h, den_h);
            if (!include_pid) {
                num = b_h; den = a_h;
            } else {
                auto [num_c, den_c] = DiscreteFilterSimulator::buildDiscretePid(model->d_kp, model->d_ki, model->d_kd, ts, true);
                Poly nl = polyMul(num_c, b_h);
                Poly dl = polyMul(den_c, a_h);
                num = nl;
                den = polyAdd(dl, nl);
            }
        }

        // Frequency grid
        if (mode_idx == 2) {
            double w_nyq = PI / ts;
            double w0 = std::max(1e-4, w_nyq * 1e-3);
            double log_min = std::log10(w0), log_max = std::log10(w_nyq);
            for (int i = 0; i < n_pts; ++i) {
                res.w[i] = std::pow(10.0, log_min + (log_max - log_min) * double(i) / double(n_pts - 1));
            }
        } else {
            for (int i = 0; i < n_pts; ++i) {
                res.w[i] = std::pow(10.0, w_min_exp + (w_max_exp - w_min_exp) * double(i) / double(n_pts - 1));
            }
        }

        // Evaluate exact frequency response and unwrap phase
        double prev_rad = 0.0;
        double unwrap_offset = 0.0;
        for (int i = 0; i < n_pts; ++i) {
            Complex arg_pt = (mode_idx == 2) ? std::exp(Complex(0.0, res.w[i] * ts)) : Complex(0.0, res.w[i]);
            Complex n_v = polyVal(num, arg_pt);
            Complex d_v = polyVal(den, arg_pt);
            Complex val = (std::abs(d_v) > 1e-15) ? (n_v / d_v) : Complex(1e12, 0.0);

            res.c_vals[i] = val;
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

        // Compute Straight-Line Bode Asymptote for Continuous s-Domain
        if (mode_idx != 2) {
            int n_zero_orig = 0, n_pole_orig = 0;
            std::vector<double> z_corners, p_corners;
            for (const auto& z : active_zeros) {
                if (std::abs(z) < 1e-5) ++n_zero_orig;
                else { z_corners.push_back(std::abs(z)); res.corner_freqs.push_back(std::abs(z)); }
            }
            for (const auto& p : active_poles) {
                if (std::abs(p) < 1e-5) ++n_pole_orig;
                else { p_corners.push_back(std::abs(p)); res.corner_freqs.push_back(std::abs(p)); }
            }
            std::sort(res.corner_freqs.begin(), res.corner_freqs.end());

            // Base constant K_bode = |num_last_nz / den_last_nz|
            double num_c0 = 1.0, den_c0 = 1.0;
            for (auto it = num.rbegin(); it != num.rend(); ++it) if (std::abs(*it) > 1e-10) { num_c0 = std::abs(*it); break; }
            for (auto it = den.rbegin(); it != den.rend(); ++it) if (std::abs(*it) > 1e-10) { den_c0 = std::abs(*it); break; }
            double base_db = 20.0 * std::log10(std::max(num_c0 / den_c0, 1e-15));
            int orig_slope = n_zero_orig - n_pole_orig;

            for (int i = 0; i < n_pts; ++i) {
                double w = res.w[i];
                double db = base_db + 20.0 * orig_slope * std::log10(w);
                for (double wc : z_corners) if (w > wc) db += 20.0 * std::log10(w / wc);
                for (double wc : p_corners) if (w > wc) db -= 20.0 * std::log10(w / wc);
                res.asymp_db[i] = db;
            }
        }

        // Extract Gain Crossover (0 dB -> Phase Margin) & Phase Crossover (-180 deg -> Gain Margin)
        for (int i = 0; i + 1 < n_pts; ++i) {
            double m1 = res.mag_db[i], m2 = res.mag_db[i + 1];
            if (!res.w_gc.has_value() && ((m1 >= 0.0 && m2 <= 0.0) || (m1 <= 0.0 && m2 >= 0.0))) {
                double frac = (std::abs(m1 - m2) > 1e-12) ? (m1 / (m1 - m2)) : 0.5;
                res.w_gc = res.w[i] + frac * (res.w[i + 1] - res.w[i]);
                double ph_at_gc = res.phase_deg[i] + frac * (res.phase_deg[i + 1] - res.phase_deg[i]);
                // Normalize phase near -180 to compute PM = 180 + phase
                double ph_norm = std::fmod(ph_at_gc + 180.0, 360.0);
                if (ph_norm < 0.0) ph_norm += 360.0;
                res.pm_deg = ph_norm;
            }
            double p1 = res.phase_deg[i] + 180.0, p2 = res.phase_deg[i + 1] + 180.0;
            if (!res.w_pc.has_value() && ((p1 >= 0.0 && p2 <= 0.0) || (p1 <= 0.0 && p2 >= 0.0))) {
                double frac = (std::abs(p1 - p2) > 1e-12) ? (p1 / (p1 - p2)) : 0.5;
                res.w_pc = res.w[i] + frac * (res.w[i + 1] - res.w[i]);
                double db_at_pc = res.mag_db[i] + frac * (res.mag_db[i + 1] - res.mag_db[i]);
                res.gm_db = -db_at_pc;
            }
        }

        // Peak resonance & -3 dB Bandwidth
        res.dc_gain_db = res.mag_db.front();
        res.peak_mag_db = -1e30;
        for (int i = 0; i < n_pts; ++i) {
            if (res.mag_db[i] > res.peak_mag_db) {
                res.peak_mag_db = res.mag_db[i];
                res.w_peak = res.w[i];
            }
        }
        double bw_target = res.dc_gain_db - 3.0103;
        for (int i = 0; i + 1 < n_pts; ++i) {
            if (res.mag_db[i] >= bw_target && res.mag_db[i + 1] < bw_target) {
                double frac = (res.mag_db[i] - bw_target) / std::max(res.mag_db[i] - res.mag_db[i + 1], 1e-12);
                res.w_bw = res.w[i] + frac * (res.w[i + 1] - res.w[i]);
                break;
            }
        }

        // Cursor interpolation
        res.cursor_w = std::clamp(cursor_w, res.w.front(), res.w.back());
        int c_idx = 0;
        double best_d = 1e30;
        for (int i = 0; i < n_pts; ++i) {
            double d = std::abs( std::log10(res.w[i]) - std::log10(res.cursor_w) );
            if (d < best_d) { best_d = d; c_idx = i; }
        }
        res.cursor_w = res.w[c_idx];
        res.cursor_mag_db = res.mag_db[c_idx];
        res.cursor_phase_deg = res.phase_deg[c_idx];
        res.cursor_c = res.c_vals[c_idx];
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
    std::optional<FreqResponseResult> freq_res;
    std::function<void(double)> onFrequencySelected;

    // Nyquist Pan/Zoom bounds
    bool nyq_user_bounds = false;
    double n_min_r = -2.5, n_max_r = 2.5, n_min_i = -2.5, n_max_i = 2.5;
    bool panning = false;
    QPoint pan_start;

    explicit BodeNyquistCanvas(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumSize(480, 380);
        setCursor(Qt::CrossCursor);
    }

    void resetView() {
        nyq_user_bounds = false;
        n_min_r = -2.5; n_max_r = 2.5; n_min_i = -2.5; n_max_i = 2.5;
        update();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.fillRect(rect(), QColor(Theme::BG_PLOT));
        if (!freq_res.has_value() || freq_res->w.empty()) return;

        if (!nyquist_mode) drawBodePlot(p, freq_res.value());
        else drawNyquistPlot(p, freq_res.value());
    }

    void drawBodePlot(QPainter& p, const FreqResponseResult& r) {
        double pad_l = 58.0, pad_r = 22.0, pad_t = 18.0, pad_b = 34.0, gap = 24.0;
        double ew = std::max(20.0, width() - pad_l - pad_r);
        double eh_tot = std::max(40.0, height() - pad_t - pad_b - gap);
        double eh_mag = eh_tot * 0.52;
        double eh_ph  = eh_tot * 0.48;

        QRectF rect_mag(pad_l, pad_t, ew, eh_mag);
        QRectF rect_ph(pad_l, pad_t + eh_mag + gap, ew, eh_ph);

        double l_min = std::log10(r.w.front());
        double l_max = std::log10(r.w.back());

        double db_min = -60.0, db_max = 40.0;
        double ph_min = -270.0, ph_max = 0.0;
        for (size_t i = 0; i < r.w.size(); ++i) {
            if (std::isfinite(r.mag_db[i])) {
                db_min = std::min(db_min, r.mag_db[i]);
                db_max = std::max(db_max, r.mag_db[i]);
            }
            if (std::isfinite(r.phase_deg[i])) {
                ph_min = std::min(ph_min, r.phase_deg[i]);
                ph_max = std::max(ph_max, r.phase_deg[i]);
            }
        }
        db_min = std::floor((db_min - 10.0) / 20.0) * 20.0;
        db_max = std::ceil((db_max + 10.0) / 20.0) * 20.0;
        ph_min = std::floor((ph_min - 20.0) / 45.0) * 45.0;
        ph_max = std::ceil((ph_max + 20.0) / 45.0) * 45.0;

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
                if (lw < l_min || lw > l_max) continue;
                double x = toMagPx(w_v, 0.0).x();
                p.setPen(QPen(QColor(Theme::GRID_MAJOR), m == 1 ? 1.2 : 0.6));
                p.drawLine(QPointF(x, rect_mag.top()), QPointF(x, rect_mag.bottom()));
                p.drawLine(QPointF(x, rect_ph.top()),  QPointF(x, rect_ph.bottom()));
                if (m == 1) {
                    p.setPen(QColor(Theme::TEXT_SECONDARY));
                    p.drawText(QRectF(x - 28, rect_ph.bottom() + 5, 56, 18), Qt::AlignCenter, QString("10^%1").arg(dec));
                }
            }
        }

        // Horizontal Magnitude Grid (20 dB steps)
        for (double db = db_min; db <= db_max; db += 20.0) {
            double y = toMagPx(r.w.front(), db).y();
            p.setPen(QPen(QColor(std::abs(db) < 1e-3 ? Theme::AXIS_LINE : Theme::GRID_MAJOR), std::abs(db) < 1e-3 ? 1.5 : 1.0));
            p.drawLine(QPointF(rect_mag.left(), y), QPointF(rect_mag.right(), y));
            p.setPen(QColor(Theme::TEXT_SECONDARY));
            p.drawText(QRectF(2, y - 8, pad_l - 6, 16), Qt::AlignRight | Qt::AlignVCenter, QString("%1 dB").arg(db));
        }

        // Horizontal Phase Grid (45 deg steps)
        for (double deg = ph_min; deg <= ph_max; deg += 45.0) {
            double y = toPhPx(r.w.front(), deg).y();
            bool is_180 = std::abs(deg + 180.0) < 1e-3;
            p.setPen(QPen(QColor(is_180 ? Theme::ACCENT_POLE : Theme::GRID_MAJOR), is_180 ? 1.3 : 1.0, is_180 ? Qt::DashLine : Qt::SolidLine));
            p.drawLine(QPointF(rect_ph.left(), y), QPointF(rect_ph.right(), y));
            if (static_cast<int>(std::round(deg)) % 90 == 0 || is_180) {
                p.setPen(QColor(Theme::TEXT_SECONDARY));
                p.drawText(QRectF(2, y - 8, pad_l - 6, 16), Qt::AlignRight | Qt::AlignVCenter, QString("%1°").arg(deg));
            }
        }

        // Draw Magnitude & Straight-Line Asymptotes
        p.save();
        p.setClipRect(rect_mag);
        if (show_asymptotes && !r.asymp_db.empty()) {
            QPainterPath asymp_path;
            for (size_t i = 0; i < r.w.size(); ++i) {
                QPointF pt = toMagPx(r.w[i], r.asymp_db[i]);
                if (i == 0) asymp_path.moveTo(pt); else asymp_path.lineTo(pt);
            }
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor(Theme::ACCENT_OPEN), 1.5, Qt::DashLine));
            p.drawPath(asymp_path);
        }

        QPainterPath mag_path;
        for (size_t i = 0; i < r.w.size(); ++i) {
            QPointF pt = toMagPx(r.w[i], r.mag_db[i]);
            if (i == 0) mag_path.moveTo(pt); else mag_path.lineTo(pt);
        }
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(QColor(Theme::ACCENT_ZERO), 2.3));
        p.drawPath(mag_path);
        p.restore();

        // Draw Phase Curve
        p.save();
        p.setClipRect(rect_ph);
        QPainterPath ph_path;
        for (size_t i = 0; i < r.w.size(); ++i) {
            QPointF pt = toPhPx(r.w[i], r.phase_deg[i]);
            if (i == 0) ph_path.moveTo(pt); else ph_path.lineTo(pt);
        }
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(QColor(Theme::ACCENT_LOCUS), 2.2));
        p.drawPath(ph_path);
        p.restore();

        // Stability Margin Callout Lines (w_gc / PM and w_pc / GM)
        if (show_margins) {
            if (r.w_gc.has_value() && r.pm_deg.has_value()) {
                double wgc = r.w_gc.value();
                QPointF m0 = toMagPx(wgc, 0.0);
                QPointF p_m180 = toPhPx(wgc, -180.0);
                QPointF p_act  = toPhPx(wgc, -180.0 + r.pm_deg.value());
                p.setPen(QPen(QColor(Theme::ACCENT_PID), 1.4, Qt::DotLine));
                p.drawLine(m0, p_act);
                p.setPen(QPen(QColor(Theme::ACCENT_PID), 2.4));
                p.drawLine(p_m180, p_act);
                p.setBrush(QColor(Theme::ACCENT_PID));
                p.drawEllipse(m0, 4.5, 4.5);
                p.drawEllipse(p_act, 4.5, 4.5);
                p.drawText(p_act + QPointF(6, -6), QString("PM=%1° @ ωgc=%2")
                    .arg(QString::fromStdString(PRECISION.fmt(r.pm_deg)))
                    .arg(QString::fromStdString(PRECISION.fmt(wgc))));
            }
            if (r.w_pc.has_value() && r.gm_db.has_value()) {
                double wpc = r.w_pc.value();
                QPointF m0 = toMagPx(wpc, 0.0);
                QPointF m_act = toMagPx(wpc, -r.gm_db.value());
                QPointF p_m180 = toPhPx(wpc, -180.0);
                p.setPen(QPen(QColor(Theme::ACCENT_POLE), 1.4, Qt::DotLine));
                p.drawLine(m_act, p_m180);
                p.setPen(QPen(QColor(Theme::ACCENT_POLE), 2.4));
                p.drawLine(m0, m_act);
                p.setBrush(QColor(Theme::ACCENT_POLE));
                p.drawEllipse(m_act, 4.5, 4.5);
                p.drawEllipse(p_m180, 4.5, 4.5);
                p.drawText(m_act + QPointF(6, 12), QString("GM=%1dB @ ωpc=%2")
                    .arg(QString::fromStdString(PRECISION.fmt(r.gm_db)))
                    .arg(QString::fromStdString(PRECISION.fmt(wpc))));
            }
        }

        // Interactive Frequency Cursor
        QPointF cm = toMagPx(r.cursor_w, r.cursor_mag_db);
        QPointF cp = toPhPx(r.cursor_w, r.cursor_phase_deg);
        p.setPen(QPen(QColor(Theme::ACCENT_PID), 1.0, Qt::DashLine));
        p.drawLine(QPointF(cm.x(), rect_mag.top()), QPointF(cm.x(), rect_ph.bottom()));
        p.setBrush(Qt::white);
        p.setPen(QPen(QColor(Theme::ACCENT_ZERO), 2.0));
        p.drawEllipse(cm, 5.0, 5.0);
        p.setPen(QPen(QColor(Theme::ACCENT_LOCUS), 2.0));
        p.drawEllipse(cp, 5.0, 5.0);

        p.setPen(QColor(Theme::ACCENT_ZERO));
        p.setFont(QFont("Monospace", 9, QFont::Bold));
        p.drawText(cm + QPointF(8, -8), QString("ω=%1 r/s, |H|=%2 dB")
            .arg(QString::fromStdString(PRECISION.fmt(r.cursor_w)))
            .arg(QString::fromStdString(PRECISION.fmt(r.cursor_mag_db))));
        p.setPen(QColor(Theme::ACCENT_LOCUS));
        p.drawText(cp + QPointF(8, -8), QString("∠H=%1°").arg(QString::fromStdString(PRECISION.fmt(r.cursor_phase_deg))));
    }

    void drawNyquistPlot(QPainter& p, const FreqResponseResult& r) {
        double pad = 26.0;
        double ew = std::max(20.0, width() - 2.0 * pad);
        double eh = std::max(20.0, height() - 2.0 * pad);

        // Enforce 1:1 isotropic aspect ratio for Nyquist Unit Circle
        double span_r = std::max(1e-4, n_max_r - n_min_r);
        double span_i = std::max(1e-4, n_max_i - n_min_i);
        double upp = std::max(span_r / ew, span_i / eh);
        double mr = 0.5 * (n_min_r + n_max_r), mi = 0.5 * (n_min_i + n_max_i);
        double r0 = mr - 0.5 * upp * ew, r1 = mr + 0.5 * upp * ew;
        double i0 = mi - 0.5 * upp * eh, i1 = mi + 0.5 * upp * eh;

        auto toPx = [&](Complex c) {
            double x = pad + (c.real() - r0) / (r1 - r0) * ew;
            double y = pad + (i1 - c.imag()) / (i1 - i0) * eh;
            return QPointF(x, y);
        };

        // Grid & Axes
        double step = 0.5;
        if (r1 - r0 > 8.0) step = 2.0;
        else if (r1 - r0 > 4.0) step = 1.0;

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
        p.drawText(crit_px + QPointF(-42, -8), "-1+j0");

        // Mirror Nyquist Contour (w < 0)
        if (show_mirror_nyquist) {
            QPainterPath mir_path;
            bool st = false;
            for (const auto& c : r.c_vals) {
                if (std::abs(c) > 50.0) { st = false; continue; }
                QPointF pt = toPx(std::conj(c));
                if (!st) { mir_path.moveTo(pt); st = true; } else mir_path.lineTo(pt);
            }
            p.setBrush(Qt::NoBrush);
            p.setPen(QPen(QColor(Theme::TEXT_MUTED), 1.4, Qt::DashLine));
            p.drawPath(mir_path);
        }

        // Primary Nyquist Contour (w > 0)
        QPainterPath nyq_path;
        bool st = false;
        for (const auto& c : r.c_vals) {
            if (std::abs(c) > 50.0) { st = false; continue; }
            QPointF pt = toPx(c);
            if (!st) { nyq_path.moveTo(pt); st = true; } else nyq_path.lineTo(pt);
        }
        p.setBrush(Qt::NoBrush);
        p.setPen(QPen(QColor(Theme::ACCENT_LOCUS), 2.3));
        p.drawPath(nyq_path);

        // Cursor Point on Nyquist Contour
        QPointF c_px = toPx(r.cursor_c);
        p.setBrush(Qt::white);
        p.setPen(QPen(QColor(Theme::ACCENT_PID), 2.2));
        p.drawEllipse(c_px, 5.5, 5.5);
        p.setPen(QColor(Theme::ACCENT_PID));
        p.drawText(c_px + QPointF(10, -8), QString("ω=%1 r/s (%2)")
            .arg(QString::fromStdString(PRECISION.fmt(r.cursor_w)))
            .arg(QString::fromStdString(PRECISION.fmtComplex(r.cursor_c))));
    }

    void mousePressEvent(QMouseEvent* e) override {
        if (!nyquist_mode && e->button() == Qt::LeftButton) {
            updateFreqFromPx(e->pos().x());
        } else if (nyquist_mode && (e->button() == Qt::RightButton || e->button() == Qt::LeftButton)) {
            panning = true;
            pan_start = e->pos();
            setCursor(Qt::ClosedHandCursor);
        }
    }

    void mouseMoveEvent(QMouseEvent* e) override {
        if (!nyquist_mode && (e->buttons() & Qt::LeftButton)) {
            updateFreqFromPx(e->pos().x());
        } else if (nyquist_mode && panning) {
            double ew = std::max(20, width() - 52);
            double eh = std::max(20, height() - 52);
            double dr = -double(e->pos().x() - pan_start.x()) / ew * (n_max_r - n_min_r);
            double di =  double(e->pos().y() - pan_start.y()) / eh * (n_max_i - n_min_i);
            n_min_r += dr; n_max_r += dr;
            n_min_i += di; n_max_i += di;
            pan_start = e->pos();
            update();
        }
    }

    void mouseReleaseEvent(QMouseEvent*) override {
        panning = false;
        setCursor(Qt::CrossCursor);
    }

    void wheelEvent(QWheelEvent* e) override {
        if (!nyquist_mode) return;
        double factor = (e->angleDelta().y() > 0) ? 0.82 : 1.22;
        double mr = 0.5 * (n_min_r + n_max_r), mi = 0.5 * (n_min_i + n_max_i);
        double sr = (n_max_r - n_min_r) * factor * 0.5;
        double si = (n_max_i - n_min_i) * factor * 0.5;
        n_min_r = mr - sr; n_max_r = mr + sr;
        n_min_i = mi - si; n_max_i = mi + si;
        update();
    }

private:
    void updateFreqFromPx(double px) {
        if (!freq_res.has_value() || freq_res->w.empty()) return;
        double pad_l = 58.0, pad_r = 22.0;
        double ew = std::max(20.0, width() - pad_l - pad_r);
        double frac = std::clamp((px - pad_l) / ew, 0.0, 1.0);
        double l_min = std::log10(freq_res->w.front());
        double l_max = std::log10(freq_res->w.back());
        double w_sel = std::pow(10.0, l_min + frac * (l_max - l_min));
        if (onFrequencySelected) onFrequencySelected(w_sel);
    }
};

