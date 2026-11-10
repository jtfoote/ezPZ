#pragma once
#include "TimeResponseCanvas.h"
#include <QDialog>
#include <QStackedWidget>
#include <QButtonGroup>

// =============================================================================
// Polynomial Formatting Helper for Symbolic & Numerical Closed-Loop T(s)
// =============================================================================
inline std::string formatComplexPolyDesc(const std::vector<Complex>& coeffs, const std::string& var = "s") {
    int deg = static_cast<int>(coeffs.size()) - 1;
    std::string out;
    for (int i = 0; i <= deg; ++i) {
        double c = coeffs[i].real();
        if (std::abs(c) < 1e-9) continue;
        int p = deg - i;
        bool neg = (c < 0.0);
        double ac = std::abs(c);
        if (!out.empty()) out += neg ? " - " : " + ";
        else if (neg) out += "-";

        bool show_num = (std::abs(ac - 1.0) > 1e-7) || (p == 0);
        if (show_num) out += PRECISION.fmt(ac);
        if (p >= 1) {
            out += var;
            if (p >= 2) out += "^" + std::to_string(p);
        }
    }
    return out.empty() ? "0" : out;
}

// =============================================================================
// Tab 1–5 Reusable Feedback Path Bar: s-Plane H(s) & z-Plane H_fb(z)
// =============================================================================
class FeedbackConfigBar : public QWidget {
    Q_OBJECT
public:
    QRadioButton *unity_rb, *custom_rb;
    QPushButton *view_mode_btn;
    bool show_closed_loop_ts = false;
    QLineEdit *h_num_edit, *h_den_edit;
    QWidget *custom_row;
    QString domain_var;
    bool updating = false;
    bool p_cards_authoritative = false;
    std::function<void()> onChanged;
    PoleZeroEditorCard* h_poles_card = nullptr;
    PoleZeroEditorCard* h_zeros_card = nullptr;

    explicit FeedbackConfigBar(const QString& var_sym = "s", const QString& fb_sym = "H", QWidget* parent = nullptr)
        : QWidget(parent), domain_var(var_sym)
    {
        auto* vl = new QVBoxLayout(this);
        vl->setContentsMargins(0, 2, 0, 2);
        vl->setSpacing(3);

        auto* top_r = new QHBoxLayout();
        auto* lbl = new QLabel(QString("Feedback Path %1(%2):").arg(fb_sym, var_sym));
        lbl->setStyleSheet("color: #94a3b8; font-size: 10px; font-weight: bold; border: none;");
        unity_rb  = new QRadioButton(QString("Unity (%1 = 1)").arg(fb_sym));
        custom_rb = new QRadioButton(QString("Custom %1(%2)").arg(fb_sym, var_sym));
        unity_rb->setChecked(true);
        for (auto* rb : {unity_rb, custom_rb}) {
            rb->setStyleSheet(QString("QRadioButton { color: %1; font-weight: bold; font-size: 10px; border: none; }").arg(Theme::TEXT_PRIMARY));
        }

        // Toggle between displaying G(s) & H(s) vs. Closed-Loop T(s)
        QString fwd_sym = (var_sym == "s") ? "G(s), H(s)" : "H(z), H_fb(z)";
        view_mode_btn = new QPushButton(QString("Formula: %1").arg(fwd_sym));
        view_mode_btn->setCursor(Qt::PointingHandCursor);
        view_mode_btn->setStyleSheet(QString(
            "QPushButton { background-color: %1; color: %2; border: 1px solid %3; border-radius: 4px; padding: 2px 8px; font-size: 10px; font-weight: bold; }"
            "QPushButton:hover { border-color: %2; }"
        ).arg(Theme::BG_INPUT, Theme::ACCENT_ZERO, Theme::BORDER_INPUT));

        top_r->addWidget(lbl);
        top_r->addWidget(unity_rb);
        top_r->addWidget(custom_rb);
        top_r->addStretch();
        top_r->addWidget(view_mode_btn);
        vl->addLayout(top_r);

        custom_row = new QWidget();
        custom_row->setStyleSheet("background: transparent; border: none;");
        auto* cr_vl = new QVBoxLayout(custom_row);
        cr_vl->setContentsMargins(0, 2, 0, 0);
        cr_vl->setSpacing(4);

        // Tier 1: Num_H and Den_H polynomial inputs
        auto* cr_l = new QHBoxLayout();
        cr_l->setContentsMargins(0, 0, 0, 0);
        cr_l->setSpacing(6);

        auto* nh_lbl = new QLabel(QString("Num_%1(%2):").arg(fb_sym, var_sym));
        nh_lbl->setStyleSheet("color: #34d399; font-family: Monospace; font-size: 10px; font-weight: bold; border:none;");
        h_num_edit = makeDarkEntry("1");

        auto* dh_lbl = new QLabel(QString("Den_%1(%2):").arg(fb_sym, var_sym));
        dh_lbl->setStyleSheet("color: #a78bfa; font-family: Monospace; font-size: 10px; font-weight: bold; border:none;");
        h_den_edit = makeDarkEntry("1");

        auto* solve_h_btn = makeActionBtn("Solve H ➔", Theme::ACCENT_PID);

        cr_l->addWidget(nh_lbl);
        cr_l->addWidget(h_num_edit, 1);
        cr_l->addWidget(dh_lbl);
        cr_l->addWidget(h_den_edit, 1);
        cr_l->addWidget(solve_h_btn);
        cr_vl->addLayout(cr_l);

        // Tier 2: Editable H(s) / H_fb(z) Poles (×) & Zeros (○) Tables
        auto* hpz_row = new QHBoxLayout();
        hpz_row->setContentsMargins(0, 2, 0, 0);
        hpz_row->setSpacing(6);
        QString h_tag = QString("%1(%2)").arg(fb_sym, var_sym);
        double def_hp = (var_sym == "z") ? 0.5 : -2.0;
        double def_hz = (var_sym == "z") ? 0.2 : -4.0;
        h_poles_card = new PoleZeroEditorCard(h_tag + " Poles (×)", Theme::ACCENT_POLE, def_hp);
        h_zeros_card = new PoleZeroEditorCard(h_tag + " Zeros (○)", Theme::ACCENT_ZERO, def_hz);
        hpz_row->addWidget(h_poles_card, 1);
        hpz_row->addWidget(h_zeros_card, 1);
        cr_vl->addLayout(hpz_row);

        custom_row->setVisible(false);
        vl->addWidget(custom_row);

        connect(unity_rb, &QRadioButton::toggled, this, [this](bool is_unity) {
            custom_row->setVisible(!is_unity);
            if (!updating && onChanged) onChanged();
        });
        connect(view_mode_btn, &QPushButton::clicked, this, [this]() {
            show_closed_loop_ts = !show_closed_loop_ts;
            QString fwd_s = (domain_var == "s") ? "G(s), H(s)" : "H(z), H_fb(z)";
            QString cl_s  = (domain_var == "s") ? "Closed-Loop T(s)" : "Closed-Loop T(z)";
            view_mode_btn->setText(QString("Formula: %1").arg(show_closed_loop_ts ? cl_s : fwd_s));
            view_mode_btn->setStyleSheet(QString(
                "QPushButton { background-color: %1; color: %2; border: 1px solid %2; border-radius: 4px; padding: 2px 8px; font-size: 10px; font-weight: bold; }"
            ).arg(Theme::BG_INPUT, show_closed_loop_ts ? Theme::ACCENT_PID : Theme::ACCENT_ZERO));
            if (!updating && onChanged) onChanged();
        });

        auto onPolyEdited = [this]() {
            if (updating) return;
            updating = true;
            try {
                auto pn = parsePolynomialInput(h_num_edit->text().toStdString());
                auto pd = parsePolynomialInput(h_den_edit->text().toStdString());
                if (h_zeros_card) h_zeros_card->setValues(pn.roots);
                if (h_poles_card) h_poles_card->setValues(pd.roots);
            } catch (...) {}
            updating = false;
            if (onChanged) onChanged();
        };

        connect(h_num_edit, &QLineEdit::returnPressed, this, onPolyEdited);
        connect(h_den_edit, &QLineEdit::returnPressed, this, onPolyEdited);
        connect(solve_h_btn, &QPushButton::clicked, this, onPolyEdited);

        h_poles_card->onValuesChanged = [this]() {
            if (updating) return;
            updating = true;
            p_cards_authoritative = true;
            h_den_edit->setText(QString::fromStdString(
                formatPolynomialFromRoots(h_poles_card->getValues(), domain_var.toStdString())));
            updating = false;
            if (onChanged) onChanged();
            p_cards_authoritative = false;
        };

        h_zeros_card->onValuesChanged = [this]() {
            if (updating) return;
            updating = true;
            p_cards_authoritative = true;
            h_num_edit->setText(QString::fromStdString(
                formatPolynomialFromRoots(h_zeros_card->getValues(), domain_var.toStdString())));
            updating = false;
            if (onChanged) onChanged();
            p_cards_authoritative = false;
        };
    }

    // Helper: returns either "G(s) = ..., H(s) = ..." OR "T(s) = Num_T(s) / Den_T(s)" based on toggle!
    std::string formatActiveLatexS(const SharedSystemModel* m, bool include_pid = false) const {
        if (!show_closed_loop_ts) {
            std::string gs_ltx = formatTransferFunctionLatex(m->poles, m->zeros, m->plant_gain, "s", "G");
            if (m->comp_enabled_s) {
                gs_ltx = formatTransferFunctionLatex(m->c_poles, m->c_zeros, m->c_gain, "s", "G_c") + " , " + gs_ltx;
            }
            if (!m->unity_feedback_s) {
                gs_ltx += " , " + formatTransferFunctionLatex(m->h_poles, m->h_zeros, m->h_gain, "s", "H");
            } else {
                gs_ltx += " , H(s) = 1";
            }
            return gs_ltx;
        }
        auto [num_g, den_g] = ComplexTransferFunctionSimulator::buildPlantPolynomials(m->poles, m->zeros, m->plant_gain);
        if (m->comp_enabled_s && (!include_pid || m->isFinitePoleCompensatorS())) {
            auto [num_gc, den_gc] = ComplexTransferFunctionSimulator::buildPlantPolynomials(m->c_poles, m->c_zeros, m->c_gain);
            num_g = polyMul(num_gc, num_g);
            den_g = polyMul(den_gc, den_g);
        }
        std::vector<Complex> num_h = {Complex(1.0, 0.0)}, den_h = {Complex(1.0, 0.0)};
        if (!m->unity_feedback_s) {
            std::tie(num_h, den_h) = ComplexTransferFunctionSimulator::buildPlantPolynomials(m->h_poles, m->h_zeros, m->h_gain);
        }
        std::vector<Complex> num_c = {Complex(1.0, 0.0)}, den_c = {Complex(1.0, 0.0)};
        if (include_pid && m->pid_enabled && !m->isFinitePoleCompensatorS()) {
            if (std::abs(m->ki) > 1e-7) {
                num_c = {Complex(m->kd, 0.0), Complex(m->kp, 0.0), Complex(m->ki, 0.0)};
                den_c = {Complex(1.0, 0.0), Complex(0.0, 0.0)};
            } else {
                num_c = {Complex(m->kd, 0.0), Complex(m->kp, 0.0)};
            }
        }
        auto num_cg = polyMul(num_c, num_g);
        auto den_cg = polyMul(den_c, den_g);
        auto num_t  = polyMul(num_cg, den_h);
        auto den_t  = polyAdd(polyMul(den_cg, den_h), polyMul(num_cg, num_h));
        return "T(s) = \\frac{" + formatComplexPolyDesc(num_t, "s") + "}{" + formatComplexPolyDesc(den_t, "s") + "}";
    }

    std::string formatActiveLatexZ(const SharedSystemModel* m) const {
        if (!show_closed_loop_ts) {
            std::string hz_ltx = formatTransferFunctionLatex(m->z_poles, m->z_zeros, m->filter_gain, "z", "H");
            if (!m->unity_feedback_z) {
                hz_ltx += " , " + formatTransferFunctionLatex(m->hz_fb_poles, m->hz_fb_zeros, m->hz_fb_gain, "z", "H_{\\mathrm{fb}}");
            } else {
                hz_ltx += " , H_{\\mathrm{fb}}(z) = 1";
            }
            return hz_ltx;
        }
        auto [num_g, den_g] = ComplexTransferFunctionSimulator::buildPlantPolynomials(m->z_poles, m->z_zeros, m->filter_gain);
        std::vector<Complex> num_h = {Complex(1.0, 0.0)}, den_h = {Complex(1.0, 0.0)};
        if (!m->unity_feedback_z) {
            std::tie(num_h, den_h) = ComplexTransferFunctionSimulator::buildPlantPolynomials(m->hz_fb_poles, m->hz_fb_zeros, m->hz_fb_gain);
        }
        auto num_t = polyMul(num_g, den_h);
        auto den_t = polyAdd(polyMul(den_g, den_h), polyMul(num_g, num_h));
        return "T(z) = \\frac{" + formatComplexPolyDesc(num_t, "z") + "}{" + formatComplexPolyDesc(den_t, "z") + "}";
    }

    void syncToModelS(SharedSystemModel* m) {
        m->unity_feedback_s = unity_rb->isChecked();
        m->h_num_str = h_num_edit->text().toStdString();
        m->h_den_str = h_den_edit->text().toStdString();
        if (!m->unity_feedback_s) {
            if (p_cards_authoritative && h_poles_card && h_zeros_card) {
                m->h_poles = h_poles_card->getValues();
                m->h_zeros = h_zeros_card->getValues();
            } else {
                try {
                    auto pn = parsePolynomialInput(m->h_num_str);
                    auto pd = parsePolynomialInput(m->h_den_str);
                    m->h_zeros = pn.roots;
                    m->h_poles = pd.roots;
                    m->h_gain  = pn.leading_gain / std::max(std::abs(pd.leading_gain), 1e-12);
                    updating = true;
                    if (h_poles_card) h_poles_card->setValues(m->h_poles);
                    if (h_zeros_card) h_zeros_card->setValues(m->h_zeros);
                    updating = false;
                } catch (...) {
                    if (h_poles_card) m->h_poles = h_poles_card->getValues();
                    if (h_zeros_card) m->h_zeros = h_zeros_card->getValues();
                }
            }
        }
    }

    void loadFromModelS(const SharedSystemModel* m) {
        updating = true;
        unity_rb->setChecked(m->unity_feedback_s);
        custom_rb->setChecked(!m->unity_feedback_s);
        custom_row->setVisible(!m->unity_feedback_s);
        h_num_edit->setText(QString::fromStdString(m->h_num_str));
        h_den_edit->setText(QString::fromStdString(m->h_den_str));
        if (h_poles_card) h_poles_card->setValues(m->h_poles);
        if (h_zeros_card) h_zeros_card->setValues(m->h_zeros);
        updating = false;
    }

    void syncToModelZ(SharedSystemModel* m) {
        m->unity_feedback_z = unity_rb->isChecked();
        m->hz_fb_num_str = h_num_edit->text().toStdString();
        m->hz_fb_den_str = h_den_edit->text().toStdString();
        if (!m->unity_feedback_z) {
            if (p_cards_authoritative && h_poles_card && h_zeros_card) {
                m->hz_fb_poles = h_poles_card->getValues();
                m->hz_fb_zeros = h_zeros_card->getValues();
            } else {
                try {
                    auto pn = parsePolynomialInput(m->hz_fb_num_str);
                    auto pd = parsePolynomialInput(m->hz_fb_den_str);
                    m->hz_fb_zeros = pn.roots;
                    m->hz_fb_poles = pd.roots;
                    m->hz_fb_gain  = pn.leading_gain / std::max(std::abs(pd.leading_gain), 1e-12);
                    updating = true;
                    if (h_poles_card) h_poles_card->setValues(m->hz_fb_poles);
                    if (h_zeros_card) h_zeros_card->setValues(m->hz_fb_zeros);
                    updating = false;
                } catch (...) {
                    if (h_poles_card) m->hz_fb_poles = h_poles_card->getValues();
                    if (h_zeros_card) m->hz_fb_zeros = h_zeros_card->getValues();
                }
            }
        }
    }

    void loadFromModelZ(const SharedSystemModel* m) {
        updating = true;
        unity_rb->setChecked(m->unity_feedback_z);
        custom_rb->setChecked(!m->unity_feedback_z);
        custom_row->setVisible(!m->unity_feedback_z);
        h_num_edit->setText(QString::fromStdString(m->hz_fb_num_str));
        h_den_edit->setText(QString::fromStdString(m->hz_fb_den_str));
        if (h_poles_card) h_poles_card->setValues(m->hz_fb_poles);
        if (h_zeros_card) h_zeros_card->setValues(m->hz_fb_zeros);
        updating = false;
    }
};

// =============================================================================
// Dedicated Series Controller / Compensator Block: Gc(s) = Kc * Num_c(s) / Den_c(s)
// =============================================================================
class CompensatorConfigCard : public QFrame {
    Q_OBJECT
public:
    QCheckBox* enable_chk;
    QLineEdit *c_num_edit, *c_den_edit, *kc_edit;
    QWidget *inputs_w, *pz_w;
    PoleZeroEditorCard *c_poles_card, *c_zeros_card;
    bool updating = false;
    bool p_cards_authoritative = false;
    std::function<void()> onChanged;

    explicit CompensatorConfigCard(QWidget* parent = nullptr) : QFrame(parent) {
        setStyleSheet(QString("QFrame { background-color: %1; border: 1px solid %2; border-radius: 6px; }")
                      .arg(Theme::BG_PANEL, Theme::BORDER_PANEL));
        auto* vl = new QVBoxLayout(this);
        vl->setContentsMargins(10, 6, 10, 6);
        vl->setSpacing(4);

        // Header Row: Title + Enable Checkbox
        auto* hdr = new QHBoxLayout();
        auto* title = new QLabel("Controller / Compensator  Gc(s) = Kc · Num_c(s) / Den_c(s)");
        title->setStyleSheet("color: #34d399; font-weight: bold; font-size: 11px; border: none;");

        enable_chk = new QCheckBox("Enable Gc(s)");
        enable_chk->setChecked(false);
        enable_chk->setStyleSheet("QCheckBox { color: #34d399; font-weight: bold; font-size: 11px; border: none; }");

        hdr->addWidget(title);
        hdr->addStretch();
        hdr->addWidget(enable_chk);
        vl->addLayout(hdr);

        // Tier 1: Num_c(s), Den_c(s), Kc row (visible, enabled only when checked)
        inputs_w = new QWidget();
        inputs_w->setStyleSheet("background: transparent; border: none;");
        auto* p_row = new QHBoxLayout(inputs_w);
        p_row->setContentsMargins(0, 2, 0, 0);
        p_row->setSpacing(6);

        auto* nc_l = new QLabel("Num_c(s):");
        nc_l->setStyleSheet("color: #34d399; font-family: Monospace; font-size: 10px; font-weight: bold; border:none;");
        c_num_edit = makeDarkEntry("1");

        auto* dc_l = new QLabel("Den_c(s):");
        dc_l->setStyleSheet("color: #a78bfa; font-family: Monospace; font-size: 10px; font-weight: bold; border:none;");
        c_den_edit = makeDarkEntry("1");

        auto* kc_l = new QLabel("Kc:");
        kc_l->setStyleSheet("color: #facc15; font-family: Monospace; font-size: 10px; font-weight: bold; border:none;");
        kc_edit = makeDarkEntry("1.0", 48);

        auto* solve_c_btn = makeActionBtn("Solve Gc ➔", Theme::ACCENT_PID);

        p_row->addWidget(nc_l); p_row->addWidget(c_num_edit, 1);
        p_row->addWidget(dc_l); p_row->addWidget(c_den_edit, 1);
        p_row->addWidget(kc_l); p_row->addWidget(kc_edit);
        p_row->addWidget(solve_c_btn);
        vl->addWidget(inputs_w);

        // Tier 2: Editable Gc(s) Poles (×) & Zeros (○) Tables (hidden when disabled)
        pz_w = new QWidget();
        pz_w->setStyleSheet("background: transparent; border: none;");
        auto* cpz_row = new QHBoxLayout(pz_w);
        cpz_row->setContentsMargins(0, 2, 0, 0);
        cpz_row->setSpacing(6);
        c_poles_card = new PoleZeroEditorCard("Gc(s) Poles (×)", "#a78bfa", -4.0);
        c_zeros_card = new PoleZeroEditorCard("Gc(s) Zeros (○)", "#34d399", -2.0);
        c_poles_card->peer_card = c_zeros_card;
        c_zeros_card->peer_card = c_poles_card;
        c_poles_card->syncHeightWithPeer();
        cpz_row->addWidget(c_poles_card, 1);
        cpz_row->addWidget(c_zeros_card, 1);
        vl->addWidget(pz_w);

        // Initial disabled state: gray out inputs_w and hide pz_w
        inputs_w->setEnabled(false);
        pz_w->setVisible(false);

        connect(enable_chk, &QCheckBox::toggled, this, [this](bool en) {
            inputs_w->setEnabled(en);
            pz_w->setVisible(en);
            pz_w->setEnabled(en);
            if (!updating && onChanged) onChanged();
        });

        auto onPolyEdited = [this]() {
            if (updating) return;
            updating = true;
            try {
                auto pn = parsePolynomialInput(c_num_edit->text().toStdString());
                auto pd = parsePolynomialInput(c_den_edit->text().toStdString());
                c_zeros_card->setValues(pn.roots);
                c_poles_card->setValues(pd.roots);
            } catch (...) {}
            updating = false;
            if (onChanged) onChanged();
        };

        connect(c_num_edit, &QLineEdit::returnPressed, this, onPolyEdited);
        connect(c_den_edit, &QLineEdit::returnPressed, this, onPolyEdited);
        connect(kc_edit,    &QLineEdit::editingFinished, this, [this]() {
            if (!updating && onChanged) onChanged();
        });
        connect(solve_c_btn, &QPushButton::clicked, this, onPolyEdited);

        c_poles_card->onValuesChanged = [this]() {
            if (updating) return;
            updating = true;
            p_cards_authoritative = true;
            c_den_edit->setText(QString::fromStdString(formatPolynomialFromRoots(c_poles_card->getValues(), "s")));
            updating = false;
            if (onChanged) onChanged();
            p_cards_authoritative = false;
        };

        c_zeros_card->onValuesChanged = [this]() {
            if (updating) return;
            updating = true;
            p_cards_authoritative = true;
            c_num_edit->setText(QString::fromStdString(formatPolynomialFromRoots(c_zeros_card->getValues(), "s")));
            updating = false;
            if (onChanged) onChanged();
            p_cards_authoritative = false;
        };
    }

    void syncToModelS(SharedSystemModel* m) {
        m->comp_enabled_s = enable_chk->isChecked();
        m->c_num_str = c_num_edit->text().toStdString();
        m->c_den_str = c_den_edit->text().toStdString();
        m->c_gain    = std::max(0.0, parseNumericInput(kc_edit->text().toStdString(), 1.0));
        if (p_cards_authoritative) {
            m->c_poles = c_poles_card->getValues();
            m->c_zeros = c_zeros_card->getValues();
        } else {
            try {
                auto pn = parsePolynomialInput(m->c_num_str);
                auto pd = parsePolynomialInput(m->c_den_str);
                m->c_zeros = pn.roots;
                m->c_poles = pd.roots;
                updating = true;
                c_poles_card->setValues(m->c_poles);
                c_zeros_card->setValues(m->c_zeros);
                updating = false;
            } catch (...) {
                m->c_poles = c_poles_card->getValues();
                m->c_zeros = c_zeros_card->getValues();
            }
        }
    }

    void loadFromModelS(const SharedSystemModel* m) {
        updating = true;
        enable_chk->setChecked(m->comp_enabled_s);
        inputs_w->setEnabled(m->comp_enabled_s);
        pz_w->setVisible(m->comp_enabled_s);
        pz_w->setEnabled(m->comp_enabled_s);
        c_poles_card->setValues(m->c_poles);
        c_zeros_card->setValues(m->c_zeros);
        c_num_edit->setText(QString::fromStdString(formatPolynomialFromRoots(m->c_zeros, "s")));
        c_den_edit->setText(QString::fromStdString(formatPolynomialFromRoots(m->c_poles, "s")));
        kc_edit->setText(QString::number(m->c_gain, 'g', 4));
        updating = false;
    }
};

// =============================================================================
// Spec-Driven Keep (🔒) vs. Change (🔄) Parameter Matrix Engine & Dialog
// =============================================================================
struct ConstraintMatrixAnalysis {
    bool feasible = true;
    std::string controller_mode = "PD";
    std::string headline;
    std::string explanation;
    std::string formulas_latex;
    std::map<std::string, double> solved;
    std::map<std::string, std::string> notes;
    std::map<std::string, std::string> step_by_step_latex;
};

class ParameterMatrixEngine {
public:
    struct ParamMeta {
        std::string key;
        QString name;
        QString unit;
        QString desc;
    };

    static const std::vector<ParamMeta>& metaList() {
        static const std::vector<ParamMeta> m = {
            {"kp",          "Proportional Gain Kp",   "",      "Controller gain on current error e(t). Controls natural frequency ωn and step S.S.E."},
            {"kd",          "Derivative Gain Kd",     "",      "Controller gain on de(t)/dt. Adds damping σd = (a1 + K0·Kd)/2 without changing step S.S.E."},
            {"ki",          "Integral Gain Ki",       "",      "Controller gain on ∫e(t)dt. Adds pole at s=0 (Type +1), driving step S.S.E. to 0."},
            {"os_pct",      "Percent Overshoot %OS",  "%",     "%OS = 100·exp(-ζπ/√(1-ζ²)). Depends strictly on ζ."},
            {"zeta",        "Damping Ratio ζ",        "",      "Dimensionless damping ratio (0<ζ<1 underdamped, ζ=1 critically damped, ζ>1 overdamped)."},
            {"wn",          "Natural Frequency ωn",   "rad/s", "Undamped natural frequency |s_p| = √(σd² + ωd²)."},
            {"ts",          "Settling Time Ts (±2%)", "s",     "2% settling time: Ts = 4/(ζωn) = 4/σd."},
            {"tp",          "Peak Time Tp",           "s",     "Time to first peak: Tp = π/ωd = π/(ωn√(1-ζ²))."},
            {"tr",          "Rise Time Tr (10–90%)",  "s",     "10–90% rise time via Nise cubic polynomial in ζ divided by ωn."},
            {"sigma_d",     "Decay Rate σd = ζωn",    "s⁻¹",   "Real envelope attenuation rate |Re(s_p)| = ζωn = 4/Ts."},
            {"omega_d",     "Damped Freq ωd",         "rad/s", "Sinusoidal oscillation frequency |Im(s_p)| = ωn√(1-ζ²) = π/Tp."},
            {"ess",         "Absolute S.S.E. (e_ss)", "",      "Steady-state error |r_ss - y_ss| = M / (1 + Kp,pos)."},
            {"rel_err_pct", "Relative Error (%e_ss)", "%",     "Normalized steady-state error (e_ss / M) × 100%."}
        };
        return m;
    }

    static ConstraintMatrixAnalysis evaluate(
        const std::map<std::string, bool>& locked,
        const std::map<std::string, double>& in,
        const SharedSystemModel* model)
    {
        ConstraintMatrixAnalysis out;
        double m_amp = std::max(std::abs(model->input_magnitude), 1e-9);
        auto [num_g, den_g] = ComplexTransferFunctionSimulator::buildPlantPolynomials(model->poles, model->zeros, model->plant_gain);

        size_t n_poles = model->poles.size();
        double a1_ol = 1.0, a0_ol = 1.0, k0_ol = std::max(std::abs(num_g.back().real()), 1e-9);
        if (n_poles == 1) {
            a1_ol = 1.0;
            a0_ol = den_g[1].real();
        } else if (n_poles >= 2) {
            a1_ol = den_g[den_g.size() - 2].real();
            a0_ol = den_g.back().real();
        }

        int plant_integrators = 0;
        for (const auto& p : model->poles) if (std::abs(p) < 1e-7) ++plant_integrators;

        std::vector<std::string> conflicts;
        std::vector<std::string> formulas;

        auto isLk = [&](const std::string& k) { auto it = locked.find(k); return it != locked.end() && it->second; };
        auto getIn = [&](const std::string& k, double def) { auto it = in.find(k); return it != in.end() ? it->second : def; };

        // 1. Resolve zeta <-> os_pct
        std::optional<double> target_zeta;
        if (isLk("zeta") && isLk("os_pct")) {
            double z_in = std::max(0.0, getIn("zeta", 0.456));
            double os_in = std::max(0.0, getIn("os_pct", 20.0));
            double z_os = PRECISION.step(niseZetaFromOs(os_in));
            if (std::abs(z_in - z_os) > 0.015) {
                conflicts.push_back("• Conflict between Kept ζ (" + PRECISION.fmt(z_in) + ") and Kept %OS (" + PRECISION.fmt(os_in) + "%): Keeping %OS = " + PRECISION.fmt(os_in) + "% requires ζ = " + PRECISION.fmt(z_os) + ".");
            }
            target_zeta = z_os;
        } else if (isLk("os_pct")) {
            double os_in = std::max(0.0, getIn("os_pct", 20.0));
            target_zeta = PRECISION.step(niseZetaFromOs(os_in));
            out.notes["os_pct"] = "🔒 KEPT (Target Input)";
            out.notes["zeta"] = "🔄 Solved from %OS = " + PRECISION.fmt(os_in) + "%";
            std::string f = "\\zeta = \\frac{-\\ln(\\%OS/100)}{\\sqrt{\\pi^2 + \\ln^2(\\%OS/100)}} = \\frac{-\\ln(" + PRECISION.fmt(os_in / 100.0) + ")}{\\sqrt{\\pi^2 + \\ln^2(" + PRECISION.fmt(os_in / 100.0) + ")}} = " + PRECISION.fmt(*target_zeta);
            formulas.push_back(f);
            out.step_by_step_latex["zeta"] = f;
        } else if (isLk("zeta")) {
            target_zeta = PRECISION.step(std::max(0.0, getIn("zeta", 0.456)));
            out.notes["zeta"] = "🔒 KEPT (Target Input)";
            out.notes["os_pct"] = "🔄 Solved from ζ = " + PRECISION.fmt(*target_zeta);
        }

        // 2. Resolve sigma_d <-> ts
        std::optional<double> target_sigma_d;
        if (isLk("ts")) {
            double ts_in = std::max(1e-6, getIn("ts", 1.6));
            target_sigma_d = PRECISION.step(4.0 / ts_in);
            out.notes["ts"] = "🔒 KEPT (Target Input)";
            out.notes["sigma_d"] = "🔄 Solved from Ts (4/Ts)";
            std::string f = "\\sigma_d = \\frac{4}{T_s} = \\frac{4}{" + PRECISION.fmt(ts_in) + "} = " + PRECISION.fmt(*target_sigma_d) + "\\;\\mathrm{s}^{-1}";
            formulas.push_back(f);
            out.step_by_step_latex["sigma_d"] = f;
        } else if (isLk("sigma_d")) {
            target_sigma_d = PRECISION.step(std::max(1e-6, getIn("sigma_d", 2.5)));
            out.notes["sigma_d"] = "🔒 KEPT (Target Input)";
            out.notes["ts"] = "🔄 Solved from σd (4/σd)";
        }

        // 3. Resolve omega_d <-> tp
        std::optional<double> target_omega_d;
        if (isLk("tp")) {
            double tp_in = std::max(1e-6, getIn("tp", 0.64));
            target_omega_d = PRECISION.step(PI / tp_in);
            out.notes["tp"] = "🔒 KEPT (Target Input)";
            out.notes["omega_d"] = "🔄 Solved from Tp (π/Tp)";
            std::string f = "\\omega_d = \\frac{\\pi}{T_p} = \\frac{\\pi}{" + PRECISION.fmt(tp_in) + "} = " + PRECISION.fmt(*target_omega_d) + "\\;\\mathrm{rad/s}";
            formulas.push_back(f);
            out.step_by_step_latex["omega_d"] = f;
        } else if (isLk("omega_d")) {
            target_omega_d = PRECISION.step(std::max(1e-6, getIn("omega_d", 4.88)));
            out.notes["omega_d"] = "🔒 KEPT (Target Input)";
            out.notes["tp"] = "🔄 Solved from ωd (π/ωd)";
        }

        // 4. Resolve ess <-> rel_err_pct
        std::optional<double> target_ess;
        if (isLk("ess")) {
            target_ess = PRECISION.step(std::max(0.0, getIn("ess", 0.2)));
            out.notes["ess"] = "🔒 KEPT (Target Input)";
        } else if (isLk("rel_err_pct")) {
            double rel_in = std::max(0.0, getIn("rel_err_pct", 20.0));
            target_ess = PRECISION.step(m_amp * (rel_in / 100.0));
            out.notes["rel_err_pct"] = "🔒 KEPT (Target Input)";
        }

        std::optional<double> target_wn;
        if (isLk("wn")) {
            target_wn = PRECISION.step(std::max(1e-4, getIn("wn", 5.48)));
            out.notes["wn"] = "🔒 KEPT (Target Input)";
        }
        std::optional<double> target_tr;
        if (isLk("tr")) {
            target_tr = PRECISION.step(std::max(1e-4, getIn("tr", 0.34)));
            out.notes["tr"] = "🔒 KEPT (Target Input)";
        }

        auto resolveZetaWn = [&]() -> std::pair<double, double> {
            if (target_zeta && target_wn) return {*target_zeta, *target_wn};
            if (target_zeta && target_sigma_d) {
                double w = PRECISION.step(*target_sigma_d / std::max(*target_zeta, 1e-5));
                out.step_by_step_latex["wn"] = "\\omega_n = \\frac{\\sigma_d}{\\zeta} = \\frac{" + PRECISION.fmt(*target_sigma_d) + "}{" + PRECISION.fmt(*target_zeta) + "} = " + PRECISION.fmt(w) + "\\;\\mathrm{rad/s}";
                return {*target_zeta, w};
            }
            if (target_zeta && target_omega_d) {
                double sq = std::sqrt(std::max(1e-6, 1.0 - (*target_zeta) * (*target_zeta)));
                double w = PRECISION.step(*target_omega_d / sq);
                out.step_by_step_latex["wn"] = "\\omega_n = \\frac{\\omega_d}{\\sqrt{1-\\zeta^2}} = \\frac{" + PRECISION.fmt(*target_omega_d) + "}{\\sqrt{1-" + PRECISION.fmt(*target_zeta) + "^2}} = " + PRECISION.fmt(w) + "\\;\\mathrm{rad/s}";
                return {*target_zeta, w};
            }
            if (target_zeta && target_tr) {
                double wn_tr = PRECISION.step(niseNormalizedTrFromZeta(*target_zeta));
                double w = PRECISION.step(wn_tr / std::max(*target_tr, 1e-6));
                out.step_by_step_latex["wn"] = "\\omega_n = \\frac{f(\\zeta)}{T_r} = \\frac{" + PRECISION.fmt(wn_tr) + "}{" + PRECISION.fmt(*target_tr) + "} = " + PRECISION.fmt(w) + "\\;\\mathrm{rad/s}";
                return {*target_zeta, w};
            }
            if (target_sigma_d && target_omega_d) {
                double w = PRECISION.step(std::hypot(*target_sigma_d, *target_omega_d));
                double z = PRECISION.step(*target_sigma_d / std::max(w, 1e-6));
                out.step_by_step_latex["wn"] = "\\omega_n = \\sqrt{\\sigma_d^2 + \\omega_d^2} = \\sqrt{" + PRECISION.fmt(*target_sigma_d) + "^2 + " + PRECISION.fmt(*target_omega_d) + "^2} = " + PRECISION.fmt(w) + "\\;\\mathrm{rad/s}";
                out.step_by_step_latex["zeta"] = "\\zeta = \\frac{\\sigma_d}{\\omega_n} = \\frac{" + PRECISION.fmt(*target_sigma_d) + "}{" + PRECISION.fmt(w) + "} = " + PRECISION.fmt(z);
                return {z, w};
            }
            return {target_zeta.value_or(0.456), target_wn.value_or(5.48)};
        };

        bool kp_lk = isLk("kp"), kd_lk = isLk("kd"), ki_lk = isLk("ki");
        double kp_in = getIn("kp", model->kp);
        double kd_in = getIn("kd", model->kd);
        double ki_in = getIn("ki", model->ki);

        if (kp_lk) out.notes["kp"] = "🔒 KEPT (Kp = " + PRECISION.fmt(kp_in) + ")";
        if (kd_lk) out.notes["kd"] = "🔒 KEPT (Kd = " + PRECISION.fmt(kd_in) + ")";
        if (ki_lk) out.notes["ki"] = "🔒 KEPT (Ki = " + PRECISION.fmt(ki_in) + ")";

        std::vector<std::string> free_g;
        if (!kp_lk) free_g.push_back("kp");
        if (!kd_lk) free_g.push_back("kd");
        if (!ki_lk) free_g.push_back("ki");

        bool has_i = (!ki_lk) || (std::abs(ki_in) > 1e-7);
        bool has_d = (!kd_lk) || (std::abs(kd_in) > 1e-7);
        out.controller_mode = (has_i && has_d) ? "PID" : (has_i ? "PI" : (has_d ? "PD" : "P"));

        double kp_out = kp_in, kd_out = kd_in, ki_out = ki_in;
        double zeta_out = target_zeta.value_or(0.456);
        double wn_out = target_wn.value_or(5.48);

        if (free_g == std::vector<std::string>{"kd"}) {
            wn_out = PRECISION.step(std::sqrt(std::max(1e-9, a0_ol + k0_ol * kp_out)));
            out.step_by_step_latex["wn"] = "\\omega_n = \\sqrt{a_0 + K_0 K_p} = \\sqrt{" + PRECISION.fmt(a0_ol) + " + " + PRECISION.fmt(k0_ol) + "(" + PRECISION.fmt(kp_out) + ")} = " + PRECISION.fmt(wn_out) + "\\;\\mathrm{rad/s}";
            if (target_zeta) {
                zeta_out = *target_zeta;
                kd_out = PRECISION.step((2.0 * zeta_out * wn_out - a1_ol) / k0_ol);
                out.step_by_step_latex["kd"] = "K_d = \\frac{2\\zeta\\omega_n - a_1}{K_0} = \\frac{2(" + PRECISION.fmt(zeta_out) + ")(" + PRECISION.fmt(wn_out) + ") - " + PRECISION.fmt(a1_ol) + "}{" + PRECISION.fmt(k0_ol) + "} = " + PRECISION.fmt(kd_out);
            } else if (target_sigma_d) {
                kd_out = PRECISION.step((2.0 * (*target_sigma_d) - a1_ol) / k0_ol);
                out.step_by_step_latex["kd"] = "K_d = \\frac{2\\sigma_d - a_1}{K_0} = \\frac{2(" + PRECISION.fmt(*target_sigma_d) + ") - " + PRECISION.fmt(a1_ol) + "}{" + PRECISION.fmt(k0_ol) + "} = " + PRECISION.fmt(kd_out);
            } else if (target_omega_d) {
                double sig_req = std::sqrt(std::max(0.0, wn_out * wn_out - (*target_omega_d) * (*target_omega_d)));
                kd_out = PRECISION.step((2.0 * sig_req - a1_ol) / k0_ol);
                out.step_by_step_latex["kd"] = "\\sigma_d = \\sqrt{\\omega_n^2 - \\omega_d^2} = " + PRECISION.fmt(sig_req) + " \\;\\Rightarrow\\; K_d = \\frac{2\\sigma_d - a_1}{K_0} = " + PRECISION.fmt(kd_out);
            }
            zeta_out = PRECISION.step((a1_ol + k0_ol * kd_out) / (2.0 * wn_out));
            out.notes["kd"] = "🔄 Solved with Kept Kp=" + PRECISION.fmt(kp_out);
            formulas.push_back(out.step_by_step_latex["kd"]);
        } else if (free_g == std::vector<std::string>{"kp"}) {
            double eff_a1 = PRECISION.step(a1_ol + k0_ol * kd_out);
            double sig_fixed = PRECISION.step(eff_a1 / 2.0);
            if (target_sigma_d && std::abs(*target_sigma_d - sig_fixed) > 0.05 * std::max(1.0, sig_fixed)) {
                conflicts.push_back("• Since Kd is KEPT at " + PRECISION.fmt(kd_out) + ", σd = (a1 + K0·Kd)/2 is locked at " +
                                    PRECISION.fmt(sig_fixed) + " s⁻¹ (Ts = " + PRECISION.fmt(4.0 / sig_fixed) + "s). Switch Kd to 🔄 CHANGE to modify Ts!");
            }
            if (target_zeta && *target_zeta > 1e-5) {
                wn_out = PRECISION.step(eff_a1 / (2.0 * (*target_zeta)));
                kp_out = PRECISION.step((wn_out * wn_out - a0_ol) / k0_ol);
                out.step_by_step_latex["wn"] = "\\omega_n = \\frac{a_1 + K_0 K_d}{2\\zeta} = \\frac{" + PRECISION.fmt(eff_a1) + "}{2(" + PRECISION.fmt(*target_zeta) + ")} = " + PRECISION.fmt(wn_out) + "\\;\\mathrm{rad/s}";
                out.step_by_step_latex["kp"] = "K_p = \\frac{\\omega_n^2 - a_0}{K_0} = \\frac{" + PRECISION.fmt(wn_out) + "^2 - " + PRECISION.fmt(a0_ol) + "}{" + PRECISION.fmt(k0_ol) + "} = " + PRECISION.fmt(kp_out);
                formulas.push_back(out.step_by_step_latex["kp"]);
            } else if (target_ess && *target_ess > 1e-7 && plant_integrators == 0) {
                kp_out = PRECISION.step((a0_ol / k0_ol) * (m_amp / (*target_ess) - 1.0));
                out.step_by_step_latex["kp"] = "K_p = \\frac{a_0}{K_0}\\left(\\frac{M}{e_{\\mathrm{ss}}} - 1\\right) = \\frac{" + PRECISION.fmt(a0_ol) + "}{" + PRECISION.fmt(k0_ol) + "}\\left(\\frac{" + PRECISION.fmt(m_amp) + "}{" + PRECISION.fmt(*target_ess) + "} - 1\\right) = " + PRECISION.fmt(kp_out);
                formulas.push_back(out.step_by_step_latex["kp"]);
            } else if (target_omega_d) {
                wn_out = PRECISION.step(std::hypot(sig_fixed, *target_omega_d));
                kp_out = PRECISION.step((wn_out * wn_out - a0_ol) / k0_ol);
                out.step_by_step_latex["wn"] = "\\omega_n = \\sqrt{\\sigma_d^2 + \\omega_d^2} = \\sqrt{" + PRECISION.fmt(sig_fixed) + "^2 + " + PRECISION.fmt(*target_omega_d) + "^2} = " + PRECISION.fmt(wn_out);
                out.step_by_step_latex["kp"] = "K_p = \\frac{\\omega_n^2 - a_0}{K_0} = \\frac{" + PRECISION.fmt(wn_out) + "^2 - " + PRECISION.fmt(a0_ol) + "}{" + PRECISION.fmt(k0_ol) + "} = " + PRECISION.fmt(kp_out);
                formulas.push_back(out.step_by_step_latex["kp"]);
            } else if (target_wn) {
                wn_out = *target_wn;
                kp_out = PRECISION.step((wn_out * wn_out - a0_ol) / k0_ol);
                out.step_by_step_latex["kp"] = "K_p = \\frac{\\omega_n^2 - a_0}{K_0} = \\frac{" + PRECISION.fmt(wn_out) + "^2 - " + PRECISION.fmt(a0_ol) + "}{" + PRECISION.fmt(k0_ol) + "} = " + PRECISION.fmt(kp_out);
            }
            wn_out = PRECISION.step(std::sqrt(std::max(1e-9, a0_ol + k0_ol * kp_out)));
            zeta_out = PRECISION.step(eff_a1 / (2.0 * wn_out));
            out.notes["kp"] = "🔄 Solved with Kept Kd=" + PRECISION.fmt(kd_out);
        } else if (free_g == std::vector<std::string>{"ki"}) {
            double two_zw = PRECISION.step(a0_ol + k0_ol * kp_out);
            zeta_out = target_zeta.value_or(0.456);
            wn_out = PRECISION.step(two_zw / (2.0 * std::max(zeta_out, 1e-5)));
            ki_out = PRECISION.step((wn_out * wn_out) / k0_ol);
            out.notes["ki"] = "🔄 Solved with Kept Kp=" + PRECISION.fmt(kp_out);
            out.step_by_step_latex["wn"] = "\\omega_n = \\frac{a_0 + K_0 K_p}{2\\zeta} = \\frac{" + PRECISION.fmt(two_zw) + "}{2(" + PRECISION.fmt(zeta_out) + ")} = " + PRECISION.fmt(wn_out);
            out.step_by_step_latex["ki"] = "K_i = \\frac{\\omega_n^2}{K_0} = \\frac{" + PRECISION.fmt(wn_out) + "^2}{" + PRECISION.fmt(k0_ol) + "} = " + PRECISION.fmt(ki_out);
            formulas.push_back(out.step_by_step_latex["ki"]);
        } else if (!kp_lk && !kd_lk && ki_lk) {
            std::tie(zeta_out, wn_out) = resolveZetaWn();
            if (target_ess && *target_ess > 1e-7 && plant_integrators == 0) {
                kp_out = PRECISION.step((a0_ol / k0_ol) * (m_amp / (*target_ess) - 1.0));
                wn_out = PRECISION.step(std::sqrt(std::max(1e-9, a0_ol + k0_ol * kp_out)));
                out.step_by_step_latex["kp"] = "K_p = \\frac{a_0}{K_0}\\left(\\frac{M}{e_{\\mathrm{ss}}} - 1\\right) = " + PRECISION.fmt(kp_out);
            } else {
                kp_out = PRECISION.step((wn_out * wn_out - a0_ol) / k0_ol);
                out.step_by_step_latex["kp"] = "K_p = \\frac{\\omega_n^2 - a_0}{K_0} = \\frac{" + PRECISION.fmt(wn_out) + "^2 - " + PRECISION.fmt(a0_ol) + "}{" + PRECISION.fmt(k0_ol) + "} = " + PRECISION.fmt(kp_out);
            }
            kd_out = PRECISION.step((2.0 * zeta_out * wn_out - a1_ol) / k0_ol);
            out.step_by_step_latex["kd"] = "K_d = \\frac{2\\zeta\\omega_n - a_1}{K_0} = \\frac{2(" + PRECISION.fmt(zeta_out) + ")(" + PRECISION.fmt(wn_out) + ") - " + PRECISION.fmt(a1_ol) + "}{" + PRECISION.fmt(k0_ol) + "} = " + PRECISION.fmt(kd_out);
            out.notes["kp"] = "🔄 Solved from (ζ, ωn)";
            out.notes["kd"] = "🔄 Solved from (ζ, ωn)";
            formulas.push_back(out.step_by_step_latex["kp"] + ", \\quad " + out.step_by_step_latex["kd"]);
        } else {
            std::tie(zeta_out, wn_out) = resolveZetaWn();
            double sig_d = std::max(zeta_out * wn_out, 0.2 * wn_out);
            double p3 = 5.0 * sig_d;
            if (!kd_lk) {
                kd_out = PRECISION.step((2.0 * zeta_out * wn_out + p3 - a1_ol) / k0_ol);
                out.step_by_step_latex["kd"] = "K_d = \\frac{2\\zeta\\omega_n + p_3 - a_1}{K_0} = \\frac{2(" + PRECISION.fmt(zeta_out) + ")(" + PRECISION.fmt(wn_out) + ") + " + PRECISION.fmt(p3) + " - " + PRECISION.fmt(a1_ol) + "}{" + PRECISION.fmt(k0_ol) + "} = " + PRECISION.fmt(kd_out);
            }
            if (!kp_lk) {
                kp_out = PRECISION.step((wn_out * wn_out + 2.0 * zeta_out * wn_out * p3 - a0_ol) / k0_ol);
                out.step_by_step_latex["kp"] = "K_p = \\frac{\\omega_n^2 + 2\\zeta\\omega_n p_3 - a_0}{K_0} = " + PRECISION.fmt(kp_out);
            }
            if (!ki_lk) {
                ki_out = PRECISION.step((wn_out * wn_out * p3) / k0_ol);
                out.step_by_step_latex["ki"] = "K_i = \\frac{\\omega_n^2 p_3}{K_0} = \\frac{" + PRECISION.fmt(wn_out) + "^2 (" + PRECISION.fmt(p3) + ")}{" + PRECISION.fmt(k0_ol) + "} = " + PRECISION.fmt(ki_out);
            }
            formulas.push_back("p_3 = 5\\sigma_d = " + PRECISION.fmt(p3) + " \\Rightarrow K_p = " + PRECISION.fmt(kp_out) + ", \\; K_i = " + PRECISION.fmt(ki_out) + ", \\; K_d = " + PRECISION.fmt(kd_out));
        }

        double sig_res = PRECISION.step(zeta_out * wn_out);
        double wd_res  = (zeta_out < 1.0) ? PRECISION.step(wn_out * std::sqrt(std::max(0.0, 1.0 - zeta_out * zeta_out))) : 0.0;
        double os_res  = (zeta_out < 1.0) ? PRECISION.step(niseOsFromZeta(zeta_out)) : 0.0;
        double ts_res  = (sig_res > 1e-9) ? PRECISION.step(4.0 / sig_res) : std::numeric_limits<double>::infinity();
        double tp_res  = (wd_res > 1e-9)  ? PRECISION.step(PI / wd_res)   : std::numeric_limits<double>::infinity();
        double tr_res  = PRECISION.step(niseTrFromZetaWn(std::clamp(zeta_out, 0.0, 1.2), wn_out));

        double ess_res = 0.0, rel_res = 0.0;
        if (std::abs(ki_out) <= 1e-9 && plant_integrators == 0) {
            double kp_pos = kp_out * (k0_ol / std::max(a0_ol, 1e-12));
            ess_res = PRECISION.step(m_amp / std::max(1.0 + kp_pos, 1e-9));
            rel_res = PRECISION.step((ess_res / m_amp) * 100.0);
            if (!out.step_by_step_latex.count("ess")) {
                out.step_by_step_latex["ess"] = "e_{\\mathrm{ss}} = \\frac{M}{1 + K_{p,\\mathrm{pos}}} = \\frac{" + PRECISION.fmt(m_amp) + "}{1 + " + PRECISION.fmt(kp_pos) + "} = " + PRECISION.fmt(ess_res);
            }
            if (!out.step_by_step_latex.count("rel_err_pct")) {
                out.step_by_step_latex["rel_err_pct"] = "\\%e_{\\mathrm{ss}} = \\frac{e_{\\mathrm{ss}}}{M}\\times 100\\% = \\frac{" + PRECISION.fmt(ess_res) + "}{" + PRECISION.fmt(m_amp) + "}\\times 100\\% = " + PRECISION.fmt(rel_res) + "\\%";
            }
        } else {
            out.step_by_step_latex["ess"] = "e_{\\mathrm{ss}} = 0 \\quad (\\text{System Type } \\ge 1 \\text{ with Integral Action})";
            out.step_by_step_latex["rel_err_pct"] = "\\%e_{\\mathrm{ss}} = 0\\% \\quad (\\text{Zero Steady-State Error})";
        }

        if (!out.step_by_step_latex.count("os_pct")) {
            out.step_by_step_latex["os_pct"] = "\\%OS = 100 \\cdot e^{-\\zeta\\pi / \\sqrt{1-\\zeta^2}} = 100 \\cdot e^{-(" + PRECISION.fmt(zeta_out) + ")\\pi / \\sqrt{1-" + PRECISION.fmt(zeta_out) + "^2}} = " + PRECISION.fmt(os_res) + "\\%";
        }
        if (!out.step_by_step_latex.count("zeta")) {
            out.step_by_step_latex["zeta"] = "\\zeta = \\frac{\\sigma_d}{\\omega_n} = \\frac{" + PRECISION.fmt(sig_res) + "}{" + PRECISION.fmt(wn_out) + "} = " + PRECISION.fmt(zeta_out);
        }
        if (!out.step_by_step_latex.count("wn")) {
            out.step_by_step_latex["wn"] = "\\omega_n = \\sqrt{a_0 + K_0 K_p} = \\sqrt{" + PRECISION.fmt(a0_ol) + " + " + PRECISION.fmt(k0_ol) + "(" + PRECISION.fmt(kp_out) + ")} = " + PRECISION.fmt(wn_out) + "\\;\\mathrm{rad/s}";
        }
        if (!out.step_by_step_latex.count("sigma_d")) {
            out.step_by_step_latex["sigma_d"] = "\\sigma_d = \\zeta \\omega_n = (" + PRECISION.fmt(zeta_out) + ")(" + PRECISION.fmt(wn_out) + ") = " + PRECISION.fmt(sig_res) + "\\;\\mathrm{s}^{-1}";
        }
        if (!out.step_by_step_latex.count("omega_d")) {
            out.step_by_step_latex["omega_d"] = "\\omega_d = \\omega_n \\sqrt{1 - \\zeta^2} = " + PRECISION.fmt(wn_out) + "\\sqrt{1 - " + PRECISION.fmt(zeta_out) + "^2} = " + PRECISION.fmt(wd_res) + "\\;\\mathrm{rad/s}";
        }
        if (!out.step_by_step_latex.count("ts")) {
            out.step_by_step_latex["ts"] = "T_s = \\frac{4}{\\zeta \\omega_n} = \\frac{4}{" + PRECISION.fmt(sig_res) + "} = " + PRECISION.fmt(ts_res) + "\\;\\mathrm{s}";
        }
        if (!out.step_by_step_latex.count("tp")) {
            out.step_by_step_latex["tp"] = "T_p = \\frac{\\pi}{\\omega_d} = \\frac{\\pi}{" + PRECISION.fmt(wd_res) + "} = " + PRECISION.fmt(tp_res) + "\\;\\mathrm{s}";
        }
        if (!out.step_by_step_latex.count("tr")) {
            double norm_tr = PRECISION.step(niseNormalizedTrFromZeta(std::clamp(zeta_out, 0.0, 1.2)));
            out.step_by_step_latex["tr"] = "T_r = \\frac{2.16\\zeta^3 - 0.40\\zeta^2 + 1.04\\zeta + 1.12}{\\omega_n} = \\frac{" + PRECISION.fmt(norm_tr) + "}{" + PRECISION.fmt(wn_out) + "} = " + PRECISION.fmt(tr_res) + "\\;\\mathrm{s}";
        }

        out.solved = {
            {"kp", kp_out}, {"kd", kd_out}, {"ki", ki_out},
            {"os_pct", os_res}, {"zeta", zeta_out}, {"wn", wn_out},
            {"ts", ts_res}, {"tp", tp_res}, {"tr", tr_res},
            {"sigma_d", sig_res}, {"omega_d", wd_res},
            {"ess", ess_res}, {"rel_err_pct", rel_res}
        };
        for (const auto& m : metaList()) {
            if (!out.notes.count(m.key)) out.notes[m.key] = "🔄 Changed (Computed from Kept Specs)";
        }

        out.feasible = conflicts.empty();
        out.headline = out.feasible
            ? ("✔ POSSIBLE & SOLVED (" + out.controller_mode + "-Control):  Kp=" + PRECISION.fmt(kp_out) +
               ", Kd=" + PRECISION.fmt(kd_out) + ", Ki=" + PRECISION.fmt(ki_out) +
               "  •  %OS=" + PRECISION.fmt(os_res) + "%, Ts=" + PRECISION.fmt(ts_res) + "s, S.S.E.=" + PRECISION.fmt(ess_res))
            : ("✖ CONFLICT DETECTED (" + out.controller_mode + "-Control)");

        out.explanation.clear();
        for (const auto& c : conflicts) out.explanation += (out.explanation.empty() ? "" : "\n") + c;
        if (out.explanation.empty()) {
            out.explanation = "All 🔄 CHANGE parameters have been solved from your 🔒 KEEP specifications. Click '▶ Show Calc' on any changed row to inspect its derivation.";
        }

        out.formulas_latex.clear();
        for (size_t i = 0; i < formulas.size(); ++i) {
            out.formulas_latex += (i ? " \\quad | \\quad " : "") + formulas[i];
        }
        if (out.formulas_latex.empty()) {
            out.formulas_latex = "\\zeta = " + PRECISION.fmt(zeta_out) + ", \\; \\omega_n = " + PRECISION.fmt(wn_out) +
                                 ", \\; K_p = " + PRECISION.fmt(kp_out) + ", \\; K_d = " + PRECISION.fmt(kd_out);
        }
        return out;
    }
};

// =============================================================================
// Lead / Lag / PD / PI Root Locus Compensator Synthesis Result
// =============================================================================
struct RootLocusCompensatorResult {
    bool valid = false;
    std::string comp_type = "Lead"; // "Lead", "PD", "Lag", "PI"
    std::string method_name;
    Complex s_target{-2.0, 2.0};
    double angle_def_deg = 0.0;
    bool has_zero = true;
    bool has_pole = true;
    double z_c = 2.0; // Compensator zero at s = -z_c (so z_c > 0 is LHP)
    double p_c = 5.0; // Compensator pole at s = -p_c (so p_c > 0 is LHP)
    double k_c = 1.0; // Required loop gain at s_target
    std::string gc_latex;
    std::string steps_latex;
    std::string summary_text;
};

class DesignParametersDialog : public QDialog {
    Q_OBJECT
public:
    SharedSystemModel* model;
    std::map<std::string, bool>* lock_map;
    std::map<std::string, double>* input_map;
    std::function<void(const ConstraintMatrixAnalysis&)> onApplyDesign;
    std::function<void(double)> onMaxKChanged;
    std::function<void(const RootLocusCompensatorResult&)> onApplyCompensator;
    std::function<void()> onRestoreUncompensated;

    QStackedWidget* sub_stack;
    QPushButton *tab_btn_fb, *tab_btn_synth, *tab_btn_comp;
    QLabel* top_subtitle_lbl;

    // Page 1: Feedback H(s) & Resulting T(s) Menu Widgets
    QCheckBox *fb_unity_chk, *fb_custom_chk;
    QCheckBox *ctrl_p_chk, *ctrl_pi_chk, *ctrl_pd_chk, *ctrl_pid_chk, *ctrl_none_chk;
    std::string active_ctrl_type = "None";
    QLineEdit *fb_kp_edit, *fb_ki_edit, *fb_kd_edit;
    QLabel *fb_kp_lbl, *fb_ki_lbl, *fb_kd_lbl;

    QLineEdit *num_h_edit, *den_h_edit, *kh_edit;
    QLabel* h_status_lbl;
    MathFormulaWidget* h_math_w;
    PoleZeroEditorCard *h_poles_card, *h_zeros_card;

    QLineEdit *num_g_edit, *den_g_edit, *kg_edit;
    MathFormulaWidget *g_math_w, *c_math_w;

    MathFormulaWidget *t_sym_math_w, *t_eval_math_w, *l_open_math_w;
    QLineEdit *num_t_sym_edit, *den_t_sym_edit, *num_t_eval_edit, *den_t_eval_edit;
    QLabel* cl_telemetry_lbl;

    // Page 2: Keep (🔒) vs. Change (🔄) Synthesizer Widgets
    std::map<std::string, QPushButton*> lock_btns;
    std::map<std::string, QLineEdit*> val_edits;
    std::map<std::string, QLabel*> solved_lbls;
    std::map<std::string, QLabel*> note_lbls;
    std::map<std::string, QPushButton*> calc_toggle_btns;
    std::map<std::string, QFrame*> calc_drawer_frames;
    std::map<std::string, MathFormulaWidget*> calc_drawer_maths;
    std::map<std::string, bool> calc_expanded;
    QCheckBox* show_all_calcs_chk;
    QLineEdit* max_k_edit;

    QFrame* verdict_banner;
    QLabel *verdict_title, *verdict_desc;
    QFrame* bottom_formula_box;
    MathFormulaWidget* formula_math_w;
    ConstraintMatrixAnalysis last_analysis;

    // Page 3: Root Locus Lead / Lag / PD / PI Compensator Synthesizer Widgets
    QComboBox *comp_type_combo = nullptr, *comp_method_combo = nullptr;
    QLineEdit *comp_sd_re_edit = nullptr, *comp_sd_im_edit = nullptr;
    QLineEdit *comp_param_edit = nullptr;
    QLabel *comp_param_lbl = nullptr, *comp_def_lbl = nullptr, *comp_verdict_lbl = nullptr;
    MathFormulaWidget *comp_gc_math_w = nullptr, *comp_steps_math_w = nullptr;
    RootLocusCompensatorResult last_comp_res;

    bool updating = false;

    DesignParametersDialog(SharedSystemModel* m,
                           std::map<std::string, bool>* lk,
                           std::map<std::string, double>* in,
                           std::function<void(const ConstraintMatrixAnalysis&)> apply_cb,
                           std::function<void(double)> maxk_cb,
                           QWidget* parent = nullptr,
                           std::function<void(const RootLocusCompensatorResult&)> comp_cb = nullptr,
                           std::function<void()> restore_cb = nullptr)
        : QDialog(parent), model(m), lock_map(lk), input_map(in),
          onApplyDesign(std::move(apply_cb)), onMaxKChanged(std::move(maxk_cb)),
          onApplyCompensator(std::move(comp_cb)), onRestoreUncompensated(std::move(restore_cb))
    {
        setWindowTitle("Spec Synthesizer, Feedback H(s) & Root Locus Compensator Designer");
        resize(1140, 790);
        setStyleSheet(QString("QDialog { background-color: %1; }").arg(Theme::BG_WINDOW));

        auto* main_l = new QVBoxLayout(this);
        main_l->setContentsMargins(14, 10, 14, 12);
        main_l->setSpacing(8);

        // Top Header Bar with 3-Page Switcher
        auto* hdr_card = makeCard();
        auto* hdr_v = new QVBoxLayout(hdr_card);
        hdr_v->setContentsMargins(12, 8, 12, 8);
        hdr_v->setSpacing(4);

        auto* hdr_top = new QHBoxLayout();
        auto* main_title = new QLabel("Parameter Synthesizer & Root Locus Compensator Designer");
        main_title->setStyleSheet(QString("color: %1; font-size: 15px; font-weight: bold; border:none;").arg(Theme::TEXT_PRIMARY));

        tab_btn_fb    = new QPushButton("1. 🔁 Feedback H(s) & T(s)");
        tab_btn_synth = new QPushButton("2. 🎛 Keep (🔒) vs. Change (🔄)");
        tab_btn_comp  = new QPushButton("3. 📐 Lead / Lag Compensator (Ghost RL)");
        for (auto* b : {tab_btn_fb, tab_btn_synth, tab_btn_comp}) b->setCursor(Qt::PointingHandCursor);

        hdr_top->addWidget(main_title);
        hdr_top->addStretch();
        hdr_top->addWidget(tab_btn_fb);
        hdr_top->addWidget(tab_btn_synth);
        hdr_top->addWidget(tab_btn_comp);
        hdr_v->addLayout(hdr_top);

        top_subtitle_lbl = new QLabel();
        top_subtitle_lbl->setStyleSheet(QString("color: %1; font-size: 11px; border:none;").arg(Theme::ACCENT_ZERO));
        hdr_v->addWidget(top_subtitle_lbl);
        main_l->addWidget(hdr_card);

        sub_stack = new QStackedWidget(this);
        sub_stack->addWidget(buildFeedbackAndTransferFunctionPage()); // Page 0
        sub_stack->addWidget(buildKeepChangeSynthesizerPage());       // Page 1
        sub_stack->addWidget(buildCompensatorDesignPage());           // Page 2
        main_l->addWidget(sub_stack, 1);

        connect(tab_btn_fb,    &QPushButton::clicked, this, [this]() { switchSubPage(0); });
        connect(tab_btn_synth, &QPushButton::clicked, this, [this]() { switchSubPage(1); });
        connect(tab_btn_comp,  &QPushButton::clicked, this, [this]() { switchSubPage(2); });

        switchSubPage(2); // Open directly on Lead/Lag Compensator or Keep/Change
        refreshFeedbackPage();
        refreshEvaluation();
        refreshCompensatorEvaluation();
    }

private:
    void switchSubPage(int idx) {
        sub_stack->setCurrentIndex(idx);
        QString active_ss = "QPushButton { background-color: #2563eb; color: #ffffff; border: 1px solid #38bdf8; border-radius: 5px; padding: 5px 12px; font-weight: bold; font-size: 11px; }";
        QString idle_ss   = QString("QPushButton { background-color: %1; color: %2; border: 1px solid %3; border-radius: 5px; padding: 5px 12px; font-weight: bold; font-size: 11px; }")
                            .arg(Theme::BG_INPUT, Theme::TEXT_SECONDARY, Theme::BORDER_INPUT);
        tab_btn_fb->setStyleSheet(idx == 0 ? active_ss : idle_ss);
        tab_btn_synth->setStyleSheet(idx == 1 ? active_ss : idle_ss);
        tab_btn_comp->setStyleSheet(idx == 2 ? active_ss : idle_ss);

        if (idx == 0) {
            top_subtitle_lbl->setText("Feedback H(s) & Closed-Loop T(s) Menu: Enter Num_H(s) & Den_H(s) just like G(s), check P / PI / PD / PID, and inspect the resulting symbolic & numerical T(s)!");
            refreshFeedbackPage();
        } else if (idx == 1) {
            top_subtitle_lbl->setText("Keep (🔒) vs. Change (🔄) Synthesizer: Lock fixed design specs, solve changed parameters, and click '▶ Show Calc' on any row to see how it is calculated.");
            refreshEvaluation();
        } else {
            top_subtitle_lbl->setText("Root Locus Compensator Designer: Synthesize Lead, PD, Lag, or PI pole-zero compensators for Test Point s_d and overlay the compensated Root Locus against the dashed Ghost uncompensated locus!");
            refreshCompensatorEvaluation();
        }
    }

    QWidget* buildCompensatorDesignPage() {
        auto* page = new QWidget();
        auto* vl = new QVBoxLayout(page);
        vl->setContentsMargins(0, 0, 0, 0);
        vl->setSpacing(8);

        // 1. Top Controls Card
        auto* ctrl_card = makeCard();
        auto* cl = new QVBoxLayout(ctrl_card);
        cl->setContentsMargins(12, 10, 12, 10);
        cl->setSpacing(8);

        auto* row1 = new QHBoxLayout();
        auto* t_lbl = new QLabel("Compensator Type:");
        t_lbl->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border:none;").arg(Theme::ACCENT_ZERO));
        comp_type_combo = new QComboBox();
        comp_type_combo->addItems({
            "Lead Compensator   Gc(s) = Kc (s + zc) / (s + pc)   [Δθ > 0°]",
            "Ideal PD Compensator   Gc(s) = Kc (s + zc)   [Single Zero]",
            "Lag Compensator   Gc(s) = Kc (s + zc) / (s + pc)   [Boost Static Error Constant]",
            "Ideal PI Compensator   Gc(s) = Kc (s + zc) / s   [Pole at Origin s = 0]"
        });
        comp_type_combo->setStyleSheet(QString("background-color: %1; color: %2; padding: 4px 8px; font-weight: bold; font-size: 11px;")
                                       .arg(Theme::BG_INPUT, Theme::TEXT_PRIMARY));

        auto* m_lbl = new QLabel("Placement Method:");
        m_lbl->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border:none;").arg(Theme::ACCENT_PID));
        comp_method_combo = new QComboBox();
        comp_method_combo->addItems({
            "Angle Bisector Method (Min Kc / Max α)",
            "Cancel Slowest Real Plant Pole (zc = -p_plant)",
            "User-Specified Zero Location zc (s = -zc)"
        });
        comp_method_combo->setStyleSheet(QString("background-color: %1; color: %2; padding: 4px 8px; font-weight: bold; font-size: 11px;")
                                         .arg(Theme::BG_INPUT, Theme::TEXT_PRIMARY));

        row1->addWidget(t_lbl);
        row1->addWidget(comp_type_combo, 1);
        row1->addSpacing(12);
        row1->addWidget(m_lbl);
        row1->addWidget(comp_method_combo, 1);
        cl->addLayout(row1);

        auto* row2 = new QHBoxLayout();
        auto* sd_lbl = new QLabel("Target Closed-Loop Pole  s_d = σ ± jωd :");
        sd_lbl->setStyleSheet("color: #facc15; font-weight: bold; font-size: 11px; border:none;");
        auto* re_l = new QLabel("Real (σ):");
        re_l->setStyleSheet("color: #94a3b8; font-size: 11px; font-weight: bold; border:none;");
        comp_sd_re_edit = makeDarkEntry(QString::number(model->test_point.real(), 'g', 4), 70);
        auto* im_l = new QLabel("Imag (jωd):");
        im_l->setStyleSheet("color: #94a3b8; font-size: 11px; font-weight: bold; border:none;");
        comp_sd_im_edit = makeDarkEntry(QString::number(std::abs(model->test_point.imag()) > 1e-4 ? std::abs(model->test_point.imag()) : 2.0, 'g', 4), 70);

        auto* pull_tp_btn = makeActionBtn("📍 Pull Current Test Point s", Theme::TEXT_PRIMARY, Theme::BG_INPUT);
        auto* pull_spec_btn = makeActionBtn("🎯 Pull from Locked (ζ, ωn)", Theme::ACCENT_ZERO, Theme::BG_INPUT);

        comp_param_lbl = new QLabel("Custom Zero zc (s = -zc):");
        comp_param_lbl->setStyleSheet("color: #38bdf8; font-size: 11px; font-weight: bold; border:none;");
        comp_param_edit = makeDarkEntry("2.0", 65);

        row2->addWidget(sd_lbl);
        row2->addWidget(re_l);
        row2->addWidget(comp_sd_re_edit);
        row2->addWidget(im_l);
        row2->addWidget(comp_sd_im_edit);
        row2->addWidget(pull_tp_btn);
        row2->addWidget(pull_spec_btn);
        row2->addStretch();
        row2->addWidget(comp_param_lbl);
        row2->addWidget(comp_param_edit);
        cl->addLayout(row2);

        comp_def_lbl = new QLabel();
        comp_def_lbl->setStyleSheet("color: #fde68a; font-family: Monospace; font-size: 11px; font-weight: bold; border:none;");
        cl->addWidget(comp_def_lbl);
        vl->addWidget(ctrl_card);

        // 2. Synthesized Gc(s) & Step-by-Step Trigonometric Derivation Card
        auto* math_card = makeCard();
        auto* ml = new QVBoxLayout(math_card);
        ml->setContentsMargins(12, 10, 12, 10);
        ml->setSpacing(8);

        auto* gc_hdr = new QLabel("1. SYNTHESIZED COMPENSATOR TRANSFER FUNCTION  Gc(s)  &  REQUIRED LOOP GAIN  Kc");
        gc_hdr->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border:none;").arg(Theme::ACCENT_PID));
        comp_gc_math_w = new MathFormulaWidget("", QColor(Theme::ACCENT_PID), QColor(Theme::BG_INPUT), 12);

        auto* st_hdr = new QLabel("2. STEP-BY-STEP GEOMETRIC & TRIGONOMETRIC SYNTHESIS AT  s_d");
        st_hdr->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border:none;").arg(Theme::ACCENT_ZERO));
        comp_steps_math_w = new MathFormulaWidget("", QColor(Theme::ACCENT_ZERO), QColor(Theme::BG_INPUT), 11);

        comp_verdict_lbl = new QLabel();
        comp_verdict_lbl->setWordWrap(true);
        comp_verdict_lbl->setStyleSheet("color: #a7f3d0; font-family: Monospace; font-size: 11px; font-weight: bold; border:none;");

        ml->addWidget(gc_hdr);
        ml->addWidget(comp_gc_math_w);
        ml->addWidget(st_hdr);
        ml->addWidget(comp_steps_math_w, 1);
        ml->addWidget(comp_verdict_lbl);
        vl->addWidget(math_card, 1);

        // 3. Bottom Action Bar: Apply with Ghost RL Overlay or Restore Uncompensated Plant
        auto* act_card = makeCard();
        auto* al = new QHBoxLayout(act_card);
        al->setContentsMargins(12, 8, 12, 8);

        auto* ghost_info = new QLabel("👻 Applying snapshots the uncompensated plant as a dashed Ghost Root Locus so you can compare both curves side-by-side.");
        ghost_info->setStyleSheet("color: #94a3b8; font-size: 11px; border:none;");

        auto* restore_btn = makeActionBtn("↺ Restore Uncompensated Plant", "#f87171", Theme::BG_INPUT);
        auto* apply_comp_btn = makeActionBtn("✔ Apply Compensator & Pin Ghost RL ➔", "#0b0e14", Theme::ACCENT_PID);

        al->addWidget(ghost_info, 1);
        al->addWidget(restore_btn);
        al->addSpacing(8);
        al->addWidget(apply_comp_btn);
        vl->addWidget(act_card);

        // Connections
        connect(comp_type_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int idx) {
            if (idx == 0) { // Lead
                comp_method_combo->setEnabled(true);
                int m_idx = comp_method_combo->currentIndex();
                comp_param_lbl->setText("Custom Zero zc (s = -zc):");
                comp_param_lbl->setVisible(m_idx == 2);
                comp_param_edit->setVisible(m_idx == 2);
            } else if (idx == 1) { // Ideal PD
                comp_method_combo->setEnabled(false);
                comp_param_lbl->setVisible(false);
                comp_param_edit->setVisible(false);
            } else if (idx == 2) { // Lag
                comp_method_combo->setEnabled(false);
                comp_param_lbl->setText("Error Boost Ratio β = zc/pc (>1):");
                comp_param_edit->setText("5.0");
                comp_param_lbl->setVisible(true);
                comp_param_edit->setVisible(true);
            } else { // Ideal PI
                comp_method_combo->setEnabled(false);
                comp_param_lbl->setText("PI Zero Location zc (s = -zc):");
                comp_param_edit->setText("0.2");
                comp_param_lbl->setVisible(true);
                comp_param_edit->setVisible(true);
            }
            refreshCompensatorEvaluation();
        });

        connect(comp_method_combo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int m_idx) {
            bool show_p = (comp_type_combo->currentIndex() == 0 && m_idx == 2);
            comp_param_lbl->setVisible(show_p);
            comp_param_edit->setVisible(show_p);
            refreshCompensatorEvaluation();
        });

        for (auto* ed : {comp_sd_re_edit, comp_sd_im_edit, comp_param_edit}) {
            connect(ed, &QLineEdit::textEdited, this, [this]() { refreshCompensatorEvaluation(); });
        }

        connect(pull_tp_btn, &QPushButton::clicked, this, [this]() {
            comp_sd_re_edit->setText(QString::number(model->test_point.real(), 'g', 4));
            comp_sd_im_edit->setText(QString::number(std::abs(model->test_point.imag()), 'g', 4));
            refreshCompensatorEvaluation();
        });

        connect(pull_spec_btn, &QPushButton::clicked, this, [this]() {
            refreshEvaluation();
            double sig = last_analysis.solved["sigma_d"];
            double wd  = last_analysis.solved["omega_d"];
            comp_sd_re_edit->setText(QString::number(- std::abs(sig), 'g', 4));
            comp_sd_im_edit->setText(QString::number(std::abs(wd), 'g', 4));
            refreshCompensatorEvaluation();
        });

        connect(apply_comp_btn, &QPushButton::clicked, this, [this]() {
            refreshCompensatorEvaluation();
            if (last_comp_res.valid && onApplyCompensator) {
                onApplyCompensator(last_comp_res);
            }
        });

        connect(restore_btn, &QPushButton::clicked, this, [this]() {
            if (onRestoreUncompensated) onRestoreUncompensated();
            refreshCompensatorEvaluation();
        });

        comp_param_lbl->setVisible(false);
        comp_param_edit->setVisible(false);
        return page;
    }

    void refreshCompensatorEvaluation() {
        if (!comp_sd_re_edit) return;
        double sig = parseNumericInput(comp_sd_re_edit->text().toStdString(), -2.0);
        double wd  = std::abs(parseNumericInput(comp_sd_im_edit->text().toStdString(), 2.0));
        Complex sd(sig, wd);

        auto loop_poles = model->uncompensatedLoopPolesS();
        auto loop_zeros = model->uncompensatedLoopZerosS();
        auto ev = evaluateTransferFunction(loop_poles, loop_zeros, sd, 1.0);
        double def_deg = computeAngleDeficiency(ev.angle_deg); // in [-180, 180]

        comp_def_lbl->setText(QString(
            "Uncompensated Evaluation @ s_d = %1 + j%2 :   ∠G(s_d)H(s_d) = %3°   •   |G(s_d)H(s_d)| = %4   •   Angle Deficiency Δθ = %5°")
            .arg(QString::fromStdString(PRECISION.fmt(sig)))
            .arg(QString::fromStdString(PRECISION.fmt(wd)))
            .arg(QString::fromStdString(PRECISION.fmt(ev.angle_deg)))
            .arg(QString::fromStdString(PRECISION.fmt(ev.magnitude)))
            .arg(QString::fromStdString(PRECISION.fmt(def_deg, true))));

        RootLocusCompensatorResult res;
        res.s_target = sd;
        res.angle_def_deg = def_deg;

        int type_idx = comp_type_combo->currentIndex();
        int meth_idx = comp_method_combo->currentIndex();
        double param_v = std::max(1e-4, std::abs(parseNumericInput(comp_param_edit->text().toStdString(), 2.0)));

        double phi_deg = std::clamp(std::abs(def_deg), 1.0, 85.0);
        double phi_rad = phi_deg * PI / 180.0;

        if (type_idx == 0) {
            // LEAD COMPENSATOR
            res.comp_type = "Lead";
            res.has_zero = true;
            res.has_pole = true;
            if (meth_idx == 0) {
                // Angle Bisector Method
                res.method_name = "Angle Bisector Method";
                // Angle of s_d from positive real axis ispsi in (0, pi)
                double psi = std::atan2(wd, sig); // rad, typically in (pi/2, pi) for LHP
                double bisector = 0.5 * (PI + psi);
                double ang_z = bisector - 0.5 * phi_rad;
                double ang_p = bisector + 0.5 * phi_rad;
                // From geometry: -z_c = sig + wd / tan(pi - ang_z) => z_c = -sig + wd * cot(pi - ang_z)
                res.z_c = std::max(1e-3, -sig - wd / std::tan(ang_z));
                res.p_c = std::max(res.z_c + 1e-3, -sig - wd / std::tan(ang_p));
                res.steps_latex =
                    "\\phi_{\\mathrm{lead}} = \\Delta\\theta = " + PRECISION.fmt(phi_deg) + "^\\circ, \\quad "
                    "\\psi = \\angle s_d = " + PRECISION.fmt(psi * 180.0 / PI) + "^\\circ \\implies "
                    "z_c = " + PRECISION.fmt(res.z_c) + ", \\; p_c = " + PRECISION.fmt(res.p_c);
            } else if (meth_idx == 1) {
                // Pole-Cancellation Method: pick slowest negative real pole (excluding s=0)
                res.method_name = "Pole-Cancellation Method";
                double p_cancel = -sig;
                for (const auto& p : model->poles) {
                    if (std::abs(p.imag()) < 1e-5 && p.real() < -1e-4) {
                        if (std::abs(p.real()) < p_cancel || p_cancel == -sig) p_cancel = -p.real();
                    }
                }
                res.z_c = std::max(1e-3, p_cancel);
                double theta_z = std::atan2(wd, sig + res.z_c);
                double theta_p = std::clamp(theta_z - phi_rad, 0.05, PI - 0.05);
                res.p_c = std::max(1e-3, -sig + wd / std::tan(theta_p));
                res.steps_latex =
                    "z_c = p_{\\mathrm{plant}} = " + PRECISION.fmt(res.z_c) + " \\implies "
                    "\\theta_z = " + PRECISION.fmt(theta_z * 180.0 / PI) + "^\\circ, \\quad "
                    "\\theta_p = \\theta_z - \\Delta\\theta = " + PRECISION.fmt(theta_p * 180.0 / PI) + "^\\circ \\implies "
                    "p_c = -\\sigma_d + \\frac{\\omega_d}{\\tan\\theta_p} = " + PRECISION.fmt(res.p_c);
            } else {
                // User-Specified Zero z_c
                res.method_name = "User-Specified Zero z_c";
                res.z_c = param_v;
                double theta_z = std::atan2(wd, sig + res.z_c);
                double theta_p = std::clamp(theta_z - phi_rad, 0.05, PI - 0.05);
                res.p_c = std::max(1e-3, -sig + wd / std::tan(theta_p));
                res.steps_latex =
                    "z_c = " + PRECISION.fmt(res.z_c) + " \\implies "
                    "\\theta_z = " + PRECISION.fmt(theta_z * 180.0 / PI) + "^\\circ, \\quad "
                    "\\theta_p = \\theta_z - \\Delta\\theta = " + PRECISION.fmt(theta_p * 180.0 / PI) + "^\\circ \\implies "
                    "p_c = " + PRECISION.fmt(res.p_c);
            }
        } else if (type_idx == 1) {
            // IDEAL PD COMPENSATOR: Gc(s) = Kc (s + zc)
            res.comp_type = "PD";
            res.method_name = "Exact Angle Deficiency Zero Placement";
            res.has_zero = true;
            res.has_pole = false;
            res.z_c = std::max(1e-3, -sig + wd / std::tan(phi_rad));
            res.p_c = 0.0;
            res.steps_latex =
                "\\theta_z = \\Delta\\theta = " + PRECISION.fmt(phi_deg) + "^\\circ \\implies "
                "z_c = -\\sigma_d + \\frac{\\omega_d}{\\tan(\\Delta\\theta)} = " + PRECISION.fmt(res.z_c);
        } else if (type_idx == 2) {
            // LAG COMPENSATOR: Gc(s) = Kc (s + zc) / (s + pc) with zc close to origin
            res.comp_type = "Lag";
            res.method_name = "Small-Angle Origin Placement (zc = |σd|/10)";
            res.has_zero = true;
            res.has_pole = true;
            double beta = std::max(1.05, param_v);
            res.z_c = std::max(0.02, std::abs(sig) * 0.10);
            res.p_c = res.z_c / beta;
            double lag_ang = (std::atan2(wd, sig + res.z_c) - std::atan2(wd, sig + res.p_c)) * 180.0 / PI;
            res.steps_latex =
                "\\beta = \\frac{z_c}{p_c} = " + PRECISION.fmt(beta) + ", \\quad "
                "z_c = 0.1|\\sigma_d| = " + PRECISION.fmt(res.z_c) + " \\implies "
                "p_c = \\frac{z_c}{\\beta} = " + PRECISION.fmt(res.p_c) + " \\quad "
                "(\\Delta\\theta_{\\mathrm{lag}} = " + PRECISION.fmt(lag_ang) + "^\\circ)";
        } else {
            // IDEAL PI COMPENSATOR: Gc(s) = Kc (s + zc) / s
            res.comp_type = "PI";
            res.method_name = "Integrator at s = 0 + Nearby Zero s = -zc";
            res.has_zero = true;
            res.has_pole = true;
            res.z_c = std::max(0.01, param_v);
            res.p_c = 0.0;
            double pi_ang = (std::atan2(wd, sig + res.z_c) - std::atan2(wd, sig)) * 180.0 / PI;
            res.steps_latex =
                "p_c = 0, \\quad z_c = " + PRECISION.fmt(res.z_c) + " \\quad "
                "(\\Delta\\theta_{\\mathrm{PI}} = " + PRECISION.fmt(pi_ang) + "^\\circ \\text{ at } s_d)";
        }

        // Compute compensated magnitude & required loop gain Kc at s_d
        std::vector<Complex> comp_poles = loop_poles;
        std::vector<Complex> comp_zeros = loop_zeros;
        if (res.has_zero) comp_zeros.push_back(Complex(-res.z_c, 0.0));
        if (res.has_pole) comp_poles.push_back(Complex(-res.p_c, 0.0));

        auto ev_comp = evaluateTransferFunction(comp_poles, comp_zeros, sd, 1.0);
        res.k_c = std::isfinite(ev_comp.rl_gain_k) ? ev_comp.rl_gain_k : 1.0;
        res.valid = true;

        if (res.comp_type == "PD") {
            res.gc_latex = "G_c(s) = K_c (s + " + PRECISION.fmt(res.z_c) + "), \\quad K_c = " + PRECISION.fmt(res.k_c);
        } else if (res.comp_type == "PI") {
            res.gc_latex = "G_c(s) = K_c \\frac{s + " + PRECISION.fmt(res.z_c) + "}{s}, \\quad K_c = " + PRECISION.fmt(res.k_c);
        } else {
            res.gc_latex = "G_c(s) = K_c \\frac{s + " + PRECISION.fmt(res.z_c) + "}{s + " + PRECISION.fmt(res.p_c) + "}, \\quad K_c = " + PRECISION.fmt(res.k_c);
        }

        double new_def = computeAngleDeficiency(ev_comp.angle_deg);
        res.summary_text = "✔ Compensated Loop @ s_d:  ∠Gc(s_d)G(s_d)H(s_d) = " + PRECISION.fmt(ev_comp.angle_deg) +
                           "°  (|Δθ| = " + PRECISION.fmt(std::abs(new_def)) + "°)  •  Required Gain Kc = " + PRECISION.fmt(res.k_c);

        comp_gc_math_w->setLatex(QString::fromStdString(res.gc_latex));
        comp_steps_math_w->setLatex(QString::fromStdString(res.steps_latex));
        comp_verdict_lbl->setText(QString::fromStdString(res.summary_text));
        last_comp_res = res;
    }

    QWidget* buildFeedbackAndTransferFunctionPage() {
        auto* page = new QWidget();
        auto* pv = new QVBoxLayout(page);
        pv->setContentsMargins(0, 0, 0, 0);
        pv->setSpacing(8);

        auto* top_card = makeCard();
        auto* top_v = new QVBoxLayout(top_card);
        top_v->setContentsMargins(10, 6, 10, 6);
        top_v->setSpacing(6);

        // Row 1: Feedback Path Mode
        auto* fb_row = new QHBoxLayout();
        auto* fb_lbl = new QLabel("Feedback Path Mode:");
        fb_lbl->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border:none;").arg(Theme::ACCENT_ZERO));

        fb_unity_chk  = new QCheckBox("Unity Feedback H(s) = 1");
        fb_custom_chk = new QCheckBox("Custom Non-Unity Feedback H(s) = Kh · Num_H(s) / Den_H(s)");
        fb_unity_chk->setChecked(model->unity_feedback_s);
        fb_custom_chk->setChecked(!model->unity_feedback_s);
        fb_unity_chk->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px;").arg(Theme::ACCENT_PID));
        fb_custom_chk->setStyleSheet("color: #c084fc; font-weight: bold; font-size: 11px;");

        auto* reset_h_btn = makeActionBtn("Reset H(s) to Unity (1)", Theme::TEXT_PRIMARY, Theme::BG_INPUT);
        fb_row->addWidget(fb_lbl);
        fb_row->addSpacing(8);
        fb_row->addWidget(fb_unity_chk);
        fb_row->addSpacing(12);
        fb_row->addWidget(fb_custom_chk);
        fb_row->addStretch();
        fb_row->addWidget(reset_h_btn);
        top_v->addLayout(fb_row);

        // Row 2: Controller Type C(s) & Gains
        auto* ctrl_row = new QHBoxLayout();
        auto* ctrl_lbl = new QLabel("Controller Type C(s):");
        ctrl_lbl->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border:none;").arg(Theme::ACCENT_PID));

        ctrl_p_chk    = new QCheckBox("P   [Kp]");
        ctrl_pi_chk   = new QCheckBox("PI   [Kp + Ki/s]");
        ctrl_pd_chk   = new QCheckBox("PD   [Kp + Kd·s]");
        ctrl_pid_chk  = new QCheckBox("PID   [Kp + Ki/s + Kd·s]");
        ctrl_none_chk = new QCheckBox("None   [C(s)=1]");

        active_ctrl_type = !model->pid_enabled ? "None" :
            ((std::abs(model->ki) > 1e-7 && std::abs(model->kd) > 1e-7) ? "PID" :
             (std::abs(model->ki) > 1e-7 ? "PI" : (std::abs(model->kd) > 1e-7 ? "PD" : "P")));

        for (auto* cb : {ctrl_p_chk, ctrl_pi_chk, ctrl_pd_chk, ctrl_pid_chk, ctrl_none_chk}) {
            cb->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; font-size: 11px;").arg(Theme::TEXT_PRIMARY));
        }

        fb_kp_lbl = new QLabel("Kp:"); fb_kp_lbl->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border:none;").arg(Theme::ACCENT_ZERO));
        fb_kp_edit = makeDarkEntry(QString::number(model->kp, 'g', 4), 52);
        fb_ki_lbl = new QLabel("Ki:"); fb_ki_lbl->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border:none;").arg(Theme::ACCENT_ZERO));
        fb_ki_edit = makeDarkEntry(QString::number(model->ki, 'g', 4), 52);
        fb_kd_lbl = new QLabel("Kd:"); fb_kd_lbl->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border:none;").arg(Theme::ACCENT_ZERO));
        fb_kd_edit = makeDarkEntry(QString::number(model->kd, 'g', 4), 52);

        ctrl_row->addWidget(ctrl_lbl);
        ctrl_row->addSpacing(6);
        ctrl_row->addWidget(ctrl_p_chk);
        ctrl_row->addWidget(ctrl_pi_chk);
        ctrl_row->addWidget(ctrl_pd_chk);
        ctrl_row->addWidget(ctrl_pid_chk);
        ctrl_row->addWidget(ctrl_none_chk);
        ctrl_row->addStretch();
        ctrl_row->addWidget(fb_kp_lbl); ctrl_row->addWidget(fb_kp_edit);
        ctrl_row->addWidget(fb_ki_lbl); ctrl_row->addWidget(fb_ki_edit);
        ctrl_row->addWidget(fb_kd_lbl); ctrl_row->addWidget(fb_kd_edit);
        top_v->addLayout(ctrl_row);
        pv->addWidget(top_card);

        auto* split = new QSplitter(Qt::Horizontal);
        pv->addWidget(split, 1);

        // Left Column
        auto* left_w = new QWidget();
        auto* left_v = new QVBoxLayout(left_w);
        left_v->setContentsMargins(0, 0, 4, 0);
        left_v->setSpacing(6);

        // 1. Feedback Path H(s) Card
        auto* h_card = makeCard();
        auto* h_l = new QVBoxLayout(h_card);
        h_l->setContentsMargins(10, 6, 10, 6);
        auto* h_hdr = new QHBoxLayout();
        auto* h_title = new QLabel("Feedback Path  H(s) = Kh · Num_H(s) / Den_H(s)");
        h_title->setStyleSheet("color: #c084fc; font-weight: bold; font-size: 11px; border:none;");
        auto* solve_h_btn = makeActionBtn("Solve & Apply H(s) ➔", "#0b0e14", Theme::ACCENT_PID);
        h_hdr->addWidget(h_title); h_hdr->addStretch(); h_hdr->addWidget(solve_h_btn);
        h_l->addLayout(h_hdr);

        auto* h_grid = new QHBoxLayout();
        auto* nh_lbl = new QLabel("Num_H(s):"); nh_lbl->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));
        num_h_edit = makeDarkEntry(QString::fromStdString(model->h_num_str));
        auto* dh_lbl = new QLabel("Den_H(s):"); dh_lbl->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; border:none;").arg(Theme::ACCENT_POLE));
        den_h_edit = makeDarkEntry(QString::fromStdString(model->h_den_str));
        auto* kh_lbl = new QLabel("Kh:"); kh_lbl->setStyleSheet("color: #94a3b8; font-family: Monospace; font-weight: bold; border:none;");
        kh_edit = makeDarkEntry(QString::number(model->h_gain, 'g', 4), 48);
        h_grid->addWidget(nh_lbl); h_grid->addWidget(num_h_edit, 1);
        h_grid->addWidget(dh_lbl); h_grid->addWidget(den_h_edit, 1);
        h_grid->addWidget(kh_lbl); h_grid->addWidget(kh_edit);
        h_l->addLayout(h_grid);

        h_status_lbl = new QLabel("✔ H(s) Solved");
        h_status_lbl->setStyleSheet("color: #94a3b8; font-size: 10px; border:none;");
        h_l->addWidget(h_status_lbl);

        h_math_w = new MathFormulaWidget("", QColor("#c084fc"), QColor(Theme::BG_INPUT), 10);
        h_l->addWidget(h_math_w);
        left_v->addWidget(h_card);

        // 2. H(s) Poles & Zeros Cards
        auto* hpz_row = new QHBoxLayout();
        h_poles_card = new PoleZeroEditorCard("H(s) Poles (×)", Theme::ACCENT_POLE, -1.0);
        h_zeros_card = new PoleZeroEditorCard("H(s) Zeros (○)", Theme::ACCENT_ZERO, -2.0);
        h_poles_card->peer_card = h_zeros_card;
        h_zeros_card->peer_card = h_poles_card;
        h_poles_card->syncHeightWithPeer();
        hpz_row->addWidget(h_poles_card, 1);
        hpz_row->addWidget(h_zeros_card, 1);
        left_v->addLayout(hpz_row);

        // 3. Forward Plant G(s) (Synced) Card
        auto* g_card = makeCard();
        auto* g_l = new QVBoxLayout(g_card);
        g_l->setContentsMargins(10, 6, 10, 6);
        auto* g_hdr = new QHBoxLayout();
        auto* g_title = new QLabel("Forward Plant  G(s) = K · Num_G(s) / Den_G(s)  (Synced)");
        g_title->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border:none;").arg(Theme::ACCENT_ZERO));
        auto* upd_g_btn = makeActionBtn("Update G(s) ➔", "#0b0e14", Theme::ACCENT_PID);
        g_hdr->addWidget(g_title); g_hdr->addStretch(); g_hdr->addWidget(upd_g_btn);
        g_l->addLayout(g_hdr);

        auto* g_grid = new QHBoxLayout();
        auto* ng_lbl = new QLabel("Num_G(s):"); ng_lbl->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));
        num_g_edit = makeDarkEntry(QString::fromStdString(formatPolynomialFromRoots(model->zeros, "s")));
        auto* dg_lbl = new QLabel("Den_G(s):"); dg_lbl->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; border:none;").arg(Theme::ACCENT_POLE));
        den_g_edit = makeDarkEntry(QString::fromStdString(formatPolynomialFromRoots(model->poles, "s")));
        auto* kg_lbl = new QLabel("K:"); kg_lbl->setStyleSheet("color: #94a3b8; font-family: Monospace; font-weight: bold; border:none;");
        kg_edit = makeDarkEntry(QString::number(model->plant_gain, 'g', 4), 48);
        g_grid->addWidget(ng_lbl); g_grid->addWidget(num_g_edit, 1);
        g_grid->addWidget(dg_lbl); g_grid->addWidget(den_g_edit, 1);
        g_grid->addWidget(kg_lbl); g_grid->addWidget(kg_edit);
        g_l->addLayout(g_grid);

        g_math_w = new MathFormulaWidget("", QColor(Theme::ACCENT_ZERO), QColor(Theme::BG_INPUT), 10);
        c_math_w = new MathFormulaWidget("", QColor(Theme::ACCENT_PID), QColor(Theme::BG_INPUT), 10);
        g_l->addWidget(g_math_w);
        g_l->addWidget(c_math_w);
        left_v->addWidget(g_card);

        // Right Column: Resulting Closed-Loop Transfer Function T(s) = Y(s)/R(s)
        auto* right_card = makeCard();
        auto* right_v = new QVBoxLayout(right_card);
        right_v->setContentsMargins(12, 8, 12, 8);
        right_v->setSpacing(8);

        auto* r_title = new QLabel("Resulting Closed-Loop Transfer Function  T(s) = Y(s) / R(s)");
        r_title->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 13px; border:none;").arg(Theme::ACCENT_PID));
        right_v->addWidget(r_title);

        // Box 1: Symbolic Closed-Loop T(s)
        auto* sym_box = makeCard();
        sym_box->setStyleSheet(QString("QFrame { background-color: %1; border: 1px solid #a855f7; border-radius: 6px; }").arg(Theme::BG_INPUT));
        auto* sym_l = new QVBoxLayout(sym_box);
        auto* sym_t = new QLabel("1. SYMBOLIC CLOSED-LOOP T(s)  (In Terms of Selected Controller Gains Kp, Ki, Kd)");
        sym_t->setStyleSheet("color: #c084fc; font-weight: bold; font-size: 10px; border:none;");
        t_sym_math_w = new MathFormulaWidget("", QColor("#c084fc"), QColor(Theme::BG_INPUT), 11);
        sym_l->addWidget(sym_t);
        sym_l->addWidget(t_sym_math_w);

        auto* sym_grid = new QGridLayout();
        auto* nts_l = new QLabel("Num_T(s):"); nts_l->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));
        num_t_sym_edit = makeDarkEntry(); num_t_sym_edit->setReadOnly(true);
        auto* dts_l = new QLabel("Den_T(s):"); dts_l->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; border:none;").arg(Theme::ACCENT_POLE));
        den_t_sym_edit = makeDarkEntry(); den_t_sym_edit->setReadOnly(true);
        sym_grid->addWidget(nts_l, 0, 0); sym_grid->addWidget(num_t_sym_edit, 0, 1);
        sym_grid->addWidget(dts_l, 1, 0); sym_grid->addWidget(den_t_sym_edit, 1, 1);
        sym_l->addLayout(sym_grid);
        right_v->addWidget(sym_box);

        // Box 2: Evaluated Numerical Closed-Loop T(s)
        auto* ev_box = makeCard();
        ev_box->setStyleSheet(QString("QFrame { background-color: %1; border: 1px solid %2; border-radius: 6px; }").arg(Theme::BG_INPUT, Theme::BORDER_INPUT));
        auto* ev_l = new QVBoxLayout(ev_box);
        auto* ev_t = new QLabel("2. EVALUATED NUMERICAL CLOSED-LOOP T(s)  (With Current Kp, Ki, Kd Values Plugged In)");
        ev_t->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 10px; border:none;").arg(Theme::ACCENT_ZERO));
        t_eval_math_w = new MathFormulaWidget("", QColor(Theme::ACCENT_ZERO), QColor(Theme::BG_INPUT), 11);
        ev_l->addWidget(ev_t);
        ev_l->addWidget(t_eval_math_w);

        auto* ev_grid = new QGridLayout();
        auto* nte_l = new QLabel("Eval Num_T(s):"); nte_l->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));
        num_t_eval_edit = makeDarkEntry(); num_t_eval_edit->setReadOnly(true);
        auto* dte_l = new QLabel("Eval Den_T(s):"); dte_l->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; border:none;").arg(Theme::ACCENT_POLE));
        den_t_eval_edit = makeDarkEntry(); den_t_eval_edit->setReadOnly(true);
        ev_grid->addWidget(nte_l, 0, 0); ev_grid->addWidget(num_t_eval_edit, 0, 1);
        ev_grid->addWidget(dte_l, 1, 0); ev_grid->addWidget(den_t_eval_edit, 1, 1);
        ev_l->addLayout(ev_grid);
        right_v->addWidget(ev_box);

        // Box 3: Open-Loop Return Ratio L(s) & Closed-Loop Poles/Zeros
        auto* ol_box = makeCard();
        ol_box->setStyleSheet(QString("QFrame { background-color: %1; border: 1px solid %2; border-radius: 6px; }").arg(Theme::BG_INPUT, Theme::BORDER_INPUT));
        auto* ol_l = new QVBoxLayout(ol_box);
        auto* ol_t = new QLabel("3. OPEN-LOOP RETURN RATIO  L(s) = C(s)G(s)H(s)  &  CLOSED-LOOP POLES / ZEROS");
        ol_t->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 10px; border:none;").arg(Theme::ACCENT_PID));
        l_open_math_w = new MathFormulaWidget("", QColor(Theme::ACCENT_PID), QColor(Theme::BG_INPUT), 11);
        cl_telemetry_lbl = new QLabel();
        cl_telemetry_lbl->setStyleSheet(QString("color: %1; font-family: Monospace; font-size: 11px; font-weight: bold; border:none;").arg(Theme::TEXT_PRIMARY));
        ol_l->addWidget(ol_t);
        ol_l->addWidget(l_open_math_w);
        ol_l->addWidget(cl_telemetry_lbl);
        right_v->addWidget(ol_box, 1);

        split->addWidget(left_w);
        split->addWidget(right_card);
        split->setStretchFactor(0, 5);
        split->setStretchFactor(1, 6);

        // Connections
        connect(fb_unity_chk, &QCheckBox::clicked, this, [this]() {
            model->unity_feedback_s = true;
            fb_unity_chk->setChecked(true);
            fb_custom_chk->setChecked(false);
            refreshFeedbackPage();
            model->notify("feedback_dlg");
        });
        connect(fb_custom_chk, &QCheckBox::clicked, this, [this]() {
            model->unity_feedback_s = false;
            fb_unity_chk->setChecked(false);
            fb_custom_chk->setChecked(true);
            refreshFeedbackPage();
            model->notify("feedback_dlg");
        });
        connect(reset_h_btn, &QPushButton::clicked, this, [this]() {
            model->unity_feedback_s = true;
            model->h_poles.clear();
            model->h_zeros.clear();
            model->h_gain = 1.0;
            model->h_num_str = "1";
            model->h_den_str = "1";
            num_h_edit->setText("1");
            den_h_edit->setText("1");
            kh_edit->setText("1");
            refreshFeedbackPage();
            model->notify("feedback_dlg");
        });

        auto setCtrlMode = [this](const std::string& m) {
            active_ctrl_type = m;
            model->pid_enabled = (m != "None");
            refreshFeedbackPage();
            model->notify("feedback_dlg");
        };
        connect(ctrl_p_chk,    &QCheckBox::clicked, this, [setCtrlMode]() { setCtrlMode("P"); });
        connect(ctrl_pi_chk,   &QCheckBox::clicked, this, [setCtrlMode]() { setCtrlMode("PI"); });
        connect(ctrl_pd_chk,   &QCheckBox::clicked, this, [setCtrlMode]() { setCtrlMode("PD"); });
        connect(ctrl_pid_chk,  &QCheckBox::clicked, this, [setCtrlMode]() { setCtrlMode("PID"); });
        connect(ctrl_none_chk, &QCheckBox::clicked, this, [setCtrlMode]() { setCtrlMode("None"); });

        for (auto* ed : {fb_kp_edit, fb_ki_edit, fb_kd_edit}) {
            connect(ed, &QLineEdit::editingFinished, this, [this]() {
                if (updating) return;
                model->kp = parseNumericInput(fb_kp_edit->text().toStdString(), model->kp);
                model->ki = parseNumericInput(fb_ki_edit->text().toStdString(), model->ki);
                model->kd = parseNumericInput(fb_kd_edit->text().toStdString(), model->kd);
                (*input_map)["kp"] = model->kp;
                (*input_map)["ki"] = model->ki;
                (*input_map)["kd"] = model->kd;
                refreshFeedbackPage();
                model->notify("feedback_dlg");
            });
        }

        auto applyHPoly = [this]() {
            try {
                model->h_num_str = num_h_edit->text().toStdString();
                model->h_den_str = den_h_edit->text().toStdString();
                auto pn = parsePolynomialInput(model->h_num_str);
                auto pd = parsePolynomialInput(model->h_den_str);
                model->h_zeros = pn.roots;
                model->h_poles = pd.roots;
                model->h_gain  = parseNumericInput(kh_edit->text().toStdString(), 1.0);
                model->unity_feedback_s = false;
                refreshFeedbackPage();
                model->notify("feedback_dlg");
            } catch (const std::exception& e) {
                h_status_lbl->setText(QString("⚠ H(s) Error: %1").arg(e.what()));
            }
        };
        connect(solve_h_btn, &QPushButton::clicked, this, applyHPoly);
        connect(num_h_edit, &QLineEdit::returnPressed, this, applyHPoly);
        connect(den_h_edit, &QLineEdit::returnPressed, this, applyHPoly);
        connect(kh_edit,    &QLineEdit::returnPressed, this, applyHPoly);

        h_poles_card->onValuesChanged = [this]() {
            if (updating) return;
            model->h_poles = h_poles_card->getValues();
            model->h_den_str = formatPolynomialFromRoots(model->h_poles, "s");
            refreshEvaluation();
            refreshFeedbackPage();
            model->notify("design_dialog");
        };

        h_zeros_card->onValuesChanged = [this]() {
            if (updating) return;
            model->h_zeros = h_zeros_card->getValues();
            model->h_num_str = formatPolynomialFromRoots(model->h_zeros, "s");
            refreshEvaluation();
            refreshFeedbackPage();
            model->notify("design_dialog");
        };

        auto applyGPoly = [this]() {
            try {
                auto pn = parsePolynomialInput(num_g_edit->text().toStdString());
                auto pd = parsePolynomialInput(den_g_edit->text().toStdString());
                model->zeros = pn.roots;
                model->poles = pd.roots;
                model->plant_gain = parseNumericInput(kg_edit->text().toStdString(), 1.0);
                refreshFeedbackPage();
                refreshEvaluation();
                model->notify("feedback_dlg");
            } catch (...) {}
        };
        connect(upd_g_btn,  &QPushButton::clicked, this, applyGPoly);
        connect(num_g_edit, &QLineEdit::returnPressed, this, applyGPoly);
        connect(den_g_edit, &QLineEdit::returnPressed, this, applyGPoly);
        connect(kg_edit,    &QLineEdit::returnPressed, this, applyGPoly);

        return page;
    }

    QWidget* buildKeepChangeSynthesizerPage() {
        auto* page = new QWidget();
        auto* main_l = new QVBoxLayout(page);
        main_l->setContentsMargins(0, 0, 0, 0);
        main_l->setSpacing(8);

        auto* preset_card = makeCard();
        auto* pr_v = new QVBoxLayout(preset_card);
        pr_v->setContentsMargins(10, 6, 10, 6);
        pr_v->setSpacing(6);

        auto* pr_top = new QHBoxLayout();
        auto* pr_lbl = new QLabel("Quick Setup:");
        pr_lbl->setStyleSheet("color: #94a3b8; font-weight: bold; font-size: 10px; border:none;");
        pr_top->addWidget(pr_lbl);

        for (const auto& [btn_txt, pat] : std::vector<std::pair<QString, std::string>>{
            {"P: Change Kp (%OS)", "p_os"},
            {"PD: Keep Kp, Change Kd (%OS)", "pd_kd"},
            {"PD: Keep Kd, Change Kp (%OS)", "pd_kp"},
            {"PD: Change Kp & Kd (%OS + Ts)", "pd_both"},
            {"PI: Keep Kp, Change Ki (%OS)", "pi_ki"},
            {"PID: Change Kp, Ki, Kd (%OS + Ts)", "pid_all"}
        }) {
            auto* b = makeActionBtn(btn_txt, Theme::TEXT_PRIMARY);
            connect(b, &QPushButton::clicked, this, [this, pat]() { applyQuickPattern(pat); });
            pr_top->addWidget(b);
        }
        pr_top->addStretch();
        pr_v->addLayout(pr_top);

        auto* pr_bot = new QHBoxLayout();
        show_all_calcs_chk = new QCheckBox("Show All Step-by-Step Calculations for 'Changed' Parameters");
        show_all_calcs_chk->setChecked(false);
        show_all_calcs_chk->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px;").arg(Theme::ACCENT_ZERO));

        auto* kmax_lbl = new QLabel("Max K-Slider Endpoint:");
        kmax_lbl->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border:none;").arg(Theme::ACCENT_PID));
        max_k_edit = makeDarkEntry(QString::number(model->max_k_slider, 'g', 5), 65);

        auto* design_btn = makeActionBtn("✔ Design & Apply Controller ➔", "#0b0e14", Theme::ACCENT_PID);

        pr_bot->addWidget(show_all_calcs_chk);
        pr_bot->addStretch();
        pr_bot->addWidget(kmax_lbl);
        pr_bot->addWidget(max_k_edit);
        pr_bot->addSpacing(10);
        pr_bot->addWidget(design_btn);
        pr_v->addLayout(pr_bot);
        main_l->addWidget(preset_card);

        auto* tbl_scroll = new QScrollArea();
        tbl_scroll->setWidgetResizable(true);
        tbl_scroll->setStyleSheet(QString("background-color: %1; border: 1px solid %2; border-radius: 6px;").arg(Theme::BG_PANEL, Theme::BORDER_PANEL));
        auto* tbl_w = new QWidget();
        auto* grid = new QGridLayout(tbl_w);
        grid->setContentsMargins(10, 8, 10, 8);
        grid->setVerticalSpacing(4);
        grid->setHorizontalSpacing(10);

        const std::vector<QString> headers = {
            "Mode (Click)", "Parameter", "Target / Kept Value", "Solved Value", "Role & Status", "Calculation (Optional)"
        };
        for (int c = 0; c < static_cast<int>(headers.size()); ++c) {
            auto* hl = new QLabel(headers[c]);
            hl->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border:none;").arg(Theme::ACCENT_ZERO));
            grid->addWidget(hl, 0, c);
        }

        const auto& metas = ParameterMatrixEngine::metaList();
        for (int r = 0; r < static_cast<int>(metas.size()); ++r) {
            const auto& m = metas[r];
            int main_row = r * 2 + 1;
            int calc_row = r * 2 + 2;

            bool is_lk = (*lock_map)[m.key];
            auto* lk_b = new QPushButton(is_lk ? "🔒 KEEP" : "🔄 CHANGE");
            lk_b->setCursor(Qt::PointingHandCursor);
            lk_b->setFixedWidth(105);
            lock_btns[m.key] = lk_b;

            QString unit_s = m.unit.isEmpty() ? "" : QString(" (%1)").arg(m.unit);
            auto* name_l = new QLabel(m.name + unit_s);
            name_l->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 11px; border:none;").arg(Theme::TEXT_PRIMARY));
            name_l->setToolTip(m.desc);

            auto* ed = makeDarkEntry(QString::number((*input_map)[m.key], 'g', 5), 95);
            val_edits[m.key] = ed;

            auto* sol_l = new QLabel("—");
            sol_l->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; font-size: 11px; border:none;").arg(Theme::ACCENT_ZERO));
            solved_lbls[m.key] = sol_l;

            auto* note_l = new QLabel("—");
            note_l->setStyleSheet("color: #94a3b8; font-size: 10px; border:none;");
            note_lbls[m.key] = note_l;

            auto* calc_btn = new QPushButton("▶ Show Calc");
            calc_btn->setCursor(Qt::PointingHandCursor);
            calc_btn->setFixedWidth(105);
            calc_toggle_btns[m.key] = calc_btn;
            calc_expanded[m.key] = false;

            auto* drawer = new QFrame();
            drawer->setStyleSheet(QString("QFrame { background-color: %1; border-left: 3px solid %2; border-radius: 4px; }")
                                  .arg(Theme::BG_INPUT, Theme::ACCENT_ZERO));
            auto* dr_l = new QHBoxLayout(drawer);
            dr_l->setContentsMargins(10, 4, 10, 4);
            auto* dr_tag = new QLabel("Step-by-Step Derivation:");
            dr_tag->setStyleSheet(QString("color: %1; font-size: 10px; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));
            auto* dr_math = new MathFormulaWidget("", QColor(Theme::ACCENT_ZERO), QColor(Theme::BG_INPUT), 10);
            dr_l->addWidget(dr_tag);
            dr_l->addWidget(dr_math, 1);
            drawer->setVisible(false);

            calc_drawer_frames[m.key] = drawer;
            calc_drawer_maths[m.key]  = dr_math;

            grid->addWidget(lk_b,     main_row, 0);
            grid->addWidget(name_l,   main_row, 1);
            grid->addWidget(ed,       main_row, 2);
            grid->addWidget(sol_l,    main_row, 3);
            grid->addWidget(note_l,   main_row, 4);
            grid->addWidget(calc_btn, main_row, 5);
            grid->addWidget(drawer,   calc_row, 1, 1, 5);

            std::string key_cp = m.key;
            connect(lk_b, &QPushButton::clicked, this, [this, key_cp]() {
                (*lock_map)[key_cp] = !(*lock_map)[key_cp];
                if ((*lock_map)[key_cp]) {
                    (*input_map)[key_cp] = last_analysis.solved[key_cp];
                    updating = true;
                    val_edits[key_cp]->setText(QString::number((*input_map)[key_cp], 'g', 5));
                    updating = false;
                }
                refreshEvaluation();
            });
            connect(ed, &QLineEdit::textEdited, this, [this, key_cp, ed]() {
                if (updating) return;
                (*input_map)[key_cp] = parseNumericInput(ed->text().toStdString(), 0.0);
                (*lock_map)[key_cp] = true;
                refreshEvaluation();
            });
            connect(calc_btn, &QPushButton::clicked, this, [this, key_cp]() {
                calc_expanded[key_cp] = !calc_expanded[key_cp];
                updateRowCalcVisibility(key_cp);
            });
        }
        tbl_scroll->setWidget(tbl_w);
        main_l->addWidget(tbl_scroll, 1);

        verdict_banner = new QFrame();
        auto* vb_l = new QVBoxLayout(verdict_banner);
        vb_l->setContentsMargins(12, 8, 12, 8);
        verdict_title = new QLabel("✔ POSSIBLE & SOLVED");
        verdict_desc  = new QLabel();
        verdict_desc->setWordWrap(true);

        bottom_formula_box = new QFrame();
        bottom_formula_box->setStyleSheet("border: none;");
        auto* bf_l = new QVBoxLayout(bottom_formula_box);
        bf_l->setContentsMargins(0, 4, 0, 0);
        auto* f_hdr = new QLabel("Primary Controller Synthesis Equations:");
        f_hdr->setStyleSheet("color: #94a3b8; font-size: 10px; font-weight: bold; border: none;");
        formula_math_w = new MathFormulaWidget("", QColor(Theme::ACCENT_ZERO), QColor(Theme::BG_INPUT), 11);
        bf_l->addWidget(f_hdr);
        bf_l->addWidget(formula_math_w);
        bottom_formula_box->setVisible(false);

        vb_l->addWidget(verdict_title);
        vb_l->addWidget(verdict_desc);
        vb_l->addWidget(bottom_formula_box);
        main_l->addWidget(verdict_banner);

        connect(show_all_calcs_chk, &QCheckBox::toggled, this, [this](bool checked) {
            for (const auto& m : ParameterMatrixEngine::metaList()) {
                calc_expanded[m.key] = checked && !(*lock_map)[m.key];
                updateRowCalcVisibility(m.key);
            }
            bottom_formula_box->setVisible(checked);
        });

        connect(max_k_edit, &QLineEdit::editingFinished, this, [this]() {
            double mk = std::max(1.0, parseNumericInput(max_k_edit->text().toStdString(), 100.0));
            model->max_k_slider = mk;
            if (onMaxKChanged) onMaxKChanged(mk);
        });

        connect(design_btn, &QPushButton::clicked, this, [this]() {
            double mk = std::max(1.0, parseNumericInput(max_k_edit->text().toStdString(), 100.0));
            model->max_k_slider = mk;
            if (onMaxKChanged) onMaxKChanged(mk);
            refreshEvaluation();
            if (onApplyDesign) onApplyDesign(last_analysis);
        });

        return page;
    }

    void updateRowCalcVisibility(const std::string& k) {
        bool is_lk = (*lock_map)[k];
        auto* btn = calc_toggle_btns[k];
        auto* dr  = calc_drawer_frames[k];
        if (is_lk) {
            btn->setText("🔒 Kept Value");
            btn->setEnabled(false);
            btn->setStyleSheet(QString("QPushButton { background-color: %1; color: #64748b; border: 1px solid %2; border-radius: 4px; padding: 3px 6px; font-size: 10px; }")
                               .arg(Theme::BG_INPUT, Theme::BORDER_INPUT));
            dr->setVisible(false);
        } else {
            btn->setEnabled(true);
            bool exp = calc_expanded[k];
            btn->setText(exp ? "▼ Hide Calc" : "▶ Show Calc");
            btn->setStyleSheet(QString("QPushButton { background-color: %1; color: %2; border: 1px solid %3; border-radius: 4px; padding: 3px 6px; font-weight: bold; font-size: 10px; }")
                               .arg(exp ? "#1e3a8a" : Theme::BG_INPUT,
                                    exp ? "#ffffff" : Theme::ACCENT_ZERO,
                                    Theme::BORDER_INPUT));
            dr->setVisible(exp);
        }
    }

public:
    void refreshFeedbackPage() {
        updating = true;
        fb_unity_chk->setChecked(model->unity_feedback_s);
        fb_custom_chk->setChecked(!model->unity_feedback_s);

        ctrl_p_chk->setChecked(active_ctrl_type == "P");
        ctrl_pi_chk->setChecked(active_ctrl_type == "PI");
        ctrl_pd_chk->setChecked(active_ctrl_type == "PD");
        ctrl_pid_chk->setChecked(active_ctrl_type == "PID");
        ctrl_none_chk->setChecked(active_ctrl_type == "None");

        bool show_kp = (active_ctrl_type != "None");
        bool show_ki = (active_ctrl_type == "PI" || active_ctrl_type == "PID");
        bool show_kd = (active_ctrl_type == "PD" || active_ctrl_type == "PID");
        fb_kp_lbl->setVisible(show_kp); fb_kp_edit->setVisible(show_kp);
        fb_ki_lbl->setVisible(show_ki); fb_ki_edit->setVisible(show_ki);
        fb_kd_lbl->setVisible(show_kd); fb_kd_edit->setVisible(show_kd);

        fb_kp_edit->setText(QString::number(model->kp, 'g', 4));
        fb_ki_edit->setText(QString::number(model->ki, 'g', 4));
        fb_kd_edit->setText(QString::number(model->kd, 'g', 4));

        h_poles_card->setValues(model->h_poles);
        h_zeros_card->setValues(model->h_zeros);
        num_h_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->h_zeros, "s")));
        den_h_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->h_poles, "s")));
        kh_edit->setText(QString::number(model->h_gain, 'g', 4));

        num_g_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->zeros, "s")));
        den_g_edit->setText(QString::fromStdString(formatPolynomialFromRoots(model->poles, "s")));
        kg_edit->setText(QString::number(model->plant_gain, 'g', 4));
        updating = false;

        if (model->unity_feedback_s) {
            h_status_lbl->setText("✔ Unity Feedback Active: H(s) = 1");
            h_math_w->setLatex("H(s) = 1");
        } else {
            std::string hp_s, hz_s;
            for (size_t i = 0; i < model->h_poles.size(); ++i) hp_s += (i ? ", " : "") + PRECISION.fmtComplex(model->h_poles[i]);
            for (size_t i = 0; i < model->h_zeros.size(); ++i) hz_s += (i ? ", " : "") + PRECISION.fmtComplex(model->h_zeros[i]);
            h_status_lbl->setText(QString("✔ H(s) Solved — Poles: [%1] • Zeros: [%2] (Kh = %3)")
                .arg(hp_s.empty() ? "None" : QString::fromStdString(hp_s))
                .arg(hz_s.empty() ? "None" : QString::fromStdString(hz_s))
                .arg(model->h_gain));
            h_math_w->setLatex(QString::fromStdString(formatTransferFunctionLatex(model->h_poles, model->h_zeros, model->h_gain, "s", "H")));
        }

        g_math_w->setLatex(QString::fromStdString(formatTransferFunctionLatex(model->poles, model->zeros, model->plant_gain, "s", "G")));

        double eff_kp = show_kp ? model->kp : 1.0;
        double eff_ki = show_ki ? model->ki : 0.0;
        double eff_kd = show_kd ? model->kd : 0.0;

        if (active_ctrl_type == "None") {
            c_math_w->setLatex("C(s) = 1 \\quad (\\mathrm{Uncompensated})");
        } else if (active_ctrl_type == "P") {
            c_math_w->setLatex(QString("C(s) = K_p = %1").arg(eff_kp));
        } else if (active_ctrl_type == "PD") {
            c_math_w->setLatex(QString("C(s) = K_p + K_d s = %1 + %2 s").arg(eff_kp).arg(eff_kd));
        } else if (active_ctrl_type == "PI") {
            c_math_w->setLatex(QString("C(s) = K_p + \\frac{K_i}{s} = \\frac{%1 s + %2}{s}").arg(eff_kp).arg(eff_ki));
        } else {
            c_math_w->setLatex(QString("C(s) = K_p + \\frac{K_i}{s} + K_d s = \\frac{%1 s^2 + %2 s + %3}{s}").arg(eff_kd).arg(eff_kp).arg(eff_ki));
        }

        auto [num_g, den_g] = ComplexTransferFunctionSimulator::buildPlantPolynomials(model->poles, model->zeros, model->plant_gain);
        std::vector<Complex> num_h = {Complex(1.0, 0.0)}, den_h = {Complex(1.0, 0.0)};
        if (!model->unity_feedback_s) {
            std::tie(num_h, den_h) = ComplexTransferFunctionSimulator::buildPlantPolynomials(model->h_poles, model->h_zeros, model->h_gain);
        }

        std::vector<Complex> num_c, den_c;
        if (active_ctrl_type == "None") {
            num_c = {Complex(1.0, 0.0)}; den_c = {Complex(1.0, 0.0)};
        } else if (active_ctrl_type == "P") {
            num_c = {Complex(eff_kp, 0.0)}; den_c = {Complex(1.0, 0.0)};
        } else if (active_ctrl_type == "PD") {
            num_c = {Complex(eff_kd, 0.0), Complex(eff_kp, 0.0)}; den_c = {Complex(1.0, 0.0)};
        } else if (active_ctrl_type == "PI") {
            num_c = {Complex(eff_kp, 0.0), Complex(eff_ki, 0.0)}; den_c = {Complex(1.0, 0.0), Complex(0.0, 0.0)};
        } else {
            num_c = {Complex(eff_kd, 0.0), Complex(eff_kp, 0.0), Complex(eff_ki, 0.0)}; den_c = {Complex(1.0, 0.0), Complex(0.0, 0.0)};
        }

        auto num_cg = polyMul(num_c, num_g);
        auto den_cg = polyMul(den_c, den_g);
        auto num_t  = polyMul(num_cg, den_h);
        auto den_t  = polyAdd(polyMul(den_cg, den_h), polyMul(num_cg, num_h));
        auto num_l  = polyMul(num_cg, num_h);
        auto den_l  = polyMul(den_cg, den_h);

        std::string num_t_str = formatComplexPolyDesc(num_t, "s");
        std::string den_t_str = formatComplexPolyDesc(den_t, "s");
        std::string num_l_str = formatComplexPolyDesc(num_l, "s");
        std::string den_l_str = formatComplexPolyDesc(den_l, "s");

        t_sym_math_w->setLatex(QString::fromStdString(
            "T(s) = \\frac{C(s)G(s)}{1 + C(s)G(s)H(s)} = \\frac{" + num_t_str + "}{" + den_t_str + "}"));
        num_t_sym_edit->setText(QString::fromStdString(num_t_str));
        den_t_sym_edit->setText(QString::fromStdString(den_t_str));

        t_eval_math_w->setLatex(QString::fromStdString(
            "T(s)\\big|_{\\mathrm{eval}} = \\frac{" + num_t_str + "}{" + den_t_str + "}"));
        num_t_eval_edit->setText(QString::fromStdString(num_t_str));
        den_t_eval_edit->setText(QString::fromStdString(den_t_str));

        l_open_math_w->setLatex(QString::fromStdString(
            "L(s) = C(s)G(s)H(s) = \\frac{" + num_l_str + "}{" + den_l_str + "}"));

        auto cl_poles = solvePolynomialRoots(den_t);
        auto cl_zeros = solvePolynomialRoots(num_t);
        std::string clp_s, clz_s;
        for (size_t i = 0; i < cl_poles.size(); ++i) clp_s += (i ? ", " : "") + PRECISION.fmtComplex(cl_poles[i]);
        for (size_t i = 0; i < cl_zeros.size(); ++i) clz_s += (i ? ", " : "") + PRECISION.fmtComplex(cl_zeros[i]);

        double dc_gain = (std::abs(den_t.back()) > 1e-12) ? (num_t.back().real() / den_t.back().real()) : std::numeric_limits<double>::infinity();
        double sse_step = std::isfinite(dc_gain) ? (model->input_magnitude * (1.0 - dc_gain)) : 0.0;

        double dom_z = 0.0, dom_wn = 0.0;
        if (!cl_poles.empty()) {
            Complex dp = cl_poles[0];
            for (const auto& p : cl_poles) {
                if (std::abs(p.imag()) > 1e-4 && (std::abs(dp.imag()) <= 1e-4 || p.real() > dp.real())) dp = p;
            }
            dom_wn = std::abs(dp);
            dom_z  = (dom_wn > 1e-9) ? (-dp.real() / dom_wn) : 0.0;
        }

        cl_telemetry_lbl->setText(QString(
            "• Closed-Loop Poles of T(s) : [%1]\n"
            "• Closed-Loop Zeros of T(s) : [%2]\n"
            "• Closed-Loop DC Gain T(0)  : %3   |   Step S.S.E. (M=%4) : %5\n"
            "• Dominant Damping & ωn     : ζ = %6   |   ωn = %7 rad/s"
        ).arg(clp_s.empty() ? "None" : QString::fromStdString(clp_s))
         .arg(clz_s.empty() ? "None" : QString::fromStdString(clz_s))
         .arg(QString::fromStdString(PRECISION.fmt(dc_gain)))
         .arg(QString::fromStdString(PRECISION.fmt(model->input_magnitude)))
         .arg(QString::fromStdString(PRECISION.fmt(sse_step)))
         .arg(QString::fromStdString(PRECISION.fmt(dom_z)))
         .arg(QString::fromStdString(PRECISION.fmt(dom_wn))));
    }

    void applyQuickPattern(const std::string& pat) {
        for (auto& [k, v] : *lock_map) v = false;
        (*input_map)["kp"] = model->kp;
        (*input_map)["kd"] = model->kd;
        (*input_map)["ki"] = model->ki;

        if (pat == "p_os") {
            (*input_map)["kd"] = 0.0; (*input_map)["ki"] = 0.0;
            (*lock_map)["kd"] = true; (*lock_map)["ki"] = true; (*lock_map)["os_pct"] = true;
        } else if (pat == "pd_kd") {
            (*input_map)["ki"] = 0.0;
            (*lock_map)["kp"] = true; (*lock_map)["ki"] = true; (*lock_map)["os_pct"] = true;
        } else if (pat == "pd_kp") {
            (*input_map)["ki"] = 0.0;
            (*lock_map)["kd"] = true; (*lock_map)["ki"] = true; (*lock_map)["os_pct"] = true;
        } else if (pat == "pd_both") {
            (*input_map)["ki"] = 0.0;
            (*lock_map)["ki"] = true; (*lock_map)["os_pct"] = true; (*lock_map)["ts"] = true;
        } else if (pat == "pi_ki") {
            (*input_map)["kd"] = 0.0;
            (*lock_map)["kp"] = true; (*lock_map)["kd"] = true; (*lock_map)["os_pct"] = true;
        } else if (pat == "pid_all") {
            (*lock_map)["os_pct"] = true; (*lock_map)["ts"] = true;
        }
        updating = true;
        for (auto& [k, ed] : val_edits) ed->setText(QString::number((*input_map)[k], 'g', 5));
        updating = false;
        refreshEvaluation();
    }

    void refreshEvaluation() {
        last_analysis = ParameterMatrixEngine::evaluate(*lock_map, *input_map, model);
        for (auto& [k, btn] : lock_btns) {
            bool lk = (*lock_map)[k];
            btn->setText(lk ? "🔒 KEEP" : "🔄 CHANGE");
            btn->setStyleSheet(QString(
                "QPushButton { background-color: %1; color: %2; border: 1px solid %3; border-radius: 4px; padding: 4px 8px; font-family: Monospace; font-weight: bold; font-size: 10px; }"
            ).arg(lk ? Theme::ACCENT_PID : Theme::BG_KEY_ACCENT,
                  lk ? "#0b0e14" : "#ffffff",
                  Theme::BORDER_INPUT));
            val_edits[k]->setEnabled(lk);
            val_edits[k]->setStyleSheet(QString(
                "QLineEdit { background-color: %1; color: %2; border: 1px solid %3; border-radius: 4px; padding: 3px 6px; font-family: Monospace; font-weight: bold; }"
            ).arg(lk ? Theme::BG_INPUT : Theme::BG_PANEL,
                  lk ? Theme::ACCENT_PID : "#64748b",
                  lk ? Theme::ACCENT_PID : Theme::BORDER_INPUT));
        }
        for (auto& [k, sl] : solved_lbls) {
            double v = last_analysis.solved[k];
            sl->setText(QString::fromStdString(PRECISION.fmt(v)));
            sl->setStyleSheet(QString("color: %1; font-family: Monospace; font-weight: bold; font-size: 11px; border:none;")
                              .arg((*lock_map)[k] ? Theme::ACCENT_PID : Theme::ACCENT_ZERO));
        }
        for (auto& [k, nl] : note_lbls) {
            nl->setText(QString::fromStdString(last_analysis.notes[k]));
        }
        for (auto& [k, mw] : calc_drawer_maths) {
            mw->setLatex(QString::fromStdString(last_analysis.step_by_step_latex[k]));
            updateRowCalcVisibility(k);
        }

        QString bg = last_analysis.feasible ? Theme::BANNER_OK_BG : Theme::BANNER_WARN_BG;
        QString bd = last_analysis.feasible ? Theme::BANNER_OK_BD : Theme::BANNER_WARN_BD;
        QString fg = last_analysis.feasible ? Theme::BANNER_OK_FG : Theme::BANNER_WARN_FG;

        verdict_banner->setStyleSheet(QString("QFrame { background-color: %1; border: 1.5px solid %2; border-radius: 6px; }").arg(bg, bd));
        verdict_title->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 12px; border:none;").arg(fg));
        verdict_desc->setStyleSheet(QString("color: %1; font-size: 10px; border:none;").arg(last_analysis.feasible ? "#a7f3d0" : "#fde68a"));

        verdict_title->setText(QString::fromStdString(last_analysis.headline));
        verdict_desc->setText(QString::fromStdString(last_analysis.explanation));
        formula_math_w->setLatex(QString::fromStdString(last_analysis.formulas_latex));
    }
};