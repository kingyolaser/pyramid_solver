
pyramid: pyramid.cpp
	g++ -Wall -o $@ pyramid.cpp

pyramid_test: pyramid.cpp
	g++ -g -Wall -DTEST -o $@ pyramid.cpp -lcppunit


run: pyramid
	./$<

bench: pyramid
	./$<  7 3j 620 280j 4j0ak 7k5q58 12k8462 4q17q0jkq716593394863599

bench2: pyramid
	./$< 2 q5 29k 710k 40657 kkq63j 7aj353j 895q44898j22q96137401860

bench3: pyramid
	./$< 9 29 845 j200 75j5k 7a04qa kq32671 j3k89j4467829a6653q3qk80

test: pyramid_test
	./$< --test

clean:
	rm -f pyramid.exe
	rm -f pyramid_test.exe
	rm -f *.stackdump
