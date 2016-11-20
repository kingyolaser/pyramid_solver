
pyramid: pyramid.cpp
	g++ -Wall -o $@ pyramid.cpp

pyramid_test: pyramid.cpp
	g++ -g -Wall -DTEST -o $@ pyramid.cpp -lcppunit


run: pyramid
	./$<

bench: pyramid
	./$<  7 3j 620 280j 4j0ak 7k5q58 12k8462 4q17q0jkq716593394863599

test: pyramid_test
	./$< --test

clean:
	rm -f pyramid.exe
	rm -f pyramid_test.exe
