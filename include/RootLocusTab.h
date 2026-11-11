#pragma once
#include "DesignParametersDialog.h"


class RootLocusTab : public QWidget {
    Q_OBJECT
public:
    SharedSystemModel* model;
    std::function<void(double)> onSendGainToPid;

    QLineEdit *num_edit, *den_edit, *tp_v1_edit, *tp_v2_edit, *tol_edit;
    QLabel *tp_lbl1, *tp_lbl2, *poly_status_lbl;
    MathFormulaWidget* tf_math_w;
    PoleZeroEditorCard *poles_card, *zeros_card;
    QRadioButton *cart_radio, *polar_radio;
    QCheckBox* chk_tp_show = nullptr;
    QCheckBox* chk_ghost_rl = nullptr;
    std::vector<Complex> saved_uncomp_poles;
    std::vector<Complex> saved_uncomp_zeros;
    double saved_uncomp_gain = 1.0;
    bool is_polar_mode = false;

    QFrame* rl_banner;
    QLabel *banner_title, *banner_right, *banner_sub;
    QLabel *val_angle, *val_mag, *val_gain;
    ComplexPlaneCanvas* canvas;
    QCheckBox *chk_ts_spec, *chk_tp_spec, *chk_zeta_spec, *chk_live_k;
    QLineEdit *edit_ts_spec, *edit_tp_spec, *edit_zeta_spec, *edit_live_k;
    QSlider *slider_live_k;
    QLabel *live_k_poles_lbl;

    QLineEdit* edit_max_k;
    std::map<std::string, bool> rl_lock_map = {
        {"kp", false}, {"kd", true}, {"ki", true}, {"os_pct", true},
        {"zeta", false}, {"wn", false}, {"ts", false}, {"tp", false},
        {"tr", false}, {"sigma_d", false}, {"omega_d", false}, {"ess", false}, {"rel_err_pct", false}
    };
    std::map<std::string, double> rl_input_map = {
        {"kp", 1.5}, {"kd", 0.0}, {"ki", 0.0}, {"os_pct", 20.0},
        {"zeta", 0.456}, {"wn", 5.48}, {"ts", 1.60}, {"tp", 0.64},
        {"tr", 0.34}, {"sigma_d", 2.50}, {"omega_d", 4.88}, {"ess", 0.20}, {"rel_err_pct", 20.0}
    };

    double last_gain = 1.0;
    bool updating_ui = false;
    FeedbackConfigBar* fb_bar;

    QComboBox *locus_param_combo = nullptr, *vary_pole_combo = nullptr;
    QLineEdit *edit_fixed_k0 = nullptr;
    QLabel *lbl_vary_pole = nullptr, *lbl_fixed_k0 = nullptr, *gain_title_lbl = nullptr;

    CompensatorConfigCard* comp_card = nullptr;

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
        left_v->setSpacing(6);

        // 1. Num(s) / Den(s) Polynomial Auto-Solver Card
        auto* poly_card = makeCard();
        auto* poly_l = new QVBoxLayout(poly_card);
        poly_l->setContentsMargins(10, 8, 10, 8);
        auto* poly_hdr = new QHBoxLayout();
        auto* title_l = new QLabel("Transfer Function  G(s) = Num(s) / Den(s)");
        title_l->setStyleSheet(QString("color: %1; font-weight: bold; border: none;").arg(Theme::ACCENT_ZERO));
        auto* solve_btn = makeActionBtn("Solve Zeros & Poles ➔", Theme::ACCENT_PID);
        poly_hdr->addWidget(title_l); poly_hdr->addStretch(); poly_hdr->addWidget(solve_btn);
        poly_l->addLayout(poly_hdr);

        auto* pgrid = new QGridLayout();
        auto* nl = new QLabel("Num(s):"); nl->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; border: none;").arg(Theme::ACCENT_ZERO));
        num_edit = makeDarkEntry(QString::fromStdString(formatPolynomialFromRoots(model->zeros, "s")));
        auto* dl = new QLabel("Den(s):"); dl->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; border: none;").arg(Theme::ACCENT_POLE));
        den_edit = makeDarkEntry(QString::fromStdString(formatPolynomialFromRoots(model->poles, "s")));
        pgrid->addWidget(nl, 0, 0); pgrid->addWidget(num_edit, 0, 1);
        pgrid->addWidget(dl, 0, 2); pgrid->addWidget(den_edit, 0, 3);
        poly_l->addLayout(pgrid);

        // poly_status_lbl = new QLabel("Enter s-polynomials (e.g. s^2+2s+2 or (s+1)(s+3)) and press Enter / Solve");
        // poly_status_lbl->setStyleSheet(QString("color: %1; font-size: 10px; border: none;").arg(Theme::TEXT_SECONDARY));
        // poly_l->addWidget(poly_status_lbl);

        // Editable G(s) Poles (×) & G(s) Zeros (○) Tables directly under G(s) Num/Den
        auto* pz_row = new QHBoxLayout();
        pz_row->setContentsMargins(0, 2, 0, 2);
        pz_row->setSpacing(6);
        poles_card = new PoleZeroEditorCard("G(s) Poles (×)", Theme::ACCENT_POLE, -1.0);
        zeros_card = new PoleZeroEditorCard("G(s) Zeros (○)", Theme::ACCENT_ZERO, -3.0);
        poles_card->peer_card = zeros_card;
        zeros_card->peer_card = poles_card;
        poles_card->syncHeightWithPeer();
        pz_row->addWidget(poles_card, 1);
        pz_row->addWidget(zeros_card, 1);
        poly_l->addLayout(pz_row);

        // Feedback Path H(s) (with its own collapsible H(s) Poles & Zeros tables)
        fb_bar = new FeedbackConfigBar("s", "H");
        poly_l->addWidget(fb_bar);
        fb_bar->onChanged = [this]() {
            if (updating_ui) return;
            fb_bar->syncToModelS(model);
            evaluateCurrent(true);
        };

        // Combined LaTeX Transfer Function Display at the bottom of the card
        tf_math_w = new MathFormulaWidget("", QColor(Theme::ACCENT_ZERO), QColor(Theme::BG_INPUT), 10);
        poly_l->addWidget(tf_math_w);
        left_v->addWidget(poly_card);

        // 2. Controller / Compensator Gc(s) Block (Poles, Zeros, Gain Kc & Enable Toggle)
        comp_card = new CompensatorConfigCard();
        left_v->addWidget(comp_card);
        comp_card->onChanged = [this]() {
            if (updating_ui) return;
            comp_card->syncToModelS(model);
            model->syncCompensatorToPidGains(); // <-- Converts Gc(s) into Kp, Ki, Kd for Time Domain!
            if (model->comp_enabled_s && chk_ghost_rl && !chk_ghost_rl->isChecked()) {
                updating_ui = true;
                chk_ghost_rl->setChecked(true);
                updating_ui = false;
            }
            evaluateCurrent(true);
        };

        // 3B. s-Plane Design Specs (Ts, Tp, zeta) & Live Gain K Pole Explorer Card
        auto* spec_card = makeCard();
        auto* spec_l = new QVBoxLayout(spec_card);
        spec_l->setContentsMargins(10, 6, 10, 6);
        spec_l->setSpacing(4);

        auto* spec_title = new QLabel("s-Plane Spec Lines (Ts, Tp, ζ) & Live Gain K Explorer");
        spec_title->setStyleSheet(QString("color: %1; font-weight: bold; border: none;").arg(Theme::ACCENT_ASYMP));
        spec_l->addWidget(spec_title);

        auto* sp_grid = new QGridLayout();
        chk_ts_spec   = new QCheckBox("Ts (s):");
        chk_ts_spec->setStyleSheet("color: #38bdf8; font-weight: bold; font-size: 10px;");
        edit_ts_spec  = makeDarkEntry("2.0", 48);
        chk_tp_spec   = new QCheckBox("Tp (s):");
        chk_tp_spec->setStyleSheet("color: #f472b6; font-weight: bold; font-size: 10px;");
        edit_tp_spec  = makeDarkEntry("1.0", 48);
        chk_zeta_spec = new QCheckBox("ζ Ray:");
        chk_zeta_spec->setStyleSheet("color: #facc15; font-weight: bold; font-size: 10px;");
        edit_zeta_spec = makeDarkEntry("0.707", 48);

        sp_grid->addWidget(chk_ts_spec,   0, 0); sp_grid->addWidget(edit_ts_spec,   0, 1);
        sp_grid->addWidget(chk_tp_spec,   0, 2); sp_grid->addWidget(edit_tp_spec,   0, 3);
        sp_grid->addWidget(chk_zeta_spec, 0, 4); sp_grid->addWidget(edit_zeta_spec, 0, 5);
        spec_l->addLayout(sp_grid);

        auto* mode_row = new QHBoxLayout();
        auto* mode_lbl = new QLabel("Sweep Parameter:");
        mode_lbl->setStyleSheet("color: #94a3b8; font-size: 10px; font-weight: bold; border:none;");
        locus_param_combo = new QComboBox();
        locus_param_combo->addItems({
            "Vary Loop Gain K  (Standard Root Locus)",
            "Vary Pole Location s = -a  (Fixed Gain K₀)"
        });
        locus_param_combo->setStyleSheet(QString("background-color: %1; color: %2; padding: 2px 4px; font-weight: bold; font-size: 10px;")
                                         .arg(Theme::BG_INPUT, Theme::TEXT_PRIMARY));

        lbl_vary_pole = new QLabel("Pole:");
        lbl_vary_pole->setStyleSheet("color: #f472b6; font-size: 10px; font-weight: bold; border:none;");
        vary_pole_combo = new QComboBox();
        vary_pole_combo->setStyleSheet(QString("background-color: %1; color: %2; padding: 2px 4px; font-family: Monospace; font-size: 10px;")
                                       .arg(Theme::BG_INPUT, Theme::TEXT_PRIMARY));

        lbl_fixed_k0 = new QLabel("Fixed K₀:");
        lbl_fixed_k0->setStyleSheet("color: #38bdf8; font-size: 10px; font-weight: bold; border:none;");
        edit_fixed_k0 = makeDarkEntry("10.0", 48);

        lbl_vary_pole->setVisible(false);
        vary_pole_combo->setVisible(false);
        lbl_fixed_k0->setVisible(false);
        edit_fixed_k0->setVisible(false);

        mode_row->addWidget(mode_lbl);
        mode_row->addWidget(locus_param_combo, 1);
        mode_row->addWidget(lbl_vary_pole);
        mode_row->addWidget(vary_pole_combo);
        mode_row->addWidget(lbl_fixed_k0);
        mode_row->addWidget(edit_fixed_k0);
        spec_l->addLayout(mode_row);

        auto* k_row = new QHBoxLayout();
        chk_live_k = new QCheckBox("Vary Gain K (×/○ CL):");
        chk_live_k->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 10px;").arg(Theme::ACCENT_PID));
        edit_live_k = makeDarkEntry("1.0", 50);
        slider_live_k = new QSlider(Qt::Horizontal);
        slider_live_k->setRange(0, 1000);
        slider_live_k->setValue(100);
        auto* max_k_lbl = new QLabel("Max K:");
        max_k_lbl->setStyleSheet("color: #94a3b8; font-size: 10px; font-weight: bold; border:none;");
        edit_max_k = makeDarkEntry(QString::number(model->max_k_slider, 'g', 4), 48);

        k_row->addWidget(chk_live_k);
        k_row->addWidget(edit_live_k);
        k_row->addWidget(slider_live_k, 1);
        k_row->addWidget(max_k_lbl);
        k_row->addWidget(edit_max_k);
        spec_l->addLayout(k_row);

        auto* d_row = new QHBoxLayout();
        auto* rl_params_btn = makeActionBtn("Design Parameters...", Theme::ACCENT_ZERO);
        d_row->addWidget(rl_params_btn);
        d_row->addStretch();
        spec_l->addLayout(d_row);

        live_k_poles_lbl = new QLabel("Enable Ts, Tp, ζ to view RL intersections (K, ωn) or click Design Parameters.");
        live_k_poles_lbl->setStyleSheet("color: #94a3b8; font-family: Monospace; font-size: 10px; border: none;");
        live_k_poles_lbl->setWordWrap(true);
        spec_l->addWidget(live_k_poles_lbl);

        left_v->addWidget(spec_card);

        // 3. Test Point s & Cartesian / Polar Switcher Card
        auto* tp_card = makeCard();
        auto* tp_l = new QVBoxLayout(tp_card);
        tp_l->setContentsMargins(10, 8, 10, 8);
        auto* tp_hdr = new QHBoxLayout();
        auto* tp_title = new QLabel("Test Point  s  &  Angle Tolerance");
        tp_title->setStyleSheet(QString("color: %1; font-weight: bold; border: none;").arg(Theme::ACCENT_PID));
        cart_radio  = new QRadioButton("Cartesian"); cart_radio->setChecked(true);
        polar_radio = new QRadioButton("Polar");
        for (auto* rb : {cart_radio, polar_radio}) {
            rb->setStyleSheet(QString("QRadioButton { color: %1; font-weight: bold; font-size: 11px; border: none; }").arg(Theme::TEXT_PRIMARY));
        }
        tp_hdr->addWidget(tp_title); tp_hdr->addStretch(); tp_hdr->addWidget(cart_radio); tp_hdr->addWidget(polar_radio);
        tp_l->addLayout(tp_hdr);

        auto* tp_grid = new QGridLayout();
        tp_lbl1 = new QLabel("Real (σ)"); tp_lbl1->setStyleSheet("color: #94a3b8; font-size: 10px; font-weight: bold; border:none;");
        tp_lbl2 = new QLabel("Imag (jω)"); tp_lbl2->setStyleSheet("color: #94a3b8; font-size: 10px; font-weight: bold; border:none;");
        auto* l_tl = new QLabel("Tol (±°)"); l_tl->setStyleSheet("color: #94a3b8; font-size: 10px; font-weight: bold; border:none;");
        tp_v1_edit = makeDarkEntry(QString::number(model->test_point.real()));
        tp_v2_edit = makeDarkEntry(QString::number(model->test_point.imag()));
        tol_edit   = makeDarkEntry(QString::number(model->rl_tol_deg), 60);
        tp_grid->addWidget(tp_lbl1, 0, 0); tp_grid->addWidget(tp_v1_edit, 1, 0);
        tp_grid->addWidget(tp_lbl2, 0, 1); tp_grid->addWidget(tp_v2_edit, 1, 1);
        tp_grid->addWidget(l_tl,    0, 2); tp_grid->addWidget(tol_edit,   1, 2);
        tp_l->addLayout(tp_grid);
        left_v->addWidget(tp_card);

        // 4. Evaluation Results Card (Turns Orange when OFF Root Locus!)
        auto* res_card = makeCard();
        auto* res_l = new QVBoxLayout(res_card);
        res_l->setContentsMargins(10, 8, 10, 8);
        auto* res_hdr = new QHBoxLayout();
        auto* res_title = new QLabel("Evaluation Results");
        res_title->setStyleSheet(QString("color: %1; font-weight: bold; border: none;").arg(Theme::TEXT_PRIMARY));
        auto* send_btn = makeActionBtn("Send Gain K → PID Simulator ➔", Theme::ACCENT_PID);
        res_hdr->addWidget(res_title); res_hdr->addStretch(); res_hdr->addWidget(send_btn);
        res_l->addLayout(res_hdr);

        rl_banner = new QFrame();
        rl_banner->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        auto* b_l = new QVBoxLayout(rl_banner);
        b_l->setContentsMargins(10, 6, 10, 6);
        b_l->setSpacing(2);
        auto* b_top = new QHBoxLayout();
        banner_title = new QLabel("✔ ON ROOT LOCUS");
        banner_right = new QLabel("Δθ = 0.00°");
        b_top->addWidget(banner_title); b_top->addStretch(); b_top->addWidget(banner_right);
        banner_sub = new QLabel("Satisfies ∠G(s) = ±180°(2k+1)");
        banner_sub->setWordWrap(true);
        banner_sub->setStyleSheet(QString("color: %1; font-size: 10px; font-weight: bold; border:none;").arg(Theme::BANNER_OK_FG));
        b_l->addLayout(b_top);
        b_l->addWidget(banner_sub);
        res_l->addWidget(rl_banner);

        auto* m_grid = new QHBoxLayout();
        auto makeMetric = [&](const QString& lbl, QLabel*& out_lbl, QLabel** out_title = nullptr) {
            auto* box = makeCard();
            box->setStyleSheet(QString("background-color: %1; border: 1px solid %2; border-radius: 4px; padding: 5px;").arg(Theme::BG_INPUT, Theme::BORDER_INPUT));
            auto* vl = new QVBoxLayout(box);
            vl->setContentsMargins(6, 4, 6, 4);
            auto* t = new QLabel(lbl);
            t->setStyleSheet("color: #94a3b8; font-size: 9px; font-weight: bold; border:none;");
            if (out_title) *out_title = t;
            out_lbl = new QLabel("—");
            out_lbl->setStyleSheet(QString("color: %1; font-family: Monospace; font-size: 12px; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));
            vl->addWidget(t);
            vl->addWidget(out_lbl);
            m_grid->addWidget(box);
        };
        makeMetric("NET ANGLE (deg)", val_angle);
        makeMetric("MAGNITUDE |G(s)|", val_mag);
        makeMetric("GAIN K = 1/|G(s)|", val_gain, &gain_title_lbl);
        res_l->addLayout(m_grid);
        left_v->addWidget(res_card);
        left_v->addStretch();

        // Right Plot Canvas
        auto* right_card = makeCard();
        auto* right_l = new QVBoxLayout(right_card);
        auto* plot_hdr = new QHBoxLayout();
        auto* plot_title = new QLabel("Complex s-Plane Root Locus Diagram");
        plot_title->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 13px; border: none;").arg(Theme::TEXT_PRIMARY));
        auto* chk_rl  = new QCheckBox("Root Locus");   chk_rl->setChecked(true);  chk_rl->setStyleSheet("color: #a855f7; font-weight: bold;");
        auto* chk_as  = new QCheckBox("Asymptotes");   chk_as->setChecked(true);  chk_as->setStyleSheet("color: #22d3ee; font-weight: bold;");
        auto* chk_brk = new QCheckBox("Breakaway");    chk_brk->setChecked(true); chk_brk->setStyleSheet("color: #fb923c; font-weight: bold;");
        auto* chk_jw  = new QCheckBox("jω Crossings"); chk_jw->setChecked(true);  chk_jw->setStyleSheet("color: #38bdf8; font-weight: bold;");
        auto* chk_dep = new QCheckBox("Dep/Arr ∠");    chk_dep->setChecked(true); chk_dep->setStyleSheet("color: #f472b6; font-weight: bold;");
        chk_tp_show   = new QCheckBox("Test Point");   chk_tp_show->setChecked(true); chk_tp_show->setStyleSheet("color: #facc15; font-weight: bold;");
        auto* fit_btn = makeActionBtn("Fit View", Theme::TEXT_PRIMARY, Theme::BG_PANEL);
        chk_ghost_rl  = new QCheckBox("Ghost RL");  chk_ghost_rl->setChecked(false); chk_ghost_rl->setStyleSheet("color: #94a3b8; font-weight: bold;");
        chk_ghost_rl->setToolTip("Pin current Root Locus as a dashed uncompensated reference curve, or compare before/after compensation.");
        plot_hdr->addWidget(plot_title); plot_hdr->addStretch();
        plot_hdr->addWidget(chk_ghost_rl);
        plot_hdr->addWidget(chk_rl); plot_hdr->addWidget(chk_as);
        plot_hdr->addWidget(chk_brk); plot_hdr->addWidget(chk_jw);
        plot_hdr->addWidget(chk_dep); plot_hdr->addWidget(chk_tp_show);
        plot_hdr->addWidget(fit_btn);
        right_l->addLayout(plot_hdr);

        canvas = new ComplexPlaneCanvas(false);
        attachSvgExportMenu(canvas, "s_plane_root_locus.svg");
        right_l->addWidget(canvas, 1);

        splitter->addWidget(wrapInLeftScroll(left_w, 380));
        splitter->addWidget(right_card);
        splitter->setStretchFactor(0, 0);
        splitter->setStretchFactor(1, 1);

        // Connections
        connect(solve_btn, &QPushButton::clicked, this, &RootLocusTab::applyPolyNumDen);
        connect(num_edit, &QLineEdit::returnPressed, this, &RootLocusTab::applyPolyNumDen);
        connect(den_edit, &QLineEdit::returnPressed, this, &RootLocusTab::applyPolyNumDen);

        poles_card->onValuesChanged = [this]() {
            if (updating_ui) return;
            model->poles = poles_card->getValues();
            den_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->poles, "s")));
            evaluateCurrent(true);
            refreshVaryPoleCombo();
        };
        zeros_card->onValuesChanged = [this]() {
            if (updating_ui) return;
            model->zeros = zeros_card->getValues();
            num_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->zeros, "s")));
            evaluateCurrent(true);
        };

        connect(cart_radio, &QRadioButton::toggled, this, [this](bool checked) {
            if (updating_ui) return;
            onCoordModeSwitch(!checked);
        });

        auto evalFn = [this]() { if (!updating_ui) evaluateCurrent(false); };
        connect(tp_v1_edit, &QLineEdit::textEdited, this, evalFn);
        connect(tp_v2_edit, &QLineEdit::textEdited, this, evalFn);
        connect(tol_edit,   &QLineEdit::textEdited, this, evalFn);
        connect(chk_rl, &QCheckBox::toggled, this, [this](bool v) { canvas->show_locus = v; canvas->update(); });
        connect(chk_as, &QCheckBox::toggled, this, [this](bool v) { canvas->show_asymp = v; canvas->update(); });
        connect(fit_btn, &QPushButton::clicked, this, [this]() { canvas->resetView(); });
        connect(chk_brk, &QCheckBox::toggled, this, [this](bool v) { canvas->show_breakaway = v; canvas->update(); });
        connect(chk_jw,  &QCheckBox::toggled, this, [this](bool v) { canvas->show_jw_crossings = v; canvas->update(); });
        connect(chk_dep, &QCheckBox::toggled, this, [this](bool v) { canvas->show_dep_arr_angles = v; canvas->update(); });
        connect(chk_tp_show, &QCheckBox::toggled, this, [this, tp_card, res_card](bool v) {
            canvas->show_test_point = v;
            tp_card->setEnabled(v);
            res_card->setEnabled(v);
            canvas->update();
        });
        connect(chk_ghost_rl, &QCheckBox::toggled, this, [this](bool v) {
            canvas->show_ghost_locus = v;
            if (v) {
                canvas->ghost_poles = model->uncompensatedLoopPolesS();
                canvas->ghost_zeros = model->uncompensatedLoopZerosS();
            }
            canvas->update();
        });
        connect(send_btn, &QPushButton::clicked, this, [this]() {
            if (!std::isfinite(last_gain) || last_gain < 0.0) return;

            int vary_idx = (vary_pole_combo && vary_pole_combo->currentData().isValid())
                ? vary_pole_combo->currentData().toInt()
                : -1;
            bool is_pole_sweep = (locus_param_combo && locus_param_combo->currentIndex() == 1 &&
                                  vary_idx >= 0 && vary_idx < static_cast<int>(model->poles.size()));

            if (is_pole_sweep) {
                double fixed_k0 = edit_fixed_k0
                    ? std::max(0.0, parseNumericInput(edit_fixed_k0->text().toStdString(), 10.0))
                    : 10.0;

                // Update the selected real open-loop pole to s = -a (where a = last_gain)
                model->poles[vary_idx] = Complex(-last_gain, 0.0);
                model->plant_gain = fixed_k0;

                // Refresh UI widgets in RootLocusTab to reflect the updated pole
                updating_ui = true;
                poles_card->setValues(model->poles);
                den_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->poles, "s")));
                refreshVaryPoleCombo();
                updating_ui = false;
                evaluateCurrent(true);

                // Send fixed_k0 to PID Simulator and sync tabs
                if (onSendGainToPid) onSendGainToPid(fixed_k0);
            } else {
                // If user is in Vary Pole mode with no real poles, don't accidentally send K
                if (locus_param_combo && locus_param_combo->currentIndex() == 1 && vary_idx < 0) return;

                if (model->comp_enabled_s) {
                    model->c_gain = last_gain;
                    model->syncCompensatorToPidGains();
                    if (comp_card) comp_card->loadFromModelS(model);
                    if (onSendGainToPid) onSendGainToPid(model->kp);
                } else {
                    if (onSendGainToPid) onSendGainToPid(last_gain);
                }
            }
        });

        canvas->onPointSelected = [this](Complex pt) {
            if (chk_tp_show && !chk_tp_show->isChecked()) return;
            updating_ui = true;
            if (!is_polar_mode) {
                tp_v1_edit->setText(QString::number(pt.real(), 'g', 7));
                tp_v2_edit->setText(QString::number(pt.imag(), 'g', 7));
            } else {
                tp_v1_edit->setText(QString::number(std::abs(pt), 'g', 7));
                tp_v2_edit->setText(QString::number(std::arg(pt) * 180.0 / PI, 'g', 7));
            }
            updating_ui = false;
            evaluateCurrent(false);
        };

        auto updateSpecsAndK = [this]() {
            if (updating_ui) return;
            evaluateCurrent(false);
        };
        connect(chk_ts_spec,   &QCheckBox::toggled, this, updateSpecsAndK);
        connect(chk_tp_spec,   &QCheckBox::toggled, this, updateSpecsAndK);
        connect(chk_zeta_spec, &QCheckBox::toggled, this, updateSpecsAndK);
        connect(chk_live_k,    &QCheckBox::toggled, this, updateSpecsAndK);

        connect(edit_ts_spec,   &QLineEdit::textEdited, this, updateSpecsAndK);
        connect(edit_tp_spec,   &QLineEdit::textEdited, this, updateSpecsAndK);
        connect(edit_zeta_spec, &QLineEdit::textEdited, this, updateSpecsAndK);

        connect(edit_max_k, &QLineEdit::editingFinished, this, [this]() {
            model->max_k_slider = std::max(1.0, parseNumericInput(edit_max_k->text().toStdString(), 100.0));
            evaluateCurrent(false);
        });

        connect(slider_live_k, &QSlider::valueChanged, this, [this](int val) {
            if (updating_ui) return;
            updating_ui = true;
            double max_k = std::max(1.0, parseNumericInput(edit_max_k->text().toStdString(), model->max_k_slider));
            model->max_k_slider = max_k;
            double norm = val / 1000.0;
            double k_v = max_k * norm * norm;
            edit_live_k->setText(QString::fromStdString(PRECISION.fmt(k_v)));
            chk_live_k->setChecked(true);
            updating_ui = false;
            evaluateCurrent(false);
        });

        connect(edit_live_k, &QLineEdit::editingFinished, this, [this]() {
            if (updating_ui) return;
            double max_k = std::max(1.0, parseNumericInput(edit_max_k->text().toStdString(), model->max_k_slider));
            double k_v = std::max(0.0, parseNumericInput(edit_live_k->text().toStdString(), 1.0));
            if (k_v > max_k) {
                max_k = k_v * 1.25;
                model->max_k_slider = max_k;
                edit_max_k->setText(QString::number(max_k, 'g', 5));
            }
            updating_ui = true;
            int sl_v = static_cast<int>(std::round(std::sqrt(std::min(k_v, max_k) / max_k) * 1000.0));
            slider_live_k->setValue(sl_v);
            chk_live_k->setChecked(true);
            updating_ui = false;
            evaluateCurrent(false);
        });

        connect(locus_param_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
            if (updating_ui) return;
            bool pole_mode = (idx == 1);
            lbl_vary_pole->setVisible(pole_mode);
            vary_pole_combo->setVisible(pole_mode);
            lbl_fixed_k0->setVisible(pole_mode);
            edit_fixed_k0->setVisible(pole_mode);
            chk_live_k->setText(pole_mode ? "Vary Pole a (×/○ CL):" : "Vary Gain K (×/○ CL):");
            if (gain_title_lbl) gain_title_lbl->setText(pole_mode ? "POLE PARAM a (s = -a)" : "REQUIRED GAIN K");
            refreshVaryPoleCombo();
            evaluateCurrent(false);
        });

        connect(vary_pole_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int) {
            if (!updating_ui) evaluateCurrent(false);
        });

        connect(edit_fixed_k0, &QLineEdit::editingFinished, this, [this]() {
            if (!updating_ui) evaluateCurrent(false);
        });

        auto applyRlDesign = [this](const ConstraintMatrixAnalysis& ans) {
            model->pid_enabled = true;
            model->kp = ans.solved.at("kp");
            model->ki = ans.solved.at("ki");
            model->kd = ans.solved.at("kd");
            // Overlay designed specs on the s-plane canvas
            edit_zeta_spec->setText(QString::fromStdString(PRECISION.fmt(ans.solved.at("zeta"))));
            chk_zeta_spec->setChecked(true);
            if (std::isfinite(ans.solved.at("ts"))) {
                edit_ts_spec->setText(QString::fromStdString(PRECISION.fmt(ans.solved.at("ts"))));
                chk_ts_spec->setChecked(true);
            }
            if (std::isfinite(ans.solved.at("tp"))) {
                edit_tp_spec->setText(QString::fromStdString(PRECISION.fmt(ans.solved.at("tp"))));
                chk_tp_spec->setChecked(true);
            }
            edit_live_k->setText(QString::fromStdString(PRECISION.fmt(model->kp)));
            chk_live_k->setChecked(true);
            evaluateCurrent(true);
        };

        connect(rl_params_btn, &QPushButton::clicked, this, [this, applyRlDesign]() {
            rl_input_map["kp"] = model->kp;
            rl_input_map["kd"] = model->kd;
            rl_input_map["ki"] = model->ki;
            auto* dlg = new DesignParametersDialog(
                model, &rl_lock_map, &rl_input_map, applyRlDesign,
                [this](double new_max_k) {
                    edit_max_k->setText(QString::number(new_max_k, 'g', 5));
                },
                this,
                // 1. On Apply Lead/Lag/PD/PI Compensator -> Populate Gc(s) Controller Card (G(s) stays untouched!)
                [this](const RootLocusCompensatorResult& comp) {
                    model->comp_enabled_s = true;
                    model->c_zeros.clear();
                    model->c_poles.clear();
                    for (int s = 0; s < std::max(1, comp.stages); ++s) {
                        if (comp.has_zero) model->c_zeros.push_back(Complex(-comp.z_c, 0.0));
                        if (comp.has_pole) model->c_poles.push_back(Complex(-comp.p_c, 0.0));
                    }
                    model->c_gain = comp.k_c;
                    model->c_num_str = formatPolynomialFromRoots(model->c_zeros, "s");
                    model->c_den_str = formatPolynomialFromRoots(model->c_poles, "s");
                    model->syncCompensatorToPidGains(); // <-- Syncs Kp, Ki, Kd to Time Domain tab!
                    model->test_point = comp.s_target;

                    updating_ui = true;
                    if (comp_card) comp_card->loadFromModelS(model);
                    if (chk_ghost_rl) chk_ghost_rl->setChecked(true);
                    if (!is_polar_mode) {
                        tp_v1_edit->setText(QString::number(comp.s_target.real(), 'g', 4));
                        tp_v2_edit->setText(QString::number(comp.s_target.imag(), 'g', 4));
                    } else {
                        tp_v1_edit->setText(QString::number(std::abs(comp.s_target), 'g', 4));
                        tp_v2_edit->setText(QString::number(std::arg(comp.s_target) * 180.0 / PI, 'g', 4));
                    }
                    edit_live_k->setText(QString::fromStdString(PRECISION.fmt(comp.k_c)));
                    chk_live_k->setChecked(true);
                    updating_ui = false;

                    evaluateCurrent(true);
                },
                // 2. On Disable / Reset Compensator Gc(s):
                [this]() {
                    model->comp_enabled_s = false;
                    updating_ui = true;
                    if (comp_card) comp_card->loadFromModelS(model);
                    if (chk_ghost_rl) chk_ghost_rl->setChecked(false);
                    updating_ui = false;
                    evaluateCurrent(true);
                }
            );
            dlg->setAttribute(Qt::WA_DeleteOnClose);
            dlg->show();
        });

        loadFromModel();
    }

    void onCoordModeSwitch(bool to_polar) {
        if (to_polar == is_polar_mode) return;
        double v1 = parseNumericInput(tp_v1_edit->text().toStdString(), 0.0);
        double v2 = parseNumericInput(tp_v2_edit->text().toStdString(), 0.0);
        updating_ui = true;
        if (to_polar) {
            Complex s(v1, v2);
            tp_lbl1->setText("Magnitude |s|");
            tp_lbl2->setText("Angle (deg)");
            tp_v1_edit->setText(QString::number(std::abs(s), 'g', 5));
            tp_v2_edit->setText(QString::number(std::arg(s) * 180.0 / PI, 'g', 5));
        } else {
            Complex s = std::polar(v1, v2 * PI / 180.0);
            tp_lbl1->setText("Real (σ)");
            tp_lbl2->setText("Imag (jω)");
            tp_v1_edit->setText(QString::number(s.real(), 'g', 5));
            tp_v2_edit->setText(QString::number(s.imag(), 'g', 5));
        }
        is_polar_mode = to_polar;
        updating_ui = false;
        evaluateCurrent(true);
    }

    void refreshVaryPoleCombo() {
        if (!vary_pole_combo) return;
        bool prev = updating_ui;
        updating_ui = true;
        int prev_data = vary_pole_combo->currentData().isValid() ? vary_pole_combo->currentData().toInt() : -1;
        vary_pole_combo->clear();
        int restore_combo_idx = 0;
        for (size_t i = 0; i < model->poles.size(); ++i) {
            if (std::abs(model->poles[i].imag()) <= 1e-5) { // Only physically realizable real poles!
                if (static_cast<int>(i) == prev_data) restore_combo_idx = vary_pole_combo->count();
                vary_pole_combo->addItem(
                    QString("p%1 = %2").arg(i + 1).arg(QString::fromStdString(PRECISION.fmt(model->poles[i].real()))),
                    static_cast<int>(i));
            }
        }
        if (vary_pole_combo->count() == 0) {
            vary_pole_combo->addItem("(No real poles)", -1);
        } else {
            vary_pole_combo->setCurrentIndex(restore_combo_idx);
        }
        updating_ui = prev;
    }

    void applyPolyNumDen() {
        try {
            auto p_num = parsePolynomialInput(num_edit->text().toStdString());
            auto p_den = parsePolynomialInput(den_edit->text().toStdString());
            if (std::abs(p_den.leading_gain) <= 1e-14)
                throw std::runtime_error("Divide by zero: Den(s) cannot be 0.");
            if (std::abs(p_num.leading_gain) <= 1e-14)
                throw std::runtime_error("Zero numerator: Num(s) cannot be 0.");
            model->zeros = p_num.roots;
            model->poles = p_den.roots;
            double ratio = p_num.leading_gain / std::max(std::abs(p_den.leading_gain), 1e-12);
            if (std::abs(ratio) > 1e-12) model->plant_gain = ratio;
            poles_card->setValues(model->poles);
            zeros_card->setValues(model->zeros);
            if (model->zeros.size() > model->poles.size()) {
                poly_status_lbl->setStyleSheet("color: #f59e0b; font-size: 10px; font-weight: bold; border:none;");
                poly_status_lbl->setText(QString("⚠ Improper G(s): deg(Num)=%1 > deg(Den)=%2 (Non-causal)")
                    .arg(model->zeros.size()).arg(model->poles.size()));
            } else {
                poly_status_lbl->setStyleSheet("color: #94a3b8; font-size: 10px; border:none;");
                poly_status_lbl->setText(QString("✔ Auto-filled %1 Poles & %2 Zeros (K = %3)")
                    .arg(model->poles.size()).arg(model->zeros.size())
                    .arg(QString::fromStdString(PRECISION.fmt(model->plant_gain))));
            }
            refreshVaryPoleCombo();
            if (fb_bar) fb_bar->syncToModelS(model);
            evaluateCurrent(true);
        } catch (const std::exception& e) {
            poly_status_lbl->setStyleSheet("color: #ef4444; font-size: 10px; font-weight: bold; border:none;");
            poly_status_lbl->setText(QString("⚠ Polynomial error: %1").arg(e.what()));
        }
    }

    void loadFromModel() {
        updating_ui = true;
        poles_card->setValues(model->poles);
        zeros_card->setValues(model->zeros);
        num_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->zeros, "s")));
        den_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->poles, "s")));
        if (!is_polar_mode) {
            tp_v1_edit->setText(QString::number(model->test_point.real(), 'g', 4));
            tp_v2_edit->setText(QString::number(model->test_point.imag(), 'g', 4));
        } else {
            tp_v1_edit->setText(QString::number(std::abs(model->test_point), 'g', 4));
            tp_v2_edit->setText(QString::number(std::arg(model->test_point) * 180.0 / PI, 'g', 4));
        }
        tol_edit->setText(QString::number(model->rl_tol_deg, 'g', 3));
        fb_bar->loadFromModelS(model);
        if (comp_card) comp_card->loadFromModelS(model);
        updating_ui = false;
        refreshVaryPoleCombo();
        evaluateCurrent(false);
    }

    void evaluateCurrent(bool notify) {
        double v1 = parseNumericInput(tp_v1_edit->text().toStdString(), 0.0);
        double v2 = parseNumericInput(tp_v2_edit->text().toStdString(), 0.0);
        Complex tp = is_polar_mode ? std::polar(v1, v2 * PI / 180.0) : Complex(v1, v2);
        double tol = std::abs(parseNumericInput(tol_edit->text().toStdString(), 0.5));
        model->test_point = tp;
        model->rl_tol_deg = tol;

        // Combine G(s) and H(s) poles/zeros
        auto raw_loop_poles = model->effectiveLoopPolesS();
        auto raw_loop_zeros = model->effectiveLoopZerosS();

        // ---> IMPROPER CHECK (WITH AUTOMATIC CLEAR WHEN PROPER AGAIN) <---
        if (model->zeros.size() > model->poles.size()) {
            poly_status_lbl->setStyleSheet("color: #f59e0b; font-size: 10px; font-weight: bold; border:none;");
            poly_status_lbl->setText(QString("⚠ Improper G(s): deg(Num)=%1 > deg(Den)=%2 (Non-causal in time domain)")
                .arg(model->zeros.size()).arg(model->poles.size()));
        } else if (raw_loop_zeros.size() > raw_loop_poles.size()) {
            poly_status_lbl->setStyleSheet("color: #f59e0b; font-size: 10px; font-weight: bold; border:none;");
            poly_status_lbl->setText(QString("⚠ Improper Loop G(s)H(s): deg(Num)=%1 > deg(Den)=%2")
                .arg(raw_loop_zeros.size()).arg(raw_loop_poles.size()));
        } else if (poly_status_lbl->text().startsWith("⚠ Improper")) {
            poly_status_lbl->setStyleSheet(QString("color: %1; font-size: 10px; border:none;").arg(Theme::TEXT_SECONDARY));
            poly_status_lbl->setText(QString("✔ Active: %1 Poles & %2 Zeros (K = %3)")
                .arg(model->poles.size()).arg(model->zeros.size())
                .arg(QString::fromStdString(PRECISION.fmt(model->plant_gain))));
        }

        int vary_idx = (vary_pole_combo && vary_pole_combo->currentData().isValid())
            ? vary_pole_combo->currentData().toInt() : -1;
        bool is_pole_sweep = (locus_param_combo && locus_param_combo->currentIndex() == 1 &&
                              vary_idx >= 0 && vary_idx < static_cast<int>(raw_loop_poles.size()));
        if (locus_param_combo && locus_param_combo->currentIndex() == 1 && vary_idx < 0) {
            poly_status_lbl->setStyleSheet("color: #f59e0b; font-size: 10px; font-weight: bold; border:none;");
            poly_status_lbl->setText("⚠ Vary Pole s = -a requires at least one real pole in G(s). Showing standard K locus.");
        }
        double fixed_k0 = edit_fixed_k0 ? std::max(0.0, parseNumericInput(edit_fixed_k0->text().toStdString(), 10.0)) : 10.0;

        std::vector<Complex> loop_poles = raw_loop_poles;
        std::vector<Complex> loop_zeros = raw_loop_zeros;

        if (is_pole_sweep) {
            auto eq = buildPoleSweepSystem(raw_loop_poles, raw_loop_zeros, vary_idx, fixed_k0);
            loop_poles = eq.eq_poles;
            loop_zeros = eq.eq_zeros;
        }

        std::string gs_ltx = fb_bar ? fb_bar->formatActiveLatexS(model, false)
                                    : formatTransferFunctionLatex(model->poles, model->zeros, model->plant_gain, "s", "G");
        if (is_pole_sweep) {
            gs_ltx += " \\; , \\quad \\mathrm{Sweep}\\; p_{" + std::to_string(vary_idx + 1) +
                      "} = -a \\; (K_0 = " + PRECISION.fmt(fixed_k0) + ")";
        }
        tf_math_w->setLatex(QString::fromStdString(gs_ltx));

        // Evaluate Root Locus condition on combined loop poles & zeros
        auto ev = evaluateTransferFunction(loop_poles, loop_zeros, tp, 1.0);
        last_gain = ev.rl_gain_k;
        double def = computeAngleDeficiency(ev.angle_deg);
        bool on_rl = ev.at_pole || ev.at_zero || (std::abs(def) <= tol);

        val_angle->setText((ev.at_pole || ev.at_zero) ? "Undefined" : QString::fromStdString(PRECISION.fmt(ev.angle_deg) + "°"));
        val_mag->setText(ev.at_pole ? "Inf" : QString::fromStdString(PRECISION.fmt(ev.magnitude)));
        val_gain->setText(ev.at_zero ? "Inf" : QString::fromStdString(PRECISION.fmt(ev.rl_gain_k)));

        QString bg = on_rl ? Theme::BANNER_OK_BG : Theme::BANNER_WARN_BG;
        QString bd = on_rl ? Theme::BANNER_OK_BD : Theme::BANNER_WARN_BD;
        QString fg = on_rl ? Theme::BANNER_OK_FG : Theme::BANNER_WARN_FG;
        QString sub_fg = on_rl ? "#a7f3d0" : "#fde68a";
        QString val_col = on_rl ? Theme::ACCENT_ZERO : Theme::ACCENT_OPEN;

        rl_banner->setStyleSheet(QString("QFrame { background-color: %1; border: 1.5px solid %2; border-radius: 5px; }").arg(bg, bd));
        banner_title->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 12px; border: none;").arg(fg));
        banner_right->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; font-size: 11px; border: none;").arg(Theme::TEXT_PRIMARY));
        banner_sub->setStyleSheet(QString("color: %1; font-size: 10px; border: none;").arg(sub_fg));

        for (auto* vl : {val_angle, val_mag, val_gain}) {
            vl->setStyleSheet(QString("color: %1; font-family: Monospace; font-size: 12px; font-weight: bold; border:none;").arg(val_col));
        }

        banner_title->setText(on_rl ? "✔ ON ROOT LOCUS" : "✖ NOT ON ROOT LOCUS");
        banner_right->setText(on_rl
            ? QString("|Δθ| = %1° (≤ %2°)").arg(QString::fromStdString(PRECISION.fmt(std::abs(def))), QString::number(tol))
            : QString("Deficiency Δθ = %1°").arg(QString::fromStdString(PRECISION.fmt(def, true))));
        if (ev.at_pole && ev.at_zero) {
            banner_sub->setText("Exact open-loop pole-zero cancellation at s");
        } else if (ev.at_pole) {
            banner_sub->setText("Open-loop pole — Root Locus branch starting point (K = 0)");
        } else if (ev.at_zero) {
            banner_sub->setText("Open-loop zero — Root Locus branch endpoint (K → ∞)");
        } else if (on_rl) {
            banner_sub->setText(QString("Angle condition ∠G(s)H(s) = ±180°(2k+1) satisfied — Closed-loop pole at s for K = %1")
                .arg(QString::fromStdString(PRECISION.fmt(ev.rl_gain_k))));
        } else {
            banner_sub->setText(QString("Off by %1° from ±180° (requires %2° compensator phase to place pole at s)")
                .arg(QString::fromStdString(PRECISION.fmt(std::abs(def))),
                     QString::fromStdString(PRECISION.fmt(def, true))));
        }

        // Pass combined loop poles and zeros to the s-plane canvas
        canvas->poles = loop_poles;
        canvas->zeros = loop_zeros;
        // Always keep Ghost RL synced to the uncompensated G(s)H(s)
        canvas->ghost_poles = model->uncompensatedLoopPolesS();
        canvas->ghost_zeros = model->uncompensatedLoopZerosS();
        canvas->show_ghost_locus = chk_ghost_rl && chk_ghost_rl->isChecked();

        // Highlight Gc(s) poles & zeros when Gc(s) is enabled
        canvas->comp_poles = model->comp_enabled_s ? model->c_poles : std::vector<Complex>{};
        canvas->comp_zeros = model->comp_enabled_s ? model->c_zeros : std::vector<Complex>{};


        canvas->test_point = tp;
        canvas->is_on_rl = on_rl;

        canvas->param_sym = is_pole_sweep ? "a" : "K";

        // Sync s-Plane Spec Overlay Lines
        canvas->show_ts_line   = chk_ts_spec->isChecked();
        canvas->spec_ts        = std::max(1e-4, parseNumericInput(edit_ts_spec->text().toStdString(), 2.0));
        canvas->show_tp_line   = chk_tp_spec->isChecked();
        canvas->spec_tp        = std::max(1e-4, parseNumericInput(edit_tp_spec->text().toStdString(), 1.0));
        canvas->show_zeta_line = chk_zeta_spec->isChecked();
        canvas->spec_zeta      = std::clamp(parseNumericInput(edit_zeta_spec->text().toStdString(), 0.7071), 0.0, 1.0);

        // Compute Live Closed-Loop Poles & Zeros
        canvas->show_live_k_poles = chk_live_k->isChecked();
        canvas->live_k_val = std::max(0.0, parseNumericInput(edit_live_k->text().toStdString(), 1.0));
        if (canvas->show_live_k_poles) {
            size_t deg = std::max(loop_poles.size(), loop_zeros.size());
            Poly den_p = polyFromRoots(loop_poles);
            Poly num_p = polyFromRoots(loop_zeros);
            Poly den_pad(deg + 1, Complex(0.0, 0.0));
            Poly num_pad(deg + 1, Complex(0.0, 0.0));
            for (size_t i = 0; i < den_p.size(); ++i) den_pad[deg + 1 - den_p.size() + i] = den_p[i];
            for (size_t i = 0; i < num_p.size(); ++i) num_pad[deg + 1 - num_p.size() + i] = num_p[i];

            Poly char_poly = polyAdd(den_pad, polyScale(num_pad, Complex(canvas->live_k_val, 0.0)));
            canvas->live_cl_poles = solvePolynomialRoots(char_poly);

            // Closed-loop zeros of T(s) = K*Gc(s)*G(s) / (1 + K*Gc(s)*G(s)*H(s)):
            // Forward zeros of G(s) and Gc(s), plus poles of feedback H(s) when non-unity
            std::vector<Complex> cl_z = model->zeros;
            if (model->comp_enabled_s) {
                cl_z.insert(cl_z.end(), model->c_zeros.begin(), model->c_zeros.end());
            }
            if (!model->unity_feedback_s) {
                cl_z.insert(cl_z.end(), model->h_poles.begin(), model->h_poles.end());
            }
            canvas->live_cl_zeros = cl_z;

            std::string p_list, z_list;
            for (size_t i = 0; i < canvas->live_cl_poles.size(); ++i) {
                p_list += (i ? ",  " : "") + PRECISION.fmtComplex(canvas->live_cl_poles[i]);
            }
            for (size_t i = 0; i < canvas->live_cl_zeros.size(); ++i) {
                z_list += (i ? ",  " : "") + PRECISION.fmtComplex(canvas->live_cl_zeros[i]);
            }
            QString param_sym = is_pole_sweep ? "a" : "K";
            live_k_poles_lbl->setText(QString("CL Poles @ %1=%2: [%3]  •  CL Zeros: [%4]")
                .arg(param_sym)
                .arg(QString::fromStdString(PRECISION.fmt(canvas->live_k_val)))
                .arg(p_list.empty() ? "None" : QString::fromStdString(p_list))
                .arg(z_list.empty() ? "None" : QString::fromStdString(z_list)));
        } else {
            canvas->live_cl_poles.clear();
            canvas->live_cl_zeros.clear();
            live_k_poles_lbl->setText("Enable Ts, Tp, ζ to view RL intersections (K, ωn) or click Design Parameters.");
        }

        canvas->repaint();
        if (notify) model->notify("root_locus");
    }
};