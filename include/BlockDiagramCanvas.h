#pragma once
#include "FormulaCheatSheetTab.h"


// =============================================================================
// Engine 6 & Tab 7: Interactive Block Diagram Studio (Mason's Gain / Linear Algebra Reducer)
// =============================================================================
struct DiagramNode {
    std::string id;
    std::string kind; // "INPUT", "SUM", "BLOCK", "BRANCH", "OUTPUT"
    std::string label;
    double x = 100.0, y = 200.0;
    std::string num_str = "1";
    std::string den_str = "1";
};

struct DiagramEdge {
    std::string id;
    std::string src_id;
    std::string dst_id;
    int sign = 1; // +1 or -1
};

struct DiagramSolution {
    std::vector<Complex> poles_T, zeros_T;
    double gain_T = 1.0;
    std::string latex_T;
    std::vector<Complex> poles_G, zeros_G;
    double gain_G = 1.0;
    std::string latex_G;
};

class BlockDiagramAlgebraEngine {
public:
    static std::tuple<std::vector<Complex>, std::vector<Complex>, double> reduceRational(const Poly& num_in, const Poly& den_in, double cancel_tol = 1e-3) {
        Poly nt = polyTrim(num_in, 1e-11);
        Poly dt = polyTrim(den_in, 1e-11);
        if (dt.size() == 1 && std::abs(dt[0]) < 1e-12) throw std::runtime_error("Algebraic loop / singular denominator (1 - LoopGain = 0)");
        if (nt.size() == 1 && std::abs(nt[0]) < 1e-12) return {{}, {}, 0.0};

        double gain = nt[0].real() / dt[0].real();
        auto z_raw = solvePolynomialRoots(nt);
        auto p_raw = solvePolynomialRoots(dt);

        // Cancel matching pole-zero pairs
        std::vector<bool> p_used(p_raw.size(), false);
        std::vector<Complex> z_keep;
        for (const auto& z : z_raw) {
            int best_j = -1;
            double best_d = cancel_tol;
            for (size_t j = 0; j < p_raw.size(); ++j) {
                if (!p_used[j]) {
                    double d = std::abs(z - p_raw[j]);
                    if (d < best_d) { best_d = d; best_j = static_cast<int>(j); }
                }
            }
            if (best_j >= 0) p_used[best_j] = true;
            else z_keep.push_back(z);
        }
        std::vector<Complex> p_keep;
        for (size_t j = 0; j < p_raw.size(); ++j) if (!p_used[j]) p_keep.push_back(p_raw[j]);
        return {p_keep, z_keep, gain};
    }

    // Substitute a numeric value for K/k inside a polynomial string (e.g. "2*K*(s+1)" -> "2*(1.0)*(s+1)")
    static std::string substituteK(const std::string& expr_raw, double kv) {
        std::string sub;
        for (size_t i = 0; i < expr_raw.size(); ++i) {
            char c = expr_raw[i];
            if (c == 'K' || c == 'k') {
                bool prev = (i > 0) && (std::isdigit(static_cast<unsigned char>(expr_raw[i - 1])) || expr_raw[i - 1] == ')' || expr_raw[i - 1] == '.');
                if (prev) sub += "*";
                sub += "(" + std::to_string(kv) + ")";
                bool next = (i + 1 < expr_raw.size()) && (std::isalnum(static_cast<unsigned char>(expr_raw[i + 1])) || expr_raw[i + 1] == '(');
                if (next) sub += "*";
            } else {
                sub.push_back(c);
            }
        }
        return sub;
    }

    static bool containsSymbolK(const std::vector<DiagramNode>& nodes) {
        for (const auto& nd : nodes) {
            if (nd.kind == "BLOCK") {
                if (nd.num_str.find('K') != std::string::npos || nd.num_str.find('k') != std::string::npos ||
                    nd.den_str.find('K') != std::string::npos || nd.den_str.find('k') != std::string::npos) {
                    return true;
                }
            }
        }
        return false;
    }

    // Format a 2D polynomial P(s, K) = c0(s) + c1(s)*K + c2(s)*K^2 into LaTeX
    static std::string formatPoly2DSK(const Poly& p0, const Poly& p1, const Poly& p2) {
        size_t n = std::max({p0.size(), p1.size(), p2.size()});
        if (n == 0) return "0";
        auto pad = [&](const Poly& p) {
            Poly out(n, Complex(0.0, 0.0));
            for (size_t i = 0; i < p.size(); ++i) out[n - p.size() + i] = p[i];
            return out;
        };
        Poly y0 = pad(p0), y1 = pad(p1), y2 = pad(p2);

        int deg = static_cast<int>(n) - 1;
        std::string out;
        for (int i = 0; i <= deg; ++i) {
            int pwr = deg - i;
            // Reconstruct quadratic coefficients in K from values at K = 0, 1, 2
            Complex a2 = 0.5 * (y2[i] - 2.0 * y1[i] + y0[i]);
            Complex a1 = y1[i] - y0[i] - a2;
            Complex a0 = y0[i];

            SymRat sr(Poly{a2, a1, a0});
            if (sr.isZero(1e-8)) continue;

            std::string k_str = sr.toLatex("K");
            bool has_add_sub = (k_str.find('+') != std::string::npos || (k_str.find('-', 1) != std::string::npos));
            if (pwr > 0 && has_add_sub) k_str = "(" + k_str + ")";
            if (pwr > 0 && k_str == "1") k_str = "";
            else if (pwr > 0 && k_str == "-1") k_str = "-";

            std::string term = k_str;
            if (pwr > 0) {
                term += "s";
                if (pwr > 1) term += "^{" + std::to_string(pwr) + "}";
            }
            if (out.empty()) {
                out = term;
            } else if (!term.empty() && term[0] == '-' && term.find('(') != 1) {
                out += " - " + term.substr(1);
            } else {
                out += " + " + term;
            }
        }
        return out.empty() ? "0" : out;
    }

    // Run fraction-free polynomial Gaussian elimination at a specific numeric value of K
    static std::pair<Poly, Poly> eliminateAtK(
        const std::vector<DiagramNode>& nodes,
        const std::vector<DiagramEdge>& edges,
        int r_idx,
        int y_idx,
        double k_val)
    {
        size_t N = nodes.size();
        std::map<std::string, size_t> id_to_idx;
        for (size_t i = 0; i < N; ++i) id_to_idx[nodes[i].id] = i;

        std::vector<Poly> n_num(N, {Complex(1.0, 0.0)});
        std::vector<Poly> n_den(N, {Complex(1.0, 0.0)});
        for (size_t i = 0; i < N; ++i) {
            if (nodes[i].kind == "BLOCK") {
                n_num[i] = parsePolynomialInput(substituteK(nodes[i].num_str, k_val)).coeffs;
                n_den[i] = parsePolynomialInput(substituteK(nodes[i].den_str, k_val)).coeffs;
            }
        }

        std::vector<std::vector<Poly>> aug(N, std::vector<Poly>(N + 1, {Complex(0.0, 0.0)}));
        for (size_t i = 0; i < N; ++i) aug[i][i] = n_den[i];
        aug[r_idx][N] = {Complex(1.0, 0.0)};

        for (const auto& e : edges) {
            if (!id_to_idx.count(e.src_id) || !id_to_idx.count(e.dst_id)) continue;
            size_t u = id_to_idx[e.src_id];
            size_t v = id_to_idx[e.dst_id];
            if (v == static_cast<size_t>(r_idx)) continue;
            Poly term = polyScale(n_num[v], Complex(double(e.sign), 0.0));
            aug[v][u] = polySub(aug[v][u], term);
        }

        // Fraction-free Gaussian elimination to solve for node y_idx
        for (size_t k = 0; k < N; ++k) {
            size_t piv_row = k;
            while (piv_row < N && aug[piv_row][k].size() == 1 && std::abs(aug[piv_row][k][0]) < 1e-12) ++piv_row;
            if (piv_row == N) continue;
            if (piv_row != k) std::swap(aug[k], aug[piv_row]);

            Poly piv = aug[k][k];
            for (size_t i = 0; i < N; ++i) {
                if (i == k) continue;
                Poly fac = aug[i][k];
                if (fac.size() == 1 && std::abs(fac[0]) < 1e-12) continue;
                for (size_t j = k; j <= N; ++j) {
                    aug[i][j] = polyTrim(polySub(polyMul(aug[i][j], piv), polyMul(aug[k][j], fac)));
                }
            }
        }

        Poly raw_num_T = polyTrim(aug[y_idx][N], 1e-10);
        Poly raw_den_T = polyTrim(aug[y_idx][y_idx], 1e-10);

        if (!raw_den_T.empty() && std::abs(raw_den_T[0]) > 1e-12) {
            Poly g = polyGCD(raw_num_T, raw_den_T);
            if (g.size() > 1) {
                raw_num_T = polyDivMod(raw_num_T, g).first;
                raw_den_T = polyDivMod(raw_den_T, g).first;
            }
            Complex lead = raw_den_T[0];
            if (std::abs(lead) > 1e-12) {
                raw_num_T = polyScale(raw_num_T, Complex(1.0, 0.0) / lead);
                raw_den_T = polyScale(raw_den_T, Complex(1.0, 0.0) / lead);
            }
        }
        return {raw_num_T, raw_den_T};
    }

    static DiagramSolution solveDiagram(const std::vector<DiagramNode>& nodes, const std::vector<DiagramEdge>& edges) {
        DiagramSolution sol;
        size_t N = nodes.size();
        if (N == 0) throw std::runtime_error("Diagram is empty.");

        int r_idx = -1, y_idx = -1;
        for (size_t i = 0; i < N; ++i) {
            if (nodes[i].kind == "INPUT")  r_idx = static_cast<int>(i);
            if (nodes[i].kind == "OUTPUT") y_idx = static_cast<int>(i);
        }
        if (r_idx < 0 || y_idx < 0) {
            throw std::runtime_error("Diagram requires both an INPUT R(s) and OUTPUT Y(s) node.");
        }

        // 1. Solve numeric transfer functions at K = 1.0 (for pole/zero extraction & export)
        auto [raw_num_T, raw_den_T] = eliminateAtK(nodes, edges, r_idx, y_idx, 1.0);
        std::tie(sol.poles_T, sol.zeros_T, sol.gain_T) = reduceRational(raw_num_T, raw_den_T);
        sol.latex_T = formatTransferFunctionLatex(sol.poles_T, sol.zeros_T, sol.gain_T, "s", "T");

        // Open-loop equivalent G_eq(s) = T(s) / (1 - T(s)) = Num_T / (Den_T - Num_T)
        Poly raw_den_G = polySub(raw_den_T, raw_num_T);
        if (raw_den_G.size() == 1 && std::abs(raw_den_G[0]) < 1e-11) {
            sol.poles_G = sol.poles_T; sol.zeros_G = sol.zeros_T; sol.gain_G = sol.gain_T;
        } else {
            std::tie(sol.poles_G, sol.zeros_G, sol.gain_G) = reduceRational(raw_num_T, raw_den_G);
        }
        sol.latex_G = formatTransferFunctionLatex(sol.poles_G, sol.zeros_G, sol.gain_G, "s", "G_{\\mathrm{eq}}");

        // 2. If any block contains symbolic 'K', interpolate at K = 0, 1, 2 to build symbolic LaTeX
        if (containsSymbolK(nodes)) {
            auto [n0, d0] = eliminateAtK(nodes, edges, r_idx, y_idx, 0.0);
            auto [n1, d1] = std::pair<Poly, Poly>{raw_num_T, raw_den_T};
            auto [n2, d2] = eliminateAtK(nodes, edges, r_idx, y_idx, 2.0);

            std::string sym_num_T = formatPoly2DSK(n0, n1, n2);
            std::string sym_den_T = formatPoly2DSK(d0, d1, d2);
            sol.latex_T = "T(s, K) = \\frac{" + sym_num_T + "}{" + sym_den_T + "}";

            Poly dg0 = polySub(d0, n0);
            Poly dg1 = polySub(d1, n1);
            Poly dg2 = polySub(d2, n2);
            std::string sym_den_G = formatPoly2DSK(dg0, dg1, dg2);
            sol.latex_G = "G_{\\mathrm{eq}}(s, K) = \\frac{" + sym_num_T + "}{" + sym_den_G + "}";
        }

        return sol;
    }
};

class BlockDiagramCanvas : public QWidget {
    Q_OBJECT
public:
    std::vector<DiagramNode>* nodes = nullptr;
    std::vector<DiagramEdge>* edges = nullptr;
    std::string selected_node_id;
    std::string selected_edge_id;
    std::string wire_src_id;

    std::function<void(const std::string&)> onNodeSelected;
    std::function<void(const std::string&)> onEdgeSelected;
    std::function<void()> onTopologyModified;

    explicit BlockDiagramCanvas(QWidget* parent = nullptr) : QWidget(parent) {
        setMinimumSize(520, 380);
        setCursor(Qt::PointingHandCursor);
    }

protected:
    std::string drag_node_id;
    QPointF drag_offset;

    static QPointF edgeMidpoint(const DiagramNode& u, const DiagramNode& v) {
        if (u.x > v.x + 20 && std::abs(u.y - v.y) < 40) {
            return QPointF(0.5 * (u.x + v.x), std::max(u.y, v.y) + 85.0);
        }
        return QPointF(0.5 * (u.x + v.x), 0.5 * (u.y + v.y));
    }

    DiagramNode* hitNode(QPointF pt) {
        if (!nodes) return nullptr;
        for (auto it = nodes->rbegin(); it != nodes->rend(); ++it) {
            if (it->kind == "BLOCK" && std::abs(pt.x() - it->x) <= 58 && std::abs(pt.y() - it->y) <= 30) return &(*it);
            if ((it->kind == "INPUT" || it->kind == "OUTPUT") && std::abs(pt.x() - it->x) <= 36 && std::abs(pt.y() - it->y) <= 20) return &(*it);
            if (it->kind == "SUM" && std::hypot(pt.x() - it->x, pt.y() - it->y) <= 22) return &(*it);
            if (it->kind == "BRANCH" && std::hypot(pt.x() - it->x, pt.y() - it->y) <= 14) return &(*it);
        }
        return nullptr;
    }

    DiagramEdge* hitEdge(QPointF pt) {
        if (!nodes || !edges) return nullptr;
        std::map<std::string, DiagramNode> nmap;
        for (const auto& n : *nodes) nmap[n.id] = n;
        for (auto& e : *edges) {
            if (!nmap.count(e.src_id) || !nmap.count(e.dst_id)) continue;
            QPointF m = edgeMidpoint(nmap[e.src_id], nmap[e.dst_id]);
            if (std::hypot(pt.x() - m.x(), pt.y() - m.y()) <= 16) return &e;
        }
        return nullptr;
    }

    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.fillRect(rect(), QColor(Theme::BG_PLOT));

        p.setPen(QPen(QColor(Theme::GRID_MAJOR), 1.0));
        for (int gx = 0; gx < width(); gx += 40) p.drawLine(gx, 0, gx, height());
        for (int gy = 0; gy < height(); gy += 40) p.drawLine(0, gy, width(), gy);

        if (!nodes || !edges) return;
        std::map<std::string, DiagramNode> nmap;
        for (const auto& n : *nodes) nmap[n.id] = n;

        // Draw Directed Wires
        for (const auto& e : *edges) {
            if (!nmap.count(e.src_id) || !nmap.count(e.dst_id)) continue;
            const auto& u = nmap[e.src_id];
            const auto& v = nmap[e.dst_id];
            bool is_sel = (e.id == selected_edge_id);
            QColor col(is_sel ? Theme::ACCENT_OPEN : (e.sign < 0 ? Theme::ACCENT_POLE : Theme::ACCENT_ZERO));
            p.setPen(QPen(col, is_sel ? 2.6 : 1.8));
            p.setBrush(Qt::NoBrush);

            QPointF mid;
            if (u.x > v.x + 20 && std::abs(u.y - v.y) < 40) {
                double my = std::max(u.y, v.y) + 85.0;
                QPainterPath path;
                path.moveTo(u.x, u.y);
                path.lineTo(u.x, my);
                path.lineTo(v.x, my);
                path.lineTo(v.x, v.y);
                p.drawPath(path);
                mid = QPointF(0.5 * (u.x + v.x), my);
            } else {
                p.drawLine(QPointF(u.x, u.y), QPointF(v.x, v.y));
                mid = QPointF(0.5 * (u.x + v.x), 0.5 * (u.y + v.y));
            }

            p.setBrush(QColor(Theme::BG_PANEL));
            p.setPen(QPen(col, 1.4));
            p.drawEllipse(mid, 10, 10);
            p.setFont(QFont("Monospace", 10, QFont::Bold));
            p.drawText(QRectF(mid.x() - 10, mid.y() - 10, 20, 20), Qt::AlignCenter, e.sign >= 0 ? "+" : "−");
        }

        // Draw Nodes
        for (const auto& n : *nodes) {
            bool is_sel = (n.id == selected_node_id);
            bool is_w = (n.id == wire_src_id);
            QColor out_col(is_w ? Theme::ACCENT_OPEN : (is_sel ? "#ffffff" : Theme::ACCENT_ZERO));
            double lw = (is_sel || is_w) ? 2.5 : 1.6;

            if (n.kind == "BLOCK") {
                QRectF r(n.x - 56, n.y - 28, 112, 56);
                p.setBrush(QColor(Theme::BG_PANEL_ALT));
                p.setPen(QPen(out_col, lw));
                p.drawRoundedRect(r, 4, 4);

                p.setPen(QColor(Theme::ACCENT_PID));
                p.setFont(QFont("Sans", 8, QFont::Bold));
                p.drawText(QRectF(n.x - 54, n.y - 26, 108, 14), Qt::AlignCenter, QString::fromStdString(n.label));

                p.setPen(QColor(Theme::TEXT_PRIMARY));
                p.setFont(QFont("Monospace", 9, QFont::Bold));
                p.drawText(QRectF(n.x - 54, n.y - 12, 108, 14), Qt::AlignCenter, QString::fromStdString(n.num_str));

                p.setPen(QPen(QColor(Theme::TEXT_SECONDARY), 1.2));
                p.drawLine(QPointF(n.x - 44, n.y + 4), QPointF(n.x + 44, n.y + 4));

                p.setPen(QColor(Theme::ACCENT_POLE));
                p.drawText(QRectF(n.x - 54, n.y + 6, 108, 16), Qt::AlignCenter, QString::fromStdString(n.den_str));
            } else if (n.kind == "SUM") {
                p.setBrush(QColor(Theme::BG_PANEL_ALT));
                p.setPen(QPen(is_sel ? QColor("#ffffff") : QColor(Theme::ACCENT_PID), lw));
                p.drawEllipse(QPointF(n.x, n.y), 20, 20);
                p.setFont(QFont("Sans", 12, QFont::Bold));
                p.drawText(QRectF(n.x - 20, n.y - 20, 40, 40), Qt::AlignCenter, "Σ");
            } else if (n.kind == "BRANCH") {
                p.setBrush(QColor(Theme::ACCENT_LOCUS));
                p.setPen(QPen(is_sel ? QColor("#ffffff") : QColor(Theme::ACCENT_LOCUS), 1.5));
                p.drawEllipse(QPointF(n.x, n.y), 6, 6);
            } else {
                QColor c(n.kind == "INPUT" ? Theme::ACCENT_PID : Theme::ACCENT_LOCUS);
                QRectF r(n.x - 34, n.y - 18, 68, 36);
                p.setBrush(QColor(Theme::BG_INPUT));
                p.setPen(QPen(is_sel ? QColor("#ffffff") : c, lw));
                p.drawRoundedRect(r, 4, 4);
                p.setPen(c);
                p.setFont(QFont("Monospace", 10, QFont::Bold));
                p.drawText(r, Qt::AlignCenter, QString::fromStdString(n.label));
            }
        }
    }

    void mousePressEvent(QMouseEvent* e) override {
        if ((e->modifiers() & Qt::ShiftModifier) || e->button() == Qt::RightButton) {
            auto* hn = hitNode(e->pos());
            if (!hn) { wire_src_id.clear(); update(); return; }
            if (wire_src_id.empty()) {
                wire_src_id = hn->id;
                update();
            } else {
                if (wire_src_id != hn->id) {
                    bool found = false;
                    for (auto& ed : *edges) {
                        if (ed.src_id == wire_src_id && ed.dst_id == hn->id) { ed.sign = -ed.sign; found = true; break; }
                    }
                    if (!found) {
                        edges->push_back({"E" + std::to_string(edges->size() + 10), wire_src_id, hn->id, +1});
                    }
                    if (onTopologyModified) onTopologyModified();
                }
                wire_src_id.clear();
                update();
            }
            return;
        }

        if (auto* hn = hitNode(e->pos())) {
            selected_node_id = hn->id;
            selected_edge_id.clear();
            drag_node_id = hn->id;
            drag_offset = QPointF(e->pos().x() - hn->x, e->pos().y() - hn->y);
            if (onNodeSelected) onNodeSelected(hn->id);
            update();
            return;
        }
        if (auto* he = hitEdge(e->pos())) {
            selected_edge_id = he->id;
            selected_node_id.clear();
            if (onEdgeSelected) onEdgeSelected(he->id);
            update();
        }
    }

    void mouseMoveEvent(QMouseEvent* e) override {
        if (!drag_node_id.empty() && (e->buttons() & Qt::LeftButton)) {
            for (auto& n : *nodes) {
                if (n.id == drag_node_id) {
                    n.x = std::max(45.0, e->pos().x() - drag_offset.x());
                    n.y = std::max(45.0, e->pos().y() - drag_offset.y());
                    break;
                }
            }
            update();
        }
    }

    void mouseReleaseEvent(QMouseEvent*) override { drag_node_id.clear(); }
};

