#pragma once
#include "CoreAndWidgets.h"

class ComplexPlaneCanvas : public QWidget {
    Q_OBJECT
public:
    bool is_z_plane = false;
    bool show_locus = true;
    bool show_ghost_locus = false;
    std::vector<Complex> ghost_poles;
    std::vector<Complex> ghost_zeros;
    std::vector<Complex> cached_ghost_poles;
    std::vector<Complex> cached_ghost_zeros;
    std::vector<std::vector<Complex>> cached_ghost_branches;
    bool show_asymp = true;
    bool show_unit_circle = false;
    bool show_zgrid = false;

    std::vector<Complex> poles;
    std::vector<Complex> zeros;
    std::vector<Complex> cached_poles;
    std::vector<Complex> cached_zeros;
    std::vector<Complex> comp_poles;
    std::vector<Complex> comp_zeros;
    std::vector<std::vector<Complex>> cached_branches;
    Complex test_point = {-2.0, 2.0};
    bool is_on_rl = true;

    // s-Plane Design Spec Overlays (Ts, Tp, zeta) & Intersections
    bool show_ts_line = false;
    double spec_ts = 2.0;
    bool show_tp_line = false;
    double spec_tp = 1.0;
    bool show_zeta_line = false;
    double spec_zeta = 0.7071;

    struct SpecIntersection {
        Complex pt;
        double gain_k;
        double wn;
        QString spec_tag;
        QColor color;
    };
    std::vector<SpecIntersection> current_intersections;

    // Live Gain K Closed-Loop Pole Tracker
    bool show_live_k_poles = false;
    double live_k_val = 1.0;
    std::vector<Complex> live_cl_poles;

    // Breakaway / Break-In & jw-Axis Crossing Overlays
    bool show_breakaway = true;
    bool show_jw_crossings = true;
    bool show_dep_arr_angles = true;
    bool show_test_point = true;

    struct DepArrInfo {
        Complex pt;
        double angle_deg;
        bool is_departure; // true = Pole Departure, false = Zero Arrival
    };

    struct CriticalLocusPoint {
        Complex pt;
        double gain_k;
        double wn;
        QString label_tag; // "Brk-Out", "Brk-In", or "jω"
        QColor color;
    };
    std::vector<CriticalLocusPoint> critical_points;

    std::function<void(Complex)> onPointSelected;

    explicit ComplexPlaneCanvas(bool z_mode = false, QWidget* parent = nullptr)
        : QWidget(parent), is_z_plane(z_mode), show_unit_circle(z_mode), show_zgrid(z_mode)
    {
        setMinimumSize(420, 360);
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
        std::vector<Complex> pts = poles;
        pts.insert(pts.end(), zeros.begin(), zeros.end());

        if (show_ghost_locus) {
            pts.insert(pts.end(), ghost_poles.begin(), ghost_poles.end());
            pts.insert(pts.end(), ghost_zeros.begin(), ghost_zeros.end());
        }

        pts.push_back(test_point);
        if (is_z_plane) {
            pts.push_back({1.0, 0.0}); pts.push_back({-1.0, 0.0});
            pts.push_back({0.0, 1.0}); pts.push_back({0.0, -1.0});
        } else {
            pts.push_back({0.0, 0.0});
        }
        double r0 = pts[0].real(), r1 = pts[0].real(), i0 = pts[0].imag(), i1 = pts[0].imag();
        for (const auto& p : pts) {
            r0 = std::min(r0, p.real()); r1 = std::max(r1, p.real());
            i0 = std::min(i0, p.imag()); i1 = std::max(i1, p.imag());
        }
        double span = std::max({r1 - r0, i1 - i0, 2.4}) * 0.68;
        double mr = 0.5 * (r0 + r1), mi = 0.5 * (i0 + i1);
        min_r = mr - span; max_r = mr + span;
        min_i = mi - span; max_i = mi + span;
    }

    void enforceAspectRatio() {
        double pad = 24.0;
        double ew = std::max(1.0, width() - 2.0 * pad);
        double eh = std::max(1.0, height() - 2.0 * pad);
        double span_r = std::max(1e-6, max_r - min_r);
        double span_i = std::max(1e-6, max_i - min_i);
        double mid_r = 0.5 * (min_r + max_r);
        double mid_i = 0.5 * (min_i + max_i);

        // Use a single unified scale (units per pixel) for both X and Y
        double units_per_px = std::max(span_r / ew, span_i / eh);
        double new_span_r = units_per_px * ew;
        double new_span_i = units_per_px * eh;

        min_r = mid_r - 0.5 * new_span_r;
        max_r = mid_r + 0.5 * new_span_r;
        min_i = mid_i - 0.5 * new_span_i;
        max_i = mid_i + 0.5 * new_span_i;
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

    static void drawDirectionArrow(QPainter& p, QPointF p1, QPointF p2) {
        double dx = p2.x() - p1.x();
        double dy = p2.y() - p1.y();
        double len = std::hypot(dx, dy);
        if (len < 1e-3) return;
        double ux = dx / len, uy = dy / len;
        double px = -uy, py = ux;
        double mx = 0.5 * (p1.x() + p2.x()), my = 0.5 * (p1.y() + p2.y());
        double alen = 9.0, aw = 4.4;
        QPointF tip(mx + ux * (alen * 0.45), my + uy * (alen * 0.45));
        QPointF base(tip.x() - ux * alen, tip.y() - uy * alen);

        QPolygonF poly;
        poly << tip
             << QPointF(base.x() + px * aw, base.y() + py * aw)
             << QPointF(base.x() + ux * 2.2, base.y() + uy * 2.2)
             << QPointF(base.x() - px * aw, base.y() - py * aw);
        p.setPen(QPen(QColor(Theme::BG_PLOT), 0.8));
        p.setBrush(QColor(Theme::ACCENT_ARROW));
        p.drawPolygon(poly);
    }

    void paintEvent(QPaintEvent*) override {
        if (!user_bounds_set) computeAutoBounds();
        enforceAspectRatio();

        QPainter p(this);
        QColor badge_bg(Theme::BG_PANEL);
        badge_bg.setAlpha(235);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.fillRect(rect(), QColor(Theme::BG_PLOT));

        // Grid & Tick Labels (1:1 Square Grid)
        double span = std::min(max_r - min_r, max_i - min_i);
        double raw_step = std::max(span / 6.0, 1e-5);
        double mag = std::pow(10.0, std::floor(std::log10(raw_step)));
        double frac = raw_step / mag;
        double step = (frac < 1.5) ? (1.0 * mag) : ((frac < 3.5) ? (2.0 * mag) : ((frac < 7.5) ? (5.0 * mag) : (10.0 * mag)));

        QPointF orig = toPx({0.0, 0.0});
        p.setFont(QFont("Monospace", 8));

        for (double rv = std::floor(min_r / step) * step; rv <= max_r; rv += step) {
            if (std::abs(rv) < 1e-9) rv = 0.0;
            double x = toPx({rv, 0.0}).x();
            p.setPen(QPen(QColor(Theme::GRID_MAJOR), 1.0));
            p.drawLine(QPointF(x, 0), QPointF(x, height()));
            if (std::abs(rv) > 1e-9 && x > 28 && x < width() - 28) {
                p.setPen(QColor(Theme::TEXT_SECONDARY));
                double lbl_y = std::clamp(orig.y() + 14.0, 14.0, height() - 6.0);
                p.drawText(QRectF(x - 24, lbl_y - 10, 48, 16), Qt::AlignCenter, QString::number(rv, 'g', 4));
            }
        }
        for (double iv = std::floor(min_i / step) * step; iv <= max_i; iv += step) {
            if (std::abs(iv) < 1e-9) iv = 0.0;
            double y = toPx({0.0, iv}).y();
            p.setPen(QPen(QColor(Theme::GRID_MAJOR), 1.0));
            p.drawLine(QPointF(0, y), QPointF(width(), y));
            if (std::abs(iv) > 1e-9 && y > 18 && y < height() - 18) {
                p.setPen(QColor(Theme::TEXT_SECONDARY));
                double lbl_x = std::clamp(orig.x() + 6.0, 6.0, width() - 48.0);
                p.drawText(QRectF(lbl_x, y - 8, 48, 16), Qt::AlignLeft | Qt::AlignVCenter, QString::number(iv, 'g', 4) + "j");
            }
        }

        // Axes
        p.setPen(QPen(QColor(Theme::AXIS_LINE), 1.5));
        p.drawLine(QPointF(0, orig.y()), QPointF(width(), orig.y()));
        p.drawLine(QPointF(orig.x(), 0), QPointF(orig.x(), height()));

        // Constant-zeta spirals on z-plane
        if (is_z_plane && show_zgrid) {
            p.setPen(QPen(QColor("#1e3a4c"), 1.0, Qt::DotLine));
            p.setBrush(Qt::NoBrush);
            for (double zg : {0.2, 0.4, 0.6, 0.8}) {
                double factor = zg / std::sqrt(std::max(1e-6, 1.0 - zg * zg));
                QPainterPath up_path, dn_path;
                for (int k = 0; k <= 80; ++k) {
                    double th = PI * double(k) / 80.0;
                    double r_sp = std::exp(-factor * th);
                    Complex zu = std::polar(r_sp, th * std::sqrt(1.0 - zg * zg));
                    QPointF pu = toPx(zu);
                    QPointF pd = toPx(std::conj(zu));
                    if (k == 0) { up_path.moveTo(pu); dn_path.moveTo(pd); }
                    else { up_path.lineTo(pu); dn_path.lineTo(pd); }
                }
                p.drawPath(up_path);
                p.drawPath(dn_path);
            }
        }

        // Unit Circle for z-Plane
        if (show_unit_circle) {
            QPointF p_tl = toPx({-1.0, 1.0});
            QPointF p_br = toPx({1.0, -1.0});
            p.setPen(QPen(QColor(Theme::ACCENT_ASYMP), 1.8, Qt::DashLine));
            p.setBrush(Qt::NoBrush);
            p.drawEllipse(QRectF(p_tl, p_br));
            p.setFont(QFont("Monospace", 8));
            p.drawText(toPx({1.0, 0.0}) + QPointF(6, 12), "z=1 (DC)");
            p.drawText(toPx({-1.0, 0.0}) + QPointF(-52, 12), "z=-1 (Nyq)");
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

        // Uncompensated "Ghost" Root Locus Overlay (Dashed Reference Curve + Faint Poles/Zeros)
        if (show_ghost_locus && !ghost_poles.empty()) {
            if (ghost_poles != cached_ghost_poles || ghost_zeros != cached_ghost_zeros) {
                cached_ghost_poles = ghost_poles;
                cached_ghost_zeros = ghost_zeros;
                cached_ghost_branches = computeRootLocusBranches(ghost_poles, ghost_zeros);
            }
            double max_jump = std::max(width(), height()) * 0.45;
            QColor ghost_col(148, 163, 184, 120); // Muted Slate-Silver dashed curve
            p.setPen(QPen(ghost_col, 1.8, Qt::DashLine));
            p.setBrush(Qt::NoBrush);

            for (const auto& br : cached_ghost_branches) {
                QPainterPath gpath;
                bool started = false;
                QPointF prev_px;
                for (const auto& pt : br) {
                    QPointF cur_px = toPx(pt);
                    if (!started) {
                        gpath.moveTo(cur_px);
                        started = true;
                    } else {
                        if (std::hypot(cur_px.x() - prev_px.x(), cur_px.y() - prev_px.y()) > max_jump)
                            gpath.moveTo(cur_px);
                        else
                            gpath.lineTo(cur_px);
                    }
                    prev_px = cur_px;
                }
                p.drawPath(gpath);
            }

            // Faint Ghost Open-Loop Poles (×) and Zeros (○)
            p.setPen(QPen(QColor(255, 95, 86, 110), 1.6));
            for (const auto& gp : ghost_poles) {
                QPointF gpx = toPx(gp);
                p.drawLine(gpx + QPointF(-4.5, -4.5), gpx + QPointF(4.5, 4.5));
                p.drawLine(gpx + QPointF(-4.5,  4.5), gpx + QPointF(4.5, -4.5));
            }
            p.setPen(QPen(QColor(56, 189, 248, 110), 1.6));
            for (const auto& gz : ghost_zeros) {
                p.drawEllipse(toPx(gz), 4.5, 4.5);
            }
        }

        // Root Locus Branches + Directional Arrows (➤)
        if (show_locus) {
            if (poles != cached_poles || zeros != cached_zeros) {
                cached_poles = poles;
                cached_zeros = zeros;
                cached_branches = computeRootLocusBranches(poles, zeros);
            }
            const auto& branches = cached_branches;
            double max_jump = std::max(width(), height()) * 0.45;
            for (const auto& br : branches) {
                QPainterPath path;
                bool started = false;
                QPointF prev_px;
                double accum_px = 0.0, next_arrow_at = 42.0, arrow_spacing = 85.0;
                std::vector<std::pair<QPointF, QPointF>> arrows;

                for (const auto& pt : br) {
                    QPointF cur_px = toPx(pt);
                    if (!started) {
                        path.moveTo(cur_px);
                        started = true;
                    } else {
                        double dist = std::hypot(cur_px.x() - prev_px.x(), cur_px.y() - prev_px.y());
                        if (dist > max_jump) {
                            p.setPen(QPen(QColor(Theme::ACCENT_LOCUS), 2.2));
                            p.setBrush(Qt::NoBrush);
                            p.drawPath(path);
                            path = QPainterPath();
                            path.moveTo(cur_px);
                            accum_px = 0.0; next_arrow_at = 42.0;
                        } else {
                            path.lineTo(cur_px);
                            if (cur_px.x() >= -20 && cur_px.x() <= width() + 20 &&
                                cur_px.y() >= -20 && cur_px.y() <= height() + 20 && dist > 0.8) {
                                accum_px += dist;
                                if (accum_px >= next_arrow_at) {
                                    arrows.push_back({prev_px, cur_px});
                                    next_arrow_at = accum_px + arrow_spacing;
                                }
                            }
                        }
                    }
                    prev_px = cur_px;
                }
                p.setPen(QPen(QColor(Theme::ACCENT_LOCUS), 2.2));
                p.setBrush(Qt::NoBrush);
                p.drawPath(path);
                for (const auto& [a1, a2] : arrows) drawDirectionArrow(p, a1, a2);
            }
        }

        // =====================================================================
        // s-Plane Spec Lines (Ts, Tp, zeta), RL Intersections & Live K Poles
        // =====================================================================
        current_intersections.clear();
        if (!is_z_plane) {
            if (poles != cached_poles || zeros != cached_zeros) {
                cached_poles = poles;
                cached_zeros = zeros;
                cached_branches = computeRootLocusBranches(poles, zeros);
            }
            const auto& branches = cached_branches;

            auto addCrossing = [&](Complex p1, Complex p2, double t_frac, const QString& tag, const QColor& col) {
                Complex x_pt = p1 + t_frac * (p2 - p1);
                // Avoid duplicate markers if branches overlap or touch real axis
                for (const auto& ex : current_intersections) {
                    if (std::abs(ex.pt - x_pt) < 1e-2 * span && ex.spec_tag == tag) return;
                }
                auto ev = evaluateTransferFunction(poles, zeros, x_pt, 1.0);
                current_intersections.push_back({x_pt, ev.rl_gain_k, std::abs(x_pt), tag, col});
            };

            // 1. Settling Time Ts Line: Re(s) = -4 / Ts
            if (show_ts_line && spec_ts > 1e-4) {
                double sigma_target = -4.0 / spec_ts;
                double x_px = toPx({sigma_target, 0.0}).x();
                QColor ts_col("#38bdf8"); // Sky Blue
                p.setPen(QPen(ts_col, 1.5, Qt::DashLine));
                p.drawLine(QPointF(x_px, 0), QPointF(x_px, height()));
                p.setFont(QFont("Monospace", 8, QFont::Bold));
                p.drawText(QPointF(x_px + 5, 18), QString("Ts=%1s (σ=%2)")
                    .arg(QString::fromStdString(PRECISION.fmt(spec_ts)))
                    .arg(QString::fromStdString(PRECISION.fmt(sigma_target))));

                for (const auto& br : branches) {
                    for (size_t i = 0; i + 1 < br.size(); ++i) {
                        double f1 = br[i].real() - sigma_target;
                        double f2 = br[i + 1].real() - sigma_target;
                        if ((f1 <= 0.0 && f2 >= 0.0) || (f1 >= 0.0 && f2 <= 0.0)) {
                            double denom = f1 - f2;
                            if (std::abs(denom) > 1e-12) {
                                addCrossing(br[i], br[i + 1], f1 / denom, "Ts", ts_col);
                            }
                        }
                    }
                }
            }

            // 2. Peak Time Tp Lines: Im(s) = ±pi / Tp
            if (show_tp_line && spec_tp > 1e-4) {
                double wd_target = PI / spec_tp;
                QColor tp_col("#f472b6"); // Pink-Magenta
                p.setPen(QPen(tp_col, 1.5, Qt::DashLine));
                double y_pos = toPx({0.0,  wd_target}).y();
                double y_neg = toPx({0.0, -wd_target}).y();
                p.drawLine(QPointF(0, y_pos), QPointF(width(), y_pos));
                p.drawLine(QPointF(0, y_neg), QPointF(width(), y_neg));
                p.setFont(QFont("Monospace", 8, QFont::Bold));
                p.drawText(QPointF(30, y_pos - 5), QString("Tp=%1s (+ωd=%2)")
                    .arg(QString::fromStdString(PRECISION.fmt(spec_tp)))
                    .arg(QString::fromStdString(PRECISION.fmt(wd_target))));

                for (double sign_w : {1.0, -1.0}) {
                    double w_aim = sign_w * wd_target;
                    for (const auto& br : branches) {
                        for (size_t i = 0; i + 1 < br.size(); ++i) {
                            double f1 = br[i].imag() - w_aim;
                            double f2 = br[i + 1].imag() - w_aim;
                            if ((f1 <= 0.0 && f2 >= 0.0) || (f1 >= 0.0 && f2 <= 0.0)) {
                                double denom = f1 - f2;
                                if (std::abs(denom) > 1e-12) {
                                    addCrossing(br[i], br[i + 1], f1 / denom, "Tp", tp_col);
                                }
                            }
                        }
                    }
                }
            }

            // 3. Damping Ratio zeta Rays: s = -r*zeta ± j*r*sqrt(1 - zeta^2)
            if (show_zeta_line && spec_zeta >= 0.0 && spec_zeta <= 1.0) {
                QColor z_col("#facc15"); // Amber-Gold
                double sin_th = std::sqrt(std::max(0.0, 1.0 - spec_zeta * spec_zeta));
                double ray_len = span * 4.0;
                QPointF p_orig = toPx({0.0, 0.0});
                QPointF p_up   = toPx({-ray_len * spec_zeta,  ray_len * sin_th});
                QPointF p_dn   = toPx({-ray_len * spec_zeta, -ray_len * sin_th});

                p.setPen(QPen(z_col, 1.5, Qt::DashLine));
                p.drawLine(p_orig, p_up);
                if (sin_th > 1e-6) p.drawLine(p_orig, p_dn);

                QPointF lbl_pos = toPx({-0.38 * span * spec_zeta, 0.38 * span * sin_th});
                p.setFont(QFont("Monospace", 8, QFont::Bold));
                p.drawText(lbl_pos + QPointF(6, -6), QString("ζ=%1 (%OS=%2%)")
                    .arg(QString::fromStdString(PRECISION.fmt(spec_zeta)))
                    .arg(QString::fromStdString(PRECISION.fmt(niseOsFromZeta(spec_zeta)))));

                // Ray equation in LHP: sin_th * Re(s) + spec_zeta * |Im(s)| = 0
                std::vector<double> signs = (sin_th > 1e-6) ? std::vector<double>{1.0, -1.0}
                                                            : std::vector<double>{1.0};
                for (double sign_im : signs) {
                    for (const auto& br : branches) {
                        for (size_t i = 0; i + 1 < br.size(); ++i) {
                            if (br[i].real() > 1e-4 && br[i + 1].real() > 1e-4) continue;
                            if (sign_im * br[i].imag() < -1e-5 && sign_im * br[i + 1].imag() < -1e-5) continue;
                            double f1 = sin_th * br[i].real() + spec_zeta * (sign_im * br[i].imag());
                            double f2 = sin_th * br[i + 1].real() + spec_zeta * (sign_im * br[i + 1].imag());
                            if ((f1 <= 0.0 && f2 >= 0.0) || (f1 >= 0.0 && f2 <= 0.0)) {
                                double denom = f1 - f2;
                                if (std::abs(denom) > 1e-12) {
                                    double t_f = f1 / denom;
                                    Complex cand = br[i] + t_f * (br[i + 1] - br[i]);
                                    if (cand.real() <= 1e-4 && sign_im * cand.imag() >= -1e-4 && std::abs(cand) > 1e-4) {
                                        addCrossing(br[i], br[i + 1], t_f, "ζ", z_col);
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // 3B. Pairwise Spec-Line Target Intersections (Ts∩Tp, ζ∩Ts, ζ∩Tp) — Clickable to Snap!
            bool valid_ts = (show_ts_line && spec_ts > 1e-4);
            bool valid_tp = (show_tp_line && spec_tp > 1e-4);
            bool valid_z  = (show_zeta_line && spec_zeta >= 0.0 && spec_zeta <= 1.0);

            auto addSpecTarget = [&](Complex spt, const QString& stag, const QColor& col) {
                auto ev_t = evaluateTransferFunction(poles, zeros, spt, 1.0);
                current_intersections.push_back({spt, ev_t.rl_gain_k, std::abs(spt), stag, col});
            };

            if (valid_ts && valid_tp) {
                double sig = -4.0 / spec_ts;
                double wd  = PI / spec_tp;
                addSpecTarget({sig,  wd}, "Ts∩Tp", QColor("#2dd4bf"));
                addSpecTarget({sig, -wd}, "Ts∩Tp", QColor("#2dd4bf"));
            }
            if (valid_ts && valid_z && spec_zeta > 1e-4) {
                double sig = -4.0 / spec_ts;
                double wd  = (-sig) * std::sqrt(std::max(0.0, 1.0 - spec_zeta * spec_zeta)) / spec_zeta;
                addSpecTarget({sig,  wd}, "ζ∩Ts", QColor("#34d399"));
                if (wd > 1e-6) addSpecTarget({sig, -wd}, "ζ∩Ts", QColor("#34d399"));
            }
            if (valid_tp && valid_z && spec_zeta < 1.0 - 1e-6) {
                double wd  = PI / spec_tp;
                double sig = -wd * spec_zeta / std::sqrt(std::max(1e-12, 1.0 - spec_zeta * spec_zeta));
                addSpecTarget({sig,  wd}, "ζ∩Tp", QColor("#c084fc"));
                addSpecTarget({sig, -wd}, "ζ∩Tp", QColor("#c084fc"));
            }

            // Draw RL Intersection Diamonds (◆) & Spec-Pair Target Bullseyes (⊙) + Callouts
            p.setFont(QFont("Monospace", 8, QFont::Bold));
            for (const auto& ix : current_intersections) {
                QPointF ipx = toPx(ix.pt);
                bool is_spec_pair = ix.spec_tag.contains("∩");

                if (is_spec_pair) {
                    // Concentric bullseye marker (⊙) for spec-line intersections
                    p.setPen(QPen(Qt::white, 1.3));
                    p.setBrush(badge_bg);
                    p.drawEllipse(ipx, 6.8, 6.8);
                    p.setPen(QPen(ix.color, 1.5));
                    p.setBrush(ix.color);
                    p.drawEllipse(ipx, 3.0, 3.0);
                } else {
                    // Diamond marker (◆) for Root Locus crossings
                    QPolygonF dia;
                    dia << (ipx + QPointF(0, -6)) << (ipx + QPointF(6, 0))
                        << (ipx + QPointF(0,  6)) << (ipx + QPointF(-6, 0));
                    p.setPen(QPen(Qt::white, 1.2));
                    p.setBrush(ix.color);
                    p.drawPolygon(dia);
                }

                // Draw compact callout label on upper-half-plane & real axis to prevent conjugate clutter
                if (ix.pt.imag() >= -1e-4) {
                    QString txt;
                    if (is_spec_pair) {
                        auto ev_s = evaluateTransferFunction(poles, zeros, ix.pt, 1.0);
                        double def_d = computeAngleDeficiency(ev_s.angle_deg);
                        txt = QString("[%1] s=%2 | Δθ=%3°, K=%4")
                            .arg(ix.spec_tag)
                            .arg(QString::fromStdString(PRECISION.fmtComplex(ix.pt)))
                            .arg(QString::fromStdString(PRECISION.fmt(def_d, true)))
                            .arg(ev_s.at_zero ? "Inf" : QString::fromStdString(PRECISION.fmt(ix.gain_k)));
                    } else {
                        txt = QString("[%1] K=%2, ωn=%3")
                            .arg(ix.spec_tag)
                            .arg(QString::fromStdString(PRECISION.fmt(ix.gain_k)))
                            .arg(QString::fromStdString(PRECISION.fmt(ix.wn)));
                    }

                    QFontMetrics fm(p.font());
                    int tw = fm.horizontalAdvance(txt) + 10;
                    QColor fg_col = Theme::is_light ? ix.color.darker(135) : ix.color;
                    double y_off = is_spec_pair ? 8.0 : -20.0; // Offset below for bullseye so it doesn't collide with RL diamond!
                    QRectF box(ipx.x() + 8, ipx.y() + y_off, tw, 18);
                    p.setBrush(badge_bg);
                    p.setPen(QPen(fg_col, 1.1));
                    p.drawRoundedRect(box, 3, 3);
                    p.setPen(fg_col);
                    p.drawText(box, Qt::AlignCenter, txt);
                }
            }
            
            // 4. Draw Live Gain K Closed-Loop Poles (■)
            if (show_live_k_poles && !live_cl_poles.empty()) {
                for (const auto& cp : live_cl_poles) {
                    QPointF cpx = toPx(cp);
                    QRectF sq(cpx.x() - 5.5, cpx.y() - 5.5, 11.0, 11.0);
                    p.setBrush(QColor(Theme::ACCENT_PID));
                    p.setPen(QPen(Qt::white, 1.4));
                    p.drawRect(sq);
                    if (cp.imag() >= -1e-4) {
                        p.setPen(QColor(Theme::ACCENT_PID));
                        p.setFont(QFont("Monospace", 8, QFont::Bold));
                        p.drawText(cpx + QPointF(8, 14), QString("s_CL(%1) [K=%2]")
                            .arg(QString::fromStdString(PRECISION.fmtComplex(cp)))
                            .arg(QString::fromStdString(PRECISION.fmt(live_k_val))));
                    }
                }
            }

            // 5. Real-Axis Breakaway / Break-In Points: N(s)*D'(s) - D(s)*N'(s) = 0
            critical_points.clear();
            if (show_breakaway && !poles.empty()) {
                Poly D = polyFromRoots(poles);
                Poly N = polyFromRoots(zeros);
                Poly dD = polyDeriv(D);
                Poly dN = polyDeriv(N);
                Poly char_brk = polySub(polyMul(N, dD), polyMul(D, dN));
                auto cand_roots = solvePolynomialRoots(char_brk);

                for (const auto& r : cand_roots) {
                    if (std::abs(r.imag()) > 1e-3) continue; // Must be on real axis
                    double sigma = r.real();
                    // Skip if it coincides directly with an open-loop single pole/zero
                    Complex n_val = polyVal(N, Complex(sigma, 0.0));
                    Complex d_val = polyVal(D, Complex(sigma, 0.0));
                    if (std::abs(n_val) < 1e-9) continue;

                    double k_here = -(d_val / n_val).real();
                    if (k_here <= 1e-6 || !std::isfinite(k_here)) continue;

                    // Verify angle condition at sigma + j0
                    auto ev = evaluateTransferFunction(poles, zeros, Complex(sigma, 0.0), 1.0);
                    if (!ev.at_pole && std::abs(computeAngleDeficiency(ev.angle_deg)) > 2.0) continue;

                    // Deduplicate nearby roots
                    bool dup = false;
                    for (const auto& cp : critical_points) {
                        if (std::abs(cp.pt.real() - sigma) < 1e-2 * span) { dup = true; break; }
                    }
                    if (dup) continue;

                    // Check second derivative / neighbor K to classify Breakaway vs Break-In
                    double eps = 1e-3 * std::max(1.0, std::abs(sigma));
                    double k_left  = -(polyVal(D, sigma - eps) / polyVal(N, sigma - eps)).real();
                    double k_right = -(polyVal(D, sigma + eps) / polyVal(N, sigma + eps)).real();
                    bool is_break_in = (k_here < k_left && k_here < k_right);

                    critical_points.push_back({
                        Complex(sigma, 0.0),
                        k_here,
                        std::abs(sigma),
                        is_break_in ? "Brk-In" : "Brk-Out",
                        QColor("#fb923c") // Coral-Orange
                    });
                }
            }

            // 6. jw-Axis Crossings: Re(s) = 0 along Root Locus branches (with Exact Angle Polishing)
            if (show_jw_crossings) {
                auto polishJwCrossing = [&](double w_init, double w_lo, double w_hi) -> std::pair<bool, double> {
                    // On the Root Locus (K > 0), a point s = j*w is on the locus iff angleDeficiency(j*w) == 0.
                    auto angleErr = [&](double w) -> double {
                        auto ev = evaluateTransferFunction(poles, zeros, Complex(0.0, w), 1.0);
                        // Signed angle error from ±180 deg in [-180, 180]
                        double err = std::fmod(ev.angle_deg - 180.0, 360.0);
                        if (err <= -180.0) err += 360.0;
                        if (err >   180.0) err -= 360.0;
                        return err;
                    };

                    double w = w_init;
                    for (int it = 0; it < 12; ++it) {
                        double f = angleErr(w);
                        if (std::abs(f) < 1e-9) return {true, w};
                        double eps = std::max(1e-6, std::abs(w) * 1e-5);
                        double df = (angleErr(w + eps) - angleErr(w - eps)) / (2.0 * eps);
                        if (std::abs(df) < 1e-12) break;
                        double w_next = w - f / df;
                        // Keep within reasonable neighborhood of the bracket
                        double pad = std::max(2.0, std::abs(w_hi - w_lo) * 2.0);
                        if (w_next < std::min(w_lo, w_hi) - pad || w_next > std::max(w_lo, w_hi) + pad) break;
                        w = w_next;
                    }
                    return {std::abs(angleErr(w)) < 0.05, w};
                };

                auto addUniqueJw = [&](Complex jw_pt, double k_val) {
                    if (!std::isfinite(k_val) || k_val < -1e-7) return;
                    k_val = std::max(0.0, k_val);
                    for (const auto& cp : critical_points) {
                        if (cp.label_tag == "jω" && std::abs(cp.pt - jw_pt) < 1e-2 * span) return;
                    }
                    critical_points.push_back({
                        jw_pt,
                        k_val,
                        std::abs(jw_pt.imag()),
                        "jω",
                        QColor("#22d3ee") // Bright Cyan
                    });
                };

                // 6A. Open-loop poles starting directly on the jw-axis at K = 0 (excluding origin s = 0)
                for (const auto& p_ol : poles) {
                    if (std::abs(p_ol.real()) < 1e-5 && std::abs(p_ol.imag()) > 1e-4) {
                        addUniqueJw(Complex(0.0, p_ol.imag()), 0.0);
                    }
                }

                // 6B. Exact analytical check for real-axis crossing through the origin (s = 0)
                if (!poles.empty()) {
                    Poly D_ol = polyFromRoots(poles);
                    Poly N_ol = polyFromRoots(zeros);
                    double d0 = D_ol.back().real();
                    double n0 = N_ol.back().real();
                    if (std::abs(n0) > 1e-10 && std::abs(d0) > 1e-10) {
                        double k_orig = -d0 / n0;
                        if (k_orig > 1e-6) {
                            addUniqueJw(Complex(0.0, 0.0), k_orig);
                        }
                    }
                }

                // 6C. Branch sign-change detection + exact Newton angle polishing on s = j*w
                for (const auto& br : branches) {
                    for (size_t i = 0; i + 1 < br.size(); ++i) {
                        double r1 = br[i].real();
                        double r2 = br[i + 1].real();
                        if ((r1 < -1e-7 && r2 >= -1e-7) || (r1 > 1e-7 && r2 <= 1e-7)) {
                            double denom = r1 - r2;
                            if (std::abs(denom) < 1e-12) continue;
                            double t_f = r1 / denom;
                            double w1 = br[i].imag(), w2 = br[i + 1].imag();
                            double w_guess = w1 + t_f * (w2 - w1);
                            if (std::abs(w_guess) < 1e-4) continue; // Origin handled by 6B

                            auto [ok, w_exact] = polishJwCrossing(w_guess, w1, w2);
                            double w_final = ok ? w_exact : w_guess;
                            Complex jw_pt(0.0, w_final);
                            auto ev = evaluateTransferFunction(poles, zeros, jw_pt, 1.0);
                            if (std::isfinite(ev.rl_gain_k) && ev.rl_gain_k > 0.0) {
                                addUniqueJw(jw_pt, ev.rl_gain_k);
                            }
                        }
                    }
                }
            }

            // Draw Breakaway/Break-In & jw-Axis Crossing Markers + Callout Badges
            p.setFont(QFont("Monospace", 8, QFont::Bold));
            for (const auto& cp : critical_points) {
                QPointF cpx = toPx(cp.pt);

                if (cp.label_tag == "jω") {
                    // Draw a target circle on the jw axis
                    p.setPen(QPen(Qt::white, 1.4));
                    p.setBrush(cp.color);
                    p.drawEllipse(cpx, 5.5, 5.5);
                } else {
                    // Draw a rotated square / diamond on the real axis for Breakaway/Break-In
                    QPolygonF dia;
                    dia << (cpx + QPointF(0, -6.5)) << (cpx + QPointF(6.5, 0))
                        << (cpx + QPointF(0,  6.5)) << (cpx + QPointF(-6.5, 0));
                    p.setPen(QPen(Qt::white, 1.4));
                    p.setBrush(cp.color);
                    p.drawPolygon(dia);
                }

                // Draw callout label (for upper-half-plane jw crossings and all real breakaway points)
                if (cp.pt.imag() >= -1e-4) {
                    QString txt = (cp.label_tag == "jω")
                        ? QString("[jω=±%1j] K=%2, ωn=%1")
                              .arg(QString::fromStdString(PRECISION.fmt(cp.wn)))
                              .arg(QString::fromStdString(PRECISION.fmt(cp.gain_k)))
                        : QString("[%1 s=%2] K=%3")
                              .arg(cp.label_tag)
                              .arg(QString::fromStdString(PRECISION.fmt(cp.pt.real())))
                              .arg(QString::fromStdString(PRECISION.fmt(cp.gain_k)));

                    QColor fg_col = Theme::is_light ? cp.color.darker(135) : cp.color;
                    QFontMetrics fm(p.font());
                    int tw = fm.horizontalAdvance(txt) + 10;
                    double y_off = (cp.label_tag == "jω") ? -22.0 : 10.0;
                    QRectF box(cpx.x() + 8, cpx.y() + y_off, tw, 18);
                    p.setBrush(badge_bg);
                    p.setPen(QPen(fg_col, 1.1));
                    p.drawRoundedRect(box, 3, 3);
                    p.setPen(fg_col);
                    p.drawText(box, Qt::AlignCenter, txt);
                }
            }
        }

        // Vector rays to Test Point
        QPointF tp_px;
        if (show_test_point) {
            tp_px = toPx(test_point);
            p.setPen(QPen(QColor("#1e40af"), 1.1, Qt::DotLine));
            for (const auto& z : zeros) p.drawLine(toPx(z), tp_px);
            p.setPen(QPen(QColor("#881337"), 1.1, Qt::DotLine));
            for (const auto& pl : poles) p.drawLine(toPx(pl), tp_px);
        }
        

        // Zeros (o)
        p.setPen(QPen(QColor(Theme::ACCENT_ZERO), 2.2));
        p.setBrush(QColor(Theme::BG_PLOT));
        for (const auto& z : zeros) p.drawEllipse(toPx(z), 6.0, 6.0);

        // Poles (x)
        p.setPen(QPen(QColor(Theme::ACCENT_POLE), 2.2));
        for (const auto& pl : poles) {
            QPointF px = toPx(pl);
            p.drawLine(px + QPointF(-6, -6), px + QPointF(6, 6));
            p.drawLine(px + QPointF(-6, 6), px + QPointF(6, -6));
        }

        // Compensator Gc(s) Zeros (○) & Poles (×) Highlighted in Emerald Green (#34d399)
        if (!comp_zeros.empty() || !comp_poles.empty()) {
            QColor comp_col("#34d399");
            p.setPen(QPen(comp_col, 2.5));
            p.setBrush(QColor(Theme::BG_PLOT));
            for (const auto& cz : comp_zeros) {
                QPointF zpx = toPx(cz);
                p.drawEllipse(zpx, 6.8, 6.8);
            }
            for (const auto& cp : comp_poles) {
                QPointF ppx = toPx(cp);
                p.drawLine(ppx + QPointF(-6.5, -6.5), ppx + QPointF(6.5, 6.5));
                p.drawLine(ppx + QPointF(-6.5,  6.5), ppx + QPointF(6.5, -6.5));
            }
        }

        // =====================================================================
        // Angles of Departure (Complex Poles) & Arrival (Complex Zeros)
        // =====================================================================
        if (show_dep_arr_angles) {
            auto wrapDeg = [](double d) {
                double w = std::fmod(d + 180.0, 360.0);
                if (w <= 0.0) w += 360.0;
                return w - 180.0;
            };

            std::vector<DepArrInfo> dep_arr_list;

            // 1. Compute Angles of Departure for Complex Poles (|Im(p)| > 1e-4)
            for (size_t k = 0; k < poles.size(); ++k) {
                if (std::abs(poles[k].imag()) <= 1e-4) continue;
                // Deduplicate if repeated complex pole already processed
                bool seen = false;
                for (size_t prev = 0; prev < k; ++prev) {
                    if (std::abs(poles[prev] - poles[k]) < 1e-5) { seen = true; break; }
                }
                if (seen) continue;

                int mult = 0;
                double sum_p_deg = 0.0, sum_z_deg = 0.0;
                for (const auto& pj : poles) {
                    Complex d = poles[k] - pj;
                    if (std::abs(d) < 1e-5) ++mult;
                    else sum_p_deg += std::arg(d) * (180.0 / PI);
                }
                for (const auto& zi : zeros) {
                    Complex d = poles[k] - zi;
                    if (std::abs(d) >= 1e-5) sum_z_deg += std::arg(d) * (180.0 / PI);
                }
                // double theta_dep = wrapDeg((180.0 + sum_z_deg - sum_p_deg) / std::max(1, mult));
                // At the end of // 1. Compute Angles of Departure for Complex Poles:
                int q = std::max(1, mult);
                for (int m = 0; m < q; ++m) {
                    double theta_dep = wrapDeg((180.0 * (2 * m + 1) + sum_z_deg - sum_p_deg) / q);
                    dep_arr_list.push_back({poles[k], theta_dep, true});
                }
            }

            // 2. Compute Angles of Arrival for Complex Zeros (|Im(z)| > 1e-4)
            for (size_t k = 0; k < zeros.size(); ++k) {
                if (std::abs(zeros[k].imag()) <= 1e-4) continue;
                bool seen = false;
                for (size_t prev = 0; prev < k; ++prev) {
                    if (std::abs(zeros[prev] - zeros[k]) < 1e-5) { seen = true; break; }
                }
                if (seen) continue;

                int mult = 0;
                double sum_p_deg = 0.0, sum_z_deg = 0.0;
                for (const auto& pj : poles) {
                    Complex d = zeros[k] - pj;
                    if (std::abs(d) >= 1e-5) sum_p_deg += std::arg(d) * (180.0 / PI);
                }
                for (const auto& zi : zeros) {
                    Complex d = zeros[k] - zi;
                    if (std::abs(d) < 1e-5) ++mult;
                    else sum_z_deg += std::arg(d) * (180.0 / PI);
                }
                int q = std::max(1, mult);
                for (int m = 0; m < q; ++m) {
                    double theta_arr = wrapDeg((180.0 * (2 * m + 1) - sum_z_deg + sum_p_deg) / q);
                    dep_arr_list.push_back({zeros[k], theta_arr, false});
                }
            }

            // 3. Render Reference 0° Line, Angle Arc, Tangent Vector Arrow & Callout Badge
            p.setFont(QFont("Monospace", 8, QFont::Bold));
            for (const auto& item : dep_arr_list) {
                QPointF orig_px = toPx(item.pt);
                QColor col(item.is_departure ? Theme::ACCENT_POLE : Theme::ACCENT_ZERO);

                // In screen coordinates, +X is right and -Y is up:
                double rad = item.angle_deg * (PI / 180.0);
                double ux =  std::cos(rad);
                double uy = -std::sin(rad); // Flip Y for screen pixels
                double vec_len = 36.0;

                // Horizontal 0° reference line
                p.setPen(QPen(col, 1.0, Qt::DotLine));
                p.drawLine(orig_px, orig_px + QPointF(24.0, 0.0));

                // Angle arc (Qt drawArc uses 1/16th of a degree, counter-clockwise from 3 o'clock)
                QRectF arc_rect(orig_px.x() - 15.0, orig_px.y() - 15.0, 30.0, 30.0);
                p.setPen(QPen(col, 1.1));
                p.setBrush(Qt::NoBrush);
                p.drawArc(arc_rect, 0, static_cast<int>(std::round(item.angle_deg * 16.0)));

                QPointF far_px = orig_px + QPointF(ux * vec_len, uy * vec_len);
                p.setPen(QPen(col, 1.8));
                p.drawLine(orig_px, far_px);

                // Arrowhead: for Departure, tip is at far_px pointing outward;
                //            for Arrival, tip is near orig_px pointing inward toward the zero!
                QPointF tip_px = item.is_departure ? far_px : (orig_px + QPointF(ux * 8.0, uy * 8.0));
                double dir = item.is_departure ? 1.0 : -1.0;
                double adx = dir * ux, ady = dir * uy;
                double apx = -ady, apy = adx;
                double alen = 8.0, aw = 3.8;
                QPointF base_px(tip_px.x() - adx * alen, tip_px.y() - ady * alen);

                QPolygonF head;
                head << tip_px
                     << QPointF(base_px.x() + apx * aw, base_px.y() + apy * aw)
                     << QPointF(base_px.x() - apx * aw, base_px.y() - apy * aw);
                p.setPen(QPen(QColor(Theme::BG_PLOT), 0.7));
                p.setBrush(col);
                p.drawPolygon(head);

                // Callout badge showing θ_dep or θ_arr
                QString tag = QString("%1=%2°")
                    .arg(item.is_departure ? "θdep" : "θarr")
                    .arg(QString::fromStdString(PRECISION.fmt(item.angle_deg, true)));
                QFontMetrics fm(p.font());
                int tw = fm.horizontalAdvance(tag) + 10;
                double bx = far_px.x() + (ux >= 0.0 ? 4.0 : -tw - 4.0);
                double by = far_px.y() + (uy >= 0.0 ? 2.0 : -18.0);
                QRectF box(bx, by, tw, 17);

                p.setBrush(badge_bg);
                p.setPen(QPen(col, 1.0));
                p.drawRoundedRect(box, 3, 3);
                p.setPen(col);
                p.drawText(box, Qt::AlignCenter, tag);
            }
        }

        // Test Point (Green when ON RL, Orange when OFF RL)
        if (show_test_point) {
            QColor tp_col(is_on_rl ? Theme::ACCENT_PID : Theme::ACCENT_OPEN);
            if (is_on_rl) {
                p.setPen(QPen(tp_col, 1.5));
                p.setBrush(Qt::NoBrush);
                p.drawEllipse(tp_px, 9.0, 9.0);
            }
            p.setPen(QPen(Qt::white, 1.5));
            p.setBrush(tp_col);
            p.drawEllipse(tp_px, 5.5, 5.5);

            p.setPen(tp_col);
            p.setFont(QFont("Monospace", 9, QFont::Bold));
            QString tag = is_z_plane
                ? QString("z (%1) |z|=%2 [%3]")
                    .arg(QString::fromStdString(PRECISION.fmtComplex(test_point)))
                    .arg(QString::fromStdString(PRECISION.fmt(std::abs(test_point))))
                    .arg(is_on_rl ? "ON RL" : "OFF RL")
                : QString("s (%1) [%2]")
                    .arg(QString::fromStdString(PRECISION.fmtComplex(test_point)))
                    .arg(is_on_rl ? "ON RL" : "OFF RL");
            p.drawText(tp_px + QPointF(10, -8), tag);
        }  
    }

    void mousePressEvent(QMouseEvent* e) override {
        if (e->button() == Qt::LeftButton && show_test_point) {
            user_bounds_set = true;
            Complex clicked_pt = pxToComplex(e->pos());
            // Snap to nearest spec intersection if clicked within 12 pixels
            for (const auto& ix : current_intersections) {
                QPointF ipx = toPx(ix.pt);
                if (std::hypot(e->pos().x() - ipx.x(), e->pos().y() - ipx.y()) <= 12.0) {
                    clicked_pt = ix.pt;
                    break;
                }
            }
            for (const auto& cp : critical_points) {
                QPointF cpx = toPx(cp.pt);
                if (std::hypot(e->pos().x() - cpx.x(), e->pos().y() - cpx.y()) <= 12.0) {
                    clicked_pt = cp.pt;
                    break;
                }
            }

            test_point = clicked_pt;
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

