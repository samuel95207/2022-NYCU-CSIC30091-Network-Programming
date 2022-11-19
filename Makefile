

part1:
	@$(MAKE) -C src/part1-http
	@$(MAKE) -C src/part1-cgi
	@cp src/part1-http/http_server ./http_server
	@cp src/part1-cgi/console.cgi ./console.cgi


part2:
	@$(MAKE) -C src/part2
	@cp src/part2/cgi_server.exe ./cgi_server.exe


clean1:
	@$(MAKE) clean -C src/part1-http
	@$(MAKE) clean -C src/part1-cgi

clean2:
	@$(MAKE) clean -C src/part2


clean: clean1 clean2
	rm -f http_server
	rm -f console.cgi
	rm -f cgi_server.exe


run1: part1
	@$(MAKE) run -C src/part1-cgi
	@$(MAKE) run -C src/part1-http

run2: part2
	@$(MAKE) run -C src/part2


zip:
	@make clean
	@rm -f 311511034.zip
	@rm -rf 311511034
	@mkdir 311511034
	@mkdir -p 311511034/src/part1-http/src
	@mkdir -p 311511034/src/part1-cgi/src
	@mkdir -p 311511034/src/part2/src
	@cp src/part1-http/Makefile 311511034/src/part1-http/
	@cp src/part1-http/src/*.cpp 311511034/src/part1-http/src
	@cp src/part1-http/src/*.h 311511034/src/part1-http/src
	@cp src/part1-cgi/Makefile 311511034/src/part1-cgi/
	@cp src/part1-cgi/src/*.cpp 311511034/src/part1-cgi/src
	@cp src/part1-cgi/src/*.h 311511034/src/part1-cgi/src
	@cp src/part2/Makefile 311511034/src/part2/
	@cp src/part2/src/*.cpp 311511034/src/part2/src
	@cp src/part2/src/*.h 311511034/src/part2/src
	@cp Makefile 311511034
	@zip -r 311511034.zip 311511034
	@rm -rf 311511034