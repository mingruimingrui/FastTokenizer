.PHONY: \
	build \
	clean \
	download-build-static-deps

build:
	( \
		mkdir -p build/release; \
		cd build/release; \
		cmake ../.. \
			-DBUILD_SHARED_LIBS:BOOL=OFF \
			-DCMAKE_BUILD_TYPE=Release; \
		make -j; \
	)

clean:
	rm -rf build bindings/python/fasttokenizer/*.so

download-build-static-deps:
	@mkdir -p deps

	@echo "Downloading pybind"
	curl -L -o deps/pybind-v2.5.0.tar.gz \
		https://github.com/pybind/pybind11/archive/v2.5.0.tar.gz
	tar -C deps -xf deps/pybind-v2.5.0.tar.gz

	@echo "Downloading CLI11"
	curl -L -o deps/CLI11-v1.9.1.tar.gz \
		https://github.com/CLIUtils/CLI11/archive/v1.9.1.tar.gz
	tar -C deps -xf deps/CLI11-v1.9.1.tar.gz

	@echo "Downloading and building ICU"
	curl -L -o deps/icu4c-67_1-src.tgz \
		https://github.com/unicode-org/icu/releases/download/release-67-1/icu4c-67_1-src.tgz
	tar -C deps -xf deps/icu4c-67_1-src.tgz
	( \
		cd deps/icu/source; \
		CFLAGS="-fPIC" CXXFLAGS="-fPIC -std=c++11" ./configure \
			--disable-shared --enable-static; \
		make -j8; \
	)
