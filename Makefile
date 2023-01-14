
connectlib:
	c++ -O3 -Wall -shared -std=c++11 -fPIC -I ./extern/pybind11/include `python3-config --includes` connectpy/connectlib.cpp -o connectpy/connectlib`python3-config --extension-suffix`

clean:
	rm -f connectpy/connectlib`python3-config --extension-suffix`
