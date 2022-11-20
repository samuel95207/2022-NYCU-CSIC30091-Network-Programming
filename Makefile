ifeq ($(OS), Windows_NT)
	MAKEDIR = powershell IF(-not (Test-Path $(1))){mkdir $(1)}
	REMOVE = powershell IF(Test-Path $(1)){rm -r -fo $(1)}
	COPY = Copy
else
	MAKEDIR = mkdir -p $(1)
	REMOVE = rm -rf $(1)
	COPY = cp
endif



MAKE=make

part1:
	@$(MAKE) -C src/part1-http
	@$(MAKE) -C src/part1-cgi
	@$(COPY) ./src/part1-http/http_server ./http_server
	@$(COPY) ./src/part1-cgi/console.cgi ./console.cgi


part2:
	@$(MAKE) -C src/part2
	@$(COPY) ".\src\part2\cgi_server.exe" ".\cgi_server.exe"


clean1:
	@-$(MAKE) clean -C src/part1-http
	@-$(MAKE) clean -C src/part1-cgi

clean2:
	@-$(MAKE) clean -C src/part2


clean: clean1 clean2
	@-$(call REMOVE,http_server)	 
	@-$(call REMOVE,console.cgi)
	@-$(call REMOVE,cgi_server.exe)


run_np_single:
	@cd working_dir_np_single;./np_single_golden 10000

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
	@cp -r src/part2/src/* 311511034/src/part2/src
	@rm -rf 311511034/src/part2/obj
	@cp Makefile 311511034
	@zip -r 311511034.zip 311511034
	@rm -rf 311511034