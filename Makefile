build: node_modules
	node build

serve: node_modules
	node build serve

deploy: node_modules
	node build
	rsync -avz build/* tcrawley.org:tcrawley.org    	

node_modules: package.json
	npm install

clean:
	rm -rf build

full_clean: clean
	rm -rf node_modules

.PHONY: build
