CC = clang++
ARGS = -Wall -std=c++11
DEBUG = -g -fsanitize=address

all:
	${CC} ${ARGS} test_driver.cpp parse_point_csv.cpp segment_points.cpp fit_circle.cpp -o driver.out

debug:
	${CC} ${ARGS} ${DEBUG} driver.cpp FASTAreadset_HT.cpp ${OUTPUT}
