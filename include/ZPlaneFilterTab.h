#pragma once
#include "PIDTimeResponseTab.h"


// =============================================================================
// Tab 3: z-Plane & Digital Filter Root Locus View (Full Python Parity)
// =============================================================================
class ZPlaneFilterTab : public QWidget {
    Q_OBJECT
public:
    SharedSystemModel* model;
    std::function<void(double)> onSendToDiscreteSim;

    QLineEdit *num_edit, *den_edit, *cutoff_edit, *kd_edit, *ts_edit, *tp_v1_edit, *tp_v2_edit, *tol_edit;
    QLabel *tp_lbl1, *tp_lbl2, *poly_status_lbl;
    MathFormulaWidget* tf_math_w;
    PoleZeroEditorCard *poles_card, *zeros_card;
    QComboBox *preset_combo;
    QRadioButton *cart_radio, *polar_radio;
    bool is_polar_mode = false;

    QFrame* rl_banner;
    QLabel *banner_title, *banner_right, *banner_sub;
    QLabel *val_angle, *val_mag, *val_gain, *val_zeta;
    ComplexPlaneCanvas* canvas;
    bool updating_ui = false;

    FeedbackConfigBar* fb_bar;

    ZPlaneFilterTab(SharedSystemModel* m, std::function<void(double)> send_cb, QWidget* parent = nullptr)
        : QWidget(parent), model(m), onSendToDiscreteSim(std::move(send_cb))
    {
        auto* root_layout = new QHBoxLayout(this);
        root_layout->setContentsMargins(0, 0, 0, 0);
        auto* splitter = new QSplitter(Qt::Horizontal, this);
        root_layout->addWidget(splitter);

        auto* left_w = new QWidget();
        auto* left_v = new QVBoxLayout(left_w);
        left_v->setContentsMargins(0, 0, 6, 0);
        left_v->setSpacing(6);

        // 1. Num(z) / Den(z) Solver Card
        auto* poly_card = makeCard();
        auto* poly_l = new QVBoxLayout(poly_card);
        poly_l->setContentsMargins(10, 6, 10, 6);
        auto* poly_hdr = new QHBoxLayout();
        auto* t1 = new QLabel("Digital Filter  H(z) = Kd · Num(z) / Den(z)");
        t1->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::ACCENT_ASYMP));
        auto* solve_btn = makeActionBtn("Solve z-Zeros & z-Poles ➔", Theme::ACCENT_PID);
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

        poly_status_lbl = new QLabel("Enter z-polynomials (e.g. z^2-0.94z+0.33) and press Enter / Solve");
        poly_status_lbl->setStyleSheet("color: #94a3b8; font-size: 10px; border:none;");
        poly_l->addWidget(poly_status_lbl);

        fb_bar = new FeedbackConfigBar("z", "H_fb");
        poly_l->addWidget(fb_bar);
        fb_bar->onChanged = [this]() { fb_bar->syncToModelZ(model); evaluateCurrent(true); };

        tf_math_w = new MathFormulaWidget("", QColor(Theme::ACCENT_ASYMP), QColor(Theme::BG_INPUT), 10);
        poly_l->addWidget(tf_math_w);
        left_v->addWidget(poly_card);

        // 2. Editable z-Poles (×) & z-Zeros (○) Tables Row
        auto* pz_row = new QHBoxLayout();
        poles_card = new PoleZeroEditorCard("z-Poles (×)", Theme::ACCENT_POLE, 0.6);
        zeros_card = new PoleZeroEditorCard("z-Zeros (○)", Theme::ACCENT_ZERO, -1.0);
        pz_row->addWidget(poles_card, 1);
        pz_row->addWidget(zeros_card, 1);
        left_v->addLayout(pz_row);

        // 3. Digital Filter Synthesizer & Test Point z Card
        auto* synth_card = makeCard();
        auto* synth_l = new QVBoxLayout(synth_card);
        synth_l->setContentsMargins(10, 6, 10, 6);
        auto* shdr = new QHBoxLayout();
        auto* st = new QLabel("Filter Calculator:");
        st->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::ACCENT_ASYMP));
        preset_combo = new QComboBox();
        preset_combo->addItems({
            "Butterworth LPF (2nd)", "Butterworth HPF (2nd)", "Biquad Band-Pass",
            "Biquad Notch", "Moving Avg FIR (4-Tap)", "Tustin from s-Plane G(s)", "Matched z=e^(sTs) from G(s)"
        });
        preset_combo->setStyleSheet(QString("background-color: %1; color: %2; padding: 3px; font-weight: bold; font-size: 11px;").arg(Theme::BG_INPUT, Theme::TEXT_PRIMARY));
        auto* synth_btn = makeActionBtn("Synthesize H(z)", Theme::ACCENT_PID);
        shdr->addWidget(st); shdr->addWidget(preset_combo, 1); shdr->addWidget(synth_btn);
        synth_l->addLayout(shdr);

        auto* sbot = new QHBoxLayout();
        cutoff_edit = makeDarkEntry("0.25", 50);
        kd_edit     = makeDarkEntry(QString::number(model->filter_gain), 60);
        ts_edit     = makeDarkEntry(QString::number(model->ts), 50);
        auto* snap_uc_btn = makeActionBtn("z = e^(jΩc)", Theme::ACCENT_ZERO);
        auto addInline = [&](const QString& lbl, QWidget* w) {
            auto* l = new QLabel(lbl); l->setStyleSheet("color: #94a3b8; font-size: 10px; font-weight: bold; border:none;");
            sbot->addWidget(l); sbot->addWidget(w);
        };
        addInline("Ωc/π:", cutoff_edit);
        addInline("Gain Kd:", kd_edit);
        addInline("Ts (s):", ts_edit);
        sbot->addWidget(snap_uc_btn);
        synth_l->addLayout(sbot);

        // Test Point z + Cartesian / Polar Switcher
        auto* tp_hdr = new QHBoxLayout();
        auto* tp_t = new QLabel("Test Point  z  &  Angle Tolerance");
        tp_t->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::ACCENT_PID));
        cart_radio  = new QRadioButton("Cartesian"); cart_radio->setChecked(true);
        polar_radio = new QRadioButton("Polar");
        for (auto* rb : {cart_radio, polar_radio}) {
            rb->setStyleSheet(QString("QRadioButton { color: %1; font-weight: bold; font-size: 11px; border: none; }").arg(Theme::TEXT_PRIMARY));
        }
        tp_hdr->addWidget(tp_t); tp_hdr->addStretch(); tp_hdr->addWidget(cart_radio); tp_hdr->addWidget(polar_radio);
        synth_l->addLayout(tp_hdr);

        auto* tp_grid = new QGridLayout();
        tp_lbl1 = new QLabel("Real Re(z)"); tp_lbl1->setStyleSheet("color: #94a3b8; font-size: 10px; font-weight: bold; border:none;");
        tp_lbl2 = new QLabel("Imag Im(z)"); tp_lbl2->setStyleSheet("color: #94a3b8; font-size: 10px; font-weight: bold; border:none;");
        auto* l_tl = new QLabel("Tol (±°)"); l_tl->setStyleSheet("color: #94a3b8; font-size: 10px; font-weight: bold; border:none;");
        tp_v1_edit = makeDarkEntry(QString::number(model->z_test_point.real()));
        tp_v2_edit = makeDarkEntry(QString::number(model->z_test_point.imag()));
        tol_edit   = makeDarkEntry(QString::number(model->z_rl_tol_deg), 55);
        tp_grid->addWidget(tp_lbl1, 0, 0); tp_grid->addWidget(tp_v1_edit, 1, 0);
        tp_grid->addWidget(tp_lbl2, 0, 1); tp_grid->addWidget(tp_v2_edit, 1, 1);
        tp_grid->addWidget(l_tl,    0, 2); tp_grid->addWidget(tol_edit,   1, 2);
        synth_l->addLayout(tp_grid);
        left_v->addWidget(synth_card);

        // 4. z-Domain Evaluation & Difference Equation Banner Card
        auto* res_card = makeCard();
        auto* res_l = new QVBoxLayout(res_card);
        res_l->setContentsMargins(10, 6, 10, 6);
        auto* res_hdr = new QHBoxLayout();
        auto* rt_lbl = new QLabel("z-Domain & Filter Evaluation");
        rt_lbl->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::TEXT_PRIMARY));
        auto* send_btn = makeActionBtn("Send H(z) → Discrete Sim ➔", Theme::ACCENT_PID);
        res_hdr->addWidget(rt_lbl); res_hdr->addStretch(); res_hdr->addWidget(send_btn);
        res_l->addLayout(res_hdr);

        rl_banner = new QFrame();
        rl_banner->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        auto* bl = new QVBoxLayout(rl_banner);
        bl->setContentsMargins(10, 5, 10, 5);
        bl->setSpacing(2);
        auto* btop = new QHBoxLayout();
        banner_title = new QLabel("✔ BIBO STABLE  •  ON z-LOCUS");
        banner_right = new QLabel("max|z_p| = 0.7500");
        btop->addWidget(banner_title); btop->addStretch(); btop->addWidget(banner_right);
        banner_sub = new QLabel("y[k] = ...");
        banner_sub->setWordWrap(true);
        bl->addLayout(btop);
        bl->addWidget(banner_sub);
        res_l->addWidget(rl_banner);

        auto* mgrid = new QGridLayout();
        auto makeM = [&](int r, int c, const QString& t, QLabel*& out) {
            auto* b = makeCard();
            b->setStyleSheet(QString("background-color: %1; border: 1px solid %2; padding: 4px;").arg(Theme::BG_INPUT, Theme::BORDER_INPUT));
            auto* vl = new QVBoxLayout(b);
            vl->setContentsMargins(6, 3, 6, 3);
            auto* tl = new QLabel(t); tl->setStyleSheet("color: #94a3b8; font-size: 9px; font-weight: bold; border:none;");
            out = new QLabel("—"); out->setStyleSheet(QString("color: %1; font-family: Monospace; font-size: 11px; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));
            vl->addWidget(tl); vl->addWidget(out);
            mgrid->addWidget(b, r, c);
        };
        makeM(0, 0, "NET ANGLE ∠H(z)", val_angle);
        makeM(0, 1, "MAG |H(z)| & dB", val_mag);
        makeM(1, 0, "RL GAIN K = 1/|H(z)|", val_gain);
        makeM(1, 1, "EQUIV DAMPING ζ / ωn", val_zeta);
        res_l->addLayout(mgrid);
        left_v->addWidget(res_card);
        left_v->addStretch();

        // Right z-Plane Canvas
        auto* right_card = makeCard();
        auto* right_l = new QVBoxLayout(right_card);
        auto* phdr = new QHBoxLayout();
        auto* pt = new QLabel("Complex z-Plane & Unit Circle Filter Diagram");
        pt->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 13px; border:none;").arg(Theme::TEXT_PRIMARY));
        auto* chk_uc = new QCheckBox("Unit Circle |z|=1"); chk_uc->setChecked(true); chk_uc->setStyleSheet("color: #22d3ee; font-weight: bold;");
        auto* chk_zg = new QCheckBox("ζ/ωn Grid"); chk_zg->setChecked(true); chk_zg->setStyleSheet("color: #34d399; font-weight: bold;");
        auto* chk_rl = new QCheckBox("Root Locus"); chk_rl->setChecked(true); chk_rl->setStyleSheet("color: #a855f7; font-weight: bold;");
        auto* chk_dep = new QCheckBox("Dep/Arr ∠"); chk_dep->setChecked(true); chk_dep->setStyleSheet("color: #f472b6; font-weight: bold;");
        auto* fit_b  = makeActionBtn("Fit View", Theme::TEXT_PRIMARY, Theme::BG_PANEL);
        phdr->addWidget(pt); phdr->addStretch();
        phdr->addWidget(chk_uc); phdr->addWidget(chk_zg); phdr->addWidget(chk_rl); phdr->addWidget(fit_b);
        right_l->addLayout(phdr);

        canvas = new ComplexPlaneCanvas(true);
        attachSvgExportMenu(canvas, "z_plane_root_locus.svg");
        right_l->addWidget(canvas, 1);

        splitter->addWidget(wrapInLeftScroll(left_w, 380));
        splitter->addWidget(right_card);
        splitter->setStretchFactor(0, 0);
        splitter->setStretchFactor(1, 1);

        // Connections
        connect(solve_btn, &QPushButton::clicked, this, &ZPlaneFilterTab::applyPolyNumDen);
        connect(num_edit, &QLineEdit::returnPressed, this, &ZPlaneFilterTab::applyPolyNumDen);
        connect(den_edit, &QLineEdit::returnPressed, this, &ZPlaneFilterTab::applyPolyNumDen);

        poles_card->onValuesChanged = [this]() {
            if (updating_ui) return;
            model->z_poles = poles_card->getValues();
            den_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->z_poles, "z")));
            evaluateCurrent(true);
        };
        zeros_card->onValuesChanged = [this]() {
            if (updating_ui) return;
            model->z_zeros = zeros_card->getValues();
            num_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->z_zeros, "z")));
            evaluateCurrent(true);
        };

        connect(synth_btn, &QPushButton::clicked, this, [this]() {
            static const std::vector<std::string> keys = {
                "butter_lp", "butter_hp", "biquad_bp", "biquad_notch", "fir_ma4", "tustin_from_s", "matched_from_s"
            };
            int idx = std::clamp(preset_combo->currentIndex(), 0, static_cast<int>(keys.size()) - 1);
            double wc = parseNumericInput(cutoff_edit->text().toStdString(), 0.25);
            double ts = parseNumericInput(ts_edit->text().toStdString(), 0.05);
            std::tie(model->z_poles, model->z_zeros, model->filter_gain) =
                DiscreteFilterSimulator::designFilterPreset(keys[idx], wc, ts, model->poles, model->zeros, model->plant_gain);
            model->ts = ts;
            if (!model->z_poles.empty()) model->z_test_point = model->z_poles[0];
            loadFromModel();
            model->notify("z_plane");
        });

        connect(snap_uc_btn, &QPushButton::clicked, this, [this]() {
            double wc = parseNumericInput(cutoff_edit->text().toStdString(), 0.25);
            Complex z_uc = std::polar(1.0, PI * wc);
            updating_ui = true;
            if (!is_polar_mode) {
                tp_v1_edit->setText(QString::number(z_uc.real(), 'g', 4));
                tp_v2_edit->setText(QString::number(z_uc.imag(), 'g', 4));
            } else {
                tp_v1_edit->setText("1");
                tp_v2_edit->setText(QString::number(180.0 * wc, 'g', 4));
            }
            updating_ui = false;
            evaluateCurrent(true);
        });

        connect(cart_radio, &QRadioButton::toggled, this, [this](bool checked) {
            if (!updating_ui) onCoordModeSwitch(!checked);
        });

        for (auto* ed : {kd_edit, ts_edit, tp_v1_edit, tp_v2_edit, tol_edit}) {
            connect(ed, &QLineEdit::textEdited, this, [this]() { if (!updating_ui) evaluateCurrent(true); });
        }
        connect(chk_uc, &QCheckBox::toggled, this, [this](bool v) { canvas->show_unit_circle = v; canvas->update(); });
        connect(chk_zg, &QCheckBox::toggled, this, [this](bool v) { canvas->show_zgrid = v; canvas->update(); });
        connect(chk_rl, &QCheckBox::toggled, this, [this](bool v) { canvas->show_locus = v; canvas->update(); });
        connect(chk_dep, &QCheckBox::toggled, this, [this](bool v) { canvas->show_dep_arr_angles = v; canvas->update(); });
        connect(fit_b,  &QPushButton::clicked, this, [this]() { canvas->resetView(); });
        connect(send_btn, &QPushButton::clicked, this, [this]() {
            if (onSendToDiscreteSim) onSendToDiscreteSim(model->filter_gain);
        });

        canvas->onPointSelected = [this](Complex zt) {
            updating_ui = true;
            if (!is_polar_mode) {
                tp_v1_edit->setText(QString::number(zt.real(), 'g', 4));
                tp_v2_edit->setText(QString::number(zt.imag(), 'g', 4));
            } else {
                tp_v1_edit->setText(QString::number(std::abs(zt), 'g', 4));
                tp_v2_edit->setText(QString::number(std::arg(zt) * 180.0 / PI, 'g', 4));
            }
            updating_ui = false;
            evaluateCurrent(false);
        };

        loadFromModel();
    }

    void onCoordModeSwitch(bool to_polar) {
        if (to_polar == is_polar_mode) return;
        double v1 = parseNumericInput(tp_v1_edit->text().toStdString(), 0.0);
        double v2 = parseNumericInput(tp_v2_edit->text().toStdString(), 0.0);
        updating_ui = true;
        if (to_polar) {
            Complex z(v1, v2);
            tp_lbl1->setText("Magnitude |z|");
            tp_lbl2->setText("Angle (deg)");
            tp_v1_edit->setText(QString::number(std::abs(z), 'g', 5));
            tp_v2_edit->setText(QString::number(std::arg(z) * 180.0 / PI, 'g', 5));
        } else {
            Complex z = std::polar(v1, v2 * PI / 180.0);
            tp_lbl1->setText("Real Re(z)");
            tp_lbl2->setText("Imag Im(z)");
            tp_v1_edit->setText(QString::number(z.real(), 'g', 5));
            tp_v2_edit->setText(QString::number(z.imag(), 'g', 5));
        }
        is_polar_mode = to_polar;
        updating_ui = false;
        evaluateCurrent(true);
    }

    void applyPolyNumDen() {
        try {
            auto pn = parsePolynomialInput(num_edit->text().toStdString());
            auto pd = parsePolynomialInput(den_edit->text().toStdString());
            if (std::abs(pd.leading_gain) <= 1e-14)
                throw std::runtime_error("Divide by zero: Den(s) cannot be 0.");
            if (std::abs(pn.leading_gain) <= 1e-14)
                throw std::runtime_error("Zero numerator: Num(s) cannot be 0.");
            model->z_zeros = pn.roots;
            model->z_poles = pd.roots;

            if (model->z_zeros.size() > model->z_poles.size()) {
                poly_status_lbl->setStyleSheet("color: #f59e0b; font-size: 10px; font-weight: bold; border:none;");
                poly_status_lbl->setText(QString("⚠ Improper / Non-Causal H(z): deg(Num)=%1 > deg(Den)=%2")
                    .arg(model->z_zeros.size()).arg(model->z_poles.size()));
            } else {
                poly_status_lbl->setStyleSheet("color: #94a3b8; font-size: 10px; border:none;");
                poly_status_lbl->setText(QString("✔ Solved %1 z-Poles & %2 z-Zeros")
                    .arg(model->z_poles.size()).arg(model->z_zeros.size()));
            }

            poles_card->setValues(model->z_poles);
            zeros_card->setValues(model->z_zeros);
            poly_status_lbl->setText(QString("✔ Solved %1 z-Poles & %2 z-Zeros").arg(model->z_poles.size()).arg(model->z_zeros.size()));
            evaluateCurrent(true);
        } catch (const std::exception& e) {
            poly_status_lbl->setText(QString("⚠ Polynomial error: %1").arg(e.what()));
        }
    }

    void loadFromModel() {
        updating_ui = true;
        poles_card->setValues(model->z_poles);
        zeros_card->setValues(model->z_zeros);
        num_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->z_zeros, "z")));
        den_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->z_poles, "z")));
        kd_edit->setText(QString::number(model->filter_gain, 'g', 5));
        ts_edit->setText(QString::number(model->ts, 'g', 4));
        if (!is_polar_mode) {
            tp_v1_edit->setText(QString::number(model->z_test_point.real(), 'g', 4));
            tp_v2_edit->setText(QString::number(model->z_test_point.imag(), 'g', 4));
        } else {
            tp_v1_edit->setText(QString::number(std::abs(model->z_test_point), 'g', 4));
            tp_v2_edit->setText(QString::number(std::arg(model->z_test_point) * 180.0 / PI, 'g', 4));
        }
        tol_edit->setText(QString::number(model->z_rl_tol_deg, 'g', 3));
        updating_ui = false;
        evaluateCurrent(false);
        fb_bar->loadFromModelZ(model);
    }

    void evaluateCurrent(bool notify) {
        model->filter_gain = parseNumericInput(kd_edit->text().toStdString(), 1.0);
        model->ts = std::max(1e-5, parseNumericInput(ts_edit->text().toStdString(), 0.05));
        double v1 = parseNumericInput(tp_v1_edit->text().toStdString(), 0.68);
        double v2 = parseNumericInput(tp_v2_edit->text().toStdString(), 0.32);
        Complex zt = is_polar_mode ? std::polar(v1, v2 * PI / 180.0) : Complex(v1, v2);
        double tol = std::abs(parseNumericInput(tol_edit->text().toStdString(), 0.5));
        model->z_test_point = zt;
        model->z_rl_tol_deg = tol;

        // Combine H(z) and H_fb(z) poles/zeros for the discrete open-loop return ratio
        auto loop_z_poles = model->effectiveLoopPolesZ();
        auto loop_z_zeros = model->effectiveLoopZerosZ();
        double loop_z_gain = model->filter_gain * (model->unity_feedback_z ? 1.0 : model->hz_fb_gain);

        tf_math_w->setLatex(QString::fromStdString(fb_bar->formatActiveLatexZ(model)));

        Poly nz = polyScale(polyFromRoots(model->z_zeros), Complex(model->filter_gain, 0.0));
        Poly dz = polyFromRoots(model->z_poles);
        auto [b, a] = DiscreteFilterSimulator::toCausalBA(nz, dz);
        std::string diff_str = DiscreteFilterSimulator::formatDifferenceEquation(b, a);

        double max_p_mag = 0.0;
        for (const auto& p : loop_z_poles) max_p_mag = std::max(max_p_mag, std::abs(p));
        bool bibo_ok = max_p_mag < 1.0 - 1e-5;
        QString bibo_tag = bibo_ok ? "BIBO STABLE" : (max_p_mag <= 1.0 + 1e-5 ? "MARGINAL |z|=1" : "UNSTABLE |z|>1");

        // Evaluate transfer function on combined discrete loop poles & zeros
        auto ev = evaluateTransferFunction(loop_z_poles, loop_z_zeros, zt, loop_z_gain);
        double def = computeAngleDeficiency(ev.angle_deg);
        bool on_rl = ev.at_pole || (std::abs(def) <= tol);

        double db_val = (ev.magnitude > 1e-12 && std::isfinite(ev.magnitude)) ? (20.0 * std::log10(ev.magnitude)) : -999.0;
        val_angle->setText(ev.at_pole ? "Pole (Undef)" : QString::fromStdString(PRECISION.fmt(ev.angle_deg) + "°"));
        val_mag->setText(ev.at_pole ? "Inf (+∞ dB)" : QString("%1 (%2 dB)")
            .arg(QString::fromStdString(PRECISION.fmt(ev.magnitude)))
            .arg(QString::fromStdString(PRECISION.fmt(db_val, true))));
        val_gain->setText(QString::fromStdString(PRECISION.fmt(ev.rl_gain_k)));

        auto zm = computeDominantZMetrics(loop_z_poles, model->ts);
        if (zm.zeta.has_value() && zm.omega_n.has_value()) {
            val_zeta->setText(QString("ζ=%1 | ωn=%2 r/s")
                .arg(QString::fromStdString(PRECISION.fmt(zm.zeta)))
                .arg(QString::fromStdString(PRECISION.fmt(zm.omega_n))));
        } else {
            val_zeta->setText("FIR (All-Zero)");
        }

        bool all_ok = bibo_ok && on_rl;
        QString bg = all_ok ? Theme::BANNER_OK_BG : (bibo_ok ? Theme::BANNER_WARN_BG : Theme::BANNER_ERR_BG);
        QString bd = all_ok ? Theme::BANNER_OK_BD : (bibo_ok ? Theme::BANNER_WARN_BD : Theme::BANNER_ERR_BD);
        QString fg = all_ok ? Theme::BANNER_OK_FG : (bibo_ok ? Theme::BANNER_WARN_FG : Theme::BANNER_ERR_FG);
        QString sub_fg = all_ok ? "#a7f3d0" : "#fde68a";
        QString val_col = on_rl ? Theme::ACCENT_ZERO : Theme::ACCENT_OPEN;

        rl_banner->setStyleSheet(QString("QFrame { background-color: %1; border: 1.5px solid %2; border-radius: 5px; }").arg(bg, bd));
        banner_title->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border: none;").arg(fg));
        banner_right->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; font-size: 11px; border: none;").arg(Theme::TEXT_PRIMARY));
        banner_sub->setStyleSheet(QString("color: %1; font-family: Monospace; font-size: 10px; border: none;").arg(sub_fg));

        for (auto* vl : {val_angle, val_mag, val_gain}) {
            vl->setStyleSheet(QString("color: %1; font-family: Monospace; font-size: 11px; font-weight: bold; border:none;").arg(val_col));
        }

        QString rl_tag = on_rl ? "ON z-LOCUS" : QString("OFF z-LOCUS (Δθ=%1°)").arg(QString::fromStdString(PRECISION.fmt(def, true)));
        banner_title->setText(QString("%1 %2  •  %3").arg(bibo_ok ? "✔" : "⚠", bibo_tag, rl_tag));
        banner_right->setText(QString("max|z_p| = %1").arg(QString::fromStdString(PRECISION.fmt(max_p_mag))));
        banner_sub->setText(QString::fromStdString(diff_str));

        // Pass combined discrete loop poles & zeros to the z-plane canvas
        canvas->poles = loop_z_poles;
        canvas->zeros = loop_z_zeros;
        canvas->test_point = zt;
        canvas->is_on_rl = on_rl;
        canvas->repaint();

        if (notify) model->notify("z_plane");
    }
};

