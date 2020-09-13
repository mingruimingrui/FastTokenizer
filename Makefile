
foo:
	( \
		mkdir -p build; \
		cd build; \
		cmake .. \
			-DBUILD_SHARED_LIBS:BOOL=OFF \
			-DCMAKE_BUILD_TYPE=Release; \
		make -j; \
	)

clean:
	rm -rf build

download-build-static-deps:
	@mkdir -p deps

	@echo "Downloading pybind"
	curl -L -o deps/pybind-v2.5.0.tar.gz \
		https://github.com/pybind/pybind11/archive/v2.5.0.tar.gz
	tar -C deps -xf deps/pybind-v2.5.0.tar.gz

	@echo "Downloadinng and building ICU"
	curl -L -o deps/icu4c-67_1-src.tgz \
		https://github.com/unicode-org/icu/releases/download/release-67-1/icu4c-67_1-src.tgz
	tar -C deps -xf deps/icu4c-67_1-src.tgz
	( \
		cd deps/icu/source; \
		CFLAGS="-fPIC" CXXFLAGS="-fPIC -std=c++11" ./configure \
			--disable-shared --enable-static; \
		make -j8; \
	)
