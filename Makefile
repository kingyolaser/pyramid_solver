
pyramid: pyramid.cpp
	g++ -Wall -o $@ pyramid.cpp

pyramid_test: pyramid.cpp
	g++ -g -Wall -DTEST -o $@ pyramid.cpp -lcppunit


run: pyramid
	./$<

test: pyramid_test
	./$< --test

clean:
	rm -f pyramid.exe
	rm -f pyramid_test.exe
