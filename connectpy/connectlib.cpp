#include <pybind11/pybind11.h>
#include <sstream>
#include <iostream>

namespace py = pybind11;

class Board {
public:
    static const int WIDTH = 7;
    static const int HEIGHT = 6;

    Board() {
        for (int i = 0; i < HEIGHT; ++i)
            for (int j = 0; j < WIDTH; ++j)
                board_[i][j] = 0;
        for (int j = 0; j < WIDTH; ++j)
            height_[j] = 0;
        moves_ = 0;
        winner_ = 0;
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
            } else if (i == 0 && winner_ == 0 && moves_ == WIDTH * HEIGHT) {
                os << "   draw";
            } else if (i == 0 && winner_ > 0) {
                os << "   winner: "
                   << (winner_ == 1 ? large_red_circle
                                    : large_yellow_circle);
            } else if (i == 0) {
                os << "   "
                   << (moves_ % 2 == 0 ? large_red_circle
                                       : large_yellow_circle)
                   << "'s turn";
            }
            if (i > 0)
                os << "\n";
        }
        return os.str();
    }

    bool canPlay(int col) const {
        return winner_ == 0 && col > 0 && col <= WIDTH
            && height_[col - 1] < HEIGHT;
    }

    void play(int col) {
        if (!canPlay(col)) {
            std::ostringstream os;
            os << "Cannot play there (" << col << ").";
            throw std::runtime_error(os.str());
        }
        int player = (moves_ % 2) + 1;
        board_[height_[col - 1]][col - 1] = player;
        height_[col - 1]++;
        moves_++;
        if (hasAlignment())
            winner_ = player;
    }

    void play(std::string sequence) {
        for (int i = 0; i < sequence.size(); i++) {
            int col = (int) (sequence[i] - '1') + 1;
            play(col);
        }
    }

private:
    int height_[WIDTH];
    int board_[HEIGHT][WIDTH];
    int moves_;
    int winner_;

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
};

class Solver {
public:
    Solver() : num_explored_pos_(0) {}

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

        // https://github.com/pybind/pybind11/issues/682
        .def_property_readonly_static("WIDTH",
            [](py::object) { return Board::WIDTH; })
        .def_property_readonly_static("HEIGHT",
            [](py::object) { return Board::HEIGHT; });
}