build: node_modules
	node build

serve: node_modules
	node build serve

copy-worklog:
	cp -r src/clojars-worklog build/

deploy: build copy-worklog
	rsync -avz build/* tcrawley.org:tcrawley.org    	

node_modules: package.json
	npm install

clean:
	rm -rf build

full_clean: clean
	rm -rf node_modules

.PHONY: build
