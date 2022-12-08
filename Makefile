
MAKE=make

all:
	@$(MAKE) -C src/socksServer
	@$(MAKE) -C src/consoleCgi
	@cp ./src/socksServer/socks_server ./socks_server
	@cp ./src/consoleCgi/console.cgi ./console.cgi



clean:
	@$(MAKE) clean -C src/socksServer
	@$(MAKE) clean -C src/consoleCgi
	@rm -f cgi_server
	@rm -f console.cgi

run:
	@$(MAKE) run -C src/consoleCgi
	@$(MAKE) run -C src/socksServer


zip:
	@make clean
	@rm -f 311511034.zip
	@rm -rf 311511034
	@mkdir 311511034
	@mkdir -p 311511034/src/socksServer/src
	@mkdir -p 311511034/src/consoleCgi/src
	@cp src/socksServer/Makefile 311511034/src/socksServer/
	@cp src/socksServer/src/*.cpp 311511034/src/socksServer/src
	@cp src/socksServer/src/*.h 311511034/src/socksServer/src
	@cp src/consoleCgi/Makefile 311511034/src/consoleCgi/
	@cp src/consoleCgi/src/*.cpp 311511034/src/consoleCgi/src
	@cp src/consoleCgi/src/*.h 311511034/src/consoleCgi/src
	@cp Makefile 311511034
	@zip -r 311511034.zip 311511034
	@rm -rf 311511034