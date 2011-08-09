BUILD_DIR=build

all:
	cd $(BUILD_DIR) && cmake .. && make -j

debug:
	cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=Debug .. && make -j

release:
	cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=Release .. && make -j
