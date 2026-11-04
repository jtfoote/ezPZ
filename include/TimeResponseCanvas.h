#pragma once
#include "ComplexPlaneCanvas.h"


// =============================================================================
// Time Response Canvas (Fixed Stem Polygon Fill Bug + Pole-Zero Inset)
// =============================================================================
class TimeResponseCanvas : public QWidget {
    Q_OBJECT
public:
    bool is_discrete = false;
    bool show_cl = true;
    bool show_ol = false;
    bool show_bounds = true;
    bool show_u = false;
    bool show_stems = true;
    bool show_inset = true;

    std::optional<SimulationResult> sim_res;
    std::optional<DiscreteSimulationResult> dsim_res;
    double settle_tol_pct = 2.0;

    std::function<void(double)> onCursorChanged;

    explicit TimeResponseCanvas(bool discrete_mode = false, QWidget* parent = nullptr)
        : QWidget(parent), is_discrete(discrete_mode)
    {
        setMinimumSize(460, 360);
        setCursor(Qt::CrossCursor);
    }

    void resetView() {
        user_bounds_set = false;
        update();
    }

protected:
    bool user_bounds_set = false;
    double x_min = 0.0, x_max = 10.0;
    double y_min = -0.2, y_max = 1.5;
    bool panning = false;
    QPoint pan_start_px;

    void computeAutoBounds() {
        if (!is_discrete && sim_res.has_value()) {
            const auto& r = sim_res.value();
            x_min = 0.0;
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
        } else if (is_discrete && dsim_res.has_value()) {
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
    }

    void drawPoleZeroInset(QPainter& p, const QRectF& plot_rect) {
        double iw = std::clamp(plot_rect.width() * 0.27, 130.0, 210.0);
        double ih = std::clamp(plot_rect.height() * 0.31, 110.0, 175.0);
        QRectF ir(plot_rect.right() - iw - 12.0, plot_rect.top() + 12.0, iw, ih);

        p.save();
        p.setClipping(false);
        QColor inset_bg(Theme::BG_PANEL);
        inset_bg.setAlpha(235);
        p.setBrush(inset_bg);
        p.setPen(QPen(QColor(Theme::BORDER_PANEL), 1.2));
        p.drawRoundedRect(ir, 6, 6);
        p.setBrush(Qt::NoBrush);

        p.setPen(QColor(Theme::TEXT_SECONDARY));
        p.setFont(QFont("Sans", 8, QFont::Bold));
        QString title = is_discrete ? "z-Plane & |z|=1" : "Closed-Loop s-Plane";
        p.drawText(QRectF(ir.left(), ir.top() + 3, ir.width(), 14), Qt::AlignCenter, title);

        QRectF inner = ir.adjusted(8, 18, -8, -8);
        p.setClipRect(inner);

        std::vector<Complex> ol_p, ol_z, cl_p;
        if (!is_discrete && sim_res.has_value()) {
            ol_p = sim_res->ol_poles;
            ol_z = sim_res->ol_zeros;
            for (const auto& cp : sim_res->cl_poles) {
                if (std::abs(cp.real()) < 45.0) cl_p.push_back(cp);
            }
        } else if (is_discrete && dsim_res.has_value()) {
            ol_p = dsim_res->ol_z_poles;
            ol_z = dsim_res->ol_z_zeros;
            if (show_cl) cl_p = dsim_res->cl_z_poles;
        }

        double r0 = -1.35, r1 = 1.35, i0 = -1.35, i1 = 1.35;
        if (!is_discrete) {
            std::vector<Complex> pts = ol_p;
            pts.insert(pts.end(), ol_z.begin(), ol_z.end());
            pts.insert(pts.end(), cl_p.begin(), cl_p.end());
            pts.push_back({0.0, 0.0});
            r0 = 0.0; r1 = 0.0; i0 = 0.0; i1 = 0.0;
            for (const auto& pt : pts) {
                r0 = std::min(r0, pt.real()); r1 = std::max(r1, pt.real());
                i0 = std::min(i0, pt.imag()); i1 = std::max(i1, pt.imag());
            }
            double sp = std::max({r1 - r0, i1 - i0, 2.0}) * 0.68;
            double mr = 0.5 * (r0 + r1), mi = 0.5 * (i0 + i1);
            r0 = mr - sp; r1 = mr + sp;
            i0 = mi - sp; i1 = mi + sp;
        }

        // Enforce 1:1 aspect ratio inside the Pole-Zero inset box
        double in_w = std::max(1.0, inner.width());
        double in_h = std::max(1.0, inner.height());
        double in_upp = std::max((r1 - r0) / in_w, (i1 - i0) / in_h);
        double mr_in = 0.5 * (r0 + r1), mi_in = 0.5 * (i0 + i1);
        r0 = mr_in - 0.5 * in_upp * in_w; r1 = mr_in + 0.5 * in_upp * in_w;
        i0 = mi_in - 0.5 * in_upp * in_h; i1 = mi_in + 0.5 * in_upp * in_h;

        auto toInsetPx = [&](Complex c) {
            double x = inner.left() + (c.real() - r0) / std::max(r1 - r0, 1e-9) * inner.width();
            double y = inner.top()  + (i1 - c.imag()) / std::max(i1 - i0, 1e-9) * inner.height();
            return QPointF(x, y);
        };

        // Origin Axes
        QPointF orig = toInsetPx({0.0, 0.0});
        p.setPen(QPen(QColor(Theme::AXIS_LINE), 1.0));
        p.drawLine(QPointF(inner.left(), orig.y()), QPointF(inner.right(), orig.y()));
        p.drawLine(QPointF(orig.x(), inner.top()), QPointF(orig.x(), inner.bottom()));

        // Unit Circle for Discrete Inset
        if (is_discrete) {
            QPointF tl = toInsetPx({-1.0, 1.0});
            QPointF br = toInsetPx({1.0, -1.0});
            p.setPen(QPen(QColor(Theme::ACCENT_ASYMP), 1.1, Qt::DashLine));
            p.setBrush(Qt::NoBrush);
            p.drawEllipse(QRectF(tl, br));
        }

        // Open-Loop Zeros (o)
        p.setPen(QPen(QColor(Theme::ACCENT_ZERO), 1.4));
        p.setBrush(QColor("#0d111a"));
        for (const auto& z : ol_z) p.drawEllipse(toInsetPx(z), 4.0, 4.0);

        // Open-Loop Poles (x)
        p.setPen(QPen(QColor(Theme::ACCENT_POLE), 1.4));
        for (const auto& pl : ol_p) {
            QPointF px = toInsetPx(pl);
            p.drawLine(px + QPointF(-3.8, -3.8), px + QPointF(3.8, 3.8));
            p.drawLine(px + QPointF(-3.8, 3.8),  px + QPointF(3.8, -3.8));
        }

        // Closed-Loop Poles (purple x)
        p.setPen(QPen(QColor(Theme::ACCENT_LOCUS), 1.8));
        for (const auto& pl : cl_p) {
            QPointF px = toInsetPx(pl);
            p.drawLine(px + QPointF(-4.5, -4.5), px + QPointF(4.5, 4.5));
            p.drawLine(px + QPointF(-4.5, 4.5),  px + QPointF(4.5, -4.5));
        }

        p.restore();
    }

    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.fillRect(rect(), QColor(Theme::BG_PLOT));

        if (!is_discrete && !sim_res.has_value()) return;
        if (is_discrete && !dsim_res.has_value()) return;

        if (!user_bounds_set) computeAutoBounds();

        double pad_l = 52.0, pad_r = 22.0, pad_t = 22.0, pad_b = 36.0;
        double ew = std::max(10.0, width() - pad_l - pad_r);
        double eh = std::max(10.0, height() - pad_t - pad_b);
        QRectF plot_rect(pad_l, pad_t, ew, eh);

        auto toPx = [&](double x, double y) {
            double px = pad_l + (x - x_min) / std::max(x_max - x_min, 1e-9) * ew;
            double py = pad_t + (y_max - y) / std::max(y_max - y_min, 1e-9) * eh;
            return QPointF(px, py);
        };

        // Gridlines & Tick Labels (unclipped so labels draw in margins)
        p.setPen(QPen(QColor(Theme::GRID_MAJOR), 1.0));
        for (int i = 0; i <= 6; ++i) {
            double gx = pad_l + ew * double(i) / 6.0;
            p.drawLine(QPointF(gx, pad_t), QPointF(gx, pad_t + eh));
            double xv = x_min + (x_max - x_min) * double(i) / 6.0;
            p.setPen(QColor(Theme::TEXT_SECONDARY));
            p.setFont(QFont("Monospace", 8));
            p.drawText(QRectF(gx - 28, pad_t + eh + 6, 56, 20), Qt::AlignCenter, QString::number(xv, 'g', 3));
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

        // Clip waveforms to plot_rect so zoomed/panned curves don't spill over axes
        p.save();
        p.setClipRect(plot_rect);

        double y0_px = toPx(0.0, 0.0).y();
        p.setPen(QPen(QColor(Theme::AXIS_LINE), 1.3));
        p.drawLine(QPointF(pad_l, y0_px), QPointF(pad_l + ew, y0_px));

        if (!is_discrete) {
            const auto& r = sim_res.value();
            p.setBrush(Qt::NoBrush);
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
                p.setBrush(Qt::NoBrush);
                p.setPen(QPen(QColor(Theme::ACCENT_OPEN), 1.8));
                p.drawPath(ol_p);
            }
            if (show_cl) {
                QPainterPath cl_p;
                for (size_t i = 0; i < r.t.size(); ++i) {
                    QPointF pt = toPx(r.t[i], r.y_closed[i].real());
                    if (i == 0) cl_p.moveTo(pt); else cl_p.lineTo(pt);
                }
                p.setBrush(Qt::NoBrush);
                p.setPen(QPen(QColor(Theme::ACCENT_LOCUS), 2.3));
                p.drawPath(cl_p);
            }
            if (show_u) {
                QPainterPath u_p;
                for (size_t i = 0; i < r.t.size(); ++i) {
                    QPointF pt = toPx(r.t[i], r.u_control[i].real());
                    if (i == 0) u_p.moveTo(pt); else u_p.lineTo(pt);
                }
                p.setBrush(Qt::NoBrush);
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
            p.setBrush(Qt::NoBrush);
            if (show_bounds) {
                QPainterPath ref_p;
                for (size_t i = 0; i < r.k_idx.size(); ++i) {
                    QPointF pt = toPx(r.k_idx[i], r.r_seq[i]);
                    if (i == 0) ref_p.moveTo(pt); else ref_p.lineTo(pt);
                }
                p.setPen(QPen(QColor(Theme::ACCENT_ZERO), 1.2, Qt::DashLine));
                p.drawPath(ref_p);

                if (r.stability_state == "STABLE") {
                    double yss = r.y_steady.real();
                    double band = std::abs(yss) * (settle_tol_pct / 100.0);
                    p.setPen(QPen(QColor(Theme::ACCENT_PID), 0.9, Qt::DotLine));
                    p.drawLine(toPx(x_min, yss + band), toPx(x_max, yss + band));
                    p.drawLine(toPx(x_min, yss - band), toPx(x_max, yss - band));
                }
            }
            const auto& seq = r.y_primary;
            QColor seq_col(show_cl ? Theme::ACCENT_LOCUS : Theme::ACCENT_OPEN);
            QPainterPath path;
            for (size_t i = 0; i < r.k_idx.size(); ++i) {
                QPointF pt = toPx(r.k_idx[i], seq[i].real());
                if (i == 0) path.moveTo(pt); else path.lineTo(pt);
                if (show_stems) {
                    QColor stem_line_col = seq_col;
                    stem_line_col.setAlpha(110);
                    p.setPen(QPen(stem_line_col, 1.1));
                    p.drawLine(toPx(r.k_idx[i], 0.0), pt);
                    p.setPen(Qt::NoPen);
                    p.setBrush(seq_col);
                    p.drawEllipse(pt, 3.2, 3.2);
                }
            }
            p.setBrush(Qt::NoBrush);
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
        p.restore();

        // Draw Pole-Zero Inset in Top-Right Corner
        if (show_inset) {
            drawPoleZeroInset(p, plot_rect);
        }
    }

    void mousePressEvent(QMouseEvent* e) override {
        if (e->button() == Qt::LeftButton) {
            updateCursorFromPx(e->pos().x());
        } else if (e->button() == Qt::RightButton || e->button() == Qt::MiddleButton) {
            if (!user_bounds_set) computeAutoBounds();
            user_bounds_set = true;
            panning = true;
            pan_start_px = e->pos();
            setCursor(Qt::ClosedHandCursor);
        }
    }

    void mouseMoveEvent(QMouseEvent* e) override {
        if (e->buttons() & Qt::LeftButton) {
            updateCursorFromPx(e->pos().x());
        } else if (panning) {
            double pad_l = 52.0, pad_r = 22.0, pad_t = 22.0, pad_b = 36.0;
            double ew = std::max(10.0, width() - pad_l - pad_r);
            double eh = std::max(10.0, height() - pad_t - pad_b);
            double dx = -double(e->pos().x() - pan_start_px.x()) / ew * (x_max - x_min);
            double dy =  double(e->pos().y() - pan_start_px.y()) / eh * (y_max - y_min);
            x_min += dx; x_max += dx;
            y_min += dy; y_max += dy;
            pan_start_px = e->pos();
            update();
        }
    }

    void mouseReleaseEvent(QMouseEvent*) override {
        panning = false;
        setCursor(Qt::CrossCursor);
    }

    void wheelEvent(QWheelEvent* e) override {
        if (!user_bounds_set) computeAutoBounds();
        user_bounds_set = true;

        double pad_l = 52.0, pad_r = 22.0, pad_t = 22.0, pad_b = 36.0;
        double ew = std::max(10.0, width() - pad_l - pad_r);
        double eh = std::max(10.0, height() - pad_t - pad_b);

        double factor = (e->angleDelta().y() > 0) ? 0.85 : 1.18;
        double fx = std::clamp((e->position().x() - pad_l) / ew, 0.0, 1.0);
        double fy = std::clamp((e->position().y() - pad_t) / eh, 0.0, 1.0);

        double anchor_x = x_min + fx * (x_max - x_min);
        double anchor_y = y_max - fy * (y_max - y_min);

        x_min = anchor_x - (anchor_x - x_min) * factor;
        x_max = anchor_x + (x_max - anchor_x) * factor;
        y_min = anchor_y - (anchor_y - y_min) * factor;
        y_max = anchor_y + (y_max - anchor_y) * factor;
        update();
    }

private:
    void updateCursorFromPx(double px) {
        if (!user_bounds_set) computeAutoBounds();
        double pad_l = 52.0, pad_r = 22.0;
        double ew = std::max(10.0, width() - pad_l - pad_r);
        double frac = std::clamp((px - pad_l) / ew, 0.0, 1.0);
        double x_val = x_min + frac * (x_max - x_min);
        if (!is_discrete && sim_res.has_value() && !sim_res->t.empty()) {
            double clamped_t = std::clamp(x_val, 0.0, sim_res->t.back());
            if (onCursorChanged) onCursorChanged(clamped_t);
        } else if (is_discrete && dsim_res.has_value() && !dsim_res->k_idx.empty()) {
            double clamped_k = std::clamp(std::round(x_val), 0.0, double(dsim_res->k_idx.back()));
            if (onCursorChanged) onCursorChanged(clamped_k);
        }
    }
};

