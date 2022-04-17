all: comps_js

comps_js: make_comps_js
	./make_comps_js

make_comps_js: make_comps_js.cpp fs_tools.cpp
	g++ --std=c++17 -omake_comps_js make_comps_js.cpp fs_tools.cpp

format:
	cp clang_format_cpp .clang-format
	clang-format -i fs_tools.cpp fs_tools.h make_comps_js.cpp