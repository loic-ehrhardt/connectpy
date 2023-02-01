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
