#include <pybind11/pybind11.h>
#include <sstream>
#include <iostream>

namespace py = pybind11;

class Board {
public:
    static const int WIDTH = 7;
    static const int HEIGHT = 6;

    enum Status {
        InProgress,
        Draw,
        Player1Wins,
        Player2Wins,
    };

    Board() {
        for (int i = 0; i < HEIGHT; ++i)
            for (int j = 0; j < WIDTH; ++j)
                board_[i][j] = 0;
        for (int j = 0; j < WIDTH; ++j)
            height_[j] = 0;
        moves_ = 0;
        status_ = Status::InProgress;
    }

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
                if (board_[i][j] == 1) {
                    os << large_red_circle;;
                } else if (board_[i][j] == 2) {
                    os << large_yellow_circle;
                } else {
                    os << white_square_button;
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
        return status_ == Status::InProgress && col > 0 && col <= WIDTH
            && height_[col - 1] < HEIGHT;
    }

    void play(int col) {
        assertCanPlay(col);
        int player = (moves_ % 2) + 1;

        if (isWinningMove(col))
            status_ = player == 1 ? Status::Player1Wins : Status::Player2Wins;

        board_[height_[col - 1]][col - 1] = player;
        height_[col - 1]++;
        moves_++;

        if (status_ == Status::InProgress && moves_ == HEIGHT * WIDTH)
            status_ = Status::Draw;
    }

    void play(std::string sequence) {
        for (int i = 0; i < sequence.size(); i++) {
            int col = (int) (sequence[i] - '1') + 1;
            play(col);
        }
    }

    bool isWinningMove(int col) const {
        if (!canPlay(col))
            return false;
        int player = (moves_ % 2) + 1;
        int j = col - 1;
        int i = height_[j];
        // Vertical.
        if (i >= 3 && board_[i - 1][j] == player
               && board_[i - 2][j] == player
               && board_[i - 3][j] == player)
            return true;
        // Horizontal.
        int h;
        int n = 1;
        for (h = 1; h <= 3; ++h) {
            if (j - h < 0 || board_[i][j - h] != player)
                break;
            n++;
        }
        for (h = 1; h <= 3; ++h) {
            if (j + h >= WIDTH || board_[i][j + h] != player)
                break;
            n++;
        }
        if (n >= 4)
            return true;
        // Diagonal (/).
        n = 1;
        for (h = 1; h <= 3; ++h) {
            if (j - h < 0 || i - h < 0
                    || board_[i - h][j - h] != player)
                break;
            n++;
        }
        for (h = 1; h <= 3; ++h) {
            if (j + h >= WIDTH || i + h >= HEIGHT
                || board_[i + h][j + h] != player)
                break;
            n++;
        }
        if (n >= 4)
            return true;
        // Diagonal (\).
        n = 1;
        for (h = 1; h <= 3; ++h) {
            if (j - h < 0 || i + h >= HEIGHT
                    || board_[i + h][j - h] != player)
                break;
            n++;
        }
        for (h = 1; h <= 3; ++h) {
            if (j + h >= WIDTH || i - h < 0
                || board_[i - h][j + h] != player)
                break;
            n++;
        }
        if (n >= 4)
            return true;
        // No alignment.
        return false;
    }

    int getMoves() const {
        return moves_;
    }

    Status getStatus() const {
        return status_;
    }

private:
    int height_[WIDTH];
    int board_[HEIGHT][WIDTH];
    int moves_;
    Status status_;

    bool hasAlignment() const {
        // Horizontal.
        for (int i = 0; i < HEIGHT; ++i) {
            for (int j = 0; j < WIDTH - 3; ++j) {
                if (board_[i][j] > 0
                        && board_[i][j] == board_[i][j + 1]
                        && board_[i][j] == board_[i][j + 2]
                        && board_[i][j] == board_[i][j + 3])
                    return true;
            }
        }
        // Vertical.
        for (int i = 0; i < HEIGHT - 3; ++i) {
            for (int j = 0; j < WIDTH; ++j) {
                if (board_[i][j] > 0
                        && board_[i][j] == board_[i + 1][j]
                        && board_[i][j] == board_[i + 2][j]
                        && board_[i][j] == board_[i + 3][j])
                    return true;
            }
        }
        // Diagonal.
        for (int i = 0; i < HEIGHT - 3; ++i) {
            for (int j = 0; j < WIDTH - 3; ++j) {
                if (board_[i][j] > 0
                        && board_[i][j] == board_[i + 1][j + 1]
                        && board_[i][j] == board_[i + 2][j + 2]
                        && board_[i][j] == board_[i + 3][j + 3])
                    return true;
                if (board_[i + 3][j] > 0
                        && board_[i + 3][j] == board_[i + 2][j + 1]
                        && board_[i + 3][j] == board_[i + 1][j + 2]
                        && board_[i + 3][j] == board_[i][j + 3])
                    return true;
            }
        }
        return false;
    }

    void assertCanPlay(int col) const {
        if (canPlay(col))
        return;
        std::ostringstream os;
        os << "Cannot play there (" << col << ").";
        throw std::runtime_error(os.str());
    }
};

class Solver {
public:
    Solver() : num_explored_pos_(0) {}

    int negamax(const Board& B) {
        num_explored_pos_++;
        if (B.getStatus() == Board::Draw) {
            return 0;
        } else if (B.getStatus() == Board::Player1Wins
                || B.getStatus() == Board::Player2Wins) {
            return (B.getMoves() - Board::WIDTH * Board::HEIGHT) / 2 - 1;
        }

        // Shortcut if direct win.
        for (int col = 1; col <= Board::WIDTH; ++col) {
            if (B.isWinningMove(col)) {
                return 1 - (B.getMoves() + 1 - Board::WIDTH * Board::HEIGHT) / 2;
            }
        }

        int bestScore = -Board::WIDTH * Board::HEIGHT - 2; // lower bound
        for (int col = 1; col <= Board::WIDTH; ++col) {
            if (B.canPlay(col)) {
                Board B2(B);
                B2.play(col);
                int score = -negamax(B2);
                if (score > bestScore)
                    bestScore = score;
            }
        }
        return bestScore;
    }

    int getNumExploredPos() const {
        return num_explored_pos_;
    }

private:
    int num_explored_pos_;
};






PYBIND11_MODULE(connectlib, m) {
    py::class_<Board>(m, "Board")
        .def(py::init<>())
        .def(py::init<std::string>())
        .def("__repr__",
            [](const Board& b) { return b.toString(); })

        .def("canPlay", &Board::canPlay)
        .def("play", static_cast<void (Board::*)(int)>(&Board::play))
        .def("play", static_cast<void (Board::*)(std::string)>(&Board::play))
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
        .def("solve", &Solver::negamax)
        .def_property_readonly("num_explored_pos", &Solver::getNumExploredPos);
}
