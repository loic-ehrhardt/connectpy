from . import connectlib as cpp
from .connectlib import Board
from .connectlib import GameStatus
from .connectlib import OpeningBook
from .connectlib import Solver
from .connectlib import TranspositionTable

import os
import time
import sys

class Benchmark:
    def __init__(self):
        self.load_benchmarks()

    def load_benchmarks(self):
        self.benchmark_names = sorted([
            filename
            for filename in os.listdir("benchmarks")
            if filename.startswith("Test_")])
        self.benchmark_data = dict()
        for benchmark_name in self.benchmark_names:
            with open(os.path.join("benchmarks", benchmark_name), "r") as f:
                for line in f:
                    (sequence, score_str) = line.split()
                    self.benchmark_data.setdefault(benchmark_name, []).append(
                        (sequence, int(score_str)))

    def run_all(self):
        for benchmark_name in ["Test_L3_R1",
                               "Test_L2_R1",
                               "Test_L2_R2",
                               "Test_L1_R1",
                               "Test_L1_R2",
                               "Test_L1_R3"]:
            print(f"=== {benchmark_name} weak ===")
            self.run(benchmark_name, use_weak_solver=True)
            print()
            print(f"=== {benchmark_name} strong ===")
            self.run(benchmark_name, use_weak_solver=False)
            print()
            sys.stdout.flush()

    def run(self, benchmark_name, use_weak_solver=False):
        def _sign(x):
            if x == 0:
                return 0
            return x / abs(x)
        if use_weak_solver:
            solve_func = lambda s, b: s.dichotomicSolve(b, use_weak_solver=True)
        else:
            solve_func = lambda s, b: s.dichotomicSolve(b)

        compute_times = []
        explored_positions = []
        solver = Solver()
        for (sequence, score) in self.benchmark_data[benchmark_name]:
            solver.reset()
            t0 = time.time()
            computed_score = solve_func(solver, Board(sequence))
            runtime = time.time() - t0
            assert computed_score == (
                _sign(score) if use_weak_solver else score), (
                    sequence, score, computed_score)
            compute_times.append(runtime)
            explored_positions.append(solver.num_explored_pos)
        N = float(len(compute_times))
        mean_compute_time = sum(compute_times) / N
        mean_explored_positions = sum(explored_positions) / N
        print("mean compute time: %.3f ms" % (mean_compute_time * 1e3,))
        print("mean explored pos: %.2f" % (mean_explored_positions,))
        print("K pos / seconds:   %.2f" % (
            0.001 * mean_explored_positions / mean_compute_time,))


class InteractiveGame:
    def __init__(self):
        self.board = Board()
        self.solver = Solver()
        opening_book_path = os.path.join(
            os.path.dirname(os.path.realpath(__file__)),
            "opening_book_8.bin")
        self.opening_book = OpeningBook(opening_book_path)
        self._score_by_key = dict()
        self._undo_states = [self.board.key()]
        self._undo_ix = 0
        try:
            import termios
            self.input = self._read_single_key_with_termios
        except ImportError:
            self.input = lambda : input(">>> ")

    def play(self):
        self.print_board()
        while True:
            i = self.input()
            if len(i) != 1:
                self.print_help()
            elif i in "1234567":
                if self.board.status != GameStatus.InProgress:
                    print("Game is finished.")
                elif not self.board.canPlay(int(i)):
                    print("Cannot play here.")
                else:
                    self.board.play(int(i))
                    self._undo_ix += 1
                    self._undo_states = self._undo_states[:self._undo_ix] \
                                        + [self.board.key()]
                    self.print_board()
            elif i == "u" and self._undo_ix > 0:
                self._undo_ix -= 1
                self.board = Board(self._undo_states[self._undo_ix])
                self.print_board()
            elif i == "r" and self._undo_ix < len(self._undo_states) - 1:
                self._undo_ix += 1
                self.board = Board(self._undo_states[self._undo_ix])
                self.print_board()
            elif i == "b":
                self.print_board()
            elif i == "n":
                if self.ask_confirmation("start a new game"):
                    self.board = Board()
                    self._undo_states = [self.board.key()]
                    self._undo_ix = 0
                    self.print_board()
            elif i == "q":
                if self.ask_confirmation("quit"):
                    break
            else:
                self.print_help()

    def print_board(self):
        print()
        for board_line in repr(self.board).splitlines():
            print("    " + board_line)
        print("    " + "".join(
            chr(0xff10 + i) if self.board.canPlay(i) else "\u3000"
            for i in range(1, 8)))
        if self.board.status == GameStatus.InProgress:
            play_scores = []
            for i in range(1, 8):
                self._print_score_line(play_scores, is_computing=True)
                play_scores.append(self.play_score(i))
            self._print_score_line(play_scores, is_computing=False)
        print()

    def _print_score_line(self, play_scores, is_computing):
        score_line = "\r" # go to start of line
        score_line += "    "
        max_score = max([float("-inf")] + [
            score for score in play_scores if score is not None])
        for score in play_scores:
            if score is None:
                score_line += "\u3000"
            else:
                if score < 0:
                    color_code = 91 # red fg
                elif score == 0:
                    color_code = 90 # gray fg
                else:
                    color_code = 92 # green fg
                if score == max_score:
                    color_code += 10 # bg instead of fg
                score_line += "\033[%dm" % (color_code,)
                if abs(score) < 10:
                    score_line += chr(0xff10 + abs(score))
                else:
                    score_line += str(abs(score))
                score_line += "\033[0m" # reset
        if is_computing:
            print(score_line + "\uff0d", end="")
            sys.stdout.flush()
        else:
            print(score_line)

    def print_help(self):
        print("1, 2, ..., 7  play on this column")
        print("u             undo")
        print("r             redo")
        print("b             print board")
        print("n             start a new game")
        print("q             quit")
        print("otherwise     display help")

    def ask_confirmation(self, action):
        i = input(f">>> Are you sure you want to {action}? (y/n) ")
        return i.lower() in ("y", "yes")

    def score(self, key):
        if key not in self._score_by_key:
            b = Board(key)
            if b.moves <= self.opening_book.depth:
                (is_found, score) = self.opening_book[b]
                assert is_found, key
            else:
                score = self.solver.dichotomicSolve(b)
            self._score_by_key[key] = score
        return self._score_by_key[key]

    def play_score(self, col):
        if not self.board.canPlay(col):
            return None
        b = Board(self.board.key())
        b.play(col)
        # Negative score because point of view of current player:
        return -self.score(b.key())

    def _read_single_key_with_termios(self):
        import termios, tty
        fd = sys.stdin.fileno()
        old_settings = termios.tcgetattr(fd)
        print(">>> ", end="")
        sys.stdout.flush()
        try:
            tty.setraw(fd)
            ch = sys.stdin.read(1)
        finally:
            termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
        print(ch)
        return ch


def test_Board():
    def _format(lines):
        large_red_circle    = "\U0001F534"
        large_yellow_circle = "\U0001F7E1"
        white_square_button = "\U0001F533"
        return "\n".join(lines).replace(".", white_square_button) \
                               .replace("O", large_red_circle) \
                               .replace("X", large_yellow_circle)
    def _other_asserts(board):
        assert repr(Board(board.key())) == repr(Board(board.key()))
        assert board.key() == Board(board.symmetricKey()).symmetricKey()

    board1 = Board("44455554221")
    assert board1.status == GameStatus.InProgress
    assert repr(board1) == _format([
        ".......",
        ".......",
        "...XO..",
        "...OX..",
        ".X.XO..   11 moves",
        "OO.OX..   X's turn"])
    _other_asserts(board1)

    board2 = Board("4455326")
    assert board2.status == GameStatus.Player1Wins
    assert repr(board2) == _format([
        ".......",
        ".......",
        ".......",
        ".......",
        "...XX..   7 moves",
        ".XOOOO.   winner: O"])
    _other_asserts(board2)

    board3 = Board("121212212121343434434343565656656565777777")
    assert board3.status == GameStatus.Draw
    assert repr(board3) == _format([
        "XOXOXOX",
        "XOXOXOO",
        "XOXOXOX",
        "OXOXOXO",
        "OXOXOXX   42 moves",
        "OXOXOXO   draw"])
    _other_asserts(board3)

def test_TranspositionTable():
    t = TranspositionTable(10)
    for i in range(13):
        t[i] = 10 * (i - 8)
    assert [(i, t[i]) for i in range(15)] == [
        (0, (False,  0)), (1, (False,  0)), (2, (False,  0)), (3, (True, -50)),
        (4, (True, -40)), (5, (True, -30)), (6, (True, -20)), (7, (True, -10)),
        (8, (True,   0)), (9, (True,  10)), (10, (True, 20)), (11, (True, 30)),
        (12, (True, 40)), (13, (False, 0)), (14, (False, 0))]
