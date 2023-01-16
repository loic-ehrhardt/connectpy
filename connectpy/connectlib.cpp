#include <pybind11/pybind11.h>
#include <sstream>
#include <iostream>

namespace py = pybind11;

class Board {
public:
    static const int WIDTH = 7;
    static const int HEIGHT = 6;
    static_assert(WIDTH < 10);
    static_assert(WIDTH * (HEIGHT + 1) <= 64);

    enum Status {
        InProgress,
        Draw,
        Player1Wins,
        Player2Wins,
    };

    Board() : mask_(0), position_(0), moves_(0), status_(Status::InProgress) {}

    Board(std::string sequence) : Board() {
        play(sequence);
    }

    std::string toString() const {
        static const std::string large_red_circle = "\xF0\x9F\x94\xB4";
        static const std::string large_yellow_circle = "\xF0\x9F\x9F\xA1";
        static const std::string white_square_button = "\xF0\x9F\x94\xB3";
        std::ostringstream os;
        for (int i = HEIGHT - 1; i >= 0; --i) {
            for (int j = 0; j < WIDTH; ++j) {
                uint64_t bitmask = UINT64_C(1) << i << j * (HEIGHT + 1);
                if ((mask_ & bitmask) == 0) {
                    os << white_square_button;
                } else if (((position_ & bitmask) == 0) ^ ((moves_ % 2) == 0)) {
                    os << large_red_circle;
                } else {
                    os << large_yellow_circle;
                }
            }
            if (i == 1) {
                os << "   " << moves_ << " moves";
            } else if (i == 0 && status_ == Status::InProgress) {
                os << "   "
                   << (moves_ % 2 == 0 ? large_red_circle
                                       : large_yellow_circle)
                   << "'s turn";
            } else if (i == 0 && status_ == Status::Draw) {
                os << "   draw";
            } else if (i == 0) {
                os << "   winner: "
                   << (status_ == Status::Player1Wins ? large_red_circle
                                                      : large_yellow_circle);
            }
            if (i > 0)
                os << "\n";
        }
        return os.str();
    }

    bool canPlay(int col) const {
        return status_ == Status::InProgress && col >= 0 && col < WIDTH
            && (mask_ & topMask(col)) == 0;
    }

    void assertCanPlay(int col) const {
        if (canPlay(col))
            return;
        std::ostringstream os;
        os << "Cannot play there (" << col << ").";
        throw std::runtime_error(os.str());
    }

    void play(int col, bool check_alignment=true) {
        position_ ^= mask_;
        mask_ |= mask_ + bottomMask(col);

        if (check_alignment && hasAlignment(position_ ^ mask_))
            status_ = (moves_ % 2) == 0 ? Status::Player1Wins : Status::Player2Wins;

        moves_++;

        if (status_ == Status::InProgress && moves_ == HEIGHT * WIDTH)
            status_ = Status::Draw;
    }

    void play(std::string sequence) {
        for (unsigned int i = 0; i < sequence.size(); i++) {
            int col = (int) (sequence[i] - '1'); // "1" -> 0, "2" -> 1, etc.
            assertCanPlay(col);
            play(col);
        }
    }

    static bool hasAlignment(uint64_t pos) {
        // Horizontal.
        uint64_t m = pos & (pos << (HEIGHT + 1));
        if (m & (m << (2 * (HEIGHT + 1))))
            return true;
        // Diagonal (\).
        m = pos & (pos << HEIGHT);
        if (m & (m << (2 * HEIGHT)))
            return true;
        // Diagonal (/).
        m = pos & (pos << (HEIGHT + 2));
        if (m & (m << (2 * (HEIGHT + 2))))
            return true;
        // Vertical.
        m = pos & (pos << 1);
        if (m & (m << 2))
            return true;
        // No alignment found.
        return false;
    }

    bool isWinningMove(int col) const {
        return hasAlignment(position_ | (
            (mask_ + bottomMask(col)) & columnMask(col)));
    }

    int getMoves() const {
        return moves_;
    }

    Status getStatus() const {
        return status_;
    }

private:
    // Positions are stored with two bitfields. The bits correspond to the
    // following positions:
    //     .   .   .   .   .   .   .
    //     5  12  19  26  33  40  47
    //     4  11  18  25  32  39  46
    //     3  10  17  24  31  38  45
    //     2   9  16  23  30  37  44
    //     1   8  15  22  29  36  43
    //     0   7  14  21  28  35  42
    // mask is 1 for non-empty cells:
    uint64_t mask_;
    // position is 1 if a non-empty cell is for the current player:
    uint64_t position_;

    int moves_;
    Status status_;

    static uint64_t topMask(int col) {
        return (UINT64_C(1) << (HEIGHT - 1)) << col * (HEIGHT + 1);
    }

    static uint64_t bottomMask(int col) {
        return UINT64_C(1) << col * (HEIGHT + 1);
    }

    static uint64_t columnMask(int col) {
        return ((UINT64_C(1) << HEIGHT) - 1) << col * (HEIGHT + 1);
    }
};

class Solver {
public:
    Solver() : num_explored_pos_(0) {
        // Explore columns from the middle first.
        for (int i = 0; i < Board::WIDTH; ++i)
            column_order_[i] = Board::WIDTH / 2
                + ( 1 - 2 * (i % 2)) * (i + 1) / 2;
    }

    int negamax(const Board& B) {
        int max_score = Board::WIDTH * Board::HEIGHT / 2;
        return negamax(B, -max_score, max_score);
    }

    int negamax(const Board& B, int alpha, int beta) {
        num_explored_pos_++;

        // Check board status.
        if (B.getStatus() == Board::Draw) {
            return 0;
        } else if (B.getStatus() == Board::Player1Wins
                || B.getStatus() == Board::Player2Wins) {
            return (B.getMoves() - Board::WIDTH * Board::HEIGHT) / 2 - 1;
        }

        int max_score = (1 + Board::WIDTH * Board::HEIGHT - B.getMoves()) / 2; // direct win

        // Shortcut if direct win.
        for (int col = 0; col < Board::WIDTH; ++col) {
            if (B.canPlay(col) && B.isWinningMove(col)) {
                return max_score;
            }
        }
        // Cannot win directly, max score decreases.
        max_score--;

        // Prune beta with max score.
        if (max_score < beta) {
            beta = max_score;
            if (alpha >= beta) {
                // Empty alpha-beta range.
                if (alpha > beta)
                    throw std::runtime_error("alpha > beta ?");
                return beta;
            }
        }

        // Recursive exploration.
        for (int i = 0; i < Board::WIDTH; ++i) {
            if (B.canPlay(column_order_[i])) {
                Board B2(B);
                B2.play(column_order_[i], false);
                int score = -negamax(B2, -beta, -alpha);
                if (score >= beta) {
                    // Outside research range (can happen for weak solver).
                    return beta;
                } else if (score > alpha) {
                    // Prune alpha (keeps track of best score).
                    alpha = score;
                }
            }
        }
        return alpha; // best score obtained.
    }

    int getNumExploredPos() const {
        return num_explored_pos_;
    }

private:
    uint64_t num_explored_pos_;
    int column_order_[Board::WIDTH];
};




PYBIND11_MODULE(connectlib, m) {
    py::class_<Board>(m, "Board")
        .def(py::init<>())
        .def(py::init<std::string>())
        .def("__repr__",
            [](const Board& b) { return b.toString(); })

        .def("play", static_cast<void (Board::*)(std::string)>(&Board::play))
        .def("play", [](Board& b, int col) {
                b.assertCanPlay(col - 1);
                b.play(col - 1); })
        .def_property_readonly("moves", &Board::getMoves)
        .def_property_readonly("status", &Board::getStatus)

        // https://github.com/pybind/pybind11/issues/682
        .def_property_readonly_static("WIDTH",
            [](py::object) { return Board::WIDTH; })
        .def_property_readonly_static("HEIGHT",
            [](py::object) { return Board::HEIGHT; });

    py::enum_<Board::Status>(m, "GameStatus")
        .value("InProgress", Board::Status::InProgress)
        .value("Draw", Board::Status::Draw)
        .value("Player1Wins", Board::Status::Player1Wins)
        .value("Player2Wins", Board::Status::Player2Wins)
        .export_values();

    py::class_<Solver>(m, "Solver")
        .def(py::init<>())
        .def("solve", [](Solver& s, const Board& b)
            { return s.negamax(b); })
        .def("solve_weak", [](Solver& s, const Board& b)
            { return s.negamax(b, -1, 1); })
        .def_property_readonly("num_explored_pos", &Solver::getNumExploredPos);
}
