#pragma once
#include "BodeNyquistCanvas.h"


class FrequencyResponseTab : public QWidget {
    Q_OBJECT
public:
    SharedSystemModel* model;
    QLineEdit *num_edit, *den_edit, *gain_edit;
    QLabel *poly_title_lbl, *num_lbl, *den_lbl, *gain_lbl, *poly_status_lbl;
    FeedbackConfigBar* fb_bar;
    MathFormulaWidget* tf_math_w;
    PoleZeroEditorCard *poles_card, *zeros_card;

    QComboBox* domain_combo;
    QCheckBox* include_pid_chk;
    QLineEdit *w_min_edit, *w_max_edit, *cursor_w_edit;

    QFrame* margin_banner;
    QLabel *banner_title, *banner_right, *banner_sub;
    QLabel *m_card1, *m_card2, *m_card3, *m_card4;
    BodeNyquistCanvas* canvas;
    bool updating_ui = false;

    explicit FrequencyResponseTab(SharedSystemModel* m, QWidget* parent = nullptr)
        : QWidget(parent), model(m)
    {
        auto* root_l = new QHBoxLayout(this);
        root_l->setContentsMargins(0, 0, 0, 0);
        auto* splitter = new QSplitter(Qt::Horizontal, this);
        root_l->addWidget(splitter);

        auto* left_w = new QWidget();
        auto* left_v = new QVBoxLayout(left_w);
        left_v->setContentsMargins(0, 0, 6, 0);
        left_v->setSpacing(6);

        // 1. Transfer Function Polynomial (Num / Den) & Feedback Editor Card
        auto* poly_card = makeCard();
        auto* poly_l = new QVBoxLayout(poly_card);
        poly_l->setContentsMargins(10, 6, 10, 6);
        poly_l->setSpacing(4);

        auto* poly_hdr = new QHBoxLayout();
        poly_title_lbl = new QLabel("1. Plant Transfer Function  G(s) = K · Num(s) / Den(s)");
        poly_title_lbl->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));
        auto* solve_btn = makeActionBtn("Solve Zeros & Poles ➔", Theme::ACCENT_PID);
        poly_hdr->addWidget(poly_title_lbl);
        poly_hdr->addStretch();
        poly_hdr->addWidget(solve_btn);
        poly_l->addLayout(poly_hdr);

        auto* pgrid = new QGridLayout();
        num_lbl = new QLabel("Num(s):");
        num_lbl->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));
        num_edit = makeDarkEntry();

        den_lbl = new QLabel("Den(s):");
        den_lbl->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; border:none;").arg(Theme::ACCENT_POLE));
        den_edit = makeDarkEntry();

        gain_lbl = new QLabel("Gain K:");
        gain_lbl->setStyleSheet("color: #94a3b8; font-family: Monospace; font-weight: bold; border:none;");
        gain_edit = makeDarkEntry("1.0", 50);

        pgrid->addWidget(num_lbl,  0, 0); pgrid->addWidget(num_edit,  0, 1);
        pgrid->addWidget(den_lbl,  0, 2); pgrid->addWidget(den_edit,  0, 3);
        pgrid->addWidget(gain_lbl, 0, 4); pgrid->addWidget(gain_edit, 0, 5);
        poly_l->addLayout(pgrid);

        poly_status_lbl = new QLabel("Enter polynomials (e.g. s^2+2s+2 or (s+1)(s+3)) and press Enter / Solve");
        poly_status_lbl->setStyleSheet("color: #94a3b8; font-size: 10px; border:none;");
        poly_l->addWidget(poly_status_lbl);

        fb_bar = new FeedbackConfigBar("s", "H");
        poly_l->addWidget(fb_bar);

        tf_math_w = new MathFormulaWidget("", QColor(Theme::ACCENT_ZERO), QColor(Theme::BG_INPUT), 10);
        poly_l->addWidget(tf_math_w);
        left_v->addWidget(poly_card);

        // 2. Editable Poles (×) & Zeros (○) Tables Row
        auto* pz_row = new QHBoxLayout();
        poles_card = new PoleZeroEditorCard("Poles (×)", Theme::ACCENT_POLE, -1.0);
        zeros_card = new PoleZeroEditorCard("Zeros (○)", Theme::ACCENT_ZERO, -3.0);
        pz_row->addWidget(poles_card, 1);
        pz_row->addWidget(zeros_card, 1);
        left_v->addLayout(pz_row);

        // 3. Frequency Sweep & Loop Mode Configuration Card
        auto* sys_card = makeCard();
        auto* sl = new QVBoxLayout(sys_card);
        sl->setContentsMargins(10, 6, 10, 6);
        sl->setSpacing(4);
        auto* t1 = new QLabel("2. Frequency-Domain Loop Mode & Sweep Range");
        t1->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::ACCENT_PID));
        sl->addWidget(t1);

        domain_combo = new QComboBox();
        domain_combo->addItems({
            "Continuous Open-Loop Return Ratio L(jω) = C(jω)G(jω)H(jω)  [GM & PM]",
            "Continuous Closed-Loop System T(jω) = Y(jω)/R(jω)  [Bandwidth & Mr]",
            "Discrete Digital Filter Frequency Response H(e^{jωTs})"
        });
        domain_combo->setStyleSheet(QString("background-color: %1; color: %2; padding: 3px; font-weight: bold; font-size: 11px;")
                                    .arg(Theme::BG_INPUT, Theme::TEXT_PRIMARY));
        sl->addWidget(domain_combo);

        include_pid_chk = new QCheckBox("Include Active PID Controller C(s) / C(z) in Frequency Loop");
        include_pid_chk->setChecked(false);
        include_pid_chk->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 10px; border:none;").arg(Theme::ACCENT_PID));
        sl->addWidget(include_pid_chk);

        auto* w_grid = new QGridLayout();
        w_min_edit    = makeDarkEntry("-2", 55);
        w_max_edit    = makeDarkEntry("2", 55);
        cursor_w_edit = makeDarkEntry("1.0", 70);
        auto addW = [&](int c, const QString& lbl, QWidget* w) {
            auto* l = new QLabel(lbl); l->setStyleSheet("color: #94a3b8; font-size: 9px; font-weight: bold; border:none;");
            w_grid->addWidget(l, 0, c); w_grid->addWidget(w, 1, c);
        };
        addW(0, "Min log10(ω)", w_min_edit);
        addW(1, "Max log10(ω)", w_max_edit);
        addW(2, "Cursor ω (rad/s)", cursor_w_edit);
        sl->addLayout(w_grid);
        left_v->addWidget(sys_card);

        // 4. Stability Margins & Bandwidth Telemetry Card
        auto* tel_card = makeCard();
        auto* tl = new QVBoxLayout(tel_card);
        tl->setContentsMargins(10, 6, 10, 6);
        tl->setSpacing(4);
        auto* t2 = new QLabel("3. Bode Stability Margins, Bandwidth & Cursor Telemetry");
        t2->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::TEXT_PRIMARY));
        tl->addWidget(t2);

        margin_banner = new QFrame();
        margin_banner->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        auto* bl = new QVBoxLayout(margin_banner);
        bl->setContentsMargins(10, 5, 10, 5);
        bl->setSpacing(2);
        auto* btop = new QHBoxLayout();
        banner_title = new QLabel("✔ POSITIVE STABILITY MARGINS");
        banner_right = new QLabel("PM = —° | GM = — dB");
        btop->addWidget(banner_title); btop->addStretch(); btop->addWidget(banner_right);
        banner_sub = new QLabel();
        banner_sub->setWordWrap(true);
        bl->addLayout(btop);
        bl->addWidget(banner_sub);
        tl->addWidget(margin_banner);

        auto* mgrid = new QGridLayout();
        auto makeM = [&](int r, int c, const QString& title, QLabel*& out) {
            auto* b = makeCard();
            b->setStyleSheet(QString("background-color: %1; border: 1px solid %2; padding: 4px;").arg(Theme::BG_INPUT, Theme::BORDER_INPUT));
            auto* vl = new QVBoxLayout(b);
            vl->setContentsMargins(6, 3, 6, 3);
            auto* l = new QLabel(title); l->setStyleSheet("color: #94a3b8; font-size: 9px; font-weight: bold; border:none;");
            out = new QLabel("—"); out->setStyleSheet(QString("color: %1; font-family: Monospace; font-size: 10px; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));
            vl->addWidget(l); vl->addWidget(out);
            mgrid->addWidget(b, r, c);
        };
        makeM(0, 0, "PHASE MARGIN (PM) & ω_gc", m_card1);
        makeM(0, 1, "GAIN MARGIN (GM) & ω_pc", m_card2);
        makeM(1, 0, "RESONANT PEAK Mr & -3dB BW", m_card3);
        makeM(1, 1, "CURSOR INSPECTION @ ω", m_card4);
        tl->addLayout(mgrid);
        left_v->addWidget(tel_card);
        left_v->addStretch(1);

        // Right Plot Panel (Bode / Nyquist Switcher + Asymptotes Toggle)
        auto* right_card = makeCard();
        auto* rl = new QVBoxLayout(right_card);
        auto* phdr = new QHBoxLayout();
        auto* pt = new QLabel("Frequency-Domain Bode & Nyquist Diagram");
        pt->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 13px; border:none;").arg(Theme::TEXT_PRIMARY));

        auto* rb_bode = new QRadioButton("Bode Plot"); rb_bode->setChecked(true);
        auto* rb_nyq  = new QRadioButton("Nyquist Polar");
        for (auto* rb : {rb_bode, rb_nyq}) rb->setStyleSheet("color: #f0f6fc; font-weight: bold; font-size: 11px;");

        auto* chk_asymp = new QCheckBox("Straight-Line Asymptotes"); chk_asymp->setChecked(true); chk_asymp->setStyleSheet("color: #f59e0b; font-weight: bold;");
        auto* chk_marg  = new QCheckBox("GM/PM Lines");              chk_marg->setChecked(true);  chk_marg->setStyleSheet("color: #34d399; font-weight: bold;");
        auto* fit_b     = makeActionBtn("Fit View", Theme::TEXT_PRIMARY, Theme::BG_PANEL);

        phdr->addWidget(pt); phdr->addStretch();
        phdr->addWidget(rb_bode); phdr->addWidget(rb_nyq);
        phdr->addWidget(chk_asymp); phdr->addWidget(chk_marg); phdr->addWidget(fit_b);
        rl->addLayout(phdr);

        canvas = new BodeNyquistCanvas();
        rl->addWidget(canvas, 1);

        splitter->addWidget(left_w);
        splitter->addWidget(right_card);
        splitter->setStretchFactor(0, 0);
        splitter->setStretchFactor(1, 1);
        left_w->setMinimumWidth(360);

        // Connections
        connect(solve_btn, &QPushButton::clicked, this, &FrequencyResponseTab::applyPolyNumDen);
        connect(num_edit, &QLineEdit::returnPressed, this, &FrequencyResponseTab::applyPolyNumDen);
        connect(den_edit, &QLineEdit::returnPressed, this, &FrequencyResponseTab::applyPolyNumDen);

        connect(gain_edit, &QLineEdit::editingFinished, this, [this]() {
            if (updating_ui) return;
            double g = parseNumericInput(gain_edit->text().toStdString(), 1.0);
            if (domain_combo->currentIndex() != 2) model->plant_gain = g;
            else model->filter_gain = g;
            evaluateCurrent(true);
        });

        fb_bar->onChanged = [this]() {
            if (updating_ui) return;
            if (domain_combo->currentIndex() != 2) fb_bar->syncToModelS(model);
            else fb_bar->syncToModelZ(model);
            evaluateCurrent(true);
        };

        poles_card->onValuesChanged = [this]() {
            if (updating_ui) return;
            if (domain_combo->currentIndex() != 2) {
                model->poles = poles_card->getValues();
                den_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->poles, "s")));
            } else {
                model->z_poles = poles_card->getValues();
                den_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->z_poles, "z")));
            }
            evaluateCurrent(true);
        };

        zeros_card->onValuesChanged = [this]() {
            if (updating_ui) return;
            if (domain_combo->currentIndex() != 2) {
                model->zeros = zeros_card->getValues();
                num_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->zeros, "s")));
            } else {
                model->z_zeros = zeros_card->getValues();
                num_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->z_zeros, "z")));
            }
            evaluateCurrent(true);
        };

        connect(domain_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
            if (updating_ui) return;
            loadFromModel();
        });
        connect(include_pid_chk, &QCheckBox::toggled, this, [this](bool) { evaluateCurrent(false); });
        for (auto* ed : {w_min_edit, w_max_edit, cursor_w_edit}) {
            connect(ed, &QLineEdit::editingFinished, this, [this]() { evaluateCurrent(false); });
        }
        connect(rb_bode, &QRadioButton::toggled, this, [this](bool is_bode) {
            canvas->nyquist_mode = !is_bode;
            canvas->update();
        });
        connect(chk_asymp, &QCheckBox::toggled, this, [this](bool v) { canvas->show_asymptotes = v; canvas->update(); });
        connect(chk_marg,  &QCheckBox::toggled, this, [this](bool v) { canvas->show_margins = v; canvas->update(); });
        connect(fit_b,     &QPushButton::clicked, this, [this]() { canvas->resetView(); });

        canvas->onFrequencySelected = [this](double w_sel) {
            cursor_w_edit->setText(QString::number(w_sel, 'g', 4));
            evaluateCurrent(false);
        };

        loadFromModel();
    }

    void applyPolyNumDen() {
        try {
            auto pn = parsePolynomialInput(num_edit->text().toStdString());
            auto pd = parsePolynomialInput(den_edit->text().toStdString());
            double ratio = pn.leading_gain / std::max(std::abs(pd.leading_gain), 1e-12);

            if (domain_combo->currentIndex() != 2) {
                model->zeros = pn.roots;
                model->poles = pd.roots;
                if (std::abs(ratio) > 1e-12) {
                    model->plant_gain = ratio;
                    gain_edit->setText(QString::number(ratio, 'g', 5));
                }
                poles_card->setValues(model->poles);
                zeros_card->setValues(model->zeros);
                poly_status_lbl->setText(QString("✔ Solved %1 s-Poles & %2 s-Zeros (K = %3)")
                                         .arg(model->poles.size()).arg(model->zeros.size()).arg(model->plant_gain));
            } else {
                model->z_zeros = pn.roots;
                model->z_poles = pd.roots;
                if (std::abs(ratio - 1.0) > 1e-6 && std::abs(ratio) > 1e-12) {
                    model->filter_gain = ratio;
                    gain_edit->setText(QString::number(ratio, 'g', 5));
                }
                poles_card->setValues(model->z_poles);
                zeros_card->setValues(model->z_zeros);
                poly_status_lbl->setText(QString("✔ Solved %1 z-Poles & %2 z-Zeros")
                                         .arg(model->z_poles.size()).arg(model->z_zeros.size()));
            }
            evaluateCurrent(true);
        } catch (const std::exception& e) {
            poly_status_lbl->setText(QString("⚠ Polynomial error: %1").arg(e.what()));
        }
    }

    void loadFromModel() {
        updating_ui = true;
        bool is_z = (domain_combo->currentIndex() == 2);
        if (!is_z) {
            poly_title_lbl->setText("1. Plant Transfer Function  G(s) = K · Num(s) / Den(s)");
            num_lbl->setText("Num(s):");
            den_lbl->setText("Den(s):");
            gain_lbl->setText("Gain K:");
            poles_card->setValues(model->poles);
            zeros_card->setValues(model->zeros);
            num_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->zeros, "s")));
            den_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->poles, "s")));
            gain_edit->setText(QString::number(model->plant_gain, 'g', 5));
            fb_bar->loadFromModelS(model);
        } else {
            poly_title_lbl->setText("1. Digital Filter  H(z) = Kd · Num(z) / Den(z)");
            num_lbl->setText("Num(z):");
            den_lbl->setText("Den(z):");
            gain_lbl->setText("Gain Kd:");
            poles_card->setValues(model->z_poles);
            zeros_card->setValues(model->z_zeros);
            num_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->z_zeros, "z")));
            den_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->z_poles, "z")));
            gain_edit->setText(QString::number(model->filter_gain, 'g', 5));
            fb_bar->loadFromModelZ(model);
        }
        updating_ui = false;
        evaluateCurrent(false);
    }

    void evaluateCurrent(bool notify = false) {
        if (updating_ui) return;
        int m_idx = domain_combo->currentIndex();
        bool pid_on = include_pid_chk->isChecked();
        double wmin_e = parseNumericInput(w_min_edit->text().toStdString(), -2.0);
        double wmax_e = parseNumericInput(w_max_edit->text().toStdString(), 2.0);
        if (wmax_e <= wmin_e) wmax_e = wmin_e + 2.0;
        double cw = std::max(1e-5, parseNumericInput(cursor_w_edit->text().toStdString(), 1.0));

        if (m_idx != 2) {
            std::string gs_ltx = formatTransferFunctionLatex(model->poles, model->zeros, model->plant_gain, "s", "G");
            if (!model->unity_feedback_s) {
                gs_ltx += " \\; , \\quad " + formatTransferFunctionLatex(model->h_poles, model->h_zeros, model->h_gain, "s", "H");
            } else {
                gs_ltx += " \\; , \\quad H(s) = 1";
            }
            tf_math_w->setLatex(QString::fromStdString(gs_ltx));
        } else {
            std::string hz_ltx = formatTransferFunctionLatex(model->z_poles, model->z_zeros, model->filter_gain, "z", "H");
            if (!model->unity_feedback_z) {
                hz_ltx += " \\; , \\quad " + formatTransferFunctionLatex(model->hz_fb_poles, model->hz_fb_zeros, model->hz_fb_gain, "z", "H_{\\mathrm{fb}}");
            } else {
                hz_ltx += " \\; , \\quad H_{\\mathrm{fb}}(z) = 1";
            }
            tf_math_w->setLatex(QString::fromStdString(hz_ltx));
        }

        auto res = FrequencyDomainEngine::evaluate(model, m_idx, pid_on, wmin_e, wmax_e, cw);
        canvas->freq_res = res;

        bool pm_ok = !res.pm_deg.has_value() || (res.pm_deg.value() > 0.0 && res.pm_deg.value() < 180.0);
        bool gm_ok = !res.gm_db.has_value()  || (res.gm_db.value() > 0.0);
        bool all_ok = pm_ok && gm_ok;

        QString bg = all_ok ? Theme::BANNER_OK_BG : Theme::BANNER_WARN_BG;
        QString bd = all_ok ? Theme::BANNER_OK_BD : Theme::BANNER_WARN_BD;
        QString fg = all_ok ? Theme::BANNER_OK_FG : Theme::BANNER_WARN_FG;

        margin_banner->setStyleSheet(QString("QFrame { background-color: %1; border: 1.5px solid %2; border-radius: 5px; }").arg(bg, bd));
        banner_title->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border:none;").arg(fg));
        banner_right->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; font-size: 11px; border:none;").arg(Theme::TEXT_PRIMARY));
        banner_sub->setStyleSheet(QString("color: %1; font-size: 10px; border:none;").arg(all_ok ? "#a7f3d0" : "#fde68a"));

        banner_title->setText(all_ok ? "✔ POSITIVE STABILITY MARGINS" : "⚠ NEGATIVE / CRITICAL STABILITY MARGINS");
        banner_right->setText(QString("PM = %1° | GM = %2 dB")
            .arg(QString::fromStdString(PRECISION.fmt(res.pm_deg, false, false, "∞")))
            .arg(QString::fromStdString(PRECISION.fmt(res.gm_db, false, false, "∞"))));

        std::string corners_s;
        for (size_t i = 0; i < std::min<size_t>(5, res.corner_freqs.size()); ++i) {
            corners_s += (i ? ", " : "") + PRECISION.fmt(res.corner_freqs[i]);
        }
        banner_sub->setText(QString("Corner Frequencies ωc (rad/s): [%1]   •   DC Gain: %2 dB")
            .arg(QString::fromStdString(corners_s.empty() ? "None" : corners_s))
            .arg(QString::fromStdString(PRECISION.fmt(res.dc_gain_db))));

        m_card1->setText(QString("PM: %1°\n@ ω_gc = %2 rad/s")
            .arg(QString::fromStdString(PRECISION.fmt(res.pm_deg, false, false, "∞")))
            .arg(QString::fromStdString(PRECISION.fmt(res.w_gc, false, false, "None"))));

        m_card2->setText(QString("GM: %1 dB\n@ ω_pc = %2 rad/s")
            .arg(QString::fromStdString(PRECISION.fmt(res.gm_db, false, false, "∞")))
            .arg(QString::fromStdString(PRECISION.fmt(res.w_pc, false, false, "None"))));

        m_card3->setText(QString("Peak Mr: %1 dB (@ %2 r/s)\n-3dB BW: %3 rad/s")
            .arg(QString::fromStdString(PRECISION.fmt(res.peak_mag_db)))
            .arg(QString::fromStdString(PRECISION.fmt(res.w_peak)))
            .arg(QString::fromStdString(PRECISION.fmt(res.w_bw, false, false, "N/A"))));

        m_card4->setText(QString("ω = %1 rad/s\n|H| = %2 dB  |  ∠H = %3°")
            .arg(QString::fromStdString(PRECISION.fmt(res.cursor_w)))
            .arg(QString::fromStdString(PRECISION.fmt(res.cursor_mag_db)))
            .arg(QString::fromStdString(PRECISION.fmt(res.cursor_phase_deg))));

        canvas->update();
        if (notify) model->notify("freq_resp");
    }
};

