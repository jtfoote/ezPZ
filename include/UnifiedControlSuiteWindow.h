#pragma once
#include "AppSettingsDialog.h"
#include "SignalPlotterTab.h"


// =============================================================================
// Main Unified Application Window (All 7 Tabs + Precision Bar)
// =============================================================================
class UnifiedControlSuiteWindow : public QMainWindow {
    Q_OBJECT
public:
    SharedSystemModel model;
    QStackedWidget* stack;
    // std::vector<QPushButton*> tab_btns;
    QComboBox* tab_combo = nullptr;

    RootLocusTab* rl_tab;
    PIDTimeResponseTab* pid_tab;
    ZPlaneFilterTab* zp_tab;
    DiscreteTimeResponseTab* dsim_tab;
    ScientificCalculatorTab* calc_tab;
    FormulaCheatSheetTab* cheat_tab;
    BlockDiagramStudioTab* block_tab;
    FrequencyResponseTab* freq_tab;
    RouthHurwitzTab* routh_tab;
    SignalPlotterTab* sig_tab;

    explicit UnifiedControlSuiteWindow(QWidget* parent = nullptr) : QMainWindow(parent) {
        setWindowTitle("ezPZ - Control & DSP Workbench");
        setWindowIcon(QIcon(":/ezpz.svg"));
        resize(900, 600);
        setStyleSheet(QString("QMainWindow { background-color: %1; }").arg(Theme::BG_WINDOW));

        QSettings settings("ControlSuite", "ezPZ");
        model.loadFromSettings(settings);
        int saved_theme = model.theme_index;
        Theme::setTheme(0);

        auto* central = new QWidget(this);
        setCentralWidget(central);
        auto* main_v = new QVBoxLayout(central);
        main_v->setContentsMargins(12, 8, 12, 10);

        // Top Navigation Bar
        auto* nav = makeCard();
        nav->setStyleSheet(QString("background-color: %1; border: 1px solid %2; border-radius: 6px; padding: 5px;")
                           .arg(Theme::BG_HEADER, Theme::BORDER_PANEL));
        auto* nav_l = new QHBoxLayout(nav);

        auto* brand = new QLabel("Control Systems & Digital Filter Suite");
        brand->setStyleSheet(QString("color: %1; font-size: 13px; font-weight: bold; border:none;").arg(Theme::TEXT_PRIMARY));
        nav_l->addWidget(brand);
        nav_l->addSpacing(8);

        const std::vector<QString> tab_names = {
            "1. s-Plane Locus",
            "2. Time & PID Sim",
            "3. z-Plane & Filters",
            "4. Discrete Time Sim",
            "5. Scientific Calc",
            "6. Formulas & Cheat Sheet",
            "7. Block Diagram",
            "8. Bode & Nyquist",
            "9. Routh-Hurwitz",
            "10. Signal & Spectrum"
        };

        auto* ws_lbl = new QLabel("Workspace:");
        ws_lbl->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 12px; border: none;").arg(Theme::TEXT_MUTED));
        nav_l->addWidget(ws_lbl);

        tab_combo = new QComboBox();
        tab_combo->setCursor(Qt::PointingHandCursor);
        tab_combo->setMinimumWidth(240);
        for (const auto& name : tab_names) {
            tab_combo->addItem(name);
        }
        tab_combo->setStyleSheet(QString(
            "QComboBox {"
            "  background-color: %1; color: %2; border: 1px solid %3;"
            "  border-radius: 5px; padding: 5px 12px; font-weight: bold; font-size: 12px;"
            "}"
            "QComboBox:hover { border: 1px solid %4; }"
            "QComboBox::drop-down { border: none; width: 24px; }"
            "QComboBox QAbstractItemView {"
            "  background-color: %1; color: %2; selection-background-color: %4;"
            "  selection-color: #ffffff; border: 1px solid %3; padding: 4px; outline: 0;"
            "}"
        ).arg(Theme::BG_INPUT, Theme::TEXT_PRIMARY, Theme::BORDER_INPUT, Theme::BG_KEY_ACCENT));

        connect(tab_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
            if (idx >= 0) switchTab(idx);
        });
        nav_l->addWidget(tab_combo);

        // Optional: Quick Prev / Next tab buttons right next to the dropdown
        auto* prev_tab_btn = makeActionBtn("◀", Theme::TEXT_PRIMARY, Theme::BG_INPUT);
        auto* next_tab_btn = makeActionBtn("▶", Theme::TEXT_PRIMARY, Theme::BG_INPUT);
        prev_tab_btn->setToolTip("Previous Workspace Tab");
        next_tab_btn->setToolTip("Next Workspace Tab");
        connect(prev_tab_btn, &QPushButton::clicked, this, [this, tab_names]() {
            int next_i = (stack->currentIndex() - 1 + static_cast<int>(tab_names.size())) % static_cast<int>(tab_names.size());
            switchTab(next_i);
        });
        connect(next_tab_btn, &QPushButton::clicked, this, [this, tab_names]() {
            int next_i = (stack->currentIndex() + 1) % static_cast<int>(tab_names.size());
            switchTab(next_i);
        });
        nav_l->addWidget(prev_tab_btn);
        nav_l->addWidget(next_tab_btn);

        // auto stepTabPrev = [this]() {
        //     if (!stack || stack->count() == 0) return;
        //     int prev_i = (stack->currentIndex() - 1 + stack->count()) % stack->count();
        //     switchTab(prev_i);
        // };

        // auto stepTabNext = [this]() {
        //     if (!stack || stack->count() == 0) return;
        //     int next_i = (stack->currentIndex() + 1) % stack->count();
        //     switchTab(next_i);
        // };

        // Install global event filter on qApp so Alt+Left / Alt+Right always work
        if (qApp) {
            qApp->installEventFilter(this);
        }

        nav_l->addStretch();

        auto* sync_btn = makeActionBtn(
            model.sync_enabled ? "🔗 Sync: ON" : "🔓 Sync: OFF",
            model.sync_enabled ? Theme::ACCENT_PID : Theme::TEXT_MUTED,
            Theme::BG_PANEL_ALT
        );
        sync_btn->setToolTip("Toggle cross-tab transfer function G(s) / H(z) synchronization");
        connect(sync_btn, &QPushButton::clicked, this, [this, sync_btn]() {
            model.sync_enabled = !model.sync_enabled;
            sync_btn->setText(model.sync_enabled ? "🔗 Sync: ON" : "🔓 Sync: OFF");
            sync_btn->setStyleSheet(QString(
                "QPushButton { background-color: %1; color: %2; border: 1px solid %2; border-radius: 4px; padding: 4px 9px; font-weight: bold; font-size: 11px; }"
                "QPushButton:hover { background-color: %3; color: %2; border: 1px solid %2; }"
            ).arg(Theme::BG_PANEL_ALT,
                  model.sync_enabled ? Theme::ACCENT_PID : Theme::TEXT_MUTED,
                  Theme::BG_KEY_NUM));

            // If turning Sync back ON, push the currently visible tab's G(s)/H(z) to all other tabs!
            if (model.sync_enabled) {
                int cur = stack->currentIndex();
                if (cur == 0 && rl_tab) {
                    model.poles = rl_tab->poles_card->getValues();
                    model.zeros = rl_tab->zeros_card->getValues();
                    rl_tab->fb_bar->syncToModelS(&model);
                } else if (cur == 1 && pid_tab) {
                    model.poles = pid_tab->poles_card->getValues();
                    model.zeros = pid_tab->zeros_card->getValues();
                    pid_tab->fb_bar->syncToModelS(&model);
                } else if (cur == 7 && freq_tab && freq_tab->domain_combo->currentIndex() != 2) {
                    model.poles = freq_tab->poles_card->getValues();
                    model.zeros = freq_tab->zeros_card->getValues();
                    freq_tab->fb_bar->syncToModelS(&model);
                }
                refreshAllTabs();
            }
        });
        nav_l->addWidget(sync_btn);

        auto* settings_btn = makeActionBtn("⚙ Settings", Theme::TEXT_PRIMARY, Theme::BG_PANEL);
        connect(settings_btn, &QPushButton::clicked, this, [this]() {
            auto* dlg = new AppSettingsDialog(
                &model,
                [this]() { refreshAllTabs(); },
                [this]() { model.resetToDefaults(); refreshAllTabs(); },
                this,
                [this](int new_theme_idx) {
                    applyThemeLive(new_theme_idx);
                }
            );
            dlg->setAttribute(Qt::WA_DeleteOnClose);
            dlg->show();
        });
        nav_l->addWidget(settings_btn);
        main_v->addWidget(nav);

        // 7-Tab Stack
        stack = new QStackedWidget();
        rl_tab = new RootLocusTab(&model, [this](double k_val) {
            model.pid_enabled = true;
            model.kp = k_val; model.ki = 0.0; model.kd = 0.0;
            pid_tab->loadFromModel();
            switchTab(1);
        });
        pid_tab = new PIDTimeResponseTab(&model);
        zp_tab  = new ZPlaneFilterTab(&model, [this](double kd_val) {
            model.filter_gain = kd_val;
            dsim_tab->loadFromModel();
            switchTab(3);
        });
        dsim_tab  = new DiscreteTimeResponseTab(&model);
        calc_tab  = new ScientificCalculatorTab(&model);
        cheat_tab = new FormulaCheatSheetTab(&model);
        block_tab = new BlockDiagramStudioTab(&model, [this](const std::vector<Complex>& p, const std::vector<Complex>& z, double k) {
            model.poles = p; model.zeros = z; model.plant_gain = k;
            if (!p.empty()) model.test_point = p[0];
            refreshAllTabs();
        });
        freq_tab = new FrequencyResponseTab(&model);
        routh_tab = new RouthHurwitzTab(&model);
        sig_tab = new SignalPlotterTab(&model);

        stack->addWidget(rl_tab);
        stack->addWidget(pid_tab);
        stack->addWidget(zp_tab);
        stack->addWidget(dsim_tab);
        stack->addWidget(calc_tab);
        stack->addWidget(cheat_tab);
        stack->addWidget(block_tab);
        stack->addWidget(freq_tab);
        stack->addWidget(routh_tab);
        stack->addWidget(sig_tab);
        main_v->addWidget(stack, 1);

        model.listeners.push_back([this](const std::string& src) {
            if (src != "root_locus")   rl_tab->loadFromModel();
            if (src != "pid_sim")      pid_tab->loadFromModel();
            if (src != "z_plane")      zp_tab->loadFromModel();
            if (src != "discrete_sim") dsim_tab->loadFromModel();
            if (src != "freq_resp")    freq_tab->loadFromModel();
            calc_tab->evaluateAll();
            cheat_tab->refreshBanner();
            routh_tab->loadFromModel();
        });

        if (settings.contains("calc/expressions")) {
            calc_tab->setExpressions(settings.value("calc/expressions").toStringList());
        }
        if (settings.contains("win/geometry")) {
            restoreGeometry(settings.value("win/geometry").toByteArray());
        }
        if (QScreen* screen = QGuiApplication::primaryScreen()) {
            QRect avail = screen->availableGeometry();
            if (width() > avail.width() || height() > avail.height()) {
                resize(std::min(width(), avail.width()), std::min(height(), avail.height()));
                move(avail.topLeft());
            }
        }
        int last_tab = std::clamp(settings.value("win/active_tab", 0).toInt(), 0, 9);
        switchTab(last_tab);

        applyThemeLive(saved_theme, 0);
    }

    void activateIndependentTabState(int idx) {
        if (model.sync_enabled) return; // When synced, all tabs already share model

        // Push the active tab's own UI cards/inputs into model without notifying other tabs
        if (idx == 0 && rl_tab) {
            model.poles = rl_tab->poles_card->getValues();
            model.zeros = rl_tab->zeros_card->getValues();
            rl_tab->fb_bar->syncToModelS(&model);
            rl_tab->evaluateCurrent(false);
        } else if (idx == 1 && pid_tab) {
            model.poles = pid_tab->poles_card->getValues();
            model.zeros = pid_tab->zeros_card->getValues();
            pid_tab->fb_bar->syncToModelS(&model);
            pid_tab->runSimulation(false);
        } else if (idx == 2 && zp_tab) {
            model.z_poles = zp_tab->poles_card->getValues();
            model.z_zeros = zp_tab->zeros_card->getValues();
            zp_tab->fb_bar->syncToModelZ(&model);
            zp_tab->evaluateCurrent(false);
        } else if (idx == 3 && dsim_tab) {
            model.z_poles = dsim_tab->poles_card->getValues();
            model.z_zeros = dsim_tab->zeros_card->getValues();
            dsim_tab->fb_bar->syncToModelZ(&model);
            dsim_tab->runSimulation(false);
        } else if (idx == 7 && freq_tab) {
            if (freq_tab->domain_combo->currentIndex() != 2) {
                model.poles = freq_tab->poles_card->getValues();
                model.zeros = freq_tab->zeros_card->getValues();
                freq_tab->fb_bar->syncToModelS(&model);
            } else {
                model.z_poles = freq_tab->poles_card->getValues();
                model.z_zeros = freq_tab->zeros_card->getValues();
                freq_tab->fb_bar->syncToModelZ(&model);
            }
            freq_tab->evaluateCurrent(false);
        }
    }

    void switchTab(int idx) {
        if (idx < 0 || idx >= stack->count()) return;
        stack->setCurrentIndex(idx);
        if (tab_combo) {
            QSignalBlocker blocker(tab_combo);
            tab_combo->setCurrentIndex(idx);
        }
        activateIndependentTabState(idx);
    }

    void refreshAllTabs() {
        rl_tab->loadFromModel();
        pid_tab->loadFromModel();
        zp_tab->loadFromModel();
        dsim_tab->loadFromModel();
        freq_tab->loadFromModel();
        calc_tab->evaluateAll();
        cheat_tab->refreshBanner();
        routh_tab->loadFromModel();
    }
    
    void applyThemeLive(int target_theme_idx, int source_theme_idx = -1) {
        if (source_theme_idx >= 0) {
            Theme::setTheme(source_theme_idx);
        }
        std::vector<QString> old_c = Theme::currentPaletteVector();

        Theme::setTheme(target_theme_idx);
        model.theme_index = target_theme_idx;
        model.light_mode  = Theme::is_light;

        std::vector<QString> new_c = Theme::currentPaletteVector();

        auto swapSheetStr = [&](QString ss) -> QString {
            if (ss.isEmpty()) return ss;
            for (size_t i = 0; i < old_c.size(); ++i) {
                ss.replace(old_c[i], QString("__THEME_TOK_%1__").arg(i), Qt::CaseInsensitive);
            }
            for (size_t i = 0; i < new_c.size(); ++i) {
                ss.replace(QString("__THEME_TOK_%1__").arg(i), new_c[i]);
            }
            return ss;
        };

        if (qApp) {
            QPalette pal = qApp->palette();
            pal.setColor(QPalette::Window, QColor(Theme::BG_WINDOW));
            pal.setColor(QPalette::WindowText, QColor(Theme::TEXT_PRIMARY));
            pal.setColor(QPalette::Base, QColor(Theme::BG_INPUT));
            pal.setColor(QPalette::AlternateBase, QColor(Theme::BG_PANEL_ALT));
            pal.setColor(QPalette::Text, QColor(Theme::TEXT_PRIMARY));
            pal.setColor(QPalette::Button, QColor(Theme::BG_INPUT));
            pal.setColor(QPalette::ButtonText, QColor(Theme::TEXT_PRIMARY));
            pal.setColor(QPalette::Highlight, QColor(Theme::ACCENT_ZERO));
            pal.setColor(QPalette::HighlightedText, QColor(Theme::BG_WINDOW));
            pal.setColor(QPalette::Mid, QColor(Theme::BORDER_INPUT));
            pal.setColor(QPalette::Dark, QColor(Theme::BORDER_INPUT));
            qApp->setPalette(pal);

            QString global_controls_ss = QString(
                "QSlider::groove:horizontal { "
                "    height: 6px; background: %1; border: 1px solid %2; border-radius: 3px; "
                "} "
                "QSlider::sub-page:horizontal { "
                "    background: %3; border-radius: 3px; "
                "} "
                "QSlider::handle:horizontal { "
                "    background: %4; border: 1px solid %3; width: 12px; margin: -4px 0; border-radius: 6px; "
                "} "
                "QSlider::groove:horizontal:disabled { background: %1; border-color: %2; } "
                "QSlider::sub-page:horizontal:disabled { background: %2; } "
                "QSlider::handle:horizontal:disabled { background: %2; border-color: %2; } "
            ).arg(Theme::BG_PANEL_ALT, Theme::BORDER_INPUT, Theme::ACCENT_ZERO, Theme::TEXT_PRIMARY);

            qApp->setStyleSheet(global_controls_ss);
        }

        this->setStyleSheet(swapSheetStr(this->styleSheet()));
        for (QWidget* w : this->findChildren<QWidget*>()) {
            if (!w->styleSheet().isEmpty()) {
                w->setStyleSheet(swapSheetStr(w->styleSheet()));
            }
            if (auto* mw = dynamic_cast<MathFormulaWidget*>(w)) {
                if (mw->bg_color.isValid() && mw->bg_color.alpha() > 0) {
                    mw->bg_color = QColor(swapSheetStr(mw->bg_color.name()));
                }
                mw->fg_color = QColor(swapSheetStr(mw->fg_color.name()));
            }
            w->update();
        }
    }

    protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
        if (event->type() == QEvent::ShortcutOverride || event->type() == QEvent::KeyPress) {
            auto* ke = static_cast<QKeyEvent*>(event);
            // Check that ONLY the Alt modifier is held
            if ((ke->modifiers() & (Qt::AltModifier | Qt::ControlModifier | Qt::ShiftModifier | Qt::MetaModifier)) == Qt::AltModifier) {
                if (ke->key() == Qt::Key_Left || ke->key() == Qt::Key_Right) {
                    if (event->type() == QEvent::ShortcutOverride) {
                        // Tell Qt not to let QLineEdit/QComboBox swallow Alt+Left/Right
                        event->accept();
                        return true;
                    }
                    // On the actual KeyPress, switch the tab
                    if (stack && stack->count() > 0 && this->isActiveWindow()) {
                        int delta = (ke->key() == Qt::Key_Left) ? -1 : 1;
                        int next_i = (stack->currentIndex() + delta + stack->count()) % stack->count();
                        switchTab(next_i);
                        event->accept();
                        return true;
                    }
                }
            }
        }
        return QMainWindow::eventFilter(watched, event);
    }

    void closeEvent(QCloseEvent* event) override {
        QSettings settings("ControlSuite", "ezPZ");
        model.saveToSettings(settings);
        settings.setValue("calc/expressions", calc_tab->getExpressions());
        settings.setValue("win/active_tab", stack->currentIndex());
        settings.setValue("win/geometry", saveGeometry());
        QMainWindow::closeEvent(event);
    }
};


// =============================================================================
// CLI Runner & Entry Point
// =============================================================================
inline int runCli(int argc, char* argv[]) {
    SharedSystemModel model;
    bool do_sim = false, do_dsim = false, no_pid = false;
    std::optional<Complex> test_pt;
    std::vector<std::string> calc_exprs;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        auto nextVal = [&]() -> std::string { return (i + 1 < argc) ? argv[++i] : ""; };

        if (arg == "--num") model.zeros = parsePolynomialInput(nextVal()).roots;
        else if (arg == "--den") model.poles = parsePolynomialInput(nextVal()).roots;
        else if (arg == "--znum") model.z_zeros = parsePolynomialInput(nextVal()).roots;
        else if (arg == "--zden") model.z_poles = parsePolynomialInput(nextVal()).roots;
        else if (arg == "--gain") model.plant_gain = std::stod(nextVal());
        else if (arg == "--rmag") model.input_magnitude = std::stod(nextVal());
        else if (arg == "--kp") model.kp = std::stod(nextVal());
        else if (arg == "--ki") model.ki = std::stod(nextVal());
        else if (arg == "--kd") model.kd = std::stod(nextVal());
        else if (arg == "--no-pid") { no_pid = true; model.pid_enabled = false; model.d_pid_enabled = false; }
        else if (arg == "--simulate") do_sim = true;
        else if (arg == "--dsimulate") do_dsim = true;
        else if (arg == "--point") {
            std::string p = nextVal();
            auto comma = p.find(',');
            double re = std::stod(p.substr(0, comma));
            double im = (comma != std::string::npos) ? std::stod(p.substr(comma + 1)) : 0.0;
            test_pt = Complex(re, im);
        } else if (arg == "--calc") {
            while (i + 1 < argc && argv[i + 1][0] != '-') calc_exprs.push_back(argv[++i]);
        }
    }

    if (test_pt.has_value()) {
        auto ev = evaluateTransferFunction(model.poles, model.zeros, test_pt.value(), 1.0);
        std::cout << "=== 1. s-Plane Root Locus Evaluation ===\n"
                  << "  Test Point s : " << PRECISION.fmtComplex(test_pt.value()) << "\n"
                  << "  Angle ∠G(s)  : " << PRECISION.fmt(ev.angle_deg) << " deg\n"
                  << "  Mag |G(s)|   : " << PRECISION.fmt(ev.magnitude) << "\n"
                  << "  RL Gain K    : " << PRECISION.fmt(ev.rl_gain_k) << "\n";
    }
    if (do_sim) {
        SimulationConfig cfg;
        cfg.poles = model.poles; cfg.zeros = model.zeros;
        cfg.plant_gain = model.plant_gain; cfg.input_magnitude = model.input_magnitude;
        cfg.pid_enabled = !no_pid;
        cfg.kp = model.kp; cfg.ki = model.ki; cfg.kd = model.kd;
        auto res = ComplexTransferFunctionSimulator::evaluate(cfg);
        model.last_sim_result = res;
        std::cout << "=== 2. Continuous Time Response Simulation ===\n"
                  << "  Controller   : " << (cfg.pid_enabled ? "PID Active" : "PID Disabled (C(s) = 1)") << "\n"
                  << "  Stability    : " << res.stability_state << " (" << res.damping_regime << ")\n"
                  << "  Damping ζ    : " << PRECISION.fmt(res.zeta) << " | ωn = " << PRECISION.fmt(res.omega_n) << " rad/s\n"
                  << "  Overshoot    : " << PRECISION.fmt(res.overshoot_pct) << "% | Peak = " << PRECISION.fmt(res.peak_value) << "\n"
                  << "  Settling Ts  : " << PRECISION.fmt(res.settling_time) << " s | Rise Tr = " << PRECISION.fmt(res.rise_time) << " s\n"
                  << "  S.S.E.       : " << PRECISION.fmt(res.steady_state_error) << " (" << PRECISION.fmt(res.relative_error_pct) << "%)\n";
    }
    if (do_dsim) {
        DiscreteSimulationConfig dcfg;
        dcfg.z_poles = model.z_poles; dcfg.z_zeros = model.z_zeros;
        dcfg.filter_gain = model.filter_gain; dcfg.ts = model.ts;
        dcfg.pid_enabled = !no_pid;
        auto dres = DiscreteFilterSimulator::evaluate(dcfg);
        std::cout << "=== 4. Discrete Time Response Simulation ===\n"
                  << "  Diff Eq      : " << dres.diff_eq_str << "\n"
                  << "  Stability    : " << dres.stability_state << " (max|z| = " << PRECISION.fmt(dres.max_pole_mag) << ")\n"
                  << "  S.S.E.       : " << PRECISION.fmt(dres.steady_state_error) << "\n";
    }
    if (!calc_exprs.empty()) {
        ScientificCalcEngine eng(&model);
        std::cout << "=== 5. Scientific Calculator ===\n";
        for (const auto& ex : calc_exprs) {
            auto out = eng.evalLine(ex);
            std::cout << "  " << ex << "  -->  " << (out.ok ? out.formatted_dec : ("ERROR: " + out.error_msg)) << "\n";
        }
    }
    return 0;
}
