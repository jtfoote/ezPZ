#pragma once
#include "BlockDiagramCanvas.h"


class BlockDiagramStudioTab : public QWidget {
    Q_OBJECT
public:
    SharedSystemModel* model;
    std::function<void(const std::vector<Complex>&, const std::vector<Complex>&, double)> onManualPush;

    std::vector<DiagramNode> nodes;
    std::vector<DiagramEdge> edges;
    std::optional<DiagramSolution> last_sol;
    int id_counter = 20;

    QLineEdit *lbl_edit, *num_edit, *den_edit;
    MathFormulaWidget *geq_math_w, *ts_math_w;
    QLabel *geq_pz_lbl, *ts_pz_lbl, *status_lbl;
    BlockDiagramCanvas* canvas;

    QPushButton *push_g_btn = nullptr;
    QPushButton *push_t_btn = nullptr;

    BlockDiagramStudioTab(SharedSystemModel* m,
                          std::function<void(const std::vector<Complex>&, const std::vector<Complex>&, double)> push_cb,
                          QWidget* parent = nullptr)
        : QWidget(parent), model(m), onManualPush(std::move(push_cb))
    {
        auto* root_l = new QHBoxLayout(this);
        root_l->setContentsMargins(0, 0, 0, 0);
        auto* splitter = new QSplitter(Qt::Horizontal, this);
        root_l->addWidget(splitter);

        auto* left_w = new QWidget();
        auto* left_l = new QVBoxLayout(left_w);
        left_l->setContentsMargins(0, 0, 6, 0);
        left_l->setSpacing(6);

        // 1. Palette & Templates Card
        auto* tools_card = makeCard();
        auto* tl = new QVBoxLayout(tools_card);
        auto* t_hdr = new QLabel("Block Diagram Builder & Topology Templates");
        t_hdr->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::ACCENT_ZERO));
        tl->addWidget(t_hdr);

        auto* add_row = new QHBoxLayout();
        auto* b_blk = makeActionBtn("+ Transfer Block G(s)", Theme::ACCENT_ZERO);
        auto* b_sum = makeActionBtn("+ Summing Junction Σ", Theme::ACCENT_PID);
        auto* b_br  = makeActionBtn("+ Branch Point •", Theme::ACCENT_LOCUS);
        add_row->addWidget(b_blk); add_row->addWidget(b_sum); add_row->addWidget(b_br);
        tl->addLayout(add_row);

        auto* tmpl_row = new QHBoxLayout();
        for (const auto& [lbl, key] : std::vector<std::pair<QString, std::string>>{
            {"Unity Feedback", "unity"}, {"G1·G2 + H1 Feedback", "cascade"}, {"Inner + Outer Loop", "nested"}, {"Parallel Feedforward", "parallel"}
        }) {
            auto* b = makeActionBtn(lbl, Theme::TEXT_PRIMARY);
            connect(b, &QPushButton::clicked, this, [this, key]() { loadTemplate(key); });
            tmpl_row->addWidget(b);
        }
        tl->addLayout(tmpl_row);
        left_l->addWidget(tools_card);

        // 2. Selected Block / Wire Inspector Card
        auto* insp_card = makeCard();
        auto* il = new QVBoxLayout(insp_card);
        auto* ihdr = new QHBoxLayout();
        auto* it = new QLabel("Selected Element Inspector");
        it->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::ACCENT_PID));
        auto* del_btn = makeActionBtn("Delete Selected ⌫", Theme::ACCENT_POLE);
        ihdr->addWidget(it); ihdr->addStretch(); ihdr->addWidget(del_btn);
        il->addLayout(ihdr);

        auto* igrid = new QGridLayout();
        lbl_edit = makeDarkEntry("G1(s)");
        num_edit = makeDarkEntry("2");
        den_edit = makeDarkEntry("s + 1");
        auto* l1 = new QLabel("Block Name:"); l1->setStyleSheet("color: #94a3b8; font-weight: bold; border:none;");
        auto* l2 = new QLabel("Num(s):"); l2->setStyleSheet("color: #38bdf8; font-family: Monospace; font-weight: bold; border:none;");
        auto* l3 = new QLabel("Den(s):"); l3->setStyleSheet("color: #ff5f56; font-family: Monospace; font-weight: bold; border:none;");
        igrid->addWidget(l1, 0, 0); igrid->addWidget(lbl_edit, 0, 1);
        igrid->addWidget(l2, 1, 0); igrid->addWidget(num_edit, 1, 1);
        igrid->addWidget(l3, 2, 0); igrid->addWidget(den_edit, 2, 1);
        il->addLayout(igrid);

        auto* ibar = new QHBoxLayout();
        auto* apply_b = makeActionBtn("Apply Block Num(s)/Den(s) ↵", Theme::ACCENT_PID);
        auto* flip_b  = makeActionBtn("Toggle Selected Wire Sign (±)", Theme::ACCENT_OPEN);
        ibar->addWidget(apply_b); ibar->addWidget(flip_b);
        il->addLayout(ibar);
        left_l->addWidget(insp_card);

        // 3. Reduced Equivalent G_eq(s) & T(s) Card + Manual Push
        auto* res_card = makeCard();
        auto* rl = new QVBoxLayout(res_card);
        auto* rt = new QLabel("Equivalent Reduced Transfer Functions  G_eq(s) & T(s)");
        rt->setStyleSheet(QString("color: %1; font-weight: bold; border:none;").arg(Theme::TEXT_PRIMARY));
        rl->addWidget(rt);

        geq_math_w = new MathFormulaWidget("", QColor(Theme::ACCENT_ZERO), QColor(Theme::BG_INPUT), 10);
        geq_pz_lbl = new QLabel("Poles: — | Zeros: —");
        geq_pz_lbl->setStyleSheet("color: #94a3b8; font-family: Monospace; font-size: 10px; border:none;");
        rl->addWidget(geq_math_w);
        rl->addWidget(geq_pz_lbl);

        ts_math_w = new MathFormulaWidget("", QColor(Theme::ACCENT_LOCUS), QColor(Theme::BG_INPUT), 10);
        ts_pz_lbl = new QLabel("Poles: — | Zeros: —");
        ts_pz_lbl->setStyleSheet("color: #94a3b8; font-family: Monospace; font-size: 10px; border:none;");
        rl->addWidget(ts_math_w);
        rl->addWidget(ts_pz_lbl);

        push_g_btn = makeActionBtn("Manual Push: Export G_eq(s) → Suite Plant (Tabs 1–6) ➔", "#0b0e14", Theme::ACCENT_ZERO);
        push_t_btn = makeActionBtn("Manual Push: Export Closed-Loop T(s) → Suite Plant (Tabs 1–6) ➔", Theme::TEXT_PRIMARY, Theme::BG_PANEL_ALT);
        rl->addWidget(push_g_btn);
        rl->addWidget(push_t_btn);

        status_lbl = new QLabel("Isolated Workspace: Changes here do NOT overwrite Tabs 1–6 until you click Manual Push.");
        status_lbl->setWordWrap(true);
        status_lbl->setStyleSheet(QString("color: %1; font-size: 10px; border:none;").arg(Theme::ACCENT_PID));
        rl->addWidget(status_lbl);
        left_l->addWidget(res_card);
        left_l->addStretch();

        // Right Canvas
        auto* right_card = makeCard();
        auto* right_l = new QVBoxLayout(right_card);
        auto* chdr = new QHBoxLayout();
        auto* ct = new QLabel("Interactive Block Diagram Canvas (L-Drag: Move • Shift+Click or R-Click Node A → Node B: Wire)");
        ct->setStyleSheet(QString("color: %1; font-weight: bold; font-size: 12px; border:none;").arg(Theme::TEXT_PRIMARY));
        chdr->addWidget(ct);
        right_l->addLayout(chdr);

        canvas = new BlockDiagramCanvas();
        canvas->nodes = &nodes;
        canvas->edges = &edges;
        right_l->addWidget(canvas, 1);

        splitter->addWidget(left_w);
        splitter->addWidget(right_card);
        splitter->setStretchFactor(0, 0);
        splitter->setStretchFactor(1, 1);
        left_w->setMinimumWidth(465);

        // Connections
        connect(b_blk, &QPushButton::clicked, this, [this]() {
            ++id_counter;
            std::string id = "G" + std::to_string(id_counter);
            nodes.push_back({id, "BLOCK", id + "(s)", 320, 120, "1", "s + 2"});
            selectNode(id);
            solveAndRefresh();
        });
        connect(b_sum, &QPushButton::clicked, this, [this]() {
            ++id_counter;
            std::string id = "S" + std::to_string(id_counter);
            nodes.push_back({id, "SUM", "Σ", 240, 120});
            selectNode(id);
            solveAndRefresh();
        });
        connect(b_br, &QPushButton::clicked, this, [this]() {
            ++id_counter;
            std::string id = "B" + std::to_string(id_counter);
            nodes.push_back({id, "BRANCH", "•", 420, 120});
            selectNode(id);
            solveAndRefresh();
        });

        auto applyEdits = [this]() {
            for (auto& n : nodes) {
                if (n.id == canvas->selected_node_id) {
                    n.label = lbl_edit->text().toStdString();
                    if (n.kind == "BLOCK") {
                        n.num_str = num_edit->text().toStdString();
                        n.den_str = den_edit->text().toStdString();
                    }
                    break;
                }
            }
            solveAndRefresh();
        };
        connect(apply_b, &QPushButton::clicked, this, applyEdits);
        connect(num_edit, &QLineEdit::returnPressed, this, applyEdits);
        connect(den_edit, &QLineEdit::returnPressed, this, applyEdits);

        connect(flip_b, &QPushButton::clicked, this, [this]() {
            for (auto& e : edges) {
                if (e.id == canvas->selected_edge_id) { e.sign = -e.sign; break; }
            }
            solveAndRefresh();
        });

        connect(del_btn, &QPushButton::clicked, this, [this]() {
            if (!canvas->selected_edge_id.empty()) {
                edges.erase(std::remove_if(edges.begin(), edges.end(), [&](const DiagramEdge& e) { return e.id == canvas->selected_edge_id; }), edges.end());
                canvas->selected_edge_id.clear();
                solveAndRefresh();
            } else if (!canvas->selected_node_id.empty()) {
                std::string nid = canvas->selected_node_id;
                if (nid == "R" || nid == "Y") return;
                nodes.erase(std::remove_if(nodes.begin(), nodes.end(), [&](const DiagramNode& n) { return n.id == nid; }), nodes.end());
                edges.erase(std::remove_if(edges.begin(), edges.end(), [&](const DiagramEdge& e) { return e.src_id == nid || e.dst_id == nid; }), edges.end());
                canvas->selected_node_id.clear();
                solveAndRefresh();
            }
        });

        connect(push_g_btn, &QPushButton::clicked, this, [this]() {
            if (last_sol.has_value() && onManualPush) {
                onManualPush(last_sol->poles_G, last_sol->zeros_G, last_sol->gain_G);
                status_lbl->setText("✔ Exported Equivalent Open-Loop G_eq(s) to Tabs 1–6!");
            }
        });
        connect(push_t_btn, &QPushButton::clicked, this, [this]() {
            if (last_sol.has_value() && onManualPush) {
                onManualPush(last_sol->poles_T, last_sol->zeros_T, last_sol->gain_T);
                status_lbl->setText("✔ Exported Closed-Loop T(s) to Tabs 1–6!");
            }
        });

        canvas->onNodeSelected = [this](const std::string& nid) { selectNode(nid); };
        canvas->onTopologyModified = [this]() { solveAndRefresh(); };

        loadTemplate("cascade");
    }

    void selectNode(const std::string& nid) {
        canvas->selected_node_id = nid;
        for (const auto& n : nodes) {
            if (n.id == nid) {
                lbl_edit->setText(QString::fromStdString(n.label));
                num_edit->setText(QString::fromStdString(n.num_str));
                den_edit->setText(QString::fromStdString(n.den_str));
                break;
            }
        }
        canvas->update();
    }

    void loadTemplate(const std::string& key) {
        if (key == "unity") {
            nodes = {
                {"R", "INPUT", "R(s)", 70, 200}, {"S1", "SUM", "Σ1", 175, 200},
                {"G1", "BLOCK", "G1(s)", 335, 200, "s + 8", "s^2 + 2s + 2"},
                {"B1", "BRANCH", "•", 495, 200}, {"Y", "OUTPUT", "Y(s)", 610, 200}
            };
            edges = {
                {"E1", "R", "S1", +1}, {"E2", "S1", "G1", +1}, {"E3", "G1", "B1", +1},
                {"E4", "B1", "Y", +1}, {"E5", "B1", "S1", -1}
            };
        } else if (key == "nested") {
            nodes = {
                {"R", "INPUT", "R(s)", 55, 190}, {"S1", "SUM", "Σ1", 145, 190},
                {"G1", "BLOCK", "Gc(s)", 260, 190, "2", "1"}, {"S2", "SUM", "Σ2", 375, 190},
                {"G2", "BLOCK", "Gp(s)", 495, 190, "4", "s(s + 3)"}, {"B1", "BRANCH", "•", 615, 190},
                {"H1", "BLOCK", "H_in(s)", 495, 320, "0.5s", "1"}, {"Y", "OUTPUT", "Y(s)", 715, 190}
            };
            edges = {
                {"E1", "R", "S1", +1}, {"E2", "S1", "G1", +1}, {"E3", "G1", "S2", +1},
                {"E4", "S2", "G2", +1}, {"E5", "G2", "B1", +1}, {"E6", "B1", "Y", +1},
                {"E7", "B1", "H1", +1}, {"E8", "H1", "S2", -1}, {"E9", "B1", "S1", -1}
            };
        } else if (key == "parallel") {
            nodes = {
                {"R", "INPUT", "R(s)", 65, 200}, {"B_in", "BRANCH", "•", 150, 200},
                {"G1", "BLOCK", "G_ff(s)", 340, 110, "0.5", "s + 4"},
                {"G2", "BLOCK", "G_main(s)", 340, 250, "4", "s^2 + 2s + 4"},
                {"S1", "SUM", "Σ1", 525, 200}, {"Y", "OUTPUT", "Y(s)", 650, 200}
            };
            edges = {
                {"E1", "R", "B_in", +1}, {"E2", "B_in", "G1", +1}, {"E3", "B_in", "G2", +1},
                {"E4", "G1", "S1", +1}, {"E5", "G2", "S1", +1}, {"E6", "S1", "Y", +1}
            };
        } else {
            nodes = {
                {"R", "INPUT", "R(s)", 60, 185}, {"S1", "SUM", "Σ1", 160, 185},
                {"G1", "BLOCK", "G1(s)", 290, 185, "2", "s + 1"},
                {"G2", "BLOCK", "G2(s)", 455, 185, "s + 4", "s + 3"},
                {"B1", "BRANCH", "•", 590, 185},
                {"H1", "BLOCK", "H1(s)", 375, 320, "1", "s + 5"},
                {"Y", "OUTPUT", "Y(s)", 695, 185}
            };
            edges = {
                {"E1", "R", "S1", +1}, {"E2", "S1", "G1", +1}, {"E3", "G1", "G2", +1},
                {"E4", "G2", "B1", +1}, {"E5", "B1", "Y", +1}, {"E6", "B1", "H1", +1}, {"E7", "H1", "S1", -1}
            };
        }
        selectNode("G1");
        solveAndRefresh();
    }

    void solveAndRefresh() {
        try {
            auto sol = BlockDiagramAlgebraEngine::solveDiagram(nodes, edges);
            last_sol = sol;
            geq_math_w->setLatex(QString::fromStdString(sol.latex_G));
            ts_math_w->setLatex(QString::fromStdString(sol.latex_T));

            auto fmtVec = [](const std::vector<Complex>& v) {
                std::string s;
                for (size_t i = 0; i < v.size(); ++i) s += (i ? ", " : "") + PRECISION.fmtComplex(v[i]);
                return s.empty() ? "None" : s;
            };
            geq_pz_lbl->setText(QString("K_eq = %1  •  Poles: [%2]  •  Zeros: [%3]")
                .arg(QString::fromStdString(PRECISION.fmt(sol.gain_G)))
                .arg(QString::fromStdString(fmtVec(sol.poles_G)))
                .arg(QString::fromStdString(fmtVec(sol.zeros_G))));
            ts_pz_lbl->setText(QString("K_cl = %1  •  Poles: [%2]  •  Zeros: [%3]")
                .arg(QString::fromStdString(PRECISION.fmt(sol.gain_T)))
                .arg(QString::fromStdString(fmtVec(sol.poles_T)))
                .arg(QString::fromStdString(fmtVec(sol.zeros_T))));

            bool has_sym_k = BlockDiagramAlgebraEngine::containsSymbolK(*canvas->nodes);

            if (push_g_btn && push_t_btn) {
                push_g_btn->setEnabled(!has_sym_k);
                push_t_btn->setEnabled(!has_sym_k);

                if (has_sym_k) {
                    QString disabled_style = QString(
                        "QPushButton { background-color: %1; color: #64748b; border: 1px solid %2; border-radius: 4px; padding: 5px 10px; font-weight: bold; font-size: 11px; }"
                    ).arg(Theme::BG_INPUT, Theme::BORDER_INPUT);
                    push_g_btn->setStyleSheet(disabled_style);
                    push_t_btn->setStyleSheet(disabled_style);
                    push_g_btn->setText("🔒 Export Disabled (Contains Symbolic 'K')");
                    push_t_btn->setText("🔒 Export Disabled (Contains Symbolic 'K')");
                } else {
                    push_g_btn->setStyleSheet(QString(
                        "QPushButton { background-color: %1; color: #0b0e14; border: 1px solid %2; border-radius: 4px; padding: 5px 10px; font-weight: bold; font-size: 11px; }"
                    ).arg(Theme::ACCENT_ZERO, Theme::ACCENT_ZERO));
                    push_t_btn->setStyleSheet(QString(
                        "QPushButton { background-color: %1; color: %2; border: 1px solid %3; border-radius: 4px; padding: 5px 10px; font-weight: bold; font-size: 11px; }"
                    ).arg(Theme::BG_PANEL_ALT, Theme::TEXT_PRIMARY, Theme::BORDER_INPUT));
                    push_g_btn->setText("Manual Push: Export G_eq(s) → Suite Plant (Tabs 1–6) ➔");
                    push_t_btn->setText("Manual Push: Export Closed-Loop T(s) → Suite Plant (Tabs 1–6) ➔");
                }
            }
        } catch (const std::exception& e) {
            last_sol = std::nullopt;
            geq_pz_lbl->setText(QString("⚠ Reduction Error: %1").arg(e.what()));
        }
        canvas->update();
    }
};

