build: 
	cmake -B build -S ./app 
	cmake --build build 

setup_backend: 
	git clone https://github.com/CPS-research-group/kedro-umbrella.git
	cd kedro-umbrella && pip install -e .

test:
	cd build && QT_QPA_PLATFORM=offscreen ctest --output-on-failure

.PHONY: build
