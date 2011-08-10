BUILD_DIR=build

.PHONY: all debug release doc clean

all:
	cd $(BUILD_DIR) && make -j || echo "Type either \"make debug\" or \"make release\"!"

debug:
	cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=Debug .. && make -j

release:
	cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=Release .. && make -j

doc:
	cd $(BUILD_DIR) && make doc -j

test:
	cd $(BUILD_DIR) && make test

clean:
	cd $(BUILD_DIR) && rm -rf *
	rm -rf html
