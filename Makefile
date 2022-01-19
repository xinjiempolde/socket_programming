.phony: all
all: server.app client.app webget

webget: http.c
	gcc $< -o $@

%.app: %.c
	gcc $< -o $@

.phony: clean
clean:
	rm -rf *.app webget