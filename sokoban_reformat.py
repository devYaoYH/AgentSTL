import os
import sys

if (sys.version_info.major == 2):
    input = raw_input

# Grab max width and pad lines with '.'
def prep_board(board_lines):
    max_width = 0
    for i in range(len(board_lines)):
        board_lines[i] = board_lines[i].replace(' ','.')
        max_width = max(len(board_lines[i]), max_width)
    # Pad with extra '.'
    for i in range(len(board_lines)):
        board_lines[i] += '.'*(max_width-len(board_lines[i]))
    return board_lines

f_in = input("File to read from: ")
with open(f_in, 'r') as fin:
    fin_dir = '/'.join(os.path.split(f_in)[:-1])
    fin_name = os.path.split(f_in)[-1].split('.')[0]
    board_lines = []
    puzzle_num = 1
    # Read till line starting with ';'
    for line in fin:
        line = line.rstrip('\n')
        if (len(line) < 1):
            continue
        if (line[0] == ';'):
            # End of 1 board input -> read name and write to file
            # name = '_'.join(line.split()[1:])
            name = str(puzzle_num)
            puzzle_num += 1
            with open(os.path.join(fin_dir,"{}_{}.in".format(fin_name, name)), 'w+') as fout:
                board_lines = prep_board(board_lines)
                fout.write("{} {}\n".format(len(board_lines), len(board_lines[0])))
                for out_line in board_lines:
                    fout.write(out_line)
                    fout.write("\n")
            # fin.readline()  # Skip over newline
            board_lines = []
        else:
            # Read line as game input line
            formatted_line = line
            # Replace '.' with '_'  [goal]
            formatted_line = formatted_line.replace('.','_')
            # Replace '@' with 'o'  [player]
            formatted_line = formatted_line.replace('@','o')
            # Replace '+' with '!'  [player ON goal]
            formatted_line = formatted_line.replace('+','!')
            # Replace '*' with '@'  [box on goal]
            formatted_line = formatted_line.replace('*','@')
            # Replace '$' with 'x'  [box]
            formatted_line = formatted_line.replace('$','x')
            # Add to current board
            board_lines.append(formatted_line)