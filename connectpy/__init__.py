from . import connectlib as cpp
from .connectlib import Board
from .connectlib import GameStatus
from .connectlib import Solver

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


