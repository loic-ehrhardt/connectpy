from . import connectlib as cpp
from .connectlib import Board
from .connectlib import GameStatus
from .connectlib import Solver

import os
import time

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

    def run(self, benchmark_name):
        compute_times = []
        explored_positions = []
        for (sequence, score) in self.benchmark_data[benchmark_name]:
            solver = Solver()
            t0 = time.time()
            computed_score = solver.solve(Board(sequence))
            runtime = time.time() - t0
            assert computed_score == score, (sequence, score, computed_score)
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
    board1 = Board("44455554221")
    assert board1.status == GameStatus.InProgress
    assert repr(board1) == _format([
        ".......",
        ".......",
        "...XO..",
        "...OX..",
        ".X.XO..   11 moves",
        "OO.OX..   X's turn"])
    board2 = Board("4455326")
    assert board2.status == GameStatus.Player1Wins
    assert repr(board2) == _format([
        ".......",
        ".......",
        ".......",
        ".......",
        "...XX..   7 moves",
        ".XOOOO.   winner: O"])
    board3 = Board("121212212121343434434343565656656565777777")
    assert board3.status == GameStatus.Draw
    assert repr(board3) == _format([
        "XOXOXOX",
        "XOXOXOO",
        "XOXOXOX",
        "OXOXOXO",
        "OXOXOXX   42 moves",
        "OXOXOXO   draw"])
