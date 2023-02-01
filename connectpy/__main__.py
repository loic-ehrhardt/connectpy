from . import test_Board, test_TranspositionTable, InteractiveGame

def main():
    test_Board()
    test_TranspositionTable()
    InteractiveGame().play()

if __name__ == "__main__":
    main()
