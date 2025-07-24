all: micro80bios.js micro80font.js test.js

micro80bios.js: micro80.bin Makefile
	echo "const bios = [" >micro80bios.js
	xxd -i micro80.bin | grep , >>micro80bios.js
	echo "];" >>micro80bios.js

test.js: test.com Makefile
	echo "const testApp = [" >test.js
	xxd -i test.com | grep , >>test.js
	echo "];" >>test.js

micro80font.js: font.bin Makefile
	echo "const font = [" >micro80font.js
	xxd -i font.bin | grep , >>micro80font.js
	echo "];" >>micro80font.js
