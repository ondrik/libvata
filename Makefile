BUILD_DIR=build
#MAKE_FLAGS=-j

.PHONY: all debug release doc clean

all:
	cd $(BUILD_DIR) && make $(MAKE_FLAGS) || echo "Type either \"make debug\" or \"make release\"!"

debug:
	cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=Debug .. && make $(MAKE_FLAGS)

release:
	cd $(BUILD_DIR) && cmake -DCMAKE_BUILD_TYPE=Release .. && make $(MAKE_FLAGS)

doc:
	cd $(BUILD_DIR) && make $(MAKE_FLAGS) doc 

test:
	cd $(BUILD_DIR) && make test

clean:
	cd $(BUILD_DIR) && rm -rf *
	rm -rf html
