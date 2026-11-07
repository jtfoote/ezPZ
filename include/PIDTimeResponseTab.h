#pragma once
#include "RootLocusTab.h"


// =============================================================================
// Tab 2: Time Response & PID Simulator View (With Editable Poles/Zeros + PID Disable)
// =============================================================================
class PIDTimeResponseTab : public QWidget {
    Q_OBJECT
public:
    SharedSystemModel* model;
    SimulationConfig cfg;

    QLineEdit *num_edit, *den_edit, *os_target_edit, *zeta_target_edit;
    PoleZeroEditorCard *poles_card, *zeros_card;
    QLineEdit *kp_edit, *ki_edit, *kd_edit;
    QSlider *kp_slider, *ki_slider, *kd_slider;
    QCheckBox *enable_pid_chk;
    QComboBox *loop_mode_combo, *sig_combo;
    QLineEdit *mag_edit, *gain_edit, *cursor_edit, *tmax_edit, *tol_edit;
    QLabel *poly_status_lbl;

    QFrame* status_banner;
    QLabel *status_title_lbl, *status_right_lbl;
    QLabel *m_card1, *m_card2, *m_card3, *m_card4, *m_card5, *m_card6;
    TimeResponseCanvas* canvas;
    bool updating_ui = false;

    FeedbackConfigBar* fb_bar;
    MathFormulaWidget* tf_math_w;

    MathFormulaWidget* pid_design_formula_w;
    std::map<std::string, bool> pid_lock_map = {
        {"kp", false}, {"kd", true}, {"ki", true}, {"os_pct", true},
        {"zeta", false}, {"wn", false}, {"ts", false}, {"tp", false},
        {"tr", false}, {"sigma_d", false}, {"omega_d", false}, {"ess", false}, {"rel_err_pct", false}
    };
    std::map<std::string, double> pid_input_map = {
        {"kp", 1.5}, {"kd", 0.0}, {"ki", 0.0}, {"os_pct", 20.0},
        {"zeta", 0.456}, {"wn", 5.48}, {"ts", 1.60}, {"tp", 0.64},
        {"tr", 0.34}, {"sigma_d", 2.50}, {"omega_d", 4.88}, {"ess", 0.20}, {"rel_err_pct", 20.0}
    };

    explicit PIDTimeResponseTab(SharedSystemModel* m, QWidget* parent = nullptr)
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

        // 1. Plant G(s) Polynomial Solver Card
        auto* poly_card = makeCard();
        auto* poly_l = new QVBoxLayout(poly_card);
        poly_l->setContentsMargins(10, 6, 10, 6);
        auto* poly_hdr = new QHBoxLayout();
        auto* t1 = new QLabel("1. Plant Transfer Function  G(s) = K · Num(s) / Den(s)");
        t1->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));
        auto* solve_btn = makeActionBtn("Solve ➔", Theme::ACCENT_PID);
        poly_hdr->addWidget(t1); poly_hdr->addStretch(); poly_hdr->addWidget(solve_btn);
        poly_l->addLayout(poly_hdr);

        auto* pgrid = new QGridLayout();
        num_edit = makeDarkEntry();
        den_edit = makeDarkEntry();
        auto* nl = new QLabel("Num(s):"); nl->setStyleSheet("color: #38bdf8; font-family: Monospace; font-weight: bold; border:none;");
        auto* dl = new QLabel("Den(s):"); dl->setStyleSheet("color: #ff5f56; font-family: Monospace; font-weight: bold; border:none;");
        pgrid->addWidget(nl, 0, 0); pgrid->addWidget(num_edit, 0, 1);
        pgrid->addWidget(dl, 0, 2); pgrid->addWidget(den_edit, 0, 3);
        poly_l->addLayout(pgrid);
        poly_status_lbl = new QLabel("Type Num(s) & Den(s) (e.g. (s+2)(s+3)) and press Enter.");
        poly_status_lbl->setStyleSheet("color: #94a3b8; font-size: 10px; border:none;");
        poly_l->addWidget(poly_status_lbl);

        fb_bar = new FeedbackConfigBar("s", "H");
        poly_l->addWidget(fb_bar);
        tf_math_w = new MathFormulaWidget("", QColor(Theme::ACCENT_ZERO), QColor(Theme::BG_INPUT), 10);
        poly_l->addWidget(tf_math_w);
        fb_bar->onChanged = [this]() { fb_bar->syncToModelS(model); runSimulation(true); };

        left_v->addWidget(poly_card);

        // 2. Editable Poles (×) & Zeros (○) Tables Row
        auto* pz_row = new QHBoxLayout();
        poles_card = new PoleZeroEditorCard("Poles (×)", Theme::ACCENT_POLE, -1.0);
        zeros_card = new PoleZeroEditorCard("Zeros (○)", Theme::ACCENT_ZERO, -3.0);
        pz_row->addWidget(poles_card, 1);
        pz_row->addWidget(zeros_card, 1);
        left_v->addLayout(pz_row);

        // 3. PID Controller & Bypass / Loop Mode Card
        auto* pid_card = makeCard();
        auto* pid_l = new QVBoxLayout(pid_card);
        pid_l->setContentsMargins(10, 6, 10, 6);
        auto* pid_hdr = new QHBoxLayout();
        auto* t2 = new QLabel("2. PID Controller & Spec Synthesizer");
        t2->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::ACCENT_PID));

        enable_pid_chk = new QCheckBox("Enable PID C(s)");
        enable_pid_chk->setChecked(true);
        enable_pid_chk->setStyleSheet(QString("QCheckBox { color: %1; font-weight: bold; font-size: 11px; border: none; }").arg(Theme::ACCENT_PID));

        loop_mode_combo = new QComboBox();
        loop_mode_combo->addItems({
            "Closed-Loop (with PID)",
            "Closed-Loop (No PID, C=1)",
            "Open-Loop Plant Only"
        });
        loop_mode_combo->setStyleSheet(QString("background-color: %1; color: %2; border: 1px solid %3; padding: 3px; font-weight: bold; font-size: 11px;")
                                       .arg(Theme::BG_INPUT, Theme::TEXT_PRIMARY, Theme::BORDER_INPUT));

        pid_hdr->addWidget(t2); pid_hdr->addStretch(); pid_hdr->addWidget(enable_pid_chk); pid_hdr->addWidget(loop_mode_combo);
        pid_l->addLayout(pid_hdr);

        auto* synth_bar = new QHBoxLayout();
        auto* l_os = new QLabel("Target %OS:"); l_os->setStyleSheet("color: #94a3b8; font-size: 10px; border:none;");
        os_target_edit = makeDarkEntry("20.0", 48);
        auto* l_zt = new QLabel("or ζ:"); l_zt->setStyleSheet("color: #94a3b8; font-size: 10px; border:none;");
        zeta_target_edit = makeDarkEntry("0.456", 48);
        auto* design_ctrl_btn = makeActionBtn("Design Controller ➔", "#ffffff", Theme::ACCENT_PID);
        auto* open_params_btn = makeActionBtn("🎛 Design Parameters...", Theme::ACCENT_ZERO);
        synth_bar->addWidget(l_os); synth_bar->addWidget(os_target_edit);
        synth_bar->addWidget(l_zt); synth_bar->addWidget(zeta_target_edit);
        synth_bar->addWidget(design_ctrl_btn);
        synth_bar->addWidget(open_params_btn);
        pid_l->addLayout(synth_bar);

        pid_design_formula_w = new MathFormulaWidget("", QColor(Theme::ACCENT_ZERO), QColor(Theme::BG_INPUT), 9, this);
        pid_design_formula_w->hide();
        // pid_design_formula_w->setVisible(false);
        // pid_l->addWidget(pid_design_formula_w);

        auto* gains_grid = new QHBoxLayout();
        auto makeGainCol = [&](const QString& title, QLineEdit*& ed, QSlider*& sl, double max_v) {
            auto* col = new QVBoxLayout();
            auto* lbl = new QLabel(title); lbl->setStyleSheet("color: #94a3b8; font-size: 10px; font-weight: bold; border:none;");
            ed = makeDarkEntry("1.0", 65);
            sl = new QSlider(Qt::Horizontal);
            sl->setRange(0, static_cast<int>(max_v * 100));
            col->addWidget(lbl); col->addWidget(ed); col->addWidget(sl);
            gains_grid->addLayout(col);
        };
        makeGainCol("Kp (Prop)", kp_edit, kp_slider, 50.0);
        makeGainCol("Ki (Integ)", ki_edit, ki_slider, 25.0);
        makeGainCol("Kd (Deriv)", kd_edit, kd_slider, 10.0);
        pid_l->addLayout(gains_grid);

        auto* sim_row = new QGridLayout();
        sig_combo = new QComboBox();
        sig_combo->addItems({"Step", "Impulse", "Ramp", "Sine"});
        sig_combo->setStyleSheet(loop_mode_combo->styleSheet());
        mag_edit    = makeDarkEntry("1.0", 50);
        gain_edit   = makeDarkEntry("1.0", 50);
        cursor_edit = makeDarkEntry("2.5", 50);
        tmax_edit   = makeDarkEntry("10.0", 50);
        tol_edit    = makeDarkEntry("2.0", 50);

        auto addLabeled = [&](int col, const QString& txt, QWidget* w) {
            auto* l = new QLabel(txt); l->setStyleSheet("color: #94a3b8; font-size: 9px; font-weight: bold; border:none;");
            sim_row->addWidget(l, 0, col);
            sim_row->addWidget(w, 1, col);
        };
        addLabeled(0, "Input r(t)", sig_combo);
        addLabeled(1, "Amp M", mag_edit);
        addLabeled(2, "Gain K", gain_edit);
        addLabeled(3, "Cursor t", cursor_edit);
        addLabeled(4, "T_max (s)", tmax_edit);
        addLabeled(5, "Tol (±%)", tol_edit);
        pid_l->addLayout(sim_row);
        left_v->addWidget(pid_card);

        // 4. Unified System Telemetry Card with Compact Full-Width Banner
        auto* tel_card = makeCard();
        auto* tel_l = new QVBoxLayout(tel_card);
        tel_l->setContentsMargins(10, 6, 10, 6);

        // Compact Full-Width Horizontal Stability Banner (Fixed Height, Never Stretches Vertically!)
        status_banner = new QFrame();
        status_banner->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        status_banner->setFixedHeight(32);
        auto* sb_l = new QHBoxLayout(status_banner);
        sb_l->setContentsMargins(10, 4, 10, 4);
        status_title_lbl = new QLabel("✔ STABLE • CL (PID ON) • Underdamped");
        status_right_lbl = new QLabel("max Re(s) = -2.00");
        sb_l->addWidget(status_title_lbl);
        sb_l->addStretch();
        sb_l->addWidget(status_right_lbl);
        tel_l->addWidget(status_banner);

        auto* mgrid = new QGridLayout();
        auto makeTelCard = [&](int r, int c, const QString& title, QLabel*& val_lbl) {
            auto* b = makeCard();
            b->setStyleSheet(QString("background-color: %1; border: 1px solid %2; border-radius: 4px; padding: 4px;").arg(Theme::BG_INPUT, Theme::BORDER_INPUT));
            auto* vl = new QVBoxLayout(b);
            vl->setContentsMargins(6, 3, 6, 3);
            auto* tl = new QLabel(title); tl->setStyleSheet("color: #94a3b8; font-size: 9px; font-weight: bold; border:none;");
            val_lbl = new QLabel("—"); val_lbl->setStyleSheet(QString("color: %1; font-family: Monospace; font-size: 10px; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));
            vl->addWidget(tl); vl->addWidget(val_lbl);
            mgrid->addWidget(b, r, c);
        };
        makeTelCard(0, 0, "OVERSHOOT, S.S.E. & RELATIVE ERROR", m_card1);
        makeTelCard(0, 1, "RESPONSE TIMES (Tr, Tp & Ts)", m_card2);
        makeTelCard(1, 0, "DAMPING ζ & FREQUENCIES (ωn, ωd, σd)", m_card3);
        makeTelCard(1, 1, "STEADY-STATE y_ss, TYPE & Kp,pos / Kv", m_card4);
        makeTelCard(2, 0, "CLOSED-LOOP & OPEN-LOOP POLES", m_card5);
        makeTelCard(2, 1, "CURSOR INSPECTION @ t", m_card6);
        tel_l->addLayout(mgrid);
        left_v->addWidget(tel_card, 1);

        // Right Plot Panel
        auto* right_card = makeCard();
        auto* right_l = new QVBoxLayout(right_card);
        auto* plot_hdr = new QHBoxLayout();
        auto* pt = new QLabel("Time-Domain Response  y(t)  (L-Click: Cursor t • Scroll: Zoom • R-Drag: Pan)");
        pt->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 13px; border:none;").arg(Theme::TEXT_PRIMARY));
        auto* chk_ol = new QCheckBox("Overlay Open-Loop"); chk_ol->setStyleSheet("color: #f59e0b; font-weight: bold;");
        auto* chk_u  = new QCheckBox("Control u(t)");      chk_u->setStyleSheet("color: #ff5f56; font-weight: bold;");
        auto* chk_in = new QCheckBox("s-Plane Inset");     chk_in->setChecked(true); chk_in->setStyleSheet("color: #34d399; font-weight: bold;");
        auto* fit_b  = makeActionBtn("Fit View", Theme::TEXT_PRIMARY, Theme::BG_PANEL);
        plot_hdr->addWidget(pt); plot_hdr->addStretch();
        plot_hdr->addWidget(chk_ol); plot_hdr->addWidget(chk_u); plot_hdr->addWidget(chk_in); plot_hdr->addWidget(fit_b);
        right_l->addLayout(plot_hdr);

        canvas = new TimeResponseCanvas(false);
        attachSvgExportMenu(canvas, "time_response.svg");
        right_l->addWidget(canvas, 1);

        splitter->addWidget(wrapInLeftScroll(left_w, 380));
        splitter->addWidget(right_card);
        splitter->setStretchFactor(0, 0);
        splitter->setStretchFactor(1, 1);

        // Connections
        connect(solve_btn, &QPushButton::clicked, this, &PIDTimeResponseTab::applyPolyNumDen);
        connect(num_edit, &QLineEdit::returnPressed, this, &PIDTimeResponseTab::applyPolyNumDen);
        connect(den_edit, &QLineEdit::returnPressed, this, &PIDTimeResponseTab::applyPolyNumDen);

        poles_card->onValuesChanged = [this]() {
            if (updating_ui) return;
            model->poles = poles_card->getValues();
            den_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->poles, "s")));
            runSimulation(true);
        };
        zeros_card->onValuesChanged = [this]() {
            if (updating_ui) return;
            model->zeros = zeros_card->getValues();
            num_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->zeros, "s")));
            runSimulation(true);
        };

        connect(enable_pid_chk, &QCheckBox::toggled, this, [this](bool checked) {
            if (updating_ui) return;
            updating_ui = true;
            if (!checked && loop_mode_combo->currentIndex() == 0) loop_mode_combo->setCurrentIndex(1);
            else if (checked && loop_mode_combo->currentIndex() == 1) loop_mode_combo->setCurrentIndex(0);
            updating_ui = false;
            updatePidWidgetsState();
            runSimulation(true);
        });

        connect(loop_mode_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
            if (updating_ui) return;
            updating_ui = true;
            if (idx == 0) enable_pid_chk->setChecked(true);
            else if (idx == 1) enable_pid_chk->setChecked(false);
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

        auto applyPidDesign = [this](const ConstraintMatrixAnalysis& ans) {
            // pid_design_formula_w->setVisible(true);
            pid_design_formula_w->setLatex(QString::fromStdString(ans.formulas_latex));
            updating_ui = true;
            enable_pid_chk->setChecked(true);
            loop_mode_combo->setCurrentIndex(0);
            kp_edit->setText(QString::fromStdString(PRECISION.fmt(ans.solved.at("kp"))));
            ki_edit->setText(QString::fromStdString(PRECISION.fmt(ans.solved.at("ki"))));
            kd_edit->setText(QString::fromStdString(PRECISION.fmt(ans.solved.at("kd"))));
            os_target_edit->setText(QString::fromStdString(PRECISION.fmt(ans.solved.at("os_pct"))));
            zeta_target_edit->setText(QString::fromStdString(PRECISION.fmt(ans.solved.at("zeta"))));
            kp_slider->setRange(0, static_cast<int>(std::max(model->max_k_slider, ans.solved.at("kp") * 1.25) * 100));
            kp_slider->setValue(static_cast<int>(std::round(ans.solved.at("kp") * 100.0)));
            ki_slider->setValue(static_cast<int>(std::round(ans.solved.at("ki") * 100.0)));
            kd_slider->setValue(static_cast<int>(std::round(ans.solved.at("kd") * 100.0)));
            updating_ui = false;
            updatePidWidgetsState();
            runSimulation(true);
        };

        connect(os_target_edit, &QLineEdit::editingFinished, this, [this]() {
            double os_v = parseNumericInput(os_target_edit->text().toStdString(), 20.0);
            pid_input_map["os_pct"] = os_v;
            zeta_target_edit->setText(QString::fromStdString(PRECISION.fmt(niseZetaFromOs(os_v))));
        });

        connect(zeta_target_edit, &QLineEdit::editingFinished, this, [this]() {
            double z_v = parseNumericInput(zeta_target_edit->text().toStdString(), 0.456);
            double os_v = niseOsFromZeta(z_v);
            pid_input_map["zeta"] = z_v;
            pid_input_map["os_pct"] = os_v;
            os_target_edit->setText(QString::fromStdString(PRECISION.fmt(os_v)));
        });

        connect(design_ctrl_btn, &QPushButton::clicked, this, [this, applyPidDesign]() {
            pid_input_map["kp"] = parseNumericInput(kp_edit->text().toStdString(), model->kp);
            pid_input_map["kd"] = parseNumericInput(kd_edit->text().toStdString(), model->kd);
            pid_input_map["ki"] = parseNumericInput(ki_edit->text().toStdString(), model->ki);
            pid_input_map["os_pct"] = parseNumericInput(os_target_edit->text().toStdString(), 20.0);
            auto ans = ParameterMatrixEngine::evaluate(pid_lock_map, pid_input_map, model);
            applyPidDesign(ans);
        });

        connect(open_params_btn, &QPushButton::clicked, this, [this, applyPidDesign]() {
            pid_input_map["kp"] = parseNumericInput(kp_edit->text().toStdString(), model->kp);
            pid_input_map["kd"] = parseNumericInput(kd_edit->text().toStdString(), model->kd);
            pid_input_map["ki"] = parseNumericInput(ki_edit->text().toStdString(), model->ki);
            pid_input_map["os_pct"] = parseNumericInput(os_target_edit->text().toStdString(), 20.0);
            auto* dlg = new DesignParametersDialog(
                model, &pid_lock_map, &pid_input_map, applyPidDesign,
                [this](double new_max_k) {
                    kp_slider->setRange(0, static_cast<int>(new_max_k * 100));
                },
                this
            );
            dlg->setAttribute(Qt::WA_DeleteOnClose);
            dlg->show();
        });

        for (auto* ed : {mag_edit, gain_edit, cursor_edit, tmax_edit, tol_edit}) {
            connect(ed, &QLineEdit::editingFinished, this, [this]() { runSimulation(true); });
        }
        connect(sig_combo, &QComboBox::currentTextChanged, this, [this]() { runSimulation(true); });
        connect(chk_ol, &QCheckBox::toggled, this, [this](bool v) { canvas->show_ol = v; canvas->update(); });
        connect(chk_u,  &QCheckBox::toggled, this, [this](bool v) { canvas->show_u  = v; canvas->update(); });

        connect(chk_in, &QCheckBox::toggled, this, [this](bool v) { canvas->show_inset = v; canvas->update(); });
        connect(fit_b,  &QPushButton::clicked, this, [this]() { canvas->resetView(); });

        canvas->onCursorChanged = [this](double t_val) {
            cursor_edit->setText(QString::number(t_val, 'g', 4));
            runSimulation(false);
        };

        loadFromModel();
    }

    void updatePidWidgetsState() {
        bool pid_active = enable_pid_chk->isChecked() && (loop_mode_combo->currentIndex() == 0);
        for (QWidget* w : std::vector<QWidget*>{kp_edit, ki_edit, kd_edit, kp_slider, ki_slider, kd_slider}) {
            w->setEnabled(pid_active);
        }
    }

    void synthesizeController(bool design_kd) {
        double os_v = parseNumericInput(os_target_edit->text().toStdString(), 20.0);
        double z = PRECISION.step(niseZetaFromOs(os_v));
        zeta_target_edit->setText(QString::fromStdString(PRECISION.fmt(z)));

        auto [num_g, den_g] = ComplexTransferFunctionSimulator::buildPlantPolynomials(model->poles, model->zeros, model->plant_gain);
        if (den_g.size() >= 3) {
            double a1 = den_g[den_g.size() - 2].real();
            double a0 = den_g.back().real();
            double k0 = std::max(std::abs(num_g.back().real()), 1e-9);
            enable_pid_chk->setChecked(true);
            loop_mode_combo->setCurrentIndex(0);
            if (!design_kd) {
                double wn = PRECISION.step(a1 / (2.0 * std::max(z, 1e-5)));
                double kp_new = PRECISION.step((wn * wn - a0) / k0);
                kp_edit->setText(QString::fromStdString(PRECISION.fmt(kp_new)));
                ki_edit->setText("0");
                kd_edit->setText("0");
            } else {
                double kp_cur = parseNumericInput(kp_edit->text().toStdString(), 24.06);
                double wn = PRECISION.step(std::sqrt(std::max(1e-9, a0 + k0 * kp_cur)));
                double kd_new = PRECISION.step((2.0 * z * wn - a1) / k0);
                kd_edit->setText(QString::fromStdString(PRECISION.fmt(kd_new)));
                ki_edit->setText("0");
            }
            runSimulation(true);
        }
    }

    void applyPolyNumDen() {
        try {
            auto pn = parsePolynomialInput(num_edit->text().toStdString());
            auto pd = parsePolynomialInput(den_edit->text().toStdString());
            if (std::abs(pd.leading_gain) <= 1e-14)
                throw std::runtime_error("Divide by zero: Den(s) cannot be 0.");
            if (std::abs(pn.leading_gain) <= 1e-14)
                throw std::runtime_error("Zero numerator: Num(s) cannot be 0.");
            model->zeros = pn.roots;
            model->poles = pd.roots;

            if (model->zeros.size() > model->poles.size()) {
                poly_status_lbl->setStyleSheet("color: #f59e0b; font-size: 10px; font-weight: bold; border:none;");
                poly_status_lbl->setText(QString("⚠ Improper Plant G(s): deg(Num)=%1 > deg(Den)=%2 (Non-causal derivative action)")
                    .arg(model->zeros.size()).arg(model->poles.size()));
            } else {
                poly_status_lbl->setStyleSheet("color: #94a3b8; font-size: 10px; border:none;");
                poly_status_lbl->setText(QString("✔ Solved %1 Poles & %2 Zeros (K = %3)")
                    .arg(model->poles.size()).arg(model->zeros.size())
                    .arg(QString::fromStdString(PRECISION.fmt(model->plant_gain))));
            }

            double ratio = pn.leading_gain / std::max(std::abs(pd.leading_gain), 1e-12);
            if (std::abs(ratio) > 1e-12) {
                model->plant_gain = ratio;
                gain_edit->setText(QString::number(ratio, 'g', 5));
            }
            poles_card->setValues(model->poles);
            zeros_card->setValues(model->zeros);
            runSimulation(true);
        } catch (const std::exception& e) {
            poly_status_lbl->setText(QString("⚠ %1").arg(e.what()));
        }
    }

    void loadFromModel() {
        updating_ui = true;
        poles_card->setValues(model->poles);
        zeros_card->setValues(model->zeros);
        num_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->zeros, "s")));
        den_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->poles, "s")));
        gain_edit->setText(QString::number(model->plant_gain, 'g', 4));
        mag_edit->setText(QString::number(model->input_magnitude, 'g', 4));
        enable_pid_chk->setChecked(model->pid_enabled);
        kp_edit->setText(QString::number(model->kp, 'g', 4));
        ki_edit->setText(QString::number(model->ki, 'g', 4));
        kd_edit->setText(QString::number(model->kd, 'g', 4));
        kp_slider->setValue(static_cast<int>(model->kp * 100));
        ki_slider->setValue(static_cast<int>(model->ki * 100));
        kd_slider->setValue(static_cast<int>(model->kd * 100));
        updating_ui = false;
        updatePidWidgetsState();
        runSimulation(false);
        fb_bar->loadFromModelS(model);
    }

    void runSimulation(bool notify) {
        if (updating_ui) return;
        cfg.poles = model->poles;
        cfg.zeros = model->zeros;
        cfg.plant_gain = parseNumericInput(gain_edit->text().toStdString(), 1.0);

        // ---> IMPROPER TRANSFER FUNCTION CHECK (WITH AUTO-CLEAR) <---
        bool is_improper = (cfg.zeros.size() > cfg.poles.size());
        if (is_improper) {
            poly_status_lbl->setStyleSheet("color: #f59e0b; font-size: 10px; font-weight: bold; border:none;");
            poly_status_lbl->setText(QString("⚠ Improper Plant G(s): deg(Num)=%1 > deg(Den)=%2 (Non-causal in time domain)")
                .arg(cfg.zeros.size()).arg(cfg.poles.size()));
        } else if (poly_status_lbl->text().startsWith("⚠ Improper")) {
            poly_status_lbl->setStyleSheet(QString("color: %1; font-size: 10px; border:none;").arg(Theme::TEXT_SECONDARY));
            poly_status_lbl->setText(QString("✔ Active: %1 Poles & %2 Zeros (K = %3)")
                .arg(cfg.poles.size()).arg(cfg.zeros.size())
                .arg(QString::fromStdString(PRECISION.fmt(cfg.plant_gain))));
        }
        // ---> END IMPROPER CHECK <---

        cfg.unity_feedback = model->unity_feedback_s;
        cfg.h_poles = model->h_poles;
        cfg.h_zeros = model->h_zeros;
        cfg.h_gain  = model->h_gain;

        tf_math_w->setLatex(QString::fromStdString(fb_bar->formatActiveLatexS(model, true)));
        cfg.input_magnitude = parseNumericInput(mag_edit->text().toStdString(), 1.0);

        int mode_idx = loop_mode_combo->currentIndex();
        cfg.closed_loop = (mode_idx != 2);
        cfg.pid_enabled = enable_pid_chk->isChecked() && (mode_idx == 0);

        cfg.kp = parseNumericInput(kp_edit->text().toStdString(), 1.5);
        cfg.ki = parseNumericInput(ki_edit->text().toStdString(), 0.0);
        cfg.kd = parseNumericInput(kd_edit->text().toStdString(), 0.0);
        cfg.signal_type = sig_combo->currentText().toStdString();
        cfg.cursor_time = parseNumericInput(cursor_edit->text().toStdString(), 2.5);
        cfg.t_max = std::max(0.2, parseNumericInput(tmax_edit->text().toStdString(), 10.0));
        cfg.settle_tol_pct = std::max(0.1, parseNumericInput(tol_edit->text().toStdString(), 2.0));

        SimulationResult res = ComplexTransferFunctionSimulator::evaluate(cfg);
        model->last_sim_result = res;

        if (notify) {
            model->plant_gain = cfg.plant_gain;
            model->input_magnitude = cfg.input_magnitude;
            model->pid_enabled = cfg.pid_enabled;
            model->kp = cfg.kp;
            model->ki = cfg.ki;
            model->kd = cfg.kd;
            model->notify("pid_sim");
        }

        // Style Compact Full-Width Stability Banner (Turns Amber if Improper OR Marginal/Unstable!)
        bool is_stable = (res.stability_state == "STABLE");
        bool banner_ok = is_stable && !is_improper;
        QString bg = banner_ok ? Theme::BANNER_OK_BG : Theme::BANNER_WARN_BG;
        QString bd = banner_ok ? Theme::BANNER_OK_BD : Theme::BANNER_WARN_BD;
        QString fg = banner_ok ? Theme::BANNER_OK_FG : Theme::BANNER_WARN_FG;
        QString mode_tag = !cfg.closed_loop ? "Open-Loop" : (cfg.pid_enabled ? "CL (PID ON)" : "CL (PID OFF, C=1)");
        if (is_improper) {
            mode_tag += QString(" • ⚠ IMPROPER G(s) (%1>%2)").arg(cfg.zeros.size()).arg(cfg.poles.size());
        }

        status_banner->setStyleSheet(QString("QFrame { background-color: %1; border: 1.5px solid %2; border-radius: 5px; }").arg(bg, bd));
        status_title_lbl->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border: none;").arg(fg));
        status_right_lbl->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; font-size: 11px; border: none;").arg(Theme::TEXT_PRIMARY));

        status_title_lbl->setText(QString("%1 %2 • %3 • %4")
            .arg(banner_ok ? "✔" : "⚠")
            .arg(QString::fromStdString(res.stability_state))
            .arg(mode_tag)
            .arg(QString::fromStdString(res.damping_regime)));
        status_right_lbl->setText(QString("max Re(s) = %1").arg(QString::fromStdString(PRECISION.fmt(res.max_real_pole, true))));

        m_card1->setText(QString("%OS: %1% (Nise: %2%) | Peak: %3\nS.S.E.: %4 | Rel. Err: %5%")
            .arg(QString::fromStdString(PRECISION.fmt(res.overshoot_pct)))
            .arg(QString::fromStdString(PRECISION.fmt(res.theo_os_pct)))
            .arg(QString::fromStdString(PRECISION.fmt(res.peak_value)))
            .arg(QString::fromStdString(PRECISION.fmt(res.steady_state_error)))
            .arg(QString::fromStdString(PRECISION.fmt(res.relative_error_pct))));

        m_card2->setText(QString("Tr: %1s (Nise: %2s) | Tp: %3s\nTs(±2%): %4s (4/σd: %5s) | Ts(±5%): %6s")
            .arg(QString::fromStdString(PRECISION.fmt(res.rise_time)))
            .arg(QString::fromStdString(PRECISION.fmt(res.theo_tr_sec)))
            .arg(QString::fromStdString(PRECISION.fmt(res.peak_time)))
            .arg(QString::fromStdString(PRECISION.fmt(res.settling_time, false, false, ">Tmax")))
            .arg(QString::fromStdString(PRECISION.fmt(res.theo_ts2_sec)))
            .arg(QString::fromStdString(PRECISION.fmt(res.settling_time_5pct, false, false, ">Tmax"))));

        m_card3->setText(QString("ζ: %1 (OL: %2) | ωn: %3 r/s\nωd: %4 r/s | σd: %5 s⁻¹ (τ=%6s)")
            .arg(QString::fromStdString(PRECISION.fmt(res.zeta)))
            .arg(QString::fromStdString(PRECISION.fmt(res.ol_zeta)))
            .arg(QString::fromStdString(PRECISION.fmt(res.omega_n)))
            .arg(QString::fromStdString(PRECISION.fmt(res.omega_d)))
            .arg(QString::fromStdString(PRECISION.fmt(res.sigma_d)))
            .arg(QString::fromStdString(PRECISION.fmt(res.time_constant_tau))));

        m_card4->setText(QString("y_ss: %1 (M=%2) | Type: %3\nKp,pos: %4 | Kv: %5 | T(0): %6")
            .arg(QString::fromStdString(PRECISION.fmtComplex(res.y_steady)))
            .arg(QString::fromStdString(PRECISION.fmt(cfg.input_magnitude)))
            .arg(res.system_type)
            .arg(QString::fromStdString(PRECISION.fmt(res.kp_pos)))
            .arg(QString::fromStdString(PRECISION.fmt(res.kv_vel)))
            .arg(QString::fromStdString(PRECISION.fmt(res.dc_gain_cl))));

        std::string cl_p_s, ol_p_s;
        for (size_t i = 0; i < std::min<size_t>(3, res.cl_poles.size()); ++i) cl_p_s += (i ? ", " : "") + PRECISION.fmtComplex(res.cl_poles[i]);
        for (size_t i = 0; i < std::min<size_t>(3, res.ol_poles.size()); ++i) ol_p_s += (i ? ", " : "") + PRECISION.fmtComplex(res.ol_poles[i]);
        m_card5->setText(QString("CL Poles: [%1]\nOL Poles: [%2]").arg(QString::fromStdString(cl_p_s), QString::fromStdString(ol_p_s)));

        m_card6->setText(QString("y(%1s): %2 | e(t): %3 (%4%)\nu(%1s): %5 | r(t): %6")
            .arg(QString::fromStdString(PRECISION.fmt(res.cursor_t)))
            .arg(QString::fromStdString(PRECISION.fmtComplex(res.cursor_y)))
            .arg(QString::fromStdString(PRECISION.fmt(res.cursor_abs_err)))
            .arg(QString::fromStdString(PRECISION.fmt(res.cursor_rel_err_pct)))
            .arg(QString::fromStdString(PRECISION.fmtComplex(res.cursor_u)))
            .arg(QString::fromStdString(PRECISION.fmt(res.cursor_r))));

        canvas->sim_res = res;
        canvas->show_cl = cfg.closed_loop;
        if (!cfg.closed_loop) canvas->show_ol = true;
        canvas->settle_tol_pct = cfg.settle_tol_pct;
        canvas->update();
    }
};

