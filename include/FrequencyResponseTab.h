#pragma once
#include "BodeNyquistCanvas.h"


class FrequencyResponseTab : public QWidget {
    Q_OBJECT
public:
    SharedSystemModel* model;
    QLineEdit *num_edit, *den_edit, *gain_edit;
    QLabel *poly_title_lbl, *num_lbl, *den_lbl, *gain_lbl, *poly_status_lbl;
    CompensatorConfigCard* comp_card = nullptr;
    FeedbackConfigBar* fb_bar;
    MathFormulaWidget* tf_math_w;
    PoleZeroEditorCard *poles_card, *zeros_card;

    QComboBox* domain_combo;
    QCheckBox* include_pid_chk;
    QLineEdit *w_min_edit, *w_max_edit, *cursor_w_edit;

    QCheckBox *chk_asymp = nullptr, *chk_mirror = nullptr, *chk_marg = nullptr, *chk_ghost = nullptr;

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

        // 1. Transfer Function Polynomial (Num / Den), Compensator Gc(s) & Feedback Editor Card
        auto* poly_card = makeCard();
        auto* poly_l = new QVBoxLayout(poly_card);
        poly_l->setContentsMargins(10, 6, 10, 6);
        poly_l->setSpacing(4);

        auto* poly_hdr = new QHBoxLayout();
        poly_title_lbl = new QLabel("1. Plant  G(s) = K · Num(s) / Den(s)");
        poly_title_lbl->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));
        auto* solve_btn = makeActionBtn("Solve Zeros & Poles ➔", Theme::ACCENT_PID);
        solve_btn->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
        poly_hdr->addWidget(poly_title_lbl, 1);
        poly_hdr->addWidget(solve_btn);
        poly_l->addLayout(poly_hdr);

        auto* pgrid = new QGridLayout();
        pgrid->setHorizontalSpacing(6);
        pgrid->setVerticalSpacing(4);
        num_lbl = new QLabel("Num(s):");
        num_lbl->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));
        num_edit = makeDarkEntry();
        num_edit->setMinimumWidth(50);

        den_lbl = new QLabel("Den(s):");
        den_lbl->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; border:none;").arg(Theme::ACCENT_POLE));
        den_edit = makeDarkEntry();
        den_edit->setMinimumWidth(50);

        gain_lbl = new QLabel("Gain K:");
        gain_lbl->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; border:none;").arg(Theme::TEXT_MUTED));
        gain_edit = makeDarkEntry("1.0", 55);

        poly_status_lbl = new QLabel("Enter polynomials (e.g. s^2+2s+2) and press Enter / Solve");
        poly_status_lbl->setWordWrap(true);
        poly_status_lbl->setStyleSheet(QString("color: %1; font-size: 10px; border:none;").arg(Theme::TEXT_MUTED));

        pgrid->addWidget(num_lbl,  0, 0); pgrid->addWidget(num_edit,  0, 1);
        pgrid->addWidget(den_lbl,  0, 2); pgrid->addWidget(den_edit,  0, 3);
        pgrid->addWidget(gain_lbl, 1, 0); pgrid->addWidget(gain_edit, 1, 1);
        pgrid->addWidget(poly_status_lbl, 1, 2, 1, 2);
        pgrid->setColumnStretch(1, 1);
        pgrid->setColumnStretch(3, 1);
        poly_l->addLayout(pgrid);

        poly_status_lbl = new QLabel("Enter polynomials (e.g. s^2+2s+2 or (s+1)(s+3)) and press Enter / Solve");
        poly_status_lbl->setStyleSheet(QString("color: %1; font-size: 10px; border:none;").arg(Theme::TEXT_MUTED));
        poly_l->addWidget(poly_status_lbl);

        // Editable Poles (×) & Zeros (○) Tables Row
        auto* pz_row = new QHBoxLayout();
        poles_card = new PoleZeroEditorCard("G(s) Poles (×)", Theme::ACCENT_POLE, -1.0);
        zeros_card = new PoleZeroEditorCard("G(s) Zeros (○)", Theme::ACCENT_ZERO, -3.0);
        poles_card->peer_card = zeros_card;
        zeros_card->peer_card = poles_card;
        poles_card->syncHeightWithPeer();
        pz_row->addWidget(poles_card, 1);
        pz_row->addWidget(zeros_card, 1);
        poly_l->addLayout(pz_row);

        // Series Compensator Gc(s) Card (Synced with Root Locus & Time Simulator)
        comp_card = new CompensatorConfigCard();
        poly_l->addWidget(comp_card);

        fb_bar = new FeedbackConfigBar("s", "H");
        poly_l->addWidget(fb_bar);

        tf_math_w = new MathFormulaWidget("", QColor(Theme::ACCENT_ZERO), QColor(Theme::BG_INPUT), 10);
        poly_l->addWidget(tf_math_w);
        left_v->addWidget(poly_card);

        // 2. Frequency Sweep & Loop Mode Configuration Card
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
        domain_combo->setMinimumWidth(140);
        domain_combo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
        domain_combo->setStyleSheet(QString("background-color: %1; color: %2; padding: 3px; font-weight: bold; font-size: 11px;")
                                    .arg(Theme::BG_INPUT, Theme::TEXT_PRIMARY));
        sl->addWidget(domain_combo);

        include_pid_chk = new QCheckBox("Include Tab 2 / Tab 4 PID Controller C(s) / C(z) in Frequency Loop");
        include_pid_chk->setChecked(false);
        include_pid_chk->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 10px; border:none;").arg(Theme::ACCENT_PID));
        sl->addWidget(include_pid_chk);

        auto* w_grid = new QGridLayout();
        w_min_edit    = makeDarkEntry("-2", 55);
        w_max_edit    = makeDarkEntry("2", 55);
        cursor_w_edit = makeDarkEntry("1.0", 70);
        auto addW = [&](int c, const QString& lbl, QWidget* w) {
            auto* l = new QLabel(lbl);
            l->setStyleSheet(QString("color: %1; font-size: 9px; font-weight: bold; border:none;").arg(Theme::TEXT_MUTED));
            w_grid->addWidget(l, 0, c);
            w_grid->addWidget(w, 1, c);
        };
        addW(0, "Min log10(ω)", w_min_edit);
        addW(1, "Max log10(ω)", w_max_edit);
        addW(2, "Cursor ω (rad/s)", cursor_w_edit);
        sl->addLayout(w_grid);
        left_v->addWidget(sys_card);

        // 3. Stability Margins, Bandwidth & Cursor Telemetry Card
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
        btop->addWidget(banner_title);
        btop->addStretch();
        btop->addWidget(banner_right);
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
            auto* l = new QLabel(title);
            l->setStyleSheet(QString("color: %1; font-size: 9px; font-weight: bold; border:none;").arg(Theme::TEXT_MUTED));
            out = new QLabel("—");
            l->setWordWrap(true); out->setWordWrap(true);
            out->setStyleSheet(QString("color: %1; font-family: Monospace; font-size: 10px; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));
            vl->addWidget(l);
            vl->addWidget(out);
            mgrid->addWidget(b, r, c);
        };
        makeM(0, 0, "PHASE MARGIN (PM) & ω_gc", m_card1);
        makeM(0, 1, "GAIN MARGIN (GM) & ω_pc", m_card2);
        makeM(1, 0, "CLOSED-LOOP Mr & -3dB BW", m_card3);
        makeM(1, 1, "CURSOR INSPECTION @ ω", m_card4);
        tl->addLayout(mgrid);
        left_v->addWidget(tel_card);
        left_v->addStretch(1);

        // Right Plot Panel (Bode / Nyquist Switcher + Asymptotes, Mirror, Margin & Ghost Toggles)
        auto* right_card = makeCard();
        auto* rl = new QVBoxLayout(right_card);

        auto* plot_top_row = new QHBoxLayout();
        plot_top_row->setContentsMargins(0, 0, 0, 0);
        auto* pt = new QLabel("Frequency-Domain Bode & Nyquist");
        pt->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 13px; border:none;").arg(Theme::TEXT_PRIMARY));
        auto* fit_b = makeActionBtn("Fit View", Theme::TEXT_PRIMARY, Theme::BG_PANEL);
        fit_b->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
        plot_top_row->addWidget(pt);
        plot_top_row->addStretch();
        plot_top_row->addWidget(fit_b);
        rl->addLayout(plot_top_row);

        auto* rb_bode = new QRadioButton("Bode Plot"); rb_bode->setChecked(true);
        auto* rb_nyq  = new QRadioButton("Nyquist Polar");
        for (auto* rb : {rb_bode, rb_nyq}) {
            rb->setStyleSheet(QString("QRadioButton { color: %1; font-weight: bold; font-size: 11px; border:none; }").arg(Theme::TEXT_PRIMARY));
        }

        chk_asymp  = new QCheckBox("Asymptotes");       chk_asymp->setChecked(true);
        chk_mirror = new QCheckBox("ω < 0 Mirror");     chk_mirror->setChecked(true); chk_mirror->setVisible(false);
        chk_marg   = new QCheckBox("GM/PM & BW");       chk_marg->setChecked(true);
        chk_ghost  = new QCheckBox("👻 Ghost Uncomp."); chk_ghost->setChecked(true);

        chk_asymp->setStyleSheet(QString("QCheckBox { color: %1; font-weight: bold; font-size: 11px; border:none; }").arg(Theme::ACCENT_OPEN));
        chk_mirror->setStyleSheet(QString("QCheckBox { color: %1; font-weight: bold; font-size: 11px; border:none; }").arg(Theme::ACCENT_ASYMP));
        chk_marg->setStyleSheet(QString("QCheckBox { color: %1; font-weight: bold; font-size: 11px; border:none; }").arg(Theme::ACCENT_PID));
        chk_ghost->setStyleSheet(QString("QCheckBox { color: %1; font-weight: bold; font-size: 11px; border:none; }").arg(Theme::TEXT_SECONDARY));

        auto* chk_flow = new FlowLayout(nullptr, 0, 10, 4);
        chk_flow->addWidget(rb_bode);
        chk_flow->addWidget(rb_nyq);
        chk_flow->addWidget(chk_asymp);
        chk_flow->addWidget(chk_mirror);
        chk_flow->addWidget(chk_marg);
        chk_flow->addWidget(chk_ghost);
        rl->addLayout(chk_flow);

        canvas = new BodeNyquistCanvas();
        attachSvgExportMenu(canvas, "ezpz_bode_nyquist.svg");
        attachCanvasZoomShortcuts(this, canvas);
        rl->addWidget(canvas, 1);

        configureMainSplitter(splitter, wrapInLeftScroll(left_w, 365), right_card);

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

        comp_card->onChanged = [this]() {
            if (updating_ui) return;
            comp_card->syncToModelS(model);
            model->syncCompensatorToPidGains();
            syncPidCheckboxState();
            evaluateCurrent(true);
        };

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
            connect(ed, &QLineEdit::returnPressed,   this, [this]() { evaluateCurrent(false); });
        }

        connect(rb_bode, &QRadioButton::toggled, this, [this](bool is_bode) {
            canvas->nyquist_mode = !is_bode;
            bool is_z = (domain_combo->currentIndex() == 2);
            chk_asymp->setVisible(is_bode && !is_z);
            chk_mirror->setVisible(!is_bode);
            canvas->update();
        });
        connect(chk_asymp,  &QCheckBox::toggled, this, [this](bool v) { canvas->show_asymptotes = v; canvas->update(); });
        connect(chk_mirror, &QCheckBox::toggled, this, [this](bool v) { canvas->show_mirror_nyquist = v; canvas->update(); });
        connect(chk_marg,   &QCheckBox::toggled, this, [this](bool v) { canvas->show_margins = v; canvas->update(); });
        connect(chk_ghost,  &QCheckBox::toggled, this, [this](bool v) { canvas->show_ghost = v; canvas->update(); });
        connect(fit_b,      &QPushButton::clicked, this, [this]() { canvas->resetView(); });

        canvas->onFrequencySelected = [this](double w_sel) {
            cursor_w_edit->setText(QString::number(w_sel, 'g', 5));
            evaluateCurrent(false);
        };

        loadFromModel();
    }

    void syncPidCheckboxState() {
        bool is_z = (domain_combo->currentIndex() == 2);
        if (!is_z && model->comp_enabled_s) {
            include_pid_chk->setEnabled(false);
            include_pid_chk->setText("Gc(s) Active in Loop (Uncheck Gc(s) to use PID)");
        } else if (!is_z) {
            include_pid_chk->setEnabled(true);
            include_pid_chk->setText("Include Active Tab 2 PID Controller C(s) in Frequency Loop");
        } else {
            include_pid_chk->setEnabled(true);
            include_pid_chk->setText("Include Active Tab 4 Discrete PID C(z) in Closed-Loop T(e^{jωTs})");
        }
    }

    void applyPolyNumDen() {
        try {
            auto pn = parsePolynomialInput(num_edit->text().toStdString());
            auto pd = parsePolynomialInput(den_edit->text().toStdString());
            if (std::abs(pd.leading_gain) <= 1e-14)
                throw std::runtime_error("Divide by zero: Den cannot be 0.");
            if (std::abs(pn.leading_gain) <= 1e-14)
                throw std::runtime_error("Zero numerator: Num cannot be 0.");

            // Preserve exact leading sign (e.g. Den(s) = -s - 2 -> K = -1)
            double ratio = pn.leading_gain / pd.leading_gain;

            if (domain_combo->currentIndex() != 2) {
                model->zeros = pn.roots;
                model->poles = pd.roots;
                if (std::abs(ratio) > 1e-12) {
                    model->plant_gain = ratio;
                }
                updating_ui = true;
                poles_card->setValues(model->poles);
                zeros_card->setValues(model->zeros);
                num_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->zeros, "s")));
                den_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->poles, "s")));
                gain_edit->setText(QString::number(model->plant_gain, 'g', 5));
                updating_ui = false;

                if (comp_card) comp_card->syncToModelS(model);
                if (fb_bar) fb_bar->syncToModelS(model);

                poly_status_lbl->setText(QString("✔ Solved %1 s-Poles & %2 s-Zeros (K = %3)")
                                         .arg(model->poles.size()).arg(model->zeros.size())
                                         .arg(QString::fromStdString(PRECISION.fmt(model->plant_gain))));
            } else {
                model->z_zeros = pn.roots;
                model->z_poles = pd.roots;
                if (std::abs(ratio - 1.0) > 1e-6 && std::abs(ratio) > 1e-12) {
                    model->filter_gain = ratio;
                }
                updating_ui = true;
                poles_card->setValues(model->z_poles);
                zeros_card->setValues(model->z_zeros);
                num_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->z_zeros, "z")));
                den_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->z_poles, "z")));
                gain_edit->setText(QString::number(model->filter_gain, 'g', 5));
                updating_ui = false;

                if (fb_bar) fb_bar->syncToModelZ(model);

                poly_status_lbl->setText(QString("✔ Solved %1 z-Poles & %2 z-Zeros (Kd = %3)")
                                         .arg(model->z_poles.size()).arg(model->z_zeros.size())
                                         .arg(QString::fromStdString(PRECISION.fmt(model->filter_gain))));
            }
            evaluateCurrent(true);
        } catch (const std::exception& e) {
            poly_status_lbl->setText(QString("⚠ Polynomial error: %1").arg(e.what()));
        }
    }

    void loadFromModel() {
        updating_ui = true;
        bool is_z = (domain_combo->currentIndex() == 2);
        w_min_edit->setEnabled(!is_z);
        w_max_edit->setEnabled(!is_z);
        if (chk_asymp) chk_asymp->setVisible(!canvas->nyquist_mode && !is_z);

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

            if (comp_card) {
                comp_card->setVisible(true);
                comp_card->loadFromModelS(model);
            }
            fb_bar->domain_var = "s";
            fb_bar->unity_rb->setText("Unity (H = 1)");
            fb_bar->custom_rb->setText("Custom H(s)");
            fb_bar->view_mode_btn->setText(fb_bar->show_closed_loop_ts ? "Formula: Closed-Loop T(s)" : "Formula: G(s), H(s)");
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

            if (comp_card) comp_card->setVisible(false);
            fb_bar->domain_var = "z";
            fb_bar->unity_rb->setText("Unity (H_fb = 1)");
            fb_bar->custom_rb->setText("Custom H_fb(z)");
            fb_bar->view_mode_btn->setText(fb_bar->show_closed_loop_ts ? "Formula: Closed-Loop T(z)" : "Formula: H(z), H_fb(z)");
            fb_bar->loadFromModelZ(model);
        }
        syncPidCheckboxState();
        updating_ui = false;
        evaluateCurrent(false);
    }

    void evaluateCurrent(bool notify = false) {
        if (updating_ui) return;
        int m_idx = domain_combo->currentIndex();
        bool pid_on = include_pid_chk->isChecked();
        double wmin_e = parseNumericInput(w_min_edit->text().toStdString(), -2.0);
        double wmax_e = parseNumericInput(w_max_edit->text().toStdString(),  2.0);
        if (wmax_e <= wmin_e) wmax_e = wmin_e + 2.0;
        double cw = std::max(1e-6, parseNumericInput(cursor_w_edit->text().toStdString(), 1.0));

        // Honor FeedbackConfigBar's G(s), H(s) <-> Closed-Loop T(s) formula toggle & Gc(s) display
        if (m_idx != 2) {
            tf_math_w->setLatex(QString::fromStdString(fb_bar->formatActiveLatexS(model, pid_on)));
        } else {
            tf_math_w->setLatex(QString::fromStdString(fb_bar->formatActiveLatexZ(model)));
        }

        auto res = FrequencyDomainEngine::evaluate(model, m_idx, pid_on, wmin_e, wmax_e, cw);
        canvas->freq_res = res;

        // Context-aware Stability Banner & Telemetry Cards
        QString bg = Theme::BANNER_OK_BG, bd = Theme::BANNER_OK_BD, fg = Theme::BANNER_OK_FG;

        if (m_idx != 2) {
            bool pm_ok = !res.pm_deg.has_value() || (res.pm_deg.value() > 0.0);
            bool gm_ok = !res.gm_db.has_value()  || (res.gm_db.value() > 0.0);

            if (!res.cl_stable) {
                bg = Theme::BANNER_ERR_BG; bd = Theme::BANNER_ERR_BD; fg = Theme::BANNER_ERR_FG;
                banner_title->setText(QString("✖ CLOSED-LOOP UNSTABLE (%1 RHP Pole%2)")
                                      .arg(res.cl_rhp_poles).arg(res.cl_rhp_poles == 1 ? "" : "s"));
            } else if (res.ol_rhp_poles > 0) {
                bg = Theme::BANNER_WARN_BG; bd = Theme::BANNER_WARN_BD; fg = Theme::BANNER_WARN_FG;
                banner_title->setText(QString("✔ CL STABLE VIA FEEDBACK (Open-Loop P = %1 RHP Pole%2)")
                                      .arg(res.ol_rhp_poles).arg(res.ol_rhp_poles == 1 ? "" : "s"));
            } else if (!pm_ok || !gm_ok) {
                bg = Theme::BANNER_WARN_BG; bd = Theme::BANNER_WARN_BD; fg = Theme::BANNER_WARN_FG;
                banner_title->setText("⚠ CRITICAL / MARGINAL STABILITY MARGINS");
            } else {
                banner_title->setText(m_idx == 0 ? "✔ CLOSED-LOOP STABLE & POSITIVE MARGINS"
                                                 : "✔ CLOSED-LOOP T(jω) STABLE");
            }

            if (m_idx == 0) {
                banner_right->setText(QString("PM = %1° | GM = %2 dB")
                    .arg(QString::fromStdString(PRECISION.fmt(res.pm_deg, false, false, "∞")))
                    .arg(QString::fromStdString(PRECISION.fmt(res.gm_db, false, false, "∞"))));
            } else {
                banner_right->setText(QString("Mr = %1 dB | BW = %2 r/s")
                    .arg(QString::fromStdString(PRECISION.fmt(res.mr_db)))
                    .arg(QString::fromStdString(PRECISION.fmt(res.w_bw, false, false, "N/A"))));
            }

            std::string corners_s;
            for (size_t i = 0; i < std::min<size_t>(4, res.corner_labels.size()); ++i) {
                corners_s += (i ? ", " : "") + res.corner_labels[i];
            }
            if (res.corner_labels.size() > 4) corners_s += ", …";

            std::string dc_str;
            if (res.dc_gain_db.has_value()) {
                dc_str = PRECISION.fmt(res.dc_gain_db.value()) + " dB";
            } else if (res.n_pole_orig > 0) {
                dc_str = "+∞ dB (Type " + std::to_string(res.n_pole_orig) + ")";
            } else {
                dc_str = "-∞ dB (Zero @ s=0)";
            }

            QString ghost_note;
            if (res.has_ghost && (res.ghost_pm_deg.has_value() || res.ghost_gm_db.has_value())) {
                ghost_note = QString("   •   Uncomp: PM=%1°, GM=%2 dB")
                    .arg(QString::fromStdString(PRECISION.fmt(res.ghost_pm_deg, false, false, "∞")))
                    .arg(QString::fromStdString(PRECISION.fmt(res.ghost_gm_db, false, false, "∞")));
            }

            banner_sub->setText(QString("Corners ωc: [%1]   •   DC Gain: %2%3")
                .arg(QString::fromStdString(corners_s.empty() ? "None" : corners_s))
                .arg(QString::fromStdString(dc_str))
                .arg(ghost_note));

            QString gc_multi = (res.num_gc_crossings > 1) ? QString(" (%1×)").arg(res.num_gc_crossings) : "";
            QString pc_multi = (res.num_pc_crossings > 1) ? QString(" (%1×)").arg(res.num_pc_crossings) : "";

            m_card1->setText(QString("PM: %1°%2\n@ ω_gc = %3 rad/s")
                .arg(QString::fromStdString(PRECISION.fmt(res.pm_deg, false, false, "∞")))
                .arg(gc_multi)
                .arg(QString::fromStdString(PRECISION.fmt(res.w_gc, false, false, "None"))));

            m_card2->setText(QString("GM: %1 dB%2\n@ ω_pc = %3 rad/s")
                .arg(QString::fromStdString(PRECISION.fmt(res.gm_db, false, false, "∞")))
                .arg(pc_multi)
                .arg(QString::fromStdString(PRECISION.fmt(res.w_pc, false, false, "None"))));

            m_card3->setText(QString("CL Mr: %1 dB (@ %2 r/s)\n-3dB BW: %3 rad/s")
                .arg(QString::fromStdString(PRECISION.fmt(res.mr_db)))
                .arg(QString::fromStdString(PRECISION.fmt(res.w_peak)))
                .arg(QString::fromStdString(PRECISION.fmt(res.w_bw, false, false, "N/A"))));
        } else {
            // Discrete Mode Banner & Telemetry
            double w_nyq = PI / std::max(model->ts, 1e-5);
            if (!res.cl_stable) {
                bg = Theme::BANNER_ERR_BG; bd = Theme::BANNER_ERR_BD; fg = Theme::BANNER_ERR_FG;
                banner_title->setText(QString("✖ UNSTABLE DIGITAL FILTER (max|z| = %1 ≥ 1)")
                                      .arg(QString::fromStdString(PRECISION.fmt(res.max_z_pole_mag))));
            } else {
                banner_title->setText(QString("✔ STABLE DIGITAL FILTER (max|z| = %1 < 1)")
                                      .arg(QString::fromStdString(PRECISION.fmt(res.max_z_pole_mag))));
            }
            banner_right->setText(QString("ω_N = %1 r/s (Ts = %2 s)")
                .arg(QString::fromStdString(PRECISION.fmt(w_nyq)))
                .arg(QString::fromStdString(PRECISION.fmt(model->ts))));

            double nyq_db = res.mag_db.empty() ? 0.0 : res.mag_db.back();
            banner_sub->setText(QString("DC Gain |H(1)|: %1 dB   •   Nyquist Gain |H(-1)|: %2 dB")
                .arg(QString::fromStdString(PRECISION.fmt(res.dc_gain_db, false, false, "-∞")))
                .arg(QString::fromStdString(PRECISION.fmt(nyq_db))));

            m_card1->setText(QString("DC Gain (ω=0): %1 dB\nNyquist (π/Ts): %2 dB")
                .arg(QString::fromStdString(PRECISION.fmt(res.dc_gain_db, false, false, "-∞")))
                .arg(QString::fromStdString(PRECISION.fmt(nyq_db))));

            m_card2->setText(QString("Nyquist ω_N: %1 r/s\nf_s/2 = %2 Hz")
                .arg(QString::fromStdString(PRECISION.fmt(w_nyq)))
                .arg(QString::fromStdString(PRECISION.fmt(0.5 / std::max(model->ts, 1e-5)))));

            m_card3->setText(QString("Peak: %1 dB (@ %2 r/s)\n-3dB Cutoff: %3 rad/s")
                .arg(QString::fromStdString(PRECISION.fmt(res.peak_mag_db)))
                .arg(QString::fromStdString(PRECISION.fmt(res.w_peak)))
                .arg(QString::fromStdString(PRECISION.fmt(res.w_bw, false, false, "N/A"))));
        }

        const char* sym = (m_idx == 0) ? "L" : ((m_idx == 1) ? "T" : "H");
        m_card4->setText(QString("ω = %1 rad/s\n|%2| = %3 dB  |  ∠%2 = %4°")
            .arg(QString::fromStdString(PRECISION.fmt(res.cursor_w)))
            .arg(sym)
            .arg(QString::fromStdString(PRECISION.fmt(res.cursor_mag_db)))
            .arg(QString::fromStdString(PRECISION.fmt(res.cursor_phase_deg))));

        margin_banner->setStyleSheet(QString("QFrame { background-color: %1; border: 1.5px solid %2; border-radius: 5px; }").arg(bg, bd));
        banner_title->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border:none;").arg(fg));
        banner_right->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; font-size: 11px; border:none;").arg(Theme::TEXT_PRIMARY));
        banner_sub->setStyleSheet(QString("color: %1; font-size: 10px; border:none;").arg(Theme::TEXT_SECONDARY));

        canvas->update();
        if (notify) model->notify("freq_resp");
    }
};