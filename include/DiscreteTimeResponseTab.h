#pragma once
#include "ZPlaneFilterTab.h"


// =============================================================================
// Tab 4: Discrete Time Response & Filter Simulator View (With Editable z-Poles/Zeros & PID Disable)
// =============================================================================
class DiscreteTimeResponseTab : public QWidget {
    Q_OBJECT
public:
    SharedSystemModel* model;
    DiscreteSimulationConfig cfg;

    QLineEdit *num_edit, *den_edit, *proto_zeta_edit, *proto_wn_edit, *ts_edit;
    PoleZeroEditorCard *poles_card, *zeros_card;
    QLineEdit *kp_edit, *ki_edit, *kd_edit;
    QSlider *kp_slider, *ki_slider, *kd_slider;
    QLineEdit *mag_edit, *kd_filt_edit, *cursor_edit, *nsamp_edit, *tol_edit;
    QCheckBox *enable_dpid_chk;
    QComboBox *mode_combo, *sig_combo;
    QLabel *poly_status_lbl;

    QFrame* status_banner;
    QLabel *banner_title_lbl, *banner_right_lbl, *banner_sub_lbl;
    QLabel *m_card1, *m_card2, *m_card3, *m_card4;
    TimeResponseCanvas* canvas;
    bool updating_ui = false;

    FeedbackConfigBar* fb_bar;
    MathFormulaWidget* tf_math_w;

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
        left_v->setSpacing(6);

        // 1. Num(z) / Den(z) Card
        auto* poly_card = makeCard();
        auto* poly_l = new QVBoxLayout(poly_card);
        poly_l->setContentsMargins(10, 6, 10, 6);
        auto* phdr = new QHBoxLayout();
        auto* t1 = new QLabel("1. Digital Filter  H(z) = Kd · Num(z) / Den(z)");
        t1->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::ACCENT_ASYMP));
        auto* solve_btn = makeActionBtn("Solve z-Zeros & z-Poles ➔", Theme::ACCENT_PID);
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

        poly_status_lbl = new QLabel("Enter Num(z) & Den(z) (e.g. z^2-0.94z+0.33) to auto-compute z-Zeros & z-Poles");
        poly_status_lbl->setStyleSheet("color: #94a3b8; font-size: 10px; border:none;");
        poly_l->addWidget(poly_status_lbl);
        fb_bar = new FeedbackConfigBar("z", "H_fb");
        poly_l->addWidget(fb_bar);
        tf_math_w = new MathFormulaWidget("", QColor(Theme::ACCENT_ASYMP), QColor(Theme::BG_INPUT), 10);
        poly_l->addWidget(tf_math_w);
        fb_bar->onChanged = [this]() { fb_bar->syncToModelZ(model); runSimulation(true); };
        left_v->addWidget(poly_card);

        // 2. Editable z-Poles (×) & z-Zeros (○) Tables Row
        auto* pz_row = new QHBoxLayout();
        poles_card = new PoleZeroEditorCard("z-Poles (×)", Theme::ACCENT_POLE, 0.6);
        zeros_card = new PoleZeroEditorCard("z-Zeros (○)", Theme::ACCENT_ZERO, -1.0);
        pz_row->addWidget(poles_card, 1);
        pz_row->addWidget(zeros_card, 1);
        left_v->addLayout(pz_row);

        // 3. Discrete PID & Prototype Card
        auto* pid_card = makeCard();
        auto* pid_l = new QVBoxLayout(pid_card);
        pid_l->setContentsMargins(10, 6, 10, 6);
        auto* mhdr = new QHBoxLayout();
        auto* t2 = new QLabel("2. Digital Filter H(z), Discrete PID & Input r[k]");
        t2->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::ACCENT_PID));

        enable_dpid_chk = new QCheckBox("Enable PID C(z)");
        enable_dpid_chk->setChecked(false);
        enable_dpid_chk->setStyleSheet(QString("QCheckBox { color: %1; font-weight: bold; font-size: 11px; border: none; }").arg(Theme::ACCENT_PID));

        mode_combo = new QComboBox();
        mode_combo->addItems({
            "Filter H(z) (Open-Loop, No PID)",
            "Closed-Loop (No PID, C(z)=1)",
            "Closed-Loop Discrete PID"
        });
        mode_combo->setStyleSheet(QString("background-color: %1; color: %2; padding: 3px; font-weight: bold; font-size: 11px;").arg(Theme::BG_INPUT, Theme::TEXT_PRIMARY));

        mhdr->addWidget(t2); mhdr->addStretch(); mhdr->addWidget(enable_dpid_chk); mhdr->addWidget(mode_combo);
        pid_l->addLayout(mhdr);

        // Prototype z = e^(s*Ts) & Norm H(1)=1 Bar
        auto* proto_bar = new QHBoxLayout();
        proto_zeta_edit = makeDarkEntry("0.707", 45);
        proto_wn_edit   = makeDarkEntry("15.0", 45);
        ts_edit         = makeDarkEntry(QString::number(model->ts), 45);
        auto* set_z_btn = makeActionBtn("Set z = e^(sTs)", Theme::ACCENT_PID);
        auto* norm_btn  = makeActionBtn("Norm H(1)=1", Theme::ACCENT_ZERO);
        auto addP = [&](const QString& lbl, QWidget* w) {
            auto* l = new QLabel(lbl); l->setStyleSheet("color: #94a3b8; font-family: Monospace; font-size: 10px; font-weight: bold; border:none;");
            proto_bar->addWidget(l); proto_bar->addWidget(w);
        };
        addP("ζ=", proto_zeta_edit);
        addP("ωn=", proto_wn_edit);
        addP("Ts=", ts_edit);
        proto_bar->addWidget(set_z_btn);
        proto_bar->addWidget(norm_btn);
        pid_l->addLayout(proto_bar);

        // Discrete PID Sliders
        auto* gains_grid = new QHBoxLayout();
        auto makeGainCol = [&](const QString& title, QLineEdit*& ed, QSlider*& sl, double max_v) {
            auto* col = new QVBoxLayout();
            auto* lbl = new QLabel(title); lbl->setStyleSheet("color: #94a3b8; font-size: 10px; font-weight: bold; border:none;");
            ed = makeDarkEntry("1.0", 60);
            sl = new QSlider(Qt::Horizontal);
            sl->setRange(0, static_cast<int>(max_v * 100));
            col->addWidget(lbl); col->addWidget(ed); col->addWidget(sl);
            gains_grid->addLayout(col);
        };
        makeGainCol("Discrete Kp", kp_edit, kp_slider, 15.0);
        makeGainCol("Discrete Ki", ki_edit, ki_slider, 15.0);
        makeGainCol("Discrete Kd", kd_edit, kd_slider, 5.0);
        pid_l->addLayout(gains_grid);

        auto* ggrid = new QGridLayout();
        sig_combo = new QComboBox();
        sig_combo->addItems({"Step", "Impulse", "Ramp", "Sine", "Two-Tone"});
        sig_combo->setStyleSheet(mode_combo->styleSheet());
        mag_edit     = makeDarkEntry("1.0", 48);
        kd_filt_edit = makeDarkEntry("0.0955", 55);
        cursor_edit  = makeDarkEntry("15", 45);
        nsamp_edit   = makeDarkEntry("80", 45);
        tol_edit     = makeDarkEntry("2.0", 45);

        auto addC = [&](int c, const QString& lbl, QWidget* w) {
            auto* l = new QLabel(lbl); l->setStyleSheet("color: #94a3b8; font-size: 9px; font-weight: bold; border:none;");
            ggrid->addWidget(l, 0, c); ggrid->addWidget(w, 1, c);
        };
        addC(0, "Input r[k]", sig_combo);
        addC(1, "r[k] Mag", mag_edit);
        addC(2, "Filter Kd", kd_filt_edit);
        addC(3, "Cursor k", cursor_edit);
        addC(4, "Samples N", nsamp_edit);
        addC(5, "Tol (±%)", tol_edit);
        pid_l->addLayout(ggrid);
        left_v->addWidget(pid_card);

        // 4. Discrete Evaluation & Stability Banner Card (Turns Orange when |z| >= 1!)
        auto* tel_card = makeCard();
        auto* tel_l = new QVBoxLayout(tel_card);
        tel_l->setContentsMargins(10, 6, 10, 6);

        status_banner = new QFrame();
        status_banner->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        auto* sb_l = new QVBoxLayout(status_banner);
        sb_l->setContentsMargins(10, 5, 10, 5);
        sb_l->setSpacing(2);
        auto* sb_top = new QHBoxLayout();
        banner_title_lbl = new QLabel("✔ DIGITAL FILTER BIBO STABLE");
        banner_right_lbl = new QLabel("max|z_p| = 0.7520");
        sb_top->addWidget(banner_title_lbl); sb_top->addStretch(); sb_top->addWidget(banner_right_lbl);
        banner_sub_lbl = new QLabel("y[k] = ...");
        banner_sub_lbl->setWordWrap(true);
        sb_l->addLayout(sb_top);
        sb_l->addWidget(banner_sub_lbl);
        tel_l->addWidget(status_banner);

        auto* mgrid = new QGridLayout();
        auto makeM = [&](int r, int c, const QString& t, QLabel*& out) {
            auto* b = makeCard();
            b->setStyleSheet(QString("background-color: %1; border: 1px solid %2; padding: 4px;").arg(Theme::BG_INPUT, Theme::BORDER_INPUT));
            auto* vl = new QVBoxLayout(b);
            vl->setContentsMargins(6, 3, 6, 3);
            auto* tl = new QLabel(t); tl->setStyleSheet("color: #94a3b8; font-size: 9px; font-weight: bold; border:none;");
            out = new QLabel("—"); out->setStyleSheet(QString("color: %1; font-family: Monospace; font-size: 10px; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));
            vl->addWidget(tl); vl->addWidget(out);
            mgrid->addWidget(b, r, c);
        };
        makeM(0, 0, "RISE / SETTLING SAMPLES", m_card1);
        makeM(0, 1, "OVERSHOOT / PEAK y[k] & S.S.E.", m_card2);
        makeM(1, 0, "EQUIV DAMPING ζ / ωn", m_card3);
        makeM(1, 1, "SAMPLE y[k] & |e_ss|", m_card4);
        tel_l->addLayout(mgrid);
        left_v->addWidget(tel_card, 1);

        // Right Plot Canvas
        auto* right_card = makeCard();
        auto* right_l = new QVBoxLayout(right_card);
        auto* r_hdr = new QHBoxLayout();
        auto* rt = new QLabel("Discrete Time Sequence  y[k]  (L-Click: Sample k • Scroll: Zoom • R-Drag: Pan)");
        rt->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 13px; border:none;").arg(Theme::TEXT_PRIMARY));
        auto* stems_chk = new QCheckBox("Stems ●");       stems_chk->setChecked(true); stems_chk->setStyleSheet("color: #38bdf8; font-weight: bold;");
        auto* inset_chk = new QCheckBox("z-Plane Inset"); inset_chk->setChecked(true); inset_chk->setStyleSheet("color: #22d3ee; font-weight: bold;");
        auto* fit_b     = makeActionBtn("Fit View", Theme::TEXT_PRIMARY, Theme::BG_PANEL);
        r_hdr->addWidget(rt); r_hdr->addStretch();
        r_hdr->addWidget(stems_chk); r_hdr->addWidget(inset_chk); r_hdr->addWidget(fit_b);
        right_l->addLayout(r_hdr);

        canvas = new TimeResponseCanvas(true);
        attachSvgExportMenu(canvas, "discrete_time_response.svg");
        right_l->addWidget(canvas, 1);

        splitter->addWidget(left_w);
        splitter->addWidget(right_card);
        splitter->setStretchFactor(0, 0);
        splitter->setStretchFactor(1, 1);
        left_w->setMinimumWidth(360);

        // Connections
        connect(solve_btn, &QPushButton::clicked, this, &DiscreteTimeResponseTab::applyPolyNumDen);
        connect(num_edit, &QLineEdit::returnPressed, this, &DiscreteTimeResponseTab::applyPolyNumDen);
        connect(den_edit, &QLineEdit::returnPressed, this, &DiscreteTimeResponseTab::applyPolyNumDen);

        poles_card->onValuesChanged = [this]() {
            if (updating_ui) return;
            model->z_poles = poles_card->getValues();
            den_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->z_poles, "z")));
            runSimulation(true);
        };
        zeros_card->onValuesChanged = [this]() {
            if (updating_ui) return;
            model->z_zeros = zeros_card->getValues();
            num_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->z_zeros, "z")));
            runSimulation(true);
        };

        connect(set_z_btn, &QPushButton::clicked, this, [this]() {
            double zv = parseNumericInput(proto_zeta_edit->text().toStdString(), 0.7071);
            double wv = std::max(1e-4, parseNumericInput(proto_wn_edit->text().toStdString(), 15.0));
            double tsv = std::max(1e-5, parseNumericInput(ts_edit->text().toStdString(), 0.05));
            model->z_poles = zPolesFromZetaWn(zv, wv, tsv);
            auto ev = evaluateTransferFunction(model->z_poles, model->z_zeros, Complex(1.0, 0.0), 1.0);
            if (ev.magnitude > 1e-9) model->filter_gain = 1.0 / ev.magnitude;
            loadFromModel();
            model->notify("discrete_sim");
        });

        connect(norm_btn, &QPushButton::clicked, this, [this]() {
            auto ev = evaluateTransferFunction(model->z_poles, model->z_zeros, Complex(1.0, 0.0), 1.0);
            if (ev.magnitude > 1e-9) {
                kd_filt_edit->setText(QString::number(1.0 / ev.magnitude, 'g', 5));
                runSimulation(true);
            }
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

        for (auto* ed : {mag_edit, kd_filt_edit, ts_edit, cursor_edit, nsamp_edit, tol_edit}) {
            connect(ed, &QLineEdit::editingFinished, this, [this]() { runSimulation(true); });
        }
        connect(sig_combo, &QComboBox::currentTextChanged, this, [this]() { runSimulation(true); });
        connect(stems_chk, &QCheckBox::toggled, this, [this](bool v) { canvas->show_stems = v; canvas->update(); });

        connect(inset_chk, &QCheckBox::toggled, this, [this](bool v) { canvas->show_inset = v; canvas->update(); });
        connect(fit_b,     &QPushButton::clicked, this, [this]() { canvas->resetView(); });

        canvas->onCursorChanged = [this](double k_val) {
            cursor_edit->setText(QString::number(static_cast<int>(k_val)));
            runSimulation(false);
        };

        loadFromModel();
    }

    void updatePidWidgetsState() {
        bool pid_on = enable_dpid_chk->isChecked() && (mode_combo->currentIndex() == 2);
        for (QWidget* w : std::vector<QWidget*>{kp_edit, ki_edit, kd_edit, kp_slider, ki_slider, kd_slider}) {
            w->setEnabled(pid_on);
        }
    }

    void applyPolyNumDen() {
        try {
            auto pn = parsePolynomialInput(num_edit->text().toStdString());
            auto pd = parsePolynomialInput(den_edit->text().toStdString());
            model->z_zeros = pn.roots;
            model->z_poles = pd.roots;
            poles_card->setValues(model->z_poles);
            zeros_card->setValues(model->z_zeros);
            runSimulation(true);
        } catch (const std::exception& e) {
            poly_status_lbl->setText(QString("⚠ %1").arg(e.what()));
        }
    }

    void loadFromModel() {
        updating_ui = true;
        poles_card->setValues(model->z_poles);
        zeros_card->setValues(model->z_zeros);
        num_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->z_zeros, "z")));
        den_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->z_poles, "z")));
        kd_filt_edit->setText(QString::number(model->filter_gain, 'g', 5));
        ts_edit->setText(QString::number(model->ts, 'g', 4));
        kp_edit->setText(QString::number(model->d_kp, 'g', 4));
        ki_edit->setText(QString::number(model->d_ki, 'g', 4));
        kd_edit->setText(QString::number(model->d_kd, 'g', 4));
        kp_slider->setValue(static_cast<int>(model->d_kp * 100));
        ki_slider->setValue(static_cast<int>(model->d_ki * 100));
        kd_slider->setValue(static_cast<int>(model->d_kd * 100));
        updating_ui = false;
        updatePidWidgetsState();
        runSimulation(false);
        fb_bar->loadFromModelZ(model);
    }

    void runSimulation(bool notify) {
        if (updating_ui) return;
        cfg.z_poles = model->z_poles;
        cfg.z_zeros = model->z_zeros;
        cfg.filter_gain = parseNumericInput(kd_filt_edit->text().toStdString(), 0.0955);
        cfg.unity_feedback = model->unity_feedback_z;
        cfg.hz_fb_poles = model->hz_fb_poles;
        cfg.hz_fb_zeros = model->hz_fb_zeros;
        cfg.hz_fb_gain  = model->hz_fb_gain;

        std::string hz_ltx = formatTransferFunctionLatex(model->z_poles, model->z_zeros, cfg.filter_gain, "z", "H");
        if (!model->unity_feedback_z) {
            hz_ltx += " \\; , \\quad " + formatTransferFunctionLatex(model->hz_fb_poles, model->hz_fb_zeros, model->hz_fb_gain, "z", "H_{\\mathrm{fb}}");
        } else {
            hz_ltx += " \\; , \\quad H_{\\mathrm{fb}}(z) = 1";
        }
        tf_math_w->setLatex(QString::fromStdString(hz_ltx));
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
        cfg.settle_tol_pct = std::max(0.1, parseNumericInput(tol_edit->text().toStdString(), 2.0));

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

        // Style Discrete Stability Banner (Green when |z| < 1, Orange when Marginal/Unstable!)
        bool is_stable = (res.stability_state == "STABLE");
        QString bg = is_stable ? Theme::BANNER_OK_BG : Theme::BANNER_WARN_BG;
        QString bd = is_stable ? Theme::BANNER_OK_BD : Theme::BANNER_WARN_BD;
        QString fg = is_stable ? Theme::BANNER_OK_FG : Theme::BANNER_WARN_FG;
        QString sub_fg = is_stable ? "#a7f3d0" : "#fde68a";

        status_banner->setStyleSheet(QString("QFrame { background-color: %1; border: 1.5px solid %2; border-radius: 5px; }").arg(bg, bd));
        banner_title_lbl->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border: none;").arg(fg));
        banner_right_lbl->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; font-size: 11px; border: none;").arg(Theme::TEXT_PRIMARY));
        banner_sub_lbl->setStyleSheet(QString("color: %1; font-family: Monospace; font-size: 10px; border: none;").arg(sub_fg));

        if (is_stable) {
            banner_title_lbl->setText(cfg.closed_loop ? "✔ CLOSED-LOOP BIBO STABLE" : "✔ DIGITAL FILTER BIBO STABLE");
        } else if (res.stability_state == "MARGINAL") {
            banner_title_lbl->setText("⚠ MARGINALLY STABLE (|z| = 1)");
        } else {
            banner_title_lbl->setText("✖ DISCRETE SYSTEM UNSTABLE (|z| > 1)");
        }
        banner_right_lbl->setText(QString("max|z_p| = %1").arg(QString::fromStdString(PRECISION.fmt(res.max_pole_mag))));
        banner_sub_lbl->setText(QString::fromStdString(res.diff_eq_str));

        QString kr_s = res.rise_samples.has_value()
            ? QString("%1 (%2s)").arg(res.rise_samples.value()).arg(QString::fromStdString(PRECISION.fmt(res.rise_time_sec)))
            : "N/A";
        QString ks_s = res.settling_samples.has_value()
            ? QString("%1 (%2s)").arg(res.settling_samples.value()).arg(QString::fromStdString(PRECISION.fmt(res.settling_time_sec)))
            : "> N";
        m_card1->setText(QString("kr: %1 | ks: %2").arg(kr_s, ks_s));

        double rel_err = (res.steady_state_error / std::max(std::abs(cfg.input_magnitude), 1e-12)) * 100.0;
        m_card2->setText(QString("%OS: %1% (pk %2) | S.S.E.: %3 (%4%)")
            .arg(QString::fromStdString(PRECISION.fmt(res.overshoot_pct)))
            .arg(QString::fromStdString(PRECISION.fmt(res.peak_value)))
            .arg(QString::fromStdString(PRECISION.fmt(res.steady_state_error)))
            .arg(QString::fromStdString(PRECISION.fmt(rel_err))));

        m_card3->setText(res.zeta.has_value()
            ? QString("ζ: %1 | ωn: %2 r/s").arg(QString::fromStdString(PRECISION.fmt(res.zeta)), QString::fromStdString(PRECISION.fmt(res.omega_n)))
            : "FIR / All-Zero");

        m_card4->setText(QString("y[%1] = %2 | S.S.E. = %3")
            .arg(res.cursor_k)
            .arg(QString::fromStdString(PRECISION.fmtComplex(res.cursor_y)))
            .arg(QString::fromStdString(PRECISION.fmt(res.steady_state_error))));

        canvas->dsim_res = res;
        canvas->show_cl = cfg.closed_loop;
        canvas->settle_tol_pct = cfg.settle_tol_pct;
        canvas->update();
    }
};

