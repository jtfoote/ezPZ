#pragma once
#include "RouthHurwitzTab.h"


class AppSettingsDialog : public QDialog {
    Q_OBJECT
public:
    SharedSystemModel* model;
    std::function<void()> onSettingsChanged;
    std::function<void()> onResetDefaults;

    QComboBox* mode_combo;
    QSpinBox *sub1_spin, *ge1_spin;
    QCheckBox *step_round_chk, *sync_chk;
    QLabel* preview_lbl;

    AppSettingsDialog(SharedSystemModel* m,
                      std::function<void()> change_cb,
                      std::function<void()> reset_cb,
                      QWidget* parent = nullptr)
        : QDialog(parent), model(m),
          onSettingsChanged(std::move(change_cb)),
          onResetDefaults(std::move(reset_cb))
    {
        setWindowTitle("Application Settings & Precision Preferences");
        setMinimumWidth(480);
        setStyleSheet(QString("QDialog { background-color: %1; }").arg(Theme::BG_WINDOW));

        auto* main_l = new QVBoxLayout(this);
        main_l->setContentsMargins(16, 14, 16, 14);
        main_l->setSpacing(10);

        // 1. Decimal Precision & Classroom Rounding Card
        auto* prec_card = makeCard();
        auto* pl = new QVBoxLayout(prec_card);
        pl->setContentsMargins(12, 10, 12, 10);
        pl->setSpacing(8);

        auto* pt = new QLabel("1. Numerical Formatting & Classroom Rounding Rule");
        pt->setStyleSheet(QString("color: %1; font-size: 12px; font-weight: bold; border: none;").arg(Theme::ACCENT_ZERO));
        pl->addWidget(pt);

        auto* pgrid = new QGridLayout();
        auto* m_lbl = new QLabel("Rounding Mode:");
        m_lbl->setStyleSheet("color: #94a3b8; font-weight: bold; font-size: 11px; border: none;");
        mode_combo = new QComboBox();
        mode_combo->addItems({
            "Classroom Rule (0 < |x| < 1 vs. |x| ≥ 1)",
            "Fixed Decimal Places (All Values)",
            "Auto Precision"
        });
        if (PRECISION.mode == "class") mode_combo->setCurrentIndex(0);
        else if (PRECISION.mode == "fixed") mode_combo->setCurrentIndex(1);
        else mode_combo->setCurrentIndex(2);
        mode_combo->setStyleSheet(QString("background-color: %1; color: %2; border: 1px solid %3; padding: 4px 8px; font-weight: bold; font-size: 11px;")
                                  .arg(Theme::BG_INPUT, Theme::TEXT_PRIMARY, Theme::BORDER_INPUT));

        auto makeSpin = [](int val) {
            auto* sp = new QSpinBox();
            sp->setRange(0, 8);
            sp->setValue(val);
            sp->setStyleSheet(QString(
                "QSpinBox { background-color: %1; color: %2; border: 1px solid %3; border-radius: 4px; padding: 3px 6px; font-family: Monospace; font-weight: bold; font-size: 11px; }"
            ).arg(Theme::BG_INPUT, Theme::TEXT_PRIMARY, Theme::BORDER_INPUT));
            return sp;
        };

        auto* sub1_lbl = new QLabel("Decimals for 0 < |x| < 1:");
        sub1_lbl->setStyleSheet("color: #94a3b8; font-weight: bold; font-size: 11px; border: none;");
        sub1_spin = makeSpin(PRECISION.dec_sub1);

        auto* ge1_lbl = new QLabel("Decimals for |x| ≥ 1 (or Fixed):");
        ge1_lbl->setStyleSheet("color: #94a3b8; font-weight: bold; font-size: 11px; border: none;");
        ge1_spin = makeSpin(PRECISION.dec_ge1);

        pgrid->addWidget(m_lbl,    0, 0); pgrid->addWidget(mode_combo, 0, 1);
        pgrid->addWidget(sub1_lbl, 1, 0); pgrid->addWidget(sub1_spin,  1, 1);
        pgrid->addWidget(ge1_lbl,  2, 0); pgrid->addWidget(ge1_spin,   2, 1);
        pl->addLayout(pgrid);

        step_round_chk = new QCheckBox("Enable Intermediate Step-Rounding (Step-Round)");
        step_round_chk->setChecked(PRECISION.step_round);
        step_round_chk->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border: none;").arg(Theme::ACCENT_PID));
        pl->addWidget(step_round_chk);

        auto* sr_hint = new QLabel("When enabled, intermediate synthesis steps (such as ζ from %OS and ωn before solving Kp/Kd) are rounded at each step to match hand-worked classroom solutions.");
        sr_hint->setWordWrap(true);
        sr_hint->setStyleSheet("color: #64748b; font-size: 10px; border: none;");
        pl->addWidget(sr_hint);

        preview_lbl = new QLabel();
        preview_lbl->setStyleSheet(QString("background-color: %1; color: %2; font-family: Monospace; font-size: 11px; font-weight: bold; padding: 8px; border-radius: 4px; border: 1px solid %3;")
                                   .arg(Theme::BG_INPUT, Theme::ACCENT_ZERO, Theme::BORDER_INPUT));
        pl->addWidget(preview_lbl);
        main_l->addWidget(prec_card);

        // 2. Synchronization & System State Reset Card
        auto* app_card = makeCard();
        auto* al = new QVBoxLayout(app_card);
        al->setContentsMargins(12, 10, 12, 10);
        al->setSpacing(8);

        auto* at = new QLabel("2. Workspace Synchronization & System Reset");
        at->setStyleSheet(QString("color: %1; font-size: 12px; font-weight: bold; border: none;").arg(Theme::ACCENT_LOCUS));
        al->addWidget(at);

        sync_chk = new QCheckBox("⇄ Synchronize Plant G(s) & Filter H(z) Across Tabs 1–6");
        sync_chk->setChecked(model->sync_enabled);
        sync_chk->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border: none;").arg(Theme::TEXT_PRIMARY));
        al->addWidget(sync_chk);

        auto* reset_row = new QHBoxLayout();
        auto* reset_btn = makeActionBtn("↺ Reset All System Parameters to Textbook Defaults", Theme::ACCENT_POLE);
        reset_row->addWidget(reset_btn);
        reset_row->addStretch();
        al->addLayout(reset_row);
        main_l->addWidget(app_card);

        // Bottom Close Button
        auto* bot_row = new QHBoxLayout();
        bot_row->addStretch();
        auto* close_btn = makeActionBtn("Done / Close", "#ffffff", Theme::BG_KEY_ACCENT);
        bot_row->addWidget(close_btn);
        main_l->addLayout(bot_row);

        auto applyChanges = [this]() {
            int idx = mode_combo->currentIndex();
            PRECISION.mode = (idx == 0) ? "class" : ((idx == 1) ? "fixed" : "auto");
            PRECISION.dec_sub1 = sub1_spin->value();
            PRECISION.dec_ge1  = ge1_spin->value();
            PRECISION.step_round = step_round_chk->isChecked();
            model->sync_enabled  = sync_chk->isChecked();

            sub1_spin->setEnabled(idx == 0);
            ge1_spin->setEnabled(idx != 2);

            preview_lbl->setText(QString("Active Rule: %1\nSample < 1 : 0.45598  →  %2\nSample ≥ 1 : 24.0642  →  %3")
                .arg(QString::fromStdString(PRECISION.summaryLabel()))
                .arg(QString::fromStdString(PRECISION.fmt(0.45598)))
                .arg(QString::fromStdString(PRECISION.fmt(24.0642))));

            if (onSettingsChanged) onSettingsChanged();
        };

        connect(mode_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [applyChanges](int) { applyChanges(); });
        connect(sub1_spin,  QOverload<int>::of(&QSpinBox::valueChanged),         this, [applyChanges](int) { applyChanges(); });
        connect(ge1_spin,   QOverload<int>::of(&QSpinBox::valueChanged),         this, [applyChanges](int) { applyChanges(); });
        connect(step_round_chk, &QCheckBox::toggled, this, [applyChanges](bool) { applyChanges(); });
        connect(sync_chk,       &QCheckBox::toggled, this, [applyChanges](bool) { applyChanges(); });

        connect(reset_btn, &QPushButton::clicked, this, [this]() {
            if (onResetDefaults) onResetDefaults();
        });
        connect(close_btn, &QPushButton::clicked, this, &QDialog::accept);

        applyChanges();
    }
};

